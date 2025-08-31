#pragma once
#include "PreCompiledHeader.h"

namespace  EngineCore
{
    class RenderAPI
    {
    public:
        static RenderAPI& GetInstance(){ return *s_Instance.get();}
        virtual void BeginFrame() = 0;
        virtual void Render() = 0;
        virtual void EndFrame() = 0;

        static void Create();
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    };
    
} // namespace  EngineCore

