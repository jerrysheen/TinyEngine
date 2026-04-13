#pragma once
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class RenderTexture
    {
    public:
        RenderTexture() = default;
        RenderTexture(TextureDesc textureDesc);


        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
        inline int GetMipCount() { return textureDesc.mipCount; };
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
    };

}