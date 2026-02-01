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
                if(m_Stopped) return;
                m_Queue.push_back(value);
            }
            m_IsQueueEmpty.notify_one();
        }

        bool WaitAndPop(T& value)
        { 
            std::unique_lock<std::mutex> lock(m_mutex);
            // 如果不 bool m_Stopped = true，则我没办法唤醒，外部notifyall，这个线程
            // 判断m_Queue empty 还是会睡在这里？
            m_IsQueueEmpty.wait(lock, [&](){ return m_Stopped || !m_Queue.empty();});
            if(m_Queue.empty())
            {
                return false;
            }
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

        // notify all一下， 确保workthread执行完
        void Stop()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_Stopped = true;
            }
            m_IsQueueEmpty.notify_all();
        }
        
        inline int size()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_Queue.size();
        }
    private:
        std::deque<T> m_Queue;
        std::mutex m_mutex;
        std::condition_variable m_IsQueueEmpty;
        bool m_Stopped = false;
    };
};