#pragma once

namespace EngineCore
{
    class FrameTicket
    {
        public:
            
            inline uint64_t GetFenceValue() const { return mGPUFenceValue.load(std::memory_order_acquire); }
            inline void SetFenceValue(uint64_t fenceValue) { mGPUFenceValue.store(fenceValue, std::memory_order_release); }
            inline uint64_t GetSubmittedFrameID() const { return mSubmittedFrameID.load(std::memory_order_acquire); }
            
            inline bool IsSubmissionReadyForFrame(uint64_t frameID) const
            {
                return GetSubmittedFrameID() == frameID;
            }

            inline void PublishSubmission(uint64_t frameID, uint64_t fenceValue)
            {
                // 先发布 fence，再发布 frameID，主线程看到 frameID 后一定能看到对应 fence。
                mGPUFenceValue.store(fenceValue, std::memory_order_release);
                mSubmittedFrameID.store(frameID, std::memory_order_release);
            }

            inline void operator=(const FrameTicket& ticket)
            {
                mGPUFenceValue = ticket.GetFenceValue();
                mSubmittedFrameID = ticket.GetSubmittedFrameID();
            }

            inline bool operator<=(const FrameTicket& ticket) 
            {
                return this->mGPUFenceValue < ticket.GetFenceValue();
            }

        private:
            
            std::atomic<uint64_t> mGPUFenceValue{0};
            std::atomic<uint64_t> mSubmittedFrameID{UINT64_MAX};

    };
}