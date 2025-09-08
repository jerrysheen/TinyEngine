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
    }

    SceneManager::~SceneManager()
    {
        delete mTestGameObject;
        mTestGameObject = nullptr;
    };

    void SceneManager::Update()
    {
        auto meshFilterComponent = mTestGameObject->GetComponent<MeshFilterComponent>();
        std::cout << static_cast<int>(meshFilterComponent->GetType()) << std::endl;
    }

    void SceneManager:: Create()
    {
        SceneManager::s_Instance = std::make_unique<SceneManager>();
    }
}