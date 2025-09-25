#pragma once
#include "d3dUtil.h"


#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
namespace EngineCore
{


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

    // struct TD3D12TextureBuffer
    // {
    //     ComPtr<ID3D12Resource> texture = nullptr;
    //     TD3D12DescriptorHandle handleSRV = {};
    //     TD3D12DescriptorHandle handleRTV = {};
    //     TD3D12DescriptorHandle handleDSV = {};
    // };
    struct TD3D12TextureHander 
    {
        string textureID;
    };

    struct TD3D12TextureBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;  
        D3D12_RESOURCE_STATES state;
    };

    using TD3D12FrameBuffer = TD3D12TextureBuffer;

    struct TD3D12MaterialData
    {
        /* data */
        vector<TD3D12ConstantBuffer> mConstantBufferArray;
        vector<TD3D12TextureHander> mTextureBufferArray;
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

    struct TD3D12ShaderPSO
    {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        
        // 添加默认构造函数
        TD3D12ShaderPSO() = default;

        TD3D12ShaderPSO(Microsoft::WRL::ComPtr<ID3D12PipelineState> pso,
            Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
            std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout)
            : pso(pso), rootSignature(rootSignature), inputLayout(inputLayout)
        {

        };
    };
        


} // namespace EngineCore
