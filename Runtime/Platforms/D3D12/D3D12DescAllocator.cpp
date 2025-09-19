#include "PreCompiledHeader.h"
#include "D3D12DescAllocator.h"
#include "D3D12DescManager.h"
#include "Renderer/RenderAPI.h"
#include "D3D12RenderAPI.h"

namespace EngineCore
{
    D3D12DescAllocator::D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    {
        // 能cast的是指针， 所以对象要转成指针。
        auto md3dDevice = D3D12DescManager::mD3DDevice;
        mHeapType = heapType;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = ConfigAllocatorDescSize(heapType);
		heapDesc.Type = heapType;
		heapDesc.Flags = GetHeapVisible(heapType);
		heapDesc.NodeMask = 0;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.GetAddressOf())));
        isInUse.resize(maxCount, false);
        mDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(mHeapType);
    }

    // 需要取到GPU地址的，要保持着色器可见。
    D3D12_DESCRIPTOR_HEAP_FLAGS D3D12DescAllocator::GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    {
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        default:
            cout << "Wrong D3D12_DESCRIPTOR_HEAP_TYPE Get!!!" << endl;
            return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            break;
        }
    }
    
    
    int D3D12DescAllocator::ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    {
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            maxCount = 1000;
            return 1000;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            maxCount = 100;
            return 100;
        default:
            cout << "Wrong D3D12_DESCRIPTOR_HEAP_TYPE Get!!!" << endl;
            maxCount = 100;
            return 100;
            break;
        }
    }

    TD3D12DescriptorHandle D3D12DescAllocator::CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
    {
        auto handle = GetNextAvaliableDesc();
        handle.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(&RenderAPI::GetInstance())->md3dDevice;

		// 创建CBV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateConstantBufferView(&desc, descriptorHandle);

        // update 时候绑定，用GPU Handle
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
        gpuHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        handle.gpuHandle = gpuHandle;
        return handle;
    }
   
    TD3D12DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
    {
        auto handle = GetNextAvaliableDesc();
        handle.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(&RenderAPI::GetInstance())->md3dDevice;

        // 创建RTV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateRenderTargetView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle;

        return handle;
    }


    TD3D12DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
    {
        auto handle = GetNextAvaliableDesc();
        handle.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(&RenderAPI::GetInstance())->md3dDevice;

		// 创建DSV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateRenderTargetView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle;

        return handle;
    }

    TD3D12DescriptorHandle D3D12DescAllocator::GetNextAvaliableDesc()
    {
        TD3D12DescriptorHandle handle;
        for(int i = 0; i < isInUse.size(); i++)
        {
            if(!isInUse[i])
            {
                isInUse[i] = 1;
                handle.descriptorIdx = i;
                return handle;
            }
        }
        return handle;
    }

}
