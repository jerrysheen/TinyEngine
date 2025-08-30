#include "PreCompiledHeader.h"
#include "SceneManager.h"

namespace EngineCore
{
    std::unique_ptr<SceneManager> SceneManager::s_Instance = nullptr;

    SceneManager::SceneManager()
    {
        std::cout << "Init Scene Manager!!" << std::endl;
    }

    SceneManager::~SceneManager(){};

    void SceneManager::Update()
    {
        std::cout << "SceManager Update" << std:: endl;
    }

    void SceneManager:: Create()
    {
        SceneManager::s_Instance = std::make_unique<SceneManager>();
    }
}