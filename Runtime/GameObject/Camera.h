#pragma once
#include "Core/PublicStruct.h"
#include "Graphics/Texture.h"
#include "Component.h"

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

        static ComponentType GetType() { return ComponentType::Camera; };
        void Update();
    public:
        RenderPassAsset mRenderPassAsset;
        // 这两个只是描述符， 没必要用指针
        ResourceHandle<FrameBufferObject> depthAttachment;
        ResourceHandle<FrameBufferObject> colorAttachment;
        void UpdateCameraMatrix();
    private:
    };

}

