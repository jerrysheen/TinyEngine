#include "PreCompiledHeader.h"
#include "Vector3.h"

namespace EngineCore
{
    Vector3 Vector3::One = Vector3(1.0);
    Vector3 Vector3::Zero = Vector3(0.0);

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

}