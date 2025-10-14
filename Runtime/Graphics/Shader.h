#pragma once
#include "PreCompiledHeader.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"
#include "Core/Object.h"


namespace EngineCore
{
    class Shader : public Object
    {
    public:
        static Shader* Compile(const string& path);
        ShaderReflectionInfo* mShaderBindingInfo = nullptr;
        vector<InputLayout> mShaderInputLayout;

        Shader(){};
        ~Shader();
        string name;
    };
}