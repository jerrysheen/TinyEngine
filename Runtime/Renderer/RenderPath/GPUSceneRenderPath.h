#pragma once
#include "IRenderPath.h"
#include "Scene/GPUSCene.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUResource.h"
#include "Math/Frustum.h"
#include "Renderer/BatchManager.h"
#include "Scene/SceneManager.h"
#include "Renderer/FrameContext.h"

namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Execute(RenderContext& context) override;


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}