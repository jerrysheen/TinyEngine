#pragma once
#include "Graphics/IGPUBuffer.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/PersistantBuffer.h"

namespace EngineCore
{

    class GPUSceneManager
    {
    public:
        static GPUSceneManager* GetInstance();
        static void Create();
        void Tick();
        void Destroy();
        BufferAllocation GetSinglePerObjectData();
        void RemoveSinglePerObjectData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerObjectData(const BufferAllocation& bufferalloc, void* data);
        GPUSceneManager();
        PersistantBuffer* allObjectDataBuffer;
    private:
        static GPUSceneManager* sInstance; 
    };

}