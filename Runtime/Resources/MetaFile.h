#pragma once
#include "PreCompiledHeader.h"
#include "Math/Math.h"
#include "Core/PublicEnum.h"
#include "Resources/Asset.h"

namespace EngineCore
{
    class Texture;
    class ModelData;
    class Material;
    struct MetaData
    {
        string path;
        AssetType assetType;
        // k,v = <ResouceName, MetaData>
        std::unordered_map<std::string, MetaData> dependentMap;
        MetaData() = default;
        MetaData(const std::string& path, AssetType type)
            : path(path), assetType(type) {};
    };


    struct TextureMetaData : MetaData
    {
        TextureDimension dimension;
        TextureFormat format;
        int width, height;
        // todo
        // warp mode, read write..
        // enable mipmap...
    };

    struct MaterialMetaData : MetaData 
    {
		string shaderPath;       
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;
        unordered_map<string, Matrix4x4> matrix4x4Data;
        // 无意义的Texture*，只是为了和MaterialData做方便的同步而已。
        unordered_map<string, Texture*> textureData;
    };

    struct ModelMetaData : MetaData 
    {
        
    };

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

    };
    // 只声明特化
    template<>
    MetaData* MetaLoader::LoadMetaData<Texture>(const std::string& path);
    
    template<>
    MetaData* MetaLoader::LoadMetaData<ModelData>(const std::string& path);

    template<>
    MetaData* MetaLoader::LoadMetaData<Material>(const std::string& path);
}