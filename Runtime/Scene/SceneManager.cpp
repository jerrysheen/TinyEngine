#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "BistroSceneLoader.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Scripts/CameraController.h"
#include "Scene.h"

namespace EngineCore
{
    SceneManager* SceneManager::s_Instance = nullptr;
    
    SceneManager::SceneManager()
    {
    }

    SceneManager::~SceneManager()
    {
        mCurrentScene = nullptr;
        for (auto& [key, value] : mSceneMap) 
        {
            delete value;
        }
        mSceneMap.clear();
    }

    void SceneManager::Destroy()
    {
        for (auto& [key, scene] : s_Instance->mSceneMap)
        {
            delete scene;
        }
    }

    void SceneManager::EndFrame()
    {
        if (s_Instance->mCurrentScene != nullptr)
        {
            s_Instance->mCurrentScene->EndFrame();
        }
    }

    inline Scene* SceneManager::AddNewScene(const std::string& name)
    {
        ASSERT_MSG(mSceneMap.count(name) == 0, "The Same Name Scene Has Been Created");
        Scene* scene = new Scene(name);
        mSceneMap.try_emplace(name, scene);
        if (mCurrentScene == nullptr) 
        {
            mCurrentScene = scene;
        }
        return scene;
    }

    // todo : 谁去持有这个Scene， 如果是这里， 那这里要负责管理
    void SceneManager::RemoveScene(const std::string& name)
    {
        ASSERT_MSG(mSceneMap.count(name) > 0, "Can't find this scene");
        // auto* 和 auto 是一样的道理， *在这个地方其实是一种语义，表示指针
        auto* scene = mSceneMap[name];
        if(mCurrentScene == scene)
        {
            scene->Close();
        }
        if(mCurrentScene == scene)
        {
            mCurrentScene = nullptr;
        }
        delete scene;
        mSceneMap.erase(name);
    }

    void SceneManager::SwitchSceneTo(const std::string& name)
    {
        ASSERT(mSceneMap.count(name) > 0);
        auto* scene = mSceneMap[name];
        if(scene == mCurrentScene) return;
        mCurrentScene->Close();
        scene->Open();
        mCurrentScene  = scene;
    };

    void SceneManager::Update(uint32_t frameIndex)
    {
        if (s_Instance->mCurrentScene != nullptr) 
        {
            s_Instance->mCurrentScene->Update(frameIndex);
        }
    }

    void SceneManager::SetCurrentFrame(uint32_t currentFrameIndex)
    {
        if (mCurrentScene) mCurrentScene->SetCurrentFrame(currentFrameIndex);
    }

    void SceneManager::Create()
    {
        if (s_Instance) return;
        s_Instance = new SceneManager();
        s_Instance->Init();
    }

    void SceneManager::Init() 
    {

        // Load Bistro Scene
        string bistroPath = PathSettings::ResolveAssetPath("/Scenes/niagara_bistro/bistro.gltf"); 
        string fullPath = PathSettings::ResolveAssetPath(bistroPath);
        Scene* scene = BistroSceneLoader::Load(fullPath);

        testTexture = ResourceManager::GetInstance()->LoadAssetAsync<Texture>((uint64_t)17377329839595119579, nullptr, nullptr);
        auto* gameObject = scene->CreateGameObject("Camera");
        auto* cam = gameObject->AddComponent<Camera>();
        gameObject->transform->SetLocalPosition(Vector3(-15, 2.5, -3));
        //gameObject->transform->SetLocalPosition(Vector3(-35, 57.85, -103.45));
        //gameObject->transform->SetLocalEulerAngles(Vector3(21.46, 11.97, 5.781));
        scene->mainCamera = cam;


        ASSERT(scene);
        if (scene) {
             mSceneMap[scene->name] = scene;
             mCurrentScene = scene;
             scene->Open();
        }

        quadMesh = new Mesh(Primitive::Quad);
        blitShader = ResourceManager::GetInstance()->LoadAsset<Shader>("Shader/BlitShader.hlsl");
        blitMaterial = new Material(blitShader);


    }

    GameObject *SceneManager::CreateGameObject(const std::string& name)
    {
        ASSERT(mCurrentScene != nullptr);
        return mCurrentScene->CreateGameObject(name);
    }

    GameObject *SceneManager::FindGameObject(const std::string& name)
    {
        ASSERT(mCurrentScene != nullptr);
        return mCurrentScene->FindGameObject(name);
    }
}