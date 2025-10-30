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

        GameObject* mGO = nullptr;
        bool enabled = true;
    };
}