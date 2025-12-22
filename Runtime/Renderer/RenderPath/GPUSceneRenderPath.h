#pragma once
#include "IRenderPath.h"
#include "Graphics/GPUSceneManager.h"
#include "Renderer/Renderer.h"

namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override {};
        virtual void Execute(RenderContext& context) override
        {
            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");
        
            DrawCommand cmd;
            PayLoad_DispatchCompute p;
            Renderer::GetInstance()->DispatchComputeShader();
        }
    };
}