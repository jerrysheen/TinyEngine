# ResourceBarrier 与 Pass 顺序理解

## 背景

这次讨论里，我们连续遇到了几类很容易混淆的问题：

- `SetResourceState` 和 `UAVBarrier` 到底是不是一回事
- 从一个 pass 到下一个 pass，如果资源作为 `SRV` 继续读取，要不要每次都写 barrier
- `depthAttachment` 从深度写入转成 Hi-Z 输入时，需不需要额外“等前面彻底用完”
- `mainLightShadow` 这种前面写、后面采样的资源，到底什么时候必须显式切状态
- 两个 pass 连续往同一个 `RT` 写，GPU 明明内部并行，为什么不需要额外 barrier 去保证顺序

这些问题看起来分散，但本质上都指向同一个核心：

- 我们需要把“命令顺序”“资源状态”“写入结果可见性”这三件事拆开理解

## 最初容易混在一起的点

一开始很容易形成下面这种直觉：

- 既然 GPU 是并行的，那两个 draw / dispatch 中间如果不写 barrier，后面的工作可能会抢跑
- 所以 barrier 像是一个“停一下，等前面彻底结束”的总开关
- `SetResourceState` 和 `UAVBarrier` 似乎都在干“同步”这件事，看起来只是写法不同
- 如果前后都是 `SRV`，或者前后都是 `RT`，也许为了稳妥最好还是写一个 barrier

这个直觉的问题在于：

- 它把 API 级别的“命令有序提交”
- 和资源级别的“访问合法性”
- 以及内存级别的“UAV 写结果何时可见”

混成了一件事

## 三件事必须分开看

### 1. 命令先后顺序

如果命令都在同一个 command list、同一个 queue 上：

- `Pass A` 先记录
- `Pass B` 后记录

那么从 API 语义上，它们就是有序执行的。

也就是说：

- 后面的 draw / dispatch 不会在“程序语义上”跑到前面的前面去

GPU 内部当然可以流水和并行：

- A 的一部分 shading 可能和 B 的前端准备阶段重叠
- 硬件也会尽量填满流水线

但这些内部并行不能破坏最终可观察到的顺序结果。

所以：

- barrier 不是用来保证“同一个 queue 里的 drawcall 按记录顺序执行”的主要手段
- 这个顺序本身由 command submission 语义保证

### 2. 资源状态是否合法

资源当前以什么身份被访问，是另一件事。

例如：

- 前一个 pass 把资源当 `RenderTarget` 写
- 后一个 pass 想把它当 `ShaderResource` 读

这不是“顺序问题”，而是：

- 资源访问方式变了

这时需要 `Transition Barrier`，也就是工程里的 `SetResourceState`。

它表达的是：

- 这个资源从旧状态切到新状态
- 前一种访问必须在后一种访问前被正确完成

### 3. UAV 写结果的可见性 / 有序性

`UAV` 是第三类问题。

如果一个资源前后都还是 `UAV`：

- 状态可能根本没变
- 但前一个 dispatch 的写结果，不一定天然对后一个 dispatch 立即按你预期可见

这时需要的是：

- `UAVBarrier`

它的职责不是改状态，而是：

- 告诉 GPU：这个资源前面的 UAV 访问，和后面的 UAV 访问之间，需要建立可见性和顺序约束

## 在这个工程里对应到什么

目前工程里的实现是分开的：

- `SetResourceState` 最终发的是 `Transition` barrier
- `UAVBarrier` 最终发的是 `D3D12_RESOURCE_BARRIER_TYPE_UAV`

也就是说，这个项目当前的接口语义本身就已经把两者区分开了：

- 一个管“状态切换”
- 一个管“UAV 写后同步”

## 对 `SetResourceState` 的更准确理解

`SetResourceState` 不是单纯改一个枚举值。

它更准确的含义是：

- 把资源从旧访问状态切到新访问状态
- 并为这次状态变化建立前后访问约束

所以它可以理解为：

```text
资源身份切换 + 与这次切换相关的同步
```

例如：

- `DEPTH_WRITE -> DEPTH_READ_SHADER_RESOURCE`
- `RENDER_TARGET -> SHADER_RESOURCE`
- `UNORDERED_ACCESS -> INDIRECT_ARGUMENT`

这些都属于“访问方式变了”，所以要显式 transition。

## 对 `UAVBarrier` 的更准确理解

`UAVBarrier` 的意思不是：

- “GPU 整体停下来，前面所有事情全做完，后面再继续”

更接近的是：

- “针对这个 UAV 资源，前面的 UAV 写入结果要对后续访问按顺序可见”

所以它可以理解为：

```text
不改资源状态，只补 UAV 访问之间的有序性 / 可见性
```

典型场景是：

- 同一个资源，dispatch A 作为 UAV 写
- dispatch B 仍然把它作为 UAV 读/写

这时状态没变，但通常要 `UAVBarrier`。

## 我们这次最关键的几条结论

### 结论 1：`SRV -> SRV` 一般不需要 barrier

原因很直接：

- 两边都是只读
- 没有写后读 hazard
- 资源状态也没变

所以：

- “前一个 pass 里读 shadow map，后一个 pass 里继续读 shadow map”
- 这种 `SRV -> SRV` 通常不需要额外 barrier

### 结论 2：`RT/DSV -> SRV` 必须显式 transition

如果前一个 pass 写：

- `RenderTarget`
- `DepthStencil`

后一个 pass 读：

- `SRV`

那就一定要切状态。

例如：

- `mainLightShadow` 在 shadow pass 里写
- opaque pass 里要拿来采样

这里要做的不是“额外等待”，而是：

- 明确写一个 `SetResourceState(..., SRV状态)`

### 结论 3：`SRV -> RT/DSV` 也必须显式 transition

反过来，如果资源前面是拿来读的，后面又想回到：

- `RenderTarget`
- `DepthWrite`

也一样要切状态。

### 结论 4：`RT -> RT` 一般不需要额外 barrier

这个点最容易让人困惑，因为 GPU 内部确实很并行。

但如果：

- 前一个 pass 把同一个资源当 `RT` 写
- 后一个 pass 继续把同一个资源当 `RT` 写
- 状态始终都是 `RENDER_TARGET`
- 并且它们在同一个 queue 上顺序提交

那么通常不需要额外 barrier。

原因不是“硬件完全串行”，而是：

- queue 的命令顺序本来就保证了结果语义
- GPU 可以内部流水并行
- 但不能让后面的 render target 写在语义上跑到前面的前面

所以这里不需要专门写一个“RT 到 RT 的同步 barrier”去保证 drawcall 顺序。

### 结论 5：`UAV -> UAV` 往往需要 `UAVBarrier`

如果前后都还是 UAV，用的是同一个资源：

- 状态没变
- 但需要保证前一个 dispatch 的写结果对下一个 dispatch 可见

这时要靠 `UAVBarrier`，不是靠 `SetResourceState`。

### 结论 6：`UAV -> SRV / Indirect / CopySrc...` 要关注两层语义

这类情况要同时想两件事：

- 资源身份是不是变了
- 前面的 UAV 写结果是不是要被后面正确看到

通常至少需要：

- `Transition` 到目标状态

而在一些写法里，也会保守地先补一个：

- `UAVBarrier`

再做 transition。

## 回到这次具体例子

### 例子 1：`depthAttachment` 给 Hi-Z blit 用

前面深度 pass 把 `depthAttachment` 当 `DepthWrite` 写入，后面 compute 想把它当输入纹理读。

这里真正需要的是：

- `DEPTH_WRITE -> DEPTH_READ_SHADER_RESOURCE`

也就是：

- 显式 `SetResourceState`

一般不需要再额外加一个：

- “等待 depth 前面彻底用完”的自定义同步

因为：

- 同一个 queue 上，前面 pass 的 draw、后面的 transition、再后面的 compute dispatch，本来就是顺序语义
- depth 不是 UAV 写出来的，所以这里也不是 `UAVBarrier` 的场景

### 例子 2：`mainLightShadow` 给 opaque pass 采样

如果 shadow pass 前面写的是深度贴图，后面 opaque 要当 `SRV` 采样：

- 需要从 `DepthWrite` 切到对应可读状态

这里不能因为“两个 pass 本来就有先后”就省掉 transition。

因为真正的问题不是“谁先执行”，而是：

- 后一个 pass 的访问身份已经变了

### 例子 3：indirect args 从 compute 输出给 ExecuteIndirect 用

如果 buffer 前面在 compute 里作为 UAV 写入，后面要作为：

- `INDIRECT_ARGUMENT`

那么：

- `SetResourceState(..., STATE_INDIRECT_ARGUMENT)` 是在做状态转换

而额外的 `UAVBarrier`，表达的是：

- 前面 UAV 写的结果，需要在后面被正确看到

两者语义不重合。

## 为什么 `RT -> RT` 不需要靠 barrier 来“排队”

这个点值得单独强调，因为它最反直觉。

我们容易脑补成：

- A pass 还在 shading
- B pass 也开始 shading
- 那 B 会不会比 A 更早写到 RT

这里要建立的正确心智模型是：

- GPU 内部可以对多个 draw 做流水和重叠
- 但这种内部 overlap 不能破坏 API 规定的执行顺序结果
- 对同一个 graphics queue，后记录的 draw 不会在程序语义上越过先记录的 draw

所以：

- “shader 阶段可能 overlap”
- 和
- “最终 render target 写入顺序语义受 command order 约束”

这两件事并不冲突。

## 这次讨论后建立的速查规则

可以先用下面这套简单规则判断：

- `SRV -> SRV`：通常不用 barrier
- `RT/DSV -> SRV`：要 `Transition`
- `SRV -> RT/DSV`：要 `Transition`
- `RT -> RT`：通常不用额外 barrier，顺序由 queue 保证
- `UAV -> UAV`：通常要 `UAVBarrier`
- `UAV -> SRV / Indirect / CopySrc`：至少要 `Transition`，必要时再补 `UAVBarrier`

## 还要注意的边界

上面的结论成立，有一个隐含前提：

- 大多数讨论默认都在同一个 command queue 上

如果后面变成：

- graphics queue 和 compute queue 交叉访问同一资源

那就不是只靠普通 resource barrier 就能完整解决了，还要考虑：

- queue 间同步
- fence
- ownership / 提交时机

也就是说，这次讨论里的结论主要适用于：

- 当前工程这种单 queue、单 command list 顺序记录的理解框架

## 一个很容易再次混淆的重点

讨论到后面，我们又把一句话压缩成了：

- `SRV -> SRV` 时，drawcall 这条线严格保证前后顺序
- `UAV 写 -> UAV 读/写` 时，需要 `UAVBarrier` 来严格保证前后顺序

这句话方向是对的，但还不够精确，容易留下一个误解：

- 好像只有 `SRV -> SRV` 时，命令顺序才天然有保证
- 而 `UAV` 时，顺序要靠 `UAVBarrier` 才建立起来

更准确的说法应该是：

- 不管前后是 `SRV`、`RT` 还是 `UAV`，同一个 queue 上的命令顺序本来就存在
- `UAVBarrier` 不是用来创造“命令谁先谁后”
- 它是用来补上 `UAV` 写结果对后续访问的可见性和有序性

也就是说，要分清两层：

- 命令顺序：一直都由 queue / command list 保证
- `UAV` 写结果是否已经能被后续访问正确看到：这件事还要靠 `UAVBarrier`

因此更适合记成下面这版：

- `SRV -> SRV`：命令顺序有，而且因为只是读，通常已经够用
- `UAV 写 -> UAV 读/写`：命令顺序也有，但还不够，还要补 `UAVBarrier`

这条修正很重要，因为它能避免一种常见误判：

- 误以为 `UAVBarrier` 的作用是“让 dispatch B 不会跑到 dispatch A 前面”

其实不是。

dispatch 的先后本来就由命令流保证。

`UAVBarrier` 真正补的是：

- dispatch A 对 UAV 的写
- 在 dispatch B 继续访问这个 UAV 时
- 已经按预期完成并可见

也可以把它压成一句更短的心智模型：

- `SRV -> SRV`：顺序有，且够用
- `UAV -> UAV`：顺序也有，但可见性还不够，所以要 `UAVBarrier`

## 一句话总结

这次真正需要拆清楚的，不是某个 API 名字，而是三层语义：

- 命令顺序由 queue / command list 保证
- 资源访问身份变化由 `Transition Barrier` 保证
- UAV 写后结果的有序可见性由 `UAVBarrier` 保证

一旦把这三层分开看，很多问题就不会再混：

- 不是 pass 和 pass 之间都要 barrier
- 而是资源访问方式变了，才需要 transition
- 如果是 UAV 连续访问，还要额外关注可见性
