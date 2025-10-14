#include "PreCompiledHeader.h"
#include "Shader.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    Shader* Shader::Compile(const string& path)
    {
        return RenderAPI::s_Instance->CompileShader(path);
    }

    Shader::~Shader()
    {
        delete mShaderBindingInfo;
    }
}