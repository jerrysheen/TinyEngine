#include "PreCompiledHeader.h"
#include "OpaqueRenderPass.h"

// temp include
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"
#include "Renderer/FrameBufferManager.h"
#include "Core/PublicStruct.h"
#include "Renderer/Renderer.h"
#include "Core/PublicEnum.h"

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
        ResourceHandle<FrameBufferObject> colorAttachment = context.camera->colorAttachment;
        ResourceHandle<FrameBufferObject> depthAttachment = context.camera->depthAttachment;
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0,0), Vector2(colorAttachment->mWidth, colorAttachment->mHeight));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }
    
    // maybe send a context here?
    void EngineCore::OpaqueRenderPass::Execute(const RenderContext& context)
    {
        // 往哪里添加这个执行结果？
        for(auto* items : context.cameraVisibleItems)
        {
            auto& mpb = items->meshRenderer->GetMaterialPropertyBlock();
            mpb.SetValue("ProjectionMatrix", context.camera->mProjectionMatrix);
            mpb.SetValue("ViewMatrix", context.camera->mViewMatrix);
            mpb.SetValue("WorldMatrix", items->transform->GetWorldMatrix());
            
            PerDrawHandle handle = RenderAPI::GetInstance()->AllocatePerDrawData(mpb.GetSize());
            memcpy(handle.destPtr, mpb.GetData(), mpb.GetSize());
            mRenderPassInfo.drawRecordList.emplace_back(items->meshRenderer->GetMaterial(), items->meshFilter->mMeshHandle.Get(), handle, 1);
        }
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
