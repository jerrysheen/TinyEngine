#include "PreCompiledHeader.h"
#include "GPUSceneManager.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUResource.h"
#include "Renderer/RenderUniforms.h"
#include "Renderer/RenderAPI.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer.h"
#include "Renderer/BatchManager.h"

namespace EngineCore
{
    GPUSceneManager* GPUSceneManager::sInstance = nullptr;
    
    GPUSceneManager* GPUSceneManager::GetInstance()
    {
        if(sInstance != nullptr) return sInstance;
        GPUSceneManager::Create();
        return sInstance;   
    }

    void GPUSceneManager::Create()
    {
        if(sInstance != nullptr) return;
        sInstance = new GPUSceneManager();
    }

    void GPUSceneManager::Destroy()
    {
        delete allObjectDataBuffer;
        delete allMaterialDataBuffer;
        delete perFrameBatchBuffer;
        delete perFramelinearMemoryAllocator;
        delete visibilityBuffer;
    }

    void GPUSceneManager::Tick()
    {
        perFrameBatchBuffer->Reset();
        perFramelinearMemoryAllocator->Reset();
        mPendingBatchCopies.clear();
        // 消化sceneRenderData中的dirty
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        vector<CopyOp> copyList;


        vector<DrawIndirectArgs> drawIndirectArgsList = BatchManager::GetInstance()->GetBatchInfo();
        UpdateRenderProxyBuffer(renderSceneData.materialDirtyList);
        UpdateAABBandPerObjectBuffer(renderSceneData.transformDirtyList, renderSceneData.materialDirtyList);

        // 重置visibilityBuffer
        vector<uint8_t> empty;
        empty.resize(4 * 10000, 0);
        visibilityBuffer->UploadBuffer(visiblityAlloc, empty.data(), empty.size());
        visibilityBuffer->Reset();
    }

    BufferAllocation GPUSceneManager::GetSinglePerMaterialData()
    {
        return allMaterialDataBuffer->Allocate(512);
    }

    void GPUSceneManager::RemoveSinglePerMaterialData(const BufferAllocation &bufferalloc)
    {
        allMaterialDataBuffer->Free(bufferalloc);
    }

    void GPUSceneManager::UpdateSinglePerMaterialData(const BufferAllocation &bufferalloc, void *data)
    {
        allMaterialDataBuffer->UploadBuffer(bufferalloc, data, 512);
    }

    void GPUSceneManager::TryFreeRenderProxyBlock(uint32_t index)
    {
        if(perObjectDataBuffer[index].renderProxyCount == 0) return;
        BufferAllocation allocation;
        allocation.offset = perObjectDataBuffer[index].renderProxyStartIndex * sizeof(RenderProxy);
        allocation.size = perObjectDataBuffer[index].renderProxyCount * sizeof(RenderProxy);
        renderProxyBuffer->Free(allocation);
    }

    void GPUSceneManager::TryCreateRenderProxyBlock(uint32_t index)
    {
        
        //BufferAllocation alloc = renderProxyBuffer->Allocate()
    }

    BufferAllocation GPUSceneManager::LagacyRenderPathUploadBatch(void *data, uint32_t size)
    {
        auto& destAllocation = visibilityBuffer->Allocate(size);
        auto& srcAllocation = perFrameBatchBuffer->Allocate(size);
        perFrameBatchBuffer->UploadBuffer(srcAllocation, data, size);

        CopyOp op = {};
        op.srcOffset = srcAllocation.offset;
        op.dstOffset = destAllocation.offset;
        op.size = size;
        mPendingBatchCopies.push_back(op);

        return destAllocation;        
    }

    void GPUSceneManager::FlushBatchUploads()
    {
        Payload_CopyBufferRegion copyRegionCmd = {};
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = visibilityBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = mPendingBatchCopies.data();
        copyRegionCmd.count = mPendingBatchCopies.size();
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
        mPendingBatchCopies.clear();
    }

    void GPUSceneManager::UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList)
    {
        BatchManager* batchManager = BatchManager::GetInstance();
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        // 更新脏的PerObjectDataList
        // todo:
        for (int i = 0; i < materialDirtyList.size(); i++)
        {
            uint32_t index = materialDirtyList[i];
            uint32_t vaoID = renderSceneData.vaoIDList[i];
            MeshRenderer* meshRenderer = renderSceneData.meshRendererList[index];
            if (vaoID == UINT32_MAX) return;
            if (meshRenderer == nullptr)
            {
                // delete: 只处理RenderProxy相关信息。
                TryFreeRenderProxyBlock(index);
                perObjectDataBuffer[index].renderProxyStartIndex = 0;
                perObjectDataBuffer[index].renderProxyCount = 0;
                perObjectDataBuffer[index].matIndex = 0;
            }
            else
            {
                // todo: 假设Proxy换了呢？ 就比如这个材质换成了另一个材质了？
                // 这个事情似乎可以交给BatchManager去做，这里就访问就ok
                vector<RenderProxy> proxyList = batchManager->GetAvaliableRenderProxyList(meshRenderer, vaoID);
                BufferAllocation allocation = renderProxyBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
                renderProxyBuffer->UploadBuffer(allocation, proxyList.data(), proxyList.size() * sizeof(RenderProxy));
                perObjectDataBuffer[index].renderProxyStartIndex = static_cast<uint32_t>(allocation.offset / sizeof(RenderProxy));
                perObjectDataBuffer[index].renderProxyCount = proxyList.size();
                perObjectDataBuffer[index].matIndex = meshRenderer->GetMaterial()->materialAllocation.offset;

            }
        }
    }

    void GPUSceneManager::UpdateAABBandPerObjectBuffer(const vector<uint32_t>& transformDirtyList, const vector<uint32_t>& materialDirtyList)
    {
        // 更新AABB
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;

        int transformDirtyCount = transformDirtyList.size();
        uint32_t aabbBufferSize = transformDirtyCount * sizeof(AABB);

        auto& aabbAllocation = perFrameBatchBuffer->Allocate(aabbBufferSize);
        CopyOp* copyAABBOP = perFramelinearMemoryAllocator->allocArray<CopyOp>(transformDirtyCount);
        CopyOp* currCopyAABBPtr = copyAABBOP;
        vector<AABB> aabbTempData;

        for (int i = 0; i < transformDirtyCount; i++)
        {
            int index = transformDirtyList[i];
            MeshRenderer* meshRenderer = renderSceneData.meshRendererList[index];
            if (meshRenderer == nullptr)
            {
                currCopyAABBPtr->srcOffset = aabbAllocation.offset + i * sizeof(AABB);
                currCopyAABBPtr->dstOffset = index * sizeof(AABB);
                currCopyAABBPtr->size = sizeof(AABB);
                currCopyAABBPtr++;
                aabbTempData.push_back({});
            }
            else
            {
                currCopyAABBPtr->srcOffset = aabbAllocation.offset + i * sizeof(AABB);
                currCopyAABBPtr->dstOffset = index * sizeof(AABB);
                currCopyAABBPtr->size = sizeof(AABB);
                currCopyAABBPtr++;
                aabbTempData.push_back(renderSceneData.aabbList[index]);
            }
        }
        if (transformDirtyCount != 0) 
        {
            perFrameBatchBuffer->UploadBuffer(aabbAllocation, aabbTempData.data(), aabbBufferSize);
            Payload_CopyBufferRegion copyRegionCmd = {};
            copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
            copyRegionCmd.destDefaultBuffer = allAABBBuffer->GetGPUBuffer();
            copyRegionCmd.copyList = copyAABBOP;
            copyRegionCmd.count = transformDirtyCount;
            Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
        }


        // 更新所有dirty的PerObjectData，来自于 tranformdirty + materialDirty
        std::set<uint32_t> s(transformDirtyList.begin(), transformDirtyList.end());
        s.insert(materialDirtyList.begin(), materialDirtyList.end());
        vector<uint32_t> dirtyList = std::vector<uint32_t>(s.begin(), s.end());
        int perObjectDataCount = dirtyList.size();

        uint32_t perObjectBufferSize = perObjectDataCount * sizeof(PerObjectData);
        auto& perObjectAllocation = perFrameBatchBuffer->Allocate(perObjectBufferSize);
        CopyOp* copyPerObjectOP = perFramelinearMemoryAllocator->allocArray<CopyOp>(perObjectDataCount);
        CopyOp* currCopyPerObjectOPPtr = copyPerObjectOP;
        vector<PerObjectData> perObjectTempData;

        for (int i = 0; i < perObjectDataCount; i++)
        {
            int index = dirtyList[i];
            MeshRenderer* meshRenderer = renderSceneData.meshRendererList[index];
            if (meshRenderer == nullptr)
            {
                // 清空数据
                currCopyPerObjectOPPtr->srcOffset = perObjectAllocation.offset + i * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->dstOffset = index * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->size = sizeof(PerObjectData);
                currCopyPerObjectOPPtr++;
                perObjectDataBuffer[i] = {};
                perObjectTempData.push_back({});
            }
            else
            {
                currCopyPerObjectOPPtr->srcOffset = perObjectAllocation.offset + i * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->dstOffset = index * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->size = sizeof(PerObjectData);
                currCopyPerObjectOPPtr++;
                perObjectDataBuffer[i].objectToWorld = renderSceneData.objectToWorldMatrixList[i];
                perObjectTempData.push_back(perObjectDataBuffer[i]);
            }
        }

        if (perObjectDataCount != 0) 
        {
            perFrameBatchBuffer->UploadBuffer(perObjectAllocation, perObjectTempData.data(), perObjectBufferSize);
            Payload_CopyBufferRegion copyRegionCmd;
            copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
            copyRegionCmd.destDefaultBuffer = allObjectDataBuffer->GetGPUBuffer();
            copyRegionCmd.copyList = copyPerObjectOP;
            copyRegionCmd.count = perObjectDataCount;
            Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
        }
    }

    GPUSceneManager::GPUSceneManager()
    {
        BufferDesc desc;
        desc.debugName = L"AllObjectBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = sizeof(PerObjectData) * 10000;
        desc.stride = sizeof(PerObjectData);
        desc.usage = BufferUsage::StructuredBuffer;
        allObjectDataBuffer = new GPUBufferAllocator(desc);

        // 强烈建议使用 512， 注意，这边都是Byte
        // 理由：
        // 1. 容纳 32 个 float4 向量，足够应对 Uber Shader + 矩阵 + 几十个 TextureID。
        // 2. 即使浪费了一半空间，1万个材质也只多占 2.5MB 显存，完全可忽略。
        // 3. 512 是 256 (D3D12 ConstantBuffer 对齐) 的倍数，也是 16 (float4) 的倍数，对齐非常友好。

        desc.debugName = L"AllMaterialBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 512 * 10000; // 预分配约 5MB
        desc.stride = 512;       // PageSize
        desc.usage = BufferUsage::ByteAddressBuffer;
        allMaterialDataBuffer = new GPUBufferAllocator(desc);

        desc.debugName = L"PerFrameBatchBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = 1024 * 1024 * 4;
        desc.stride = 1;
        desc.usage = BufferUsage::ByteAddressBuffer;
        perFrameBatchBuffer = new GPUBufferAllocator(desc);

        perObjectDataBuffer.resize(10000);

        perFramelinearMemoryAllocator = new LinearAllocator(4 * 1024 * 1024);
    
        // 创建Compute Shader
        string path = PathSettings::ResolveAssetPath("Shader/GPUCulling.hlsl");
        GPUCullingShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);
    
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
        visiblityAlloc = visibilityBuffer->Allocate(4 * 10000);


        desc.debugName = L"allAABBBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(AABB);
        desc.stride = 10000 * sizeof(AABB);
        desc.usage = BufferUsage::StructuredBuffer;
        allAABBBuffer = new GPUBufferAllocator(desc);

    }


}
