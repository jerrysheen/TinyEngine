#include "PrecompiledHeader.h"
#include "AABB.h"
#include "Vector4.h"

namespace EngineCore
{
    void AABB::Transform(const Matrix4x4 &matrix)
    {
        // 1. 计算中心点和半长宽高 (Extents)
        Vector3 center = (minValue + maxValue) * 0.5f;
        Vector3 extents = (maxValue - minValue) * 0.5f;

        // 2. 变换中心点 (应用旋转、缩放、平移)
        // 注意：平移包含在 center 的变换里
        Vector4 newCenterV4 = Matrix4x4::Multiply(matrix, Vector4(center.x, center.y, center.z, 1.0f));
        Vector3 newCenter = Vector3(newCenterV4.x, newCenterV4.y, newCenterV4.z);

        // 3. 计算新的 Extents (核心逻辑：绝对值求和)
        // 只要 3x3 矩阵部分，因为平移不影响盒子大小
        Vector3 newExtents;
        
        // X轴的新半径 = 原X、Y、Z轴半径在 新X轴 上的投影长度之和, 想象正方向旋转45°
        newExtents.x = std::abs(matrix.m00) * extents.x + 
                    std::abs(matrix.m01) * extents.y + 
                    std::abs(matrix.m02) * extents.z;

        // Y轴的新半径
        newExtents.y = std::abs(matrix.m10) * extents.x + 
                    std::abs(matrix.m11) * extents.y + 
                    std::abs(matrix.m12) * extents.z;

        // Z轴的新半径
        newExtents.z = std::abs(matrix.m20) * extents.x + 
                    std::abs(matrix.m21) * extents.y + 
                    std::abs(matrix.m22) * extents.z;

        // 4. 重组 Min/Max
        minValue = newCenter - newExtents;
        maxValue = newCenter + newExtents;
    }
}