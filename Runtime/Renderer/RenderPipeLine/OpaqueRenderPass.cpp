#include "PreCompiledHeader.h"
#include "OpaqueRenderPass.h"

// temp include
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"
#include "Core/PublicStruct.h"
#include "Renderer/Renderer.h"
#include "Core/PublicEnum.h"
#include "Renderer/BatchManager.h"
#include "Renderer/RenderEngine.h"
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    OpaqueRenderPass::OpaqueRenderPass()
    {
        Create();
    }

    void EngineCore::OpaqueRenderPass::Create()
    {

    }
    
    void EngineCore::OpaqueRenderPass::Configure(const RenderContext& context)
    {
        mRenderPassInfo.passName = "OpaqueRenderPass";
        mRenderPassInfo.enableBatch = true;
        RenderTexture* colorAttachment = context.camera->colorAttachment;
        RenderTexture* depthAttachment = context.camera->depthAttachment;
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0,0), Vector2(colorAttachment->GetWidth(), colorAttachment->GetHeight()));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }

    void OpaqueRenderPass::Prepare(RenderContext &context)
    {
        // 往哪里添加这个执行结果？
        ContextFilterSettings filterSettings;
        ContextDrawSettings drawSettings;
        drawSettings.sortingCriteria = SortingCriteria::ComonOpaque;
        RenderContext::DrawRenderers(context,
                                        drawSettings,
                                        filterSettings,
                                        mRenderPassInfo.renderBatchList);
    }

    // maybe send a context here?
    void EngineCore::OpaqueRenderPass::Execute(RenderContext& context)
    {


        PROFILER_EVENT_BEGIN("MainThread::OpaqueRenderPass::SetDrawCall");

        // todo： 后面挪到别的地方， 先做Batch的部分：
        Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        for each(auto& record in mRenderPassInfo.renderBatchList)
        {
            // 根据mat + pass信息组织pippeline
            Renderer::GetInstance()->SetRenderState(record.mat, mRenderPassInfo);
            // copy gpu material data desc 
            Renderer::GetInstance()->SetMaterialData(record.mat);
            // bind mesh vertexbuffer and indexbuffer.
            Renderer::GetInstance()->SetMeshData(record.mesh);
            Renderer::GetInstance()->DrawIndexedInstanced(record.mesh, record.instanceCount, PerDrawHandle{ nullptr, (uint32_t)record.alloc.offset, 0 });
        }

        PROFILER_EVENT_END("MainThread::OpaqueRenderPass::SetDrawCall");

    }

    void OpaqueRenderPass::Filter(const RenderContext &context)
    {
    }

    // 整理成一个结构体发往渲染管线？
    void OpaqueRenderPass::Submit()
    {
        //Renderer::GetInstance()->AddRenderPassInfo(&mRenderPassInfo);
    }


}
