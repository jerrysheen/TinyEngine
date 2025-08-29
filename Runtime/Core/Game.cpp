#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"

namespace EngineCore
{
    void Game::Launch()
    {
        // InitManagers Here.
        SceneManager::GetInstance();
        std::cout << "Launch Game" << std::endl;
        // init Manager...
        while(true)
        {
            Update();

            Render();
        }
    }

    void Game::Update()
    {
        std::cout << "Update Game" << std::endl;
        SceneManager::GetInstance().Update();

    }

    void Game::Render()
    {
        std::cout << "Render Scene" << std::endl;
    }

}