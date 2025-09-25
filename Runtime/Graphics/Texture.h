#pragma once
#include "Core/PublicEnum.h"

namespace EngineCore
{
    class Texture
    {
    public:
        Texture() = default;
        Texture(const string& textureID) :mTextureName(textureID) {};
        inline const string GetName() const { return mTextureName; };
    private:
        string mTextureName;
    };

}