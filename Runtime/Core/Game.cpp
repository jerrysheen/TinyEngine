#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"

#ifdef EDITOR
#include "Windows/EditorGUIManager.h"
#endif

namespace EngineCore
{
    void Game::Launch()
    {
        // InitManagers Here.
        RenderEngine::Create();
        SceneManager::Create();
        //std::cout << "Launch Game" << std::endl;
        // init Manager...
        #ifdef EDITOR
        //EngineEditor::EditorGUIManager::Create();
        #endif
        while(!WindowManager::GetInstance().WindowShouldClose())
        {
            Update();

            Render();
        }
    }

    void Game::Update()
    {
        //std::cout << "Update Game" << std::endl;
        SceneManager::GetInstance().Update();
        RenderEngine::GetInstance().Update();
        #ifdef EDITOR
        //EngineEditor::EditorGUIManager::GetInstance().Update();
        #endif
    }

    void Game::Render()
    {
        //std::cout << "Render Scene" << std::endl;
        RenderEngine::GetInstance().BeginRender();
        #ifdef EDITOR
        //EngineEditor::EditorGUIManager::GetInstance().BeginFrame();
        #endif
        RenderEngine::GetInstance().Render();
        #ifdef EDITOR
        //EngineEditor::EditorGUIManager::GetInstance().Render();
        //EngineEditor::EditorGUIManager::GetInstance().EndFrame();
        #endif
        RenderEngine::GetInstance().EndRender();

    }

}