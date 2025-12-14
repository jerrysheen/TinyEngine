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

    Scene* MetaFactory::CreateSceneFromMeta(const json& source)
    {
        Scene* scene = new Scene();
        // todo: 判断是否是主场景
        SceneManager::GetInstance()->SetCurrentScene(scene);
        scene->name = source.at("Name").get<string>();

        // Create Component
        for (const auto& goJson : source.at("RootObject"))
        {
            GameObject* go = MetaFactory::CreateGameObjectFromMeta(goJson);
            scene->AddGameObjectToSceneList(go);


            // todo： 根据priority设置maincamera：
            if (go->GetComponent<Camera>() != nullptr) 
            {
                scene->SetMainCamera(go->GetComponent<Camera>());
            }
        }


        return scene;
    }

    
    Component *MetaFactory::CreateComponent(const json &source, GameObject *go)
    {
        string componentName  = source.at("Type");
        json data  = source.at("Data");
        // 特殊处理Transform：不创建新的，直接反序列化现有的
        if(componentName == "Transform")
        {
            Transform* existingTransform = go->GetComponent<Transform>();
            if(existingTransform != nullptr)
            {
                existingTransform->DeserializedFields(data);
                return existingTransform;
            }
        }
        
        // 其他组件正常创建
        Component* component = ComponentFactory::Create(componentName, go);
        component->DeserializedFields(data);
        return component;
    }
}
