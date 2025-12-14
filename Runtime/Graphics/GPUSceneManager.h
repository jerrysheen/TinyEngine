#pragma once
#include "Graphics/IGPUBuffer.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/PersistantBuffer.h"
#include "Graphics/LinearAllocateBuffer.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{

    class GPUSceneManager
    {
    public:
        static GPUSceneManager* GetInstance();
        GPUSceneManager();
        static void Create();
        void Tick();
        void Destroy();
        BufferAllocation GetSinglePerObjectData();
        void RemoveSinglePerObjectData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerObjectData(const BufferAllocation& bufferalloc, void* data);
        
        BufferAllocation GetSinglePerMaterialData();
        void RemoveSinglePerMaterialData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerMaterialData(const BufferAllocation& bufferalloc, void* data);
        
        BufferAllocation SyncDataToPerFrameBatchBuffer(void *data, uint32_t size);

        PersistantBuffer* allObjectDataBuffer;
        PersistantBuffer* allMaterialDataBuffer;
        LinearAllocateBuffer* perFrameBatchBuffer;
        std::vector<PerObjectCPUData> perObjectCPUBuffer;
        
        PerObjectCPUHandler ResgisterNewObject();
        void DeleteSceneObject(PerObjectCPUHandler& handler);
        
    private:
        std::queue<int> m_FreePerObjectIndex;
        int m_CurrentPerObjectIndex;
        static GPUSceneManager* sInstance; 
    };

}