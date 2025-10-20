#pragma once
#include "Core/PublicStruct.h"
#include "Graphics/Texture.h"


namespace EngineCore
{
    class Camera
    {
    public:
        Camera();
        static Camera* GetMainCamera(){ return nullptr;};

    public:
        RenderPassAsset mRenderPassAsset;
        // 这两个只是描述符， 没必要用指针
        ResourceHandle<FrameBufferObject> depthAttachment;
        ResourceHandle<FrameBufferObject> colorAttachment;
    };

}