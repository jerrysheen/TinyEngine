#pragma once
#include "Settings/ProjectSettings.h"
#include "unordered_map"
#include "Resource.h"

namespace EngineCore
{
    class AssetRegistry
    {
    public:
        static void Create();
        static void Destroy();
        static AssetRegistry* GetInstance();
        void RegisterAsset(Resource* resource);
        std::string GetAssetPath(uint64_t id);
        void SaveToDisk(const std::string& manifestPath);
        void LoadFromDisk(const std::string& manifestPath);
    private:
        static AssetRegistry* s_Instacnce;
        std::unordered_map<uint64_t, std::string> assetPathMap;
    };
}