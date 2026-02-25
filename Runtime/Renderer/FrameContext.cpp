#include "PreCompiledHeader.h"
#include "FrameContext.h"
#include "Scene/SceneStruct.h"
#include "Renderer/BatchManager.h"
#include "Renderer/Renderer.h"


namespace EngineCore
{
    FrameContext::FrameContext()
    {
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
        visibilityBuffer = new GPUBufferAllocator(desc);
       
       
        desc.debugName = L"allAABBBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(AABB);
        desc.stride = 10000 * sizeof(AABB);
        desc.usage = BufferUsage::StructuredBuffer;
        allAABBBuffer = new GPUBufferAllocator(desc);
       
 
        desc.debugName = L"PerFrameUploadBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = 1024 * 1024 * 4;
        desc.stride = 1;
        desc.usage = BufferUsage::ByteAddressBuffer;
        perFrameUploadBuffer = new GPUBufferAllocator(desc);
    }

    void FrameContext::EnsureCapacity(uint32_t renderID)
    {
        int count = mDirtyFlags.size();
        int need = renderID + 1;
        if(count < need)
        {
            mDirtyFlags.resize(need, 0);
            mPerObjectDatas.resize(need);
        }
    }

    BufferAllocation FrameContext::UploadDrawBatch(void *data, uint32_t size)
    {
        ASSERT(data != nullptr);
        ASSERT(size > 0);

        BufferAllocation destAllocation = visibilityBuffer->Allocate(size);
        BufferAllocation srcAllocation = perFrameUploadBuffer->Allocate(size);
        perFrameUploadBuffer->UploadBuffer(srcAllocation, data, size);

        CopyOp op = {};
        op.srcOffset = srcAllocation.offset;
        op.dstOffset = destAllocation.offset;
        op.size = size;
        mCopyOpsVisibility.push_back(op);

        return destAllocation;
    }

    void FrameContext::UpdateDirtyFlags(uint32_t renderID, uint32_t flags)
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

    void FrameContext::UploadCopyOp()
    {
        if (mCopyOpsObject.size() > 0) 
        {
            Payload_CopyBufferRegion copyObject;
            copyObject.srcUploadBuffer = perFrameUploadBuffer->GetGPUBuffer();
            copyObject.destDefaultBuffer = allObjectDataBuffer->GetGPUBuffer();
            copyObject.copyList = mCopyOpsObject.data();
            copyObject.count = mCopyOpsObject.size();
            Renderer::GetInstance()->CopyBufferRegion(copyObject);
        }

        if (mCopyOpsAABB.size() > 0)
        {
            Payload_CopyBufferRegion copyAABB;
            copyAABB.srcUploadBuffer = perFrameUploadBuffer->GetGPUBuffer();
            copyAABB.destDefaultBuffer = allAABBBuffer->GetGPUBuffer();
            copyAABB.copyList = mCopyOpsAABB.data();
            copyAABB.count = mCopyOpsAABB.size();
            Renderer::GetInstance()->CopyBufferRegion(copyAABB);
        }

        if (mCopyOpsProxy.size() > 0)
        {
            Payload_CopyBufferRegion copyRenderProxy;
            copyRenderProxy.srcUploadBuffer = perFrameUploadBuffer->GetGPUBuffer();
            copyRenderProxy.destDefaultBuffer = renderProxyBuffer->GetGPUBuffer();
            copyRenderProxy.copyList = mCopyOpsProxy.data();
            copyRenderProxy.count = mCopyOpsProxy.size();
            Renderer::GetInstance()->CopyBufferRegion(copyRenderProxy);
        }

        if (mCopyOpsVisibility.size() > 0)
        {
            Payload_CopyBufferRegion copyVisibility;
            copyVisibility.srcUploadBuffer = perFrameUploadBuffer->GetGPUBuffer();
            copyVisibility.destDefaultBuffer = visibilityBuffer->GetGPUBuffer();
            copyVisibility.copyList = mCopyOpsVisibility.data();
            copyVisibility.count = mCopyOpsVisibility.size();
            Renderer::GetInstance()->CopyBufferRegion(copyVisibility);
        }

    }

    void FrameContext::UpdateShadowData(uint32_t renderID, CPUSceneView &view)
    {
        uint32_t flags = mDirtyFlags[renderID];
        if(flags & (uint32_t)NodeDirtyFlags::None) return;
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
            BufferAllocation allocation = renderProxyBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
            mPerObjectDatas[renderID].renderProxyCount = proxyList.size();
            mPerObjectDatas[renderID].renderProxyStartIndex = static_cast<uint32_t>(allocation.offset / sizeof(RenderProxy));

            // copyop:
            BufferAllocation tempAllocation = perFrameUploadBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
            perFrameUploadBuffer->UploadBuffer(tempAllocation, proxyList.data(), proxyList.size() * sizeof(RenderProxy));
            CopyOp op;
            op.srcOffset = tempAllocation.offset;
            op.dstOffset = allocation.offset;
            op.size = proxyList.size() * sizeof(RenderProxy);
            mCopyOpsProxy.push_back(std::move(op));
        }
    }

    void FrameContext::UpdatePerFrameDirtyNode(CPUSceneView& cpuScene)
    {
        for(int i = 0; i < mPerFrameDirtyID.size(); i++)
        {
            int renderID = mPerFrameDirtyID[i];
            uint32_t dirtyFlags = mDirtyFlags[renderID];
            // PerFrameObject基本都要更新
            if(!(dirtyFlags & (uint32_t)NodeDirtyFlags::None))
            {
                BufferAllocation allocation = allObjectDataBuffer->Allocate(sizeof(PerObjectData));
                BufferAllocation tempAllocation = perFrameUploadBuffer->Allocate(sizeof(PerObjectData));
                perFrameUploadBuffer->UploadBuffer(tempAllocation, &mPerObjectDatas[renderID], sizeof(PerObjectData));
                CopyOp op;
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                op.size = sizeof(PerObjectData);
                mCopyOpsObject.push_back(std::move(op));
                mDirtyFlags[renderID] = 0;
            }

            if((dirtyFlags & (uint32_t)NodeDirtyFlags::Created) || (dirtyFlags & (uint32_t)NodeDirtyFlags::TransformDirty) || (dirtyFlags & (uint32_t)NodeDirtyFlags::MeshDirty))
            {
                BufferAllocation allocation = allAABBBuffer->Allocate(sizeof(AABB));
                BufferAllocation tempAllocation = perFrameUploadBuffer->Allocate(sizeof(AABB));
                perFrameUploadBuffer->UploadBuffer(tempAllocation, (void*)&cpuScene.worldBoundsList[renderID], sizeof(AABB));
                CopyOp op;
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                op.size = sizeof(AABB);
                mCopyOpsAABB.push_back(std::move(op));
                mDirtyFlags[renderID] = 0;
            }
        }
    }

    FrameContext::~FrameContext()
    {
        delete allObjectDataBuffer;
        delete allAABBBuffer;
        delete renderProxyBuffer;
        delete perFrameUploadBuffer;
        delete visibilityBuffer;
    }

    void FrameContext::Reset()
    {

        visibilityBuffer->Reset();
        vector<uint8_t> empty;
        empty.resize(4 * 10000, 0);
        BufferAllocation allocation = visibilityBuffer->Allocate(4 * 10000);
        visibilityBuffer->UploadBuffer(allocation, empty.data(), empty.size());
        // 初始化之后再清除
        visibilityBuffer->Reset();
        
        perFrameUploadBuffer->Reset();
        mPerFrameDirtyID.clear();


        mCopyOpsObject.clear();
        mCopyOpsAABB.clear();
        mCopyOpsProxy.clear();
        mCopyOpsVisibility.clear();
    }

    void FrameContext::TryFreeRenderProxyByRenderIndex(uint32_t renderID)
    {
        if(mPerObjectDatas[renderID].renderProxyCount == 0) return;
        BufferAllocation allocation;
        allocation.offset = mPerObjectDatas[renderID].renderProxyStartIndex * sizeof(RenderProxy);
        allocation.size = mPerObjectDatas[renderID].renderProxyCount * sizeof(RenderProxy);
        allocation.isValid = true;
        renderProxyBuffer->Free(allocation);
    }

    void EngineCore::FrameContext::TryFreePerObjectDataAndAABBData(uint32_t renderID)
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
