#pragma once
#include <string>
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "GameObject/Component.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "GameObject/MonoBehaviour.h"
#include "GameObject/Transform.h"
#include "json.hpp"
#include "Serialization/ScriptFactory.h"
#include "GameObjectAndComponentSerialization.h"

// 提供各种反序列化操作工厂，比如输入一个json，输出一个反序列化完的GameObject
namespace EngineCore
{
    class MetaFactory
    {
    public:
        using json = nlohmann::json;

        static GameObject* CreateGameObjectFromMeta(const json& json);

        inline static Component* CreateComponent(const json& source, GameObject* go)
        {
            ComponentType type = source.at("Type").get<ComponentType>();
            const json& data  = source.at("Data");

            switch(type)
            {
                case ComponentType::Camera: return CreateComponentHelper<Camera>(data, go);
                case ComponentType::MeshFilter:   return CreateComponentHelper<MeshFilter>(data, go);
                case ComponentType::MeshRenderer: return CreateComponentHelper<MeshRenderer>(data, go);
                case ComponentType::Transform:    return CreateComponentHelper<Transform>(data, go);
                case ComponentType::Script: return CreateScriptComponentHelper(data, go);
            }
            return nullptr;
        }

        template<typename T>
        inline static Component* CreateComponentHelper(const json& json, GameObject* go)
        {
            T* comp = new T();
            *comp = json.get<T>();
            comp->gameObject = go;
            return comp;
        }

        inline static Component* CreateScriptComponentHelper(const json& json, GameObject* go)
        {
            string scriptName = json["ScriptName"];
            MonoBehaviour* script = ScriptFactory::Create(scriptName, go);
            script->DeserializedFields(json["Fields"]);
            return script;
        }
        
        // 模板特化，针对Transform
        template<>
        inline static Component* MetaFactory::CreateComponentHelper<Transform>(const json& source, GameObject* go)
        {
            Transform* comp = new Transform();
            *comp = source.get<Transform>();
            comp->gameObject = go;
            go->transform = comp;
            return comp;
        }

    };
    

}