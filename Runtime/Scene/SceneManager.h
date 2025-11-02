#pragma once
#include <map>
#include "Managers/Manager.h"
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "GameObject/Transform.h"
#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
#include "Resources/MetaFile.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceHandle.h"


namespace EngineCore
{
    class Scene;
    class SceneManager : public Manager<SceneManager>
    {
        // 允许Manager类访问SceneManager私有函数。
        friend class Manager<SceneManager>;
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void Update();
        static void Create();

    public:
        SceneManager();
        ~SceneManager();

        // todo： 这部分数据也要找地方存， maybe一个Global的渲染处
        ResourceHandle<Material> blitMaterial;
        ResourceHandle<ModelData> quadMesh;
        ResourceHandle<Shader> blitShader;
        //
        ResourceHandle<Texture> testTexture;

        inline Scene* GetCurrentScene() 
        { 
            return mCurrentScene; 
        };
        inline void SetCurrentScene(Scene* scene) 
        { 
            mCurrentScene = scene; 
        };
        Scene* AddNewScene(const std::string& name);
        void SwitchSceneTo(const std::string& name);
    private:
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
    };

}