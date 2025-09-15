#pragma once
#include "d3dUtil.h"

namespace EngineCore
{



    struct TD3D12DescriptorHandle
    {
        // 在描述符堆里的索引
        uint32_t descriptorIdx = UINT32_MAX;
        // 在描述符堆数组里的索引
        uint32_t descriptorHeapIdx = UINT32_MAX;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    };

    struct TD3D12ConstantBuffer
    {
        ComPtr<ID3D12Resource> mBufferResource;
        int mSize;
        void* mCpuAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = 0;
        TD3D12DescriptorHandle handleCBV = {};

        TD3D12ConstantBuffer(){};
    };

    struct TD3D12TextureBuffer
    {
        ComPtr<ID3D12Resource> texture = nullptr;
        TD3D12DescriptorHandle handleSRV = {};
        TD3D12DescriptorHandle handleRTV = {};
        TD3D12DescriptorHandle handleDSV = {};
    };

    struct TD3D12MaterialData
    {
        /* data */
        vector<TD3D12ConstantBuffer> mConstantBufferArray;
        vector<TD3D12TextureBuffer> mTextureBufferArray;
        struct TD3D12MaterialData(){};
    };


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
