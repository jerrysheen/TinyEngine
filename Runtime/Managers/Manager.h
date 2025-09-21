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
        static T& GetInstance()
        {
            return *s_Instance;
        }
        
        // 为什么析构函数需要是虚的，需要每个子类去实现自己的析构，防止内存泄漏
        virtual ~Manager() = default;
        static void Update();
        static void Create();
    protected:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;
    };

}