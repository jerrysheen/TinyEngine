#pragma once
#include "Resources/Resource.h"
#include "Core/PublicEnum.h"
#include "Serialization/MetaData.h"
#include "Resources/Asset.h"
//#include "Resources/Resource.h"

namespace EngineCore
{
    class Buffer2D : public Resource
    {
    public:
        string mTextureName;
        TextureDimension mDimension = TextureDimension::TEXTURE2D;
        TextureFormat mFormat = TextureFormat::R8G8B8A8;
        int mWidth = 0, mHeight = 0;
        Buffer2D() = default;
        Buffer2D(MetaData* metaData)
        {

            mPath = metaData->path;
            mAssetType = AssetType::Texture2D;
            SetAssetID(AssetIDGenerator::NewFromFile(mPath));
        };
    };
}