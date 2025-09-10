#include "PreCompiledHeader.h"
#include "SceneManager.h"
#include "Graphics/ModelUtils.h"

namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;
    GameObject* SceneManager::mTestGameObject = nullptr;
    
    SceneManager::SceneManager()
    {
        //std::cout << "Init Scene Manager!!" << std::endl;
        mTestGameObject = new GameObject();
        mTestGameObject->AddComponent<MeshFilterComponent>();
        ModelData* testMesh = ModelUtils::LoadMesh("D:/GitHubST/TinyEngine/Assets/Model/cube.obj");
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

    void SceneManager:: Create()
    {
        SceneManager::s_Instance = std::make_unique<SceneManager>();
    }
}