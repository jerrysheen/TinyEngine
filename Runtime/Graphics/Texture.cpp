#include "PreCompiledHeader.h"
#include "Texture.h"
#include "stb_image.h"
#include "Core/Resources.h"
#include "Renderer/RenderAPI.h"


namespace EngineCore
{
    Texture* Texture::LoadTexture(const TextureStruct* texStruct)
    {
        // todo: 检测是否存在这张图
        // 1. stbiLoad
        int nrComponents;
        Texture* texture = new Texture();

        stbi_uc* pixels = stbi_load(texStruct->path.c_str(), &texture->width, &texture->height, &nrComponents, STBI_rgb_alpha);
        // 2. CreateData
        *texture = *texStruct;
        RenderAPI::GetInstance().CreateTextureBuffer(pixels, texture);

        stbi_image_free(pixels);
        return texture;
    }

}

