#pragma once
#include "Graphics/IGPUResource.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Core/PublicStruct.h"
#include "Core/Allocator/LinearAllocator.h"
#include "Graphics/ComputeShader.h"
#include "Resources/ResourceHandle.h"
#include "Renderer/FrameContext.h"


namespace EngineCore
{

    class GPUScene
    {
    public:
        GPUScene();
        void Create();
        void Tick();
        void Update(uint32_t currentFrameIndex);
        void Destroy();
        void EndFrame();
        
        BufferAllocation GetSinglePerMaterialData();
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdateDirtyNode(CPUSceneView& view);
        void UploadCopyOp();

        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);


        void UpdateFrameContextDirtyFlags(uint32_t renderID, uint32_t flag);
        void UpdateCurrentFrameContextShadowData(uint32_t renderID, CPUSceneView& view);
        
        FrameContext* GetCurrentFrameContexts();

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }
    private:
        void EnsureCapacity(uint32_t renderID);
    private:
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
        static const int mMaxFrameCount = 3;
        uint32_t mCurrentFrameID = 0;
        FrameContext* mCurrentFrameContext;
        FrameContext mCPUFrameContext[mMaxFrameCount];
        GPUBufferAllocator* allMaterialDataBuffer;
    };

}
