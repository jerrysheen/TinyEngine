#pragma once
#include "json.hpp"
#include "PreCompiledHeader.h"
#include "Math/Math.h"
#include "Core/PublicEnum.h"
#include "Resources/Asset.h"
#include "MetaData.h"
#include "Graphics/Mesh.h"

namespace EngineCore
{
    class MetaLoader
    {
    public:
        static MaterialMetaData* LoadMaterialMetaData(const std::string& path);
        static ShaderVariableType GetShaderVaribleType(uint32_t size);
        static TextureMetaData* LoadTextureMetaData(const std::string& path);
        static ModelMetaData* LoadModelMetaData(const std::string& path);
        template<typename T>
        static MetaData* LoadMetaData(const std::string& path)
        {
            MetaData* metaData = new MetaData();
            metaData->path = path;
            return metaData;
            // do nothing, for example shader..
        };

        

        template<typename T>
        static T* LoadMeta(const std::string& path)
        {
            std::ifstream file(path);
            
            if(!file.is_open())
            {
                throw std::runtime_error("Can't open Meta File" + path);
            }

            json j = json::parse(file);
            file.close();
            
            return j.get<T>();
        }

    };
    // 只声明特化
    template<>
    MetaData* MetaLoader::LoadMetaData<Texture>(const std::string& path);
    
    template<>
    MetaData* MetaLoader::LoadMetaData<Mesh>(const std::string& path);

    template<>
    MetaData* MetaLoader::LoadMetaData<Material>(const std::string& path);

}