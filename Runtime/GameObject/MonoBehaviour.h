#pragma once
#include "Component.h"
#include "json.hpp"

namespace EngineCore
{
    using json = nlohmann::json;
    class GameObject;
    class MonoBehaviour : public Component
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
        static ComponentType GetStaticType() { return ComponentType::Script; }
        virtual ComponentType GetType() const override{ return ComponentType::Script; };
        
        virtual const char* GetScriptName() const = 0;
        virtual json SerializeFields() const { return json::object(); }
        virtual void DeserializedFields(const json& j) {};
    protected:


    };
}