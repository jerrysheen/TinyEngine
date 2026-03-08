#include "PreCompiledHeader.h"
#include "LagacyRenderPath.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    void LagacyRenderPath::Prepare(RenderContext &context)
    {
        PROFILER_ZONE("LagacyRenderPath::Prepare");

        context.Reset();
        // todo： 这个地方culling逻辑是不是应该放到Update
        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        // temp 方案
        cam->Update();
        context.camera = cam;
        Culling::Run(cam, context);

        Renderer::GetInstance()->Prepare(context);
    }

    void LagacyRenderPath::Execute(RenderContext &context)
    {
        Renderer::GetInstance()->Render(context);

#ifdef EDITOR
        Renderer::GetInstance()->OnDrawGUI();
        EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
        EngineEditor::EditorGUIManager::GetInstance()->Render();
#endif
        Renderer::GetInstance()->EndFrame();
    }


}