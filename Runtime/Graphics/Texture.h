#pragma once
#include "Buffer.h"

namespace EngineCore
{
    class Texture : public Buffer2D
    {
    public:
        Texture() = default;
        Texture(const string& textureID);

        Texture(MetaData* textureMetaData);
        inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return mWidth; };
        inline int GetHeight() { return mHeight; };


    };

}