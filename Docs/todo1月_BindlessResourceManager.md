【】 考虑 keyword 来实现bindless和传统的模式

// ShaderCommon.hlsl

#ifdef BINDLESS_ENABLED
    // Bindless 模式：资源是全局堆的一个索引
    #define TEXTURE2D(Name) uint Name##Index
    #define SAMPLE_TEXTURE2D(Name, Sampler, UV) ResourceDescriptorHeap[NonUniformResourceIndex(Name##Index)].Sample(Sampler, UV)
#else
    // 传统模式：资源是绑定的 Slot
    #define TEXTURE2D(Name) Texture2D Name
    #define SAMPLE_TEXTURE2D(Name, Sampler, UV) Name.Sample(Sampler, UV)
#endif

// 用户材质代码 UserMaterial.hlsl
cbuffer MaterialData : register(b0)
{
    float4 Color;
    TEXTURE2D(AlbedoMap); // 宏展开：Bindless下是uint，传统下是Texture2D
};

float4 main(VSOutput input) : SV_Target
{
    return SAMPLE_TEXTURE2D(AlbedoMap, LinearSampler, input.uv) * Color;
}



# 1月开发计划：Bindless 资源管理与统一架构

## 核心目标
构建统一的 Bindless 资源管理系统，收编 Texture、Mesh、FrameBuffer 等零散资源，消除手动 Slot 绑定，为 GPU Driven Pipeline 和 Render Graph 打下基础。

## Phase 1: 基础设施 (Global Heaps)
**目标**：建立全局持久化的描述符堆，取代每帧重置的 FrameAllocator（部分）。

- [ ] **Global Descriptor Heap (SRV/UAV/CBV)**
  - 位置：`D3D12DescManager`
  - 特性：ShaderVisible, Capacity = 500,000, Persistent (不每帧重置)。
  - 管理策略：使用 FreeList 或 BitMap 管理 Slot 分配/回收。
  - 接口：`AllocateIndex()`, `FreeIndex()`.

- [ ] **Bindless Global Root Signature**
  - 设计：
    - `RootParameter[0]`: 32-bit Constants (RootConstants) -> 传递 Index/Offset。
    - `RootParameter[1]`: Global Heap (SRV/UAV) -> 绑定整个堆。
    - `RootParameter[2]`: Sampler Heap (Static/Dynamic)。
  - 替换：逐步替换现有的复杂 RootSignature。

## Phase 2: 资源统整 (Resource Unification)

### 2.1 Texture 统整
**目标**：Texture 创建即注册，Shader 只认 Index。

- [ ] 修改 `Texture` 类：
  - 新增成员 `uint32_t bindlessSRVIndex`。
  - 构造时：调用 `D3D12DescManager` 分配 Index，并 CreateSRV 到该位置。
  - 析构时：归还 Index。
- [ ] 修改 `Material` 系统：
  - 不再保存 `Texture*` 到 Slot Map。
  - 直接保存 `uint32_t textureIndex` 到 `MaterialData` (StructuredBuffer)。
- [ ] Shader 改造：
  - 引入 `Texture2D g_Textures[] : register(t0, space1);`
  - 采样：`g_Textures[mat.diffuseTexIdx].Sample(...)`

### 2.2 FrameBuffer 统整
**目标**：RT 也可以作为 Texture 被任意 Shader 读取（实现 RenderGraph 的基础）。

- [ ] 修改 `FrameBufferObject`：
  - RTV/DSV 保持原样（Output Merger 阶段不需要 Bindless）。
  - 为其 Color Attachment 创建 SRV，并注册到 Global Heap，获取 `bindlessIndex`。

### 2.3 Mesh 统整 (Global Geometry Buffer) - *进阶*
**目标**：消除 VertexBuffer 切换开销，实现 Vertex Pulling。

- [ ] **Global Big Buffer**
  - 创建 512MB~1GB 的 `ByteAddressBuffer` (Default Heap)。
  - 实现简单的显存分配器 (Linear or TLSF)。
- [ ] **Mesh Upload**
  - 修改 `Mesh` 类：不再创建独立的 ComPtr<ID3D12Resource>。
  - 改为：将数据 Copy 到 Big Buffer，记录 `offset` 和 `size`。
- [ ] **Vertex Pulling (Shader)**
  - VS 不再声明 InputLayout。
  - VS 根据 `SV_VertexID` 和 `RootConstant.MeshOffset` 手动 Load 属性。

## Phase 3: 渲染流程改造 (Render Loop Refactor)

- [ ] **移除 SetShaderResourceView**
  - 渲染循环中不再调用 `SetShaderResourceView(t0, ...)`。
  - 只设置 RootSignature 和 RootDescriptorTable (一次性绑定 Global Heap)。
- [ ] **GPUScene 联动**
  - 确保 ObjectBuffer/MaterialBuffer 也注册在 Global Heap 中。
  - DrawCall 只需要设置一个 RootConstant (Object Index)。

## 时间表 (预估)
- **Week 1**: Global Descriptor Heap 实现 & Texture Bindless 改造。
- **Week 2**: Material 系统适配 & Shader 采样逻辑修改。
- **Week 3**: FrameBuffer 适配 & 验证全场景 Bindless 渲染。
- **Week 4**: (可选) Global Mesh Buffer 原型 & Vertex Pulling 调研。


