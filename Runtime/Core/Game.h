#pragma once


namespace EngineCore
{

    class Game
    {
    public:
        static std::unique_ptr<Game> m_Instance;
        // 回传的是一个对象的引用，所以返回*ptr
        static Game* GetInstance()
        {
            if(m_Instance == nullptr)
            {
                m_Instance = std::make_unique<Game>();
            }
            return m_Instance.get();
        };
        Game(){};
        ~Game(){};

        void Launch();
    private:
        void Update(uint32_t frameIndex);
        void Render();
        void EndFrame();
        void Shutdown();

        uint32_t mFrameIndex = 0;
    };

}