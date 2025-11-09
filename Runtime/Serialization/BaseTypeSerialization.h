#pragma once
#include "json.hpp"
#include "Math/Math.h"
#include "Core/PublicEnum.h"
#include "Resources/Asset.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceHandle.h"
#include "Resources/ResourceManager.h"
namespace EngineCore
{
    // // Vector3
    using json = nlohmann::json;
    inline void to_json(json& j, const Vector3& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
    }

    inline void from_json(const json& j, EngineCore::Vector3& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
    }

    //vector2
    inline void to_json(json& j, const EngineCore::Vector2& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}};
    }

    inline void from_json(const json& j, EngineCore::Vector2& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
    }

    // Matrix4x4
    inline void to_json(json& j, const EngineCore::Matrix4x4& m)
    {
        j = json{
            {"m00", m.m00}, {"m01", m.m01}, {"m02", m.m02},{"m03", m.m03},
            {"m10", m.m10}, {"m11", m.m11}, {"m12", m.m12}, {"m13", m.m13},
            {"m20", m.m20}, {"m21", m.m21}, {"m22", m.m22}, {"m23", m.m23},
            {"m30", m.m30}, {"m31", m.m31}, {"m32", m.m32}, {"m33", m.m33}
        };
    }

    inline void from_json(const json& j, EngineCore::Matrix4x4& m)
    {
        j.at("m00").get_to(m.m00); j.at("m01").get_to(m.m01); j.at("m02").get_to(m.m02); j.at("m03").get_to(m.m03);
        j.at("m10").get_to(m.m10); j.at("m11").get_to(m.m11); j.at("m12").get_to(m.m12); j.at("m13").get_to(m.m13);
        j.at("m20").get_to(m.m20); j.at("m21").get_to(m.m21); j.at("m22").get_to(m.m22); j.at("m23").get_to(m.m23);
        j.at("m30").get_to(m.m30); j.at("m31").get_to(m.m31); j.at("m32").get_to(m.m32); j.at("m33").get_to(m.m33);
    }

    // Quaternion
    inline void to_json(json& j, const EngineCore::Quaternion& m)
    {
        j = json{
            {"x", m.x},
            {"y", m.y},
            {"z", m.z},
            {"w", m.w},
        };
    }

    inline void from_json(const json& j, EngineCore::Quaternion& m)
    {
        j.at("x").get_to(m.x);
        j.at("y").get_to(m.y);
        j.at("z").get_to(m.z);
        j.at("w").get_to(m.w);
    }


    // todo:
    // tempData for texture
    inline void to_json(json& j, const EngineCore::Texture& v)
    {
        j = nullptr;  
    }

    inline void from_json(const json& j, EngineCore::Texture& v)
    {
    }

    //texture
    NLOHMANN_JSON_SERIALIZE_ENUM(EngineCore::AssetType, {
        {EngineCore::AssetType::Default, "Default"},
        {EngineCore::AssetType::Texture2D, "Texture2D"},
        {EngineCore::AssetType::Mesh, "Mesh"},
        {EngineCore::AssetType::Material, "Material"},
        {EngineCore::AssetType::SkinMesh, "SkinMesh"},
        {EngineCore::AssetType::Animation, "Animation"},
        {EngineCore::AssetType::Shader, "Shader"},
        {EngineCore::AssetType::Runtime, "Runtime"}
    })

    //texture
    NLOHMANN_JSON_SERIALIZE_ENUM(EngineCore::TextureDimension, {
        {EngineCore::TextureDimension::TEXTURE2D, "Texture2D"},
        {EngineCore::TextureDimension::TEXTURE3D, "Texture3D"},
        {EngineCore::TextureDimension::TEXTURECUBE, "TextureCube"},
        {EngineCore::TextureDimension::TEXTURE2DARRAY, "Texture2DArray"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(EngineCore::TextureFormat, {
        {EngineCore::TextureFormat::R8G8B8A8, "Texture2D"},
        {EngineCore::TextureFormat::D24S8, "Texture3D"},
        {EngineCore::TextureFormat::EMPTY, "TextureCube"},
    })

    // Asset ID：
    inline void to_json(json& j, const AssetID& v)
    {
        j = json{
           {"ID", v.value}
        };
    }

    inline void from_json(const json& j, AssetID& assetID)
    {
        j.at("ID").get_to(assetID.value);
    }


    // ResourceHandle：   
    template<typename T>
    inline void to_json(json& j, const ResourceHandle<T>& v)
    {
        // 空地址怎么处理？ 空地址表示的是程序生成的资源
        string path = v.Get()->GetPath();
        j = json{
           {"ID", v.GetAssetID()},
           {"Path", path}
        };
    }

    template<typename T>
    inline void from_json(const json& j, ResourceHandle<T>& handle)
    {
        // 空地址，程序生成的资源，等运行起来会有别的地方进行创建？这里只需要填id即可
        string path = j.at("Path");
        if(path != "")
        {
            handle = ResourceManager::GetInstance()->LoadAsset<T>(path);
        }
        else
        {
            j.at("ID").get_to(handle.mAssetID);
        }
    }

}
