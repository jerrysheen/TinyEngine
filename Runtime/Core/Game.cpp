#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"

namespace EngineCore
{
    void Game::Launch()
    {
        // InitManagers Here.
        SceneManager::Create();
        WindowManager::Create();
        std::cout << "Launch Game" << std::endl;
        // init Manager...
        while(!WindowManager::GetInstance().WindowShouldClose())
        {
            Update();

            Render();
        }
    }

    void Game::Update()
    {
        std::cout << "Update Game" << std::endl;
        SceneManager::GetInstance().Update();
        WindowManager::GetInstance().Update();
    }

    void Game::Render()
    {
        std::cout << "Render Scene" << std::endl;
    }

}