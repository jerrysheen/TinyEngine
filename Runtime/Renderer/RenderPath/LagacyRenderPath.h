#pragma once
#include "IRenderPath.h"
#include "Renderer/Renderer.h"
#include "Renderer/Culling.h"
#include "GameObject/Camera.h"
#include "Scene/Scene.h"
#include "Scene/GPUSCene.h"

namespace EngineCore
{
    class LagacyRenderPath : public IRenderPath
    {
    public:
        virtual ~LagacyRenderPath() override {};
        virtual void Execute(RenderContext& context) override;
    };
};