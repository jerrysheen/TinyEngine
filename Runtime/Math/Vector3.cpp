#include "PreCompiledHeader.h"
#include "Vector3.h"

namespace EngineCore
{
    Vector3 Vector3::One = Vector3(1.0);
    Vector3 Vector3::Zero = Vector3(0.0);

    Vector3 Vector3::Normalized(const Vector3 &value)
    {
        float sqrtRoot = std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);

        return Vector3(value.x / sqrtRoot, value.y / sqrtRoot, value.z / sqrtRoot);
    }

    Vector3 Vector3::Cross(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    float Vector3::Length(const Vector3 &a)
    {
        return std::sqrt(Vector3::Dot(a, a));
    }

    float Vector3::Dot(const Vector3 &a, const Vector3 &b)
    {
        return a.x * b.x +  a.y * b.y +  a.z * b.z;
    }

    Vector3::Vector3(float Inx)
    {
        x = Inx;
        y = Inx;
        z = Inx;
    }

    Vector3::Vector3(float Inx, float Iny, float Inz)
    {
        x = Inx;
        y = Iny;
        z = Inz;
    }

    Vector3 Vector3::operator-(const Vector3 &value) const
    {
        return Vector3(x - value.x, y - value.y, z - value.z);
    }

    Vector3 Vector3::operator+(const Vector3 &value) const
    {
        return Vector3(x + value.x, y + value.y, z + value.z);
    }

    Vector3 Vector3::operator/=(float value) const
    {
        return Vector3(x/value, y/value, z/value);
    }
}