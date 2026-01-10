#include "PreCompiledHeader.h"
#include "D3D12DescAllocator.h"
#include "D3D12DescManager.h"
#include "Renderer/RenderAPI.h"
#include "D3D12RenderAPI.h"

namespace EngineCore
{
    D3D12DescAllocator::D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible)
    {
        // 能cast的是指针， 所以对象要转成指针。
        auto md3dDevice = D3D12DescManager::mD3DDevice;
        mHeapType = heapType;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = ConfigAllocatorDescSize(heapType);
		heapDesc.Type = heapType;
        // 如果强制开启 ShaderVisible，或者它是 FrameHeap (且符合类型)，则设为 Visible
		heapDesc.Flags = (isShaderVisible || (isFrameHeap && (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER))) 
            ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            
		heapDesc.NodeMask = 0;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.GetAddressOf())));
        mDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(mHeapType);
    }

    // 需要取到GPU地址的，要保持着色器可见。
    D3D12_DESCRIPTOR_HEAP_FLAGS D3D12DescAllocator::GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap)
    {
        switch (heapType)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return isFrameHeap ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
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
            maxCount = 2000; // 大幅增加 Size 以支持 Bindless + Dynamic
            return 2000;
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

    DescriptorHandle D3D12DescAllocator::CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc)
    {
        auto handle = AllocateStaticHandle();
        //handle.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(RenderAPI::GetInstance())->md3dDevice;

		// 创建CBV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateConstantBufferView(&desc, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;
        // update 时候绑定，用GPU Handle
        // 不用有GPU地址， 因为这个堆运行时会拷贝到另外一个堆，这个堆只能CPU可见。
        //CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
        //gpuHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        //handle.gpuHandle = gpuHandle;
        return handle;
    }
   
    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
    {
        auto handle = AllocateStaticHandle();
        //handle.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(RenderAPI::GetInstance())->md3dDevice;

        // 创建RTV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateRenderTargetView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;

        return handle;
    }


    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
    {
        auto handle = AllocateStaticHandle();
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(RenderAPI::GetInstance())->md3dDevice;

		// 创建DSV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateDepthStencilView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;

        return handle;
    }

    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
    {
        auto handle = AllocateStaticHandle();
        auto mD3D12Device = static_cast<D3D12RenderAPI*>(RenderAPI::GetInstance())->md3dDevice;

        // 创建CBV
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
        descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        mD3D12Device->CreateShaderResourceView(resource.Get(), &desc, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;
        // update 时候绑定，用GPU Handle
        // 不用有GPU地址， 因为这个堆运行时会拷贝到另外一个堆，这个堆只能CPU可见。
        //CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
        //gpuHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        //handle.gpuHandle = gpuHandle;
        return handle;
    }

    DescriptorHandle D3D12DescAllocator::AllocateStaticHandle()
    {
        DescriptorHandle handle;
        
        // 分配Persistane的handle
        if (freeIndexList.size() > 0) 
        {
            handle.descriptorIdx = freeIndexList.back();
            freeIndexList.pop_back();
            return handle;
        }
        handle.descriptorIdx = currentOffset;
        currentOffset++;
        return handle;
    }

    void D3D12DescAllocator::Reset()
    {
        // 动态分配部分（currentOffset管理）回到 dynamicStartOffset
        if (dynamicStartOffset > 0)
        {
            // 如果是 FrameAllocator，dynamicStartOffset 默认为 0，全量重置
            // 如果是 GlobalBindlessHeap，dynamicStartOffset > 0，只重置动态区
             currDynamicoffset = dynamicStartOffset;
        }
        else
        {
            freeIndexList.clear();
            currentOffset = 0;
        }
    }

    void D3D12DescAllocator::CleanPerFrameData()
    {
        ASSERT(dynamicStartOffset > 0);
        currDynamicoffset = dynamicStartOffset;
    }

    DescriptorHandle D3D12DescAllocator::AllocateDynamicSpace(int count)
    {
         ASSERT_MSG(currDynamicoffset + count < maxCount, "Global Heap Dynamic Space Run out!");
         ASSERT(dynamicStartOffset > 0);

         //直接给handle，然后更新offSet;
         DescriptorHandle handle;
         handle.descriptorIdx = currDynamicoffset;
         D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
         D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

         handle.cpuHandle = cpuStart.ptr + currDynamicoffset * mDescriptorSize;
         handle.gpuHandle = gpuStart.ptr + currDynamicoffset * mDescriptorSize;
         currDynamicoffset += count;

         return handle;
    }

}
