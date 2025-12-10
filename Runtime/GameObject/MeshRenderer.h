#pragma once
#include "Component.h"
#include "ComponentType.h"
#include "Resources/ResourceHandle.h"
#include "Graphics/Material.h"
#include "Serialization/BaseTypeSerialization.h"
#include "GameObject/Transform.h"
#include "Math/AABB.h"
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override;
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };

        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        virtual json SerializedFields() const override {
            return json{
                {"MatHandle", mShardMatHandler},
            };
        }
        
        virtual void DeserializedFields(const json& data) override {
            data.at("MatHandle").get_to(mShardMatHandler);
            SetUpMaterialPropertyBlock();
        }
        void SetUpMaterialPropertyBlock();

        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };

        inline void SetSharedMaterial(const ResourceHandle<Material>& mat) 
        {
            mShardMatHandler = mat;
            SetUpMaterialPropertyBlock();
        }

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        inline void MarkWorldBoundsDirty(){ needUpdateWorldBounds = true;}
        inline bool ShouldUpdateWorldBounds(){ return needUpdateWorldBounds;}
        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);
        void SyncPerObjectDataIfDirty();
        AABB worldBounds;
        bool IsDirty = true;
        BufferAllocation perObjectDataAllocation;
    private:
        bool needUpdateWorldBounds = true;
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

    };

}