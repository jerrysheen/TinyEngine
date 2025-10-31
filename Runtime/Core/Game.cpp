#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"
#include "Resources/ResourceManager.h"
#ifdef EDITOR
#include "Windows/EditorGUIManager.h"
#endif
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    void Game::Launch()
    {
        ProjectSettings::Initialize();
        // InitManagers Here.
        ResourceManager::Create();
        RenderEngine::Create();
        SceneManager::Create();

        //std::cout << "Launch Game" << std::endl;
        // init Manager...
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::Create();
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
        #endif
    }

    void Game::Render()
    {
        RenderEngine::GetInstance().BeginRender();

        #ifdef EDITOR
        // 需要在RenderEngineRender之前，提前标记这一帧需要IMGUI绘制。
        EngineEditor::EditorGUIManager::GetInstance().Update();
        #endif


        RenderEngine::GetInstance().Render();



        RenderEngine::GetInstance().EndRender();

    }

}