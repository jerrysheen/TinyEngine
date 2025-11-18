#include "PreCompiledHeader.h"
#include "D3D12PerDrawAllocator.h"

namespace EngineCore
{
    D3D12PerDrawAllocator::~D3D12PerDrawAllocator()
    {
    }

    D3D12PerDrawAllocator::D3D12PerDrawAllocator(ID3D12Device *device, uint32_t capacityBytes)
    {
            // 创建 Upload Heap buffer
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(capacityBytes);
        
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mBuffer)
        );
        
        // 持久 Map
        mBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedCPU));
        mGPUBaseAddress = mBuffer->GetGPUVirtualAddress();
        mCapacityBytes = capacityBytes;
    }



    PerDrawHandle D3D12PerDrawAllocator::Allocate(uint32_t size)
    {
        uint32_t alignedSize = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
        PerDrawHandle handle;
        handle.offset = mCurrentOffset;
        handle.size = alignedSize;
        
        mCurrentOffset += alignedSize;
        ASSERT(mCurrentOffset < mCapacityBytes);
        handle.destPtr = mMappedCPU + handle.offset;
        return handle;
    }

    uint8_t* D3D12PerDrawAllocator::GetCPUAddress(PerDrawHandle h)
    {
        return mMappedCPU + h.offset;
    }

    uint64_t D3D12PerDrawAllocator::GetGPUBaseAddress() const
    {
        return mGPUBaseAddress;
    }

    void D3D12PerDrawAllocator::ResetFrame()
    {
        mCurrentOffset = 0;
    }
}