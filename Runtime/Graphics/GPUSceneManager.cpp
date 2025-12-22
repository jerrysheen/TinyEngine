#include "PreCompiledHeader.h"
#include "GPUSceneManager.h"
#include "Graphics/PersistantBuffer.h"
#include "Graphics/LinearAllocateBuffer.h"
#include "Graphics/IGPUBuffer.h"
#include "Renderer/RenderUniforms.h"
#include "Renderer/RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer.h"
#include "Graphics/ComputeShader.h"

namespace EngineCore
{
    GPUSceneManager* GPUSceneManager::sInstance = nullptr;
    
    GPUSceneManager* GPUSceneManager::GetInstance()
    {
        if(sInstance != nullptr) return sInstance;
        GPUSceneManager::Create();
        return sInstance;   
    }

    void GPUSceneManager::Create()
    {
        if(sInstance != nullptr) return;
        sInstance = new GPUSceneManager();
    }

    void GPUSceneManager::Destroy()
    {
        delete allObjectDataBuffer;
        delete allMaterialDataBuffer;
        delete perFrameBatchBuffer;
        delete allAABBBuffer;
        delete allInstanceDataBuffer;
        delete perFramelinearMemoryAllocator;
    }

    void GPUSceneManager::Tick()
    {
        perFrameBatchBuffer->Reset();
        perFramelinearMemoryAllocator->Reset();
        // 消化sceneRenderData中的dirty
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        vector<CopyOp> copyList;
        vector<AABB> aabbList;
        vector<PerObjectData> perObjectList;
        int count = renderSceneData.transformDirtyList.size();
        if (count == 0) return;
        CopyOp* data = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        CopyOp* currPtr = data;

        uint32_t AABBbufferSize = count * sizeof(AABB);
        auto& AABBAllocation = perFrameBatchBuffer->Allocate(AABBbufferSize);
        for (int i = 0; i < renderSceneData.transformDirtyList.size(); i++) 
        {   
            uint32_t nodeIndex = renderSceneData.transformDirtyList[i];
            currPtr->srcOffset = AABBAllocation.offset + i * sizeof(AABB);
            currPtr->dstOffset = nodeIndex * sizeof(AABB);
            currPtr->size = sizeof(AABB);
            currPtr++;
            aabbList.push_back(renderSceneData.aabbList[nodeIndex]);
        }
        perFrameBatchBuffer->UploadBuffer(AABBAllocation, aabbList.data(), AABBbufferSize);

        Payload_CopyBufferRegion copyRegionCmd;
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allAABBBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = data;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);


        uint32_t PerObjectbufferSize = count * sizeof(PerObjectData);
        auto& PerObjectAllocation = perFrameBatchBuffer->Allocate(PerObjectbufferSize);
        data = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        currPtr = data;
        for (int i = 0; i < renderSceneData.transformDirtyList.size(); i++)
        {
            uint32_t nodeIndex = renderSceneData.transformDirtyList[i];
            currPtr->srcOffset = PerObjectAllocation.offset + i * sizeof(PerObjectData);
            currPtr->dstOffset = nodeIndex * sizeof(PerObjectData);
            currPtr->size = sizeof(PerObjectData);
            currPtr++;
            perObjectList.emplace_back(renderSceneData.objectToWorldMatrixList[nodeIndex]
                , renderSceneData.meshRendererList[nodeIndex]->GetMaterial()->materialAllocation.offset);
        }
        perFrameBatchBuffer->UploadBuffer(PerObjectAllocation, perObjectList.data(), PerObjectbufferSize);

        copyRegionCmd;
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allObjectDataBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = data;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);

    }

    BufferAllocation GPUSceneManager::GetSinglePerMaterialData()
    {
        return allMaterialDataBuffer->Allocate(512);
    }

    void GPUSceneManager::RemoveSinglePerMaterialData(const BufferAllocation &bufferalloc)
    {
        allMaterialDataBuffer->Free(bufferalloc);
    }

    void GPUSceneManager::UpdateSinglePerMaterialData(const BufferAllocation &bufferalloc, void *data)
    {
        allMaterialDataBuffer->UploadBuffer(bufferalloc, data, 512);
    }

    BufferAllocation GPUSceneManager::SyncDataToPerFrameBatchBuffer(void *data, uint32_t size)
    {
        auto& allocation = perFrameBatchBuffer->Allocate(size);
        perFrameBatchBuffer->UploadBuffer(allocation, data, allocation.size);
        return allocation;
    }

    PerObjectCPUHandler GPUSceneManager::ResgisterNewObject()
    {
        PerObjectCPUHandler handle;
        if(!m_FreePerObjectIndex.empty())
        {
            handle.perObejectIndex = m_FreePerObjectIndex.front();
            m_FreePerObjectIndex.pop();
            return handle;
        }
        handle.perObejectIndex = m_CurrentPerObjectIndex;
        m_CurrentPerObjectIndex++;        
        return handle;
    }

    void GPUSceneManager::DeleteSceneObject(PerObjectCPUHandler &handler)
    {
        ASSERT(handler.isValid());
        PerObjectCPUData& data = perObjectCPUBuffer[handler.perObejectIndex];
        data.active = false;
        m_FreePerObjectIndex.push(handler.perObejectIndex);
        handler.perObejectIndex = UINT32_MAX;
    }

    GPUSceneManager::GPUSceneManager()
    {
        BufferDesc desc;
        desc.debugName = L"AllObjectBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(PerObjectData) * 10000;
        desc.stride = sizeof(PerObjectData);
        desc.usage = BufferUsage::StructuredBuffer;
        allObjectDataBuffer = new PersistantBuffer(desc);

        // 强烈建议使用 512， 注意，这边都是Byte
        // 理由：
        // 1. 容纳 32 个 float4 向量，足够应对 Uber Shader + 矩阵 + 几十个 TextureID。
        // 2. 即使浪费了一半空间，1万个材质也只多占 2.5MB 显存，完全可忽略。
        // 3. 512 是 256 (D3D12 ConstantBuffer 对齐) 的倍数，也是 16 (float4) 的倍数，对齐非常友好。

        desc.debugName = L"AllMaterialBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 512 * 10000; // 预分配约 5MB
        desc.stride = 512;       // PageSize
        desc.usage = BufferUsage::ByteAddressBuffer;
        allMaterialDataBuffer = new PersistantBuffer(desc);

        desc.debugName = L"PerFrameBatchBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = 1024 * 1024 * 4;
        desc.stride = 1;
        desc.usage = BufferUsage::ByteAddressBuffer;
        perFrameBatchBuffer = new LinearAllocateBuffer(desc);

        perObjectCPUBuffer.reserve(10000);
        std::queue<int> empty;
        std::swap(m_FreePerObjectIndex, empty);

        desc.debugName = L"AABBBufferDataBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(AABB) * 10000;
        desc.stride = sizeof(AABB);
        desc.usage = BufferUsage::StructuredBuffer;
        allAABBBuffer = new PersistantBuffer(desc);

        desc.debugName = L"InstanceDataBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = 4 * 10000; 
        desc.stride = 4;       
        desc.usage = BufferUsage::ByteAddressBuffer;
        allInstanceDataBuffer = new PersistantBuffer(desc);

        perFramelinearMemoryAllocator = new LinearAllocator(4 * 1024 * 1024);
    
        // 创建Compute Shader
        ResourceHandle<ComputeShader> shader = ResourceManager::GetInstance()->CreateResource<ComputeShader>("Assets/Shader/GPUCulling.hlsl");
    
    }


}
