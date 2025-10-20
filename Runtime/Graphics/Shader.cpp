#include "PreCompiledHeader.h"
#include "Shader.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    Shader::Shader(MetaData* metaFile) : Resource(metaFile)
    {
        ASSERT(metaFile->dependentMap.size() == 0);
        RenderAPI::s_Instance->CompileShader(mPath, this);
    }

    Shader::~Shader()
    {
    }
}