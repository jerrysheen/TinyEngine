#pragma once
#include "Core/PublicEnum.h"
#include "Resources/MetaFile.h"
#include "Resources/Asset.h"
#include "Resources/Resource.h"

namespace EngineCore
{
    class Texture : public Resource
    {
    public:
        Texture() = default;
        Texture(const string& textureID) :mTextureName(textureID) {};
        Texture(MetaData* textureMetaData);
        inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return mWidth; };
        inline int GetHeight() { return mHeight; };

    private:
        string mTextureName;
        TextureDimension mDimension = TextureDimension::TEXTURE2D;
        TextureFormat mFormat = TextureFormat::R8G8B8A8;
        int mWidth = 0, mHeight = 0;
    };

}