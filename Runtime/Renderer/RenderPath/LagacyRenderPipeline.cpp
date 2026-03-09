#include "PreCompiledHeader.h"
#include "LagacyRenderPipeline.h"
#include "Renderer/RenderEngine.h"

namespace EngineCore
{
    void LagacyRenderPipeline::Prepare(RenderContext &context)
    {
        PROFILER_ZONE("LagacyRenderPipeline::Prepare");

        context.Reset();
        // todo： 这个地方culling逻辑是不是应该放到Update
        Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
        // temp 方案
        cam->Update();
        context.camera = cam;
        Culling::Run(cam, context);

        RenderBackend::GetInstance()->Prepare(context);
    }

    void LagacyRenderPipeline::Record(RenderContext& context)
    {
        RenderBackend::GetInstance()->Render(context);

#ifdef EDITOR
        RenderBackend::GetInstance()->OnDrawGUI();
        EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
        EngineEditor::EditorGUIManager::GetInstance()->Render();
#endif
        RenderBackend::GetInstance()->EndFrame();
    }


}