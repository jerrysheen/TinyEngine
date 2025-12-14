#include "PreCompiledHeader.h"
#include "Scene.h"
#include "GameObject/GameObject.h"
#include "GameObject/Transform.h"
#include "GameObject/MeshRenderer.h"
#include "Graphics/Material.h"

namespace EngineCore
{
    Scene::Scene()
    {
        std::queue<uint32_t> empty;
        std::swap(empty, m_FreeSceneNode);
        renderSceneData.isDataValidList.resize(10000, false);
        renderSceneData.meshRendererList.resize(10000);
        renderSceneData.vaoIDList.resize(10000, UINT32_MAX);
        renderSceneData.aabbList.resize(10000);
        renderSceneData.objectToWorldMatrixList.resize(10000);
        renderSceneData.layerList.resize(10000);
        renderSceneData.needUpdateList.resize(10000, false);
    }

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
        RunLogicUpdate();
        RunTransformUpdate();
        RunRendererSync();
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

    void Scene::RunLogicUpdate()
    {
        for(auto* go : allObjList)
        {
            if(go != nullptr)
            {
                for(auto& script : go->scripts)
                {
                    script->Update();
                }
                if (go->GetComponent<Camera>() != nullptr) 
                {
                    go->GetComponent<Camera>()->Update();
                }
            }
        }
    }

    void Scene::RunTransformUpdate()
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

    void Scene::RunRendererSync()
    {
        for(auto* go : allObjList)
        {
            if(go != nullptr && go->enabled)
            {
                auto* meshRenderer = go->GetComponent<MeshRenderer>();
                auto* meshFilter = go->GetComponent<MeshFilter>();
                if(meshRenderer && meshFilter) 
                {
                    bool needSync = false;
                    uint32_t transformVersion = meshRenderer->gameObject->transform->transformVersion;
                    uint32_t lastSyncVersion = meshRenderer->lastSyncTransformVersion;
                    
                    // 1. Transform 发生变化，需要更新 Bounds 和 Matrix
                    if(transformVersion != lastSyncVersion )
                    {
                        meshRenderer->UpdateBounds(meshFilter->mMeshHandle.Get()->bounds, meshRenderer->gameObject->transform->GetWorldMatrix());
                        meshRenderer->lastSyncTransformVersion = transformVersion;
                        needSync = true;
                    } 
                    
                    int index = meshRenderer->sceneRenderNodeIndex;
                    needSync = needSync || renderSceneData.vaoIDList[index] == UINT32_MAX;

                    if(needSync)
                    {
                        renderSceneData.SyncData(meshRenderer, meshRenderer->sceneRenderNodeIndex);
                    }
                }
            }
        }

    }

    int Scene::AddNewRenderNodeToCurrentScene(MeshRenderer *renderer)
    {
        uint32_t index = 0;
        if(!m_FreeSceneNode.empty())
        {
            index = m_FreeSceneNode.front();
            m_FreeSceneNode.pop();
        }
        else
        {
            index = m_CurrentSceneRenderNodeIndex;
            m_CurrentSceneRenderNodeIndex++;
        }
        renderSceneData.SyncData(renderer, index);
        m_CurrentSceneMaxRenderNode++;
        return index;
    }

    void Scene::DeleteRenderNodeFromCurrentScene(uint32_t index)
    {
        m_FreeSceneNode.push(index);
        renderSceneData.DeleteData(index);
        m_CurrentSceneMaxRenderNode--;
    }
}