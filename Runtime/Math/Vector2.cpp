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

}