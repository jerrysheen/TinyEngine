# HIZ实现

## 当前阻塞

1. RenderTexture 不支持多 mip
- `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp` 的 `CreateRenderTexture` 当前把 `resourceDesc.MipLevels` 和 SRV `Texture2D.MipLevels` 都写死为 `1`。
- `TextureDesc` 虽然已经有 `mipCount` 字段，但 render texture 路径没有真正创建多 mip 资源。

2. Compute Shader 目前不能绑定 Texture SRV/UAV
- `Runtime/Graphics/ComputeShader.h` 与 `Runtime/Graphics/ComputeShader.cpp` 当前只有 `SetBuffer` / `GetBufferResource`，没有 `SetTexture`。
- `Runtime/Renderer/RenderCommand.h` 的 `Payload_DispatchComputeShader` 只有 `IGPUBuffer**` 形式的 `cbv/srv/uav` 三组绑定。
- `Runtime/Platforms/D3D12/D3D12RootSignature.cpp` 与 `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp` 的 compute 路径当前把所有 SRV/UAV 都按 root buffer view 处理，不能绑定 `Texture2D` / `RWTexture2D`。

## 影响

- Hi-Z 无法创建 mip chain，也就无法逐级 downsample。
- 后续所有依赖 compute 读写纹理的功能都会碰到同一套基础设施缺口。

## 实现拆分

### 第 1 步
- 补齐 `CreateRenderTexture` 的多 mip 资源创建。
- 目标是至少支持 `TextureDesc.mipCount > 1` 的 render texture 创建。
- 需要同时考虑整资源 SRV 与逐 mip 视图的描述符布局。

### 第 2 步
- 扩展 `ComputeShader` 资源接口，补 `SetTexture`。
- 不再把 texture 继续塞进 buffer 绑定接口。

### 第 3 步
- 扩展 `Payload_DispatchComputeShader` 与 `RenderBackend::DispatchComputeShader`。
- 让 compute command 能携带 `IGPUTexture*` 的 SRV/UAV 绑定信息。

### 第 4 步
- 重做 compute root signature / dispatch 绑定策略。
- buffer SRV/UAV 可继续走 root descriptor。
- texture SRV/UAV 改成 descriptor table 绑定。

### 第 5 步
- 在底层能力补齐后，再开始写 HIZ pass。
- 这一阶段再处理 depth 输入、逐 mip dispatch、资源状态切换。
