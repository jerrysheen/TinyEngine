#pragma once
#include "d3dUtil.h"


#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
namespace EngineCore
{

    struct TD3D12DrawRecord
    {
        // temp 方案：
        // 为了测试，先用直接塞数据的方式。
        Material* mat;
        ModelData* model;
        TD3D12DrawRecord(Material* mat, ModelData* data):mat(mat),model(data){};
    };

    struct TD3D12DescriptorHandle
    {
        // 在描述符堆里的索引
        uint32_t descriptorIdx = UINT32_MAX;
        // 在描述符堆数组里的索引
        uint32_t descriptorHeapIdx = UINT32_MAX;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    };

    struct TD3D12ConstantBuffer
    {
        ComPtr<ID3D12Resource> mBufferResource;
        int mSize;
        void* mCpuAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = 0;
        TD3D12DescriptorHandle handleCBV = {};
        int registerSlot;
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

    struct TD3D12VAO
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
	    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer = nullptr;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
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

    struct TD3D12FrameBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;  
    };
} // namespace EngineCore
