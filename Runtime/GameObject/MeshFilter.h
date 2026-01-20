#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Mesh.h"
#include "Serialization/BaseTypeSerialization.h"
#include "Math/AABB.h"

namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);

        virtual ~MeshFilter() override;
        static ComponentType GetStaticType() { return ComponentType::MeshFilter; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshFilter; };
        void OnLoadResourceFinished();
    public:
        ResourceHandle<Mesh> mMeshHandle;
        
        virtual const char* GetScriptName() const override { return "MeshFilter"; }
        virtual json SerializedFields() const override {
            return json{
                {"MeshHandle", mMeshHandle},
            };
        }
        
        virtual void DeserializedFields(const json& data) override;

        uint32_t GetHash()
        {
            return mMeshHandle->GetInstanceID();
        }
    private:
        uint32_t hash;
    };

}