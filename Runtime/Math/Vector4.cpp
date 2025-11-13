#include "PreCompiledHeader.h"
#include "Vector4.h"

namespace EngineCore
{
    Vector4 Vector4::One = Vector4(1.0);
    Vector4 Vector4::Zero = Vector4(0.0);

    Vector4 Vector4::Normalized(const Vector4 &value)
    {
        float sqrtRoot = std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w);

        return Vector4(value.x / sqrtRoot, value.y / sqrtRoot, value.z / sqrtRoot, value.w / sqrtRoot);
    }

    Vector4 Vector4::Cross(const Vector4 &a, const Vector4 &b)
    {
        return Vector4(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x,
            a.w * b.w - a.w * b.w
        );
    }

    float Vector4::Length(const Vector4 &a)
    {
        return std::sqrt(Vector4::Dot(a, a));
    }

    float Vector4::Dot(const Vector4 &a, const Vector4 &b)
    {
        return a.x * b.x +  a.y * b.y +  a.z * b.z + a.w * b.w;
    }

    Vector4::Vector4(float Inx)
    {
        x = Inx;
        y = Inx;
        z = Inx;
        w = Inx;
    }

    Vector4::Vector4(float Inx, float Iny, float Inz, float Inw)
    {
        x = Inx;
        y = Iny;
        z = Inz;
        w = Inw;
    }

    Vector4 Vector4::operator-(const Vector4 &value) const
    {
        return Vector4(x - value.x, y - value.y, z - value.z, w - value.w);
    }

    Vector4 Vector4::operator+(const Vector4 &value) const
    {
        return Vector4(x + value.x, y + value.y, z + value.z, w + value.w);
    }

    Vector4 Vector4::operator/=(float value) const
    {
        return Vector4(x/value, y/value, z/value, w/value);
    }

    Vector4 Vector4::operator*(float value) const
    {
        return Vector4(x * value, y * value, z * value, w * value);
    }
}