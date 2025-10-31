#pragma once
#include "Managers/Manager.h"
#include "RenderContext.h"

namespace EngineCore
{
    // 平台无关类， 负责拉起循环而已，内部各种渲染类组成。
    // 首先明确下这些函数都在负责什么功能。
    // begin render: RenderAPI->BeginFrame() 等待上一帧结束，OnResize处理
    // render: 执行场景渲染，不同管线进行渲染等。
    // end render : RenderAPI->EndFrame() 调用Present，buckbufferIndex Update
    class RenderEngine
    {
    public:
        static RenderEngine& GetInstance(){return *s_Instance;};
        static bool IsInitialized(){return s_Instance != nullptr;};
        static void Update();
        static void Create();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void BeginRender();
        static void Render();
        static void EndRender();

        RenderEngine(){};
        ~RenderEngine(){};
    private:
        static std::unique_ptr<RenderEngine> s_Instance;
        static RenderContext renderContext;
    };
    
}