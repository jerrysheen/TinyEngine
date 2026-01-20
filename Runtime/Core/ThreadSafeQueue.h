#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>

namespace EngineCore
{
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        void TryPush(T value)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_Queue.push_back(value);
            }
            m_IsQueueEmpty.notify_one();
        }

        bool WaitAndPop(T& value)
        { 
            std::unique_lock<std::mutex> lock(m_mutex);
            m_IsQueueEmpty.wait(lock, [&](){ return !m_Queue.empty();});
            value = m_Queue.front();
            m_Queue.pop_front();
            return true;
        }

        bool TryPop(T& value)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(m_Queue.empty()) return false;
            value = m_Queue.front();
            m_Queue.pop_front();
            return true;
        }
        
        inline int size(){return m_Queue.size();}
    private:
        std::deque<T> m_Queue;
        std::mutex m_mutex;
        std::condition_variable m_IsQueueEmpty;
    };
};