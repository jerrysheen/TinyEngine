#pragma once
#include "GameObject/MonoBehaviour.h"
#include "json.hpp"

namespace EngineCore
{
    using json = nlohmann::json;
    class CameraController : public MonoBehaviour
    {
    public:
        CameraController() = default;
        CameraController(GameObject* go);
        virtual ~CameraController() override {};

        virtual void Update() override;
        virtual const char* GetScriptName() const override;
        virtual json SerializedFields() const override;
        virtual void DeserializedFields(const json& j) override;
    public:
        float testVal = 0;
    };
}