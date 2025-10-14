#pragma once
#include <array>
// SPSC means:: single producer single consumer
#include <condition_variable>
#include "RenderCommand.h"


namespace EngineCore
{
    template <size_t CapacityPow2>
    class alignas(64) SPSCRingBuffer
    {
        static_assert(((CapacityPow2 & (CapacityPow2 - 1)) == 0), "Must Be Power of two");
    public:
        bool TryPush(const DrawCommand& cmd)
        {
            // get head
            size_t head = mHead.load(std::memory_order_relaxed);
            size_t next = (head + 1) & (CapacityPow2 - 1);
            if(next == mTail.load(std::memory_order_acquire))
            {
                cout << "Render Queue is Full" << endl;
                return false;
            }
            mCmdQueue[head] = std::move(cmd);
            mHead.store(next, std::memory_order_release);
            return true;
        }
        // 阻塞直到push成功
        void PushBlocking(const DrawCommand& cmd)
        {
            while(!TryPush(cmd))
            {
                std::unique_lock<std::mutex> lk(m_mu);
                mCVIsNotFull.wait(lk, [&](){return !IsFull();});
            }

            mCVIsNotEmpty.notify_all();
        };

        // 消费者消费：
        bool TryPop(DrawCommand& cmd)
        {
            size_t tail = mTail.load(std::memory_order_relaxed);
            if( tail == mHead.load(std::memory_order_acquire))
            {
                cout << "Render Queue is Empty" << endl;
                return false;
            }
            cmd = mCmdQueue[tail];
            size_t next = (tail + 1) & (CapacityPow2 - 1);
            mTail.store(next, std::memory_order_release);
            mCVIsNotFull.notify_all();
            return true;
        }

        // 阻塞直到pop成功
        void PopBlocking(DrawCommand& cmd)
        {
            while(!TryPop(cmd))
            {
                std::unique_lock<std::mutex> lk(m_mu);
                mCVIsNotEmpty.wait(lk, [&]{return !IsEmpty();});
            }
            mCVIsNotFull.notify_all();
        }

        bool IsEmpty() const 
        {
            return mTail.load(std::memory_order_relaxed) == mHead.load(std::memory_order_acquire);
        }

        bool IsFull() const
        {
            size_t head = mHead.load(std::memory_order_acquire);
            size_t next = (head + 1) & (CapacityPow2 - 1);
            return (next == mTail.load(std::memory_order_relaxed)); 
        }

    private:
        std::array<DrawCommand, CapacityPow2> mCmdQueue;
        std::atomic<size_t> mHead{0};
        std::atomic<size_t> mTail{0};

        mutable std::mutex m_mu;
        std::condition_variable mCVIsNotFull;
        std::condition_variable mCVIsNotEmpty;
    };

};