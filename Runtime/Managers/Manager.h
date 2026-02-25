#pragma once
#include "PreCompiledHeader.h"

namespace EngineCore
{

    template<typename T>
    class Manager
    {
    public:
        static std::unique_ptr<T> s_Instance;
        // 一定被提前创建，Create在Get之前。
        static T* GetInstance()
        {
            if (s_Instance == nullptr) 
            {
                s_Instance = std::make_unique<T>();
            }
            return s_Instance.get();
        }
        
        // 为什么析构函数需要是虚的，需要每个子类去实现自己的析构，防止内存泄漏
        virtual ~Manager() = default;
        
        static void Update();
        static void Create();
        
        // 销毁单例实例（正确方式）
        static void Destroy()
        {
            if (s_Instance)
            {
                s_Instance.reset();  // 触发析构函数，然后释放内存
                // 或者: s_Instance = nullptr;  // 效果相同
            }
        }
        
    protected:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;
    };

}