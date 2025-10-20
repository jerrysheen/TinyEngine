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

    // 用于AssetID等需要持久化的ID
    inline uint64_t GetStringHash(const string& path)
    {
        if (path.empty())
            return 0;
        
        // FNV-1a 参数
        constexpr uint64_t FNV_offset_basis = 14695981039346656037ULL;
        constexpr uint64_t FNV_prime = 1099511628211ULL;
        
        uint64_t hash = FNV_offset_basis;
        for (char c : path)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_prime;
        }
        return hash;

    }

}