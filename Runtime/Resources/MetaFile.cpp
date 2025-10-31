#include "PreCompiledHeader.h"
#include "MetaFile.h"
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    // 定义特化
    template<>
    MetaData* MetaLoader::LoadMetaData<Texture>(const std::string& path)
    {
        return LoadTextureMetaData(path);
    }

    template<>
    MetaData* MetaLoader::LoadMetaData<ModelData>(const std::string& path)
    {
        return LoadModelMetaData(path);
    }

    template<>
    MetaData* MetaLoader::LoadMetaData<Material>(const std::string& path)
    {
        return LoadMaterialMetaData(path);
    }

    MaterialMetaData* MetaLoader::LoadMaterialMetaData(const string& path)
    {
        // todo: 解析Matea数据；
        // todo 写成对象 然后move
        // todo bug修改， cpu数据靠这里同步的话， 会报错， 变成一定要在这里写了
        MaterialMetaData* matMetaData = new MaterialMetaData();
        matMetaData->path = path;
        matMetaData->vec3Data.emplace("LightDirection", Vector3(0.3,0.4,0.5));
        matMetaData->vec3Data.emplace("LightColor", Vector3(0.3,0.4,0.5));
        matMetaData->vec3Data.emplace("CameraPosition", Vector3(0.3,0.4,0.5));
        matMetaData->matrix4x4Data.emplace("WorldMatrix", Matrix4x4());
        matMetaData->matrix4x4Data.emplace("ProjectionMatrix", Matrix4x4::Identity);
        matMetaData->matrix4x4Data.emplace("ViewMatrix", Matrix4x4::Identity);
        matMetaData->matrix4x4Data.emplace("WorldMatrix", Matrix4x4::Identity);
        // 空插槽索引？
        matMetaData->textureData.emplace("DiffuseTexture", nullptr);
        
        
        // todo : 真实的Meta数据序列化
        MetaData texMeta;
        texMeta.path = "Textures/viking_room.png";
        texMeta.assetType = AssetType::Texture2D;
        MetaData shaderMeta;
        shaderMeta.path = "Shader/SimpleTestShader.hlsl";
        shaderMeta.assetType = AssetType::Shader;                                         
        
        matMetaData->dependentMap.try_emplace("DiffuseTexture",texMeta);
        matMetaData->dependentMap.try_emplace("Shader",shaderMeta);
        return matMetaData;
    }
    

    // todo : 创建AssetID这个部分，目前还没做
    TextureMetaData* MetaLoader::LoadTextureMetaData(const std::string &path)
    {
        TextureMetaData* texMetaData = new TextureMetaData();
        texMetaData->path = path;
        texMetaData->dimension = TextureDimension::TEXTURE2D;
        texMetaData->format = TextureFormat::R8G8B8A8;
        texMetaData->width = 64;
        texMetaData->height = 64;
        return texMetaData;
    }

    ModelMetaData* MetaLoader::LoadModelMetaData(const std::string& path)
    {
        ModelMetaData* metaData = new ModelMetaData();
        metaData->path = path;
        return metaData;
    }


    
    ShaderVariableType MetaLoader::GetShaderVaribleType(uint32_t size)
    {
        // 以byte为准 比如float为4byte
        switch (size)
        {
        case 4:
            return ShaderVariableType::FLOAT; 
            break;
        case 8:
            return ShaderVariableType::VECTOR2;
            break;
        case 12:
            return ShaderVariableType::VECTOR2;
            break;
        case 16:
            return ShaderVariableType::VECTOR2;
            break;
        case 64:
            return ShaderVariableType::VECTOR2;
            break;
        case 48:
            return ShaderVariableType::VECTOR2;
            break;
        case 36:
            return ShaderVariableType::VECTOR2;
            break;
        default:
            break;
        }
    }


}


