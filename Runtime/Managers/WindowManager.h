#pragma once
#include "Manager.h"

namespace EngineCore
{
    class  WindowManager
    {
    public:
        inline static WindowManager* GetInstance() 
        {
            if (s_Instance == nullptr)
            {
                Create();
            };
            return s_Instance.get();
        }
        static void Update();
        static void Create();
        static void Destroy();
        virtual bool WindowShouldClose() = 0;
        virtual void OnResize() = 0;
        virtual void* GetWindow() = 0;
        inline int GetWidth() { return mWindowWidth; };
        inline int GetHeight() { return mWindowHeight; };
        std::pair<int, int> GetWindowSize()
        {
            return {mWindowWidth, mWindowHeight};
        }
        ~WindowManager(){};
        WindowManager(){};

    protected:
        int mWindowWidth;
        int mWindowHeight;
        bool mMinimized = false;
        bool mResizing = false;
        bool mResized = false;
        bool mAppPaused = false;
		bool mMaximized = false;
        static std::unique_ptr<WindowManager> s_Instance;
    };
}