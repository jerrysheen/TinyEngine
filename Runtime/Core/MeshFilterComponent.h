#pragma once
#include "Component.h"
#include "ComponentType.h"

namespace EngineCore
{
    class MeshFilterComponent : public Component
    {
    public:
        MeshFilterComponent(){};
        ~MeshFilterComponent(){};
        static ComponentType GetType(){ return mType; };
    };

}