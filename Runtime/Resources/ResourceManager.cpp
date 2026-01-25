#include "PreCompiledHeader.h"
#include "ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Serialization/MeshLoader.h"
#include "Serialization/SceneLoader.h"
#include "Serialization/DDSTextureLoader.h"

namespace EngineCore
{
    ResourceManager* ResourceManager::sInstance = nullptr;

    ResourceManager::ResourceManager()
        :mLoadThread(&ResourceManager::WorkThreadLoad, this)
    {
        m_Loaders[AssetType::Mesh] = new MeshLoader();
        m_Loaders[AssetType::Scene] = new SceneLoader();
        m_Loaders[AssetType::Texture2D] = new DDSTextureLoader();
        defaultMesh = new Mesh(Primitive::Cube);
        isRuning = true;
    }

    void ResourceManager::Update()
    {
        while(m_MainThreadQueue.size() > 0)
        {
            std::function<void()> lambda;
            bool hasWork = m_MainThreadQueue.TryPop(lambda);
            if(hasWork)lambda();
        }
    }

    void ResourceManager::WorkThreadLoad()
    {
        while(isRuning || m_WorkThreadQueue.size() > 0)
        {
            std::function<void()> work;
            m_WorkThreadQueue.WaitAndPop(work);
            work();
        }
    }

    ResourceManager::~ResourceManager()
    {
        isRuning = false;
        delete m_Loaders[AssetType::Mesh];
        delete m_Loaders[AssetType::Scene];
        delete m_Loaders[AssetType::Texture2D];
    }

    void ResourceManager::Create()
    {
        if (sInstance) return;
        sInstance = new ResourceManager();
    }


    void ResourceManager::Destroy()
    {
        ASSERT(sInstance != nullptr);  
        delete defaultMesh;
        delete sInstance;
        sInstance = nullptr;
    }


}