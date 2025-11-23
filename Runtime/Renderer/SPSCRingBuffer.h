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
        // 阻塞直到push成功
        bool PushBlocking(const DrawCommand& cmd)
        {
            std::unique_lock<std::mutex> lk(m_mu);
            mCVIsNotFull.wait(lk, [&]() {return !IsFull(); });

            if (IsFull()) 
            {
                return false;
            }
            size_t head = mHead.load(std::memory_order_relaxed);
            size_t next = (head + 1) & (CapacityPow2 - 1);
            mCmdQueue[head] = std::move(cmd);
            mHead.store(next, std::memory_order_release);
            mCVIsNotEmpty.notify_all();
            return true;
        };


        // 阻塞直到pop成功
        bool PopBlocking(DrawCommand& cmd)
        {
            std::unique_lock<std::mutex> lk(m_mu);
            mCVIsNotEmpty.wait(lk, [&] {return !IsEmpty(); });

            if (IsEmpty()) 
            {
                return false;
            }

            size_t tail = mTail.load(std::memory_order_relaxed);
            cmd = mCmdQueue[tail];
            size_t next = (tail + 1) & (CapacityPow2 - 1);
            mTail.store(next, std::memory_order_release);
            mCVIsNotFull.notify_all();
            return true;
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