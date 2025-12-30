#include "PreCompiledHeader.h"
#include "GPUSceneManager.h"
#include "Graphics/GPUBufferAllocator.h"
#include "Graphics/IGPUBuffer.h"
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

        // 消化sceneRenderData中的dirty
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        vector<CopyOp> copyList;
        vector<PerObjectData> perObjectList;

        vector<DrawIndirectArgs> drawIndirectArgsList = BatchManager::GetInstance()->GetBatchInfo();
        
        UpdateRenderProxyBuffer(renderSceneData.materialDirtyList);
        UpdateAABBandPerObjectBuffer(renderSceneData.transformDirtyList);

        // 重置visibilityBuffer
        vector<uint8_t> empty;
        empty.resize(4 * 10000, 0);
        visibilityBuffer->UploadBuffer(visiblityAlloc, empty.data(), empty.size());
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
        if(PerObjectRenderInfoDataBuffer[index].renderProxyCount == 0) return;
        BufferAllocation allocation;
        allocation.offset = PerObjectRenderInfoDataBuffer[index].renderProxyStartIndex * sizeof(RenderProxy);
        allocation.size = PerObjectRenderInfoDataBuffer[index].renderProxyCount * sizeof(RenderProxy);
        renderProxyBuffer->Free(allocation);
    }

    void GPUSceneManager::TryCreateRenderProxyBlock(uint32_t index)
    {
        
        //BufferAllocation alloc = renderProxyBuffer->Allocate()
    }

    BufferAllocation GPUSceneManager::SyncDataToPerFrameBatchBuffer(void *data, uint32_t size)
    {
        auto& allocation = perFrameBatchBuffer->Allocate(size);
        perFrameBatchBuffer->UploadBuffer(allocation, data, allocation.size);
        return allocation;
    }

    void GPUSceneManager::UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList)
    {
        BatchManager* batchManager = BatchManager::GetInstance();
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        // 更新脏MeshRenderer的RenderProxy
        // todo:
        int count = materialDirtyList.size();
        uint32_t bufferSize = count * sizeof(PerObjectRenderInfoData);
        auto& allocation = perFrameBatchBuffer->Allocate(bufferSize);
        vector<PerObjectRenderInfoData> tempCPUUploadData;
        CopyOp* data = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        CopyOp* currPtr = data;
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
                PerObjectRenderInfoDataBuffer[index].renderProxyStartIndex = 0;
                PerObjectRenderInfoDataBuffer[index].renderProxyCount = 0;
                PerObjectRenderInfoDataBuffer[index].matIndex = 0;
            }
            else
            {
                // todo: 假设Proxy换了呢？ 就比如这个材质换成了另一个材质了？
                // 这个事情似乎可以交给BatchManager去做，这里就访问就ok
                vector<RenderProxy> proxyList = batchManager->GetAvaliableRenderProxyList(meshRenderer, vaoID);
                BufferAllocation allocation = renderProxyBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
                renderProxyBuffer->UploadBuffer(allocation, proxyList.data(), proxyList.size() * sizeof(RenderProxy));
                PerObjectRenderInfoDataBuffer[index].renderProxyStartIndex = static_cast<uint32_t>(allocation.offset / sizeof(RenderProxy));
                PerObjectRenderInfoDataBuffer[index].renderProxyCount = proxyList.size();
                PerObjectRenderInfoDataBuffer[index].matIndex = meshRenderer->GetMaterial()->materialAllocation.offset;

            }
            currPtr->srcOffset = allocation.offset + i * sizeof(PerObjectRenderInfoData);
            currPtr->dstOffset = index * sizeof(PerObjectRenderInfoData);
            currPtr->size = sizeof(PerObjectRenderInfoData);
            currPtr++;
            tempCPUUploadData.push_back(PerObjectRenderInfoDataBuffer[index]);
        }

        if (count == 0) return;
        perFrameBatchBuffer->UploadBuffer(allocation, tempCPUUploadData.data(), bufferSize);
        Payload_CopyBufferRegion copyRegionCmd;
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allObjectRenderInfoBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = data;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
    }

    void GPUSceneManager::UpdateAABBandPerObjectBuffer(const vector<uint32_t>& transformDirtyList)
    {
        // 获取需要更新的PerObjectData，传到GPU
        int count = transformDirtyList.size();
        
        uint32_t perObjectBufferSize = count * sizeof(PerObjectData);
        auto& perObjectAllocation = perFrameBatchBuffer->Allocate(perObjectBufferSize);
        CopyOp* copyPerObjectOP = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        CopyOp* currCopyPerObjectOPPtr = copyPerObjectOP;
        vector<PerObjectData> perObjectTempData;
        
        uint32_t aabbBufferSize = count * sizeof(AABB);
        auto& aabbAllocation = perFrameBatchBuffer->Allocate(aabbBufferSize);
        CopyOp* copyAABBOP = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        CopyOp* currCopyAABBPtr = copyAABBOP;
        vector<AABB> aabbTempData;

        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;

        for (int i = 0; i < count; i++)
        {
            int index = transformDirtyList[i];
            MeshRenderer* meshRenderer = renderSceneData.meshRendererList[index];
            if (meshRenderer == nullptr)
            {
                // 清空数据
                currCopyPerObjectOPPtr->srcOffset = perObjectAllocation.offset + i * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->dstOffset = index * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->size = sizeof(PerObjectData);
                currCopyPerObjectOPPtr++;
                perObjectTempData.push_back({});

                currCopyAABBPtr->srcOffset = aabbAllocation.offset + i * sizeof(AABB);
                currCopyAABBPtr->dstOffset = index * sizeof(AABB);
                currCopyAABBPtr->size = sizeof(AABB);
                currCopyAABBPtr++;
                aabbTempData.push_back({});
            }
            else
            {
                currCopyPerObjectOPPtr->srcOffset = perObjectAllocation.offset + i * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->dstOffset = index * sizeof(PerObjectData);
                currCopyPerObjectOPPtr->size = sizeof(PerObjectData);
                currCopyPerObjectOPPtr++;
                perObjectTempData.push_back(renderSceneData.objectToWorldMatrixList[index]);

                currCopyAABBPtr->srcOffset = aabbAllocation.offset + i * sizeof(AABB);
                currCopyAABBPtr->dstOffset = index * sizeof(AABB);
                currCopyAABBPtr->size = sizeof(AABB);
                currCopyAABBPtr++;
                aabbTempData.push_back(renderSceneData.aabbList[index]);
            }
        }

        if (count == 0) return;
        perFrameBatchBuffer->UploadBuffer(perObjectAllocation, perObjectTempData.data(), perObjectBufferSize);
        Payload_CopyBufferRegion copyRegionCmd;
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allObjectDataBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = copyPerObjectOP;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);

        perFrameBatchBuffer->UploadBuffer(aabbAllocation, aabbTempData.data(), aabbBufferSize);
        copyRegionCmd = {};
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allAABBBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = copyAABBOP;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
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

        PerObjectRenderInfoDataBuffer.resize(10000);

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

        desc.debugName = L"AllObjectRenderInfoBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(PerObjectRenderInfoData);
        desc.stride = 10000 * sizeof(PerObjectRenderInfoData);
        desc.usage = BufferUsage::StructuredBuffer;
        allObjectRenderInfoBuffer = new GPUBufferAllocator(desc);
        allObjectRenderInfoBufferAllocation = allObjectRenderInfoBuffer->Allocate(sizeof(PerObjectRenderInfoData) * 10000);

        desc.debugName = L"allAABBBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 10000 * sizeof(AABB);
        desc.stride = 10000 * sizeof(AABB);
        desc.usage = BufferUsage::StructuredBuffer;
        allAABBBuffer = new GPUBufferAllocator(desc);

    }


}
