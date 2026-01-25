#pragma once
#include <iostream>
#include "Graphics/IGPUResource.h"
#include "Resources/Resource.h"

namespace EngineCore
{
    class Texture : public Resource
    {
    public:
        Texture() = default;
        Texture(const string& textureID);

        Texture(MetaData* textureMetaData);
        //inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
        virtual void OnLoadComplete() override;
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
        std::vector<uint8_t> cpuData;
    };

}