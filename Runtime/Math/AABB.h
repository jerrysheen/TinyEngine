#pragma once
#include "Vector3.h"
#include <iostream>
#include <algorithm>  // for std::min, std::max
#include "Matrix4x4.h"

namespace EngineCore
{
    struct AABB
    {
    public:
        Vector3 minValue{FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 maxValue{-FLT_MAX, -FLT_MAX, -FLT_MAX};

        AABB() = default;
        AABB(const Vector3& minPoint, const Vector3& maxPoint)
            : minValue(minPoint), maxValue(maxPoint) {}
        
        Vector3 GetCenter() const 
        {
            return (minValue + maxValue) * 0.5f;
        }

        Vector3 GetExtents() const 
        {
            return (maxValue - minValue) * 0.5f;
        }

        /// 扩展包围盒以包含一个点
        void Encapsulate(const Vector3& point)
        {
            minValue.x = std::min(minValue.x, point.x);
            minValue.y = std::min(minValue.y, point.y);
            minValue.z = std::min(minValue.z, point.z);
            
            maxValue.x = std::max(maxValue.x, point.x);
            maxValue.y = std::max(maxValue.y, point.y);
            maxValue.z = std::max(maxValue.z, point.z);
        }

        /// Transform变换后的AABB（考虑旋转和缩放）
        void Transform(const Matrix4x4& matrix);
    };
}
