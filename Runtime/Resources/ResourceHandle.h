#pragma once
#include "Asset.h"

namespace EngineCore
{
    enum class ResourceState
    {
        NotExits = 0,
        Loading = 1,
        Success = 2,
    };

    // 这个TypeT可以用来做一些强转功能。
    // ResourceHandle本身的创建并不会设计到Ref++， 因为已经在资源层记录
    // 但是Resource的拷贝，移动等，会涉及到Ref++；
    template<typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;

        ResourceHandle(AssetID id);
        
        // 拷贝构造：
        ResourceHandle(const ResourceHandle& other);

        ResourceHandle(ResourceHandle&& other) noexcept : mAssetID(other.mAssetID)
        {
            // 原来的要 releaseRef， 新的要AddRef所以刚刚好抵消。
            // 当前内容已经在初始化列表中赋值了
            other.mAssetID.Reset();
        }

        ~ResourceHandle();

        // 赋值拷贝
        ResourceHandle& operator=(const ResourceHandle& other);

        //移动赋值：
        ResourceHandle& operator=(ResourceHandle&& other) noexcept;

        // ReourceHandle.Get()， 获得具体的Resource，
        T* Get() const;

        T* operator ->() const 
        {
            return Get();
        }

        // 解引用
        T& operator*() const {
            return *Get();
        }

        // 判断是否有效
        bool IsValid() const {
            return mAssetID.IsValid() && Get() != nullptr;
        }

        ResourceState GetResourecState()
        {
            return ResourceManager::GetInstance()->GetResourceStateByID(mAssetID.value);
        }

        inline AssetID GetAssetID() const {return mAssetID;};
    // todo : temp data for jason sereilization;
    public :
        AssetID mAssetID;
    };
}
#include "ResourceManager.h"
namespace EngineCore
{
    template<typename T>
    inline ResourceHandle<T>::ResourceHandle(AssetID id)
        : mAssetID(id)
    {
        if (mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->AddRef(mAssetID);
        }
    }

    template <typename T>
    inline ResourceHandle<T>::ResourceHandle(const ResourceHandle &other)
        : mAssetID(other.GetAssetID())
    {
        if(mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->AddRef(mAssetID);
        }
    }
    
    template <typename T>
    inline ResourceHandle<T>::~ResourceHandle()
    {
        if(mAssetID.IsValid())
        {
            ResourceManager::GetInstance()->DecreaseRef(mAssetID);
        }
    }

    
    template <typename T>
    inline ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle& other)
    {
        if(this != &other)
        {
            AssetID oldID = mAssetID;  // ✓ 先保存旧 ID
            mAssetID = other.mAssetID;  // 赋新值
            // 先去add， 然后再去释放， 避免count为0直接销毁
            if(mAssetID.IsValid())
            {
                ResourceManager::GetInstance()->AddRef(mAssetID);
            }

            //  释放现在的
           if(oldID.IsValid())
           {
               ResourceManager::GetInstance()->DecreaseRef(oldID);
           } 

        }
        return *this;
    }

    template <typename T>
    inline ResourceHandle<T>& ResourceHandle<T>::operator=(ResourceHandle&& other) noexcept
    {
           // 将亡值，只要保证 IsValid是false， 就不会去自己析构的时候Ref--，
            // 所以这个地方获得控制权， 然后直接Ref--
            // 将other move 到 current， 也就是other的ref不用减， current要release
            if(this != &other)
            {
                if(mAssetID.IsValid())
                {
                    ResourceManager::GetInstance()->AddRef(mAssetID);
                }
                // 转移所有权
                mAssetID = other.mAssetID;
                other.mAssetID.Reset();  // 清空 other
            }
            // 如果两者一样的话，就不做操作， 将亡值走自己的析构函数 decrease ref
            return *this;

    }

    template <typename T>
    inline T *ResourceHandle<T>::Get() const
    {
        if (mAssetID.IsValid()) 
        {
            return ResourceManager::GetInstance()->GetResource<T>(mAssetID);
        }
        ASSERT(false);
    }
}