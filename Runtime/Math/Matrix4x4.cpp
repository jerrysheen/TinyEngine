#include "PreCompiledHeader.h"
#include "Matrix4x4.h"
#include "Vector3.h"

namespace EngineCore
{
    //-------------------------------------------
    // RowMajor的形式
    // D3D12驱动读取的时候会当成ColMajor，相当于做了一次隐式Transpose()
    // 用mul(pos, Matrix)刚好又能和RowMajor对上
    //-------------------------------------------
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


    // 统一处理成， 左手坐标系，z轴正方向指向target
    // 后续变化在ViewSpace中完成
    Matrix4x4 Matrix4x4::LookAt(const Vector3 &position, const Vector3 &target, const Vector3 &up)
    {
        Vector3 zAxis = Vector3::Normalized((target - position));
        Vector3 xAxis = Vector3::Normalized((Vector3::Cross(zAxis , up)));
        Vector3 yAxis = Vector3::Normalized((Vector3::Cross(xAxis , zAxis)));
    
        // 这个问题这么理解，首先思考从View -> World，很快能的出来R也就是旋转部分
        // 为什么，因为col方向是原先的基向量（view）在新的空间下的表示（world的xyz）
        //  xAxis.x, yAxis.y, zAxis.z;
        //  xAxis.x, yAxis.y, zAxis.z;
        //  xAxis.x, yAxis.y, zAxis.z;
        // 又因为R的逆矩阵， 是正交矩阵，所以就是 R^T
        // 剩下来的p怎么思考， 就思考p代表世界空间， 需要先根据相机矩阵做一个旋转，得到正确的值
        // 也就是 R^T T， 这是行列式表示，变成公式就是点积
        return Matrix4x4(
            xAxis.x, xAxis.y, xAxis.z, -Vector3::Dot(position, xAxis),
            yAxis.x, yAxis.y, yAxis.z, -Vector3::Dot(position, yAxis),
            zAxis.x, zAxis.y, zAxis.z, -Vector3::Dot(position, zAxis),
                0,      0,      0,          1
        );
    }   

    Matrix4x4 Matrix4x4::Perspective(const float &mFov, const float &mAspect, const float &mNear, const float &mFar)
    {
        //[NDC 0, 1] z轴已经对齐，不用转化
        #ifdef D3D12_API
            return Matrix4x4(
                1.0f/(mAspect * std::tan(mFov/2)), 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f / (std::tan(mFov/2)), 0.0f, 0.0f,
                0.0f, 0.0f, mFar / (mFar - mNear), -(mFar * mNear) / (mFar - mNear),  // DirectX风格
                0.0f, 0.0f, 1.0f, 0.0f
            );
        #elif
        //[NDC -1, 1] z轴需要反向，不用转化
        #endif
    }
}