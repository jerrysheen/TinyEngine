#pragma once
#include "Core/PublicEnum.h"
#include "Core/Resources.h"
#include "Core/Object.h"

namespace EngineCore
{
    struct TextureStruct;
    class Texture : public Object
    {
    public:
        static Texture* LoadTexture(const TextureStruct* texStruct);
        Texture() = default;
        Texture(const string& textureID) :mTextureName(textureID) {};
        inline const string GetName() const { return mTextureName; };

        Texture& operator=(const TextureStruct& other) 
        {
            mTextureName = other.name;
            path = other.path;
            dimension = other.dimension;
            format = other.format;
            width = other.width;
            height = other.height;
            return *this;
        }

        inline int GetWidth() { return width; };
        inline int GetHeight() { return height; };

    private:
        string mTextureName;
        string path;
        TextureDimension dimension = TextureDimension::TEXTURE2D;
        TextureFormat format = TextureFormat::R8G8B8A8;
        int width = 0, height = 0;
    };

}