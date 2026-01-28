# Architecture Digest: GPU_ARCHITECTURE
> Auto-generated. Focus: Runtime/Platforms/D3D12, Runtime/Graphics, Runtime/Serialization, GPUBuffer, Descriptor, RootSignature, Bindless, ResourceState

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- GPU体系结构聚焦资源布局、绑定方式与跨API抽象。
- 关注GPUBuffer、Texture与Bindless/Descriptor策略。

## Key Files Index
- `[40]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp** *(Content Included)*
- `[39]` **Runtime/Platforms/D3D12/D3D12RootSignature.h** *(Content Included)*
- `[37]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GPUBufferAllocator.cpp** *(Content Included)*
- `[35]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[33]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[33]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[26]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[23]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[22]` **Runtime/Platforms/D3D12/D3D12DescManager.h** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[20]` **Runtime/Platforms/D3D12/D3D12DescManager.cpp** *(Content Included)*
- `[18]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[18]` **Runtime/Platforms/D3D12/D3D12Buffer.h** *(Content Included)*
- `[17]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[17]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[17]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/D3D12Texture.h** *(Content Included)*
- `[17]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[16]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[16]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[15]` **Runtime/Graphics/ComputeShader.h**
- `[15]` **Runtime/Platforms/D3D12/D3D12PSO.cpp**
- `[15]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[14]` **Runtime/Graphics/Mesh.h**
- `[14]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[14]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Graphics/GeometryManager.cpp**
- `[12]` **Runtime/Graphics/GPUTexture.h**
- `[12]` **Runtime/Graphics/Material.h**
- `[12]` **Runtime/Graphics/MeshUtils.h**
- `[12]` **Runtime/Graphics/RenderTexture.h**
- `[12]` **Runtime/Graphics/Shader.h**
- `[12]` **Runtime/Graphics/Texture.h**
- `[12]` **Runtime/Renderer/RenderEngine.cpp**
- `[12]` **Runtime/Serialization/AssetHeader.h**
- `[12]` **Runtime/Serialization/DDSTextureLoader.h**
- `[12]` **Runtime/Serialization/MaterialLoader.h**
- `[12]` **Runtime/Serialization/MeshLoader.h**
- `[12]` **Runtime/Serialization/SceneLoader.h**
- `[12]` **Runtime/Serialization/StreamHelper.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12PSO.h**
- `[11]` **Runtime/Graphics/ComputeShader.cpp**
- `[11]` **Runtime/Renderer/RenderAPI.h**
- `[11]` **Runtime/Renderer/Renderer.cpp**
- `[10]` **Runtime/Graphics/Mesh.cpp**
- `[10]` **Runtime/Graphics/MeshUtils.cpp**
- `[10]` **Runtime/Graphics/RenderTexture.cpp**
- `[10]` **Runtime/Graphics/Shader.cpp**
- `[10]` **Runtime/Graphics/Texture.cpp**
- `[10]` **Runtime/Renderer/Renderer.h**
- `[10]` **Runtime/Renderer/RenderStruct.h**
- `[8]` **Runtime/Scene/SceneManager.cpp**
- `[7]` **Runtime/Resources/ResourceManager.h**
- `[7]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**

## Evidence & Implementation Details

### File: `Runtime/Platforms/D3D12/D3D12RootSignature.cpp`
```cpp
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    unordered_map<RootSignatureKey, ComPtr<ID3D12RootSignature>, RootSignatureKey> D3D12RootSignature::mRootSigMap;
    
    ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateARootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, Shader *shader)
    {
        if (mRootSigMap.count(shader->mShaderReflectionInfo.mRootSigKey) > 0) 
        {
            return mRootSigMap[shader->mShaderReflectionInfo.mRootSigKey];
        }

        auto& srvInfo = shader->mShaderReflectionInfo.mTextureInfo;

        vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
        vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;
        descriptorRanges.reserve(10);

        slotRootParameter.emplace_back();
        auto indiceBind = GetRootSigBinding(RootSigSlot::DrawIndiceConstant);
        slotRootParameter.back().InitAsConstants(1, indiceBind.RegisterIndex, indiceBind.RegisterSpace);

        slotRootParameter.emplace_back();
        auto frameBind = GetRootSigBinding(RootSigSlot::PerFrameData);
        slotRootParameter.back().InitAsConstantBufferView(frameBind.RegisterIndex, frameBind.RegisterSpace);
        
        slotRootParameter.emplace_back();
        auto passBind = GetRootSigBinding(RootSigSlot::PerPassData);
        slotRootParameter.back().InitAsConstantBufferView(passBind.RegisterIndex, passBind.RegisterSpace);


        slotRootParameter.emplace_back();
        auto allObjectBind = GetRootSigBinding(RootSigSlot::AllObjectData);
        slotRootParameter.back().InitAsShaderResourceView(allObjectBind.RegisterIndex, allObjectBind.RegisterSpace); // register(u0, space0)
        
        slotRootParameter.emplace_back();
        auto allMaterialBind = GetRootSigBinding(RootSigSlot::AllMaterialData);
        slotRootParameter.back().InitAsShaderResourceView(allMaterialBind.RegisterIndex, allMaterialBind.RegisterSpace);
        
        slotRootParameter.emplace_back();
        auto perDrawInstanceList = GetRootSigBinding(RootSigSlot::PerDrawInstanceObjectsList);
        slotRootParameter.back().InitAsShaderResourceView(perDrawInstanceList.RegisterIndex, perDrawInstanceList.RegisterSpace);

        // ⭐ 处理 SRV (纹理) - Root Param 5+
        std::unordered_map<UINT, std::vector<ShaderBindingInfo>> srvBySpace;
        for (auto& bindingInfo : srvInfo)
        {
            srvBySpace[bindingInfo.space].push_back(bindingInfo);
        }

        // 执行空绑定
        if(srvBySpace.size() == 0)
        {
            descriptorRanges.emplace_back();
            descriptorRanges.back().Init(
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                1,
                0,  // 使用实际的起始寄存器位置
                0,
                0
            );

            slotRootParameter.emplace_back();
            slotRootParameter.back().InitAsDescriptorTable(1, &descriptorRanges.back());

        }
        for (auto& [space, srvList] : srvBySpace)
        {
            std::sort(srvList.begin(), srvList.end(),
                [](const ShaderBindingInfo& a, const ShaderBindingInfo& b) {
                    return a.registerSlot < b.registerSlot;
                });

            UINT rootParamIndex = slotRootParameter.size();

            UINT numDescriptors = 0;
            for(const auto& info : srvList)
            {
                if(info.bindCount == 0) 
```
...
```cpp
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&tempRootSignature)));

        mRootSigMap[shader->mShaderReflectionInfo.mRootSigKey] = tempRootSignature;
        return tempRootSignature;
    }

    ComPtr<ID3D12RootSignature> D3D12RootSignature::GetOrCreateAComputeShaderRootSig(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, ComputeShader *csShader)
    {
```
...
```cpp
        std::vector<ShaderBindingInfo> uavs = csShader->mShaderReflectionInfo.mUavInfo;
        std::vector<ShaderBindingInfo> samplersInfo = csShader->mShaderReflectionInfo.mSamplerInfo;
        std::sort(cbvs.begin(), cbvs.end(), SortBinding);
        std::sort(srvs.begin(), srvs.end(), SortBinding);
        std::sort(uavs.begin(), uavs.end(), SortBinding);
        std::sort(samplersInfo.begin(), samplersInfo.end(), SortBinding);

        std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
        rootParameters.reserve(cbvs.size() + srvs.size() + uavs.size());

        for (const auto& cbv : cbvs)
        {
            rootParameters.emplace_back();
            rootParameters.back().InitAsConstantBufferView(cbv.registerSlot, cbv.space);
        }
```
...
```cpp
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rootSig)));

        mRootSigMap[key] = rootSig;
        return rootSig;
    }
}
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

### File: `Runtime/Graphics/GPUBufferAllocator.h`
```cpp
    // Allocates small chunks of memory from a large GPU buffer.
    // Handles free list management for reusable blocks.
    class GPUBufferAllocator : public IGPUBufferAllocator
    {
    public:
        GPUBufferAllocator(const BufferDesc &usage);
        virtual ~GPUBufferAllocator();
        
        void Destory();

        // Allocates a block of 'size' bytes.
        // The allocator will search for a suitable free block or append to the end.
        virtual BufferAllocation Allocate(uint32_t size) override;
        
        // Frees an allocation, making its range available for reuse.
        virtual void Free(const BufferAllocation& allocation) override;
        
        // Resets the allocator, clearing all allocations (effectively freeing everything).
        // Useful for per-frame allocators.
        virtual void Reset() override;
        
        virtual uint64_t GetBaseGPUAddress() const override;
        virtual void UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size) override;
        virtual IGPUBuffer* GetGPUBuffer() override;
        BufferDesc bufferDesc;
    private:

        IGPUBuffer* m_Buffer = nullptr;
        uint64_t m_MaxSize = 0;
        uint64_t m_CurrOffset = 0; // Tracks the end of the used contiguous space

        // Keeps track of free ranges [offset, size]
        // This is a simple implementation; for high fragmentation scenarios, 
        // a more complex structure (like a segregated free list or RB tree) might be needed.
        struct FreeRange
        {
            uint64_t offset;
            uint32_t size;
        };
        std::vector<FreeRange> m_FreeRanges;
        
        // Helper to find a free block
        bool FindFreeBlock(uint32_t size, uint64_t& outOffset);
    };
```

### File: `Runtime/Graphics/IGPUBufferAllocator.h`
```cpp
namespace EngineCore
{
    class IGPUBufferAllocator
    {
        virtual BufferAllocation Allocate(uint32_t size) = 0;
        virtual void Free(const BufferAllocation& allocation) = 0;
        virtual void Reset() = 0;
        virtual uint64_t GetBaseGPUAddress() const = 0; 
        virtual void UploadBuffer(const BufferAllocation& alloc, void* data, uint32_t size) = 0;
        virtual IGPUBuffer* GetGPUBuffer() = 0;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.h`
```cpp
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
        {
            return mBackBuffer[mCurrBackBuffer].m_Resource.Get();
        }

        void SignalFence(TD3D12Fence* mFence);
        void WaitForFence(TD3D12Fence* mFence);
        void WaitForRenderFinish(TD3D12Fence* mFence);
        virtual void WaitForGpuFinished() override { WaitForFence(mFrameFence); }

        ComPtr<ID3D12PipelineState> psoObj;
        ComPtr<ID3D12RootSignature> rootSignature;
        
        void ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function);
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) override;
        virtual void UploadBuffer(IGPUBuffer* bufferResource, uint32_t offset, void* data, uint32_t size) override;
        static D3D12_RESOURCE_STATES GetResourceState(BufferResourceState state);
    private:

        bool InitDirect3D();
```

### File: `Runtime/Graphics/IGPUResource.h`
```cpp
    };

    enum class BufferResourceState 
    {
        STATE_COMMON = 0,
        STATE_UNORDERED_ACCESS, // UAV (RWStructuredBuffer, etc)
        STATE_SHADER_RESOURCE,  // SRV (StructuredBuffer, Texture, etc)
        STATE_INDIRECT_ARGUMENT, // Indirect Draw Args
        STATE_COPY_DEST,        // Copy Destination
        STATE_COPY_SOURCE,      // Copy Source
        STATE_GENERIC_READ,      // Generic Read (Vertex, Index, Constant, etc)
        STATE_DEPTH_WRITE,
        STATE_RENDER_TARGET,
        STATE_PRESENT
    };
```
...
```cpp
    };

    struct DescriptorHandle
    {
        uint64_t cpuHandle = UINT64_MAX;
        uint64_t gpuHandle = UINT64_MAX;
        uint32_t descriptorIdx = UINT32_MAX;  // bindless模式下，用的是index而不是地址
        bool isValid() const { return cpuHandle != UINT64_MAX; }
    };
```
...
```cpp
        virtual void SetName(const wchar_t* name) = 0;

        inline BufferResourceState GetState() const { return m_ResourceState;}
```
...
```cpp
    };

    class IGPUBuffer : public IGPUResource
    {
    public:
        virtual const BufferDesc& GetDesc() const = 0;
        virtual void* Map() = 0;
        virtual void UnMap() = 0;
    public:
        DescriptorHandle srvHandle;
        DescriptorHandle uavHandle;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12DescManager.h`
```cpp
        static Microsoft::WRL::ComPtr<ID3D12Device> mD3DDevice;
        
        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
```
...
```cpp
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, bool isShaderVisible = false);

        inline DescriptorHandle GetFrameCbvSrvUavAllocator(int count) 
        {
            return mBindlessAllocator->AllocateDynamicSpace(count);
        }
```

### File: `Runtime/Renderer/RenderCommand.h`
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
    };

    struct Payload_SetBindlessMat
    {
        Material* mat;
    };
```

### File: `Runtime/Platforms/D3D12/D3D12Buffer.h`
```cpp
namespace EngineCore
{
    class D3D12Buffer : public IGPUBuffer
    {
    public:
        D3D12Buffer(ComPtr<ID3D12Resource> resource, const BufferDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            if(desc.memoryType == BufferMemoryType::Upload)
            {
                Map();
            }

            switch(initialState)
            {
                case D3D12_RESOURCE_STATE_COMMON:
                m_ResourceState = BufferResourceState::STATE_COMMON;
                break;
                case D3D12_RESOURCE_STATE_GENERIC_READ:
                m_ResourceState = BufferResourceState::STATE_GENERIC_READ;
                break;
                case D3D12_RESOURCE_STATE_COPY_DEST:
                m_ResourceState = BufferResourceState::STATE_COPY_DEST;
                break;
                default:
                    ASSERT("Wrong InitialState");
                break;
            }
        }

        virtual ~D3D12Buffer()
        {
            if(m_Desc.memoryType != BufferMemoryType::Default) UnMap();
        }

        virtual const BufferDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}

    
        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }
    
        virtual void* Map() override
        {
            if(!m_MappedData){
                m_Resource->Map(0, nullptr, &m_MappedData);
            }
            return m_MappedData;
        }

        virtual void UnMap() override
        {
            if(m_MappedData)
            {
                m_Resource->Unmap(0, nullptr);
                m_MappedData = nullptr;
            }
        }
    
        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}

    private:
        ComPtr<ID3D12Resource> m_Resource;
        BufferDesc m_Desc;
        void* m_MappedData = nullptr;
    };
}
```

### File: `Runtime/Graphics/GeometryManager.h`
```cpp
#include "Graphics/Mesh.h"

namespace EngineCore
{
    class GeometryManager
    {
    public:
        GeometryManager();
        ~GeometryManager()
        {
            delete m_GlobalVertexBufferAllocator;
            delete m_GLobalIndexBufferAllocator;
        }
        static GeometryManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                s_Instance = new GeometryManager();
            }
            return s_Instance;
        }
        
        MeshBufferAllocation* AllocateVertexBuffer(void* data, int size);
        MeshBufferAllocation* AllocateIndexBuffer(void* data, int size);

        void FreeVertexAllocation(MeshBufferAllocation* allocation);
        void FreeIndexAllocation(MeshBufferAllocation* allocation);
        inline IGPUBuffer* GetVertexBuffer(){ return m_GlobalVertexBufferAllocator->GetGPUBuffer();}
        inline IGPUBuffer* GetIndexBuffer(){ return m_GLobalIndexBufferAllocator->GetGPUBuffer();}
        inline uint32_t GetIndexBufferSize(){ return m_GLobalIndexBufferAllocator->bufferDesc.size;}    
    private:
        GPUBufferAllocator* m_GlobalVertexBufferAllocator;
        GPUBufferAllocator* m_GLobalIndexBufferAllocator;
        static GeometryManager* s_Instance;
    };
};
```

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
#include "Resources/ResourceHandle.h"

namespace EngineCore
{

    class GPUSceneManager
    {
    public:
        static GPUSceneManager* GetInstance();
        GPUSceneManager();
        static void Create();
        void Tick();
        void Destroy();
        
        BufferAllocation GetSinglePerMaterialData();
        void RemoveSinglePerMaterialData(const BufferAllocation& bufferalloc);
        void UpdateSinglePerMaterialData(const BufferAllocation& bufferalloc, void* data);

        void TryFreeRenderProxyBlock(uint32_t index);
        void TryCreateRenderProxyBlock(uint32_t index);
        BufferAllocation LagacyRenderPathUploadBatch(void *data, uint32_t size);
        void FlushBatchUploads();
        void UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList);
        void UpdateAABBandPerObjectBuffer(const vector<uint32_t>& transformDirtyList, const vector<uint32_t>& materialDirtyList);

        vector<PerObjectData> perObjectDataBuffer;

        LinearAllocator* perFramelinearMemoryAllocator;

        GPUBufferAllocator* allMaterialDataBuffer;
        GPUBufferAllocator* allObjectDataBuffer;
        GPUBufferAllocator* perFrameBatchBuffer;
        GPUBufferAllocator* allAABBBuffer;
        GPUBufferAllocator* renderProxyBuffer;


        BufferAllocation visiblityAlloc;
        GPUBufferAllocator* visibilityBuffer;
        
        ResourceHandle<ComputeShader> GPUCullingShaderHandler;
    private:
        static GPUSceneManager* sInstance; 
        vector<CopyOp> mPendingBatchCopies;
    };

}
```

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible = false);
        ~D3D12DescAllocator(){};
```
...
```cpp

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
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
```
...
```cpp
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

}
```

### File: `Runtime/Platforms/D3D12/D3D12Texture.h`
```cpp
#include "d3dUtil.h"

namespace EngineCore
{
    // 只是一个资源的壳，IGPUTexture的实现，持有指针
    class D3D12Texture : public IGPUTexture
    {
    public:
        D3D12Texture() = default;

        D3D12Texture(const TextureDesc& desc) 
            : m_Desc(desc)
        {
        }

        D3D12Texture(ComPtr<ID3D12Resource> resource, const TextureDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            switch(initialState)
            {
                case D3D12_RESOURCE_STATE_COMMON:
                m_ResourceState = BufferResourceState::STATE_COMMON;
                break;
                case D3D12_RESOURCE_STATE_GENERIC_READ:
                m_ResourceState = BufferResourceState::STATE_GENERIC_READ;
                break;
                case D3D12_RESOURCE_STATE_COPY_DEST:
                m_ResourceState = BufferResourceState::STATE_COPY_DEST;
                break;
                default:
                    ASSERT("Wrong InitialState");
                break;
            }
        }

        virtual ~D3D12Texture()
        {
        }

        virtual const TextureDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}


        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }

        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}
    public:
        ComPtr<ID3D12Resource> m_Resource;
        TextureDesc m_Desc;
    };
}
```