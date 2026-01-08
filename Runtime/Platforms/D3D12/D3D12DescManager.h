#pragma once
#include "PreCompiledHeader.h"
#include "d3dUtil.h"
#include "D3D12DescAllocator.h"
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class D3D12DescManager
    {
    public:
        D3D12DescManager();
        ~D3D12DescManager();
        static D3D12DescManager* GetInstance(){return mInstance;};
        static void Create(Microsoft::WRL::ComPtr<ID3D12Device> device);
        static Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice;
        
        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        
        void ResetFrameAllocator();
        DescriptorHandle GetFrameCbvSrvUavAllocator(int count);
        DescriptorHandle GetFrameSamplerAllocator(int count);
        vector<D3D12DescAllocator> mDescAllocators;
        vector<D3D12DescAllocator> mFrameAllocators;
        
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetFrameCbvSrvUavHeap()
        {
            return mFrameAllocators[0].mHeap;
        };
    private:
        static D3D12DescManager* mInstance;

    };
}