#pragma once
#include "d3dUtil.h"


#include "Graphics/ModelUtils.h"
#include "Graphics/Material.h"
#include "Core/InstanceID.h"
#include "Renderer/RenderCommand.h"

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
        int mSize = 0;
        void* mCpuAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = 0;
        TD3D12DescriptorHandle handleCBV = {};
        int registerSlot = 0;
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
        uint32_t textureID;
    };

    struct TD3D12TextureBuffer
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;  
        D3D12_RESOURCE_STATES state;
        TD3D12TextureBuffer() = default;
    };

    using TD3D12FrameBuffer = TD3D12TextureBuffer;

    struct TD3D12MaterialData
    {
        ///* data */
        //vector<TD3D12ConstantBuffer> mConstantBufferArray;
        vector<TD3D12TextureHander> mTextureBufferArray;
        TD3D12MaterialData(){};
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

    // // 这个结构应该拆分，更多的应该是一个组合的形式，因为pso运行时创建
    // // rootsignature初始化就创建了。
    // struct TD3D12ShaderPSO
    // {
    //     Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    //     Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    //     std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        
    //     // 添加默认构造函数
    //     TD3D12ShaderPSO() = default;

    //     TD3D12ShaderPSO(Microsoft::WRL::ComPtr<ID3D12PipelineState> pso,
    //         Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
    //         std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout)
    //         : pso(pso), rootSignature(rootSignature), inputLayout(inputLayout)
    //     {

    //     };
    // };

    // 存储创建PSO需要的信息
    struct TD3D12PSO
    {
        PSODesc desc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        ComPtr<ID3DBlob> vsBlob;
        ComPtr<ID3DBlob> psBlob;
    };
        


} // namespace EngineCore
