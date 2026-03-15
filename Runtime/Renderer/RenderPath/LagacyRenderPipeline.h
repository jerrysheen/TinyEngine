#pragma once
#include "IRenderPipeline.h"
#include "Renderer/RenderBackend.h"
#include "Renderer/Culling.h"
#include "GameObject/Camera.h"
#include "Scene/Scene.h"
#include "Scene/GPUSCene.h"

namespace EngineCore
{
    class LagacyRenderPipeline : public IRenderPipeline
    {
    public:
        virtual ~LagacyRenderPipeline() override {};
        virtual void Prepare(RenderContext& context) override;
        virtual void RecordAndFlush(RenderContext& context) override;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) override;
    
    };

};