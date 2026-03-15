# Architecture Digest: CONCURRENCY
> Auto-generated. Focus: Runtime/Core/Concurrency, JobSystem, CpuEvent, Renderer, RenderThreadMain, RenderAPI, D3D12RenderAPI, Fence, Wait, WaitForLastFrameFinished, SignalMainThreadSubmited, FrameContext, mMaxFrameCount

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
- CPU和GPU的同步，渲染线程和主线程的同步，多queue之间的同步
- 关注CpuEvent、Fence、WaitForLastFrameFinished、SignalMainThreadSubmited与多帧FrameContext协同是否合理

## Key Files Index
- `[64]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[63]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[41]` **Runtime/Renderer/RenderAPI.h** *(Content Included)*
- `[41]` **Runtime/Renderer/RenderEngine.cpp** *(Content Included)*
- `[39]` **Runtime/Core/Concurrency/CpuEvent.h** *(Content Included)*
- `[38]` **Runtime/Core/Concurrency/JobSystem.cpp** *(Content Included)*
- `[38]` **Runtime/Core/Concurrency/JobSystem.h** *(Content Included)*
- `[37]` **Runtime/Renderer/RenderAPI.cpp** *(Content Included)*
- `[35]` **Runtime/Core/Concurrency/CpuEvent.cpp** *(Content Included)*
- `[33]` **Runtime/Renderer/RenderBackend.h** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[25]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[21]` **Runtime/Renderer/RenderBackend.cpp** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[20]` **Runtime/Renderer/RenderEngine.h** *(Content Included)*
- `[18]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[18]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp** *(Content Included)*
- `[17]` **Runtime/Renderer/FrameTicket.h** *(Content Included)*
- `[17]` **Runtime/Renderer/RenderPipeLine/RenderPass.h** *(Content Included)*
- `[17]` **Editor/D3D12/D3D12EditorGUIManager.cpp** *(Content Included)*
- `[16]` **Runtime/Renderer/BatchManager.h**
- `[16]` **Runtime/Renderer/UploadPagePool.h**
- `[16]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h**
- `[16]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[15]` **Runtime/Renderer/RenderContext.cpp**
- `[14]` **Runtime/Core/Game.cpp**
- `[14]` **Runtime/Renderer/RenderCommand.h**
- `[14]` **Runtime/Renderer/RenderPath/IRenderPipeline.h**
- `[14]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.h**
- `[13]` **Runtime/Renderer/RenderContext.h**
- `[13]` **Runtime/Renderer/RenderSorter.h**
- `[13]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp**
- `[13]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[13]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[13]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h**
- `[12]` **Runtime/Renderer/Culling.cpp**
- `[12]` **Runtime/Renderer/Culling.h**
- `[12]` **Runtime/Renderer/PerDrawAllocator.h**
- `[12]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Renderer/RenderUniforms.h**
- `[12]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[12]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.cpp**
- `[11]` **Runtime/Renderer/UploadPagePool.cpp**
- `[11]` **Runtime/Renderer/RenderPipeLine/RenderPass.cpp**
- `[11]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp**
- `[10]` **Runtime/Renderer/BatchManager.cpp**
- `[10]` **Runtime/Renderer/FrameTicket.cpp**
- `[7]` **Runtime/Scene/Scene.h**
- `[7]` **Runtime/Serialization/SceneLoader.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[6]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[6]` **Runtime/Scene/GPUScene.cpp**
- `[6]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[6]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/Core/PublicStruct.h**
- `[5]` **Runtime/Scene/GPUScene.h**
- `[5]` **Runtime/Scene/Scene.cpp**
- `[5]` **Runtime/Scene/SceneManager.cpp**
- `[5]` **Editor/Panel/EditorMainBar.cpp**
- `[4]` **Runtime/Core/ThreadSafeQueue.h**

## Evidence & Implementation Details

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp`
```cpp
    {
        InitDirect3D();
        InitFence();
        InitDescritorHeap();
        InitCommandObject();
        InitSwapChain();

        InitRenderTarget();
        D3D12DescManager::Create(md3dDevice);
        //m_DataMap = unordered_map<uint32_t, TD3D12MaterialData>();


    }

    bool D3D12RenderAPI::InitDirect3D()
    {
```
...
```cpp
        mImediatelyFence = new TD3D12Fence();
        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFrameFence->mFence)));
        mFrameFence->mCurrentFence = 0;

        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mImediatelyFence->mFence)));
        mImediatelyFence->mCurrentFence = 0;

    }
    
    void D3D12RenderAPI::InitRenderTarget()
    {
```
...
```cpp
        mClientHeight = height;
        // Flush before changing any resources.
	    WaitForFence(mFrameFence);
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc[mCurrBackBuffer].Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mBackBuffer[i].m_Resource.Reset();
        mDepthStencilBuffer.Reset();
        
        // Resize the swap chain.
        ThrowIfFailed(mSwapChain->ResizeBuffers(
            SwapChainBufferCount, 
            mClientWidth, mClientHeight, 
            mBackBufferFormat, 
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        mCurrBackBuffer = mSwapChain->GetCurrentBackBufferIndex();
    
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < SwapChainBufferCount; i++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i].m_Resource)));
            md3dDevice->CreateRenderTargetView(mBackBuffer[i].m_Resource.Get(), nullptr, rtvHeapHandle);
            mBackBuffer[i].rtvHandle.cpuHandle = rtvHeapHandle.ptr;
            mBackBuffer[i].SetState(BufferResourceState::STATE_PRESENT);
            mBackBuffer[i].SetName(L"BackBuffer");
            rtvHeapHandle.Offset(1, mRtvDescriptorSize);

            // Init RenderTexture Wrapper
        }
```
...
```cpp

        // Wait until resize is complete.
        WaitForRenderFinish(mFrameFence);

        // Update the viewport transform to cover the client area.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width    = static_cast<float>(mClientWidth);
        mScreenViewport.Height   = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mScissorRect = { 0, 0, mClientWidth, mClientHeight };
```
...
```cpp
    void D3D12RenderAPI::WaitForRenderFinish(TD3D12Fence* mFence)
	{
		SignalFence(mFence);
		WaitForFence(mFence);
	}

    void D3D12RenderAPI::SignalFence(TD3D12Fence* mFence)
	{
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
{

    class D3D12RenderAPI final : public RenderAPI
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
        virtual void RenderAPISetFrame(Payload_SetFrame setFrame) override;
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

        ID3D12Resource* D3D12RenderAPI::CurrentBackBuffer()const
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
        virtual void RenderAPISetFrame(Payload_SetFrame setFrame) = 0;
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

        virtual uint64_t GetCurrentGPUCompletedFenceValue() = 0;
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    private:
        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) = 0;

    };
    
} // namespace  EngineCore

```

### File: `Runtime/Core/Concurrency/CpuEvent.h`
```cpp
namespace EngineCore
{
    class CpuEvent
    {
    public:
        //static CpuEvent& RenderThreadSubmited();
        //static CpuEvent& MainThreadSubmited();
        // 渲染线程消费完ImGui DrawData后发出信号，主线程BeginFrame等待它
        // 初始为true：第一帧不需要等待上一帧
        static CpuEvent& GUIDataConsumed();

        CpuEvent(bool startCondition = false)
            : m_signaled(startCondition){}

        void Signal()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_signaled = true;
            }
            m_cv.notify_all();
        }

        void Wait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]{ return m_signaled;});
            m_signaled = false;
        }

    private:
        std::mutex  m_mutex;
        std::condition_variable m_cv;
        bool    m_signaled = false;
    };
```

### File: `Runtime/Core/Concurrency/JobSystem.h`
```cpp
    };

    class JobSystem
    {
    public:
        JobSystem();
        ~JobSystem();
        static void Create();
        static void Shutdown();
        static JobSystem* GetInstance();

        template<typename CallableJob>
        void KickJob(CallableJob job, JobHandle& handler, JobCounter* counter)
        {
            if(counter == nullptr)
            {
                counter = GetAvaliableCounter();
            }

            void* jobData = new CallableJob(job);

            counter->value.fetch_add(1);
            auto lambda = [](void* jobData, void* rawCounter)
            {
                CallableJob* job = (CallableJob*)jobData;
                job();
                JobCounter* counter = (JobCounter*)rawCounter;
                counter->value.fetch_sub(1);
                delete jobData;
            }
            handler.counter = counter;
            InternalKickJob(lambda, jobData, counter);
        }

        std::deque<InternalJob> jobQueue;
        bool isRunning = false;
        std::mutex queueMutex;
        std::condition_variable wakeWorker;
        std::vector<std::thread> m_Workers;

        void WaitForJob(JobHandle handle);

    private:
        void InternalKickJob(void (*function)(void*, void*), void* JobData, void* rawCounter);
        void WorkerThreadLoop(); 
        bool TryExecuteOneJob();
        static JobSystem* s_Instance;
        JobCounter* GetAvaliableCounter();
        std::deque<JobCounter*> counterQueue;
    };
```

### File: `Runtime/Renderer/RenderBackend.h`
```cpp
                
                // later do Gpu Fence...
                RenderAPI::GetInstance()->RenderAPISubmit();

#ifdef EDITOR          
                PROFILER_EVENT_BEGIN("RenderThread::ProcessEditorGUI");
                if (hasDrawGUI)
                {
                    EngineEditor::EditorGUIManager::GetInstance()->EndFrame();
                    hasDrawGUI = false;
                }
```
...
```cpp


                RenderAPI::GetInstance()->RenderAPIPresentFrame();


                if (hasResize)
                {
                    RenderAPI::GetInstance()->RenderAPIWindowResize(pendingResize);
                    hasResize = false;
                    pendingResize = { 0,0 };
                }
```
...
```cpp
    private:
        void EnqueueCommand(const DrawCommand& cmd);
        void WaitForQueueSpace();

        SPSCRingBuffer<DrawCommand, 16384> mRenderBuffer;
        std::thread mRenderThread;
        bool hasResize = false;
        bool hasDrawGUI = false;
        Payload_WindowResize pendingResize = { 0, 0 };
```

### File: `Runtime/GameObject/MeshRenderer.h`
```cpp
namespace EngineCore
{
    class MeshRenderer : public Component
    {
        class GameObejct;
    public:
        MeshRenderer() = default;
        MeshRenderer(GameObject* gamObject);
        virtual ~MeshRenderer() override;
        static ComponentType GetStaticType() { return ComponentType::MeshRenderer; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshRenderer; };

        virtual const char* GetScriptName() const override { return "MeshRenderer"; }
        
        void SetUpMaterialPropertyBlock();
        void SetDefaultMaterial();
        inline Material* GetSharedMaterial()
        { 
            return mShardMatHandler.IsValid() ? mShardMatHandler.Get() : nullptr;
        };

        void SetSharedMaterial(const ResourceHandle<Material>& mat);

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);

        bool shouldUpdateMeshRenderer = true;
        AABB worldBounds;
        bool materialDirty = true;
		
        uint32_t renderLayer = 1;
        void OnLoadResourceFinished();
        inline uint32_t GetCPUWorldIndex() { return mCPUWorldIndex;}
        inline void SetCPUWorldIndex(uint32_t index) { mCPUWorldIndex = index;}
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

        uint32_t mCPUWorldIndex = UINT32_MAX;
    };
```

### File: `Runtime/Renderer/RenderEngine.h`
```cpp
    public:
        static RenderEngine* GetInstance(){return s_Instance.get();};
        static bool IsInitialized(){return s_Instance != nullptr;};
```
...
```cpp
        static void Create();
        void BuildFrame();
        void WaitForFrameAvaliable(uint32_t frameID);
        void EndFrame();
        
        static void OnResize(int width, int height);
        static void OnDrawGUI();
        static void Destory();
        RenderEngine(){};
```
...
```cpp
        }

        inline uint32_t GetCurrentFrame(){ return mCurrentFrameID;}
```
...
```cpp
        uint32_t mCurrentFrameID = 0;

        void ComsumeDirtySceneRenderNode(const SceneDelta& delta);
        void UploadCopyOp();
    };
    
}
```

### File: `Runtime/Renderer/FrameTicket.h`
```cpp
        public:
            
            inline uint64_t GetFenceValue() const { return mGPUFenceValue.load(std::memory_order_acquire); }
```

### File: `Runtime/Renderer/RenderPipeLine/RenderPass.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    class RenderPass
    {
    public:

        // init RenderPass, for example Create FBO..
        //virtual void Create(const RenderContext& context) = 0;
        virtual void Create() = 0;
        
        // Record Rendertarget in mRenderPassInfo..
        virtual void Configure(const RenderContext& context) = 0;
        
        // decide what content we need to draw, for example
        // in opaquepass we only need gameobject renderqueue less than 3000.
        virtual void Filter(const RenderContext& context) = 0;
        
        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // CPU Prepare Data.
        virtual void Prepare(RenderContext& context) = 0;

        // first record drawInfo then sync data to RenderAPI, finally Submit to execute the
        // Submit To RenderThread
        virtual void Execute(RenderContext& context) = 0;
        
        // sent current Data to Renderer-> RenderPassInfo
        virtual void Submit() = 0;

        inline void Clear() { mRenderPassInfo.Reset(); };


        // ignore null ptr since RenderAPI level will handle this problem
        inline void SetRenderTarget(RenderTexture* colorAttachment, RenderTexture* depthAttachment)
        {
            mRenderPassInfo.colorAttachment = colorAttachment;
            mRenderPassInfo.depthAttachment = depthAttachment;
        };

        inline void SetClearFlag(ClearFlag flag, const Vector3& colorValue = Vector3::Zero, float depthValue = 1.0)
        {
            mRenderPassInfo.clearFlag = flag;
            mRenderPassInfo.clearColorValue = colorValue;
            mRenderPassInfo.clearDepthValue = depthValue;
        };

        inline void SetViewPort(const Vector2& startPos, const Vector2& endPos)
        {
            mRenderPassInfo.viewportStartPos = startPos;
            mRenderPassInfo.viewportEndPos = endPos;
        }

        inline const RenderPassInfo& GetRenderPassInfo(){return mRenderPassInfo;};
        RootSigSlot mRootSigSlot;
    protected:
        void IssueRenderCommandCommon(const RenderPassInfo& passInfo,
                                      const std::vector<RenderBatch>& batches
                                      );
    public:
        string name;
        RenderPassInfo mRenderPassInfo;
    };
} // namespace EngineCore
```