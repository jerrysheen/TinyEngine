12月开发计划：GPU Scene 演进路线图 (修订版)
核心目标：构建全持久化 GPU Scene。不仅仅是材质，物体数据 (Transform) 也驻留在 GPU 显存 (Default Heap) 中。CPU 仅负责增量更新 (Sparse Update)，为后续 Compute Shader 直接修改物体位置 (如 GPU Skinning/Physics) 铺路。

Phase 1: 基础架构与数据持久化 (Refined)
目标：实现 Object 与 Material 的双重 StructuredBuffer 持久化管理，解耦渲染循环。

1. 显存管理策略 (Buffer Management) - 修正点
不再使用每帧轮替的 Upload Heap，而是建立类似“显存堆分配器”的机制。

GPU Object Buffer (Global):
类型: StructuredBuffer<GPUObjectData> (SRV) / 在 Phase 3 变为 RWStructuredBuffer (UAV)。
位置: Default Heap (显存常驻)。
生命周期: 与场景加载生命周期一致。
索引管理: 使用 FreeList 或 SlotAllocator 在 CPU 端管理空闲的 Index。
GPU Material Buffer (Global):
类型: StructuredBuffer<MaterialData> (初期) -> ByteAddressBuffer (后期)。
位置: Default Heap。
2. 数据同步 (Sync Phase) - 补充操作
流程: Tick -> Sync -> Render。
在 Render 之前，必须完成所有脏数据的 Staging Buffer -> GPU Buffer 拷贝。

Step 1: Dirty Collection (CPU)
遍历 Scene，收集 Transform.isDirty 的物体和 Material.isDirty 的材质。
Object Sync: 将变换矩阵写入 ObjectStagingBuffer。
Material Sync: 将参数写入 MaterialStagingBuffer。
Step 2: Batch Upload (Command List)
使用 CopyBufferRegion 将脏数据块搬运到 GPU Default Heap 的对应 Slot 中。
优化: 如果变动物体很多，合并为一个大的 Copy 指令；如果很少，可以使用多个小的 Copy。
Step 3: Barrier
插入 ResourceBarrier (CopyDest -> PixelShaderResource)，确保 Draw 时数据已写入完毕。
关键代码实现 (Key Implementation)
A. CPU 端：GPU Scene Manager (简化版)
这是管理 Slot 和脏更新的核心逻辑。

cpp
// GPUSceneManager.h

struct GPUObjectData {
    Matrix4x4 WorldMatrix;
    Matrix4x4 WorldInvTrans;
    uint32_t  MaterialID; 
    uint32_t  Padding[3];
};

class GPUSceneManager {
public:
    // 1. 初始化：创建巨大的 Default Heap Buffer (例如容量 10w 个物体)
    void Initialize() {
        m_ObjectBufferGPU = CreateDefaultBuffer(MAX_OBJECTS * sizeof(GPUObjectData));
        m_ObjectStaging   = CreateUploadBuffer(MAX_UPDATE_PER_FRAME * sizeof(GPUObjectData));
        // 初始化 FreeList...
    }

    // 2. 注册物体：分配一个 GPU 端的 Index (Slot)
    uint32_t RegisterObject(const Transform& trans, uint32_t matID) {
        uint32_t slot = m_FreeList.Pop();
        MarkDirty(slot, trans, matID); // 标记为脏，准备第一帧上传
        return slot; // 这个 Slot ID 会存到 RenderItem/GameObject 里
    }

    // 3. 同步阶段：每帧调用，处理脏数据
    void Sync(ID3D12GraphicsCommandList* cmdList) {
        if (m_DirtyObjects.empty()) return;

        // Map Staging Buffer
        GPUObjectData* mappedPtr = nullptr;
        m_ObjectStaging->Map(..., &mappedPtr);

        int updateCount = 0;
        for (auto& item : m_DirtyObjects) {
            // 填充数据到 Staging
            GPUObjectData data = PackData(item.transform, item.materialID);
            mappedPtr[updateCount] = data;
            
            // 记录拷贝目标位置 (Sparse Update 甚至可以记录多个 CopyRegion)
            // 为了简单演示，这里假设如果是大量更新，建议做 defrag 或者分块 copy
            // 实际工程中：通常维护一个连续的 Dirty 区间，或者构建多个 CopyRegion
            cmdList->CopyBufferRegion(
                m_ObjectBufferGPU, item.slot * sizeof(GPUObjectData), 
                m_ObjectStaging, updateCount * sizeof(GPUObjectData), 
                sizeof(GPUObjectData)
            );
            
            updateCount++;
        }
        m_ObjectStaging->Unmap();
        
        // 必须加 Barrier，确保 Copy 完成后 VS/PS 能读到新数据
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_ObjectBufferGPU, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        );
        cmdList->ResourceBarrier(1, &barrier);

        m_DirtyObjects.clear();
    }
};
B. 渲染循环改造 (Render Loop)
cpp
void Renderer::RenderFrame(ID3D12GraphicsCommandList* cmdList) {
    // 1. Bind Global Heaps (MegaBuffers)
    // Root Signature 只有几个 Slot：ObjectBuffer, MaterialBuffer, GlobalConsts
    cmdList->SetGraphicsRootShaderResourceView(0, m_GPUScene->GetObjectBufferGPU()->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootShaderResourceView(1, m_GPUScene->GetMaterialBufferGPU()->GetGPUVirtualAddress());

    // 2. Draw Loop (Bindless)
    for (const auto& batch : renderBatches) {
        // 只需要设置 Mesh (VB/IB) 和 PSO
        cmdList->SetPipelineState(batch.pso);
        cmdList->IASetVertexBuffers(0, 1, &batch.mesh->vbView);
        cmdList->IASetIndexBuffer(&batch.mesh->ibView);

        // 关键：通过 BaseInstance 传递 Object Slot ID
        // 这样 VS 里 SV_InstanceID + BaseInstance 就是全局唯一的 Object Index
        cmdList->DrawIndexedInstanced(
            batch.indexCount, 
            1, 
            batch.startIndex, 
            0, 
            batch.objectGPUSlotIndex // <--- 这里传入之前分配的 Slot
        );
    }
}
C. Shader 端 (HLSL)
hlsl
// ShaderCommon.hlsl

struct ObjectData {
    float4x4 World;
    float4x4 WorldInvTrans;
    uint     MaterialIndex;
    uint3    Padding;
};

struct MaterialData {
    float4 BaseColor;
    float  Roughness;
    float  Metallic;
    // ...
};

// Bindless Resources: Space0 留给 Global Buffers
StructuredBuffer<ObjectData>   g_ObjectBuffer   : register(t0, space0);
StructuredBuffer<MaterialData> g_MaterialBuffer : register(t1, space0);

// Vertex Shader
struct VSInput {
    float3 Pos : POSITION;
    // ...
    uint   InstID : SV_InstanceID; // 系统自动生成的索引
};

struct PSInput {
    float4 Pos : SV_POSITION;
    uint   MatID : TEXCOORD0; // 将 Material ID 传给 PS
};

PSInput VSMain(VSInput input) {
    // 直接通过 InstanceID 索引全局 Buffer
    ObjectData obj = g_ObjectBuffer[input.InstID]; 
    
    PSInput output;
    float4 worldPos = mul(float4(input.Pos, 1.0), obj.World);
    output.Pos = mul(worldPos, g_ViewProj);
    output.MatID = obj.MaterialIndex;
    
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_Target {
    // 根据 VS 传来的 ID 获取材质数据
    MaterialData mat = g_MaterialBuffer[input.MatID];
    
    return mat.BaseColor;
}
Phase 2 & 3: 演进补充 (Look Ahead)
修改后的架构为后续步骤扫清了障碍：

Phase 2 (Indirect Draw):

因为 ObjectBuffer 已经是全局常驻的了，CPU 只需要生成一个 ArgumentBuffer (包含 InstanceCount 和 StartInstanceLocation)。
StartInstanceLocation 直接填入 Object 的 Slot ID，Shader 代码完全不用改。
Phase 3 (GPU Culling):

Compute Shader (CS) 读取 g_ObjectBuffer (包含包围盒)。
CS 做视锥剔除。
Output: CS 将可见物体的 Slot ID 写入一个 CompactIndexBuffer。
Draw: 使用 DrawInstancedIndirect。
Shader 修改: VS 需要多一步查表 uint realSlotID = g_CompactIndexBuffer[input.InstID]; ObjectData obj = g_ObjectBuffer[realSlotID];。
总结与修正建议
核心修正: 确认了 ObjectBuffer 和 MaterialBuffer 都必须是 Persistent (Default Heap)。
数据流: CPU 计算 Dirty -> Staging Buffer -> Copy -> GPU Default Heap -> Resource Barrier -> Draw。
代码关键: 利用 DrawIndexedInstanced 的最后一个参数 StartInstanceLocation 作为连接 CPU 对象逻辑与 GPU 数据的桥梁。