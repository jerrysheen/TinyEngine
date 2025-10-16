#pragma once
#include "PreCompiledHeader.h"

namespace EngineCore
{
    inline void HashCombine(uint32_t& seed, uint32_t hash)
    {
        // Magic number from Boost
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<typename T>
    inline void HashCombine(uint32_t& seed, const T& value)
    {
        HashCombine(seed, std::hash<T>{}(value));
    }

}