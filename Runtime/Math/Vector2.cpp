#include "PreCompiledHeader.h"
#include "Vector2.h"

namespace EngineCore
{
    Vector2 Vector2::One = Vector2(1.0);
    Vector2 Vector2::Zero = Vector2(0.0);

    Vector2::Vector2(float Inx)
    {
        x = Inx;
        y = Inx;
    }

    Vector2::Vector2(float Inx, float Iny)
    {
        x = Inx;
        y = Iny;
    }

    Vector2 Vector2::operator+(const Vector2 &other) const
    {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 Vector2::operator*(const Vector2 &other) const
    {
        return Vector2(x * other.x, y * other.y);
    }

}