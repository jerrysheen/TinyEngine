#pragma once
#include "IRenderPipeline.h"
#include "Scene/GPUSCene.h"
#include "Renderer/RenderBackend.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUResource.h"
#include "Math/Frustum.h"
#include "Renderer/BatchManager.h"
#include "Scene/SceneManager.h"
#include "Renderer/FrameTicket.h"
#include "Graphics/PerFrameBufferRing.h"


namespace EngineCore
{
    class GPUSceneRenderPipeline : public IRenderPipeline
    {
    public:
        GPUSceneRenderPipeline();
        virtual ~GPUSceneRenderPipeline() override 
        {
            delete cullingParamBuffers;
            delete indirectDrawSourceBuffers;
        };
        virtual void Prepare(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;

        GPUBufferAllocator* GetCurrentCullingParamBuffer(uint32_t frameID);
        GPUBufferAllocator* GetCurrentIndirectDrawArgsBuffer(uint32_t frameID);

        PerFrameBufferRing* cullingParamBuffers;
        PerFrameBufferRing* indirectDrawSourceBuffers;
        PerFrameBufferRing* indirectDrawDestBuffers;
        PerFrameBufferRing* indirectDrawCountBuffers;
        PerFrameBufferRing* batchCounterBuffers;
        //BufferAllocation cullingParamAlloc;
        //GPUBufferAllocator* currCullingParamBuffer;
        //GPUBufferAllocator* cullingParamBuffer[3];

        //BufferAllocation indirectDrawArgsAlloc;
        //GPUBufferAllocator* currIndirectDrawArgsBuffer;
        //GPUBufferAllocator* indirectDrawArgsBuffer[3];
        ResourceHandle<ComputeShader> IndirectDrawCombineShaderHandler;
        ResourceHandle<ComputeShader> HizCSShaderHandler;
        ResourceHandle<ComputeShader> BlitCSShaderHandler;

        RenderTexture* RWHiZTexture[3];
        
    };
}