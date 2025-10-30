#pragma once
#include <vector>
#include "Component.h"
#include "Math/Math.h"

namespace EngineCore
{
    class GameObject;
    struct Transform : Component
    {
        Transform() = default;
        Transform(GameObject* parent);
        virtual ~Transform() override;
        static ComponentType GetType() { return ComponentType::Transfrom; };
        void MarkDirty();
        std::vector<Transform*> childTransforms;
        Transform* parentTransform = nullptr;
    
        inline void SetParent(Transform* transform)
        {
            parentTransform = transform; 
            transform->AddChild(this);
        };

        inline void DettachParent()
        {
            if(parentTransform != nullptr) parentTransform->RemoveChild(this);
            parentTransform = nullptr;
        }

        inline void AddChild(Transform* transform)
        {
            childTransforms.push_back(transform);
        };

        inline void RemoveChild(Transform* transform)
        {
            auto it = std::find(childTransforms.begin(), childTransforms.end(), transform);
            if(it != childTransforms.end())
            {
                childTransforms.erase(it);
            }
        };

        void RotateX(float degree);
        void RotateY(float degree);
        void RotateZ(float degree);

        const Vector3& GetWorldPosition(){ return mWorldPosition; };
        const Quaternion& GetWorldQuaternion(){ return mWorldQuaternion; };
        const Vector3& GetWorldScale(){ return mWorldScale; };

        const Vector3& GetLocalPosition(){ return mLocalPosition; };
        const Quaternion& GetLocalQuaternion(){ return mLocalQuaternion; };
        const Vector3& GetLocalScale(){ return mLocalScale; };

        void SetLocalPosition(const Vector3& localPosition);
        void SetLocalQuaternion(const Quaternion& localQuaternion);
        void SetLocalScale(const Vector3& localScale);

        inline const Matrix4x4& GetWorldMatrix()
        {
            UpdateIfDirty(); 
             return mWorldMatrix;
        }
        
        inline const Matrix4x4& GetLocalMatrix()
        {
            UpdateIfDirty(); 
            return mLocalMatrix;
        }

        void UpdateIfDirty();
        void UpdateTransform();
        inline void UpdateNow() { UpdateTransform(); };
    public:
        bool isDirty = false;
    private:

        Matrix4x4 mWorldMatrix;
        Matrix4x4 mLocalMatrix;

        // 外部只读WorldPosition
        Vector3 mWorldPosition;
        Quaternion mWorldQuaternion;
        Vector3 mWorldScale;
        // 外部能读写LocalPositon
        Vector3 mLocalPosition;
        Quaternion mLocalQuaternion;
        Vector3 mLocalScale;
    };
} // namespace EngineCore
