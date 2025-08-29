#pragma once
#include "Manager.h"

namespace EngineCore
{
    class WindowManager : public Manager<WindowManager>
    {
    public:

        static void Initialize();
        static WindowManager& GetInstance()
        {
            if (!s_Instance)
            {
                Initialize(); // 调用Create来创建具体的平台实例
                WindowManager::s_Instance->Create();
            }
            return *s_Instance;
        }
        virtual void Update() override;
        virtual void Create() override;
        ~WindowManager() override {};
        WindowManager(){};
    };
}