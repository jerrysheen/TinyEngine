#pragma once
#include "Vector3.h"

namespace EngineCore
{

    class Quaternion
    {
    public:
        float x, y , z ,w;
        static Quaternion Identity;
    public:    
        Quaternion() = default;
        Quaternion(float x, float y, float z, float w);    
        // todo: implement real Forward
        Vector3 GetForward();
        Vector3 GetUp();
        Vector3 GetRight();
        static Quaternion FromRotationMatrix(const Vector3& col0, const Vector3& col1, const Vector3& col2);
        static Quaternion AngleAxisX(float degree);
        static Quaternion AngleAxisY(float degree);
        static Quaternion AngleAxisZ(float degree);

        static Quaternion FromEulerAngles(const Vector3& eulerAngles);
        Vector3 ToEulerAngles() const;

        static Quaternion Normalized(const Quaternion& quaternion);

        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
        static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);
        Quaternion operator*(const Quaternion& other) const;


        Quaternion Conjugate() const;
        Quaternion Inverse() const;
    };


}