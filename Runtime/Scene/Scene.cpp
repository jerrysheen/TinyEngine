#include "PreCompiledHeader.h"
#include "Scene.h"
#include "GameObject/GameObject.h"

namespace EngineCore
{
    void Scene::Open()
    {
        for(auto* go : objLists)
        {
            if(go != nullptr)
            {
                for(auto& script : go->scripts)
                {
                    script->Awake();
                }
            }
        }
    }

    void Scene::Update()
    {
        for(auto* go : objLists)
        {
            if(go != nullptr)
            {
                for(auto& script : go->scripts)
                {
                    script->Update();
                }
            }
        }

        // Transform延迟更新

        UpdateAllTransforms();
    }

    GameObject *Scene::FindGameObject(const std::string &name)
    {
        for(GameObject* go : objLists)
        {
            if(go->name == name) return go;
        }
        return nullptr;
    }

    GameObject *Scene::CreateGameObject(const std::string& name)
    {
        GameObject* obj = new GameObject(name);
        objLists.push_back(obj);
        return obj;
    }

    // toso: priority 为最大的默认就是mainCamera
    void Scene::AddCamToStack(Camera* cam)
    {
        //todo : 按照priority排序camera渲染顺序。
        mainCamera = cam;
        cameraStack.push_back(cam);
    }


    void Scene::UpdateAllTransforms()
    {
        for(auto* go : objLists)
        {
            if(go != nullptr && go->enabled)
            {
                if(go->transform->isDirty)
                {
                    go->transform->UpdateTransform();
                }
            }
        }
    }

    
}