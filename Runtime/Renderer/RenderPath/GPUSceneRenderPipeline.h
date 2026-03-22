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

namespace EngineCore
{
    class GPUSceneRenderPipeline : public IRenderPipeline
    {
    public:
        GPUSceneRenderPipeline();
        virtual ~GPUSceneRenderPipeline() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };
        virtual void Prepare(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;

        GPUBufferAllocator* GetCurrentCullingParamBuffer(uint32_t frameID);
        GPUBufferAllocator* GetCurrentIndirectDrawArgsBuffer(uint32_t frameID);

        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* currCullingParamBuffer;
        GPUBufferAllocator* cullingParamBuffer[3];

        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* currIndirectDrawArgsBuffer;
        GPUBufferAllocator* indirectDrawArgsBuffer[3];

    };
}