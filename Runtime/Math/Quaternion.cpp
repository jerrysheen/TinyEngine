#include "PreCompiledHeader.h"
#include "Quaternion.h"

namespace EngineCore
{
    Quaternion::Quaternion(float x, float y, float z, float w)
    {
    }

    Quaternion Quaternion::FromRotationMatrix(const Vector3 &col0, const Vector3 &col1, const Vector3 &col2)
    {
        return Quaternion();
    }

} // namespace EngineCore
