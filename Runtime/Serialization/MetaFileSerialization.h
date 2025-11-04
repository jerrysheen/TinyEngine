#pragma once
#include "json.hpp"
#include "BaseTypeAndEnumSerialization.h"
#include "Serialization/MetaData.h"


namespace EngineCore
{
    // Meta
    inline void to_json(json& j, const EngineCore::MetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
            };
    }

    inline void from_json(const json& j, EngineCore::MetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
    }


    // TextureMetaData
    inline void to_json(json& j, const EngineCore::TextureMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"Dimension", v.dimension}, 
                {"Format", v.format},
                {"Width", v.width},
                {"Height", v.height}
            };
    }

    inline void from_json(const json& j, EngineCore::TextureMetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
        j.at("Dimension").get_to(v.dimension);
        j.at("Format").get_to(v.format);
        j.at("Width").get_to(v.width);
        j.at("Height").get_to(v.height);
    }

    // MaterialMetaData
    inline void to_json(json& j, const EngineCore::MaterialMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"FloatData", v.floatData}, 
                {"Vec2Data", v.vec2Data},
                {"Vec3Data", v.vec3Data},
                {"Matrix4x4Data", v.matrix4x4Data}
            };
    }

    inline void from_json(const json& j, EngineCore::MaterialMetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
        j.at("FloatData").get_to(v.floatData);
        j.at("Vec2Data").get_to(v.vec2Data);
        j.at("Vec3Data").get_to(v.vec3Data);
        j.at("Matrix4x4Data").get_to(v.matrix4x4Data);
    }
}
