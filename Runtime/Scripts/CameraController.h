#pragma once
#include "GameObject/MonoBehaviour.h"

namespace EngineCore
{
    class CameraController : public MonoBehaviour
    {
    public:
        CameraController() = default;
        CameraController(GameObject* go);
        virtual ~CameraController() override {};

        virtual void Update() override;
        virtual const char* GetScriptName() const override;
    public:
        float testVal = 0;
    };
}