#include "PreCompiledHeader.h"
#include "Core/Resources.h"
#include "Resources.h"

namespace EngineCore
{

    MaterialStruct* Resources::LoadMaterialMeta(const string& path)
    {
        // todo: 解析Matea数据；
        MaterialStruct* matStruct = new MaterialStruct();
        matStruct->name = "unlit";
        matStruct->path = "fakepath";
        matStruct->shaderCode = "...";
        matStruct->vec3Data.emplace("LightDirection", Vector3(0.3,0.4,0.5));
        matStruct->vec3Data.emplace("LightColor", Vector3(0.3,0.4,0.5));
        matStruct->vec3Data.emplace("CameraPosition", Vector3(0.3,0.4,0.5));
        matStruct->matrix4x4Data.emplace("WorldMatrix", Matrix4x4());
        matStruct->matrix4x4Data.emplace("VPMatrix", Matrix4x4( 0.52,0,-0.37,0, 
                                                                0.4,1.58,0.57,-0.0,
                                                                0,0,-0.003,0.29,
                                                                0.53,-0.40,0.74,4.5));
        //matStruct->matrix4x4Data.emplace("ProjectionMatrix", Matrix4x4());
        return matStruct;
    }

    TextureStruct* Resources::LoadTextureMeta(const std::string &path)
    {
        TextureStruct* texMetaData = new TextureStruct();
        texMetaData->name = "Material";
        texMetaData->path = "D:/GitHubST/TinyEngine/Assets/Textures/material.png";
        texMetaData->dimension = TextureDimension::TEXTURE2D;
        texMetaData->format = TextureFormat::R8G8B8A8;
        texMetaData->width = 64;
        texMetaData->height = 64;
        return texMetaData;
    }


    
    ShaderVariableType Resources::GetShaderVaribleType(uint32_t size)
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


