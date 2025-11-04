#pragma once
#include "iostream"
#include <unordered_map>
#include <string>
#include "Resources/Asset.h"
#include "Core/PublicEnum.h"
#include "Math/Math.h"

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

}