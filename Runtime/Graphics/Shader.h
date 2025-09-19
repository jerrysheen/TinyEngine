#pragma once
#include "PreCompiledHeader.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"

namespace EngineCore
{
    class Shader
    {
    public:
        static Shader* Compile(const string& path);
        static int ID;
        ShaderReflectionInfo* mShaderBindingInfo = nullptr;
        vector<InputLayout> mShaderInputLayout;

        Shader(){ mShaderID = Shader::ID; Shader::ID++;};
        ~Shader();
        int mShaderID;
    };
}