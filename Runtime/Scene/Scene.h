#pragma once
#include <queue>
#include <vector>
#include "Core/Object.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
namespace EngineCore
{
    struct RenderSceneData
    {
        vector<bool> isDataValidList;
        vector<bool> needUpdateList;
        vector<MeshRenderer*> meshRendererList;
        vector<uint32_t> vaoIDList;
        vector<AABB> aabbList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;
        struct RenderSceneData() = default;
        inline void SyncData(MeshRenderer* renderer, uint32_t index)
        {
            ASSERT(meshRendererList.size() > index);
            meshRendererList[index] = renderer;
            isDataValidList[index] = true;
            needUpdateList[index] = true;
            if (renderer && renderer->gameObject)
            {
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
            }
            auto* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
            if(meshFilter != nullptr)
            {
                vaoIDList[index] = meshFilter->mMeshHandle.Get()->GetInstanceID();
            }
            else
            {
                vaoIDList[index] = UINT32_MAX;
            }
        }

        inline void DeleteData(uint32_t index)
        {
            isDataValidList[index] = false;
        }
    };

    class Scene : Object
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
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
        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRendererSync();

        int AddNewRenderNodeToCurrentScene(MeshRenderer* renderer);
        void DeleteRenderNodeFromCurrentScene(uint32_t index);
        
    public:
        std::queue<uint32_t> m_FreeSceneNode;
        uint32_t m_CurrentSceneRenderNodeIndex = 0;
        uint32_t m_CurrentSceneMaxRenderNode = 0;
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;
        RenderSceneData renderSceneData;
    private:
    };    
} // namespace EngineCore
