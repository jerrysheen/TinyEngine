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
        virtual ~GPUSceneRenderPipeline() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Prepare(RenderContext& context) override {};
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}