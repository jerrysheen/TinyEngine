#pragma once
#include <iostream>

namespace EngineCore
{
    class IGPUBuffer;
    struct BufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        uint64_t gpuAddress = 0;
        uint8_t* cpuAddress = nullptr;
        uint64_t offset =0;
        uint64_t size = 0;
        bool isValid = false;
    };
}