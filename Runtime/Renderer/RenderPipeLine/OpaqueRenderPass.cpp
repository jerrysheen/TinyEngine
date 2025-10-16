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
        FrameBufferObject* colorAttachment = &context.camera->colorAttachment;
        FrameBufferObject* depthAttachment = &context.camera->depthAttachment;
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0,0), Vector2(colorAttachment->width, colorAttachment->height));
        SetClearFlag(ClearFlag::All, Vector3(0.0, 0.0, 0.0), 1.0f);
    }
    
    // maybe send a context here?
    void EngineCore::OpaqueRenderPass::Execute(const RenderContext& context)
    {
        // 往哪里添加这个执行结果？
        for(auto& items : context.cameraVisibleItems)
        {
            mRenderPassInfo.drawRecordList.emplace_back(items->mat, items->model);
        }
    }

    void OpaqueRenderPass::Filter(const RenderContext &context)
    {
    }

    // 整理成一个结构体发往渲染管线？
    void OpaqueRenderPass::Submit()
    {
        //Renderer::GetInstance().AddRenderPassInfo(&mRenderPassInfo);
    }


}
