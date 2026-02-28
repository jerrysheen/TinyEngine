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
        Loading,
        Loaded,
        Finalized
    };

    enum class LoadPolicy { Sync, Async };

    struct LoadTask
    {
        AssetID id = 0;
        AssetType type = AssetType::Default;
        Resource* resource;
        int pendingDeps = 0;
        std::vector<function<void()>> calllbacks;
        LoadState loadState = LoadState::None;
        bool pendingUnload = false;
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
        ResourceHandle<T> RequestLoad(uint64_t assetPathID,
            std::function<void()> callback,
            LoadTask* parentTask,
            LoadPolicy policy)
        {
            AssetType type = AssetTypeTraits<T>::Type;
            LoadTask* task = GetOrCreateALoadTask(assetPathID, type);

            if (task->loadState == LoadState::Finalized)
            {
                if (callback) callback();
                if (parentTask)
                {
                    TryFinalize(parentTask);
                }
                return ResourceHandle<T>(assetPathID);
            }

            if (callback) task->calllbacks.push_back(callback);
            if (parentTask)
            {
                parentTask->pendingDeps++;
                task->calllbacks.push_back([=]()
                    {
                        parentTask->pendingDeps--;
                        TryFinalize(parentTask);
                    });
            }

            if (task->loadState == LoadState::None)
            {
                task->loadState = LoadState::Queue;
                task->id = assetPathID;
                task->type = type;

                if (policy == LoadPolicy::Async)
                {
                    mLoadTaskQueue.TryPush(task);
                }
                else
                {
                    DoLoadSync(task, policy);
                }
            }

            if (policy == LoadPolicy::Sync)
            {
                WaitForFinalize(task);
            }

            return ResourceHandle<T>(assetPathID);
        }

        void DoLoadSync(LoadTask* task, LoadPolicy policy)
        {
            task->loadState = LoadState::Loading;

            string path = AssetRegistry::GetInstance()->GetAssetPathFromID(task->id);
            LoadResult result = m_Loaders[task->type]->Load(path);
            result.task = task;
            ApplyLoadResult(result, policy);
        }

        void ApplyLoadResult(LoadResult& result, LoadPolicy policy)
        {
            LoadTask* task = result.task;
            ASSERT(result.resource != nullptr);

            task->loadState = LoadState::Loaded;
            task->resource = result.resource;
            for (auto& dep : result.dependencyList)
            {
                switch (dep.type)
                {
                case AssetType::Texture2D:
                    RequestLoad<Texture>(dep.id, dep.onLoaded, task, policy);
                    break;
                case AssetType::Shader:
                    RequestLoad<Shader>(dep.id, dep.onLoaded, task, policy);
                    break;
                default:
                    ASSERT(false);
                    break;
                }
            }
            TryFinalize(task);
        }
        template<typename T>
        ResourceHandle<T> LoadAssetAsync(uint64_t assetPathID, std::function<void()> callback, LoadTask* parentTask)
        {
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            return RequestLoad<T>(assetPathID, callback, parentTask, LoadPolicy::Async);
        }

        template<typename T>
        ResourceHandle<T> LoadAsset(const string& relativePath)
        {
            uint64_t assetPathID = AssetRegistry::GetInstance()->GetAssetIDFromPath(relativePath);
            LoadTask* currTask = GetOrCreateALoadTask(assetPathID, AssetTypeTraits<T>::Type);
            return RequestLoad<T>(assetPathID, nullptr, nullptr, LoadPolicy::Sync);
        }

        void WaitForFinalize(LoadTask* task)
        {
            if (task->loadState != LoadState::Loaded && task->loadState != LoadState::Finalized)
            {
                Update();
                std::this_thread::yield();
            }
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
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (mResourceCountMap.count(id) > 0)
            {
                mResourceCountMap[id] = ++mResourceCountMap[id];
            }
            else 
            {
                mResourceCountMap[id] = 1;
            }
        }

        inline void DecreaseRef(AssetID id)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if(mResourceCountMap.count(id) > 0)
            {
                mResourceCountMap[id] = --mResourceCountMap[id];
                if(mResourceCountMap[id] <= 0)
                {
                    auto* task = mLoadTaskCache[id];
                    if (task && task->loadState != LoadState::Finalized) {
                        task->pendingUnload = true;
                        return;
                    }
                }
            }
        }

        static inline ResourceManager* GetInstance()
        {
            if (sInstance == nullptr)
            {
                Create();
            }
            return sInstance;
        };

        template<typename T>
        inline T* GetResource(AssetID id)
        {
            if (!id.IsValid()) return nullptr;

            if (mResourceCache.count(id) == 0)
            {
                return static_cast<T*>(GetDefaultResource(AssetTypeTraits<T>::Type));
                return nullptr;
            }

            Resource* res = mResourceCache[id];
            if (res == nullptr)
            {
                return static_cast<T*>(GetDefaultResource(AssetTypeTraits<T>::Type));
            }

            return static_cast<T*>(res);
        }

        inline void GabageColloection()
        {
            for (int i = 0; i < mPendingDeleteList.size(); i++)
            {
                delete mPendingDeleteList[i];
            }
            mPendingDeleteList.clear();
        };

        void ResgisterResource(Resource* res, AssetID id)
        {
            if (mResourceCache.count(id) > 0)
            {
                ASSERT(mResourceCache[id] == res);
            }
            mResourceCache[id] = res;
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
        int maxResourceLoadedInMainThread = 50;
        std::vector<Resource*> mPendingDeleteList;
        std::unordered_map<AssetType, IResourceLoader*> m_Loaders;

        ResourceHandle<Material> GetDefaultMaterialHandle();

        std::thread mLoadThread;
        Mesh* defaultMesh = nullptr;
        Texture* mDefaultTexture = nullptr;
        Shader* mDefaultShader = nullptr;
        Material* mDefaultMaterial = nullptr;

        std::mutex m_Mutex;
        std::vector<LoadTask*> freeTaskList;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<AssetID, int> mResourceCountMap;
        unordered_map<AssetID, LoadTask*> mLoadTaskCache;
        // LoadTask 不回收
        ThreadSafeQueue<LoadTask*> mLoadTaskQueue;
        ThreadSafeQueue<LoadResult> mLoadResultQueue;
        LoadTask* GetOrCreateALoadTask(uint64_t assetid, AssetType assetType);
        void TryFinalize(LoadTask* task);
        void EnsureDefaultTexture();
        void EnsureDefaultShader();
        void EnsureDefaultMaterial();
    private:
        void InitDefaultResources();
    };

}
