#include "PreCompiledHeader.h"
#include "Game.h"

namespace EngineCore
{
    void Game::Launch()
    {
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

    }

    void Game::Render()
    {
        std::cout << "Render Scene" << std::endl;
    }

}