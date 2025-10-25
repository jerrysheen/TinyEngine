#include "PreCompiledHeader.h"
#include "Transform.h"

namespace EngineCore
{
    Transform::Transform(GameObject* parent)
    {
        mParentGO = parent;
    }
}