#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "Serialization/BaseTypeSerialization.h"

namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override {};
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };
    public:
        ResourceHandle<Material> mMatHandle;


        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        virtual json SerializedFields() const override {
            return json{
                {"MatHandle", mMatHandle},
            };
        }
        
        virtual void DeserializedFields(const json& data) override {
            data.at("MatHandle").get_to(mMatHandle);
        }
    };

}