# Architecture Digest: FRAME_FLOW
> Auto-generated. Focus: Runtime/Core/Game.cpp, Runtime/Renderer/RenderEngine.cpp, Runtime/Scene/GPUScene.cpp, Runtime/Renderer/FrameContext.h, Runtime/Renderer/FrameContext.cpp, Runtime/Renderer/RenderPath/LegacyRenderPath.h, Runtime/Renderer/RenderPath/LegacyRenderPath.cpp, Runtime/Renderer/RenderPath/GPUSceneRenderPath.h, Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp, Runtime/Renderer/Renderer.cpp, Game::Update, Game::Render, RenderEngine::Update, RenderEngine::Tick, GPUScene::Update, ApplyDirtyNode, UpdateDirtyNode, UploadCopyOp, FrameContext, CopyOp, perFrameUploadBuffer, PersistentGPUScene, SceneDelta, DirtyFlags, RenderBuild, RenderSubmit, BuildVsSubmit, MainThreadFrontend, RenderThreadBackend, GPUCulling, DrawIndirect, LegacyPath, GPUScenePath, frames-in-flight, fence, deferred free

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作，并建立解耦的帧更新流（GameObject/Component、Scene、CPUScene/GPUScene、FrameContext多帧同步）。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。
- 针对更新链路重点追踪：Game::Update/Render/EndFrame -> SceneManager/Scene -> CPUScene -> GPUScene -> FrameContext。
- 重点识别NodeDirtyFlags、NodeDirtyPayload、PerFrameDirtyList、CopyOp等脏数据传播与跨帧同步结构。

## Understanding Notes
- 明确主循环阶段边界：SimulationUpdate（游戏逻辑）/ RenderBuild（渲染前端构建）/ RenderSubmit（渲染后端提交）。
- 重点识别当前混用点：Game::Update/Render、RenderEngine::Update/Tick、GPUScene::Update/ApplyDirtyNode/UpdateDirtyNode 的职责交叉。
- 强调FrameContext应仅承载每帧临时数据（Upload/CopyOp/临时可见性），持久GPU场景数据应独立管理。
- 区分RenderPath差异但统一阶段模型：Legacy与GPUScene路径都应Build与Submit分离，仅构建产物不同（DrawList vs Copy+Culling+Indirect）。
- 关注同步策略现代化：避免每帧WaitForGpuFinished，转向frames-in-flight与fence驱动的延迟回收。

## Key Files Index
- `[33]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[28]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[26]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[24]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[13]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[13]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[10]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[9]` **Runtime/Renderer/RenderBackend.h** *(Content Included)*
- `[9]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[8]` **Runtime/Scene/Scene.cpp** *(Content Included)*
- `[7]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[7]` **Runtime/Renderer/FrameTicket.h** *(Content Included)*
- `[7]` **Runtime/Renderer/RenderBackend.cpp** *(Content Included)*
- `[7]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[6]` **Runtime/Scene/CPUScene.cpp** *(Content Included)*
- `[6]` **Runtime/Scene/Scene.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/D3D12Struct.h** *(Content Included)*
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h** *(Content Included)*
- `[5]` **Runtime/Renderer/BatchManager.cpp**
- `[5]` **Runtime/Renderer/RenderAPI.h**
- `[5]` **Runtime/Scene/SceneManager.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[5]` **Editor/D3D12/D3D12EditorGUIManager.cpp**
- `[4]` **Runtime/Renderer/RenderEngine.h**
- `[4]` **Runtime/Renderer/RenderStruct.h**
- `[4]` **Runtime/Scene/CPUScene.h**
- `[4]` **Runtime/Scene/SceneStruct.h**
- `[4]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/BlitShader.hlsl**
- `[4]` **Assets/Shader/IndirectDrawCallCombineComputeShader.hlsl**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/PublicStruct.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.h**
- `[2]` **Runtime/GameObject/Component.h**
- `[2]` **Runtime/GameObject/ComponentType.h**
- `[2]` **Runtime/GameObject/GameObject.h**
- `[2]` **Runtime/GameObject/MeshFilter.h**
- `[2]` **Runtime/GameObject/MeshRenderer.h**
- `[2]` **Runtime/GameObject/MonoBehaviour.h**
- `[2]` **Runtime/GameObject/Transform.h**
- `[2]` **Runtime/Graphics/ComputeShader.h**
- `[2]` **Runtime/Graphics/GeometryManager.h**

## Evidence & Implementation Details

### File: `Runtime/Renderer/RenderEngine.cpp`
```cpp
        mUploadPagePool->Recycle(*mCurrFrameTicket);
        RenderBackend::GetInstance()->RecycleStagedBuffer(mCurrFrameTicket);
        PROFILER_ZONE("RenderEngine::Update");
        mCPUScene.Update(frameID);
        mGPUScene.Update(frameID);
        ComsumeDirtySceneRenderNode(sceneDelta);
        //PROFILER_EVENT_BEGIN("MainThread::RenderEngine::Update");

        RenderBackend::GetInstance()->BeginFrame();
        // todo:  Remove
        RenderBackend::GetInstance()->FlushPerFrameData();
        uint32_t frameIndex = RenderEngine::GetInstance()->GetCurrentFrame();
        FrameTicket* currentFrameTicket = RenderEngine::GetInstance()->GetCurrentFrameTicket(frameIndex);
        RenderBackend::GetInstance()->SetFrame(currentFrameTicket, frameIndex);


        mCurrentRenderPipeline->Prepare(renderContext);
        mGPUScene.UploadCopyOp();
        //PROFILER_EVENT_END("MainThread::RenderEngine::Update");
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
```
...
```cpp
    void RenderEngine::WaitForFrameAvaliable(uint32_t frameID)
    {
        PROFILER_ZONE("MainThread::WaitForFrameContextAvaliable()");
        const FrameTicket* currentTicket = GetCurrentFrameTicket(frameID);
        // 当开始复用 ring 槽位时，必须确保“上一任帧”的提交信息已经由渲染线程发布。
        // 仅等 fence 值不够，因为渲染线程可能还未写入该槽位的最新 fence。
        if (frameID >= MAX_FRAME_INFLIGHT)
        {
            const uint64_t expectedSubmittedFrameID = static_cast<uint64_t>(frameID - MAX_FRAME_INFLIGHT);

            while (!currentTicket->IsSubmissionReadyForFrame(expectedSubmittedFrameID))
            {
                std::this_thread::yield();
            }

            const uint64_t frameFenceValue = currentTicket->GetFenceValue();
            while (RenderAPI::GetInstance()->GetCurrentGPUCompletedFenceValue() < frameFenceValue)
            {
                std::this_thread::yield();
            }
        }
```

### File: `Runtime/Scene/GPUScene.cpp`
```cpp
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
```
...
```cpp
    BufferAllocation GPUScene::UploadDrawBatch(void *data, uint32_t size)
    {
        ASSERT(data != nullptr);
        ASSERT(size > 0);

        BufferAllocation destAllocation = GetVisibilityBufferByFrameID(mCurrentFrameID)->Allocate(size);
        BufferAllocation srcAllocation = mUploadPagePool->Allocate(size, data);

        CopyOp op = {};
```
...
```cpp
    {   
        EnsureCapacity(renderID); 
        UpdateDirtyFlags(renderID, flags);
        // 记录我当前脏ID即可
        UpdateShadowData(renderID, view);
    }

    void GPUScene::EnsureCapacity(uint32_t renderID)
    {
```
...
```cpp
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
```
...
```cpp
        if(flags & (uint32_t)NodeDirtyFlags::Destory)
        {
            TryFreeRenderProxyByRenderIndex(renderID);
            TryFreePerObjectDataAndAABBData(renderID);
            mPerObjectDatas[renderID].renderProxyCount = 0;
            return;
        }

        if(flags & (uint32_t)NodeDirtyFlags::TransformDirty || flags & (uint32_t)NodeDirtyFlags::Created)
        {
```

### File: `Runtime/Core/Game.cpp`
```cpp
    void Game::Launch()
    {
        ProjectSettings::Initialize();
        // InitManagers Here.
        RenderEngine::Create();
        ResourceManager::Create();
        SceneManager::Create();
        JobSystem::Create();
        AssetRegistry::Create();
        ASSERT(!(RenderSettings::s_EnableVertexPulling == true && RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy));
        // 当前GPUScene + false vertexpulling有问题主要是 shader选择，用bindless的即可
        ASSERT(!(RenderSettings::s_EnableVertexPulling == false && RenderSettings::s_RenderPath == RenderSettings::RenderPathType::GPUScene));
        //std::cout << "Launch Game" << std::endl;
        // init Manager...
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::Create();
        #endif
        while(!WindowManager::GetInstance()->WindowShouldClose())
        {
            PROFILER_FRAME_MARK("TinyProfiler");
            TickFrame(mFrameIndex);

            mFrameIndex++;

        }
```
...
```cpp

        PROFILER_EVENT_BEGIN("TickFrame::RenderEngineUpdate");
        RenderEngine::GetInstance()->PrepareFrame(frameIndex, sceneDelta);
        PROFILER_EVENT_END("TickFrame::RenderEngineUpdate");


        RenderEngine::GetInstance()->BuildFrame();


        PROFILER_EVENT_BEGIN("TickFrame::EndFrame");
        SceneManager::GetInstance()->EndFrame();
        RenderEngine::GetInstance()->EndFrame();
        PROFILER_EVENT_END("TickFrame::EndFrame");

    }

    void Game::Shutdown()
    {
```
...
```cpp
        // 1. 先停止渲染线程（最重要！）
        //    必须在销毁任何渲染资源之前停止
        RenderEngine::Destory();
        std::cout << "RenderEngine destroyed." << std::endl;

        // 2. 销毁编辑器UI
        #ifdef EDITOR
        EngineEditor::EditorGUIManager::OnDestory();
        std::cout << "EditorGUIManager destroyed." << std::endl;
        #endif

        // 3. 销毁场景（包含所有GameObject）
        SceneManager::Destroy();
        std::cout << "SceneManager destroyed." << std::endl;

        // 4. 最后销毁资源管理器
        ResourceManager::GetInstance()->Destroy();
        std::cout << "ResourceManager destroyed." << std::endl;

        std::cout << "Game shutdown complete." << std::endl;
    }

}
```

### File: `Assets/Shader/GPUCulling.hlsl`
```hlsl
// ==========================================

struct RenderProxy
{
    uint batchID;
};
```
...
```hlsl
// 2. 常量缓冲：视锥体平面和物体总数
// register(b0) -> 对应 C++ 的 CBV
cbuffer CullingParams : register(b0)
{
    // 视锥体的6个面：Left, Right, Bottom, Top, Near, Far
    // 这里的 float4 存储平面方程: Ax + By + Cz + D = 0
    // xyz 为法线(指向视锥体内侧), w 为距离 D
    float4 g_FrustumPlanes[6]; 
    
    // 需要剔除的实例总数
    uint g_TotalInstanceCount; 
};
```
...
```hlsl

// // 1. AABB 包围盒定义 (对应 C++ 结构体)
struct AABB
{
    float3 Min; // AABB 最小点
    float3 Max; // AABB 最大点
    float2 Padding;
};
```
...
```hlsl
    {
        uint instanceCount;
        InterlockedAdd(g_IndirectDrawCallArgs[batchID].InstanceCount, 1, instanceCount);
        // 执行后：InstanceCount 原子 +1；instanceCount 是加之前的值

        uint indexStart = g_IndirectDrawCallArgs[batchID].StartInstanceLocation;
        uint currentIndex = indexStart + instanceCount;
        // 4. 如果可见，将索引追加到输出列表
        // AppendStructuredBuffer 会自动处理原子计数
        //g_VisibleInstanceIndices.Append(instanceIndex);
        g_VisibleInstanceIndices[currentIndex] = instanceIndex;
    }

}
```

### File: `Runtime/Scene/GPUScene.h`
```cpp

        void UpdatePerFrameDirtyNode(CPUSceneView& view);
        void UploadCopyOp();
        
        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);
        void UpdateDirtyFlags(uint32_t renderID, uint32_t flags);
        void UpdateShadowData(uint32_t renderID, CPUSceneView& cpuScene);

        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }
```
...
```cpp

        inline GPUBufferAllocator* GetAllMaterialDataBuffer() { return allMaterialDataBuffer; }
        inline ResourceHandle<ComputeShader> GetCullingShaderHandler() { return GPUCullingShaderHandler; }
```

### File: `Runtime/Renderer/RenderCommand.h`
```cpp
#include "Renderer/FrameTicket.h"

namespace EngineCore
{
    enum class RenderOp : uint8_t
    {
        kInvalid = 0,
        kBeginFrame = 1,
        kEndFrame = 2,
        kSetRenderState = 3,
        kSetVBIB = 4,
        kSetViewPort = 5,
        kSetSissorRect = 6,
        //kDrawIndexed = 7,
        kSetMaterial = 8,
        kConfigureRT = 9,
        kWindowResize = 10,
        kIssueEditorGUIDraw = 11,
        kSetPerDrawData = 12,
        kDrawInstanced = 13,
        kSetPerFrameData = 14,
        kSetPerPassData = 15,
        kCopyBufferRegion = 16,
        kDispatchComputeShader = 17,
        kSetBufferResourceState = 18,
        kDrawIndirect = 19,
        kSetBindlessMat = 20,
        kSetBindLessMeshIB = 21,
        kSetFrame = 22,
        kCopyBufferStaged = 23,
    };

    enum class DepthComparisonFunc : uint8_t
    {
        LESS = 0,
        LEQUAL = 1,
        EQUAL = 2,
        GREATEQUAL = 3,
        GREAT = 4
    };

    enum class CullMode : uint8_t
    {
        CULLOFF = 0,
        CULLBACK = 1,
        CULLFRONT = 2,
    };

    enum class BlendState : uint8_t
    {
        SRCALHPHA = 0,
        ONEMINUSSRCALPHA = 2,
        ONE = 3,
        ZERO = 4,
    };

    // 和材质相关，关联材质后可以做合批操作。
    struct MaterailRenderState
    {
        uint32_t shaderInstanceID = 0;
        RootSignatureKey rootSignatureKey;
        // depth stencil state:
        bool enableDepthTest = true;
        bool enableDepthWrite = true;
        DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::LEQUAL;
        bool isBindLessMaterial = false;
        // blend mode
        bool enableBlend = false;
        BlendState srcBlend = (BlendState)0;
        BlendState destBlend = (BlendState)0;
        uint32_t hashID = 0;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(enableBlend));
```
...
```cpp
    };

    struct PSODesc
    {
        MaterailRenderState matRenderState;

        TextureFormat colorAttachment = TextureFormat::EMPTY;
        TextureFormat depthAttachment = TextureFormat::EMPTY;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, matRenderState.shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.srcBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.destBlend));
            HashCombine(hashID, static_cast<uint32_t>(colorAttachment));
            HashCombine(hashID, static_cast<uint32_t>(depthAttachment));
            return hashID;
        };
    private:
        uint32_t hashID = 0;
    };
```
...
```cpp
    };

    struct ClearValue
    {
        // todo :: vector3 -> vector4
        Vector3 colorValue;
        float depthValue;
        ClearFlag flags;
        ClearValue() : colorValue(Vector3::Zero), depthValue(1.0f), flags(ClearFlag::None) {};
        ClearValue(Vector3 color, float depth, ClearFlag flag):colorValue(color), depthValue(depth), flags(flag){};
    };
```
...
```cpp
    };

    struct alignas(16) CopyOp
    {
        IGPUBuffer* srcUploadBuffer;
        IGPUBuffer* destDefaultBuffer;
        uint32_t srcOffset;
        uint32_t dstOffset;
        uint32_t size;
    };
```
...
```cpp
    };

    struct Payload_DrawIndirect
    {
        // 这个payload只关心， 我绘制哪几个IndirectDraw，怎么找到，
        IGPUBuffer* indirectArgsBuffer;
        IGPUBuffer* indirectDrawCountBuffer;
        uint32_t startIndex;
        uint32_t count;
        uint32_t startIndexInInstanceDataBuffer;
    };
```

### File: `Runtime/Renderer/RenderBackend.h`
```cpp


        void DrawIndirect(Payload_DrawIndirect payload);
        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

        void RecycleStagedBuffer(const FrameTicket* ticket);
        void SubmitStagedBuffer(const FrameTicket* ticket);
        
        void RenderThreadMain() 
        {
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                PROFILER_ZONE("RenderThread::RenderLoop");

                // PROFILER_EVENT_BEGIN("RenderThread::WaitForSignalFromMainThread");
                // CpuEvent::MainThreadSubmited().Wait();
                // PROFILER_EVENT_END("RenderThread::WaitForSignalFromMainThread");


                PROFILER_EVENT_BEGIN("RenderThread::ProcessDrawComand");
                DrawCommand cmd;
                if (!mRenderBuffer.TryPop(cmd)) 
                {
                    mDataAvailableEvent.Wait();
                    continue;
                }

                bool hasBeginFrame = false;
                bool hasEndFrame = false;
                while (mRunning.load(std::memory_order_acquire) == true)
                {
                    if (cmd.op == RenderOp::kBeginFrame) hasBeginFrame = true;
                    if (cmd.op == RenderOp::kEndFrame)
                    {
                        hasEndFrame = true;
                        break;
                    }

                    ProcessDrawCommand(cmd);

                    if (!mRenderBuffer.TryPop(cmd))
                    {
                        mDataAvailableEvent.Wait();
                        if (!mRunning.load(std::memory_order_acquire)) break;
                        if (!mRenderBuffer.TryPop(cmd)) continue;
                    }
                }
                PROFILER_EVENT_END("RenderThread::ProcessDrawComand");

                if (!hasBeginFrame || !hasEndFrame)
                {
                    continue;
                }
                
                // todo Submit UploadPage 打上帧标签
                // later do Gpu Fence...
                RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR          
                PROFILER_EVENT_BEGIN("RenderThread::ProcessEditorGUI");
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
                // ImGui DrawData 已消费完毕，通知主线程可以安全调用 ImGui::NewFrame()
                CpuEvent::GUIDataConsumed().Signal();
                PROFILER_EVENT_END("RenderThread::ProcessEditorGUI");
#endif


                RenderAPI::GetInstance()->RenderAPIPresentFrame();


                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
```
...
```cpp
        

        void TryWakeUpRenderThread();
    private:
        static constexpr uint32_t kComputeBindingArenaSize = 64 * 1024;
        static constexpr uint32_t kComputeBindingAllocatorCount = 3;

        void EnqueueCommand(const DrawCommand& cmd);
        void WaitForQueueSpace();

        SPSCRingBuffer<DrawCommand, 16384> mRenderBuffer;
        std::thread mRenderThread;
        bool hasResize = false;
        bool hasDrawGUI = false;
        Payload_WindowResize pendingResize = { 0, 0 };
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;
        virtual void RenderAPICopyBufferStaged(Payload_CopyBufferStaged copyBufferStaged) override;
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) override;
        virtual RenderTexture* GetCurrentBackBuffer() override;

        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) override;

        TD3D12ConstantBuffer CreateConstantBuffer(uint32_t size);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
        // 给底层使用的具体资源
        D3D12Texture mBackBuffer[SwapChainBufferCount];
        // 一个壳，上层用，IGPUTexture = mBackBuffer
        RenderTexture mBackBufferProxyRenderTexture;
        D3D12Texture mBackBufferProxy;
        
        TD3D12Fence* mFrameFence;
        TD3D12Fence* mImediatelyFence;

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::CurrentBackBufferView()const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                mCurrBackBuffer,
                mRtvDescriptorSize);
        }
```
...
```cpp
        }

        void SignalFence(TD3D12Fence* mFence);
        void WaitForFence(TD3D12Fence* mFence);
        void WaitForRenderFinish(TD3D12Fence* mFence);
        virtual void WaitForGpuFinished() override { WaitForFence(mFrameFence); }
```
...
```cpp
        virtual void UploadBuffer(IGPUBuffer* bufferResource, uint32_t offset, void* data, uint32_t size) override;
        static D3D12_RESOURCE_STATES GetResourceState(BufferResourceState state);
        virtual uint64_t GetCurrentGPUCompletedFenceValue() override;
    private:

        bool InitDirect3D();
        void InitFence();
        void InitDescritorHeap();
        void InitCommandObject();
        void InitSwapChain();
        void InitRenderTarget();

        int GetNextVAOIndex();

        DXGI_FORMAT ConvertD3D12Format(TextureFormat format);
        inline bool IsCompressedFormat(TextureFormat format)
        {
            return format >= TextureFormat::DXT1 && format <= TextureFormat::BC7_SRGB;
        }
```

### File: `Runtime/Renderer/BatchManager.h`
```cpp
namespace EngineCore
{
    struct DrawIndirectParam
    {
        uint32_t indexCount = 0; // 比如这个Mesh有300个索引
        uint32_t startIndexInInstanceDataList = 0; // visiblityBuffer中的index
        uint32_t indexInDrawIndirectList = 0; //indirectDrawCallBuffer中的index
        uint32_t startIndexLocation;    // mesh big buffer 中的index index
        uint32_t baseVertexLocation;    // mesh big buffer 中的vertex index；
        DrawIndirectParam(uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
            : indexCount(indexCount), startIndexLocation(startIndex), baseVertexLocation(baseVertex)
        {

        }

        DrawIndirectParam() = default;
    };
```
...
```cpp
        Mesh* mesh;
        DrawIndirectContext() = default;
        DrawIndirectContext(int index, Material* mat, Mesh* mesh) : material(mat), mesh(mesh), batchIndex(index) {}
```

### File: `Runtime/Renderer/FrameTicket.h`
```cpp
        public:
            
            inline uint64_t GetFenceValue() const { return mGPUFenceValue.load(std::memory_order_acquire); }
```

### File: `Runtime/Scene/Scene.h`
```cpp
    {
    public:
        Scene();
        ~Scene();
        Scene(const std::string& name):name(name){};
```
...
```cpp
        void Close(){};
        void TickSimulation(uint32_t frameIndex);
        void EndFrame();
        GameObject* FindGameObject(const std::string& name);
        GameObject* CreateGameObject(const std::string& name);
        void DestroyGameObject(const std::string& name);

        void AddCamToStack(Camera* cam);
        inline void SetMainCamera(Camera* cam) { mainCamera = cam; }
```
...
```cpp
        
        // 只在GameObject层用到，析构和SetParent的时候， 创建的时候调用
        void RemoveGameObjectToSceneList(GameObject* object);
        void AddGameObjectToSceneList(GameObject* object);

        void AddRootGameObject(GameObject* object);
        void TryRemoveRootGameObject(GameObject* object);

        void PushNewTransformDirtyRoot(Transform* transform);

        
        //todo: 先用vector写死，后面要用priorityqueue之类的
        std::vector<Camera*> cameraStack;

        void RunLogicUpdate();
        void RunTransformUpdate();
        void RunRemoveInvalidDirtyRenderNode();

        uint32_t CreateRenderNode();
        
        void DeleteRenderNode(MeshRenderer *renderer);
        void MarkNodeCreated(MeshRenderer* renderer);
        void MarkNodeTransformDirty(Transform* transform);
        void MarkNodeMeshFilterDirty(MeshFilter* meshFilter);
        void MarkNodeMeshRendererDirty(MeshRenderer* renderer);
        void MarkNodeRenderableDirty(GameObject* object);
        
        inline std::vector<uint32_t>& GetPerFrameDirtyNodeList(){ return mPerFrameDirtyNodeList;}
```
...
```cpp
        }

        SceneDelta FlushSceneDelta();
    public:
        std::string name;
        std::vector<GameObject*> allObjList;
        std::vector<GameObject*> rootObjList;
        bool enabled = true;
        Camera* mainCamera = nullptr;

        std::vector<Transform*> dirtyRootDepthBucket[64];
        
    private:
        uint32_t mCurrentFrame = 0;
        void ApplyQueueNodeChange(uint32_t id, uint32_t flags, const NodeDirtyPayload& p);
        void InternalMarkNodeDeleted(MeshRenderer* renderer);
        
        std::vector<uint32_t> mNodeFrameStampList;
        std::vector<uint32_t> mNodeChangeFlagList;
        std::vector<NodeDirtyPayload> mNodeDirtyPayloadList;
        SceneDelta mSceneDelta;

        std::vector<uint32_t> mPerFrameDirtyNodeList;
        
        uint32_t mCurrSceneIndex = 0;
        std::vector<uint32_t> mFreeSceneIndex;
        std::vector<uint32_t> mPendingFreeSceneIndex;

        void EnsureNodeQueueSize(uint32_t size);
        void ClearPerFrameData();
        void ClearDirtyRootTransform();
        void PushLastFrameFreeIndex();

        AssetID GetMaterialID(MeshRenderer* meshRenderer);
        AssetID GetMeshID(MeshFilter* meshFilter);
    };    
} // namespace EngineCore
```

### File: `Runtime/Platforms/D3D12/D3D12ShaderUtils.h`
```cpp
#include "d3dUtil.h"

namespace EngineCore
{
    class D3D12ShaderUtils
    {
    public:
        static bool CompileShaderAndGetReflection(const string& path, Shader* shader);
        static bool CompileShaderStageAndGetReflection(const string& path, string entryPoint, string target, Shader* shader, ShaderStageType type, Microsoft::WRL::ComPtr<ID3DBlob>& blob);
        static bool D3D12ShaderUtils::CompileComputeShaderAndGetReflection(const string& path, ComputeShader* csShader);
        
        static Microsoft::WRL::ComPtr<ID3DBlob> GetVSBlob(uint32_t shaderID)
        {
            ASSERT(vsBlobMap.count(shaderID) > 0);
            return vsBlobMap[shaderID];
        }

        static Microsoft::WRL::ComPtr<ID3DBlob> GetPSBlob(uint32_t shaderID)
        {
            ASSERT(psBlobMap.count(shaderID) > 0);
            return psBlobMap[shaderID];
        }

        static Microsoft::WRL::ComPtr<ID3DBlob> GetCSBlob(uint32_t shaderID)
        {
            ASSERT(csBlobMap.count(shaderID) > 0);
            return csBlobMap[shaderID];
        }
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> vsBlobMap;
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> psBlobMap;
        static unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3DBlob>> csBlobMap;
    };
}
```

### File: `Runtime/Platforms/D3D12/D3D12Struct.h`
```cpp
    };

    struct TD3D12Fence
    {
    public:
        uint64_t mCurrentFence;
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    };
```
...
```cpp

    // 存储创建PSO需要的信息
    struct TD3D12PSO
    {
        PSODesc desc;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
        ComPtr<ID3DBlob> vsBlob;
        ComPtr<ID3DBlob> psBlob;
    };
```

### File: `Runtime/Platforms/D3D12/d3dUtil.h`
```cpp
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

/*
#if defined(_DEBUG)
    #ifndef Assert
    #define Assert(x, description)                                  \
    {                                                               \
```
...
```cpp
                    else if(result == Debug::AssertBreak)           \
        {                                                           \
            __debugbreak();                                         \
        }                                                           \
        }                                                           \
    }
    #endif
#else
    #ifndef Assert
    #define Assert(x, description) 
    #endif
#endif 		
    */

class d3dUtil
{
```
...
```cpp
    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }
```
...
```cpp
		const std::string& target);

    static D3D12_RESOURCE_DIMENSION GetFBOD3D12Dimesnsion(const EngineCore::TextureDimension& dimension);
    static DXGI_FORMAT GetFBOD3D12Format(const EngineCore::TextureFormat& format);
};

class DxException
{
```
...
```cpp
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};


struct D3D12DrawCommand
{
```