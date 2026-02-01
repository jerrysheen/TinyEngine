#pragma once
#include "Asset.h"
#include "PreCompiledHeader.h"
#include "ResourceHandle.h"
#include "Resources/Resource.h"
#include "Settings/ProjectSettings.h"
#include "AssetRegistry.h"
#include "AssetTypeTraits.h"
#include "Core/ThreadSafeQueue.h"
#include "Resources/IResourceLoader.h"


namespace EngineCore
{

    enum class LoadState
    {
        None,
        Queue,
        Loaded,
        Finalized
    };

    struct LoadTask
    {
        AssetID id = 0;
        AssetType type = AssetType::Default;
        Resource* resource;
        int pendingDeps = 0;
        std::vector<function<void()>> calllbacks;
        LoadState loadState = LoadState::None;
        void Reset()
        {
            id = 0;
            type = AssetType::Default;
            resource = nullptr;
            pendingDeps = 0;
            calllbacks.clear();
            loadState = LoadState::None;
        }
    };

    class ResourceManager
    {
    public:
        
        static void Create();
        void Destroy();
        ~ResourceManager();
        ResourceManager();

        void Update();
        void WorkThreadLoad();

        template<typename T>
        ResourceHandle<T> LoadAssetAsync(uint64_t assetPathID, std::function<void()> callback, LoadTask* parentTask)
        {
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            // LoadAssetAsync只负责发起异步加载任务，不关心任何比如资源加载完怎么处理
            if(currTask->loadState == LoadState::Finalized)
            {
                if(callback) callback();
                // 不用TryFinalize Parent，TryFinalize已经在父节点LoadResult的时候调用过了。
                return ResourceHandle<T>(assetPathID);
            }

            if(callback != nullptr)
            {
                currTask->calllbacks.push_back(callback);
            }
            if(parentTask != nullptr)
            {
                parentTask->pendingDeps++;
                currTask->calllbacks.push_back([=]()
                    {
                        parentTask->pendingDeps--;
                        TryFinalize(parentTask);
                    });
            }

            if(currTask->loadState == LoadState::None)
            {
                currTask->loadState = LoadState::Queue;
                currTask->id = assetPathID;
                currTask->type = AssetTypeTraits<T>::Type;
                mLoadTaskQueue.TryPush(currTask);
                mResourceCache[assetPathID] = GetDefaultResource(currTask->type);
            }

            return  ResourceHandle<T>(assetPathID);
        }

        ResourceState GetResourceStateByID(uint64_t assetID )
        {
            if(mResourceCache.count(assetID) == 0) return ResourceState::NotExits;
            if(mResourceCache[assetID] == nullptr) return ResourceState::Loading;
            if(mResourceCache[assetID] != nullptr) return ResourceState::Success;
        }

        // todo 异步加载和同步加载可能会冲突
        // 感觉mResourceCache赋值和取值的一瞬间应该加锁？
        // 这个地方应该是有问题， task = 不是none 的状态， 这个地方会和Async冲突
        template<typename T>
        ResourceHandle<T> LoadAsset(const string& relativePath)
        {
            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(relativePath);
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            // LoadAssetAsync只负责发起异步加载任务，不关心任何比如资源加载完怎么处理

            if (currTask->loadState == LoadState::None)
            {
                AssetType type = AssetTypeTraits<T>::Type;
                ASSERT(m_Loaders.count(type) > 0);
                string path = AssetRegistry::GetInstance()->GetAssetPathFromID(assetPathID);
                LoadResult result = m_Loaders[type]->Load(path);
                mResourceCache[assetPathID] = result.resource;
                mLoadTaskCache[assetPathID] = currTask;
                currTask->loadState = LoadState::Loaded;
                currTask->resource = result.resource;
                for(auto& dependency : result.dependencyList)
                {
                    string dependencyPath = AssetRegistry::GetInstance()->GetAssetPathFromID(dependency.id);
                    switch (dependency.type)
                    {
                    case AssetType::Texture2D:
                        LoadAsset<Texture>(dependencyPath);
                        break;
                    case AssetType::Shader:
                        LoadAsset<Shader>(dependencyPath);
                        break;
                    default:
                        ASSERT(false);
                        break;
                    }

                    if(dependency.onLoaded)
                    {
                        dependency.onLoaded();
                    }
                }
                TryFinalize(currTask);
            }
            return  ResourceHandle<T>(assetPathID);
        }


        template<typename T>
        ResourceHandle<T> Instantiate(const ResourceHandle<T>& sourceHandle)
        {
            T* src = sourceHandle.Get();
            ASSERT(src != nullptr);

            Resource* resource = new T(*src);
            ASSERT(resource != nullptr);
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

        inline void AddRef(AssetID id)
        {
            if(mResourceCache.count(id) > 0)
            {
                auto* resource  = mResourceCache[id];
                if (resource == nullptr) return;
                resource->mRefCount++;
            }
        }

        inline void DecreaseRef(AssetID id)
        {
            //if(mResourceCache.count(id) > 0)
            //{
            //    auto* resource  = mResourceCache[id];
            //    if (resource == nullptr) return;
            //    if(--resource->mRefCount <= 0)
            //    {
            //        mPendingDeleteList.push_back(resource);
            //        mResourceCache.erase(id);
            //        freeTaskList.push_back(mLoadTaskCache[id]);
            //        mLoadTaskCache.erase(id);
            //    }
            //}
        }

        static inline ResourceManager* GetInstance()
        {
            if(sInstance == nullptr)
            {
                Create();
            }
            return sInstance;
        };

        template<typename T>
        inline T* GetResource(AssetID id) 
        {
            if(!id.IsValid()) return nullptr;

            if(mResourceCache.count(id) == 0)
            {
                return static_cast<T*>(GetDefaultResource(AssetTypeTraits<T>::Type));
                return nullptr;
            }

            Resource* res = mResourceCache[id];
            if(res == nullptr)
            {
                return static_cast<T*>(GetDefaultResource(AssetTypeTraits<T>::Type));
            }
            
            return static_cast<T*>(res);
        }

        inline void GabageColloection()
        {
            for(int i = 0; i < mPendingDeleteList.size(); i++)
            {
                delete mPendingDeleteList[i];
            }
            mPendingDeleteList.clear();
        };

        void ResgisterResource(Resource* res, AssetID id)
        {
            if(mResourceCache.count(id) > 0)
            {
                ASSERT(mResourceCache[id] == res);
            }
            mResourceCache[id] == res;
        };

        template<typename T, typename... Args>
        ResourceHandle<T> CreateResource(Args&&... args)
        {
            // 创建资源的时候无复用关系 应该是直接创建
            Resource* resource = new T(std::forward<Args>(args)...);
            ASSERT(resource != nullptr);
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

        Resource* GetDefaultResource(AssetType fileType);

    // todo: temp public for Editor Test
    public:
        static ResourceManager* sInstance;

        std::vector<Resource*> mPendingDeleteList;
        std::unordered_map<AssetType, IResourceLoader*> m_Loaders;
        

        std::thread mLoadThread;
        Mesh* defaultMesh = nullptr;
        Texture* mDefaultTexture = nullptr;
        Shader* mDefaultShader = nullptr;
        ResourceHandle<Material> mDefaultMaterial;
        
        std::vector<LoadTask*> freeTaskList;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<AssetID, LoadTask*> mLoadTaskCache;
        // LoadTask 不回收
        ThreadSafeQueue<LoadTask*> mLoadTaskQueue;
        ThreadSafeQueue<LoadResult> mLoadResultQueue;
        LoadTask* GetOrCreateALoadTask(uint64_t assetid, AssetType assetType);
        void TryFinalize(LoadTask* task);
        void EnsureDefaultTexture();
        void EnsureDefaultShader();
        void EnsureDefaultMaterial();

    };

}
