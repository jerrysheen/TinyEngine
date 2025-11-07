#include "PreCompiledHeader.h"
#include "MetaFactory.h"
#include "GameObject/GameObject.h"

namespace EngineCore 
{
    GameObject* MetaFactory::CreateGameObjectFromMeta(const json& source)
    {
        GameObject* go = new GameObject();
        go->name = source.at("Name").get<string>();
        go->enabled = source.at("Enabled").get<bool>();

        // Create Component
        for (const auto& compJson : source.at("Component"))
        {
            Component* comp = MetaFactory::CreateComponent(compJson, go);
            go->AddComponent(comp);
        }

        // Create Child Go
        const json& childrenArray = source.at("ChildGameObject");
        for (const auto& childGo : childrenArray)
        {
            GameObject* child = CreateGameObjectFromMeta(childGo);
            child->SetParent(go);
        }

        return go;
    }
}
