#pragma once
#include "ComponentType.h"

namespace EngineCore
{
    class GameObject;
    class Component
    {
    public:
        Component(){};
        ~Component(){};
        ///virtual ComponentType GetType() const = 0;

        GameObject* mParentGO = nullptr;
        bool enabled = true;
    };
}