#include "PreCompiledHeader.h"
#include "Texture.h"
#include "stb_image.h"
#include "Serialization/MetaData.h"
#include "Renderer/RenderAPI.h"
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    Texture::Texture(const string& textureID)
    {
        mTextureName = textureID;
    }

    Texture::Texture(MetaData* metaData) : Buffer2D(metaData)
    {
        ASSERT(metaData->dependentMap.size() == 0);

        TextureMetaData* texMetaData = static_cast<TextureMetaData*>(metaData);
        mDimension = texMetaData->dimension;
        mFormat = texMetaData->format;
        mWidth = texMetaData->width;
        mHeight = texMetaData->height;
        int nrComponents;
        string path = PathSettings::ResolveAssetPath(mPath);
        stbi_uc* pixels = stbi_load(path.c_str(), &mWidth, &mHeight, &nrComponents, STBI_rgb_alpha);
        // 2. CreateData
        ASSERT(pixels != nullptr);
        RenderAPI::GetInstance()->CreateTextureBuffer(pixels, this);

        stbi_image_free(pixels);
    }

}

