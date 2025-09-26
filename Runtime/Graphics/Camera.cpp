#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/RenderPipeLine/OpaqueRenderPass.h"
#include "Renderer/RenderPipeLine/FinalBlitPass.h"

namespace EngineCore
{
    Camera::Camera()
    {
        // fake one.
        mRenderPassAsset.renderPasses.push_back(new OpaqueRenderPass());
        mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

    }
}