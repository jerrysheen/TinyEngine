#pragma once

namespace EngineCore
{
    class Vector3
    {
    public:
        static Vector3 Zero;
        static Vector3 One;
    public:

        static Vector3 Normalized(const Vector3& value);
        static Vector3 Cross(const Vector3& a, const Vector3& b);
        static float Dot(const Vector3& a, const Vector3& b);
        float x;
        float y;
        float z;
        Vector3(){x = 0; y = 0; z = 0;};
        Vector3(float x);
        Vector3(float x, float y, float z);
    
        Vector3 operator-(const Vector3& value) const;
        Vector3 operator+(const Vector3& value) const;
    };
}