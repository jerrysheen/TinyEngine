#include "PreCompiledHeader.h"
#include "AssetRegistry.h"
#include "Serialization/StreamHelper.h"
#include "Settings/ProjectSettings.h"
namespace EngineCore
{
    AssetRegistry* AssetRegistry::s_Instacnce = nullptr;

    void AssetRegistry::Create()
    {
        if(!s_Instacnce)
        {
            s_Instacnce = new AssetRegistry();
        }
    }

    void AssetRegistry::Destroy()
    {

    }

    AssetRegistry *AssetRegistry::GetInstance()
    {
        if(!s_Instacnce)
        {
            Create();
        }
        return s_Instacnce;
    }

    void AssetRegistry::RegisterAsset(Resource *resource)
    {
        ASSERT(resource);
        if(resource->GetAssetCreateMethod() == AssetCreateMethod::Runtime) return;
        std::string path = resource->GetPath();
        uint64_t assetID = resource->GetAssetID();
        if(assetPathMap.count(assetID) > 0)
        {
            ASSERT(assetPathMap[assetID] == path);
            return;
        }
        assetPathMap[assetID] = path;
    }

    std::string AssetRegistry::GetAssetPath(uint64_t assetID)
    {
        ASSERT(assetPathMap.count(assetID) > 0);

        return assetPathMap[assetID];
    }

    void AssetRegistry::SaveToDisk(const std::string& relativePath)
    {
        ASSERT(assetPathMap.size() > 0);
        std::string manifestPath = PathSettings::ResolveAssetPath(relativePath);
        std::ofstream out(manifestPath, std::ios::binary);
        uint32_t count = assetPathMap.size();
        StreamHelper::Write(out, count);
        for(auto& [key, value] : assetPathMap)
        {
            StreamHelper::Write(out, key);
            StreamHelper::WriteString(out, value);
        }
    }

    void AssetRegistry::LoadFromDisk(const std::string& relativePath)
    {
        std::string manifestPath = PathSettings::ResolveAssetPath(relativePath);
        std::ifstream in(manifestPath, std::ios::binary);
        if (!in.is_open()) {
            // 严重错误：找不到账本，游戏无法运行
            ASSERT(false, "Failed to load Asset Manifest!");
            return;
        }

        uint32_t count = 0;
        StreamHelper::Read(in, count);
        ASSERT(count != 0);
        for(int i = 0; i < count; i++)
        {
            uint64_t id;
            std::string path;
            
            StreamHelper::Read(in, id);
            StreamHelper::ReadString(in, path);
            assetPathMap[id] = path;
        }
    }
}