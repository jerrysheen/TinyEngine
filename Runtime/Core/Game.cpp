#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"
#include "Resources/ResourceManager.h"
#ifdef EDITOR
#include "EditorGUIManager.h"
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
        while(!WindowManager::GetInstance()->WindowShouldClose())
        {
            Update();

            Render();
        }

        // 明确的关闭流程（顺序很重要！）
        Shutdown();
    }

    void Game::Shutdown()
    {
        std::cout << "Game shutting down..." << std::endl;

        // 1. 先停止渲染线程（最重要！）
        //    必须在销毁任何渲染资源之前停止
        RenderEngine::Destory();
        std::cout << "RenderEngine destroyed." << std::endl;

        // 2. 销毁编辑器UI
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::OnDestory();
        std::cout << "EditorGUIManager destroyed." << std::endl;
        #endif

        // 3. 销毁场景（包含所有GameObject）
        SceneManager::Destroy();
        std::cout << "SceneManager destroyed." << std::endl;

        // 4. 最后销毁资源管理器
        ResourceManager::Destroy();
        std::cout << "ResourceManager destroyed." << std::endl;

        std::cout << "Game shutdown complete." << std::endl;
    }

    void Game::Update()
    {
        //std::cout << "Update Game" << std::endl;
        SceneManager::GetInstance()->Update();
        RenderEngine::GetInstance()->Update();
    
        #ifdef EDITOR
        #endif
    }

    void Game::Render()
    {
        RenderEngine::GetInstance()->BeginRender();

        #ifdef EDITOR
        // 需要在RenderEngineRender之前，提前标记这一帧需要IMGUI绘制。
        EngineEditor::EditorGUIManager::GetInstance()->Update();
        #endif


        RenderEngine::GetInstance()->Render();



        RenderEngine::GetInstance()->EndRender();

    }

}