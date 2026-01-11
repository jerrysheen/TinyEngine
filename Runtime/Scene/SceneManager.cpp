#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "BistroSceneLoader.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
#include "Scripts/CameraController.h"
#include "Scene.h"
#include "Serialization/MetaFactory.h"
#include "Serialization/JsonSerializer.h"

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

    // scene的创建， 不应该这么玩， 应该是空场景， 或者是根据MetaData
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

    void SceneManager::Update()
    {
        if (s_Instance->mCurrentScene != nullptr) 
        {
            s_Instance->mCurrentScene->Update();
        }
    }

    void SceneManager::Create()
    {
        if (s_Instance) return;
        s_Instance = new SceneManager();
        s_Instance->Init();
    }

    void SceneManager::Init() 
    {
        //json j = EngineCore::JsonSerializer::ReadFromJson("/Scenes/PerformanceTestScene.meta");
        //json j = EngineCore::JsonSerializer::ReadFromJson("/Scenes/SampleScene.meta");
        //Scene* scene = MetaFactory::CreateSceneFromMeta(j);

        // Load Bistro Scene
        string bistroPath = PathSettings::ResolveAssetPath("/Scenes/niagara_bistro/bistro.gltf"); 
        string fullPath = PathSettings::ResolveAssetPath(bistroPath);
        Scene* scene = BistroSceneLoader::Load(fullPath);

        auto* gameObject = scene->CreateGameObject("Camera");
        auto* cam = gameObject->AddComponent<Camera>();
        scene->mainCamera = cam;

        ASSERT(scene);
        if (scene) {
             mSceneMap[scene->name] = scene;
             mCurrentScene = scene;
             scene->Open();
        }

        // todo: 应该能保存序列化 更多的meta， 包括场景中代码创建的， 只要是shader和texture是代码创建的， 应该就可以，而不是像现在这样。
        quadMesh = new ModelData(Primitive::Quad);
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