#include "PreCompiledHeader.h"
#include "CameraController.h"
#include "GameObject/GameObject.h"


namespace EngineCore
{
    CameraController::CameraController(GameObject* go)
    {
        gameObject = go;
    }

    void CameraController::Update()
    {
    }

    const char *CameraController::GetScriptName() const
    {
        return "CameraController";;
    }

}