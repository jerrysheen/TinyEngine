#pragma once
#include <vector>
#include "Component.h"
#include "Math/Math.h"
#include "Scene/SceneManager.h"
#include "Serialization/BaseTypeSerialization.h"

namespace EngineCore
{
    class GameObject;
    struct Transform : Component
    {
        Transform();
        Transform(GameObject* parent);
        virtual ~Transform() override;
        static ComponentType GetStaticType() { return ComponentType::Transform; };
        virtual ComponentType GetType() const override{ return ComponentType::Transform; };
        void MarkDirty();


        void RotateX(float degree);
        void RotateY(float degree);
        void RotateZ(float degree);

        const Vector3 GetLocalEulerAngles(); 
        void SetLocalEulerAngles(const Vector3& eulerAngles);

        const Vector3 GetWorldPosition(){ return mWorldPosition; };
        const Quaternion GetWorldQuaternion(){ return mWorldQuaternion; };
        const Vector3 GetWorldScale(){ return mWorldScale; };

        const Vector3 GetLocalPosition() const { return mLocalPosition; };
        const Quaternion GetLocalQuaternion() const { return mLocalQuaternion; };
        const Vector3 GetLocalScale() const { return mLocalScale; };

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
        //inline void UpdateNow() { UpdateTransform(); };
        uint32_t transformVersion = 1;
    public:
        bool isDirty = true;
        std::vector<Transform*> childTransforms;
        Transform* parentTransform = nullptr;
        
    protected:

        friend class GameObject;
        // 外部不能访问修改， 只能访问GameObject.SetParent
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
    
    public:

        virtual const char* GetScriptName() const override 
        { 
            return "Transform"; 
        }

        virtual json SerializedFields() const override {
            return json{
                {"LocalPosition", mLocalPosition},
                {"LocalQuaternion", mLocalQuaternion},
                {"LocalScale", mLocalScale},
            };
        }

        virtual void DeserializedFields(const json& data) override {
            data.at("LocalPosition").get_to(mLocalPosition);
            data.at("LocalQuaternion").get_to(mLocalQuaternion);
            data.at("LocalScale").get_to(mLocalScale);

            //UpdateNow();
        };

    };
} // namespace EngineCore
