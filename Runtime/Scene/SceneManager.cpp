﻿#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceManager.h"
#include "GameObject/MeshFilter.h"
#include "GameObject/MeshRenderer.h"
namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;
    
    SceneManager::SceneManager()
    {
        testTexture = ResourceManager::GetInstance()->LoadAsset<Texture>("Textures/viking_room.png");

        // quad Mesh也应该通过它生成.
        //ResourceManager::GetInstance()->CreateResource<ModelData>(Primitive::Quad);
        quadMesh = ResourceManager::GetInstance()->CreateResource<ModelData>(Primitive::Quad);
        blitShader = ResourceManager::GetInstance()->LoadAsset<Shader>("Shader/BlitShader.hlsl");
        
        blitMaterial = ResourceManager::GetInstance()->CreateResource<Material>(blitShader);
        //blitMaterial->renderState.shaderInstanceID = blitShader->GetInstanceID();
        //blitMaterial->SetUpGPUResources();
        blitMaterial->SetFloat("_FlipY", 1.0f);
        //blitMaterial->SetTexture("SrcTexture", Texture("CameraColorAttachment"));



        auto* scene = AddNewScene("SampleScene");

        auto* cameraGO = scene->CreateGameObject("Camera");
        auto* cameraComponent = cameraGO->AddComponent<Camera>();
        scene->AddCamToStack(cameraComponent);
        Transform* transform = cameraGO->GetComponent<Transform>();
        transform->SetLocalPosition(Vector3{ 0.0f, 0.0f, -5.0f });
        transform->UpdateNow();
        cameraComponent->UpdateCameraMatrix();

        auto* testObject = scene->CreateGameObject("house");
        auto meshfilter = testObject->AddComponent<MeshFilter>();
        meshfilter->mMeshHandle = ResourceManager::GetInstance()->LoadAsset<ModelData>("Model/viking_room.obj");
        auto meshRender = testObject->AddComponent<MeshRenderer>();
        meshRender->mMatHandle = ResourceManager::GetInstance()->LoadAsset<Material>("Material/testMat.mat");
        transform = testObject->GetComponent<Transform>();
        transform->RotateX(90.0f);
        transform->RotateY(135.0f);
        transform->UpdateNow();
        // todo ： 矩阵数据上传应该在哪里？
        // 应该在permatdata更新的地方
        meshRender->mMatHandle.Get()->SetMatrix4x4("WorldMatrix", transform->GetWorldMatrix());
    }

    SceneManager::~SceneManager()
    {
        for (auto& [key, value] : mSceneMap) 
        {
            delete value;
        }
        mSceneMap.clear();

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
    inline void SceneManager::RemoveScene(const std::string& name)
    {
        ASSERT_MSG(mSceneMap.count(name) > 0, "Can't find this scene");
        // auto* 和 auto 是一样的道理， *在这个地方其实是一种语义，表示指针
        auto* scene = mSceneMap[name];
        if(mCurrentScene == scene)
        {
            scene->Close();
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
        SceneManager::s_Instance = std::make_unique<SceneManager>();
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