# D: RenderDoc回放除错

## 背景

在排查 `RenderDoc` 回放崩溃时，最初现象比较混杂：

- 有时是多开 `TinyEngine.exe` + 多开 `RenderDoc` 时更容易 `device hung`
- 有时单进程回放也会 `device hung`
- Windows 事件里能看到大量 `LiveKernelEvent 141`
- `nvlddmkm` 也会同时报错

这说明问题并不是单一来源，而是：

- 引擎内部确实存在 GPU 资源状态/同步上的风险
- `RenderDoc` 回放会把这些问题放大
- 大纹理和高显存占用会进一步增加触发概率

## 最初误判

一开始有几个方向都看起来可疑：

- 深度图采样路径
- `RenderDoc` 本身 replay 不稳定
- 多进程同时运行导致显存爆满
- `ExecuteIndirect` 展示异常

这些方向都不是完全错，但它们不是同一个层级的问题。

后面逐步确认下来：

- 多实例和大纹理会放大问题，但不是唯一根因
- `RenderDoc` 回放崩，不代表一定是 `RenderDoc` 自己的逻辑 bug
- 真正更危险的是引擎内部某些 GPU 写后读链路缺少显式同步

## 关键现象

### 1. 回放失败时系统侧报的是 `device hung`

`RenderDoc` UI 中主要表现为：

- replay 失败
- `device hung`

Windows 事件查看器里进一步能看到：

- `LiveKernelEvent 141`
- `nvlddmkm 153`

这说明 replay 过程中 GPU 命令已经把驱动拖进了 watchdog / TDR 路径。

### 2. 单进程也能触发

这一步很关键。

如果只有：

- 多开程序
- 多开 `RenderDoc`
- 专用显存接近打满

才会挂，那可以把主要原因归到显存压力。

但后面已经确认：

- 单个 `TinyEngine.exe`
- 单个 `RenderDoc`

也能复现 `device hung`

所以最终判断是：

- 显存压力是放大器
- 根因仍然在引擎自己的 GPU 命令链

## 真正的问题在哪

## 1. 资源状态和同步问题

最终排查下来，最值得记录的结论是：

- 根因还是在于资源状态问题
- 尤其是 `UAV` 写后读 / 写后写这种链路
- 不能只靠“看起来状态没变”就默认顺序正确

在 `D3D12` 里：

- `UAV -> UAV`
- `UAV -> IndirectArgument`
- `UAV -> SRV`

这类链如果前后存在真实数据依赖，通常需要显式 `UAV barrier` 去保证前一个 dispatch 的写入对后一个阶段可见。

否则就可能出现：

- 平时偶发不稳定
- `RenderDoc` 回放时更容易炸
- 最后发展成 `device hung`

### 这次补的重点

在 [GPUSceneRenderPipeline.cpp](/D:/GitHubST/TinyEngine/Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp) 中，重点补了两段同步：

第一段：`culling dispatch` 之后

- `visibilityBuffer`
- `currIndirectDrawArgsBuffer->GetGPUBuffer()`

第二段：`combine dispatch` 之后

- `currIndirectDrawDestBuffer`
- `currIndirectDrawCountBuffer`

这两段的本质都是：

- 前一个 compute pass 刚写完
- 后一个 pass 或 `ExecuteIndirect` 马上就要继续消费
- 必须显式告诉 GPU：这里存在顺序依赖

## 2. depth 采样路径也确实有问题，但它不是这次唯一根因

在尝试生成 Hi-Z 时，还额外暴露出一类问题：

- 深度资源不能简单按普通 `STATE_SHADER_RESOURCE` 处理
- depth texture 作为 shader 输入时，需要更准确的状态语义

所以后面补了专门的深度读取状态：

- `STATE_DEPTH_READ_SHADER_RESOURCE`

并映射到：

- `D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE`

这一步的意义是：

- 不要把 depth resource 当普通 color SRV 去切状态
- 让 replay 和驱动更容易正确理解当前资源用途

但这部分更多是：

- 修正 depth 采样路径
- 降低 RenderDoc replay 风险

不是本次 `device hung` 的唯一主因。

## 3. 大纹理会把问题明显放大

虽然最终确认根因不只在显存，但大纹理确实是触发器之一。

尤其是：

- `4K` DDS
- 多个 `TinyEngine.exe`
- 多个 `RenderDoc`

同时运行时：

- 专用显存很容易顶到极限
- replay 额外复制/保留资源
- 本来只是“边缘不稳定”的同步问题，会更快演变成 `device hung`

因此后面在 [DDSTextureLoader.h](/D:/GitHubST/TinyEngine/Runtime/Serialization/DDSTextureLoader.h) 中补了一个保守限制：

- 默认跳过最高一级 mip

也就是：

- 实际上传到 GPU 的 DDS 资源，直接从次一级 mip 开始
- 不是只改采样 LOD
- 而是真正减小了资源尺寸和显存占用

这一条的定位应该是：

- 风险缓解
- 压力控制
- 提高 RenderDoc 可回放性

而不是从根本上替代资源同步修复。

## 关于 `ExecuteIndirect` 的观察

排查过程中还观察到一个容易误判的现象：

- 普通 drawcall 在 RenderDoc 中逐个看 event，会呈现“逐步累加”的画面
- `ExecuteIndirect` 展开子项后，看不同对象时会出现跳变
- 上一个子 draw 的结果在下一个子 draw 里看不到

这个现象不一定说明引擎真有 bug。

当前实现中，每条 indirect 命令都会自己写入：

- `RootSigSlot::DrawIndiceConstant`

对应的实际数据来自：

- `IndirectDrawDest.StartIndexInVisibilityBuffer`

这说明当前设计是：

- 每条 indirect 命令自己携带对象起始偏移
- 不是依赖“前一个 draw 的外部状态残留”

因此 RenderDoc 里“切换子 draw 有跳变”更像是：

- `ExecuteIndirect` 子命令的 replay 展示方式

而不是直接说明渲染逻辑错了。

## 本次结论

本次问题应该拆成两层理解：

### 根因

- GPU 资源状态与同步不完整
- 尤其是 `UAV` 相关链路缺少显式 barrier
- depth 资源读取状态也需要更精确的表达

### 放大器

- 大尺寸 DDS / 4K 贴图
- 多进程同时运行
- 多个 `RenderDoc` capture / replay
- 高显存占用

## 最终结论

可以把这次问题总结成一句话：

- `RenderDoc` 回放崩溃并不是单独的工具问题，根因还是引擎内部资源同步不完整，尤其是 `UAV` 链路必须显式 `barrier`；同时还要限制资源体量，避免大纹理把 GPU 压力进一步推高，导致 replay 更容易直接炸掉。

## 已落地的修复方向

- 给 GPUScene 中有真实数据依赖的 compute 链补 `UAVBarrier`
- 补充 depth shader read 专用状态
- 在 `DDSTextureLoader` 中限制 DDS 最高 mip 的加载
- 在 `D3D12` 错误路径中补充 DRED / DeviceRemoved 信息落盘能力

## 这一轮继续排查的新结论

这一轮继续往下查之后，结论比前一版更具体了一些。

### 1. 有一个真实的 `RenderDoc` / 引擎交互坑：compute root signature 的 descriptor range 生命周期

在 [D3D12RootSignature.cpp](/D:/GitHubST/TinyEngine/Runtime/Platforms/D3D12/D3D12RootSignature.cpp) 里，compute root signature 原先有一段写法是：

- `CD3DX12_DESCRIPTOR_RANGE srvRange;`
- `CD3DX12_DESCRIPTOR_RANGE uavRange;`
- 然后把这两个局部变量的地址传给 `InitAsDescriptorTable(...)`

这在语义上是错的，因为：

- `InitAsDescriptorTable(...)` 只保存指针
- 真正 `D3D12SerializeRootSignature(...)` 发生在后面
- 局部变量早就析构了

这属于标准的生命周期错误。

这一点后来已经修掉，但实际验证下来：

- 它确实是 bug
- 但它不是这次 replay 崩溃的唯一根因

也就是说，这一项必须修，但修完并不能单独解决所有问题。

### 2. 更大的坑在 dynamic descriptor heap 的回收时机

后面真正更关键的发现是：

- compute dispatch 的临时纹理 `SRV/UAV` descriptor table
- 实际上是在全局 shader-visible bindless heap 的动态区里临时分配出来的

原先的做法是：

- 每帧 `Present` 之后立即 `ResetFrameAllocator()`

这在运行时很多时候能“侥幸跑过去”，但 replay 更容易出问题，因为：

- `Present` 返回不代表 GPU 已经完全用完这一帧绑定过的动态 descriptor
- 下一帧马上覆盖同一段 descriptor，可能破坏前一帧 replay / shutdown 路径中的对象引用

这类问题特别符合：

- 程序正常跑时不一定炸
- `GPU Validation` 不一定直接报
- `RenderDoc replay` 更容易挂

### 3. dynamic descriptor 最终改成了按帧分段

最后没有把 bindless heap 粗暴拆成 3 个独立 heap，因为：

- `D3D12` 同时只能绑定一个 `CBV_SRV_UAV` heap
- graphics 和 compute 目前都依赖同一个 bindless 绑定模型

最终采用的是：

- 保留一个全局 shader-visible bindless heap
- 从动态区起点开始，按 `MAX_FRAME_INFLIGHT = 3` 分成 3 段
- 每帧只在 `frameID % 3` 对应的那一段里分配动态 descriptor
- 当前帧开始录制时，只 reset 当前帧自己的那一段

这一点很重要，因为它和现有的：

- `FrameTicket`
- `WaitForFrameAvaliable(frameID)`
- `frameID % 3`

完全一致，逻辑上也更容易推导正确性。

### 4. reset 更适合放在渲染线程 `SetFrame` 之后，而不是主线程 `PrepareFrame`

虽然从“帧资源回收”的语义上看，很多 reset 都发生在：

- `RenderEngine::PrepareFrame()`

但 dynamic descriptor allocator 这一类资源有一个额外要求：

- 谁 `allocate`
- 最好也由谁 `reset`

因为真正分配 descriptor 的位置是在：

- `D3D12RenderAPI::RenderAPIDispatchComputeShader(...)`
- `D3D12RenderAPI::RenderAPISetMaterial(...)`

这些都属于渲染线程路径。

所以这块如果放到主线程 `PrepareFrame()` 里 reset，反而会变成：

- 主线程 reset
- 渲染线程 allocate

线程 ownership 更乱。

因此最终更合理的做法是：

- 主线程通过 `WaitForFrameAvaliable(frameID)` 只负责确认这个 frame slot 可以复用
- 再把 `SetFrame(frameID)` 命令发给渲染线程
- 渲染线程在 `RenderAPISetFrame(...)` 里 reset 当前帧对应的 descriptor 分段

### 5. 官方 RenderDoc 打不开 capture，但本地 build 的 RenderDoc 更稳定

这一轮最有意思的现象是：

- 官方版本的 `RenderDoc` 打开 `.rdc` 时仍然容易失败
- 但本地自己编译的 `RenderDoc`，回放反而明显更稳定，甚至可以继续分析

而且官方版这次报错已经从单纯的“回放崩”变成了更明确的：

- `Failed creating graphics pipeline, HRESULT: E_INVALIDARG`

这说明：

- 问题不一定只是引擎自己的非法用法
- 也不一定只是 `RenderDoc` 工具自身随机崩
- 更像是两边叠加：capture / replay 边界本来就比较脆，官方版与本地 build 行为又存在差异

因此这次后面的策略也跟着调整了：

- 不再把“官方版 replay 崩”直接当作 TinyEngine 的唯一证据
- 先用本地 build 的 `RenderDoc` 继续排查
- 同时把“官方版崩 / 本地版更稳”视为一个独立线索

### 6. shutdown 栈里看到的崩溃，更像是第二现场

在本地调试 RenderDoc 时，shutdown 栈大致落在：

- `ResourceManager<D3D12ResourceManagerConfiguration>::RemoveWrapper(...)`
- `WrappedID3D12Resource::~WrappedID3D12Resource()`
- `ReplayController::Shutdown()`

从这个栈判断，更合理的理解是：

- 前面 replay / PSO 创建阶段已经出了问题
- shutdown 时 RenderDoc 又在清理 wrapper map 的过程中暴露了第二个 bug

也就是说：

- shutdown 崩不是“第一个问题”
- 更像是前面 replay 状态损坏后的连带后果

这类现象在图形调试工具里比较常见，所以这次也不应直接把锅全扣在程序退出流程上。

## 后续建议

- 不要把 `UAV` 资源依赖仅仅理解成“状态对了就够”
- 所有 `compute -> compute`、`compute -> indirect`、`compute -> srv` 的真实读写链，都要重新审视是否需要 `UAV barrier`
- 对大纹理资源做更系统的加载策略，而不是只靠临时硬编码
- 针对 `RenderDoc` replay 单独准备一套更保守的调试配置，降低显存压力和驱动风险
- 所有动态 descriptor 的生命周期都要和 `FrameTicket` / inflight frame 体系绑定，不能在 `Present` 后立即全局 reset
- 如果后面还遇到“官方版 RenderDoc 崩，本地 build 更稳”的情况，优先保留本地 build 作为分析工具，不要因为工具自身崩溃就完全阻塞引擎排查
