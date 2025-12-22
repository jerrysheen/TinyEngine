#include "PreCompiledHeader.h"
#include "LinearAllocateBuffer.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    LinearAllocateBuffer::LinearAllocateBuffer(const BufferDesc &usage)
    {
        m_CurrOffset = 0;
        m_MaxSize = (usage.size + 255) & ~255;
        m_LinearGPUBuffer = RenderAPI::GetInstance()->CreateBuffer(usage, nullptr);
    }

    void LinearAllocateBuffer::Destory()
    {
        if(m_LinearGPUBuffer)
        {
            delete m_LinearGPUBuffer;
        }
    }

    void LinearAllocateBuffer::UploadBuffer(const BufferAllocation &alloc, void *data, uint32_t size)
    {       
        ASSERT(alloc.isValid && data && m_LinearGPUBuffer);
        // Default Heap
        RenderAPI::GetInstance()->UploadBuffer(m_LinearGPUBuffer, alloc.offset, data, size);
    }

    void LinearAllocateBuffer::Reset()
    {
        m_CurrOffset = 0;
    }

    BufferAllocation LinearAllocateBuffer::Allocate(uint32_t size)
    {
        uint32_t alignedSize = (size + 3) & ~3; 
        ASSERT(m_CurrOffset + alignedSize < m_MaxSize);
        BufferAllocation alloc;
        alloc.buffer = m_LinearGPUBuffer;
        alloc.offset = m_CurrOffset;
        alloc.size = alignedSize;
        alloc.gpuAddress = m_LinearGPUBuffer->GetGPUVirtualAddress() + alloc.offset;
        alloc.isValid = true;
        alloc.cpuAddress = nullptr;

        m_CurrOffset += alignedSize;
        return alloc;
    }

    void LinearAllocateBuffer::Free(const BufferAllocation &allocation)
    {
    }
}