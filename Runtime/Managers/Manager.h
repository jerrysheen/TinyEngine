#pragma once
#include "PreCompiledHeader.h"

namespace EngineCore
{

    template<typename T>
    class Manager
    {
    public:
        static std::unique_ptr<T> s_Instance;
        static T& GetInstance()
        {
            if(!s_Instance)
            {
                s_Instance = std::make_unique<T>();
            }
            return *s_Instance;
        }

        // 为什么析构函数需要是虚的，需要每个子类去实现自己的析构，防止内存泄漏
        virtual ~Manager() = default;
        virtual void Update() = 0;
        virtual void Create() = 0;
    protected:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;
    };

}