#include "PrecompiledHeader.h"
#include "AABB.h"
#include "Vector4.h"

namespace EngineCore
{
    void AABB::Transform(const Matrix4x4 &matrix) const
    {
        Matrix4x4::Multiply(matrix, Vector4(minValue.x, minValue.y, minValue.z, 0.0f));
        Matrix4x4::Multiply(matrix, Vector4(maxValue.x, maxValue.y, maxValue.z, 0.0f));
        return;
    }
}