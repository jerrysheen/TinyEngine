#pragma once

namespace EngineCore
{
    class Mesh;
    class Texture;
    class Material;

    template<typename T> struct AssetTypeTraits { static const AssetType Type = AssetType::Default; };

    // 特化模板
    template<> struct AssetTypeTraits<Mesh> { static const AssetType Type = AssetType::Mesh; };
    template<> struct AssetTypeTraits<Texture> { static const AssetType Type = AssetType::Texture2D; };
    template<> struct AssetTypeTraits<Material> { static const AssetType Type = AssetType::Material; };
}