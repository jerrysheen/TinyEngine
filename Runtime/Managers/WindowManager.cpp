#include "PreCompiledHeader.h"
#include "WindowManager.h"
#include "Platforms/Windows/WindowManagerWindows.h"

namespace EngineCore
{
    
    std::unique_ptr<WindowManager> WindowManager::s_Instance = nullptr;

    void WindowManager::Create()
    {
        if (s_Instance) return;
        WindowManager::s_Instance = std::make_unique<WindowManagerWindows>();
        static_cast<WindowManagerWindows*>(WindowManager::s_Instance.get())->Create();
        static_cast<WindowManagerWindows*>(WindowManager::s_Instance.get())->Show();
    }

    void WindowManager::Update()
    {
        
    }

    void WindowManager::Destroy() 
    {
    }
}