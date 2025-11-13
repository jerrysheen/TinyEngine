#pragma once

namespace EngineCore
{
    class Vector4
    {
    public:
        static Vector4 Zero;
        static Vector4 One;
    public:

        static Vector4 Normalized(const Vector4& value);
        static Vector4 Cross(const Vector4& a, const Vector4& b);
        static float Length(const Vector4& a);
        static float Dot(const Vector4& a, const Vector4& b);
        float x;
        float y;
        float z;
        float w;
        Vector4(){x = 0; y = 0; z = 0; w = 0;};
        Vector4(float x);
        Vector4(float x, float y, float z, float w);
    
        Vector4 operator-(const Vector4& value) const;
        Vector4 operator+(const Vector4& value) const;
        Vector4 operator/=(float value) const;
        Vector4 operator*(float value) const;
    };
}