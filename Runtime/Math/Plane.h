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

        // 规定大于0为正向，
        // 假设 0，0，-1， 200， 可以得到平面为正方向为 -z，在z轴200处。
        // p(0, 0, 190)，在平面内， 所以是相加大于0在平面内
        float GetDistanceToPoint(const Vector3& point)
        {
            return (Vector3::Dot(normal, point) + distance);
        }
    };
}