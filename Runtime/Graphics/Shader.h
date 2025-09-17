#pragma once
#include "Graphics/ResourceStruct.h"
#include "PreCompiledHeader.h"

namespace EngineCore
{
    class Shader
    {
    public:
        static Shader* Compile(const string& path);
        static int ID;
        ShaderStageInfo* mShaderBindingInfo = nullptr;
        vector<InputLayout> mShaderInputLayout;

        Shader(){ mShaderID = Shader::ID; Shader::ID++;};
        ~Shader();
        int mShaderID;
    };
}