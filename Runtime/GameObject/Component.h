#pragma once
#include "ComponentType.h"
#include "json.hpp"

namespace EngineCore
{
    using json = nlohmann::json;

    class GameObject;
    class Component
    {
    public:
        Component(){};
        virtual ~Component() = 0;
        virtual ComponentType GetType() const = 0;
        
        GameObject* gameObject = nullptr;
        bool enabled = true;

        // 每个类需要自己实现序列化和反序列化方法。
        virtual const char* GetScriptName() const = 0;
        virtual json SerializedFields() const { return json::object(); }
        virtual void DeserializedFields(const json& j) {};
    };
}