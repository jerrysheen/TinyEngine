#pragma once
#include "ComponentType.h"

namespace EngineCore
{
    class GameObject;
    class Component
    {
    public:
        Component(){};
        virtual ~Component() = 0;
        //virtual ComponentType GetType() const = 0;

        GameObject* gameObject = nullptr;
        bool enabled = true;
    };
}