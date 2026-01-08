#include "PreCompiledHeader.h"
#include "D3D12DescManager.h"
#include "D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    D3D12DescManager* D3D12DescManager::mInstance = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device>  D3D12DescManager::mD3DDevice = nullptr;
    D3D12DescManager::D3D12DescManager()
    {
        for(int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
        {
            mDescAllocators.emplace_back(D3D12_DESCRIPTOR_HEAP_TYPE(i), false);
        }

        for(int i = 0; i < 2; i++)
        {
            mFrameAllocators.emplace_back(D3D12_DESCRIPTOR_HEAP_TYPE(i), true);
        }

    }

    D3D12DescManager::~D3D12DescManager(){};

    void D3D12DescManager::Create(Microsoft::WRL::ComPtr<ID3D12Device> device)
    {
        mD3DDevice = device;
        if(mInstance == nullptr)
        {
            mInstance = new D3D12DescManager();
        }
        else
        {
            std::cout << "D3D12DescManager already created" << std::endl;
            return;
        }
    }

    DescriptorHandle D3D12DescManager::CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
    {
        return mDescAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].CreateDescriptor(desc);
    }

    DescriptorHandle D3D12DescManager::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
    {
        return mDescAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].CreateDescriptor(resource, desc);
    }

    DescriptorHandle D3D12DescManager::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) 
    {
        return mDescAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].CreateDescriptor(resource, desc);
    }

    DescriptorHandle D3D12DescManager::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
    {
        return mDescAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].CreateDescriptor(resource, desc);
    }

    void D3D12DescManager::ResetFrameAllocator()
    {
        for(auto& alloctor : mFrameAllocators)
        {
            alloctor.Reset();
        }        
    }

    DescriptorHandle D3D12DescManager::GetFrameCbvSrvUavAllocator(int count)
    {
        return mFrameAllocators[0].GetFrameAllocator(count);
    }

    DescriptorHandle D3D12DescManager::GetFrameSamplerAllocator(int count)
    {
        return mFrameAllocators[1].GetFrameAllocator(count);
    }

    // heap 堆
    // heap offset


}