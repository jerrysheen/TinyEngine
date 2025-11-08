#include "PreCompiledHeader.h"
#include "CameraController.h"
#include "GameObject/GameObject.h"
#include "json.hpp"
#include "Serialization/ComponentFactory.h"


REGISTER_SCRIPT(CameraController)
namespace EngineCore
{
    using json = nlohmann::json;
    CameraController::CameraController(GameObject* go)
    {
        gameObject = go;
    }

    void CameraController::Update()
    {
        std::cout << testVal << std::endl;
    }

    const char *CameraController::GetScriptName() const
    {
        return "CameraController";;
    }

    json CameraController::SerializedFields() const
    {
        json j = 
        {
            {"TestVal" , testVal},
        };
        
        return j;
    }

    void CameraController::DeserializedFields(const json &j)
    {
        if(j.contains("TestVal")) testVal = j["TestVal"];
    }
}