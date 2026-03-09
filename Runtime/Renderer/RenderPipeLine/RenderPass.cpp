#include "PreCompiledHeader.h"
#include "RenderPass.h"
#include "Renderer/RenderBackend.h"

namespace EngineCore
{
    void RenderPass::IssueRenderCommandCommon(const RenderPassInfo &passInfo, const std::vector<RenderBatch> &batches)
    {
        // todo： 后面挪到别的地方， 先做Batch的部分：
        RenderBackend::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        RenderBackend::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        RenderBackend::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        RenderBackend::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        for each(auto& record in mRenderPassInfo.renderBatchList)
        {
            // 根据mat + pass信息组织pippeline
            RenderBackend::GetInstance()->SetRenderState(record.mat, mRenderPassInfo);
            // copy gpu material data desc 
            RenderBackend::GetInstance()->SetMaterialData(record.mat);
            // bind mesh vertexbuffer and indexbuffer.
            RenderBackend::GetInstance()->SetMeshData(record.mesh);
            RenderBackend::GetInstance()->DrawIndexedInstanced(record.mesh, record.instanceCount, PerDrawHandle{ nullptr, (uint32_t)record.alloc.offset, 0 });

        }
    }

}