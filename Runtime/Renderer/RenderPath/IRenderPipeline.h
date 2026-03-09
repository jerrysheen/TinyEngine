#pragma once
#include "Renderer/RenderContext.h"
namespace EngineCore
{
    class RenderBackend;
    class IRenderPipeline
    {
    public:
        virtual ~IRenderPipeline() = default;
        //virtual void Record(const CommandStream& cmdStream, RenderContext& context) = 0;
        virtual void Record(RenderContext& context) = 0;
        virtual void Prepare(RenderContext& context) = 0;
    };
}