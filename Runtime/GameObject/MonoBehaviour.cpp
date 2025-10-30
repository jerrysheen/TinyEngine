#include "PreCompiledHeader.h"
#include "MonoBehaviour.h"
#include "GameObject.h"
namespace EngineCore
{
    MonoBehaviour::MonoBehaviour(GameObject* parent)
    {
        mGO = parent;
    }

};