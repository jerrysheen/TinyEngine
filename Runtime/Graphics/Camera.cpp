#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/RenderPipeLine/OpaqueRenderPass.h"
#include "Renderer/RenderPipeLine/FinalBlitPass.h"
#include "Renderer/FrameBufferManager.h"

namespace EngineCore
{
    Camera::Camera()
    {
        // fake one.
        mRenderPassAsset.renderPasses.push_back(new OpaqueRenderPass());
        mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

        colorAttachment = FrameBufferObject();
        colorAttachment.name = "CameraColorAttachment";
        colorAttachment.dimension = TextureDimension::TEXTURE2D;
        colorAttachment.width = 800;
        colorAttachment.height = 600;
        colorAttachment.format = TextureFormat::R8G8B8A8;
        FrameBufferManager::GetInstance().CreateFBO(&colorAttachment);

        depthAttachment = FrameBufferObject();
        depthAttachment.name = "CameraDepthAttachment";
        depthAttachment.dimension = TextureDimension::TEXTURE2D;
        depthAttachment.width = 800;
        depthAttachment.height = 600;
        depthAttachment.format = TextureFormat::D24S8;
        FrameBufferManager::GetInstance().CreateFBO(&depthAttachment);
    }
}