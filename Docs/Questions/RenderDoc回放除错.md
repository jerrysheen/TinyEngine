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

## 后续建议

- 不要把 `UAV` 资源依赖仅仅理解成“状态对了就够”
- 所有 `compute -> compute`、`compute -> indirect`、`compute -> srv` 的真实读写链，都要重新审视是否需要 `UAV barrier`
- 对大纹理资源做更系统的加载策略，而不是只靠临时硬编码
- 针对 `RenderDoc` replay 单独准备一套更保守的调试配置，降低显存压力和驱动风险
