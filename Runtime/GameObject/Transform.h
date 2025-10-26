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
        static ComponentType GetType() { return ComponentType::Transfrom; };

        Vector3 position;
        Quaternion quaternion;
        Vector3 scale;
        std::vector<Transform*> childTransform;

        Vector3 localPosition;
        Vector3 localScale;
        Quaternion localQuaternion;
        Matrix4x4 worldMatrix;
        
        ~Transform()
        {
            parent->RemoveChild(this);
        };
        Transform* parent;
        GameObject* gameObject;
        
        inline void SetParent(Transform* transform)
        {
            parent = transform; 
            transform->AddChild(this);
        };

        inline void AddChild(Transform* transform)
        {
            childTransform.push_back(transform);
        };

        inline void RemoveChild(Transform* transform)
        {
            auto it = std::find(childTransform.begin(), childTransform.end(), transform);
            if(it != childTransform.end())
            {
                childTransform.erase(it);
            }
        };

        void RotateX(float degree);
        void RotateY(float degree);
        void RotateZ(float degree);
    };
} // namespace EngineCore
