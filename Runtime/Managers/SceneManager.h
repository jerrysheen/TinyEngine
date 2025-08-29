#pragma once
#include "PreCompiledHeader.h"
#include "Manager.h"

namespace EngineCore
{
    class SceneManager : public Manager<SceneManager>
    {
        // 允许Manager类访问SceneManager私有函数。
        friend class Manager<SceneManager>;
    public:
        void LoadScene();
        void UnloadScene();
        virtual void Update() override;
        virtual void Create() override;
    public:
        SceneManager();
        ~SceneManager();
    };

}