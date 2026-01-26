#pragma once
#include "PreCompiledHeader.h"
#include "Resources/Resource.h"
#include "Core/PublicStruct.h"
#include "Core/Object.h"


namespace EngineCore
{
    class Shader : public Resource
    {
    public:
        ShaderReflectionInfo mShaderReflectionInfo;
        vector<InputLayout> mShaderInputLayout;
        Shader(const std::string& path);
        Shader();
        ~Shader();
        string name;
    };
}