#include "PreCompiledHeader.h"
#include "Transform.h"

namespace EngineCore
{
    Transform::Transform(GameObject* parent)
    {
        mParentGO = parent;
    }

    // 绕X轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateX(float degree)
    {
        Matrix4x4 rotation = Matrix4x4::RotateX(degree);
        worldMatrix = rotation * worldMatrix;
    }

    // 绕Y轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateY(float degree)
    {
        Matrix4x4 rotation = Matrix4x4::RotateY(degree);
        worldMatrix = rotation * worldMatrix;
    }

    // 绕Z轴旋转（局部空间）
    // 矩阵从右往左读：先应用worldMatrix，再应用rotation
    void Transform::RotateZ(float degree)
    {
        Matrix4x4 rotation = Matrix4x4::RotateZ(degree);
        worldMatrix = rotation * worldMatrix;
    }
}