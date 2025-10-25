#include "PreCompiledHeader.h"
#include "Matrix4x4.h"
#include "Vector3.h"

namespace EngineCore
{
    Matrix4x4 Matrix4x4::Identity = Matrix4x4();
    Matrix4x4::Matrix4x4()
    {
        m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; m03 = 0.0f;
		m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
		m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
		m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
    }

    Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33):
        m00(m00), m01(m01), m02(m02), m03(m03),
        m10(m10), m11(m11), m12(m12), m13(m13),
        m20(m20), m21(m21), m22(m22), m23(m23),
        m30(m30), m31(m31), m32(m32), m33(m33)
    {
    }


    Matrix4x4 Matrix4x4::LookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up)
    {
        Vector3 zAxis = Vector3::Normalized((target - position));
        Vector3 xAxis = Vector3::Normalized((Vector3::Cross(zAxis , up)));
        Vector3 yAxis = Vector3::Normalized((Vector3::Cross(xAxis , zAxis)));
    
        // 可以把问题放在世界坐标系下理解
        // recap： viewSpace下的 (viewx， viewy， viewz)代表什么，
        // 代表的是xAxis yAxis zAxis这三个方向上的长度
        // 假设 a 是 worldspace下的一个向量，他在这三个向量上的长度怎么算？
        // 就是a在三个方向上的投影, 也就是点积
        // 所以 viewX 就是 worldspace vector *  viewX向量也就是 xAxis
        // 所以 viewY 就是 worldspace vector *  viewY向量也就是 yAxis
        // 移动怎么算？ 移动也是算投影，原点从 0，0，0， 挪到 position
        // x方向上移动了 - 投影x距离， 这个稍微空间想象一下就好。
        // 比如就想相机的位置在 1，0，0位置， 方向和原来坐标系一样
        // 那么原来的世界原点就在相机的 -1，0，0位置
        // 【主要记住， 投影！】
        return Matrix4x4(
            xAxis.x, xAxis.y, xAxis.z, -Vector3::Dot(position, xAxis),
            yAxis.x, yAxis.y, yAxis.z, -Vector3::Dot(position, yAxis),
            zAxis.x, zAxis.y, zAxis.z, -Vector3::Dot(position, zAxis),
                0,      0,      0,          1
        );
    }   

    Matrix4x4 Matrix4x4::Perspective(const float &mFov, const float &mAspect, const float &mNear, const float &mFar)
    {
        return Matrix4x4(
            1.0f/(mAspect * std::tan(mFov/2)), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / (std::tan(mFov/2)) , 0.0f, 0.0f,
            0.0f, 0.0f, -(mFar + mNear) / (mFar - mNear), -(2.0f * mFar * mNear) / (mFar - mNear),
            0.0f, 0.0f, -1.0f, 0.0f
        );
    }
}