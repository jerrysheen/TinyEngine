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
        std::lock_guard<std::mutex> lock(m_mutex);
        ASSERT(resource);
        if(resource->GetAssetCreateMethod() == AssetCreateMethod::Runtime) return;
        std::string path = resource->GetPath();
        uint64_t assetID = resource->GetAssetID();
        if(assetIDToPathMap.count(assetID) > 0)
        {
            ASSERT(assetIDToPathMap[assetID] == path);
            return;
        }
        assetIDToPathMap[assetID] = path;
        pathToAssetIDMap[path] = assetID;
    }

    std::string AssetRegistry::GetAssetPathFromID(uint64_t assetID)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ASSERT(assetIDToPathMap.count(assetID) > 0);

        return assetIDToPathMap[assetID];
    }

    uint64_t AssetRegistry::GetAssetIDFromPath(const std::string &path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ASSERT(pathToAssetIDMap.count(path) > 0);

        return pathToAssetIDMap[path];
    }

    void AssetRegistry::SaveToDisk(const std::string& relativePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ASSERT(assetIDToPathMap.size() > 0);
        std::string manifestPath = PathSettings::ResolveAssetPath(relativePath);
        std::ofstream out(manifestPath, std::ios::binary);
        uint32_t count = assetIDToPathMap.size();
        StreamHelper::Write(out, count);
        for(auto& [key, value] : assetIDToPathMap)
        {
            StreamHelper::Write(out, key);
            StreamHelper::WriteString(out, value);
        }
    }

    void AssetRegistry::LoadFromDisk(const std::string& relativePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string manifestPath = PathSettings::ResolveAssetPath(relativePath);
        std::ifstream in(manifestPath, std::ios::binary);
        if (!in.is_open()) {
            // 严重错误：找不到账本，游戏无法运行
            ASSERT(false, "Failed to load Asset Manifest!");
            return;
        }

        uint32_t count = 0;
        StreamHelper::Read(in, count);
        //ASSERT(count != 0);
        for(int i = 0; i < count; i++)
        {
            uint64_t id;
            std::string path;
            
            StreamHelper::Read(in, id);
            StreamHelper::ReadString(in, path);
            assetIDToPathMap[id] = path;
            pathToAssetIDMap[path] = id;
        }
    }
}