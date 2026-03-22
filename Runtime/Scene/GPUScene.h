#pragma once
#include "Graphics/IGPUResource.h"
#include "Math/Matrix4x4.h"
#include "Renderer/RenderUniforms.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Core/PublicStruct.h"
#include "Core/Allocator/LinearAllocator.h"
#include "Graphics/ComputeShader.h"
#include "Resources/ResourceHandle.h"
#include "Renderer/FrameTicket.h"
#include "Renderer/UploadPagePool.h"

namespace EngineCore
{
    class CPUSceneView;
    class GPUScene
    {
    public:
        GPUScene();
        void Create();
        void Update(uint32_t currentFrameIndex);
        void Destroy();
        void EndFrame();
        void BeginFrame();
        
        BufferAllocation GetSinglePerMaterialData();
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdatePerFrameDirtyNode(CPUSceneView& view);
        void UploadCopyOp();
        
        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);
        void UpdateDirtyFlags(uint32_t renderID, uint32_t flags);
        void UpdateShadowData(uint32_t renderID, CPUSceneView& cpuScene);

        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }
        inline uint32_t GetMaxFrameCount() const { return MAX_FRAME_INFLIGHT; }

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }

        inline void SetCurrentFrame(uint32_t currentFrame)
        {
            mCurrentFrameID = currentFrame;
        }

        inline void SetUploadPagePool(UploadPagePool* pool)
        {
            mUploadPagePool = pool;
        }

        inline IGPUBuffer* GetRenderProxyBuffer() { return renderProxyBuffer->GetGPUBuffer(); }
        inline uint64_t GetAllObjectDataBufferAddress() { return allObjectDataBuffer->GetBaseGPUAddress(); }
        inline IGPUBuffer* GetAllObjectDataBuffer() { return allObjectDataBuffer->GetGPUBuffer(); }
        inline uint64_t GetAllAABBDataBufferAddress() { return allAABBBuffer->GetBaseGPUAddress(); }
        inline IGPUBuffer* GetAllAABBDataBuffer() { return allAABBBuffer->GetGPUBuffer(); }

        inline IGPUBuffer* GetCurrentVisibilityBuffer(uint32_t frameID) { return GetVisibilityBufferByFrameID(frameID)->GetGPUBuffer(); }
        inline GPUBufferAllocator* GetVisibilityBufferByFrameID(uint32_t frameID) { return visibilityBuffer[frameID % 3]; }
    private:
        void EnsureCapacity(uint32_t renderID);
    private:
        void SetCurrentContext();
        void TryFreeRenderProxyByRenderIndex(uint32_t renderID);
        void TryFreePerObjectDataAndAABBData(uint32_t renderID);
        
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;
        GPUBufferAllocator* allMaterialDataBuffer;

        
        vector<uint32_t> mDirtyFlags;
        vector<uint32_t> mPerFrameDirtyID;
        vector<PerObjectData> mPerObjectDatas;

        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
        static const int MAX_FRAME_INFLIGHT = 3;
        uint32_t mCurrentFrameID = 0;

        UploadPagePool* mUploadPagePool;


        std::vector<CopyOp>* mCurrentCopyOp;
        std::vector<CopyOp> mFrameCopyOp[MAX_FRAME_INFLIGHT];
        
        GPUBufferAllocator* mCurrVisibilityBuffer;
        GPUBufferAllocator* visibilityBuffer[3];
    };

}
