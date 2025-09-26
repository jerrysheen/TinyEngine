#include "PreCompiledHeader.h"
#include "FinalBlitPass.h"
#include "Renderer/FrameBufferManager.h"
#include "Renderer/Renderer.h"
#include "Managers/SceneManager.h"
#include "Managers/WindowManager.h"

namespace EngineCore
{
    void FinalBlitPass::Create()
    {
        
    }
    void FinalBlitPass::Configure(const RenderContext& context)
    {
        SetRenderTarget(FrameBufferManager::GetInstance().GetScreenBuffer(), nullptr);
        SetClearFlag(ClearFlag::ALL);
        SetViewPort(Vector2(0, 0), Vector2(WindowManager::GetInstance().GetWidth(), WindowManager::GetInstance().GetHeight()));
    }
    void FinalBlitPass::Execute(const RenderContext& context)
    {
        Material* mat = SceneManager::GetInstance().blitMaterial;
        ModelData* model = SceneManager::GetInstance().quadMesh;
        mRenderPassInfo.drawRecordList.emplace_back(mat, model);
    }
    void FinalBlitPass::Filter(const RenderContext& context)
    {
    }
    void FinalBlitPass::Submit()
    {
        Renderer::GetInstance().AddRenderPassInfo(&mRenderPassInfo);
    }


} // namespace EngineCore