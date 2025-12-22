#pragma once
#include "Renderer/RenderContext.h"
namespace EngineCore
{
    class Renderer;
    class IRenderPath
    {
    public:
        virtual ~IRenderPath() = default;
        virtual void Execute(RenderContext& context) = 0;
    };
}