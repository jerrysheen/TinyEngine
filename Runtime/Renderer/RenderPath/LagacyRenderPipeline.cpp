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

        RenderBackend::GetInstance()->BeginFrame();
        RenderBackend::GetInstance()->FlushPerFrameData();
        uint32_t frameIndex = RenderEngine::GetInstance()->GetCurrentFrame();
        FrameTicket* currentFrameTicket= RenderEngine::GetInstance()->GetCurrentFrameTicket(frameIndex);
        RenderBackend::GetInstance()->SetFrame(currentFrameTicket, frameIndex);

        for(auto* pass : context.camera->mRenderPassAsset.renderPasses)
        {
            RenderBackend::GetInstance()->FlushPerPassData(context);
            pass->Configure(context);
            pass->Filter(context);
            pass->Prepare(context);
        }
    }

    void LagacyRenderPipeline::RecordAndFlush(RenderContext& context)
    {
        //PROFILER_ZONE("MainThread::Renderer::Render")
        for(auto* pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Execute(context);
            RenderBackend::GetInstance()->TryWakeUpRenderThread();
        }
        
        for (auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Clear();
        }

#ifdef EDITOR
        RenderBackend::GetInstance()->OnDrawGUI();
        EngineEditor::EditorGUIManager::GetInstance()->BeginFrame();
        EngineEditor::EditorGUIManager::GetInstance()->Render();
#endif
        RenderBackend::GetInstance()->EndFrame();
    }

    
}