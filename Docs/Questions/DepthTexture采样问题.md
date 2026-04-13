# DepthTexture 采样问题

## 背景

在推进 Hi-Z / HZB 方案时，我们遇到了一个很具体但又容易卡住的问题：

- 手上有一张深度图，底层格式是 `D24S8`
- CPU 在创建 RenderTarget / DepthStencil 时当然知道它的格式
- GPU 在绑定 DSV 时也知道它是 depth-stencil
- 但一旦进入 shader，我们就会自然地追问：
  - `D24` 这 24 bit 到底怎么被采样？
  - shader 里为什么可以写 `Texture2D<float>`？
  - `float` 怎么可能“直接读到前 24 位”？
  - 这一步到底是 shader 自己解码，还是硬件偷偷做了什么？

这个问题的难点不在 API 名字，而在心智模型不清晰。我们最初对“资源格式”“视图格式”“shader 看到的类型”这三层概念是混在一起的。

## 最初的疑惑

一开始的直觉大致是这样的：

- `D24S8` 在内存里应该就是 32 bit
- 其中前 24 bit 是 depth，后 8 bit 是 stencil
- 如果 shader 要读 depth，看起来就应该先把这 32 bit 原样拿出来
- 然后手工把前 24 bit 取出来，再除以 `2^24 - 1` 变成 float

这个思路的隐含假设是：

- shader 读纹理时拿到的是“原始 bit 模式”
- `Texture2D<float>` 只是“声明成 float”
- 但 GPU 不可能凭空知道该怎么从 24 bit 变成 float

所以会产生一个非常强的疑问：

- “设定对了 view，真的就能 `Texture2D<float>.Sample()` / `Load()` 直接读出来？”

这个疑问是合理的，因为它抓住了最反直觉的一点：

- 为什么 shader 没有显式写位运算，结果却拿到了正确的 depth float？

## 认知误区

这里最大的误区是把“shader 读取 typed texture”误认为“拿到原始二进制数据再自己解释”。

但普通纹理采样并不是这么工作的。

例如颜色纹理：

- 如果资源被绑定成 `R8G8B8A8_UNORM`
- shader 里写 `Texture2D<float4>`
- 读出来通常是 `[0, 1]` 范围内的 float4

此时 shader 也没有自己把 8 bit 除以 255。
是硬件根据 view 格式做了格式化读取。

`D24S8` 也是同类问题，只是格式更特殊。

## 推演过程

### 第一步：先区分“资源”和“视图”

我们逐渐意识到，不应该只盯着“资源本体格式”，还要区分：

- resource format：底层内存怎么存
- view format：这次绑定时，GPU 怎么解释这块内存

这一步非常关键。

因为 depth-stencil 资源经常不是直接拿一个固定 typed format 贯穿到底，而是：

- 资源本体建成 `typeless`
- 作为深度写入时，用 DSV 的格式解释
- 作为 shader 读取时，用 SRV 的格式解释

### 第二步：明确 D24S8 的典型 D3D12 做法

如果既要作为 DSV 写入，又要作为 SRV 采样，典型做法是：

- Resource: `DXGI_FORMAT_R24G8_TYPELESS`
- DSV: `DXGI_FORMAT_D24_UNORM_S8_UINT`
- SRV: `DXGI_FORMAT_R24_UNORM_X8_TYPELESS`

这时可以得到一个清晰结论：

- 同一块内存，在 DSV 视角下是 depth-stencil
- 在 SRV 视角下，它被解释成“R 通道是 24-bit UNORM，额外 8 bit 忽略”

### 第三步：理解 `Texture2D<float>` 到底读到的是什么

当 SRV 格式是 `R24_UNORM_X8_TYPELESS` 时，shader 中：

```hlsl
Texture2D<float> gDepth;
float d = gDepth.Load(int3(x, y, 0));
```

读到的并不是“原始 24-bit 整数”，而是：

- 硬件先取出 depth 的 24 bit
- 再按 `UNORM` 规则转成 float
- 最后把这个 float 交给 shader

等价理解为：

```hlsl
float d = depth24 / 16777215.0f;
```

但这个除法不是 shader 手写的，而是格式化读取路径帮我们做掉了。

### 第四步：我们终于明白“神奇”发生在哪里

真正“神奇”的不是 `Texture2D<float>` 本身，而是：

- SRV 的 typed format 已经定义了这次读取的解码语义
- shader 只是接收“被 view 格式解释后的结果”

所以并不是：

- “shader 知道前 24 位是什么”

而是：

- “view 告诉硬件：把这块内存按 R24_UNORM 来解释”

shader 最终看到的是解释后的 `float`。

## 最终结论

### 结论 1：`Texture2D<float>` 读到的不是原始 bit

如果使用的是 `R24_UNORM_X8_TYPELESS` 这样的 typed SRV，那么 shader 拿到的是：

- 已经被格式化解释后的 depth float

不是：

- 原始 24 bit 整数值

### 结论 2：depth 的 24 bit 会被硬件自动归一化

对于 `D24` 来说，读取后的结果可以理解为：

```text
floatDepth = depth24 / (2^24 - 1)
```

即 `[0, 16777215]` 被映射到 `[0, 1]`。

### 结论 3：关键不在 shader 类型，而在 view 格式

我们最终建立的心智模型是：

- `resource` 负责存储
- `view` 负责解释
- `shader` 负责消费解释后的值

也就是：

```text
typeless resource
    -> typed DSV / SRV view
    -> shader sees formatted value
```

### 结论 4：如果想拿“原始 bit”，就不是这条路

`Texture2D<float>` 配合 depth SRV 这条路径，本质是 typed texture read。
它适合获取“depth 数值语义”。

如果目标是：

- 自己做 bit-level 处理
- 自己拆字段
- 自己解释原始二进制

那就不是普通 typed SRV + `Texture2D<float>` 这套思路了。

## 对 Hi-Z 实现的启发

这个问题想通之后，Hi-Z 的输入策略也更明确了。

至少有两条可行路线：

### 路线 A：直接从 depth SRV 读取

- 深度资源支持 SRV
- 使用类似 `R24_UNORM_X8_TYPELESS` 的视图读取 depth
- 在 compute / pixel shader 中直接 downsample

优点：

- 少一次转换
- 逻辑直接

代价：

- depth-stencil 资源和 SRV/DSV 的格式管理更严格
- 底层实现细节更容易踩坑

### 路线 B：先转成单独的 float 纹理，再构建 Hi-Z

- 先把 depth 复制/转换到 `R32_FLOAT` 或类似格式
- 后续 Hi-Z 全程只处理普通纹理

优点：

- 心智负担更小
- mip、SRV/UAV、compute 读写更统一

代价：

- 多一个 pass 或转换步骤

在工程落地上，这条路线通常更省心。

## 一句话总结

我们这次真正补上的，不是一个 API 记忆点，而是一个底层认知：

- shader 并不是在直接读取 `D24S8` 的原始 bit
- 而是在读取“view 格式解释后的 depth 值”

因此，`Texture2D<float>` 能读 `D24` 并不神秘。
神秘感来自于我们一开始没有把“资源存储”和“视图解释”拆开理解。
