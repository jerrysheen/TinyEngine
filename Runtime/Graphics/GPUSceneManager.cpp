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
        delete allInstanceDataBuffer;
        delete perFramelinearMemoryAllocator;
    }

    void GPUSceneManager::Tick()
    {
        perFrameBatchBuffer->Reset();
        perFramelinearMemoryAllocator->Reset();
        dirtyPerObjectDataIndexList.clear();
        // 消化sceneRenderData中的dirty
        Scene* mCurrentScene = SceneManager::GetInstance()->GetCurrentScene();
        auto& renderSceneData = mCurrentScene->renderSceneData;
        vector<CopyOp> copyList;
        vector<PerObjectData> perObjectList;

        vector<DrawIndirectArgs> drawIndirectArgsList = BatchManager::GetInstance()->GetBatchInfo();
        
        BatchManager* batchManager = BatchManager::GetInstance();
        // 更新脏MeshRenderer的RenderProxy
        // todo:
        for(int i = 0; i < renderSceneData.materialDirtyList.size(); i++)
        {   
            uint32_t index = renderSceneData.materialDirtyList[i];
            uint32_t vaoID = renderSceneData.vaoIDList[i];
            if(vaoID == UINT32_MAX) return;
            if (renderSceneData.meshRendererList[index] == nullptr)
            {
                // delete: 只处理RenderProxy相关信息。
                PerObjectData& data = perObjectCPUBuffer[index];
                data.matIndex = 0;
                TryFreeRenderProxyBlock(data);
                data.renderProxyCount = 0;
                data.renderProxyStartIndex = 0;
            }
            else
            {
                // todo: 假设Proxy换了呢？ 就比如这个材质换成了另一个材质了？
                // 这个事情似乎可以交给BatchManager去做，这里就访问就ok
                PerObjectData& data = perObjectCPUBuffer[index];
                data.matIndex = renderSceneData.meshRendererList[index]->GetMaterial()->materialAllocation.offset;
                vector<RenderProxy> proxyList = batchManager->GetAvaliableRenderProxyList(renderSceneData.meshRendererList[index], renderSceneData.vaoIDList[index]);
                BufferAllocation allocation = renderProxyBuffer->Allocate(proxyList.size() * sizeof(RenderProxy));
                renderProxyBuffer->UploadBuffer(allocation, proxyList.data(), proxyList.size() * sizeof(RenderProxy));
                data.renderProxyStartIndex = allocation.offset;
                data.renderProxyCount = proxyList.size();
            }
            dirtyPerObjectDataIndexList.push_back(index);
        }

        for(int i = 0; i < renderSceneData.transformDirtyList.size(); i++)
        {   
            uint32_t index = renderSceneData.transformDirtyList[i];
            uint32_t vaoID = renderSceneData.vaoIDList[i];
            if(vaoID == UINT32_MAX) return;
            bool find = false;
            for(auto indexInList : dirtyPerObjectDataIndexList)
            {
                if(indexInList == index)
                {
                    find = true;
                }
            }
            if(find == false) dirtyPerObjectDataIndexList.push_back(index);
        }

        // 获取需要更新的PerObjectData，传到GPU
        int count = dirtyPerObjectDataIndexList.size();
        uint32_t PerObjectbufferSize = count * sizeof(PerObjectData);
        auto& PerObjectAllocation = perFrameBatchBuffer->Allocate(PerObjectbufferSize);
        vector<PerObjectData> tempCPUUploadData;
        CopyOp* data = perFramelinearMemoryAllocator->allocArray<CopyOp>(count);
        CopyOp* currPtr = data;
        for(int i = 0; i < dirtyPerObjectDataIndexList.size(); i++)
        {
            int index = dirtyPerObjectDataIndexList[i];
            if(renderSceneData.meshRendererList[i] == nullptr)
            {
                // delete:
                PerObjectData& data = perObjectCPUBuffer[index];
                data.bounds = AABB{};
                data.objectToWorld = Matrix4x4::Identity;
            }
            else
            {
                PerObjectData& data = perObjectCPUBuffer[index];
                data.bounds = renderSceneData.aabbList[index];
                data.objectToWorld = renderSceneData.objectToWorldMatrixList[index];
            }

            uint32_t nodeIndex = index;
            currPtr->srcOffset = PerObjectAllocation.offset + i * sizeof(PerObjectData);
            currPtr->dstOffset = nodeIndex * sizeof(PerObjectData);
            currPtr->size = sizeof(PerObjectData);
            currPtr++;
            tempCPUUploadData.push_back(perObjectCPUBuffer[index]);
        }

        if (count == 0) return;
        perFrameBatchBuffer->UploadBuffer(PerObjectAllocation, tempCPUUploadData.data(), PerObjectbufferSize);
        Payload_CopyBufferRegion copyRegionCmd;
        copyRegionCmd.srcUploadBuffer = perFrameBatchBuffer->GetGPUBuffer();
        copyRegionCmd.destDefaultBuffer = allObjectDataBuffer->GetGPUBuffer();
        copyRegionCmd.copyList = data;
        copyRegionCmd.count = count;
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
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

    void GPUSceneManager::TryFreeRenderProxyBlock(const PerObjectData &perObjectData)
    {
        if(perObjectData.renderProxyCount == 0) return;
        BufferAllocation allocation;
        allocation.offset = perObjectData.renderProxyStartIndex * sizeof(RenderProxy);
        allocation.size = perObjectData.renderProxyCount * sizeof(RenderProxy);
        renderProxyBuffer->Free(allocation);
    }

    void GPUSceneManager::TryCreateRenderProxyBlock(const PerObjectData &perObjectData)
    {
        
        //BufferAllocation alloc = renderProxyBuffer->Allocate()
    }

    BufferAllocation GPUSceneManager::SyncDataToPerFrameBatchBuffer(void *data, uint32_t size)
    {
        auto& allocation = perFrameBatchBuffer->Allocate(size);
        perFrameBatchBuffer->UploadBuffer(allocation, data, allocation.size);
        return allocation;
    }

    //PerObjectCPUHandler GPUSceneManager::ResgisterNewObject()
    //{
    //    PerObjectCPUHandler handle;
    //    if(!m_FreePerObjectIndex.empty())
    //    {
    //        handle.perObejectIndex = m_FreePerObjectIndex.front();
    //        m_FreePerObjectIndex.pop();
    //        return handle;
    //    }
    //    handle.perObejectIndex = m_CurrentPerObjectIndex;
    //    m_CurrentPerObjectIndex++;        
    //    return handle;
    //}

    //void GPUSceneManager::DeleteSceneObject(PerObjectCPUHandler &handler)
    //{
    //    ASSERT(handler.isValid());
    //    PerObjectCPUData& data = perObjectCPUBuffer[handler.perObejectIndex];
    //    data.active = false;
    //    m_FreePerObjectIndex.push(handler.perObejectIndex);
    //    handler.perObejectIndex = UINT32_MAX;
    //}

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

        perObjectCPUBuffer.resize(10000);
        std::queue<int> empty;
        std::swap(m_FreePerObjectIndex, empty);

        desc.debugName = L"InstanceDataBuffer";
        desc.memoryType = BufferMemoryType::Upload;
        desc.size = 4 * 10000; 
        desc.stride = 4;       
        desc.usage = BufferUsage::ByteAddressBuffer;
        allInstanceDataBuffer = new GPUBufferAllocator(desc);

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

        cpuPerObjectDataList.resize(10000);
    }


}
