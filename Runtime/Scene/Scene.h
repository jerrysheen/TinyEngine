#pragma once
#include "Core/Object.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
namespace EngineCore
{
    class Scene : Object
    {
    public:
        Scene() = default;
        Scene(const std::string& name):name(name){};
        void Open();
        void Close(){};
        void Update();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
        inline Camera* GetMainCamera() { return mainCamera; }

        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;
    public:
        std::string name;
        std::vector<GameObject*> objLists;
        bool enabled = true;
        Camera* mainCamera;
    private:
        void UpdateAllTransforms();
    };    
} // namespace EngineCore
