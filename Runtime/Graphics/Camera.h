#pragma once
#include "Core/PublicStruct.h"

namespace EngineCore
{
    class Camera
    {
    public:
        Camera();
        static Camera* GetMainCamera(){ return nullptr;};
    public:
        RenderPassAsset mRenderPassAsset;        
    };

}