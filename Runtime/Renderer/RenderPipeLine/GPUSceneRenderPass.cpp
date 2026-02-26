#include "PreCompiledHeader.h"
#include "GPUSceneRenderPass.h"
#include "Settings/ProjectSettings.h"

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
        SetViewPort(Vector2(0,0), Vector2(colorAttachment->GetWidth(), colorAttachment->GetHeight()));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }
    
    // maybe send a context here?
    void EngineCore::GPUSceneRenderPass::Execute(RenderContext& context)
    {
        //// 每Pass设置一次
        //m_LastMatState.Reset();

        //Renderer::GetInstance()->ConfigureRenderTarget(mRenderPassInfo);
        //Renderer::GetInstance()->SetViewPort(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);
        //Renderer::GetInstance()->SetSissorRect(mRenderPassInfo.viewportStartPos, mRenderPassInfo.viewportEndPos);

        //Renderer::GetInstance()->SetPerPassData((UINT)mRenderPassInfo.mRootSigSlot);
        //if(!RenderSettings::s_EnableVertexPulling)
        //{
        //    for(auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
        //    {
        //        int batchID = BatchManager::GetInstance()->drawIndirectParamMap[hashID].indexInDrawIndirectList;
        //        int stratIndex = BatchManager::GetInstance()->drawIndirectParamMap[hashID].startIndexInInstanceDataList;
        //        Material* mat = renderContext.material;
        //        Mesh* mesh = renderContext.mesh;
        //        // 根据mat + pass信息组织pippeline
        //        Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        //        // copy gpu material data desc 
        //        Renderer::GetInstance()->SetBindlessMat(mat);
        //        // bind mesh vertexbuffer and indexbuffer.
        //        Renderer::GetInstance()->SetMeshData(mesh);
        //        Payload_DrawIndirect indirectPayload;
        //        // temp:
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
        //        ASSERT(indirectDrawArgsBuffer != nullptr);
        //        indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
        //        indirectPayload.count = 1;
        //        indirectPayload.startIndex = batchID;
        //        indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
        //        Renderer::GetInstance()->DrawIndirect(indirectPayload);
        //    }
        //}
        //else
        //{
        //    for(auto& [hashID, renderContext] : BatchManager::GetInstance()->drawIndirectContextMap)
        //    {
        //        int batchID = BatchManager::GetInstance()->drawIndirectParamMap[hashID].indexInDrawIndirectList;
        //        int stratIndex = BatchManager::GetInstance()->drawIndirectParamMap[hashID].startIndexInInstanceDataList;
        //        Material* mat = renderContext.material;
        //        if (mat->GetMaterialRenderState().GetHash() != m_LastMatState.GetHash())
        //        {
        //            m_LastMatState = mat->GetMaterialRenderState();
        //            Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
        //            Renderer::GetInstance()->SetBindlessMat(mat);
        //            Renderer::GetInstance()->SetBindLessMeshIB(0);
        //        }

        //        Payload_DrawIndirect indirectPayload;
        //        // temp:
        //        GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
        //        ASSERT(indirectDrawArgsBuffer != nullptr);
        //        indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
        //        indirectPayload.count = 1;
        //        indirectPayload.startIndex = batchID;
        //        indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
        //        Renderer::GetInstance()->DrawIndirect(indirectPayload);
        //    }
        //}

    }

    void GPUSceneRenderPass::Filter(const RenderContext &context)
    {
    }

    // 整理成一个结构体发往渲染管线？
    void GPUSceneRenderPass::Submit()
    {
        //Renderer::GetInstance()->AddRenderPassInfo(&mRenderPassInfo);
    }


}
