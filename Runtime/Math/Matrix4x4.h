#pragma once
#include "Vector3.h"

namespace EngineCore
{
    class Matrix4x4
    {
    public:
      static Matrix4x4 Identity;
      Matrix4x4();
      Matrix4x4(float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23,
                  float m30, float m31, float m32, float m33);
      static Matrix4x4 Matrix4x4::LookAt(const Vector3& position, const Vector3& target, const Vector3& up);
      static Matrix4x4 Matrix4x4::Perspective(const float& mFov, const float& mAspect, const float& mNear, const float& mFar);
      
      // 旋转矩阵生成 (参数为角度)
      static Matrix4x4 RotateX(float degrees);
      static Matrix4x4 RotateY(float degrees);
      static Matrix4x4 RotateZ(float degrees);
      
      // 矩阵乘法运算符
      Matrix4x4 operator*(const Matrix4x4& other) const;
      
    private:
		  float m00; float m01; float m02; float m03;
		  float m10; float m11; float m12; float m13;
		  float m20; float m21; float m22; float m23;
		  float m30; float m31; float m32; float m33;
    };
}