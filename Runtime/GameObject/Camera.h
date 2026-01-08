#pragma once
#include "Core/PublicStruct.h"
#include "Graphics/Texture.h"
#include "Component.h"
#include "Serialization/BaseTypeSerialization.h"
#include "Math/Frustum.h"
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    class Camera : public Component
    {
    public:
        Camera() = default;
        Camera(GameObject* parent);
        virtual ~Camera() override {};
        float mFov;
        float mAspect;
        float mNear;
        float mFar; 
        float mWidth;
        float mHeight;
        Matrix4x4 mProjectionMatrix;
        Vector3 mLookAt;
        Matrix4x4 mViewMatrix;
        Frustum mFrustum;

        static ComponentType GetStaticType() { return ComponentType::Camera; };
        virtual ComponentType GetType() const override { return ComponentType::Camera; };
        void Update();
    public:
        RenderPassAsset mRenderPassAsset;
        // 这两个只是描述符， 没必要用指针
        RenderTexture* depthAttachment;
        RenderTexture* colorAttachment;
        void UpdateCameraMatrix();


        virtual const char* GetScriptName() const override { return "Camera"; }
        virtual json SerializedFields() const override {
            return json{
                {"Fov", mFov},
                {"Aspect", mAspect},
                {"Near", mNear},
                {"Far", mFar},
                {"Width", mWidth},
                {"Height", mHeight}
            };
        }
        
        virtual void DeserializedFields(const json& data) override {
            data.at("Fov").get_to(mFov);
            data.at("Aspect").get_to(mAspect);
            data.at("Near").get_to(mNear);
            data.at("Far").get_to(mFar);
            data.at("Width").get_to(mWidth);
            data.at("Height").get_to(mHeight);
        }
    };

}

