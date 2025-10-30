#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/ModelData.h"

namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);
        virtual ~MeshFilter() override {};
        static ComponentType GetType() { return ComponentType::MeshFilter; };
    public:
        ResourceHandle<ModelData> mMeshHandle;
    };

}