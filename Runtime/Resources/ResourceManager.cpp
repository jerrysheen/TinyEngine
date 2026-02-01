#include "PreCompiledHeader.h"
#include "ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Resources/IResourceLoader.h"
#include "Serialization/MeshLoader.h"
#include "Serialization/SceneLoader.h"
#include "Serialization/MaterialLoader.h"
#include "Serialization/ShaderLoader.h"
#include "Serialization/DDSTextureLoader.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

namespace EngineCore
{
    ResourceManager* ResourceManager::sInstance = nullptr;

    ResourceManager::ResourceManager()
        :mLoadThread(&ResourceManager::WorkThreadLoad, this)
    {
        m_Loaders[AssetType::Mesh] = new MeshLoader();
        m_Loaders[AssetType::Scene] = new SceneLoader();
        m_Loaders[AssetType::Texture2D] = new DDSTextureLoader();
        m_Loaders[AssetType::Material] = new MaterialLoader();
        m_Loaders[AssetType::Shader] = new ShaderLoader();
        defaultMesh = new Mesh(Primitive::Cube);
        EnsureDefaultShader();
        EnsureDefaultTexture();
    }

    void ResourceManager::Update()
    {
        while(mLoadResultQueue.size() > 0)
        {
            LoadResult loadResult;
            bool hasWork = mLoadResultQueue.TryPop(loadResult);
            if(hasWork)
            {
                ASSERT(loadResult.resource != nullptr);
                LoadTask* task = loadResult.task;
                loadResult.resource->mRefCount = mResourceCache[loadResult.task->id]->mRefCount;
                mResourceCache[loadResult.task->id] = loadResult.resource;
                task->loadState = LoadState::Loaded;
                task->resource = loadResult.resource;
                for(auto& dependency : loadResult.dependencyList)
                {
                    switch (dependency.type) 
                    {
                    case AssetType::Texture2D:
                        ResourceManager::GetInstance()->LoadAssetAsync<Texture>(dependency.id, dependency.onLoaded, loadResult.task);
                        break;
                    case AssetType::Shader:
                        ResourceManager::GetInstance()->LoadAssetAsync<Shader>(dependency.id, dependency.onLoaded, loadResult.task);
                        break;
                    default:
                        ASSERT(false);
                    }
                }
                TryFinalize(task);
            };
        }
    }

    void ResourceManager::WorkThreadLoad()
    {
        LoadTask* work;
        while(mLoadTaskQueue.WaitAndPop(work))
        {
            if(work == nullptr) continue;
            ASSERT(m_Loaders.count(work->type) > 0);
            string path = AssetRegistry::GetInstance()->GetAssetPathFromID(work->id);
            LoadResult result = m_Loaders[work->type]->Load(path);
            result.task = work;
            mLoadResultQueue.TryPush(result);
        }
    }

    Resource* ResourceManager::GetDefaultResource(AssetType fileType)
    {
        ASSERT(fileType != AssetType::Default);
        AssetID id;
        switch (fileType)
        {
        case AssetType::Mesh:
            return defaultMesh;
            break;
        case AssetType::Texture2D:
            return mDefaultTexture;
            break;
        case AssetType::Shader:
            return mDefaultShader;
            break;
        case AssetType::Material:
            EnsureDefaultMaterial();
            return mDefaultMaterial.IsValid() ? mDefaultMaterial.Get() : nullptr;
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    LoadTask *ResourceManager::GetOrCreateALoadTask(uint64_t assetid, AssetType assetType)
    {
        AssetID id(assetid);
        if(mLoadTaskCache.count(id) > 0) return mLoadTaskCache[id];

        if(freeTaskList.size() > 0)
        {
            LoadTask* task = freeTaskList.back();
            task->Reset();
            mLoadTaskCache[id] = task;
            freeTaskList.pop_back();
            return task;
        }

        LoadTask* task = new LoadTask();
        task->type = assetType;
        mLoadTaskCache[id] = task;
        return task;
    }

    void ResourceManager::TryFinalize(LoadTask *task)
    {
        if(task->resource == nullptr) return;
        if(task->loadState != LoadState::Loaded) return;
        if(task->pendingDeps > 0) return;

        for(auto& callback : task->calllbacks)
        {
            callback();
        }
        task->resource->OnLoadComplete();
        task->loadState = LoadState::Finalized;
    }

    ResourceManager::~ResourceManager()
    {
        mLoadTaskQueue.Stop();
        if(mLoadThread.joinable())
        {
            mLoadThread.join();
        }

        for(auto [key, value] : mLoadTaskCache)
        {
            delete value;
        }

        for(auto [key, value] : mResourceCache)
        {
            delete value;
        }

        mLoadTaskCache.clear();
        mResourceCache.clear();

        delete m_Loaders[AssetType::Mesh];
        delete m_Loaders[AssetType::Scene];
        delete m_Loaders[AssetType::Texture2D];
        delete m_Loaders[AssetType::Material];
        delete m_Loaders[AssetType::Shader];
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

    void ResourceManager::EnsureDefaultTexture()
    {
        if (mDefaultTexture != nullptr) return;
        const char* defaultTexturePath = "Textures/DefaultWhite.dds";
        Texture temp;
        temp.SetAssetCreateMethod(AssetCreateMethod::Serialization);
        temp.SetPath(defaultTexturePath);
        temp.SetAssetID(AssetIDGenerator::NewFromFile(defaultTexturePath));
        AssetRegistry::GetInstance()->RegisterAsset(&temp);

        mDefaultTexture = static_cast<Texture*>(m_Loaders[AssetType::Texture2D]->Load(defaultTexturePath).resource);
        mDefaultTexture->OnLoadComplete();
        mResourceCache[temp.GetAssetID()] = static_cast<Resource*>(mDefaultTexture);
    }

    void ResourceManager::EnsureDefaultShader()
    {

        if (mDefaultShader != nullptr) return;

        const char* defaultShaderPath = RenderSettings::s_EnableVertexPulling
            ? "Shader/StandardPBR_VertexPulling.hlsl"
            : "Shader/StandardPBR.hlsl";

        Shader temp;
        temp.SetAssetCreateMethod(AssetCreateMethod::Serialization);
        temp.SetPath(defaultShaderPath);
        temp.SetAssetID(AssetIDGenerator::NewFromFile(defaultShaderPath));
        AssetRegistry::GetInstance()->RegisterAsset(&temp);

        mDefaultShader = static_cast<Shader*>(m_Loaders[AssetType::Shader]->Load(defaultShaderPath).resource);
        mDefaultShader->OnLoadComplete();
        mResourceCache[temp.GetAssetID()] = static_cast<Resource*>(mDefaultShader);
    }

    void ResourceManager::EnsureDefaultMaterial()
    {
        if (mDefaultMaterial.IsValid())
        {
            return;
        }

        EnsureDefaultTexture();

        const char* defaultMaterialPath = "Material/Default.mat";
        Material temp;
        temp.SetAssetCreateMethod(AssetCreateMethod::Serialization);
        temp.SetPath(defaultMaterialPath);
        temp.SetAssetID(AssetIDGenerator::NewFromFile(defaultMaterialPath));
        AssetRegistry::GetInstance()->RegisterAsset(&temp);

        mDefaultMaterial = LoadAsset<Material>(defaultMaterialPath);
    }


}
