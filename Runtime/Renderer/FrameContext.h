#pragma once
#include "Graphics/GPUBufferAllocator.h"
#include "Renderer/RenderCommand.h"
#include "Scene/SceneStruct.h"
#include "Renderer/RenderUniforms.h"

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
    private:
        void TryFreeRenderProxyByRenderIndex(uint32_t renderID);
        void TryFreePerObjectDataAndAABBData(uint32_t renderID);
        
        vector<uint32_t> mPerFrameDirtyID;   
        vector<CopyOp> mCopyOpsObject;
        vector<CopyOp> mCopyOpsAABB;
        vector<CopyOp> mCopyOpsProxy;
        vector<CopyOp> mCopyOpsVisibility;

    };
}