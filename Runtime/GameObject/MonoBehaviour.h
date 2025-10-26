#include "Component.h"

namespace EngineCore
{
    class GameObject;
    class MonoBehaviour : Component
    {
        MonoBehaviour() = default;
        MonoBehaviour(GameObject* parent);

        // 非纯虚， 不一定要实现
        virtual void Awake() {};
        virtual void Start() {};
        virtual void Update() {};
        virtual void LateUpdate() {};
        virtual void OnDestory() {};

    };
}