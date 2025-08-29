#pragma once


#define WINDOWS_EDITOR
namespace EngineCore
{
    class Game
    {
    public:
        static std::unique_ptr<Game> m_Instance;
        // 回传的是一个对象的引用，所以返回*ptr
        static Game& GetInstance()
        {
            if(m_Instance == nullptr)
            {
                m_Instance = std::make_unique<Game>();
            }
            return *m_Instance;
        };
        Game(){};
        ~Game(){};

        void Launch();
    private:
        void Update();
        void Render();
    };

}