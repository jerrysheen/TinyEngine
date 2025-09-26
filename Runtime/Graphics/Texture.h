#pragma once
#include "Core/PublicEnum.h"
#include "Core/Resources.h"

namespace EngineCore
{
    struct TextureStruct;
    class Texture
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
        TextureDimension dimension;
        TextureFormat format;
        int width, height;
    };

}