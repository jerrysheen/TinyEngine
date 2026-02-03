#include "PreCompiledHeader.h"
#include "Camera.h"
#include "Renderer/RenderPipeline/OpaqueRenderPass.h"
#include "Renderer/RenderPipeline/FinalBlitPass.h"
#include "Renderer/RenderPipeline/GPUSceneRenderPass.h"
#include "GameObject/GameObject.h"
#include "Transform.h"
#include "Math/Frustum.h"
#include "Graphics/IGPUResource.h"
#include "Settings/ProjectSettings.h"


namespace EngineCore
{
    Camera::Camera(GameObject* go) : 
        mFov(25.0f), mAspect(1920.0f/1080.0f), mNear(0.2), mFar(400), mProjectionMatrix(Matrix4x4::Identity), mLookAt(Vector3{0.0f, 1.0f, 0.0f}), mViewMatrix(Matrix4x4::Identity), mWidth(1920.0f), mHeight(1080.0f)
    {
        gameObject = go;
        // fake one.
        if(RenderSettings::s_RenderPath == RenderSettings::RenderPathType::GPUScene)
        {
            mRenderPassAsset.renderPasses.push_back(new GPUSceneRenderPass());
        }
        else
        {
            mRenderPassAsset.renderPasses.push_back(new OpaqueRenderPass());
        }
        mRenderPassAsset.renderPasses.push_back(new FinalBlitPass());

        TextureDesc colorAttachmentDesc;
        colorAttachmentDesc.name = "CameraColorAttachment";
        colorAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        colorAttachmentDesc.width = 1920;
        colorAttachmentDesc.height = 1080;
        colorAttachmentDesc.format = TextureFormat::R8G8B8A8;
        colorAttachmentDesc.texUsage = TextureUsage::RenderTarget;
        colorAttachment = new RenderTexture(colorAttachmentDesc);

        TextureDesc depthAttachmentDesc;
        depthAttachmentDesc.name = "CameraDepthAttachment";
        depthAttachmentDesc.dimension = TextureDimension::TEXTURE2D;
        depthAttachmentDesc.width = 1920;
        depthAttachmentDesc.height = 1080;
        depthAttachmentDesc.format = TextureFormat::D24S8;
        depthAttachmentDesc.texUsage = TextureUsage::DepthStencil;
        depthAttachment = new RenderTexture(depthAttachmentDesc);
        
        //todo： 全局材质更新
        
        //mPerspectiveMatrix =  Matrix4x4(0.52, 0, -0.37, 0,
        //  0.4, 1.58, 0.57, -0.0,
        //  0, 0, -0.003, 0.29,
        //  0.53, -0.40, 0.74, 4.5);
        UpdateCameraMatrix();
    }

    // 物体已经在世界坐标系中， 所以只需要用vp矩阵判断， Mvp * p;
    void Camera::Update()
    {
        UpdateCameraMatrix();
        Matrix4x4 model = this->gameObject->GetComponent<Transform>()->GetWorldMatrix();
        Matrix4x4 vp = Matrix4x4::Multiply(mProjectionMatrix, mViewMatrix);
        mFrustum.UpdateFrustumPlane(vp);
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