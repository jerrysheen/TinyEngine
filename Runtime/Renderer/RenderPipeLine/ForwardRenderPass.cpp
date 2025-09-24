#include "PreCompiledHeader.h"
#include "ForwardRenderPass.h"

// temp include
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"
#include "Renderer/FrameBufferManager.h"
#include "Core/PublicStruct.h"
#include "Renderer/Renderer.h"


namespace EngineCore
{
    ForwardRenderPass::ForwardRenderPass()
    {
        Create();
    }

    void EngineCore::ForwardRenderPass::Create()
    {
        // todo： 修改设计，这个地方看起来怪怪的，
        FrameBufferObject* colorAttachment = new FrameBufferObject();
        colorAttachment->name = "CameraColorAttachment";
        colorAttachment->dimension = TextureDimension::TEXTURE2D;
        colorAttachment->width = 800;
        colorAttachment->height = 600;
        colorAttachment->format = TextureFormat::R8G8B8A8;
        FrameBufferManager::GetInstance().CreateFBO(colorAttachment->name, colorAttachment);

        FrameBufferObject* depthAttachment = new FrameBufferObject();
        depthAttachment->name = "CameraDepthAttachment";
        depthAttachment->dimension = TextureDimension::TEXTURE2D;
        depthAttachment->width = 800;
        depthAttachment->height = 600;
        depthAttachment->format = TextureFormat::D24S8;
        FrameBufferManager::GetInstance().CreateFBO(depthAttachment->name, depthAttachment);
    }
    
    void EngineCore::ForwardRenderPass::Configure(const RenderContext& context)
    {
        FrameBufferObject* colorAttachment = FrameBufferManager::GetInstance().GetFBO("CameraColorAttachment");
        FrameBufferObject* depthAttachment = FrameBufferManager::GetInstance().GetFBO("CameraDepthAttachment");
        SetRenderTarget(colorAttachment, depthAttachment);
        SetViewPort(Vector2(0,0), Vector2(128, 128));
    }
    
    // maybe send a context here?
    void EngineCore::ForwardRenderPass::Execute(const RenderContext& context)
    {
        // 往哪里添加这个执行结果？
        for(auto& items : context.cameraVisibleItems)
        {
            mRenderPassInfo.drawRecordList.emplace_back(items->mat, items->model);
        }
    }

    void ForwardRenderPass::Filter(const RenderContext &context)
    {
    }

    // 整理成一个结构体发往渲染管线？
    void ForwardRenderPass::Submit()
    {
        Renderer::GetInstance().AddRenderPassInfo(&mRenderPassInfo);
    }

    void ForwardRenderPass::Clear()
    {
        mRenderPassInfo.Reset();
    }
}
