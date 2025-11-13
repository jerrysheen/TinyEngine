#pragma once
#include "Matrix4x4.h"
#include "Plane.h"
#include "AABB.h"

namespace EngineCore
{
    enum class IntersectResult
    {
        Inside = 0,
        Outside = 1,
        Intersect = 2
    };

    class Frustum
    {
    public:
        Plane frustumPlane[6];
        Frustum() = default;
        void UpdateFrustumPlane(const Matrix4x4& matrix);
        IntersectResult TestAABB(const AABB& bound);
    };
}