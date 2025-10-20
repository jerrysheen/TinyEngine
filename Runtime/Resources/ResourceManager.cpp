#include "PreCompiledHeader.h"
#include "ResourceManager.h"

namespace EngineCore
{
    ResourceManager* ResourceManager::sInstance = nullptr;

    
    void ResourceManager::Create()
    {
        ASSERT(sInstance == nullptr);
        sInstance = new ResourceManager();
    }


    void ResourceManager::Delete()
    {
        ASSERT(sInstance != nullptr);  
        delete sInstance;
        sInstance = nullptr;
    }
}