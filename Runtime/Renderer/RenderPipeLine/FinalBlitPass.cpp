#include "PreCompiledHeader.h"
#include "FinalBlitPass.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneManager.h"
#include "Managers/WindowManager.h"
#include "Core/PublicEnum.h"
#include "Renderer/BatchManager.h"
#include "Renderer/RenderEngine.h"

#ifdef EDITOR
#include "EditorSettings.h"
#endif

namespace EngineCore
{
    void FinalBlitPass::Create()
    {
        
    }
    void FinalBlitPass::Configure(const RenderContext& context)
    {
        mRenderPassInfo.passName = "FinalBlitPass";
        mRenderPassInfo.enableBatch = false;
        SetRenderTarget(RenderAPI::GetInstance()->GetCurrentBackBuffer(), nullptr);
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
        #ifdef EDITOR
            SetViewPort(EngineEditor::EditorSettings::GetGameViewPanelStartPos(), EngineEditor::EditorSettings::GetGameViewPanelEndPos());
        #else
            SetViewPort(Vector2(0, 0), Vector2(WindowManager::GetInstance()->GetWidth(), WindowManager::GetInstance()->GetHeight()));
        #endif

    }
    void FinalBlitPass::Prepare(RenderContext &context)
    {
        mRootSigSlot = RootSigSlot::PerPassData;
        mat = SceneManager::GetInstance()->blitMaterial;
        mat->SetTexture("SrcTexture", context.camera->colorAttachment->textureBuffer);
        model = SceneManager::GetInstance()->quadMesh;
        mRenderPassInfo.drawRecordList.emplace_back(mat, model);
    }

    void FinalBlitPass::Execute(RenderContext &context)
    {

        // todo： 后面挪到别的地方， 先做Batch的部分：
        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        
        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        
        // 根据mat + pass信息组织pippeline
        Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        // copy gpu material data desc 
        Renderer::GetInstance()->SetMaterialData(mat);
        //Texture* tex = context.camera->colorAttachment.Get();
        //Renderer::GetInstance()->SetResourceState(tex, BufferResourceState::STATE_SHADER_RESOURCE);
        //Renderer::GetInstance()->BindTexture(tex, 0, 0);
        // bind mesh vertexbuffer and indexbuffer.
        Renderer::GetInstance()->SetMeshData(model);
        Renderer::GetInstance()->DrawIndexedInstanced(model, 1, PerDrawHandle{0,0});
    }

    void FinalBlitPass::Filter(const RenderContext& context)
    {
    }

    void FinalBlitPass::Submit()
    {
        //Renderer::GetInstance()->AddRenderPassInfo(&mRenderPassInfo);
    }


} // namespace EngineCore