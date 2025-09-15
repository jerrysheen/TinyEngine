#pragma once
#include "Graphics/ResourceStruct.h"
#include "PreCompiledHeader.h"

namespace EngineCore
{
    class Shader
    {
    public:
        static Shader* Compile(const string& path);
        
        ShaderStageInfo* vsInfo = nullptr;
        ShaderStageInfo* psInfo = nullptr;
        vector<InputLayout> mShaderInputLayout;

        ~Shader();
    };
}