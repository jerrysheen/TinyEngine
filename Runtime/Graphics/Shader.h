#pragma once
#include "PreCompiledHeader.h"
#include "Resources/MetaFile.h"
#include "Resources/Resource.h"
#include "Core/PublicStruct.h"
#include "Core/Object.h"


namespace EngineCore
{
    class Shader : public Resource
    {
    public:
        Shader(MetaData* metaFile);
        ShaderReflectionInfo mShaderBindingInfo;
        vector<InputLayout> mShaderInputLayout;

        Shader();
        ~Shader();
        string name;
    };
}