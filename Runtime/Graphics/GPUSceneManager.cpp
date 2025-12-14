#include "PreCompiledHeader.h"
#include "GPUSceneManager.h"
#include "Graphics/PersistantBuffer.h"
#include "Graphics/LinearAllocateBuffer.h"
#include "Graphics/IGPUBuffer.h"
#include "Renderer/RenderUniforms.h"
#include "Renderer/RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"

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
    }

    void GPUSceneManager::Tick()
    {
        // Sync PerObjectData:
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        int currMaxIndex = mCurrentScene->m_CurrentSceneMaxRenderNode;
        for(int i = 0; i < currMaxIndex; i++)
        {
            if(renderSceneData.needUpdateList[i] == true)
            {
                // todo: 起一个ComputeShader做一次全部更新
                renderSceneData.meshRendererList[i]->SyncPerObjectDataIfDirty();
                renderSceneData.needUpdateList[i] = false;
            }
        }
        perFrameBatchBuffer->Reset();
    }

    BufferAllocation GPUSceneManager::GetSinglePerObjectData()
    {
        return allObjectDataBuffer->Allocate(sizeof(PerObjectData));
    }

    void GPUSceneManager::RemoveSinglePerObjectData(const BufferAllocation &bufferalloc)
    {
        allObjectDataBuffer->Free(bufferalloc);
    }

    void GPUSceneManager::UpdateSinglePerObjectData(const BufferAllocation &bufferalloc, void *data)
    {
        allObjectDataBuffer->UploadBuffer(bufferalloc, data, sizeof(PerObjectData));
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
        desc.size = sizeof(uint32_t) * 10000;
        desc.stride = sizeof(uint32_t);
        desc.usage = BufferUsage::StructuredBuffer;
        perFrameBatchBuffer = new LinearAllocateBuffer(desc);

        perObjectCPUBuffer.reserve(10000);
        std::queue<int> empty;
        std::swap(m_FreePerObjectIndex, empty);
    }


}
