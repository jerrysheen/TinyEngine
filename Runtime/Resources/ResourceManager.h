#pragma once
#include "Asset.h"
#include "PreCompiledHeader.h"
#include "ResourceHandle.h"
#include "Serialization/MetaData.h"
#include "Serialization/MetaLoader.h"
#include "Resources/Resource.h"
#include "Settings/ProjectSettings.h"
#include "IResourceLoader.h"
#include "AssetRegistry.h"
#include "AssetTypeTraits.h"
#include "Core/ThreadSafeQueue.h"

namespace EngineCore
{
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
        ResourceHandle<T> LoadAssetAsync(uint64_t assetPathID, std::function<void()> callback)
        {
            if(mResourceCache.count(assetPathID) > 0)
            {
                return ResourceHandle<T>(assetPathID);
            }
            
            mResourceCache[assetPathID] = nullptr;
            string path = AssetRegistry::GetInstance()->GetAssetPath(assetPathID);
            // 找到对应的Loader：
            AssetType fileType = AssetTypeTraits<T>::Type;
            ResourceHandle<T> handle(assetPathID);
            auto* loader = m_Loaders[fileType];

            m_WorkThreadQueue.TryPush([=]()
            {
                Resource* resource = loader->Load(path);
                m_MainThreadQueue.TryPush([=]()
                {
                    // 主线程访问才对，不然会有线程安全问题
                    mResourceCache[assetPathID] = resource;
                    resource->OnLoadComplete();
                    if(callback != nullptr)callback();
                });
            });

            return handle;
        }

        ResourceState GetResourceStateByID(uint64_t assetID )
        {
            if(mResourceCache.count(assetID) == 0) return ResourceState::NotExits;
            if(mResourceCache[assetID] == nullptr) return ResourceState::Loading;
            if(mResourceCache[assetID] != nullptr) return ResourceState::Success;
        }

        template<typename T>
        ResourceHandle<T> LoadAsset(const string& relativePath)
        {
            string path = PathSettings::ResolveAssetPath(relativePath);
            // 根据path判断是否加载， 加载了就存ID和path:
            if(mPathToID.count(path) > 0)
            {
                AssetID& id = mPathToID[path];
                return ResourceHandle<T>(id);
            }

            // 加载meta文件，指向一个具体的MetaData文件
            // todo： 循环检测
            // todo: 还没做依赖， prefab的时候做
            MetaData* metaData = MetaLoader::LoadMetaData<T>(path);
            
            // 没必要在这个地方做dependency的加载，而是在具体加载的时候，这样子的话可以知道
            // 具体的dependency类型
            
            // 创建资源的时候生成对应的AssetID
            Resource* resource = new T(metaData);
            ASSERT(resource != nullptr);
            
            AssetID id = resource->GetAssetID();
            mPathToID[path] = id;
            mResourceCache[id] = resource;
            delete metaData;
            return ResourceHandle<T>(id);
        }

        template<typename T>
        ResourceHandle<T> Instantiate(const ResourceHandle<T>& sourceHandle)
        {
            T* src = sourceHandle.Get();
            ASSERT(src != nullptr);

            Resource* resource = new T(*src);
            ASSERT(resource != nullptr);
            // 将metaData设置填入Resource中
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
            if(mResourceCache.count(id) > 0)
            {
                auto* resource  = mResourceCache[id];
                if (resource == nullptr) return;
                if(--resource->mRefCount <= 0)
                {
                    mPendingDeleteList.push_back(resource);
                    // 可能有不生成PathID的运行时资源
                    if(mPathToID.count(resource->GetPath()) > 0)
                    {
                        mPathToID.erase(resource->GetPath());
                    }
                    // erase之后， reource直接被析构了，所以上面全是nullptr了。。
                    mResourceCache.erase(id);
                }
            }
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
                return nullptr;
            }

            Resource* res = mResourceCache[id];
            if(res == nullptr)
            {
                return static_cast<T*>(GetDefaultResource<T>());
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
            // 将metaData设置填入Resource中
            AssetID id = resource->GetAssetID();
            mResourceCache[id] = resource;
            return ResourceHandle<T>(id);
        }

        template<typename T>
        Resource* GetDefaultResource()
        {
            auto fileType = AssetTypeTraits<T>::Type;
            ASSERT(fileType != AssetType::Default);
            AssetID id;
            switch (fileType)
            {
            case AssetType::Mesh:
                return defaultMesh;
                break;
            default:
                ASSERT(false);
                break;
            }
        }

    // todo: temp public for Editor Test
    public:
        void LoadDependencies(const std::vector<MetaData>& dependenciesList);
        static ResourceManager* sInstance;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<string, AssetID> mPathToID;
        std::vector<Resource*> mPendingDeleteList;

        std::unordered_map<AssetType, IResourceLoader*> m_Loaders;

        ThreadSafeQueue<std::function<void()>> m_WorkThreadQueue;
        ThreadSafeQueue<std::function<void()>> m_MainThreadQueue;
        std::thread mLoadThread;
        bool isRuning = true;
        Mesh* defaultMesh;

    };

}