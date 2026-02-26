#pragma once
#include "d3dUtil.h"


#include "Graphics/MeshUtils.h"
#include "Graphics/Material.h"
#include "Core/InstanceID.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/IGPUResource.h"

namespace EngineCore
{


    struct TD3D12ConstantBuffer
    {
        ComPtr<ID3D12Resource> mBufferResource;
        int mSize = 0;
        void* mCpuAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = 0;
        DescriptorHandle handleCBV = {};
        int registerSlot = 0;
        TD3D12ConstantBuffer(){};
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

    // 存储创建PSO需要的信息
    struct TD3D12PSO
    {
        PSODesc desc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        ComPtr<ID3DBlob> vsBlob;
        ComPtr<ID3DBlob> psBlob;
    };
        
    struct TD3D12MaterialStateCache
    {
        uint64_t allObjectDataGpuAddress = 0;
        uint64_t allMaterialDataGpuAddress = 0;
        uint64_t perDrawInstanceObjectsListGpuAddress = 0;
        void Reset() 
        {
            allObjectDataGpuAddress = 0;
            allMaterialDataGpuAddress = 0;
            perDrawInstanceObjectsListGpuAddress = 0;
        }
    };

} // namespace EngineCore
