#pragma once
#include "Vector3.h"

namespace EngineCore
{
    struct Plane
    {
        // Ax + By + Cz + D = 0 ;
        // normal = (A, B, C) / sqrt(a*a + b*b + c*c);
        // distance = normal * p
        // (A, B, C) / sqrt(a*a + b*b + c*c) * p = distance
        // 又  (A, B, C) · p = -D
        // 同除 sqrt(a*a + b*b + c*c)，左边为distance
        Vector3 normal;
        float distance;
        Plane() = default;
        Plane(const Vector3& normal, float distance);

        float GetDistanceToPoint(const Vector3& point)
        {
            return (Vector3::Dot(normal, point) - distance);
        }
    };
}