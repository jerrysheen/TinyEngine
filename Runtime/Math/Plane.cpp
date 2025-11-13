#include "PreCompiledHeader.h"
#include "Plane.h"

namespace EngineCore
{
    Plane::Plane(const Vector3 &normal, float distance)
        :normal(normal), distance(distance)
    {
    }
}
