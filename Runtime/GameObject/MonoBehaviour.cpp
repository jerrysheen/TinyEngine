#include "PreCompiledHeader.h"
#include "MonoBehaviour.h"
#include "GameObject.h"
namespace EngineCore
{
    MonoBehaviour::MonoBehaviour(GameObject* go)
    {
        gameObject = go;
    }

};