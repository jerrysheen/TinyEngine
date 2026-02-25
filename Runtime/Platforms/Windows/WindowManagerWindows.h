#pragma once
#include "Managers/WindowManager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


namespace EngineCore
{
    class WindowManagerWindows : public WindowManager
    {
    public :
        static void Update();
        void Create();
        virtual bool WindowShouldClose() override;
        virtual void OnResize() override;
        WindowManagerWindows();
        virtual void* GetWindow() override {return mWindow;}
        LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void Show();
    private:
        void InitializeWindowsWindow();
        HWND mWindow;
    };
}