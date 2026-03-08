#pragma once
#include <array>
// SPSC means:: single producer single consumer
#include <condition_variable>
#include "RenderCommand.h"


namespace EngineCore
{
    template <typename T, size_t CapacityPow2>
    class alignas(64) SPSCRingBuffer
    {
    public:

        SPSCRingBuffer() 
        {
            mask = CapacityPow2 - 1;
        }
        static_assert(((CapacityPow2 & (CapacityPow2 - 1)) == 0), "Must Be Power of two");
    public:

        bool TryPush(const T& v)
        {
            auto h = mHead.load(std::memory_order_relaxed);
            auto t = mTail.load(std::memory_order_acquire);
            if (h - t == CapacityPow2) return false;
            mBuffer[h & mask] = v;
            mHead.store(h + 1, std::memory_order_release);
            return true;
        };


        bool TryPop(T& out)
        {
            auto h = mHead.load(std::memory_order_relaxed);
            auto t = mTail.load(std::memory_order_acquire);
            if (h == t) return false;
            out = std::move(mBuffer[t & mask]);
            mTail.store(t + 1, std::memory_order_release);
            return true;
        }

    private:
        uint64_t mask = 1;
        std::array<T, CapacityPow2> mBuffer;
        std::atomic<size_t> mHead{0};
        std::atomic<size_t> mTail{0};

    };

};
