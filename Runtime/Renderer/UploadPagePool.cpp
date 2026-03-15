#include "PreCompiledHeader.h"
#include "UploadPagePool.h"


namespace EngineCore
{
    UploadPagePool::UploadPagePool(uint32_t size)
    {
        mPerBufferSize = size;
        mCurrentPage = AcquireFreePage();
    }

    UploadPagePool::~UploadPagePool()
    {
        // todo:: 安全删除？
        for(auto* page : mFreeList)
        {
            delete page;
        }

        for(auto* page : mPendingList)
        {
            delete page;
        }

        for(auto* page : mInFlightList)
        {
            delete page;
        }

        if(mCurrentPage) delete mCurrentPage;
    }

    BufferAllocation UploadPagePool::Allocate(uint32_t size, void* data)
    {
        if(mCurrentPage == nullptr || !mCurrentPage->CanFit(size))
        {
            if(mCurrentPage)
            {
                mPendingList.push_back(mCurrentPage);
                mCurrentPage = AcquireFreePage();
            }
        }
        return mCurrentPage->Allocate(size, data);
    }

    void UploadPagePool::OnSubmitted(const FrameTicket &frameTicket)
    {
        for(auto* page : mPendingList)
        {
            page->ticket = frameTicket;
            mInFlightList.push_back(page);
        }
        if(mCurrentPage == nullptr) return;
        mPendingList.clear();
        mCurrentPage->ticket = frameTicket;
        mInFlightList.push_back(mCurrentPage);
        mCurrentPage = nullptr;
    }

    void UploadPagePool::Recycle(const FrameTicket &frameTicket)
    {
        while(!mInFlightList.empty() && mInFlightList.front()->ticket <= frameTicket)
        {
            auto* page = mInFlightList.front();
            mInFlightList.pop_front();
            page->Reset();
            mFreeList.push_back(page);
        }
    }

    PagedBuffer *UploadPagePool::AcquireFreePage()
    {
        if(!mFreeList.empty())
        {
            PagedBuffer* buffer = mFreeList.front();
            mFreeList.pop_front();
            return buffer;
        }

        return new PagedBuffer(mPerBufferSize);
    }

    void PagedBuffer::Reset()
    {
        mCurrentOffset = 0;
    }

    bool PagedBuffer::CanFit(uint32_t size)
    {
        return mCurrentOffset + size < mMaxSize;
    }

    BufferAllocation PagedBuffer::Allocate(uint32_t size, void* data)
    {
        ASSERT(data);
        BufferAllocation alloc;
        alloc.isValid = false;
        alloc.buffer = mBuffer;
        alloc.size = size;

        // No implicit alignment or padding. 
        // User must ensure 'size' includes any necessary padding for their data structures.
        
        uint64_t offset = 0;

        // 1. Allocate from the end (m_CurrOffset)
        if (mCurrentOffset + size <= mMaxSize)
        {
            alloc.offset = mCurrentOffset;
            alloc.gpuAddress = mBuffer->GetGPUVirtualAddress() + alloc.offset;
            alloc.isValid = true;
            
            mCurrentOffset += size;
            RenderAPI::GetInstance()->UploadBuffer(mBuffer, alloc.offset, data, size);
            return alloc;
        }

        ASSERT_MSG(false, "OUT OF MEMORY in GPUBufferAllocator");
        return alloc;
    }
}