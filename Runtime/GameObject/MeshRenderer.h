#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"

namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override {};
        static ComponentType GetType() { return ComponentType::MeshRenderer; };
    public:
        ResourceHandle<Material> mMatHandle;
    };

}