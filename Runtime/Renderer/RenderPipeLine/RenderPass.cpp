#include "PreCompiledHeader.h"
#include "RenderPass.h"
#include "Renderer/Renderer.h"

namespace EngineCore
{
    void RenderPass::IssueRenderCommandCommon(const RenderPassInfo &passInfo, const std::vector<RenderBatch> &batches)
    {
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
    }

}