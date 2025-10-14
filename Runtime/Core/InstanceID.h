#pragma once
#include "PreCompiledHeader.h"

namespace EngineCore
{
    struct InstanceID
    {
        uint64_t v{0};
        explicit operator bool() const { return v != 0; };
    public:
    };

    class InstanceIDGenerator
    {
    public:
        static InstanceID New()
        {
            // 从 1 开始，保留 0 代表“无效”
            const auto next = sCount.fetch_add(1, std::memory_order_relaxed) + 1;
            return InstanceID{ next };
        };

    private:
        static inline std::atomic<uint64_t> sCount{0};

    };
} // namespace EngineCore