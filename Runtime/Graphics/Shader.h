#pragma once
#include "Graphics/ResourceStruct.h"
#include "PreCompiledHeader.h"

namespace EngineCore
{
    class Shader
    {
    public:
        static Shader* Compile(const string& path);
        
        
        std::unordered_map<string, ShaderVariableInfo>  mShaderVariableInfoMap;
        ShaderStageInfo* vsInfo = nullptr;
        ShaderStageInfo* psInfo = nullptr;
        vector<InputLayout> mShaderInputLayout;
    };
}