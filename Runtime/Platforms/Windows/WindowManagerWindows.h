#pragma once
#include "Managers/WindowManager.h"

namespace EngineCore
{
    class WindowManagerWindows : public WindowManager
    {
    public :
        virtual void Update() override;
        WindowManagerWindows(){};
    };
}