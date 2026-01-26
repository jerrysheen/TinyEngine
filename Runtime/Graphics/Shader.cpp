#include "PreCompiledHeader.h"
#include "Shader.h"
#include "Renderer/RenderAPI.h"
#include "Resources/Asset.h"

namespace EngineCore
{
    Shader::~Shader()
    {
    }

    // todo: 后续应该用ShaderLoader，不应该用这个
    Shader::Shader(const std::string& path) : Resource(AssetType::Shader, path)
    {
        RenderAPI::s_Instance->CompileShader(path, this);
    }
}