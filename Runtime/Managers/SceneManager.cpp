#include "PreCompiledHeader.h"
#include "SceneManager.h"


namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;
    GameObject* SceneManager::mTestGameObject = nullptr;
    
    SceneManager::SceneManager()
    {
        //std::cout << "Init Scene Manager!!" << std::endl;
        mTestGameObject = new GameObject();
        mTestGameObject->AddComponent<MeshFilterComponent>();
        testMesh = ModelUtils::LoadMesh("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
        
        
        testMatStruct = Resources::LoadMaterial("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
        testMat = new Material(testMatStruct);
        testShader = Shader::Compile("D:/GitHubST/TinyEngine/Assets/Shader/SimpleTestShader.hlsl");
        testMat->shader = testShader;
        testMat->SetUpGPUResources();
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