#pragma once
#include <iostream>
#include "Resources/Resource.h"
#include "Core/PublicStruct.h"
#include "CoreAssert.h"
#include <unordered_map>
#include "Graphics/IGPUResource.h"
#include "Renderer/RenderCommand.h"

namespace EngineCore
{

    class ComputeShader : public Resource
    {
    public:
        IGPUBuffer* GetBufferResource(const std::string& slotName)
        {
            ASSERT(resourceMap.count(slotName) > 0);
            return resourceMap[slotName];
        }

        TextureBinding GetTextureBinding(const std::string& slotName)
        {
            ASSERT(textureResourceMap.count(slotName) > 0);
            return textureResourceMap[slotName];
        }
        void SetBuffer(const std::string& name, IGPUBuffer* buffer);
        void SetTexture(const std::string& name, IGPUTexture* texture, uint32_t mipLevel);

        ComputeShader(const string& path);
        ShaderReflectionInfo mShaderReflectionInfo;
    private:
        std::unordered_map<std::string, IGPUBuffer*> resourceMap;
        std::unordered_map<std::string, TextureBinding> textureResourceMap;
    };
}