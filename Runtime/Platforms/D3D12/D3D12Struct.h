#pragma once
#include "d3dUtil.h"

namespace EngineCore
{
    struct TD3D12Fence
    {
    public:
        int mCurrentFence;
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    };

    struct TD3D12DrawCommand
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocators;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandLists;
    };
} // namespace EngineCore
