#include "PreCompiledHeader.h"
#include "GPUBufferAllocator.h"
#include "Renderer/RenderAPI.h"
#include "CoreAssert.h"
#include <algorithm>

namespace EngineCore
{
    GPUBufferAllocator::GPUBufferAllocator(const BufferDesc &usage)
    {
        m_MaxSize = usage.size;
        m_Buffer = RenderAPI::GetInstance()->CreateBuffer(usage, nullptr);
        m_CurrOffset = 0;
    }

    GPUBufferAllocator::~GPUBufferAllocator()
    {
        Destory();
    }

    void GPUBufferAllocator::Destory()
    {
        if(m_Buffer)
        {
            delete m_Buffer;
            m_Buffer = nullptr;
        }
        
        m_FreeRanges.clear();
        m_CurrOffset = 0;
    }

    uint64_t GPUBufferAllocator::GetBaseGPUAddress() const 
    { 
        return m_Buffer ? m_Buffer->GetGPUVirtualAddress() : 0; 
    }
    
    IGPUBuffer* GPUBufferAllocator::GetGPUBuffer() 
    { 
        return m_Buffer; 
    }

    void GPUBufferAllocator::UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size)
    {
        ASSERT(alloc.isValid && data && m_Buffer);
        RenderAPI::GetInstance()->UploadBuffer(m_Buffer, alloc.offset, data, size);
    }

    void GPUBufferAllocator::Reset()
    {
        m_CurrOffset = 0;
        m_FreeRanges.clear();
    }

    BufferAllocation GPUBufferAllocator::Allocate(uint32_t size)
    {
        BufferAllocation alloc;
        alloc.isValid = false;
        alloc.buffer = m_Buffer;
        alloc.size = size;

        // No implicit alignment or padding. 
        // User must ensure 'size' includes any necessary padding for their data structures.
        
        uint64_t offset = 0;

        // 1. Try to find a free block in the free list
        if (FindFreeBlock(size, offset))
        {
            alloc.offset = offset;
            alloc.gpuAddress = m_Buffer->GetGPUVirtualAddress() + alloc.offset;
            alloc.isValid = true;
            return alloc;
        }

        // 2. Allocate from the end (m_CurrOffset)
        if (m_CurrOffset + size <= m_MaxSize)
        {
            alloc.offset = m_CurrOffset;
            alloc.gpuAddress = m_Buffer->GetGPUVirtualAddress() + alloc.offset;
            alloc.isValid = true;
            
            m_CurrOffset += size;
            return alloc;
        }

        ASSERT_MSG(false, "OUT OF MEMORY in GPUBufferAllocator");
        return alloc;
    }

    bool GPUBufferAllocator::FindFreeBlock(uint32_t size, uint64_t& outOffset)
    {
        // Simple first-fit strategy
        for (auto it = m_FreeRanges.begin(); it != m_FreeRanges.end(); ++it)
        {
            if (it->size >= size)
            {
                outOffset = it->offset;

                uint64_t remainingSize = it->size - size;
                
                if (remainingSize > 0)
                {
                    it->offset = it->offset + size;
                    it->size = remainingSize;
                }
                else
                {
                    // Exact match, remove node.
                    // NOTE: Use erase (O(N)) instead of swap-and-pop (O(1)) to maintain sorted order.
                    // Sorted order is required for memory coalescing (merging adjacent blocks) in Free().
                    // Without coalescing, fragmentation would eventually exhaust the buffer.
                    m_FreeRanges.erase(it);
                }

                return true;
            }
        }
        return false;
    }

    void GPUBufferAllocator::Free(const BufferAllocation& allocation)
    {

        // Free exactly what was allocated
        uint32_t size = allocation.size;
        
        FreeRange newRange;
        newRange.offset = allocation.offset;
        newRange.size = size;

        // Insert and merge
        // Find position to insert to keep sorted by offset
        auto it = std::upper_bound(m_FreeRanges.begin(), m_FreeRanges.end(), newRange, 
            [](const FreeRange& a, const FreeRange& b) {
                return a.offset < b.offset;
            });
        
        auto insertedIt = m_FreeRanges.insert(it, newRange);
        
        // Merge with next
        if (insertedIt != m_FreeRanges.end() - 1)
        {
            auto nextIt = insertedIt + 1;
            if (insertedIt->offset + insertedIt->size == nextIt->offset)
            {
                insertedIt->size += nextIt->size;
                m_FreeRanges.erase(nextIt);
            }
        }
        
        // Merge with prev
        if (insertedIt != m_FreeRanges.begin())
        {
            auto prevIt = insertedIt - 1;
            if (prevIt->offset + prevIt->size == insertedIt->offset)
            {
                prevIt->size += insertedIt->size;
                m_FreeRanges.erase(insertedIt);
            }
        }
    }
}
