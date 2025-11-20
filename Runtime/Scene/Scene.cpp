#include "PreCompiledHeader.h"
#include "Scene.h"
#include "GameObject/GameObject.h"
#include "GameObject/Transform.h"
#include "GameObject/MeshRenderer.h"
#include "Graphics/Material.h"
#include "Graphics/MaterialPropertyBlock.h"

namespace EngineCore
{
    Scene::~Scene()
    {
        // 先清空列表，防止渲染线程访问到野指针
        rootObjList.clear();
        
        // 再删除对象
        for(int i = 0; i < allObjList.size(); i++)
        {
            delete allObjList[i];
        }
        allObjList.clear();
        mainCamera = nullptr;

        // 析构的时候不应该去Remove，Remove应该是在外面
        //SceneManager::GetInstance()->RemoveScene(name);
    }

    void Scene::Open()
    {
        for(auto* go : allObjList)
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
        for(auto* go : allObjList)
        {
            if(go != nullptr)
            {
                for(auto& script : go->scripts)
                {
                    script->Update();
                }
               //temp:
                if (go->GetComponent<Camera>() != nullptr) 
                {
                    go->GetComponent<Camera>()->Update();
                }
            }

        }

        // Transform延迟更新

        UpdateAllTransforms();

        //todo: temp: 更新材质信息，这部分应该在别的地方做：
        UpdatePerMaterialData();

        
    }

    GameObject* Scene::FindGameObject(const std::string &name)
    {
        for(GameObject* go : allObjList)
        {
            if(go->name == name) return go;
        }
        return nullptr;
    }

    GameObject* Scene::CreateGameObject(const std::string& name)
    {
        GameObject* obj = new GameObject(name, this);
        return obj;
    }

    void Scene::DestroyGameObject(const std::string& name)
    {
        auto* gameObject = FindGameObject(name);
        ASSERT(gameObject != nullptr);
        auto childTransformList = gameObject->transform->childTransforms;
        if (childTransformList.size() > 0)
        {
            for(auto* child : childTransformList) 
            {
                ASSERT(child != nullptr);
                DestroyGameObject(child->gameObject->name);
            }
        }

        delete gameObject;
    }

    // toso: priority 为最大的默认就是mainCamera
    void Scene::AddCamToStack(Camera* cam)
    {
        //todo : 按照priority排序camera渲染顺序。
        mainCamera = cam;
        cameraStack.push_back(cam);
    }

    void Scene::RemoveGameObjectToSceneList(GameObject* gameObject)
    {
        // swap and- pop A：
        auto it = std::find(allObjList.begin(), allObjList.end(), gameObject);
        if(it != allObjList.end())
        {
            *it = allObjList.back();
            allObjList.pop_back();
        }
        auto rootIt = std::find(rootObjList.begin(), rootObjList.end(), gameObject);
        if(rootIt != rootObjList.end())
        {
            *rootIt = rootObjList.back();
            rootObjList.pop_back();
        }
    }

    // 默认AddGameObject的时候，都是在根下创建
    void Scene::AddGameObjectToSceneList(GameObject *gameObject)
    {
        allObjList.push_back(gameObject);
        rootObjList.push_back(gameObject);
    }

    void Scene::AddRootGameObject(GameObject* gameObject)
    {
        rootObjList.push_back(gameObject);
    }

    void Scene::TryRemoveRootGameObject(GameObject* gameObject)
    {
        auto rootIt = std::find(rootObjList.begin(), rootObjList.end(), gameObject);
        if (rootIt != rootObjList.end())
        {
            *rootIt = rootObjList.back();
            rootObjList.pop_back();
        }
    }

    void Scene::UpdateAllTransforms()
    {
        for(auto* go : allObjList)
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

    // todo ： 材质数据的更新放在渲染处
    void Scene::UpdatePerMaterialData()
    {
         for (auto& go : allObjList) 
         {
             auto* transform = go->GetComponent<Transform>();
             auto* meshRenderer = go->GetComponent<MeshRenderer>();

             if (transform && meshRenderer) 
             {
                 auto mpb = meshRenderer->GetMaterialPropertyBlock();
                 mpb.SetValue("WorldMatrix", transform->GetWorldMatrix());
                 //material->SetMatrix4x4("WorldMatrix", transform->GetWorldMatrix());
             }
         }
    }
}