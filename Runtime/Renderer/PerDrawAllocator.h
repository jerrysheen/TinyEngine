#pragma once
#include "iostream"
#include "Core/PublicStruct.h"

namespace EngineCore
{

    class PerDrawAllocator
    {
    public:
        virtual ~PerDrawAllocator() = default;

        virtual PerDrawHandle Allocate(uint32_t size) = 0;
        virtual uint8_t*    GetCPUAddress(PerDrawHandle h) = 0; // 用来 memcpy
        virtual uint64_t    GetGPUBaseAddress() const = 0;  // 大 buffer 的 base GPU VA
        virtual void        ResetFrame() = 0;
    };
}