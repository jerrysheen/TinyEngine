#pragma once
#include <queue>
#include <vector>
#include "Core/Object.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "Renderer/RenderStruct.h"

namespace EngineCore
{
    struct RenderSceneData
    {
        vector<MeshRenderer*> meshRendererList;
        vector<MeshFilter*> meshFilterList;
        vector<AABB> aabbList;
        vector<Matrix4x4> objectToWorldMatrixList;
        vector<uint32_t> layerList;

        // 每帧重置
        vector<uint32_t> transformDirtyList;
        vector<uint32_t> materialDirtyList;
        struct RenderSceneData() = default;
        inline void SyncData(MeshRenderer* renderer, uint32_t index)
        {
            meshRendererList[index] = renderer;
            if (renderer && renderer->gameObject)
            {
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
                auto* meshFilter = renderer->gameObject->GetComponent<MeshFilter>();
                if(meshFilter != nullptr)
                {
                    meshFilterList[index] = meshFilter;
                }
                else
                {
                    meshFilterList[index] = nullptr;
                }
            }
        }

        inline void PushNewData() 
        {
            meshRendererList.emplace_back();
            meshFilterList.emplace_back();
            aabbList.emplace_back();
            objectToWorldMatrixList.emplace_back();
            layerList.emplace_back();
        }


        inline void DeleteData(uint32_t index)
        {
            meshRendererList[index] = nullptr;
            meshFilterList[index] = nullptr;

            // 后续删除RenderProxy
        }

        inline void ClearDirtyList()
        {
            materialDirtyList.clear();
            transformDirtyList.clear();
        }

        inline void UpdateDirtyRenderNode()
        {
            for(uint32_t index : transformDirtyList)
            {
                // 直接访问安全，因为meshrenderer为空的不会加到这里。
                auto* renderer = meshRendererList[index];
                aabbList[index] = renderer->worldBounds;
                objectToWorldMatrixList[index] = renderer->gameObject->transform->GetWorldMatrix();
            }
            // todo: 暂时没有这个逻辑， 比如material切换pso，切换vao这种
            //materialDirtyList...
        }
    };

    class Scene : public Resource
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
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
        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRecordDirtyRenderNode();


        // todo:
        // 材质更新的时候， 也需要去调用这个逻辑，比如changeRenderNodeInfo之类的，
        int AddNewRenderNodeToCurrentScene(MeshRenderer* renderer);
        void DeleteRenderNodeFromCurrentScene(uint32_t index);
        
    public:
        int m_CurrentSceneRenderNodeIndex = 0;
        std::queue<uint32_t> m_FreeSceneNode;
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;
        RenderSceneData renderSceneData;
    private:
    };    
} // namespace EngineCore
