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
            if (!WindowManager::s_Instance)
            {
                Initialize(); // 调用Create来创建具体的平台实例
                WindowManager::s_Instance->Create();
            }
            return *WindowManager::s_Instance;
        }
        virtual void Update() override;
        virtual void Create() override;
        virtual bool WindowShouldClose() = 0;
        virtual void OnResize() = 0;
        ~WindowManager() override {};
        WindowManager(){};
    protected:
        float mWindowWidth;
        float mWindowHeight;
        bool mMinimized = false;
        bool mResizing = false;
        bool mResized = false;
        bool mAppPaused = false;
		bool mMaximized = false;
    };
}