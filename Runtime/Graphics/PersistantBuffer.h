#pragma once
#include "IGPUBuffer.h"
#include "Renderer/RenderStruct.h"
#include "IGPUBufferAllocator.h"
#include <queue>
#include <mutex>

namespace EngineCore
{
    // 基于 FreeList 的持久化 Buffer 分配器
    // 适用于：对象生灭频繁，但数据位置固定的场景 (如 GPUScene)
    class PersistantBuffer : public IGPUBufferAllocator
    {
    public:
        // 初始化
        PersistantBuffer(const BufferDesc &usage);
        void Destory();

        // 基于 Stride 的分配
        virtual BufferAllocation Allocate(uint32_t size) override;
        // 回收 Index 到 FreeList
        virtual void Free(const BufferAllocation& allocation) override;
        
        virtual uint64_t GetBaseGPUAddress() const override { return m_PerSistantBuffer->GetGPUVirtualAddress(); } 
        virtual void UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size) override;
        virtual IGPUBuffer* GetGPUBuffer() override { return m_PerSistantBuffer; };
        virtual void Reset() override;
    private: 
        std::queue<uint32_t> m_FreeIndices;
        IGPUBuffer* m_PerSistantBuffer = nullptr;
        uint32_t m_Stride = 0;
        uint32_t m_MaxElements = 0;
        uint32_t m_NextIndex = 0;
        
    };
}
