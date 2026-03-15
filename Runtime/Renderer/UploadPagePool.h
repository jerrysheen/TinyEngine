#pragma once
#include "FrameTicket.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Renderer/RenderStruct.h"
#include "Renderer/RenderAPI.h"
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    struct PagedBuffer
    {
        PagedBuffer(uint32_t size)
        {
            mMaxSize = size;
            BufferDesc desc;
            desc.debugName = L"PagedBuffer";
            desc.usage = BufferUsage::ConstantBuffer;
            desc.memoryType = BufferMemoryType::Upload;
            desc.size = size;
            mBuffer = RenderAPI::GetInstance()->CreateBuffer(desc, nullptr);
        };

        ~PagedBuffer()
        {
            delete mBuffer;
        }
        
        inline void Reset();
        inline bool CanFit(uint32_t size);
        BufferAllocation Allocate(uint32_t size, void* data);
        FrameTicket ticket;
    private:
        IGPUBuffer* mBuffer = nullptr;
        uint32_t mMaxSize;
        uint32_t mCurrentOffset = 0;
    };

    class UploadPagePool
    {
    public:
        UploadPagePool(uint32_t size);
        ~UploadPagePool();

        BufferAllocation Allocate(uint32_t size, void* data);
        void OnSubmitted(const FrameTicket& frameTicket);
        void Recycle(const FrameTicket& frameTicket);
        PagedBuffer* AcquireFreePage();
    private:
        uint32_t mPerBufferSize;    
        PagedBuffer* mCurrentPage;
        std::deque<PagedBuffer*> mPendingList;
        std::deque<PagedBuffer*> mFreeList;
        std::deque<PagedBuffer*> mInFlightList;
    };

}