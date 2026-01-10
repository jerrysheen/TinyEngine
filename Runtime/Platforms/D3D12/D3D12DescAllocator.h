#pragma once
#include "d3dUtil.h"
#include "D3D12Struct.h"
#include "PreCompiledHeader.h"
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class D3D12DescAllocator
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible = false);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        DescriptorHandle AllocateStaticHandle();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

        void Reset();
        void CleanPerFrameData();

        // 设置动态分配的起始位置（用于混合 Heap 模式）
        void SetDynamicStartOffset(int offset) 
        { 
            dynamicStartOffset = offset;
            currDynamicoffset = offset;
        }
        // 专门用于 Global Heap 的动态分配
        DescriptorHandle AllocateDynamicSpace(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        //std::vector<bool> isInUse;
        std::vector<int> freeIndexList;

        int dynamicStartOffset = 0; // 记录动态分配的起始点，Reset 时回到这里
        int currDynamicoffset = 0;
        
        int currentOffset = 0;
        uint32_t startIndex = 0;
        uint32_t mDescriptorSize = 0;
        int maxCount = 0;
    };

}