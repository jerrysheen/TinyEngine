#pragma once
#include "Renderer/RenderContext.h"
namespace EngineCore
{
    class Renderer;
    class IRenderPipeline
    {
    public:
        virtual ~IRenderPipeline() = default;
        virtual void Prepare(RenderContext& context) = 0;
        virtual void RecordAndFlush(RenderContext& context) = 0;
    };
}