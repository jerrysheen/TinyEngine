#include "PreCompiledHeader.h"
#include "Renderer.h"
#include "RenderAPI.h"

namespace EngineCore
{
    std::unique_ptr<Renderer> Renderer::s_Instance = nullptr;

    Renderer::Renderer()
    {

    }

    void Renderer::Create()
    {
        s_Instance = std::make_unique<Renderer>();
    }


    void Renderer::BeginFrame()
    {
        // Clear RenderInfo in RenderAPI
        RenderAPI::GetInstance().ClearRenderPassInfo();
    }

    void Renderer::Render(const RenderContext& context)
    {
        for(auto& pass : context.camera->mRenderPassAsset.renderPasses)
        {
            pass->Configure(context);
            pass->Filter(context);
            pass->Execute(context);
            pass->Submit();
        }
        RenderAPI::GetInstance().Submit(mRenderPassInfo);
    }

    void Renderer::EndFrame()
    {
        // submit data to renderthread.
        mRenderPassInfo.clear();
    }
}