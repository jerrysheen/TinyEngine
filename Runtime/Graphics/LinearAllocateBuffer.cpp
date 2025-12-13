#include "PreCompiledHeader.h"
#include "LinearAllocateBuffer.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    LinearAllocateBuffer::LinearAllocateBuffer(const BufferDesc &usage)
    {
        m_CurrIndex = 0;
        m_MaxSize = (usage.size + 255) & ~255;
        m_Stride = usage.stride;
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
        m_CurrIndex = 0;
    }

    BufferAllocation LinearAllocateBuffer::Allocate(uint32_t size)
    {
        ASSERT(m_CurrIndex * m_Stride + size < m_MaxSize);
        uint32_t count = size;

        BufferAllocation alloc;
        alloc.buffer = m_LinearGPUBuffer;
        alloc.offset = m_CurrIndex * m_Stride;
        alloc.size = count * m_Stride;
        alloc.gpuAddress = m_LinearGPUBuffer->GetGPUVirtualAddress() + alloc.offset;
        alloc.isValid = true;
        alloc.cpuAddress = nullptr;

        m_CurrIndex += count;
        return alloc;
    }

    void LinearAllocateBuffer::Free(const BufferAllocation &allocation)
    {
    }
}