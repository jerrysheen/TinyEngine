#pragma once
#include <map>
#include "GameObject/GameObject.h"
#include "GameObject/Camera.h"
#include "GameObject/Transform.h"
#include "Graphics/MeshUtils.h"
#include "GameObject/MeshRenderer.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceHandle.h"


namespace EngineCore
{
    class Scene;
    class SceneManager
    {
        // 允许Manager类访问SceneManager私有函数。
    public:
        void LoadScene();
        void UnloadScene();
        GameObject* CreateGameObject(const std::string& name);
        GameObject* FindGameObject(const std::string& name);

        void RemoveScene(const std::string& name);
        static void Update(uint32_t frameIndex);
        static void Create();
        static void Destroy();
        static void EndFrame();
        void Init();
    public:
        inline static SceneManager* GetInstance() 
        {
            if (!s_Instance) 
            {
                s_Instance = new SceneManager();
            }
            return s_Instance;
        }

        SceneManager();
        ~SceneManager();

        // todo： 这部分数据也要找地方存， maybe一个Global的渲染处

        Material* blitMaterial;
        Mesh* quadMesh;
        ResourceHandle<Shader> blitShader;
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
        static SceneManager* s_Instance;
        Scene* mCurrentScene = nullptr;
        unordered_map<std::string, Scene*> mSceneMap;
        vector<ResourceHandle<Texture>> texHandler;
    };

}