#pragma once
#include "IGPUBuffer.h"
#include "IGPUBufferAllocator.h"

namespace EngineCore
{
    class LinearAllocateBuffer : public IGPUBufferAllocator
    {
    public:
        // 每帧， 暂时做成只分配，不释放。
        LinearAllocateBuffer(const BufferDesc &usage);
        void Destory();

        virtual BufferAllocation Allocate(uint32_t size) override;
        virtual void Free(const BufferAllocation& allocation) override;
        
        virtual uint64_t GetBaseGPUAddress() const override { return m_LinearGPUBuffer->GetGPUVirtualAddress(); } 
        virtual void UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size) override;
        virtual IGPUBuffer* GetGPUBuffer() override { return m_LinearGPUBuffer; };
        virtual void Reset() override;

    private:
        uint32_t m_CurrIndex;
        uint32_t m_MaxSize;
        uint32_t m_Stride;
        IGPUBuffer* m_LinearGPUBuffer;

    };
}