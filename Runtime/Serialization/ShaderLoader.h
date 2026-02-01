#pragma once
#include "Graphics/Shader.h"
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    class ShaderLoader : public IResourceLoader
    {
    public:
        virtual ~ShaderLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);

            Shader* shader = new Shader(path);
            result.resource = shader; 
            return result;
        }

    };
}