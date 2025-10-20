#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/RenderPipeLine/OpaqueRenderPass.h"
#include "Renderer/RenderPipeLine/FinalBlitPass.h"
#include "Renderer/FrameBufferManager.h"
#include "Graphics/FrameBufferObject.h"

namespace EngineCore
{
    Camera::Camera()
    {
        // fake one.
        mRenderPassAsset.renderPasses.push_back(new OpaqueRenderPass());
        mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

        FrameBufferDesc colorAttachmentDesc;
        colorAttachmentDesc.name = "CameraColorAttachment";
        colorAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        colorAttachmentDesc.width = 800;
        colorAttachmentDesc.height = 600;
        colorAttachmentDesc.format = TextureFormat::R8G8B8A8;
        colorAttachment = FrameBufferManager::GetInstance().CreateFBO(colorAttachmentDesc);

        FrameBufferDesc depthAttachmentDesc;
        depthAttachmentDesc.name = "CameraColorAttachment";
        depthAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        depthAttachmentDesc.width = 800;
        depthAttachmentDesc.height = 600;
        depthAttachmentDesc.format = TextureFormat::D24S8;
        depthAttachment = FrameBufferManager::GetInstance().CreateFBO(depthAttachmentDesc);
    }
}