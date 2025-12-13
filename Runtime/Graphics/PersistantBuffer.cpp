#include "PreCompiledHeader.h"
#include "PersistantBuffer.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    PersistantBuffer::PersistantBuffer(const BufferDesc &usage)
    {
        uint64_t stride = 0;
        switch (usage.usage)
        {
        case BufferUsage::ConstantBuffer:
            stride = (usage.stride + 255) & ~255;
            /* code */
            break;
        case BufferUsage::ByteAddressBuffer:
        case BufferUsage::StructuredBuffer:
            stride = (usage.stride + 15) & ~15;
            break;
        default:
            stride = usage.stride;
            break;
        }

        m_PerSistantBuffer = RenderAPI::GetInstance()->CreateBuffer(usage, nullptr);
        m_Stride = stride;
        ASSERT(m_Stride != 0);
        m_MaxElements = usage.size / m_Stride; 
        m_NextIndex = 0;
        std::queue<uint32_t> empty;
        std::swap(m_FreeIndices, empty);
    }

    void PersistantBuffer::Destory()
    {
        if (m_PerSistantBuffer)
        {
            // 假设外部管理或者这里删除，通常建议由创建者 RenderAPI 管理销毁，或者这里显式 delete
            delete m_PerSistantBuffer; 
        }
        m_NextIndex = 0;
        std::queue<uint32_t> empty;
        std::swap(m_FreeIndices, empty);
    }

    BufferAllocation PersistantBuffer::Allocate(uint32_t size)
    {
        BufferAllocation alloc;
        alloc.isValid = false;

        ASSERT(size <= m_Stride);

        uint32_t index = 0;
        if (!m_FreeIndices.empty())
        {
            index = m_FreeIndices.front();
            m_FreeIndices.pop();
        }
        else
        {
            ASSERT_MSG(m_NextIndex <= m_MaxElements, "OUTOF MEMORY");
            index = m_NextIndex++;
        }

        alloc.buffer = m_PerSistantBuffer;
        alloc.offset = index * m_Stride;
        alloc.size = m_Stride; 
        alloc.gpuAddress = m_PerSistantBuffer->GetGPUVirtualAddress() + alloc.offset;
        alloc.isValid = true;

        // 如果是 Upload Heap，这里不一定立刻 Map，可以在 UpdateChunk 时做
        alloc.cpuAddress = nullptr;

        return alloc;
    }

    void PersistantBuffer::Free(const BufferAllocation &allocation)
    {
        ASSERT(allocation.isValid && allocation.buffer == m_PerSistantBuffer);
        // 计算 Index
        uint32_t index = (uint32_t)(allocation.offset / m_Stride);
        m_FreeIndices.push(index);
    }

    void PersistantBuffer::UploadBuffer(const BufferAllocation &alloc, void *data, uint32_t size)
    {
        ASSERT(alloc.isValid && data && m_PerSistantBuffer);

        // Default Heap
        RenderAPI::GetInstance()->UploadBuffer(m_PerSistantBuffer, alloc.offset, data, size);
    }

    void PersistantBuffer::Reset()
    {
        m_NextIndex = 0;
        std::queue<uint32_t> empty;
        std::swap(m_FreeIndices, empty);
    }
}
