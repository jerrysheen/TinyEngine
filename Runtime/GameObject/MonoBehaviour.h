#pragma once
#include "Component.h"

namespace EngineCore
{
    class GameObject;
    class MonoBehaviour : Component
    {
    public:
        MonoBehaviour() = default;
        MonoBehaviour(GameObject* parent);
        virtual ~MonoBehaviour() override {};
        // 非纯虚， 不一定要实现
        virtual void Awake() {};
        virtual void Start() {};
        virtual void Update() {};
        virtual void LateUpdate() {};
        virtual void OnDestroy() {};

    };
}