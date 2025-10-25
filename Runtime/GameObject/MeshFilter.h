#pragma once
#include "Component.h"
#include "ComponentType.h"

namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);
        ~MeshFilter(){};
        static ComponentType GetType() { return ComponentType::MeshFilter; };
         
    };

}