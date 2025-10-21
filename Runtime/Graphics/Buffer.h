#pragma once
#include "Resources/Resource.h"
#include "Core/PublicEnum.h"
#include "Resources/MetaFile.h"
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
            mAssetType = metaData->assetType;
            SetAssetID(AssetIDGenerator::NewFromFile(mPath));
        };
    };
}