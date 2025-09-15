#pragma once
#include "PreCompiledHeader.h"
#include "d3dUtil.h"
#include "D3D12DescAllocator.h"

namespace EngineCore
{
    class D3D12DescManager
    {
    public:
        D3D12DescManager();
        ~D3D12DescManager();
        static D3D12DescManager& GetInstance(){return *mInstance;};
        static void Create(Microsoft::WRL::ComPtr<ID3D12Device> device);
        static Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice;
        
        TD3D12DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		TD3D12DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		TD3D12DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc){};
		TD3D12DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc){};
		TD3D12DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc){};
    private:
        static D3D12DescManager* mInstance;
        vector<D3D12DescAllocator> mDescAllocators;

    };
}