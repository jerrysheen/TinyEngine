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
        virtual void Update() override;
        virtual void Create() override;
        virtual bool WindowShouldClose() override;
        virtual void OnResize() override;
        WindowManagerWindows(){};
        
        LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        void InitializeWindowsWindow();
        HWND mWindow;
    };
}