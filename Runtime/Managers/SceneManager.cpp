#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "Graphics/Texture.h"
#include "Resources/ResourceManager.h"

namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;
    GameObject* SceneManager::mTestGameObject = nullptr;
    
    SceneManager::SceneManager()
    {
        //std::cout << "Init Scene Manager!!" << std::endl;
        mTestGameObject = new GameObject();
        mTestGameObject->AddComponent<MeshFilterComponent>();
        
        
        testMesh = ResourceManager::GetInstance()->LoadAsset<ModelData>("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
        
        testTexture = ResourceManager::GetInstance()->LoadAsset<Texture>("D:/GitHubST/TinyEngine/Assets/Textures/material.png");

        testMat = ResourceManager::GetInstance()->LoadAsset<Material>("D:/GitHubST/TinyEngine/Assets/Material/testMat.mat");

        testShader = ResourceManager::GetInstance()->LoadAsset<Shader>("D:/GitHubST/TinyEngine/Assets/Shader/SimpleTestShader.hlsl");
        
        //testMat->mShader = testShader;
        //testMat->renderState.shaderInstanceID = testShader->GetInstanceID();
        //testMat->SetTexture("DiffuseTexture", testTexture.Get());

        mCamera = new Camera();

        // quad Mesh也应该通过它生成.
        //ResourceManager::GetInstance()->CreateResource<ModelData>(Primitive::Quad);
        quadMesh = ResourceManager::GetInstance()->CreateResource<ModelData>(Primitive::Quad);
        blitShader = ResourceManager::GetInstance()->LoadAsset<Shader>("D:/GitHubST/TinyEngine/Assets/Shader/BlitShader.hlsl");
        
        blitMaterial = ResourceManager::GetInstance()->CreateResource<Material>(blitShader);
        //blitMaterial->renderState.shaderInstanceID = blitShader->GetInstanceID();
        //blitMaterial->SetUpGPUResources();
        blitMaterial->SetFloat("_FlipY", 1.0f);
        //blitMaterial->SetTexture("SrcTexture", Texture("CameraColorAttachment"));


    }

    SceneManager::~SceneManager()
    {
        delete mTestGameObject;
        mTestGameObject = nullptr;
    };

    void SceneManager::Update()
    {
        auto meshFilterComponent = mTestGameObject->GetComponent<MeshFilterComponent>();
    }

    void SceneManager::Create()
    {
        SceneManager::s_Instance = std::make_unique<SceneManager>();
    }
}