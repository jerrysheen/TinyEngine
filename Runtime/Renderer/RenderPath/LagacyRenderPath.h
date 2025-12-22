#pragma once
#include "IRenderPath.h"
#include "Renderer/Renderer.h"
#include "Renderer/Culling.h"
#include "GameObject/Camera.h"
#include "Scene/Scene.h"
#include "Graphics/GPUSceneManager.h"

namespace EngineCore
{
    class LagacyRenderPath : public IRenderPath
    {
    public:
        virtual ~LagacyRenderPath() override {};
        virtual void Execute(RenderContext& context) override
        {
            Renderer::GetInstance()->BeginFrame();
            
            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            context.Reset();

            // todo： 这个地方culling逻辑是不是应该放到Update
            Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
            context.camera = cam;
            PROFILER_EVENT_BEGIN("MainThread::Culling::Run");
            Culling::Run(cam, context);
            PROFILER_EVENT_END("MainThread::Culling::Run");

            Renderer::GetInstance()->Render(context);

#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
#endif

            Renderer::GetInstance()->EndFrame();
        }
    };
};