#include "PreCompiledHeader.h"
#include "Transform.h"

namespace EngineCore
{
    Transform::Transform(GameObject* gameObject)
    {
        mGO = gameObject;
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
        Matrix4x4 rotation = Matrix4x4::RotateX(degree);
        MarkDirty();
    }

    // 绕Y轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateY(float degree)
    {
        Matrix4x4 rotation = Matrix4x4::RotateY(degree);
        MarkDirty();
    }

    // 绕Z轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateZ(float degree)
    {
        Matrix4x4 rotation = Matrix4x4::RotateZ(degree);
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

    void Transform::SetLocalScale(const Vector3 &localScale)
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

        for(int i = 0; i < childTransforms.size(); i++)
        {
            childTransforms[i]->UpdateTransform();
        }
        isDirty = false;
    }

    void Transform::UpdateIfDirty()
    {
        if(isDirty)
        {
            UpdateTransform();
            isDirty = false;
        }
    }
}