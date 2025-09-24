#include "PreCompiledHeader.h"
#include "FinalBlitPass.h"
#include "Renderer/FrameBufferManager.h"
#include "Renderer/Renderer.h"

namespace EngineCore
{
    void FinalBlitPass::Create()
    {
        
    }
    void FinalBlitPass::Configure(const RenderContext& context)
    {
        SetRenderTarget(FrameBufferManager::GetInstance().GetScreenBuffer(), nullptr);
    }
    void FinalBlitPass::Execute(const RenderContext& context)
    {
    }
    void FinalBlitPass::Filter(const RenderContext& context)
    {
    }
    void FinalBlitPass::Submit()
    {
        Renderer::GetInstance().AddRenderPassInfo(&mRenderPassInfo);
    }

    void FinalBlitPass::Clear()
    {
    }

} // namespace EngineCore