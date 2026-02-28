#pragma once
#include <queue>
#include <vector>
#include "Core/Object.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "Renderer/RenderStruct.h"
#include "Resources/Asset.h"
#include <unordered_set>
#include "SceneStruct.h"

namespace EngineCore
{
    class Scene : public Resource
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update(uint32_t frameIndex);
        void EndFrame();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void Scene::DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
        inline Camera* GetMainCamera() { return mainCamera; }
        
        // 只在GameObject层用到，析构和SetParent的时候， 创建的时候调用
        void RemoveGameObjectToSceneList(GameObject* object);
        void AddGameObjectToSceneList(GameObject* object);

        void AddRootGameObject(GameObject* object);
        void TryRemoveRootGameObject(GameObject* object);

        void PushNewTransformDirtyRoot(Transform* transform);

        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRemoveInvalidDirtyRenderNode();

        uint32_t CreateRenderNode();
        
        void DeleteRenderNode(MeshRenderer *renderer);
        void MarkNodeCreated(MeshRenderer* renderer);
        void MarkNodeTransformDirty(Transform* transform);
        void MarkNodeMeshFilterDirty(MeshFilter* meshFilter);
        void MarkNodeMeshRendererDirty(MeshRenderer* renderer);
        void MarkNodeRenderableDirty(GameObject* object);
        
        inline std::vector<uint32_t>& GetPerFrameDirtyNodeList(){ return mPerFrameDirtyNodeList;}
        inline std::vector<uint32_t>& GetNodeChangeFlagList(){ return mNodeChangeFlagList;}    
        inline std::vector<NodeDirtyPayload>& GetNodeDirtyPayloadList(){ return mNodeDirtyPayloadList;}   

        inline void SetCurrentFrame(uint32_t currentFrameIndex)
        {
            mCurrentFrame = currentFrameIndex;
        }
    public:
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;

        std::vector<Transform*> dirtyRootDepthBucket[64];
        
    private:
        uint32_t mCurrentFrame = 0;
        void ApplyQueueNodeChange(uint32_t id, uint32_t flags, const NodeDirtyPayload& p);
        void InternalMarkNodeDeleted(MeshRenderer* renderer);
        
        std::vector<uint32_t> mNodeFrameStampList;
        std::vector<uint32_t> mNodeChangeFlagList;
        std::vector<NodeDirtyPayload> mNodeDirtyPayloadList;


        std::vector<uint32_t> mPerFrameDirtyNodeList;
        
        uint32_t mCurrSceneIndex = 0;
        std::vector<uint32_t> mFreeSceneIndex;
        std::vector<uint32_t> mPendingFreeSceneIndex;

        void EnsureNodeQueueSize(uint32_t size);
        void ClearPerFrameData();
        void ClearDirtyRootTransform();
        void PushLastFrameFreeIndex();
    };    
} // namespace EngineCore
