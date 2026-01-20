#pragma once
#include "PreCompiledHeader.h"
#include "Core/Object.h"
#include "Utils/HashCombine.h"


namespace EngineCore
{

    //todo： 应该是一个永久的hash，保证每次的资源能对应上
    struct AssetID
    {
        uint64_t value = 0;
        explicit operator bool() const {return value != 0;};
        operator uint32_t() const { return value;}
        bool IsValid() const {return value != 0;};
        inline void Reset() { value = 0; };
        AssetID() = default;
        AssetID(uint64_t value) :value(value) {};
        // 添加相等比较运算符
        bool operator==(const AssetID& other) const {
            // 假设 AssetID 有一个 id 成员变量
            return this->value == other.value;
        }

        bool operator!=(const AssetID& other) const {
            return !(*this == other);
        }
    };

    class AssetIDGenerator
    {
    public:
        static AssetID NewFromFile(const string& path)
        {
            uint64_t hashID = GetStringHash(path);
            return AssetID{hashID};
        };

        static AssetID NewFromInstanceID(uint32_t InstanceID)
        {
            std::string path = "Rumtime://" + std::to_string(InstanceID);
            return AssetID{GetStringHash(path)};
        }
    };

    enum class AssetType
    {
        Default = 0,
        Texture2D = 1,
        Mesh = 2,
        Material = 3,
        SkinMesh = 4,
        Animation = 5,
        Shader = 6,
        Runtime = 7,
        Scene = 8
    };

    enum class AssetCreateMethod
    {
        Serialization = 0,
        Runtime = 1
    };
}

namespace std {
    template<>
    struct hash<EngineCore::AssetID> {
        size_t operator()(const EngineCore::AssetID& a) const noexcept {
            return hash<uint64_t>{}(a.value);
        }
    };
}