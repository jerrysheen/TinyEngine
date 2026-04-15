#include "PreCompiledHeader.h"
#include "Scene.h"
#include "GameObject/GameObject.h"
#include "GameObject/Transform.h"
#include "GameObject/MeshRenderer.h"
#include "Graphics/Material.h"
#include "Renderer/RenderEngine.h"
#include "Scene/CPUScene.h"

namespace EngineCore
{
    Scene::Scene()
    {
        std::queue<uint32_t> empty;
    }

    Scene::~Scene()
    {
        // 退出时先打断层级和 scene 反向引用，避免析构期间互相修改容器。
        std::vector<GameObject*> objectsToDelete = allObjList;
        for (GameObject* gameObject : objectsToDelete)
        {
            if (gameObject == nullptr || gameObject->transform == nullptr)
            {
                continue;
            }

            for (Transform* child : gameObject->transform->childTransforms)
            {
                if (child != nullptr)
                {
                    child->parentTransform = nullptr;
                }
            }

            gameObject->transform->childTransforms.clear();
            gameObject->transform->parentTransform = nullptr;
            gameObject->SetOwnerScene(nullptr);
        }

        // 先清空列表，防止渲染线程访问到野指针
        rootObjList.clear();
        allObjList.clear();
        cameraStack.clear();
        
        // 再删除对象
        for (GameObject* gameObject : objectsToDelete)
        {
            delete gameObject;
        }
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

    void Scene::TickSimulation(uint32_t frameIndex)
    {
        PROFILER_ZONE("MainThread::Scene::TickSimulation");
        mCurrentFrame = frameIndex;
        PushLastFrameFreeIndex();
        RunLogicUpdate();
        RunTransformUpdate();
        RunRemoveInvalidDirtyRenderNode();
    }

    void Scene::EndFrame()
    {
        ClearPerFrameData();
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
        allObjList.push_back(obj);
        rootObjList.push_back(obj);
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

    void Scene::PushNewTransformDirtyRoot(Transform *transform)
    {
        ASSERT(transform);
        dirtyRootDepthBucket[transform->GetNodeDepth()].push_back(transform);
    }

    void Scene::RunLogicUpdate()
    {
        //for(auto* go : allObjList)
        //{
        //    if(go != nullptr)
        //    {
        //        for(auto& script : go->scripts)
        //        {
        //            script->Update();
        //        }
        //        if (go->GetComponent<Camera>() != nullptr) 
        //        {
        //            go->GetComponent<Camera>()->Update();
        //        }
        //    }
        //}
    }

    void Scene::RunTransformUpdate()
    {
        for(auto transformList : dirtyRootDepthBucket)
        {
            for(int i = 0 ; i < transformList.size(); i++)
            {
                transformList[i]->UpdateRecursively(mCurrentFrame);
            }
        }

        for (auto& rootNode : dirtyRootDepthBucket)
        {
            rootNode.clear();
        }
    }



    void Scene::RunRemoveInvalidDirtyRenderNode()
    {
        vector<uint32_t> dirtyNodeList;
        for(auto renderID : mPerFrameDirtyNodeList)
        {
            uint32_t flags = mNodeChangeFlagList[renderID];
            if((flags & (uint32_t)NodeDirtyFlags::Created) && (flags & (uint32_t)NodeDirtyFlags::Destory)) {
                continue;
            }
            dirtyNodeList.push_back(renderID);
        }
        mPerFrameDirtyNodeList = dirtyNodeList;
    }

    uint32_t Scene::CreateRenderNode()
    {
        if(mFreeSceneIndex.size() > 0)
        {
            uint32_t index = mFreeSceneIndex.back();
            mFreeSceneIndex.pop_back();
            return index;
        }
        return mCurrSceneIndex++;
    }

    void Scene::DeleteRenderNode(MeshRenderer *renderer)
    {
        InternalMarkNodeDeleted(renderer);
        uint32_t renderID = renderer->GetCPUWorldIndex();
        mPendingFreeSceneIndex.push_back(renderID);
    }

    void Scene::MarkNodeCreated(MeshRenderer *renderer)
    {
        uint32_t renderID = renderer->GetCPUWorldIndex();
        EnsureNodeQueueSize(renderID);

        Transform* transform = renderer->gameObject->transform;

        MeshFilter* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
        NodeDirtyPayload payload(transform, 
            GetMeshID(meshFilter),
            GetMaterialID(renderer));
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::Created, payload);
    }


    void Scene::InternalMarkNodeDeleted(MeshRenderer *renderer)
    {
        uint32_t renderID = renderer->GetCPUWorldIndex();

        NodeDirtyPayload payload{};
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::Destory, payload);
    }

    void Scene::MarkNodeTransformDirty(Transform *transform)
    {
        MeshRenderer* renderer = transform->gameObject->GetComponent<MeshRenderer>();
        if (!renderer) return;
        uint32_t renderID = renderer->GetCPUWorldIndex();

        NodeDirtyPayload payload(transform);
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::TransformDirty, payload);        
    }

    void Scene::MarkNodeMeshFilterDirty(MeshFilter *meshFilter)
    {
        MeshRenderer* renderer = meshFilter->gameObject->GetComponent<MeshRenderer>();
        if(!renderer) return;
        uint32_t renderID = renderer->GetCPUWorldIndex();
        NodeDirtyPayload payload(meshFilter->gameObject->transform,
            GetMeshID(meshFilter),
            GetMaterialID(renderer));
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::MeshDirty, payload);
    }

    void Scene::MarkNodeMeshRendererDirty(MeshRenderer *renderer)
    {
        MeshFilter* meshFiler = renderer->gameObject->GetComponent<MeshFilter>();
        uint32_t renderID = renderer->GetCPUWorldIndex();
        NodeDirtyPayload payload(renderer->gameObject->transform,
            GetMeshID(meshFiler),
            GetMaterialID(renderer));
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::MaterialDirty, payload);
    }

    void Scene::MarkNodeRenderableDirty(GameObject *object)
    {
        MeshRenderer* meshRenderer  = object->GetComponent<MeshRenderer>();
        if(meshRenderer == nullptr) return;
        uint32_t renderID = meshRenderer->GetCPUWorldIndex();


        Transform* transform = object->transform;
        MeshFilter* meshFilter = object->GetComponent<MeshFilter>();
        NodeDirtyPayload payload(transform, 
            GetMeshID(meshFilter),
            GetMaterialID(meshRenderer));
        uint32_t flags = ((uint32_t)NodeDirtyFlags::MaterialDirty | (uint32_t)NodeDirtyFlags::TransformDirty);
        if(meshFilter) 
        {
            flags |= (uint32_t)NodeDirtyFlags::MeshDirty;
        }
        ApplyQueueNodeChange(renderID, flags, payload);
    }

    SceneDelta Scene::FlushSceneDelta()
    {
        SceneDelta delta;
        delta.mPerFrameDirtyNodeList = std::move(mPerFrameDirtyNodeList);
        delta.mNodeChangeFlagList = std::move(mNodeChangeFlagList);
        delta.mNodeDirtyPayloadList = std::move(mNodeDirtyPayloadList);
            
        // Scene 自己仍然会在后续帧继续按 renderID 下标写入这两份数组，
        // move 之后必须立刻恢复到与 frame stamp 一致的容量。
        mNodeChangeFlagList.resize(mNodeFrameStampList.size(), 0);
        mNodeDirtyPayloadList.resize(mNodeFrameStampList.size());

        return delta;
    }

    void Scene::EnsureNodeQueueSize(uint32_t id)
    {
        const size_t need = static_cast<size_t>(id) + 1;
        if(mNodeFrameStampList.size() < need)
        {
            mNodeFrameStampList.resize(need, UINT32_MAX);
            mNodeChangeFlagList.resize(need, 0);
            mNodeDirtyPayloadList.resize(need);
        }
    }

    void Scene::ClearPerFrameData()
    {
        mPerFrameDirtyNodeList.clear();
    }

    void Scene::PushLastFrameFreeIndex()
    {
        if (!mPendingFreeSceneIndex.empty()) 
        {
            mFreeSceneIndex.insert(
                mFreeSceneIndex.end(), 
                mPendingFreeSceneIndex.begin(), 
                mPendingFreeSceneIndex.end()
            );
            
            // 2. 清空隔离区，准备接收下一帧的删除请求
            mPendingFreeSceneIndex.clear();
        }
    }

    AssetID Scene::GetMaterialID(MeshRenderer *meshRenderer)
    {
        if(meshRenderer == nullptr) return AssetID();
        AssetID id = meshRenderer->GetMaterial().GetAssetID();
        return ResourceManager::GetInstance()->GetResource<Material>(id)->GetAssetID();
    }

    AssetID Scene::GetMeshID(MeshFilter *meshFilter)
    {
        if(meshFilter == nullptr) return AssetID();
        AssetID id = meshFilter->mMeshHandle.GetAssetID();
        return ResourceManager::GetInstance()->GetResource<Mesh>(id)->GetAssetID();
    }

    void Scene::ApplyQueueNodeChange(uint32_t id, uint32_t flags, const NodeDirtyPayload& p)
    {
        EnsureNodeQueueSize(id);
        if(mNodeFrameStampList[id] != mCurrentFrame)
        {
            // 这一帧还没创建，重置所有：
            mPerFrameDirtyNodeList.push_back(id);
            mNodeFrameStampList[id] = mCurrentFrame;
            mNodeChangeFlagList[id] = 0;
            mNodeDirtyPayloadList[id] = {};
        }
        mNodeChangeFlagList[id] |= flags;

        // 直接更新现在状态： 直接强刷
        if(flags & (uint32_t)NodeDirtyFlags::Created)
        {
            mNodeDirtyPayloadList[id].transform = p.transform;
            mNodeDirtyPayloadList[id].materialID = p.materialID;
            mNodeDirtyPayloadList[id].meshID = p.meshID.IsValid() ?  p.meshID : mNodeDirtyPayloadList[id].meshID;
        }

        if(flags & (uint32_t)NodeDirtyFlags::Destory)
        {
            mNodeDirtyPayloadList[id] = {};
        }

        if(flags & (uint32_t)NodeDirtyFlags::MaterialDirty)
        {
            mNodeDirtyPayloadList[id].materialID = p.materialID;
        }

        if(flags & (uint32_t)NodeDirtyFlags::MeshDirty)
        {
            mNodeDirtyPayloadList[id].meshID = p.meshID;
        }

        if(flags & (uint32_t)NodeDirtyFlags::TransformDirty)
        {
            mNodeDirtyPayloadList[id].transform = p.transform;
        }

    }
}
