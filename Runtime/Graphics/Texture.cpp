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

    void Texture::OnLoadComplete()
    {
        // 做保护， 避免外部多次访问
        if (textureBuffer && cpuData.size() == 0) return;
        textureBuffer = RenderAPI::GetInstance()->CreateTextureBuffer(static_cast<unsigned char*>(cpuData.data()), textureDesc);
        // 似乎还不能清？为什么
        // 代码逻辑错误。
        cpuData.clear();
        cpuData.shrink_to_fit();
    }

}

