#pragma once
#include <condition_variable>

namespace EngineCore
{
    class CpuEvent
    {
    public:
        static CpuEvent& RenderThreadSubmited();
        static CpuEvent& MainThreadSubmited();

        CpuEvent(bool startCondition = false)
            : m_signaled(startCondition){}

        void Signal()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_signaled = true;
            }
            m_cv.notify_all();
        }

        void Wait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]{ return m_signaled;});
            m_signaled = false;
        }

    private:
        std::mutex  m_mutex;
        std::condition_variable m_cv;
        bool    m_signaled = false;
    };
}