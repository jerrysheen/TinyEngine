#pragma once
#include <iostream>
#include "Resources/Resource.h"
#include "Core/PublicStruct.h"
#include "CoreAssert.h"
#include <unordered_map>
#include "Graphics/IGPUResource.h"

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

        ComputeShader(const string& path);
        ShaderReflectionInfo mShaderReflectionInfo;
        std::unordered_map<std::string, IGPUBuffer*> resourceMap;
        void SetBuffer(const std::string& name, IGPUBuffer* buffer);
    };
}