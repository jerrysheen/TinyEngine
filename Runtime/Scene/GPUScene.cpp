#include "PreCompiledHeader.h"
#include "GPUScene.h"
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
    void GPUScene::Destroy()
    {
        delete allMaterialDataBuffer;
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

        desc.debugName = L"AllMaterialBuffer";
        desc.memoryType = BufferMemoryType::Default;
        desc.size = 512 * 10000; // 预分配约 5MB
        desc.stride = 512;       // PageSize
        desc.usage = BufferUsage::ByteAddressBuffer;
        allMaterialDataBuffer = new GPUBufferAllocator(desc);


        // 创建Compute Shader
        string path = PathSettings::ResolveAssetPath("Shader/GPUCulling.hlsl");
        GPUCullingShaderHandler = ResourceManager::GetInstance()->CreateResource<ComputeShader>(path);

        mCurrentFrameContext = &mCPUFrameContext[0];
    }

    void GPUScene::Update(uint32_t currentFrameIndex)
    {
        mCurrentFrameID = currentFrameIndex;
        mCurrentFrameContext = GetCurrentFrameContexts();
        mCurrentFrameContext->Reset();
    }


    BufferAllocation GPUScene::GetSinglePerMaterialData()
    {
        return allMaterialDataBuffer->Allocate(512);
    }


    BufferAllocation GPUScene::UploadDrawBatch(void *data, uint32_t size)
    {
        return mCurrentFrameContext->UploadDrawBatch(data, size);

    }

    void GPUScene::UpdateDirtyNode(CPUSceneView& view)
    {
        mCurrentFrameContext->UpdatePerFrameDirtyNode(view);
    }

    void GPUScene::UploadCopyOp()
    {
        mCurrentFrameContext->UploadCopyOp();
    }

    // 立即更新RenderProxy，free old， Create new
    // PerObjectData、 AABB buffer只同步， 到Apply结束后做Copy
    // 更新顺序是什么？ 
    // 1. 同步当前flag到 3 FrameContext中， 与之前的合并，
    // 3. 根据flag 更新 material proxy ，加入copy队列
    // 4. 根据flag 更新 perObjectData， 加入copy队列
    // 5. 根据flag 更新 aabbData， 加入copy队列
    // 6. 这一帧的FrameContext设置为None
    void GPUScene::ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view)
    {   
        EnsureCapacity(renderID); 
        UpdateFrameContextDirtyFlags(renderID, flags);
        // 记录我当前脏ID即可
        UpdateFrameContextShadowData(renderID, view);
    }


    void GPUScene::UpdateFrameContextDirtyFlags(uint32_t renderID, uint32_t flag)
    {
        for(int i = 0; i < mMaxFrameCount; i++)
        {
            mCPUFrameContext[i].UpdateDirtyFlags(renderID, flag);
        }
    }

    void GPUScene::UpdateFrameContextShadowData(uint32_t renderID, CPUSceneView &view)
    {
        for(int i = 0; i < mMaxFrameCount; i++)
        {
            mCPUFrameContext[i].UpdateShadowData(renderID, view);
        }
    }

    FrameContext *GPUScene::GetCurrentFrameContexts()
    {
        int index = mCurrentFrameID % mMaxFrameCount;
        ASSERT(index >= 0 && index < mMaxFrameCount);
        return &mCPUFrameContext[index];
        // TODO: insert return statement here
    }

    void GPUScene::EnsureCapacity(uint32_t renderID)
    {
        for(int i = 0; i < 3; i++)
        {
            mCPUFrameContext[i].EnsureCapacity(renderID);
        }
    }

    GPUScene::GPUScene()
    {

    }


}
