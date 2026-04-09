# Architecture Digest: RHI_D3D12
> Auto-generated. Focus: Runtime/Platforms/D3D12, RenderAPI, Descriptor, RootSignature, PSO, SwapChain, Barrier, Command, Fence, Context, D3D12RenderAPI, D3D12DescAllocator, D3D12DescManager, InitDescritorHeap, CreateRenderTexture, CreateConstantBuffer, ResourceState

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
- D3D12渲染硬件接口实现，决定现代渲染器能力上限（资源状态、PSO、RootSignature、DescriptorHeap等）。
- 抽取关键API与资源/命令上下文结构，包含D3D12RenderAPI、D3D12DescAllocator等核心类。

## Key Files Index
- `[105]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[95]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[53]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp** *(Content Included)*
- `[45]` **Runtime/Platforms/D3D12/D3D12DescManager.h** *(Content Included)*
- `[44]` **Runtime/Platforms/D3D12/D3D12DescManager.cpp** *(Content Included)*
- `[40]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[40]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp** *(Content Included)*
- `[39]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[36]` **Runtime/Renderer/RenderAPI.h** *(Content Included)*
- `[36]` **Runtime/Platforms/D3D12/D3D12PSO.cpp** *(Content Included)*
- `[35]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[33]` **Runtime/Platforms/D3D12/D3D12PSO.h** *(Content Included)*
- `[30]` **Editor/D3D12/D3D12EditorGUIManager.cpp** *(Content Included)*
- `[27]` **Runtime/Renderer/RenderAPI.cpp** *(Content Included)*
- `[27]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[27]` **Runtime/Platforms/D3D12/d3dx12.h** *(Content Included)*
- `[25]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[24]` **Runtime/Platforms/D3D12/D3D12Struct.h** *(Content Included)*
- `[24]` **Runtime/Platforms/D3D12/d3dUtil.cpp** *(Content Included)*
- `[22]` **Runtime/Renderer/RenderBackend.h** *(Content Included)*
- `[21]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Runtime/Renderer/RenderBackend.cpp**
- `[18]` **Runtime/Renderer/RenderEngine.cpp**
- `[17]` **Runtime/Platforms/D3D12/D3D12Buffer.h**
- `[17]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[16]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[14]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[13]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Graphics/IGPUResource.h**
- `[11]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[10]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.cpp**
- `[9]` **Runtime/Renderer/RenderPath/GPUSceneRenderPipeline.h**
- `[9]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[8]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.h**
- `[8]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[8]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[7]` **Runtime/Renderer/BatchManager.h**
- `[7]` **Runtime/Renderer/FrameTicket.h**
- `[7]` **Runtime/Renderer/RenderEngine.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
- `[7]` **Runtime/Renderer/RenderPath/IRenderPipeline.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h**
- `[6]` **Runtime/Core/PublicStruct.h**
- `[5]` **Runtime/Renderer/Culling.h**
- `[5]` **Runtime/Resources/ResourceHandle.h**
- `[5]` **Runtime/Scene/GPUScene.cpp**
- `[5]` **Runtime/Renderer/RenderPath/LagacyRenderPipeline.cpp**
- `[5]` **Runtime/Platforms/Windows/WindowManagerWindows.cpp**
- `[4]` **Runtime/Renderer/BatchManager.cpp**
- `[4]` **Runtime/Renderer/UploadPagePool.h**
- `[4]` **Runtime/Scene/SceneManager.cpp**
- `[4]` **Assets/Shader/BlitShader.hlsl**
- `[4]` **Assets/Shader/GPUCulling.hlsl**
- `[4]` **Assets/Shader/IndirectDrawCallCombineComputeShader.hlsl**
- `[4]` **Assets/Shader/SimpleTestShader.hlsl**

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
        
        // Execute the resize commands.
        ThrowIfFailed(mCommandList->Close());
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
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
        ComPtr<ID3D12RootSignature> rootSignature;
        
        void ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function);
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) override;
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

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.cpp`
```cpp
            
		heapDesc.NodeMask = 0;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mHeap.GetAddressOf())));
        mDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(mHeapType);
    }

    // 需要取到GPU地址的，要保持着色器可见。
    D3D12_DESCRIPTOR_HEAP_FLAGS D3D12DescAllocator::GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap)
    {
```
...
```cpp

		// 创建CBV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateConstantBufferView(&desc, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;
        // update 时候绑定，用GPU Handle
        // 不用有GPU地址， 因为这个堆运行时会拷贝到另外一个堆，这个堆只能CPU可见。
        //CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
        //gpuHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        //handle.gpuHandle = gpuHandle;
        return handle;
    }
   
    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc)
    {
```
...
```cpp

        // 创建RTV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateRenderTargetView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;

        return handle;
    }


    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc)
    {
```
...
```cpp

		// 创建DSV
		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
		mD3D12Device->CreateDepthStencilView(resource.Get(), nullptr, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;

        return handle;
    }

    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
    {
```
...
```cpp

        // 创建CBV
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
        descriptorHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        mD3D12Device->CreateShaderResourceView(resource.Get(), &desc, descriptorHandle);
        handle.cpuHandle = descriptorHandle.ptr;
        // update 时候绑定，用GPU Handle
        // 不用有GPU地址， 因为这个堆运行时会拷贝到另外一个堆，这个堆只能CPU可见。
        //CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
        //gpuHandle.Offset(handle.descriptorIdx, mDescriptorSize);
        //handle.gpuHandle = gpuHandle;
        return handle;
    }

    DescriptorHandle D3D12DescAllocator::CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc)
    {
```

### File: `Runtime/Platforms/D3D12/D3D12DescManager.h`
```cpp
namespace EngineCore
{
    class D3D12DescManager
    {
    public:
        D3D12DescManager();
        ~D3D12DescManager();
        static D3D12DescManager* GetInstance(){return mInstance;};
        static void Create(Microsoft::WRL::ComPtr<ID3D12Device> device);
        static Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice;
        
        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, bool isShaderVisible = false);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

        inline DescriptorHandle GetFrameCbvSrvUavAllocator(int count) 
        {
            return mBindlessAllocator->AllocateDynamicSpace(count);
        }
        

        void ResetFrameAllocator();
        vector<D3D12DescAllocator> mDescAllocators;
 
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetBindlessCbvSrvUavHeap()
        {
            return mBindlessAllocator->mHeap;
        }


    private:
        static D3D12DescManager* mInstance;
        // Bindless Heap (Shader Visible, Global)
        D3D12DescAllocator* mBindlessAllocator = nullptr;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
namespace EngineCore
{
    class D3D12DescAllocator
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible = false);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
        DescriptorHandle AllocateStaticHandle();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

        void Reset();
        void CleanPerFrameData();

        // 设置动态分配的起始位置（用于混合 Heap 模式）
        void SetDynamicStartOffset(int offset) 
        { 
            dynamicStartOffset = offset;
            currDynamicoffset = offset;
        }
        // 专门用于 Global Heap 的动态分配
        DescriptorHandle AllocateDynamicSpace(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        //std::vector<bool> isInUse;
        std::vector<int> freeIndexList;

        int dynamicStartOffset = 0; // 记录动态分配的起始点，Reset 时回到这里
        int currDynamicoffset = 0;
        
        int currentOffset = 0;
        uint32_t startIndex = 0;
        uint32_t mDescriptorSize = 0;
        int maxCount = 0;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12RootSignature.h`
```cpp
//      5	    SRV (Root/Table)	t0, space1	        Global Object Data      	极低 (Per Scene)
//      6	    SRV (Root/Table)	t1, space1	        Global Material Data    	极低 (Per Scene)
    class D3D12RootSignature
    {
    public:
    // 第三行为比较函数
        static unordered_map<RootSignatureKey, ComPtr<ID3D12RootSignature>, RootSignatureKey> mRootSigMap;

    public:
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, Shader* shader);
        static ComPtr<ID3D12RootSignature> GetOrCreateAComputeShaderRootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, ComputeShader* csShader);
        static ComPtr<ID3D12RootSignature> GetOrCreateARootSig(const RootSignatureKey& key)
        {
            ASSERT(mRootSigMap.count(key) > 0);
            return mRootSigMap[key];
        }

        static ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateGPUSceneGraphicsRootSig(ComPtr<ID3D12Device> md3dDevice);
    };
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
        virtual void RenderAPICopyBufferStaged(Payload_CopyBufferStaged copyBufferStaged) = 0;
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

### File: `Runtime/Renderer/RenderCommand.h`
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

    struct Payload_DrawCommand 
    {
        Mesh* mesh;
        int count;
    };
```
...
```cpp
    };

    struct Payload_DrawInstancedCommand
    {
        Mesh* mesh;
        int count;
        uint32_t perDrawOffset;
        uint32_t perDrawStride;
    };
```
...
```cpp
    };

    struct Payload_SetBufferResourceState
    {
        IGPUResource* resource;
        BufferResourceState state;
    };
```
...
```cpp
        Payload_SetBindLessMeshIB SetBindLessMeshIB;
        Payload_CopyBufferStaged copyBufferStaged;
        CommandData() {};
```

### File: `Runtime/Platforms/D3D12/D3D12PSO.h`
```cpp
{

    class D3D12PSO
    {
    public:
        static unordered_map<uint32_t, ComPtr<ID3D12PipelineState>> shaderPSOMap;
         
        static ComPtr<ID3D12PipelineState> CreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, const TD3D12PSO &pso);
        static ComPtr<ID3D12PipelineState> GetOrCreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc& psodesc);
        static ComPtr<ID3D12PipelineState> GetOrCreateComputeShaderPSO(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, PSODesc& psodesc);
    private:
        static D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(const MaterailRenderState& matRenderState);
        static D3D12_BLEND_DESC GetBlendDesc(const MaterailRenderState& matRenderState);
        static D3D12_RASTERIZER_DESC GetRasterizerDesc(const MaterailRenderState& matRenderState);

    };
```

### File: `Runtime/Renderer/RenderContext.h`
```cpp
{
    // 记录灯光，阴影，物体信息？
    class RenderContext
    {
    public:
        RenderContext()
        {
            visibleItems.resize(10000);
        }
        Camera* camera;
        vector<LightData*> visibleLights;
        vector<RenderPacket> visibleItems;
        
        inline void Reset()
        {
            camera = nullptr;
            visibleLights.clear();
            visibleItems.clear();
        }

        static void DrawRenderers(RenderContext& renderContext, 
                           const ContextDrawSettings& drawingSettings, 
                           const ContextFilterSettings& filteringSettings, 
                           std::vector<RenderBatch>& outDrawRecords);
        
        static void BatchContext(std::vector<RenderPacket*>& sortedItem);

        static bool CanBatch(const RenderBatch& batch, const RenderPacket item);

        IGPUBuffer* IndirectDrawArgsBuffer = nullptr;
        IGPUBuffer* IndirectDrawCountCountBuffer = nullptr;
        IGPUBuffer* CullingParamBuffer = nullptr;
        uint32_t currFrameIndex = 0;
    private:

    };
```

### File: `Runtime/Platforms/D3D12/d3dx12.h`
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
    CD3DX12_RESOURCE_BARRIER()
    {}
    explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER &o) :
        D3D12_RESOURCE_BARRIER(o)
    {}
    static inline CD3DX12_RESOURCE_BARRIER Transition(
        _In_ ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
    {
        CD3DX12_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        result.Flags = flags;
        barrier.Transition.pResource = pResource;
        barrier.Transition.StateBefore = stateBefore;
        barrier.Transition.StateAfter = stateAfter;
        barrier.Transition.Subresource = subresource;
        return result;
    }
    static inline CD3DX12_RESOURCE_BARRIER Aliasing(
        _In_ ID3D12Resource* pResourceBefore,
        _In_ ID3D12Resource* pResourceAfter)
    {
        CD3DX12_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barrier.Aliasing.pResourceBefore = pResourceBefore;
        barrier.Aliasing.pResourceAfter = pResourceAfter;
        return result;
    }
    static inline CD3DX12_RESOURCE_BARRIER UAV(
        _In_ ID3D12Resource* pResource)
    {
        CD3DX12_RESOURCE_BARRIER result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = pResource;
        return result;
    }
    operator const D3D12_RESOURCE_BARRIER&() const { return *this; }
};
```
...
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_DESCRIPTOR_RANGE : public D3D12_DESCRIPTOR_RANGE
{
    CD3DX12_DESCRIPTOR_RANGE() { }
    explicit CD3DX12_DESCRIPTOR_RANGE(const D3D12_DESCRIPTOR_RANGE &o) :
        D3D12_DESCRIPTOR_RANGE(o)
    {}
    CD3DX12_DESCRIPTOR_RANGE(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
    {
        Init(rangeType, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    }
    
    inline void Init(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
    {
        Init(*this, rangeType, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);
    }
    
    static inline void Init(
        _Out_ D3D12_DESCRIPTOR_RANGE &range,
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
    {
        range.RangeType = rangeType;
        range.NumDescriptors = numDescriptors;
        range.BaseShaderRegister = baseShaderRegister;
        range.RegisterSpace = registerSpace;
        range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};
```
...
```cpp
struct CD3DX12_ROOT_DESCRIPTOR_TABLE : public D3D12_ROOT_DESCRIPTOR_TABLE
{
    CD3DX12_ROOT_DESCRIPTOR_TABLE() {}
```
...
```cpp
        _In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE* _pDescriptorRanges)
    {
        Init(numDescriptorRanges, _pDescriptorRanges);
    }
    
    inline void Init(
        UINT numDescriptorRanges,
        _In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE* _pDescriptorRanges)
    {
```
...
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_ROOT_DESCRIPTOR : public D3D12_ROOT_DESCRIPTOR
{
    CD3DX12_ROOT_DESCRIPTOR() {}
    explicit CD3DX12_ROOT_DESCRIPTOR(const D3D12_ROOT_DESCRIPTOR &o) :
        D3D12_ROOT_DESCRIPTOR(o)
    {}
    CD3DX12_ROOT_DESCRIPTOR(
        UINT shaderRegister,
        UINT registerSpace = 0)
    {
        Init(shaderRegister, registerSpace);
    }
    
    inline void Init(
        UINT shaderRegister,
        UINT registerSpace = 0)
    {
        Init(*this, shaderRegister, registerSpace);
    }
    
    static inline void Init(_Out_ D3D12_ROOT_DESCRIPTOR &table, UINT shaderRegister, UINT registerSpace = 0)
    {
        table.ShaderRegister = shaderRegister;
        table.RegisterSpace = registerSpace;
    }
};
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

### File: `Runtime/Renderer/RenderBackend.h`
```cpp
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void CopyBufferRegion(const Payload_CopyBufferRegion& copyCmd);

        void DispatchComputeShader(const Payload_DispatchComputeShader& dispatchCmd);
        
        void SetResourceState(IGPUResource* resource, BufferResourceState state);
        
        void SetBindlessMat(Material* mat);
        void SetBindLessMeshIB(uint32_t id);
        
        void UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size);


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
        static constexpr uint32_t kComputeBindingAllocatorCount = 3;

        void EnqueueCommand(const DrawCommand& cmd);
        void WaitForQueueSpace();

        SPSCRingBuffer<DrawCommand, 16384> mRenderBuffer;
        std::thread mRenderThread;
        bool hasResize = false;
        bool hasDrawGUI = false;
        Payload_WindowResize pendingResize = { 0, 0 };
```