#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/FrameBufferManager.h"
#include "Graphics/FrameBufferObject.h"
#include "Renderer/RenderPipeline/OpaqueRenderPass.h"
#include "Renderer/RenderPipeline/FinalBlitPass.h"
#include "GameObject/GameObject.h"
#include "Transform.h"

namespace EngineCore
{
    Camera::Camera(GameObject* go) : 
        mFov(25.0f), mAspect(1920.0f/1080.0f), mNear(0.2), mFar(200), mProjectionMatrix(Matrix4x4::Identity), mLookAt(Vector3{0.0f, 1.0f, 0.0f}), mViewMatrix(Matrix4x4::Identity), mWidth(1920.0f), mHeight(1080.0f)
    {
        gameObject = go;
        // fake one.
        mRenderPassAsset.renderPasses.push_back(new OpaqueRenderPass());
        mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

        FrameBufferDesc colorAttachmentDesc;
        colorAttachmentDesc.name = "CameraColorAttachment";
        colorAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        colorAttachmentDesc.width = 800;
        colorAttachmentDesc.height = 600;
        colorAttachmentDesc.format = TextureFormat::R8G8B8A8;
        colorAttachment = FrameBufferManager::GetInstance()->CreateFBO(colorAttachmentDesc);

        FrameBufferDesc depthAttachmentDesc;
        depthAttachmentDesc.name = "CameraColorAttachment";
        depthAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        depthAttachmentDesc.width = 800;
        depthAttachmentDesc.height = 600;
        depthAttachmentDesc.format = TextureFormat::D24S8;
        depthAttachment = FrameBufferManager::GetInstance()->CreateFBO(depthAttachmentDesc);
        //todo： 全局材质更新
        
        //mPerspectiveMatrix =  Matrix4x4(0.52, 0, -0.37, 0,
        //  0.4, 1.58, 0.57, -0.0,
        //  0, 0, -0.003, 0.29,
        //  0.53, -0.40, 0.74, 4.5);
        UpdateCameraMatrix();
    }


    void Camera::Update()
    {
        UpdateCameraMatrix();
    }
    
    // update view and perspective matrix
    void Camera::UpdateCameraMatrix()
    {
        auto transform = this->gameObject->GetComponent<Transform>();
        auto& position = transform->GetWorldPosition();
        Quaternion rotation = transform->GetWorldQuaternion();

        Vector3 forward = rotation.GetForward();
        Vector3 up = rotation.GetUp();

        // 临时测试：让相机向下倾斜看
        // forward = Vector3::Normalized(Vector3(0.0f, -0.5f, 1.0f));  // 向下看约26度
        // forward = Vector3::Normalized(Vector3(0.0f, -1.0f, 1.0f));  // 向下看约45度
        //forward = Vector3::Normalized(Vector3(0.0f, -1.0f, 1.0f));  // 向下看45度

        Vector3 target = position + forward;

        mViewMatrix = Matrix4x4::LookAt(position, target, up);
        mProjectionMatrix = Matrix4x4::Perspective(mFov, mAspect, mNear, mFar);
    }
}