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
        std::string GetAssetPathFromID(uint64_t id);
        uint64_t GetAssetIDFromPath(const std::string& path);
        void SaveToDisk(const std::string& manifestPath);
        void LoadFromDisk(const std::string& manifestPath);
    private:
        static AssetRegistry* s_Instacnce;
        std::unordered_map<uint64_t, std::string> assetIDToPathMap;
        std::unordered_map<std::string, uint64_t> pathToAssetIDMap;

        std::mutex  m_mutex;;
    };
}