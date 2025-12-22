#pragma once
#include <iostream>
#include <vector>

namespace EngineCore
{
    class LinearAllocator
    {
    public:
        LinearAllocator(uint32_t size)
        {
            buffer.resize(size);
            currentOffset = 0;
        }

        void Reset()
        {
            currentOffset = 0;
        }

        // align of 得到当前class/struct的对齐方式
        void* allocate(uint32_t size, size_t align = alignof(std::max_align_t))
        {
            // 补齐空位， 下一个位置从当前struct align倍数开始，避免出错
            size_t alignedOffset = (currentOffset + align - 1) & ~(align - 1);
            if(alignedOffset + size > buffer.size())
            {
                size_t newSize = buffer.size() * 2;
                while (alignedOffset + size > newSize) newSize *= 2;
                buffer.resize(newSize);
            }

            void* ptr = buffer.data() + alignedOffset;
            currentOffset = alignedOffset + size;
            return ptr;
        }

        template<typename T>
        T* allocArray(int size)
        {
            return (T*)allocate(size * sizeof(T), alignof(T));
        }
    private:
        std::vector<uint8_t> buffer;
        size_t currentOffset = 0;
    };

}