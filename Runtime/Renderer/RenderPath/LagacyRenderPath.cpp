#include "PreCompiledHeader.h"
#include "LagacyRenderPath.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    void LagacyRenderPath::Execute(RenderContext &context)
    {
            Renderer::GetInstance()->BeginFrame();
            
            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            RenderEngine::GetInstance()->GetGPUScene().Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            context.Reset();

            // todo： 这个地方culling逻辑是不是应该放到Update
            Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
            // temp 方案
            cam->Update();
            context.camera = cam;
            PROFILER_EVENT_BEGIN("MainThread::Culling::Run");
            Culling::Run(cam, context);
            PROFILER_EVENT_END("MainThread::Culling::Run");

            Renderer::GetInstance()->Render(context);

#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
            EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
            EngineEditor::EditorGUIManager::GetInstance()->Render();
#endif

            Renderer::GetInstance()->EndFrame();
    }
}