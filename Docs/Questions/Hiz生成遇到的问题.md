# Hiz 生成遇到的问题

## 背景

在讨论 Hi-Z 金字塔生成时，我们一开始对以下几个点没有完全想清楚：

- 原始 depth 是 Hi-Z 的 `mip0` 还是 `mip1`
- Hi-Z 纹理应该用 `16` 位还是 `32` 位格式
- 每一层 mip 的 dispatch 尺寸到底怎么递推
- 生成下一层时，是不是必须重新绑定上一层
- 如果 Hi-Z 是一张带 mip 的纹理，能不能只绑定一次，然后在 shader 里自由选择读写哪一层
- `RWTexture2D` 到底代表“一整张带 mip 的纹理”，还是“当前 dispatch 的某一层输出”

这些问题表面上分散，实际上都指向同一个根源：

- 我们把“整张带 mip 的 texture”和“某一次 dispatch 正在处理的子资源 view”混在一起理解了

## 最初的直觉

最初的想法比较接近下面这种模型：

- Hi-Z 是一张带 mip 的 texture
- mip 之间逻辑上是连续的
- 既然它们都属于同一张图，那绑定一次之后，shader 理应可以靠 level 参数自由读写不同 mip
- 如果用 UAV，似乎就更直接了
- 看起来像是 dispatch 一次处理 `0 -> 1`，再 dispatch 一次处理 `1 -> 2`，中间只改 sample level 就够了

这个直觉的问题在于：

- 它默认“读哪层”和“写哪层”都可以完全由 shader 自己决定
- 但实际 API 语义里，这两件事并不对称

## 我们真正卡住的地方

卡点不是“怎么 downsample”，而是：

- mip chain 是一张资源，还是多份 view
- shader 能决定哪些事情，CPU 绑定阶段又必须决定哪些事情

进一步说，我们被下面这个问题卡住了：

- `Texture2D` / `RWTexture2D` 到底是“按纹理语义访问”，还是“像 buffer 一样自己算偏移访问”

只要这个问题没想清楚，就会自然产生几个误判：

- 误以为 `RWTexture2D` 可以像一块线性内存一样，靠 offset 写任意 mip
- 误以为绑定一张带 mip 的 UAV 后，shader 就能随便把结果写到某一层
- 误以为只要 sample level 可变，整个 Hi-Z 生成就不需要切 view

## 认知纠偏过程

### 1. 原始 depth 不是 Hi-Z 的 `mip1`

首先明确：

- 原始 depth 用来生成 Hi-Z 的 `mip0`
- 后续 `mip1 / mip2 / mip3 ...` 都是从前一级 Hi-Z 继续 downsample 出来的

也就是：

- `Depth -> HiZ mip0`
- `HiZ mip0 -> mip1`
- `HiZ mip1 -> mip2`

而不是：

- `Depth` 直接等同于 Hi-Z 的某一级 mip

### 2. 每一级都是 2x2 递推，不是直接跳 `/4`

分辨率递推应该是：

- `1920 x 1080 -> mip0`
- `960 x 540 -> mip1`
- `480 x 270 -> mip2`
- `240 x 135 -> mip3`

即每一级通常对前一级做一次 `2x2` 归约。

### 3. 读 mip 和写 mip 不是一回事

这是整个问题里最关键的认知转折。

对于输入：

- 如果绑定的是一张覆盖整条 mip chain 的 `SRV`
- shader 确实可以通过 `Load(..., mip)` 或 `SampleLevel(..., mip)` 指定读哪一层

对于输出：

- `RWTexture2D` 对应的 `UAV` 通常只指向一个特定 mip
- shader 往哪里写，不是单靠传一个 mip level 就能随便切换的
- 目标 mip 通常由当前绑定的 `UAV view` 决定

于是我们得出一个更准确的模型：

- 读端有机会靠 shader 参数选择源 mip
- 写端通常必须靠切换 `UAV view` 指定目标 mip

### 4. `RWTexture2D` 不是线性 buffer 思维

我们一度会自然写出这种脑内模型：

```hlsl
gOutput[x * width + y] = ...
```

但这更像 `RWBuffer` 的寻址方式。

对于 Hi-Z，更符合实际的是：

```hlsl
Texture2D<float> gInput;
RWTexture2D<float> gOutput;
```

然后：

- `gInput.Load(int3(srcX, srcY, srcMip))` 读取前一级
- `gOutput[uint2(dstX, dstY)] = ...` 写当前级

这里的 `gOutput` 表示“当前 dispatch 正在生成的那一层 mip”，而不是“整张 texture 任意位置的线性地址空间”。

## 最终建立的心智模型

Hi-Z 金字塔生成更准确的理解应该是：

- 一张 Hi-Z texture，内部有完整 mip chain
- 每次 dispatch 只处理一对相邻 mip
- 输入是前一级 mip
- 输出是当前级 mip
- 读通常通过 `SRV + mip level`
- 写通常通过“当前目标 mip 的 UAV view”

也就是：

- dispatch A：读 `mip0`，写 `mip1`
- dispatch B：读 `mip1`，写 `mip2`
- dispatch C：读 `mip2`，写 `mip3`

中间需要同步和状态切换，不能把它想成“一次绑定后，shader 在整条 mip 链里随便读写”。

## 对 shader 写法的直接结论

生成某一级 Hi-Z 时，shader 的结构更接近：

```hlsl
Texture2D<float> gInput;
RWTexture2D<float> gOutput;
```

语义上是：

- `gInput`：前一级 mip 的来源
- `gOutput`：当前要生成的目标 mip

然后每个输出像素通常读取输入的 `2x2` texel，做 `min` 或 `max` 归约，再写到 `gOutput[uint2(x, y)]`。

## 一句话总结

这次 Hi-Z 讨论里，我们真正被卡住的不是算法本身，而是“texture 资源”和“当前绑定 view”的边界感。

一旦把这件事拆清楚，整个流程就顺了：

- 整张 Hi-Z 是一张多 mip 纹理
- 但每次 dispatch 只操作其中一对 mip
- 读源 mip 可以更灵活
- 写目标 mip 通常必须明确绑定到对应的 UAV view
