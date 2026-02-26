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

    void Scene::Update(uint32_t frameIndex)
    {
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
            meshFilter == nullptr ? AssetID() : meshFilter->mMeshHandle.GetAssetID(),
            renderer->GetMaterial().GetAssetID());
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
                                  meshFilter->mMeshHandle->GetAssetID(),
                                  AssetID{});
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::MeshDirty, payload);
    }

    void Scene::MarkNodeMeshRendererDirty(MeshRenderer *renderer)
    {
        uint32_t renderID = renderer->GetCPUWorldIndex();
        NodeDirtyPayload payload(renderer->gameObject->transform,
            AssetID{},
            renderer->GetMaterial().GetAssetID());
        ApplyQueueNodeChange(renderID, (uint32_t)NodeDirtyFlags::MaterialDirty, payload);
    }

    void Scene::MarkNodeRenderableDirty(GameObject *object)
    {
        MeshRenderer* mr  = object->GetComponent<MeshRenderer>();
        if(mr == nullptr) return;
        uint32_t renderID = mr->GetCPUWorldIndex();


        Transform* transform = object->transform;
        MeshFilter* meshFilter = object->GetComponent<MeshFilter>();
        NodeDirtyPayload payload(transform, 
            meshFilter == nullptr ? AssetID() : meshFilter->mMeshHandle.GetAssetID(),
            mr->GetMaterial().GetAssetID());
        uint32_t flags = ((uint32_t)NodeDirtyFlags::MaterialDirty | (uint32_t)NodeDirtyFlags::TransformDirty);
        if(meshFilter) 
        {
            flags |= (uint32_t)NodeDirtyFlags::MeshDirty;
        }
        ApplyQueueNodeChange(renderID, flags, payload);
    }

    void Scene::EnsureNodeQueueSize(uint32_t id)
    {
        const size_t need = static_cast<size_t>(id) + 1;
        if(mNodeFrameStampList.size() < need)
        {
            mNodeFrameStampList.resize(need, 0);
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

    void Scene::ApplyQueueNodeChange(uint32_t id, uint32_t flags, const NodeDirtyPayload& p)
    {
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