#pragma once
#include "ComponentType.h"

namespace EngineCore
{
    class GameObject;
    class Component
    {
        friend class MeshFilterComponent;
    public:
        Component(){};
        ~Component(){};
        static ComponentType GetType(){return mType;};
    private:
        static ComponentType mType;
    };
}