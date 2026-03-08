#pragma once
#include "Graphics/GPUBufferAllocator.h"
#include "Renderer/RenderCommand.h"
#include "Scene/SceneStruct.h"
#include "Renderer/RenderUniforms.h"
#include <atomic>
#include <cstdint>

namespace EngineCore
{
    class FrameContext
    {
    public:
        FrameContext();
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;
        //GPUBufferAllocator* perFrameBatchBuffer;
        GPUBufferAllocator* perFrameUploadBuffer;
        GPUBufferAllocator* visibilityBuffer;


        vector<uint32_t> mDirtyFlags;
        vector<PerObjectData> mPerObjectDatas;
        void EnsureCapacity(uint32_t renderID);
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void Reset();
        void UpdateShadowData(uint32_t renderID, CPUSceneView& cpuScene);
        void UpdatePerFrameDirtyNode(CPUSceneView& cpuScene);
        ~FrameContext();
        void UpdateDirtyFlags(uint32_t renderID, uint32_t flags);
        void UploadCopyOp();
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
    private:
        void TryFreeRenderProxyByRenderIndex(uint32_t renderID);
        void TryFreePerObjectDataAndAABBData(uint32_t renderID);
        
        vector<uint32_t> mPerFrameDirtyID;   
        vector<CopyOp> mCopyOpsObject;
        vector<CopyOp> mCopyOpsAABB;
        vector<CopyOp> mCopyOpsProxy;
        vector<CopyOp> mCopyOpsVisibility;
        std::atomic<uint64_t> mGPUFenceValue{0};
        std::atomic<uint64_t> mSubmittedFrameID{UINT64_MAX};
    };
}
