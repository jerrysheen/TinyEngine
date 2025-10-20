#pragma once
#include "Asset.h"
#include "PreCompiledHeader.h"
#include "ResourceHandle.h"
#include "Asset.h"
#include "MetaFile.h"
#include "Graphics/Texture.h"
#include "Graphics/ModelData.h"
#include "Graphics/Shader.h"

namespace EngineCore
{
    class ResourceManager
    {
    public:
        
        static void Create();
        static void Delete();
        ~ResourceManager(){};
        ResourceManager(){};

        template<typename T>
        ResourceHandle<T> LoadAsset(const string& path)
        {
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

        inline void AddRef(AssetID id)
        {
            if(mResourceCache.count(id) > 0)
            {
                auto& resource  = mResourceCache[id];
                resource->mRefCount++;
            }
        }

        inline void DecreaseRef(AssetID id)
        {
            if(mResourceCache.count(id) > 0)
            {
                auto& resource  = mResourceCache[id];
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
            ASSERT(sInstance != nullptr);
            return sInstance;
        };

        template<typename T>
        inline T* GetResource(AssetID id) 
        {
            if (id.IsValid()) 
            {
                ASSERT(mResourceCache.count(id) > 0);
                return static_cast<T*>(mResourceCache[id]);
            }
            return nullptr;
        }

        inline void GabageColloection()
        {
            for(int i = 0; i < mPendingDeleteList.size(); i++)
            {
                delete mPendingDeleteList[i];
            }
            mPendingDeleteList.clear();
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
    private:
        void LoadDependencies(const std::vector<MetaData>& dependenciesList);
        static ResourceManager* sInstance;
        unordered_map<AssetID, Resource*> mResourceCache;
        unordered_map<string, AssetID> mPathToID;
        std::vector<Resource*> mPendingDeleteList;

    };

}