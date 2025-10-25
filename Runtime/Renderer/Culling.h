#pragma once
#include "PreCompiledHeader.h"
#include "GameObject/Camera.h"
#include "RenderContext.h"
namespace EngineCore
{
    class Culling
    {
    public:
        static void Run(Camera* cam, RenderContext& context);
        
    };
} // namespace EngineCore