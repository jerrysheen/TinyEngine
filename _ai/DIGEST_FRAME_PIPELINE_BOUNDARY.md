# Architecture Digest: FRAME_PIPELINE_BOUNDARY
> Auto-generated. Focus: Runtime/Core/Game.cpp, Runtime/Renderer/RenderEngine.cpp, Runtime/Scene/GPUScene.cpp, Runtime/Renderer/FrameContext.h, Runtime/Renderer/FrameContext.cpp, Runtime/Renderer/RenderPath/LagacyRenderPath.h, Runtime/Renderer/RenderPath/LagacyRenderPath.cpp, Runtime/Renderer/RenderPath/GPUSceneRenderPath.h, Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp, Runtime/Renderer/Renderer.cpp, Game::Update, Game::Render, RenderEngine::Update, RenderEngine::Tick, GPUScene::Update, ApplyDirtyNode, UpdateDirtyNode, UploadCopyOp, FrameContext, CopyOp, perFrameUploadBuffer, PersistentGPUScene, SceneDelta, DirtyFlags, RenderBuild, RenderSubmit, BuildVsSubmit, MainThreadFrontend, RenderThreadBackend, GPUCulling, DrawIndirect, LegacyPath, GPUScenePath, frames-in-flight, fence, deferred free

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
- `[51]` **Runtime/Renderer/FrameContext.cpp** *(Content Included)*
- `[44]` **Runtime/Renderer/FrameContext.h** *(Content Included)*
- `[30]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[30]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[24]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[24]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[21]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[20]` **Runtime/Renderer/Renderer.cpp** *(Content Included)*
- `[16]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[14]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[14]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[13]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[13]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[12]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[12]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h** *(Content Included)*
- `[10]` **Runtime/Renderer/RenderPath/LagacyRenderPath.cpp** *(Content Included)*
- `[8]` **Runtime/Scene/SceneManager.cpp** *(Content Included)*
- `[7]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[7]` **Runtime/Renderer/RenderAPI.h** *(Content Included)*
- `[6]` **Runtime/Scene/CPUScene.cpp**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/Renderer/BatchManager.cpp**
- `[5]` **Runtime/Renderer/RenderStruct.h**
- `[5]` **Runtime/Scene/Scene.cpp**
- `[5]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[4]` **Runtime/Renderer/RenderEngine.h**
- `[4]` **Runtime/Scene/CPUScene.h**
- `[4]` **Runtime/Scene/SceneStruct.h**
- `[4]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[4]` **Assets/Shader/BlitShader.hlsl**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**
- `[4]` **Assets/Shader/StandardPBR.hlsl**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
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
- `[2]` **Runtime/Graphics/GPUBufferAllocator.h**

## Evidence & Implementation Details

### File: `Runtime/Renderer/FrameContext.cpp`
```cpp


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
```
...
```cpp
            copyObject.copyList = mCopyOpsObject.data();
            copyObject.count = mCopyOpsObject.size();
            Renderer::GetInstance()->CopyBufferRegion(copyObject);
        }

        if (mCopyOpsAABB.size() > 0)
        {
```
...
```cpp
            copyAABB.copyList = mCopyOpsAABB.data();
            copyAABB.count = mCopyOpsAABB.size();
            Renderer::GetInstance()->CopyBufferRegion(copyAABB);
        }

        if (mCopyOpsProxy.size() > 0)
        {
```
...
```cpp
            copyRenderProxy.copyList = mCopyOpsProxy.data();
            copyRenderProxy.count = mCopyOpsProxy.size();
            Renderer::GetInstance()->CopyBufferRegion(copyRenderProxy);
        }

        if (mCopyOpsVisibility.size() > 0)
        {
```
...
```cpp
            copyVisibility.copyList = mCopyOpsVisibility.data();
            copyVisibility.count = mCopyOpsVisibility.size();
            Renderer::GetInstance()->CopyBufferRegion(copyVisibility);
        }

    }

    void FrameContext::UpdateShadowData(uint32_t renderID, CPUSceneView &view)
    {
```

### File: `Runtime/Renderer/FrameContext.h`
```cpp
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
```

### File: `Runtime/Renderer/RenderEngine.cpp`
```cpp
        s_Instance = std::make_unique<RenderEngine>();
        s_Instance->GetGPUScene().Create();
        WindowManager::Create();
        RenderAPI::Create();
        Renderer::Create();


        if (RenderSettings::s_RenderPath == RenderSettings::RenderPathType::Legacy)
        {
            s_Instance->mCurrentRenderPath = new LagacyRenderPath();
        }
```
...
```cpp
        PROFILER_EVENT_END("MainThread::WaitForGpuFinished");
        mCurrentRenderPath->Prepare(renderContext);
        UploadCopyOp();
    }    
    
    void RenderEngine::OnResize(int width, int height)
    {
```
...
```cpp
        s_Instance->mCurrentRenderPath->Execute(renderContext);
        
        SignalMainThreadSubmited();

    }

    void RenderEngine::EndFrame()
    {
```
...
```cpp
    {
        // Renderer 的析构函数会自动停止渲染线程
        Renderer::Destroy();
        //RenderAPI::Destroy();
        WindowManager::Destroy();
        RenderEngine::GetInstance()->GetGPUScene().Destroy();

        delete s_Instance->mCurrentRenderPath;
        // 最后销毁 RenderEngine 自己
        if (s_Instance)
        {
            // 先调用析构（智能指针自动管理）
            s_Instance.reset();
            // 此时：
            // 1. 析构函数已完成
            // 2. 内存已释放
            // 3. s_Instance 已经是 nullptr
            // 4. 后续 GetInstance() 不会返回野指针
        }
```
...
```cpp
    void RenderEngine::WaitForLastFrameFinished()
    {
        PROFILER_EVENT_BEGIN("MainThread::WaitforSignalFromRenderThread");
        CpuEvent::RenderThreadSubmited().Wait();
        PROFILER_EVENT_END("MainThread::WaitforSignalFromRenderThread");

    }

    void RenderEngine::SignalMainThreadSubmited()
    {
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
        BufferAllocation UploadDrawBatch(void *data, uint32_t size);

        void UpdateDirtyNode(CPUSceneView& view);
        void UploadCopyOp();

        void ApplyDirtyNode(uint32_t renderID, uint32_t flags, CPUSceneView& view);


        void UpdateFrameContextDirtyFlags(uint32_t renderID, uint32_t flag);
        void UpdateFrameContextShadowData(uint32_t renderID, CPUSceneView& view);
        
        FrameContext* GetCurrentFrameContexts();
        inline uint32_t GetCurrentFrameID() const { return mCurrentFrameID; }
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
namespace EngineCore
{
    class FrameContext;

    class D3D12RenderAPI : public RenderAPI
    {
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        virtual void CompileShader(const string& path, Shader* shader) override;
        virtual void CompileComputeShader(const string& path, ComputeShader* csShader) override;

        inline TD3D12Fence* GetFrameFence() { return mFrameFence; };
        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) override;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) override;
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) override;
        virtual void RenderAPISetBindLessMeshIB() override;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) override;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) override;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) override;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) override;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) override;
        virtual void RenderAPISubmit() override;
        virtual void RenderAPIPresentFrame() override;
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) override;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) override;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) override;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) override;
        virtual void RenderAPISetFrameContext(Payload_SetFrameContext setFrameContext) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;

        
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

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::DepthStencilView()const
        {
            return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
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

### File: `Runtime/Renderer/RenderCommand.h`
```cpp
namespace EngineCore
{
    class FrameContext;

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
        kSetFrameContext = 22,
    };
```
...
```cpp
    };

    struct Payload_SetFrameContext
    {
        FrameContext* frameContext = nullptr;
        uint32_t frameID = 0;
    };
```
...
```cpp
    };

    struct Payload_DrawIndirect
    {
        // 这个payload只关心， 我绘制哪几个IndirectDraw，怎么找到，
        IGPUBuffer* indirectArgsBuffer;
        uint32_t startIndex;
        uint32_t count;
        uint32_t startIndexInInstanceDataBuffer;
    };
```

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPath.h`
```cpp
#include "Renderer/FrameContext.h"

namespace EngineCore
{
    class GPUSceneRenderPath : public IRenderPath
    {
    public:
        virtual ~GPUSceneRenderPath() override 
        {
            delete cullingParamBuffer;
            delete indirectDrawArgsBuffer;
        };

        virtual void Execute(RenderContext& context) override;
        virtual void Prepare(RenderContext& context) override;


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}
```

### File: `Runtime/Renderer/Renderer.h`
```cpp
        void SetPerFrameData(UINT perFrameBufferID);
        void SetPerPassData(UINT perPassBufferID);
        void SetFrameContext(FrameContext* frameContext, uint32_t frameID);
        
        void SetRenderState(const Material* mat, const RenderPassInfo &passinfo);

        void SetMaterialData(Material* mat);

        void ConfigureRenderTarget(const RenderPassInfo& passInfo);

        void SetMeshData(Mesh* meshFilter);

        void SetViewPort(const Vector2& viewportStartXY, const Vector2& viewportEndXY);
        // todo: complete this..
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void CopyBufferRegion(const Payload_CopyBufferRegion& copyCmd);

        void DispatchComputeShader(const Payload_DispatchComputeShader& dispatchCmd);
        
        void SetResourceState(IGPUResource* resource, BufferResourceState state);
        
        void SetBindlessMat(Material* mat);
        void SetBindLessMeshIB(uint32_t id);
        
        void DrawIndirect(Payload_DrawIndirect payload);
        
        void RenderThreadMain() 
        {
            while (mRunning.load(std::memory_order_acquire) == true) 
            {
                PROFILER_ZONE("RenderThread::RenderLoop");

                PROFILER_EVENT_BEGIN("RenderThread::WaitForSignalFromMainThread");
                CpuEvent::MainThreadSubmited().Wait();
                PROFILER_EVENT_END("RenderThread::WaitForSignalFromMainThread");

                RenderAPI::GetInstance()->RenderAPIBeginFrame();
                DrawCommand cmd;

                PROFILER_EVENT_BEGIN("RenderThread::ProcessDrawComand");
                while(mRenderBuffer.PopBlocking(cmd))
                {
                    if (cmd.op == RenderOp::kEndFrame) break;
                    ProcessDrawCommand(cmd);
                }
                PROFILER_EVENT_END("RenderThread::ProcessDrawComand");
                // later do Gpu Fence...
                RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR          
                PROFILER_EVENT_BEGIN("RenderThread::ProcessEditorGUI");
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
                PROFILER_EVENT_END("RenderThread::ProcessEditorGUI");
#endif


                RenderAPI::GetInstance()->RenderAPIPresentFrame();

                CpuEvent::RenderThreadSubmited().Signal();

                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                    hasResize = false;
                    pendingResize = { 0,0 };
                }

            }
        };
        

    private:
        SPSCRingBuffer<16384> mRenderBuffer;
```
...
```cpp
        PerPassData_Forward mPerPassData_Forward;

        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

    };
}
```

### File: `Runtime/Renderer/RenderPath/LagacyRenderPath.h`
```cpp
#include "Scene/GPUSCene.h"

namespace EngineCore
{
    class LagacyRenderPath : public IRenderPath
    {
    public:
        virtual ~LagacyRenderPath() override {};
        virtual void Execute(RenderContext& context) override;
        virtual void Prepare(RenderContext& context) override;
    };
};
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
        DrawIndirectContext(Material* mat, Mesh* mesh): material(mat), mesh(mesh){}
```
...
```cpp
        std::vector<RenderProxy> GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        uint64_t GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        static void Create();
        static BatchManager* s_Instance;

    };
}
```

### File: `Runtime/Renderer/RenderAPI.h`
```cpp


namespace  EngineCore
{
    class RenderAPI
    {
    public:
        inline static RenderAPI* GetInstance()
        {
            if (s_Instance == nullptr) 
            {   
                Create();
            }
            return s_Instance.get();
        }
        static bool IsInitialized(){return s_Instance != nullptr;};
       
        static void Create();
        virtual void  CompileShader(const string& path, Shader* shader) = 0;
        virtual void  CompileComputeShader(const string& path, ComputeShader* csShader) = 0;
        

        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) = 0;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) = 0;
        
        //virtual void GetOrCreatePSO(const Material& mat, const RenderPassInfo &passinfo) = 0;
        inline void AddRenderPassInfo(const RenderPassInfo& renderPassInfo){ mRenderPassInfoList.push_back(renderPassInfo); };
        inline void ClearRenderPassInfo(){ mRenderPassInfoList.clear(); };
        
        virtual void RenderAPIBeginFrame() = 0;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) = 0;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) = 0;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) = 0;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) = 0;
        virtual void RenderAPISetBindLessMeshIB() = 0;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) = 0;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) = 0;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) = 0;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) = 0;
        virtual void RenderAPISubmit() = 0;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) = 0;
        virtual void RenderAPIPresentFrame() = 0;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) = 0;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) = 0;
        virtual void RenderAPISetFrameContext(Payload_SetFrameContext setFrameContext) = 0;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) = 0;
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) = 0;
        
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) = 0;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) = 0;
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) = 0;
        virtual void UploadBuffer(IGPUBuffer* buffer, uint32_t offset, void* data, uint32_t size) = 0;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) = 0;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) = 0;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) = 0;
        virtual RenderTexture* GetCurrentBackBuffer() = 0;
        template<typename T>
        void SetGlobalValue(uint32_t bufferID, uint32_t offset, T* value)
        {
            uint32_t size = sizeof(T);
            SetGlobalDataImpl(bufferID, offset, size, static_cast<void*>(value));
        }
        virtual void WaitForGpuFinished() = 0;
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    private:
        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) = 0;

    };
    
} // namespace  EngineCore

```