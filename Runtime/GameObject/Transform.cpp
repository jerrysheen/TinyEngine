#include "PreCompiledHeader.h"
#include "Transform.h"
#include "Serialization/ComponentFactory.h"
#include "GameObject.h"

REGISTER_SCRIPT(Transform)
namespace EngineCore
{
    Transform::Transform()
        :mWorldMatrix(Matrix4x4::Identity), mLocalMatrix(Matrix4x4::Identity),
        mWorldPosition(Vector3::Zero), mWorldQuaternion(Quaternion::Identity), mWorldScale(Vector3::One), mLocalPosition(Vector3::Zero), mLocalQuaternion(Quaternion::Identity), mLocalScale(Vector3::One)
    {
    }
    Transform::Transform(GameObject* go)
        :mWorldMatrix(Matrix4x4::Identity), mLocalMatrix(Matrix4x4::Identity),
        mWorldPosition(Vector3::Zero), mWorldQuaternion(Quaternion::Identity), mWorldScale(Vector3::One), mLocalPosition(Vector3::Zero), mLocalQuaternion(Quaternion::Identity), mLocalScale(Vector3::One)
    {
        gameObject = go;
        go->transform = this;
    }

    Transform::~Transform()
    {
        // 断开child链接
        for (int i = 0; i < childTransforms.size(); i++)
        {
            childTransforms[i]->DettachParent();
        }
        // 断开parent链接
        if (parentTransform != nullptr) parentTransform->RemoveChild(this);
    }

    void Transform::MarkDirty()
    {
        if(isDirty) return;
        isDirty = true;
        for(auto childTrans : childTransforms)
        {
            ASSERT(childTrans != nullptr);
            childTrans->MarkDirty();
        }
    }

    // 绕X轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateX(float degree)
    {
        Quaternion rotationDelta = Quaternion::AngleAxisX(degree);
        mLocalQuaternion = rotationDelta * mLocalQuaternion;
        MarkDirty();
    }

    // 绕Y轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateY(float degree)
    {
        Quaternion rotationDelta = Quaternion::AngleAxisY(degree);
        mLocalQuaternion = rotationDelta * mLocalQuaternion;
        MarkDirty();
    }

    // 绕Z轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateZ(float degree)
    {
        Quaternion rotationDelta = Quaternion::AngleAxisZ(degree);
        mLocalQuaternion = rotationDelta * mLocalQuaternion;
        MarkDirty();
    }

    void Transform::SetLocalPosition(const Vector3 &localPosition)
    {
        mLocalPosition = localPosition;
        MarkDirty();
    }

    void Transform::SetLocalQuaternion(const Quaternion &localQuaternion)
    {
        mLocalQuaternion = localQuaternion;
        MarkDirty();
    }

    void Transform::SetLocalScale(const Vector3& localScale)
    {
        mLocalScale = localScale;
        MarkDirty();
    }

    void Transform::UpdateTransform()
    {
        mLocalMatrix =  Matrix4x4::TRS(mLocalPosition, mLocalQuaternion, mLocalScale);
        if(parentTransform != nullptr)
        {
            mWorldMatrix = parentTransform->GetWorldMatrix() * mLocalMatrix;
        }
        else
        {
            mWorldMatrix = mLocalMatrix;
        }

        //update worldPos worldScale worldRotation
        Matrix4x4::WorldMatrixDecompose(mWorldMatrix, mWorldPosition, mWorldQuaternion, mWorldScale);
        isDirty = false;

        for(int i = 0; i < childTransforms.size(); i++)
        {
            childTransforms[i]->UpdateTransform();
        }

        transformVersion++;
    }



    void Transform::UpdateIfDirty()
    {
        if(isDirty)
        {
            UpdateTransform();
            isDirty = false;
        }
    }

    const Vector3 Transform::GetLocalEulerAngles()
    {
        return mLocalQuaternion.ToEulerAngles();
    }

    void Transform::SetLocalEulerAngles(const Vector3& eulerAngles)
    {
        mLocalQuaternion = Quaternion::FromEulerAngles(eulerAngles);
        MarkDirty();
    }
}