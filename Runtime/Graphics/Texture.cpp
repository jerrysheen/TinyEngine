#include "PreCompiledHeader.h"
#include "Texture.h"
#include "stb_image.h"
#include "Serialization/MetaData.h"
#include "Renderer/RenderAPI.h"
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    Texture::Texture(const string& textureID)
        :textureBuffer(nullptr)
    {
        textureDesc.name = textureID;
    }

    Texture::Texture(MetaData* metaData) : Resource(metaData)
    {
        ASSERT(metaData->dependentMap.size() == 0);

        TextureMetaData* texMetaData = static_cast<TextureMetaData*>(metaData);
        textureDesc.dimension = texMetaData->dimension;
        textureDesc.format = texMetaData->format;
        textureDesc.width = texMetaData->width;
        textureDesc.height = texMetaData->height;
        int nrComponents;
        string path = PathSettings::ResolveAssetPath(mPath);
        stbi_uc* pixels = stbi_load(path.c_str(), &textureDesc.width, &textureDesc.height, &nrComponents, STBI_rgb_alpha);
        // 2. CreateData
        ASSERT(pixels != nullptr);
        textureBuffer = RenderAPI::GetInstance()->CreateTextureBuffer(pixels, textureDesc);

        stbi_image_free(pixels);
    }

}

