#include "PreCompiledHeader.h"
#include "WindowManager.h"
#include "Platforms/Windows/WindowManagerWindows.h"

namespace EngineCore
{
    std::unique_ptr<WindowManager> WindowManager::s_Instance = nullptr;
    void WindowManager::Create()
    {
    }

    void WindowManager::Initialize()
    {
        WindowManager::s_Instance = std::make_unique<WindowManagerWindows>();
    }

    void WindowManager::Update()
    {
        
    }
}