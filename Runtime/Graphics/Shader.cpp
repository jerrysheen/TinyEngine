#include "PreCompiledHeader.h"
#include "Shader.h"
#include "Renderer/RenderAPI.h"
#include "Resources/Asset.h"

namespace EngineCore
{
    Shader::Shader(MetaData* metaFile) : Resource(metaFile)
    {
        mAssetType = AssetType::Shader;
        ASSERT(metaFile->dependentMap.size() == 0);
        RenderAPI::s_Instance->CompileShader(mPath, this);
    }

    Shader::~Shader()
    {
    }
}