#pragma once
#include "Graphics/IGPUBuffer.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/PersistantBuffer.h"
#include "Graphics/LinearAllocateBuffer.h"
#include "Core/PublicStruct.h"
#include "Core/Allocator/LinearAllocator.h"
#include "Graphics/ComputeShader.h"
#include "Resources/ResourceHandle.h"

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
        
        BufferAllocation GetSinglePerMaterialData();
        void RemoveSinglePerMaterialData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerMaterialData(const BufferAllocation& bufferalloc, void* data);
        
        BufferAllocation SyncDataToPerFrameBatchBuffer(void *data, uint32_t size);

        PerObjectCPUHandler ResgisterNewObject();
        void DeleteSceneObject(PerObjectCPUHandler& handler);

        std::vector<PerObjectCPUData> perObjectCPUBuffer;

        PersistantBuffer* allObjectDataBuffer;
        PersistantBuffer* allMaterialDataBuffer;
        LinearAllocateBuffer* perFrameBatchBuffer;
        PersistantBuffer* allAABBBuffer;
        PersistantBuffer* allInstanceDataBuffer;

        LinearAllocator* perFramelinearMemoryAllocator;

        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
    private:
        std::queue<int> m_FreePerObjectIndex;
        int m_CurrentPerObjectIndex;
        static GPUSceneManager* sInstance; 
    };

}