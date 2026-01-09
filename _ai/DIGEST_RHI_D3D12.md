# Architecture Digest: RHI_D3D12
> Auto-generated. Focus: Runtime/Platforms/D3D12, RenderAPI, Descriptor, RootSignature, PSO, SwapChain, Barrier, Command, Fence, Context

## Key Files Index
- `[70]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[64]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[41]` **Runtime/Platforms/D3D12/D3D12PSO.cpp** *(Content Included)*
- `[40]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp** *(Content Included)*
- `[39]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[38]` **Runtime/Platforms/D3D12/D3D12PSO.h** *(Content Included)*
- `[32]` **Runtime/Renderer/RenderAPI.h** *(Content Included)*
- `[32]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[27]` **Runtime/Renderer/RenderContext.h** *(Content Included)*
- `[27]` **Runtime/Platforms/D3D12/D3D12Struct.h** *(Content Included)*
- `[27]` **Runtime/Platforms/D3D12/d3dx12.h** *(Content Included)*
- `[25]` **Runtime/Renderer/RenderAPI.cpp** *(Content Included)*
- `[25]` **Runtime/Renderer/RenderContext.cpp** *(Content Included)*
- `[23]` **Editor/D3D12/D3D12EditorGUIManager.cpp** *(Content Included)*
- `[20]` **Runtime/Renderer/Renderer.cpp** *(Content Included)*
- `[20]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp** *(Content Included)*
- `[20]` **Runtime/Platforms/D3D12/d3dUtil.cpp** *(Content Included)*
- `[18]` **Runtime/Renderer/Renderer.h** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12DescManager.cpp** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[17]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12Buffer.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[12]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[10]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[9]` **Runtime/Renderer/RenderEngine.cpp**
- `[9]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[8]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[7]` **Runtime/Graphics/IGPUResource.h**
- `[7]` **Runtime/Renderer/BatchManager.h**
- `[7]` **Runtime/Renderer/RenderEngine.h**
- `[7]` **Runtime/Renderer/RenderSorter.h**
- `[7]` **Runtime/Renderer/RenderStruct.h**
- `[7]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[7]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[7]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h**
- `[6]` **Runtime/Core/PublicStruct.h**
- `[6]` **Runtime/Renderer/SPSCRingBuffer.h**
- `[6]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[5]` **Runtime/Renderer/BatchManager.cpp**
- `[5]` **Runtime/Renderer/Culling.h**
- `[5]` **Runtime/Renderer/RenderPath/IRenderPath.h**
- `[5]` **Runtime/Platforms/Windows/WindowManagerWindows.cpp**
- `[3]` **Editor/EditorGUIManager.h**
- `[3]` **Runtime/Graphics/GPUBufferAllocator.cpp**

## Evidence & Implementation Details

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
namespace EngineCore
{
    class D3D12RenderAPI : public RenderAPI
    {
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        virtual void CompileShader(const string& path, Shader* shader) override;
        virtual void CompileComputeShader(const string& path, ComputeShader* csShader) override;
        virtual void CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) override;
        virtual void CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) override;

        inline TD3D12Fence* GetFrameFence() { return mFrameFence; };
        // todo: maybe可以清理成模板。
        //virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID) override;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) override;
        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) override;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) override;
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
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
    private:

        bool InitDirect3D();
        void InitFence();
        void InitDescritorHeap();
        void InitCommandObject();
        void InitSwapChain();
        void InitRenderTarget();

        int GetNextVAOIndex();
        TD3D12VAO& GetAvaliableModelDesc();


        Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
        Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
        
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;




        // 屏幕后台缓冲区图像格式
		DXGI_FORMAT mPresentBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// 默认的纹理和FrameBuffer色彩空间
		const DXGI_FORMAT mDefaultImageFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// 4X MSAA质量等级
		UINT m4xMSAAQuality = 0;
		UINT msaaSamplesCount = 4;


        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mImediatelyCmdListAlloc;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mImediatelyCommandList;
        
        DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        bool      m4xMsaaState = false;    // 4X MSAA enabled
        UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
        
        Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

        D3D12_VIEWPORT mScreenViewport; 
        D3D12_RECT mScissorRect;
        int mClientWidth = 1280;
        int mClientHeight = 720;

        //unordered_map<uint32_t, TD3D12MaterialData> m_DataMap;
        vector<ComPtr<ID3D12RootSignature>> mRootSignatureList;
        ComPtr<ID3D12CommandSignature> mCommandSignature;
        unordered_map<uint32_t, TD3D12VAO> VAOMap;
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
    };
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

### File: `Runtime/Renderer/RenderAPI.h`
```cpp
namespace  EngineCore
{
    class RenderAPI
    {
    public:
        static RenderAPI* GetInstance(){ return s_Instance.get();}
        static bool IsInitialized(){return s_Instance != nullptr;};
       
        static void Create();
        virtual void  CompileShader(const string& path, Shader* shader) = 0;
        virtual void  CompileComputeShader(const string& path, ComputeShader* csShader) = 0;
        
        virtual void CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;
        virtual void CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;

        //virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID) = 0;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) = 0;
        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) = 0;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) = 0;
        
        //virtual void GetOrCreatePSO(const Material& mat, const RenderPassInfo &passinfo) = 0;
        inline void AddRenderPassInfo(const RenderPassInfo& renderPassInfo){ mRenderPassInfoList.push_back(renderPassInfo); };
        inline void ClearRenderPassInfo(){ mRenderPassInfoList.clear(); };
        
        virtual void RenderAPIBeginFrame() = 0;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) = 0;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) = 0;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) = 0;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) = 0;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) = 0;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) = 0;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) = 0;
        virtual void RenderAPISubmit() = 0;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) = 0;
        virtual void RenderAPIPresentFrame() = 0;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) = 0;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) = 0;
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
        uint32_t vaoID;
        int count;
    };
```
...
```cpp
    };

    struct Payload_DrawInstancedCommand
    {
        uint32_t vaoID;
        int count;
        uint32_t perDrawOffset;
        uint32_t perDrawStride;
    };
```
...
```cpp
    };

    struct DrawCommand 
    {
        RenderOp op{ RenderOp::kInvalid };
        CommandData data;
    public:
        DrawCommand(){};
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
        int currentMaxIndex = 0;

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

    private:

    };
```

### File: `Runtime/Platforms/D3D12/D3D12Struct.h`
```cpp
    };

    struct TD3D12Fence
    {
    public:
        int mCurrentFence;
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
{
    CD3DX12_ROOT_DESCRIPTOR_TABLE() {}
    explicit CD3DX12_ROOT_DESCRIPTOR_TABLE(const D3D12_ROOT_DESCRIPTOR_TABLE &o) :
        D3D12_ROOT_DESCRIPTOR_TABLE(o)
    {}
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
...
```cpp

//------------------------------------------------------------------------------------------------
struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE() {}
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE &o) :
        D3D12_CPU_DESCRIPTOR_HANDLE(o)
    {}
    CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(other, offsetScaledByIncrementSize);
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
    { 
        ptr += offsetInDescriptors * descriptorIncrementSize;
        return *this;
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize) 
    { 
        ptr += offsetScaledByIncrementSize;
        return *this;
    }
    bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
    {
        return (ptr == other.ptr);
    }
    bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other)
    {
        return (ptr != other.ptr);
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE &operator=(const D3D12_CPU_DESCRIPTOR_HANDLE &other)
    {
        ptr = other.ptr;
        return *this;
    }
    
    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(*this, base, offsetScaledByIncrementSize);
    }
    
    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }
    
    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize)
    {
        handle.ptr = base.ptr + offsetScaledByIncrementSize;
    }
    
    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
    }
```

### File: `Runtime/Renderer/Renderer.h`
```cpp

        void BeginFrame();
        void Render(RenderContext& context);
        void EndFrame();

        void DrawIndexed(uint32_t vaoID, int count);
        void ResizeWindow(int width, int height);
        void OnDrawGUI();
        void SetPerDrawData(const PerDrawHandle& perDrawHandle);
        void DrawIndexedInstanced(uint32_t vaoID, int count, const PerDrawHandle& perDrawHandle);
        void SetPerFrameData(UINT perFrameBufferID);
        void SetPerPassData(UINT perPassBufferID);
        
        void SetRenderState(const Material* mat, const RenderPassInfo &passinfo);

        void SetMaterialData(Material* mat);

        void ConfigureRenderTarget(const RenderPassInfo& passInfo);

        void SetMeshData(uint32_t vaoID);

        void SetViewPort(const Vector2& viewportStartXY, const Vector2& viewportEndXY);
        // todo: complete this..
        void SetSissorRect(const Vector2& viewportStartXY, const Vector2& viewportEndXY);

        void ProcessDrawCommand(const DrawCommand& cmd);

        void CopyBufferRegion(const Payload_CopyBufferRegion& copyCmd);

        void DispatchComputeShader(const Payload_DispatchComputeShader& dispatchCmd);
        
        void SetResourceState(IGPUResource* resource, BufferResourceState state);
        
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
```
...
```cpp

        void FlushPerFrameData();
        void FlushPerPassData(const RenderContext& context);
        void CreatePerFrameData();
        void CreatePerPassForwardData();

    };
}
```

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};
```
...
```cpp
        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        DescriptorHandle GetNextAvaliableDesc();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
        void Reset();
        DescriptorHandle GetFrameAllocator(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        std::vector<bool> isInUse;

        int currentOffset = 0;
        int currCount = 0;
        uint32_t startIndex = 0;
        uint32_t mDescriptorSize = 0;
        int maxCount = 0;
    };

}
```