# Architecture Digest: GPUScene
> Auto-generated. Focus: GPUSceneRenderPath, GPUSceneManager, GeometryManager, Mesh, Texture, IGPUResource, GPUBufferAllocator, ComputeShader, D3D12RenderAPI, GPUSceneRenderPass, BatchManager, GPUCulling, Indirect

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- GPUScene关注GPU驱动裁剪、批处理与间接绘制数据结构。
- 提取GPUSceneManager、BatchManager与相关Buffer布局。

## Key Files Index
- `[51]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[48]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[42]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[39]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.cpp** *(Content Included)*
- `[36]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[35]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[34]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[32]` **Runtime/Graphics/GeometryManager.cpp** *(Content Included)*
- `[32]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[31]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[29]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[28]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[28]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[28]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[28]` **Runtime/Platforms/D3D12/D3D12Texture.h** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshFilter.h** *(Content Included)*
- `[27]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
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
- `[24]` **Runtime/Renderer/RenderCommand.h**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[22]` **Runtime/Renderer/Renderer.cpp**
- `[22]` **Runtime/Serialization/TextureLoader.h**
- `[20]` **Runtime/Entry.cpp**
- `[18]` **Runtime/Renderer/RenderAPI.h**
- `[16]` **Runtime/Renderer/Renderer.h**
- `[15]` **Runtime/Scene/SceneManager.cpp**
- `[14]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[13]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Core/PublicStruct.h**
- `[12]` **Runtime/Renderer/FrameContext.cpp**
- `[12]` **Runtime/Renderer/RenderEngine.cpp**
- `[11]` **Runtime/Scene/GPUScene.h**
- `[10]` **Runtime/Resources/ResourceManager.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.h**
- `[10]` **Runtime/Scene/CPUScene.cpp**
- `[10]` **Runtime/Scene/SceneManager.h**
- `[10]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[10]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[9]` **Runtime/Resources/ResourceManager.h**
- `[9]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[9]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[9]` **Assets/Shader/SimpleTestShader.hlsl**
- `[9]` **Assets/Shader/StandardPBR.hlsl**

## Evidence & Implementation Details

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
        {
            DescriptorHandle tableHandle = 
                D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(textureInfo.size());
            
            for (int i = 0; i < textureInfo.size(); i++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE dest = {
                    tableHandle.cpuHandle + i * mCbvSrvUavDescriptorSize
                };
                
                ASSERT(mat->textureData.count(textureInfo[i].resourceName) > 0);
                D3D12Texture* texture = static_cast<D3D12Texture*>(mat->textureData[textureInfo[i].resourceName]);
                DescriptorHandle texSRVHandle = texture->srvHandle;
                ASSERT(texSRVHandle.cpuHandle != UINT64_MAX);
                D3D12_CPU_DESCRIPTOR_HANDLE srcHandle;
                srcHandle.ptr = texSRVHandle.cpuHandle;
                md3dDevice->CopyDescriptorsSimple(
                    1, dest, 
                    srcHandle,
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );
            }
```

### File: `Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp`
```cpp
                Renderer::GetInstance()->SetBindlessMat(mat);
                // bind mesh vertexbuffer and indexbuffer.
                Renderer::GetInstance()->SetMeshData(mesh);
                Payload_DrawIndirect indirectPayload;
                // temp:
                GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
                ASSERT(indirectDrawArgsBuffer != nullptr);
                indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                Renderer::GetInstance()->DrawIndirect(indirectPayload);
            }
        }
        else
        {
```
...
```cpp
                    Renderer::GetInstance()->SetRenderState(mat, mRenderPassInfo);
                    Renderer::GetInstance()->SetBindlessMat(mat);
                    Renderer::GetInstance()->SetBindLessMeshIB(0);
                }

                Payload_DrawIndirect indirectPayload;
                // temp:
                GPUBufferAllocator* indirectDrawArgsBuffer = RenderEngine::gpuSceneRenderPath.indirectDrawArgsBuffer;
                ASSERT(indirectDrawArgsBuffer != nullptr);
                indirectPayload.indirectArgsBuffer = indirectDrawArgsBuffer->GetGPUBuffer();
                indirectPayload.count = 1;
                indirectPayload.startIndex = batchID;
                indirectPayload.startIndexInInstanceDataBuffer = stratIndex;
                Renderer::GetInstance()->DrawIndirect(indirectPayload);
            }
        }

    }

    void GPUSceneRenderPass::Filter(const RenderContext &context)
    {
```

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


        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
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

### File: `Runtime/Graphics/Texture.h`
```cpp
namespace EngineCore
{
    class Texture : public Resource
    {
    public:
        Texture() = default;
        Texture(const string& textureID);

        //inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
        virtual void OnLoadComplete() override;
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
        std::vector<uint8_t> cpuData;
    };
```

### File: `Runtime/Serialization/DDSTextureLoader.h`
```cpp
  HEADER      124
  HEADER_DX10* 20	(https://msdn.microsoft.com/en-us/library/bb943983(v=vs.85).aspx)
  PIXELS      fseek(f, 0, SEEK_END); (ftell(f) - 128) - (fourCC == "DX10" ? 17 or 20 : 0)
* the link tells you that this section isn't written unless its a DX10 file
Supports DXT1, DXT3, DXT5.
The problem with supporting DX10 is you need to know what it is used for and how opengl would use it.
File Byte Order:
typedef unsigned int DWORD;           // 32bits little endian
  type   index    attribute           // description
///////////////////////////////////////////////////////////////////////////////////////////////
  DWORD  0        file_code;          //. always `DDS `, or 0x20534444
  DWORD  4        size;               //. size of the header, always 124 (includes PIXELFORMAT)
  DWORD  8        flags;              //. bitflags that tells you if data is present in the file
                                      //      CAPS         0x1
                                      //      HEIGHT       0x2
                                      //      WIDTH        0x4
                                      //      PITCH        0x8
                                      //      PIXELFORMAT  0x1000
                                      //      MIPMAPCOUNT  0x20000
                                      //      LINEARSIZE   0x80000
                                      //      DEPTH        0x800000
  DWORD  12       height;             //. height of the base image (biggest mipmap)
  DWORD  16       width;              //. width of the base image (biggest mipmap)
  DWORD  20       pitchOrLinearSize;  //. bytes per scan line in an uncompressed texture, or bytes in the top level texture for a compressed texture
                                      //     D3DX11.lib and other similar libraries unreliably or inconsistently provide the pitch, convert with
                                      //     DX* && BC*: max( 1, ((width+3)/4) ) * block-size
                                      //     *8*8_*8*8 && UYVY && YUY2: ((width+1) >> 1) * 4
                                      //     (width * bits-per-pixel + 7)/8 (divide by 8 for byte alignment, whatever that means)
  DWORD  24       depth;              //. Depth of a volume texture (in pixels), garbage if no volume data
  DWORD  28       mipMapCount;        //. number of mipmaps, garbage if no pixel data
  DWORD  32       reserved1[11];      //. unused
  DWORD  76       Size;               //. size of the following 32 bytes (PIXELFORMAT)
  DWORD  80       Flags;              //. bitflags that tells you if data is present in the file for following 28 bytes
                                      //      ALPHAPIXELS  0x1
                                      //      ALPHA        0x2
                                      //      FOURCC       0x4
                                      //      RGB          0x40
                                      //      YUV          0x200
                                      //      LUMINANCE    0x20000
  DWORD  84       FourCC;             //. File format: DXT1, DXT2, DXT3, DXT4, DXT5, DX10. 
  DWORD  88       RGBBitCount;        //. Bits per pixel
  DWORD  92       RBitMask;           //. Bit mask for R channel
  DWORD  96       GBitMask;           //. Bit mask for G channel
  DWORD  100      BBitMask;           //. Bit mask for B channel
  DWORD  104      ABitMask;           //. Bit mask for A channel
  DWORD  108      caps;               //. 0x1000 for a texture w/o mipmaps
                                      //      0x401008 for a texture w/ mipmaps
                                      //      0x1008 for a cube map
  DWORD  112      caps2;              //. bitflags that tells you if data is present in the file
                                      //      CUBEMAP           0x200     Required for a cube map.
                                      //      CUBEMAP_POSITIVEX 0x400     Required when these surfaces are stored in a cube map.
                                      //      CUBEMAP_NEGATIVEX 0x800     ^
                                      //      CUBEMAP_POSITIVEY 0x1000    ^
                                      //      CUBEMAP_NEGATIVEY 0x2000    ^
                                      //      CUBEMAP_POSITIVEZ 0x4000    ^
                                      //      CUBEMAP_NEGATIVEZ 0x8000    ^
                                      //      VOLUME            0x200000  Required for a volume texture.
  DWORD  114      caps3;              //. unused
  DWORD  116      caps4;              //. unused
  DWORD  120      reserved2;          //. unused
*/

namespace EngineCore{
    struct DDSHeader {
        uint32_t magic;              // 'DDS ' (0x20534444)
        uint32_t fileSize;               // 124
        uint32_t flags;
        uint32_t height;
        uint32_t width;
        uint32_t pitchOrLinearSize;
        uint32_t depth;
        uint32_t mipMapCount;
        uint32_t reserved1[11];
        uint32_t size;           // 应该是32
        uint32_t flagsData;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
```
...
```cpp
    };

    struct DDS_HEADER_DXT10 {
        uint32_t dxgiFormat;      // DXGI_FORMAT枚举值
        uint32_t resourceDimension; // D3D11_RESOURCE_DIMENSION
        uint32_t miscFlag;        // D3D11_RESOURCE_MISC_FLAG
        uint32_t arraySize;       // 数组大小
        uint32_t miscFlags2;      // 额外标志
    };
```
...
```cpp
    };

    class DDSTextureLoader : public IResourceLoader
    {
    public:
        virtual ~DDSTextureLoader() = default;
        virtual LoadResult Load(const std::string& relativePath) override
        {
            LoadResult result;
            std::string path = PathSettings::ResolveAssetPath(relativePath);
    
            Texture* tex = new Texture();
            tex->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            tex->SetAssetID(AssetIDGenerator::NewFromFile(path));
            
            DDSLoadResult ddsResult = LoadDDSFromFile(relativePath);
            tex->textureDesc.format = ddsResult.format;
            tex->textureDesc.width = ddsResult.width;
            tex->textureDesc.height = ddsResult.height;
            tex->textureDesc.dimension = TextureDimension::TEXTURE2D;
            tex->textureDesc.texUsage = TextureUsage::ShaderResource;
            tex->textureDesc.mipCount = ddsResult.mipMapCount;
            tex->cpuData = ddsResult.pixelData;
            // 计算mip Count
            uint32_t offset = 0;
            tex->textureDesc.mipOffset[0] = 0;  // 第 0 级从 0 开始
            int width = ddsResult.width;
            int height = ddsResult.height;
            // ++i 和 i++在循环体中区别不大， 因为循环跑完才会走++操作，
            // 不过针对迭代器，++i更好， 因为i++相当于要返回一个原值，并且在原来的迭代器上叠加
            for (uint32_t i = 0; i < ddsResult.mipMapCount; ++i)
            {
                // 当前 mip level 的尺寸
                uint32_t mipWidth = std::max(1, width >> i);
                uint32_t mipHeight = std::max(1, height >> i);
                
                // 计算当前 mip level 的字节大小
                uint32_t mipSize = CalculateDXTMipSize(mipWidth, mipHeight, ddsResult.blockSize);
                
                if (i > 0) {
                    tex->textureDesc.mipOffset[i] = offset;  // 记录当前 mip 的 offset
                }
                
                offset += mipSize;  // 累加到下一个 mip level
            }

            result.resource = tex;
            return result;
        }
    
        std::vector<uint8_t> LoadMipData(const std::string& realativePath, int mipCount){}
    
    
        int CalculateDXTMipSize(uint32_t width, uint32_t height, uint32_t blockSize)
        {
            //DXT1 (BC1)：每 4×4 块占 8 字节
            //DXT3 (BC2)：每 4×4 块占 16 字节
            //DXT5 (BC3)：每 4×4 块占 16 字节
            uint32_t blockWidth = (width + 3) / 4;
            uint32_t blockHeight = (height + 3) / 4;
            return blockWidth * blockHeight * blockSize;
        }
    
    private:
    
    
        DDSLoadResult LoadDDSFromFile(const std::string& relativePath)
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream file(path, std::ios::binary);
            ASSERT(file.is_open());
    
            file.seekg(0, std::ios::end);      // 先移动到文件末尾
            std::streamsize fileSize = file.tellg();  // 获取文件大小
            file.seekg(0, std::ios::beg);      // 再移回文件开始
    
            ASSERT(fileSize > sizeof(DDSHeader));
    
            DDSHeader header;
            file.read(reinterpret_cast<char*>(&header), sizeof(DDSHeader));
```
...
```cpp
            if(header.magic != 0x20534444 || header.fileSize != 124)
            {
                ASSERT(false);
            }
    
            DDSLoadResult result;
            result.width = header.width;
            result.height = header.height;
            result.mipMapCount = header.mipMapCount > 0 ? header.mipMapCount : 1;
        
            // 7. 判断压缩格式 (DXT1/DXT3/DXT5)
            uint32_t fourCC = header.fourCC;
            
            // 提取fourCC字符串来判断
            char fourCCStr[5] = {0};
```
...
```cpp
                result.blockSize = 8;
            }
            else if (std::memcmp(fourCCStr, "DXT3", 4) == 0) {
                result.format = TextureFormat::DXT3;  // DXT3
                result.blockSize = 16;
            }
```

### File: `Runtime/Platforms/D3D12/D3D12Texture.h`
```cpp
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
```

### File: `Runtime/GameObject/MeshFilter.h`
```cpp
namespace EngineCore
{
    class MeshFilter : public Component
    {
        class GameObejct;
    public:
        MeshFilter() = default;
        MeshFilter(GameObject* gamObject);

        virtual ~MeshFilter() override;
        static ComponentType GetStaticType() { return ComponentType::MeshFilter; };
        virtual ComponentType GetType() const override{ return ComponentType::MeshFilter; };
        void OnLoadResourceFinished();
    public:
        ResourceHandle<Mesh> mMeshHandle;
        
        virtual const char* GetScriptName() const override { return "MeshFilter"; }

        uint32_t GetHash()
        {
            return mMeshHandle->GetInstanceID();
        }
    private:
        uint32_t hash;
    };
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