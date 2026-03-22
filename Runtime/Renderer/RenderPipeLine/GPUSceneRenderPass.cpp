#include "PreCompiledHeader.h"
#include "GPUSceneRenderPass.h"
#include "Settings/ProjectSettings.h"

// temp include
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderBackend.h"
#include "Core/PublicEnum.h"
#include "Renderer/BatchManager.h"
#include "Renderer/RenderEngine.h"
#include "Graphics/RenderTexture.h"
#include "Renderer/RenderBackend.h"

namespace EngineCore
{
    GPUSceneRenderPass::GPUSceneRenderPass()
    {
        Create();
    }

    void EngineCore::GPUSceneRenderPass::Create()
    {

    }

    void EngineCore::GPUSceneRenderPass::Configure(const RenderContext& context)
    {
        mRenderPassInfo.passName = "GPUSceneRenderPass";
        mRenderPassInfo.enableBatch = true;
        mRenderPassInfo.enableIndirectDrawCall = true;
        RenderTexture* colorAttachment = context.camera->colorAttachment;
        RenderTexture* depthAttachment = context.camera->depthAttachment;
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0, 0), Vector2(colorAttachment->GetWidth(), colorAttachment->GetHeight()));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }

    // maybe send a context here?
    void EngineCore::GPUSceneRenderPass::Execute(RenderContext& context)
    {
        // 每Pass设置一次
        m_LastMatState.Reset();

        RenderBackend::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        RenderBackend::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        RenderBackend::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        RenderBackend::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        if (!RenderSettings::s_EnableVertexPulling)
        {
            auto& indirectContextMap = BatchManager::GetInstance()->GetDrawIndirectContextMap();
            auto& indirectParamMap = BatchManager::GetInstance()->GetDrawIndirectParamMap();
            for (auto& [hashID, renderContext] : indirectContextMap)
            {
                int batchID = indirectParamMap[hashID].indexInDrawIndirectList;
                int stratIndex = indirectParamMap[hashID].startIndexInInstanceDataList;
                Material* mat = renderContext.material;
                Mesh* mesh = renderContext.mesh;
                // 根据mat + pass信息组织pippeline
                RenderBackend::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                // copy gpu material data desc 
                RenderBackend::GetInstance()->SetBindlessMat(mat);
                // bind mesh vertexbuffer and indexbuffer.
                RenderBackend::GetInstance()->SetMeshData(mesh);
                Payload_DrawIndirect indirectPayload;
                // temp:
                indirectPayload.indirectArgsBuffer = context.IndirectDrawArgsBuffer;
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                RenderBackend::GetInstance()->DrawIndirect(indirectPayload);
            }
        }
        else
        {
            auto& indirectContextMap = BatchManager::GetInstance()->GetDrawIndirectContextMap();
            auto& indirectParamMap = BatchManager::GetInstance()->GetDrawIndirectParamMap();
            for (auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
            {

                int batchID = indirectParamMap[hashID].indexInDrawIndirectList;
                int stratIndex = indirectParamMap[hashID].startIndexInInstanceDataList;
                Material* mat = renderContext.material;
                if (mat->GetMaterialRenderState().GetHash() != m_LastMatState.GetHash())
                {
                    m_LastMatState = mat->GetMaterialRenderState();
                    RenderBackend::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                    RenderBackend::GetInstance()->SetBindlessMat(mat);
                    RenderBackend::GetInstance()->SetBindLessMeshIB(0);
                }

                Payload_DrawIndirect indirectPayload;
                // temp:
                indirectPayload.indirectArgsBuffer = context.IndirectDrawArgsBuffer;
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                RenderBackend::GetInstance()->DrawIndirect(indirectPayload);
            }
        }

    }

    void GPUSceneRenderPass::Filter(const RenderContext& context)
    {
    }

    // 整理成一个结构体发往渲染管线？
    void GPUSceneRenderPass::Submit()
    {
        //Renderer::GetInstance()->AddRenderPassInfo(&mRenderPassInfo);
    }


}
