#include "PreCompiledHeader.h"
#include "GPUSceneManager.h"
#include "Graphics/PersistantBuffer.h"
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
        sInstance = new GPUSceneManager();
    }

    void GPUSceneManager::Destroy()
    {
        delete allObjectDataBuffer;
    }

    void GPUSceneManager::Tick()
    {
        // Sync PerObjectData:
        auto allGO = SceneManager::GetInstance()->GetCurrentScene()->allObjList;
        for (GameObject* go : allGO) 
        {
            MeshRenderer* renderer = go->GetComponent<MeshRenderer>();
            if (renderer) 
            {
                renderer->SyncPerObjectDataIfDirty();
            }
        }
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

    GPUSceneManager::GPUSceneManager()
    {
        BufferDesc desc;
        desc.debugName = L"AllObjectBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(PerObjectData) * 10000;
        desc.stride = sizeof(PerObjectData);
        desc.usage = BufferUsage::StructuredBuffer;
        allObjectDataBuffer = new PersistantBuffer(desc);
    }
}
