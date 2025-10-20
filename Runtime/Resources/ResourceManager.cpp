#include "PreCompiledHeader.h"
#include "ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/ModelData.h"
#include "Graphics/Shader.h"

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