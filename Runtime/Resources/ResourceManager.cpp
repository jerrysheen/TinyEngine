#include "PreCompiledHeader.h"
#include "ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
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
    
    void ResourceManager::RegisterMaterial(const Material *mat)
    {
    //        mAllMaterialData.push_back(mat);
    }
    
    void ResourceManager::UnRegisterMaterial(const Material *mat)
    {
        //for(int i = 0; i < mAllMaterialData.size(); i++)
        //{
        //    if(mat == mAllMaterialData[i])
        //    {
        //        mAllMaterialData[i] = nullptr;
        //        return;
        //    }
        //}
    }
}