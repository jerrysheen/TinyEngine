#include "PreCompiledHeader.h"
#include "Game.h"
#include "EngineCore.h"
#include "Resources/ResourceManager.h"
#include "Concurrency/JobSystem.h"
#ifdef EDITOR
#include "EditorGUIManager.h"
#endif
#include "Settings/ProjectSettings.h"
#include "Resources/AssetRegistry.h"
namespace EngineCore
{
    void Game::Launch()
    {
        ProjectSettings::Initialize();
        // InitManagers Here.
        RenderEngine::Create();
        ResourceManager::Create();
        SceneManager::Create();
        JobSystem::Create();
        AssetRegistry::Create();
        ASSERT(!(RenderSettings::s_EnableVertexPulling == true && RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy));
        //std::cout << "Launch Game" << std::endl;
        // init Manager...
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::Create();
        #endif
        while(!WindowManager::GetInstance()->WindowShouldClose())
        {
            PROFILER_FRAME_MARK("TinyProfiler");
            TickFrame(mFrameIndex);

            mFrameIndex++;

        }

        // 明确的关闭流程（顺序很重要！）
        Shutdown();
    }

    void Game::TickFrame(uint32_t frameIndex)
    {
        PROFILER_ZONE("MainThread::GameUpdate");
        ResourceManager::GetInstance()->Update();

        PROFILER_EVENT_BEGIN("TickFrame::TickSimulation");
        SceneManager::GetInstance()->TickSimulation(frameIndex);
        PROFILER_EVENT_END("TickFrame::TickSimulation");

        SceneDelta sceneDelta = SceneManager::GetInstance()->FlushSceneDelta();

        PROFILER_EVENT_BEGIN("TickFrame::RenderEngineUpdate");
        RenderEngine::GetInstance()->PrepareFrame(frameIndex, sceneDelta);
        PROFILER_EVENT_END("TickFrame::RenderEngineUpdate");


        RenderEngine::GetInstance()->BuildFrame();


        PROFILER_EVENT_BEGIN("TickFrame::EndFrame");
        SceneManager::GetInstance()->EndFrame();
        RenderEngine::GetInstance()->EndFrame();
        PROFILER_EVENT_END("TickFrame::EndFrame");

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
        ResourceManager::GetInstance()->Destroy();
        std::cout << "ResourceManager destroyed." << std::endl;

        std::cout << "Game shutdown complete." << std::endl;
    }

}
