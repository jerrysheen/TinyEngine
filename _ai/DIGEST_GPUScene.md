# Architecture Digest: GPUScene
> Auto-generated. Focus: GPUSceneRenderPath, Runtime/Scene/GPUScene.h, Runtime/Scene/GPUScene.cpp, Runtime/Renderer/FrameContext.h, Runtime/Renderer/FrameContext.cpp, GPUSceneManager, FrameContext, mCPUFrameContext, CPUSceneView, NodeDirtyFlags, UpdateFrameContextDirtyFlags, UpdateFrameContextShadowData, UploadCopyOp, CopyOp, PerObjectData, DrawIndirectArgs, visibilityBuffer, GeometryManager, Mesh, Texture, IGPUResource, GPUBufferAllocator, ComputeShader, D3D12RenderAPI, GPUSceneRenderPass, BatchManager, GPUCulling, Indirect

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
- GPUScene关注GPU驱动裁剪、批处理与间接绘制数据结构。
- 提取GPUSceneManager、BatchManager与相关Buffer布局，关注FrameContext多帧脏标记同步与CopyOp上传流程。

## Key Files Index
- `[70]` **Runtime/Renderer/FrameContext.cpp** *(Content Included)*
- `[64]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[52]` **Runtime/Renderer/FrameContext.h** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[48]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[47]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[41]` **Runtime/Scene/GPUScene.cpp** *(Content Included)*
- `[40]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[37]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[34]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[34]` **Runtime/Scene/GPUScene.h** *(Content Included)*
- `[34]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[33]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[32]` **Runtime/Graphics/GeometryManager.cpp** *(Content Included)*
- `[31]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[31]` **Runtime/Renderer/RenderCommand.h** *(Content Included)*
- `[28]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[28]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Texture.h**
- `[28]` **Runtime/Serialization/DDSTextureLoader.h**
- `[28]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/GameObject/MeshRenderer.h**
- `[27]` **Runtime/Renderer/Renderer.cpp**
- `[27]` **Runtime/Serialization/MeshLoader.h**
- `[26]` **Runtime/GameObject/MeshFilter.cpp**
- `[26]` **Runtime/GameObject/MeshRenderer.cpp**
- `[26]` **Runtime/Graphics/ComputeShader.cpp**
- `[26]` **Runtime/Graphics/GPUTexture.h**
- `[26]` **Runtime/Graphics/MeshUtils.cpp**
- `[25]` **Runtime/Graphics/ComputeShader.h**
- `[25]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[25]` **Runtime/Graphics/MeshUtils.h**
- `[25]` **Runtime/Graphics/RenderTexture.cpp**
- `[25]` **Runtime/Graphics/Texture.cpp**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[22]` **Runtime/Serialization/TextureLoader.h**
- `[21]` **Runtime/Renderer/RenderAPI.h**
- `[20]` **Runtime/Entry.cpp**
- `[19]` **Runtime/Renderer/Renderer.h**
- `[18]` **Runtime/Renderer/RenderEngine.cpp**
- `[17]` **Runtime/Scene/CPUScene.cpp**
- `[15]` **Runtime/Scene/SceneManager.cpp**
- `[14]` **Runtime/Renderer/RenderStruct.h**
- `[14]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[14]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[13]` **Assets/Shader/SimpleTestShader.hlsl**
- `[13]` **Assets/Shader/StandardPBR.hlsl**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Core/PublicStruct.h**
- `[11]` **Runtime/Scene/SceneStruct.h**
- `[10]` **Runtime/Resources/ResourceManager.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.h**
- `[10]` **Runtime/Scene/Scene.cpp**
- `[10]` **Runtime/Scene/SceneManager.h**
- `[9]` **Runtime/Resources/ResourceManager.h**

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
        {
            TryFreeRenderProxyByRenderIndex(renderID);
            TryFreePerObjectDataAndAABBData(renderID);
            mPerObjectDatas[renderID].renderProxyCount = 0;
            return;
        }

        if(flags & (uint32_t)NodeDirtyFlags::TransformDirty || flags & (uint32_t)NodeDirtyFlags::Created)
        {
```
...
```cpp
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                ASSERT(op.dstOffset == renderID * sizeof(PerObjectData));
                op.size = sizeof(PerObjectData);
                mCopyOpsObject.push_back(std::move(op));

                allocation = allAABBBuffer->Allocate(sizeof(AABB));
                tempAllocation = perFrameUploadBuffer->Allocate(sizeof(AABB));
                perFrameUploadBuffer->UploadBuffer(tempAllocation, (void*)&cpuScene.worldBoundsList[renderID], sizeof(AABB));
                op.srcOffset = tempAllocation.offset;
                op.dstOffset = allocation.offset;
                op.size = sizeof(AABB);
                mCopyOpsAABB.push_back(std::move(op));
            }

            if((dirtyFlags & (uint32_t)NodeDirtyFlags::TransformDirty) || (dirtyFlags & (uint32_t)NodeDirtyFlags::MeshDirty))
            {
```

### File: `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp`
```cpp
    void D3D12RenderAPI::CompileComputeShader(const string &path, ComputeShader *csShader)
    {
        D3D12ShaderUtils::CompileComputeShaderAndGetReflection(path, csShader);
        D3D12RootSignature::GetOrCreateAComputeShaderRootSig(md3dDevice, csShader);
    }


    IGPUTexture* D3D12RenderAPI::CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc)
    {
```
...
```cpp
        
        // 创建默认堆 贴图资源， CPU不可见
        CD3DX12_HEAP_PROPERTIES textureProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC d3D12TextureDesc(
            CD3DX12_RESOURCE_DESC::Tex2D(
                d3dFormat, 
                textureDesc.width, 
                textureDesc.height, 
                1, 
                textureDesc.mipCount
            )
        );

        std::vector<D3D12_SUBRESOURCE_DATA> subresources(textureDesc.mipCount);
        for(uint32_t mip = 0; mip < textureDesc.mipCount; ++mip)
        {
            uint32_t mipWidth = std::max(1, textureDesc.width >> mip);
            uint32_t mipHeight = std::max(1, textureDesc.height >> mip);

            subresources[mip].pData = data + textureDesc.mipOffset[mip];

            if(isCompressed)
            {
                subresources[mip].RowPitch = CalculateCompressedRowPitch(textureDesc.format, mipWidth);
                subresources[mip].SlicePitch = CalculateCompressedSlicePitch(textureDesc.format, mipWidth, mipHeight);
            }
            else
            {
                uint32_t pixelByte = GetBytesPerPixel(textureDesc.format);
                subresources[mip].RowPitch = mipWidth * pixelByte;
                subresources[mip].SlicePitch = subresources[mip].RowPitch * mipHeight;
            }
        }
```
...
```cpp
            initialState,
            &clearValue,
            IID_PPV_ARGS(&texture->m_Resource)));
        std::wstring debugName = std::wstring(textureDesc.name.begin(), textureDesc.name.end());
        texture->m_Resource->SetName(debugName.c_str());
        // Create Descriptor:...
        if(textureDesc.format == TextureFormat::R8G8B8A8)
        {
            DescriptorHandle descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, D3D12_RENDER_TARGET_VIEW_DESC{});
            texture->rtvHandle = descHandle;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // SRV格式
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, srvDesc, false);
            texture->srvHandle = descHandle;
            texture->bindlessHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, srvDesc, true);
        }
```
...
```cpp
    void D3D12RenderAPI::RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand)
    {
        ASSERT(payloadDrawCommand.mesh != nullptr);
        Mesh* mesh = payloadDrawCommand.mesh;
        auto indexAllocation = mesh->indexAllocation;

        mCommandList->DrawIndexedInstanced(indexAllocation->size / sizeof(uint32_t), payloadDrawCommand.count, 0, 0, 0);
    }

    void D3D12RenderAPI::RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial)
    {
```
...
```cpp
        Shader* shader = payloadSetMaterial.shader;
        Material* mat = payloadSetMaterial.mat;
        ASSERT(mCurrentFrameContext != nullptr);
        ASSERT(mCurrentFrameContext->allObjectDataBuffer != nullptr);
        ASSERT(mCurrentFrameContext->visibilityBuffer != nullptr);

        uint64_t gpuAddr = mCurrentFrameContext->allObjectDataBuffer->GetGPUBuffer()->GetGPUVirtualAddress();
        if (materialStateCache.allObjectDataGpuAddress != gpuAddr) 
        {
            mCommandList->SetGraphicsRootShaderResourceView((UINT)RootSigSlot::AllObjectData, gpuAddr);
            materialStateCache.allObjectDataGpuAddress = gpuAddr;
        }
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
        DrawIndirectContext(Material* mat, Mesh* mesh): material(mat), mesh(mesh){}
```
...
```cpp

    class MeshRenderer;
    class BatchManager
    {
    public:
        static BatchManager* GetInstance()
        {
            if(s_Instance == nullptr)
            {
                Create();
            }
            return s_Instance;
        }
        void TryAddBatches(AssetID meshID, AssetID materialID, uint32_t layer); 
        void TryDecreaseBatches(AssetID meshID, AssetID materialID, uint32_t layer);
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;
        static std::unordered_map<uint64_t, int> BatchMap;
        
        std::vector<RenderProxy> GetAvaliableRenderProxyList(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        uint64_t GetBatchHash(AssetID meshID, AssetID materialID, uint32_t layer); 
        
        static void Create();
        static BatchManager* s_Instance;

    };
```

### File: `Runtime/Graphics/GeometryManager.h`
```cpp
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
```

### File: `Runtime/Graphics/IGPUResource.h`
```cpp
namespace EngineCore
{
    enum class TextureDimension
    {
        TEXTURE2D,
        TEXTURE3D,
        TEXTURECUBE,
        TEXTURE2DARRAY
    };
```
...
```cpp


    enum class TextureUsage
    {
        ShaderResource,
        RenderTarget,
        DepthStencil,
        UnorderedAccess   // 预留，现在反正没用处
    };
```
...
```cpp
    };

    class IGPUResource
    {
    public:
        virtual ~IGPUResource() = default;
        virtual void* GetNativeHandle() const = 0;
        virtual uint64_t GetGPUVirtualAddress() const = 0;
        virtual void SetName(const wchar_t* name) = 0;

        inline BufferResourceState GetState() const { return m_ResourceState;}
        inline void SetState(BufferResourceState state) { m_ResourceState = state; };
    protected:
        BufferResourceState m_ResourceState = BufferResourceState::STATE_COMMON;
        std::wstring m_Name;
    };
```
...
```cpp
    };

    class IGPUTexture : public IGPUResource
    {
    public:
        virtual const TextureDesc& GetDesc() const = 0;
    public:
        DescriptorHandle srvHandle; // Non-Visible Heap Handle (for Copy Source)
        DescriptorHandle bindlessHandle; // Shader-Visible Heap Handle (for Bindless Access)
        DescriptorHandle rtvHandle;
        DescriptorHandle dsvHandle;
    };
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

### File: `Assets/Shader/GPUCulling.hlsl`
```hlsl
};

struct IndirectDrawCallArgs
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    uint BaseVertexLocation;
    uint StartInstanceLocation;
};
```
...
```hlsl


struct PerObjectData
{
    float4x4 objectToWorld;
    uint matIndex;
    uint renderProxyStartIndex;
    uint renderProxyCount;
    uint padding; // 显式填充 12 字节，确保 C++ (72字节) 与 HLSL 布局严格一致
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

### File: `Runtime/Renderer/RenderPath/GPUSceneRenderPath.h`
```cpp
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
        virtual void Prepare(RenderContext& context) override {};


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
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

    class ComputeShader;
    struct Payload_DispatchComputeShader
    {
        ComputeShader* csShader;
        uint32_t groupX;
        uint32_t groupY;
        uint32_t groupZ;
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
...
```cpp
    };

    struct Payload_SetBindLessMeshIB
    {
        uint32_t id;
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

### File: `Runtime/Graphics/Mesh.h`
```cpp
    };

    struct MeshBufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        // 当前数据开始位置， 可以直接绑定
        uint64_t gpuAddress = 0;
        uint64_t offset =0;
        uint64_t size = 0;
        uint32_t stride = 0;
        bool isValid = false;
        struct MeshBufferAllocation() = default;
        struct MeshBufferAllocation(IGPUBuffer* buffer, uint64_t gpuAddress, uint64_t offset, uint64_t size, uint64_t stride)
            :buffer(buffer), gpuAddress(gpuAddress), offset(offset), size(size), stride(stride)
        {
            isValid = true;
        }
    };
```
...
```cpp

        Mesh() = default;
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        bool isDynamic = true;
        virtual void OnLoadComplete() override { UploadMeshToGPU(); };
```
...
```cpp
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}
```

### File: `Runtime/Graphics/RenderTexture.h`
```cpp
namespace EngineCore
{
    class RenderTexture
    {
    public:
        RenderTexture() = default;
        RenderTexture(TextureDesc textureDesc);


        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
    };
```