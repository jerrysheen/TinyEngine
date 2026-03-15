#include "PreCompiledHeader.h"
#include "GPUScene.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUResource.h"
#include "Renderer/RenderUniforms.h"
#include "Renderer/RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/RenderBackend.h"
#include "Renderer/BatchManager.h"
#include "Scene/SceneStruct.h"

namespace EngineCore
{
    void GPUScene::Destroy()
    {
        delete allMaterialDataBuffer;
        delete allObjectDataBuffer;
        delete allAABBBuffer;
        delete renderProxyBuffer;
        for (int i = 0; i < 3; i++) 
        {
            delete visibilityBuffer[i];
        }
    }

    void GPUScene::EndFrame()
    {
    }

    void GPUScene::Create()
    {
        // 强烈建议使用 512， 注意，这边都是Byte
        // 理由：
        // 1. 容纳 32 个 float4 向量，足够应对 Uber Shader + 矩阵 + 几十个 TextureID。
        // 2. 即使浪费了一半空间，1万个材质也只多占 2.5MB 显存，完全可忽略。
        // 3. 512 是 256 (D3D12 ConstantBuffer 对齐) 的倍数，也是 16 (float4) 的倍数，对齐非常友好。
        BufferDesc desc;
        desc.debugName = L"AllObjectBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(PerObjectData) * 10000;
        desc.stride = sizeof(PerObjectData);
        desc.usage = BufferUsage::StructuredBuffer;
        allObjectDataBuffer = new GPUBufferAllocator(desc);


        // 创建RenderProxy
        desc.debugName = L"RenderProxyBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(RenderProxy);
        desc.usage = BufferUsage::StructuredBuffer;
        renderProxyBuffer = new GPUBufferAllocator(desc);

        desc.debugName = L"VisibilityBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 4 * 10000;
        desc.stride = 4 * 10000;
        desc.usage = BufferUsage::StructuredBuffer;
        for (int i = 0; i < 3; i++) 
        {
            visibilityBuffer[i] = new GPUBufferAllocator(desc);
        }
       
       
        desc.debugName = L"allAABBBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(AABB);
        desc.stride = 10000 * sizeof(AABB);
        desc.usage = BufferUsage::StructuredBuffer;
        allAABBBuffer = new GPUBufferAllocator(desc);
  
        desc.debugName = L"AllMaterialBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 512 * 10000; // 预分配约 5MB
        desc.stride = 512;       // PageSize
        desc.usage = BufferUsage::ByteAddressBuffer;
        allMaterialDataBuffer = new GPUBufferAllocator(desc);

        // 创建Compute Shader
        string path = PathSettings::ResolveAssetPath("Shader/GPUCulling.hlsl");
        GPUCullingShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);

        mCurrentCopyOp = &mFrameCopyOp[0];
        mCurrVisibilityBuffer = visibilityBuffer[0];
    }

    void GPUScene::Update(uint32_t currentFrameIndex)
    {
        mCurrentFrameID = currentFrameIndex;
        SetCurrentContext();
    }


    BufferAllocation GPUScene::GetSinglePerMaterialData()
    {
        return allMaterialDataBuffer->Allocate(512);
    }


    BufferAllocation GPUScene::UploadDrawBatch(void *data, uint32_t size)
    {
        ASSERT(data != nullptr);
        ASSERT(size > 0);

        BufferAllocation destAllocation = GetVisibilityBufferByFrameID(mCurrentFrameID)->Allocate(size);
        BufferAllocation srcAllocation = mUploadPagePool->Allocate(size, data);

        CopyOp op = {};
        op.srcUploadBuffer = srcAllocation.buffer;
        op.destDefaultBuffer = destAllocation.buffer;
        op.srcOffset = srcAllocation.offset;
        op.dstOffset = destAllocation.offset;
        op.size = size;
        mCurrentCopyOp->push_back(op);

        return destAllocation;
    }

    void GPUScene::UploadCopyOp()
    {
        size_t start = 0;
        size_t end = 1;
        while(start < mCurrentCopyOp->size())
        {
            IGPUBuffer* currSrc = (*mCurrentCopyOp)[start].srcUploadBuffer;
            IGPUBuffer* currDst = (*mCurrentCopyOp)[start].destDefaultBuffer;
            end = start + 1;
            while(end < mCurrentCopyOp->size() 
                && (*mCurrentCopyOp)[end].srcUploadBuffer == currSrc
                && (*mCurrentCopyOp)[end].destDefaultBuffer == currDst) ++end;
            
            Payload_CopyBufferRegion payload;
            payload.srcUploadBuffer = currSrc;
            payload.destDefaultBuffer = currDst;
            payload.copyList = mCurrentCopyOp->data() + start;
            payload.count    = end - start;
            RenderBackend::GetInstance()->CopyBufferRegion(payload);
            start = end;  
        }
    }

    void GPUScene::ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view)
    {   
        EnsureCapacity(renderID); 
        UpdateDirtyFlags(renderID, flags);
        // 记录我当前脏ID即可
        UpdateShadowData(renderID, view);
    }

    void GPUScene::EnsureCapacity(uint32_t renderID)
    {
        int count = mDirtyFlags.size();
        int need = renderID + 1;
        if(count < need)
        {
            mDirtyFlags.resize(need, 0);
            mPerObjectDatas.resize(need);
        }
    }

    GPUScene::GPUScene()
    {

    }


    void GPUScene::UpdateDirtyFlags(uint32_t renderID, uint32_t flags)
    {
        uint32_t& oldFlag = mDirtyFlags[renderID];
        if(oldFlag == 0) mPerFrameDirtyID.push_back(renderID);
        if(oldFlag & (uint32_t)NodeDirtyFlags::Created)
        {
            if(flags & (uint32_t)NodeDirtyFlags::Destory)
            {
                oldFlag = (uint32_t)NodeDirtyFlags::None;
            }
            return;
        }

        if(flags & (uint32_t)NodeDirtyFlags::Destory)
        {
            oldFlag = (uint32_t)NodeDirtyFlags::Destory;
            return;
        }

        oldFlag |= flags;
        return;    
    }

    void GPUScene::UpdateShadowData(uint32_t renderID, CPUSceneView &view)
    {
        uint32_t flags = mDirtyFlags[renderID];
        if(flags == 0) return;
        if(flags & (uint32_t)NodeDirtyFlags::Destory)
        {
            TryFreeRenderProxyByRenderIndex(renderID);
            TryFreePerObjectDataAndAABBData(renderID);
            mPerObjectDatas[renderID].renderProxyCount = 0;
            return;
        }

        if(flags & (uint32_t)NodeDirtyFlags::TransformDirty || flags & (uint32_t)NodeDirtyFlags::Created)
        {
            mPerObjectDatas[renderID].objectToWorld = view.objectToWorldList[renderID];
        }

        if(flags & (uint32_t)NodeDirtyFlags::MeshDirty || flags & (uint32_t)NodeDirtyFlags::Created)
        {
            Mesh* mesh = ResourceManager::GetInstance()->GetResource<Mesh>(view.meshList[renderID]);
            mPerObjectDatas[renderID].baseVertexLocation = mesh->vertexAllocation->offset;
        }

        if(flags & (uint32_t)NodeDirtyFlags::MaterialDirty || flags & (uint32_t)NodeDirtyFlags::Created)
        {
            AssetID meshID  = view.meshList[renderID];
            AssetID materialID = view.materialList[renderID];
            uint32_t layer = view.layerList[renderID];

            // 要先释放旧RenderProxy
            TryFreeRenderProxyByRenderIndex(renderID);
            vector<RenderProxy> proxyList = BatchManager::GetInstance()->GetAvaliableRenderProxyList(meshID, materialID, layer);
            if (proxyList.size() == 0) return;
            BufferAllocation dstAllocation = renderProxyBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
            dstAllocation.buffer = renderProxyBuffer->GetGPUBuffer();
            mPerObjectDatas[renderID].renderProxyCount = proxyList.size();
            mPerObjectDatas[renderID].renderProxyStartIndex = static_cast<uint32_t>(dstAllocation.offset / sizeof(RenderProxy));

            // copyop:
            BufferAllocation tempAllocation = mUploadPagePool->Allocate(proxyList.size() * sizeof(RenderProxy),  proxyList.data());
            CopyOp op;
            op.srcUploadBuffer = tempAllocation.buffer;
            op.destDefaultBuffer = dstAllocation.buffer;
            op.srcOffset = tempAllocation.offset;
            op.dstOffset = dstAllocation.offset;
            op.size = proxyList.size() * sizeof(RenderProxy);
            mCurrentCopyOp->push_back(op);
        }
    }

    void GPUScene::UpdatePerFrameDirtyNode(CPUSceneView& cpuScene)
    {
        vector<CopyOp> mCopyOpsObject;
        vector<CopyOp> mCopyOpsAABB;
        for(int i = 0; i < mPerFrameDirtyID.size(); i++)
        {
            int renderID = mPerFrameDirtyID[i];
            uint32_t dirtyFlags = mDirtyFlags[renderID];

            if(dirtyFlags == 0 || (dirtyFlags & (uint32_t)NodeDirtyFlags::Destory)) continue;
            
            if(dirtyFlags & (uint32_t) NodeDirtyFlags::Created)
            {
                BufferAllocation allocation = allObjectDataBuffer->Allocate(sizeof(PerObjectData));
                BufferAllocation tempAllocation = mUploadPagePool->Allocate(sizeof(PerObjectData), &mPerObjectDatas[renderID]);
                CopyOp op;
                op.srcUploadBuffer = tempAllocation.buffer;
                op.destDefaultBuffer = allocation.buffer;
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                ASSERT(op.dstOffset == renderID * sizeof(PerObjectData));
                op.size = sizeof(PerObjectData);
                mCopyOpsObject.push_back(op);

                allocation = allAABBBuffer->Allocate(sizeof(AABB));
                tempAllocation = mUploadPagePool->Allocate(sizeof(AABB), &mPerObjectDatas[renderID]);
                op.srcUploadBuffer = tempAllocation.buffer;
                op.destDefaultBuffer = allocation.buffer;
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                op.size = sizeof(AABB);
                mCopyOpsAABB.push_back(op);
            }

            if((dirtyFlags & (uint32_t)NodeDirtyFlags::TransformDirty) || (dirtyFlags & (uint32_t)NodeDirtyFlags::MeshDirty))
            {
                BufferAllocation dstAlloc;
                dstAlloc.offset = renderID * sizeof(PerObjectData);  // 固定位置！
                dstAlloc.buffer = allObjectDataBuffer->GetGPUBuffer();
                BufferAllocation tempAlloc = mUploadPagePool->Allocate(sizeof(PerObjectData), &mPerObjectDatas[renderID]);
                CopyOp op;
                op.srcUploadBuffer = tempAlloc.buffer;
                op.destDefaultBuffer = dstAlloc.buffer;
                op.srcOffset = tempAlloc.offset;
                op.dstOffset = dstAlloc.offset;
                op.size = sizeof(PerObjectData);
                mCopyOpsObject.push_back(op);
            }

            if((dirtyFlags & (uint32_t)NodeDirtyFlags::MeshDirty) || (dirtyFlags & (uint32_t)NodeDirtyFlags::TransformDirty))
            {
                BufferAllocation dstAlloc;
                dstAlloc.offset = renderID * sizeof(AABB);           // 固定位置！
                dstAlloc.buffer = allAABBBuffer->GetGPUBuffer();
                BufferAllocation tempAlloc = mUploadPagePool->Allocate(sizeof(AABB), (void*)&cpuScene.worldBoundsList[renderID]);
                CopyOp op;
                op.srcUploadBuffer = tempAlloc.buffer;
                op.destDefaultBuffer = dstAlloc.buffer;
                op.srcOffset = tempAlloc.offset;
                op.dstOffset = dstAlloc.offset;
                op.size = sizeof(AABB);
                mCopyOpsAABB.push_back(op);
            }
            mDirtyFlags[renderID] = 0;
        }
        mCurrentCopyOp->insert(mCurrentCopyOp->end(), mCopyOpsObject.begin(), mCopyOpsObject.end());
        mCurrentCopyOp->insert(mCurrentCopyOp->end(), mCopyOpsAABB.begin(), mCopyOpsAABB.end());
        // 不能在Reset的时候清除， 确保没有把历史脏数据清除掉。
        mPerFrameDirtyID.clear();
    }

    void GPUScene::SetCurrentContext()
    {
        int index = mCurrentFrameID % 3;
        mCurrentCopyOp = &mFrameCopyOp[index];
        mCurrentCopyOp->clear();
        mCurrVisibilityBuffer = visibilityBuffer[index];
        mCurrVisibilityBuffer->Reset();
    }

    void GPUScene::TryFreeRenderProxyByRenderIndex(uint32_t renderID)
    {
        if(mPerObjectDatas[renderID].renderProxyCount == 0) return;
        BufferAllocation allocation;
        allocation.offset = mPerObjectDatas[renderID].renderProxyStartIndex * sizeof(RenderProxy);
        allocation.size = mPerObjectDatas[renderID].renderProxyCount * sizeof(RenderProxy);
        allocation.isValid = true;
        renderProxyBuffer->Free(allocation);
    }

    void GPUScene::TryFreePerObjectDataAndAABBData(uint32_t renderID)
    {
        BufferAllocation allocation;
        allocation.offset = renderID * sizeof(PerObjectData);
        allocation.size = sizeof(PerObjectData);
        allocation.isValid = true;
        allObjectDataBuffer->Free(allocation);

        allocation.offset = renderID * sizeof(AABB);
        allocation.size = sizeof(AABB);
        allAABBBuffer->Free(allocation);
    }

}
