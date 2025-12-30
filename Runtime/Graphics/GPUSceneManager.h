#pragma once
#include "Graphics/IGPUBuffer.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/GPUBufferAllocator.h"
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

        void TryFreeRenderProxyBlock(const PerObjectData &perObjectData);
        void TryCreateRenderProxyBlock(const PerObjectData &perObjectData);
        BufferAllocation SyncDataToPerFrameBatchBuffer(void *data, uint32_t size);

        //PerObjectCPUHandler ResgisterNewObject();
        //void DeleteSceneObject(PerObjectCPUHandler& handler);

        std::vector<PerObjectData> perObjectCPUBuffer;
        LinearAllocator* perFramelinearMemoryAllocator;

        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* allMaterialDataBuffer;
        GPUBufferAllocator* perFrameBatchBuffer;
        //GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* allInstanceDataBuffer;
        GPUBufferAllocator* renderProxyBuffer;
        vector<PerObjectData> cpuPerObjectDataList;
        vector<uint32_t> dirtyPerObjectDataIndexList;

        BufferAllocation visiblityAlloc;
        GPUBufferAllocator* visibilityBuffer;
        
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
    private:
        std::queue<int> m_FreePerObjectIndex;
        //int m_CurrentPerObjectIndex;
        static GPUSceneManager* sInstance; 
    };

}