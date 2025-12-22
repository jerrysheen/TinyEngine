#pragma once
#include <iostream>
#include "Resources/Resource.h"
#include "Core/PublicStruct.h"
namespace EngineCore
{
    class ComputeShader : public Resource
    {
    public:
        ComputeShader(const string& path);
        ShaderReflectionInfo mShaderReflectionInfo;
    };
}