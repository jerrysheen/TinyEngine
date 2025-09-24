#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/RenderPipeLine/ForwardRenderPass.h"
#include "Renderer/RenderPipeLine/FinalBlitPass.h"

namespace EngineCore
{
    Camera::Camera()
    {
        // fake one.
        mRenderPassAsset.renderPasses.push_back(new ForwardRenderPass());
       // mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

    }
}