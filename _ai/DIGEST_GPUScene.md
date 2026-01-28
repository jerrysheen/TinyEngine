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
- `[56]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[55]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[50]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h** *(Content Included)*
- `[48]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[42]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h** *(Content Included)*
- `[36]` **Runtime/Renderer/BatchManager.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[35]` **Runtime/Renderer/BatchManager.cpp** *(Content Included)*
- `[34]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[32]` **Runtime/Graphics/GeometryManager.cpp** *(Content Included)*
- `[31]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[30]` **Runtime/GameObject/MeshRenderer.cpp** *(Content Included)*
- `[29]` **Assets/Shader/GPUCulling.hlsl** *(Content Included)*
- `[28]` **Runtime/GameObject/MeshFilter.cpp** *(Content Included)*
- `[28]` **Runtime/GameObject/MeshRenderer.h** *(Content Included)*
- `[28]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[28]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[28]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[28]` **Runtime/Serialization/DDSTextureLoader.h**
- `[28]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Serialization/MeshLoader.h**
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
- `[19]` **Runtime/Renderer/RenderEngine.cpp**
- `[18]` **Runtime/Renderer/RenderAPI.h**
- `[16]` **Runtime/Renderer/Renderer.h**
- `[15]` **Runtime/Scene/SceneManager.cpp**
- `[14]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[13]` **Runtime/Renderer/RenderStruct.h**
- `[12]` **Runtime/Core/Game.cpp**
- `[12]` **Runtime/Core/PublicStruct.h**
- `[12]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[10]` **Runtime/Resources/ResourceManager.cpp**
- `[10]` **Runtime/Scene/BistroSceneLoader.h**
- `[10]` **Runtime/Scene/SceneManager.h**
- `[10]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[10]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[9]` **Runtime/Graphics/Material.cpp**
- `[9]` **Runtime/Renderer/RenderContext.cpp**
- `[9]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.cpp**
- `[9]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.cpp**
- `[9]` **Assets/Shader/SimpleTestShader.hlsl**
- `[9]` **Assets/Shader/StandardPBR.hlsl**
- `[8]` **Runtime/GameObject/Camera.cpp**

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

### File: `Runtime/Graphics/GPUSceneManager.cpp`
```cpp
    {
        if(sInstance != nullptr) return sInstance;
        GPUSceneManager::Create();
        return sInstance;   
    }

    void GPUSceneManager::Create()
    {
```
...
```cpp

        vector<DrawIndirectArgs> drawIndirectArgsList = BatchManager::GetInstance()->GetBatchInfo();
        UpdateRenderProxyBuffer(renderSceneData.materialDirtyList);
        UpdateAABBandPerObjectBuffer(renderSceneData.transformDirtyList, renderSceneData.materialDirtyList);

        // 重置visibilityBuffer
        vector<uint8_t> empty;
        empty.resize(4 * 10000, 0);
        visibilityBuffer->UploadBuffer(visiblityAlloc, empty.data(), empty.size());
        visibilityBuffer->Reset();
    }

    BufferAllocation GPUSceneManager::GetSinglePerMaterialData()
    {
```
...
```cpp
        copyRegionCmd.copyList = mPendingBatchCopies.data();
        copyRegionCmd.count = mPendingBatchCopies.size();
        Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
        mPendingBatchCopies.clear();
    }

    void GPUSceneManager::UpdateRenderProxyBuffer(const vector<uint32_t>& materialDirtyList)
    {
```
...
```cpp
            {
                // delete: 只处理RenderProxy相关信息。
                TryFreeRenderProxyBlock(index);
                perObjectDataBuffer[index].renderProxyStartIndex = 0;
                perObjectDataBuffer[index].renderProxyCount = 0;
                perObjectDataBuffer[index].matIndex = 0;
                perObjectDataBuffer[index].baseVertexLocation = 0;
            }
            else
            {
```
...
```cpp
            copyRegionCmd.copyList = copyAABBOP;
            copyRegionCmd.count = transformDirtyCount;
            Renderer::GetInstance()->CopyBufferRegion(copyRegionCmd);
        }


        // 更新所有dirty的PerObjectData，来自于 tranformdirty + materialDirty
        std::set<uint32_t> s(transformDirtyList.begin(), transformDirtyList.end());
        s.insert(materialDirtyList.begin(), materialDirtyList.end());
        vector<uint32_t> dirtyList = std::vector<uint32_t>(s.begin(), s.end());
        int perObjectDataCount = dirtyList.size();

        uint32_t perObjectBufferSize = perObjectDataCount * sizeof(PerObjectData);
        auto& perObjectAllocation = perFrameBatchBuffer->Allocate(perObjectBufferSize);
        CopyOp* copyPerObjectOP = perFramelinearMemoryAllocator->allocArray<CopyOp>(perObjectDataCount);
        CopyOp* currCopyPerObjectOPPtr = copyPerObjectOP;
        vector<PerObjectData> perObjectTempData;

        for (int i = 0; i < perObjectDataCount; i++)
        {
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

        virtual void Execute(RenderContext& context) override
        {
            if (!hasSetUpBuffer) 
            {
                hasSetUpBuffer = true;

                BufferDesc desc;

                desc.debugName = L"CullingParamBuffer";
                desc.memoryType = BufferMemoryType::Upload;
                desc.size = sizeof(GPUCullingParam);
                desc.stride = sizeof(GPUCullingParam);
                desc.usage = BufferUsage::ConstantBuffer;
                cullingParamBuffer = new GPUBufferAllocator(desc);
                cullingParamAlloc = cullingParamBuffer->Allocate(sizeof(Frustum));
                
                
                desc.debugName = L"IndirectDrawArgsBuffer";
                desc.memoryType = BufferMemoryType::Default;
                desc.size = sizeof(DrawIndirectArgs) * 3000;
                desc.stride = sizeof(DrawIndirectArgs);
                desc.usage = BufferUsage::StructuredBuffer;
                indirectDrawArgsBuffer = new GPUBufferAllocator(desc);
                indirectDrawArgsAlloc = indirectDrawArgsBuffer->Allocate(sizeof(DrawIndirectArgs) * 3000);
            }

            //todo:
            // 这个地方要把ResourceState切换一下
            Renderer::GetInstance()->BeginFrame();
            auto* visibilityBuffer = GPUSceneManager::GetInstance()->visibilityBuffer;
            Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);
            Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_UNORDERED_ACCESS);


            Camera* cam = SceneManager::GetInstance()->GetCurrentScene()->mainCamera;
            int gameObjectCount = SceneManager::GetInstance()->GetCurrentScene()->allObjList.size();

            GPUCullingParam cullingParam;
            cullingParam.frustum = cam->mFrustum;
            cullingParam.totalItem = gameObjectCount;
            cullingParamBuffer->UploadBuffer(cullingParamAlloc, &cullingParam, sizeof(GPUCullingParam));

            PROFILER_EVENT_BEGIN("MainThread::GPUSceneManagerTick");
            GPUSceneManager::GetInstance()->Tick();
            PROFILER_EVENT_END("MainThread::GPUSceneManagerTick");

            
            // Get Current BatchInfo:
            vector<DrawIndirectArgs> batchInfo = BatchManager::GetInstance()->GetBatchInfo();
            if (batchInfo.size() != 0) 
            {
                indirectDrawArgsBuffer->UploadBuffer(indirectDrawArgsAlloc, batchInfo.data(), batchInfo.size() * sizeof(DrawIndirectArgs));
            }

            ComputeShader* csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            csShader->SetBuffer("CullingParams", cullingParamBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_InputAABBs", GPUSceneManager::GetInstance()->allAABBBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_InputPerObjectDatas", GPUSceneManager::GetInstance()->allObjectDataBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_RenderProxies", GPUSceneManager::GetInstance()->renderProxyBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_VisibleInstanceIndices", GPUSceneManager::GetInstance()->visibilityBuffer->GetGPUBuffer());
            csShader->SetBuffer("g_IndirectDrawCallArgs", indirectDrawArgsBuffer->GetGPUBuffer());

            
            Payload_DispatchComputeShader payload;
            payload.csShader = GPUSceneManager::GetInstance()->GPUCullingShaderHandler.Get();
            payload.groupX = gameObjectCount / 64 + 1;
            payload.groupY = 1;
            payload.groupZ = 1;
            Renderer::GetInstance()->DispatchComputeShader(payload);
```
...
```cpp
            // 这个地方简单跑个绑定测试？
            Renderer::GetInstance()->SetResourceState(visibilityBuffer->GetGPUBuffer(), BufferResourceState::STATE_SHADER_RESOURCE);
            Renderer::GetInstance()->SetResourceState(indirectDrawArgsBuffer->GetGPUBuffer(), BufferResourceState::STATE_INDIRECT_ARGUMENT);

            context.Reset();
            context.camera = cam;
            Renderer::GetInstance()->Render(context);
#ifdef EDITOR
            Renderer::GetInstance()->OnDrawGUI();
#endif

            Renderer::GetInstance()->EndFrame();
        }

        bool hasSetUpBuffer = false;
        BufferAllocation cullingParamAlloc;
        GPUBufferAllocator* cullingParamBuffer;
        BufferAllocation indirectDrawArgsAlloc;
        GPUBufferAllocator* indirectDrawArgsBuffer;

    };
}
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

        void TryAddBatchCount(MeshRenderer* meshRenderer);
        void TryDecreaseBatchCount(MeshRenderer* meshRenderer);

        void TryAddBatchCount(MeshFilter* meshFilter);
        void TryDecreaseBatchCount(MeshFilter* meshFilter);
        static std::unordered_map<uint64_t, int> BatchMap;
        static std::unordered_map<uint64_t, DrawIndirectParam> drawIndirectParamMap;
        static std::unordered_map<uint64_t, DrawIndirectContext> drawIndirectContextMap;

        std::vector<RenderProxy> GetAvaliableRenderProxyList(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        static uint64_t GetBatchHash(MeshRenderer* meshRenderer, MeshFilter* meshFilter, uint32_t layer); 

        vector<DrawIndirectArgs> GetBatchInfo();
    private:
        void TryAddBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
        void TryDecreaseBatches(MeshRenderer* meshRenderer, MeshFilter* meshFilter); 
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

### File: `Runtime/Graphics/GPUSceneManager.h`
```cpp
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

        inline void SetSharedMaterial(const ResourceHandle<Material>& mat) 
        {
            mShardMatHandler = mat;
            SetUpMaterialPropertyBlock();
        }

        // return a new Material Instance;
        Material* GetOrCreateMatInstance();
        // 
        ResourceHandle<Material> GetMaterial();
        inline bool HasMaterialOverride() { return mInstanceMatHandler.IsValid(); }

        void UpdateBounds(const AABB& localBounds, const Matrix4x4& worldMatrix);
        uint32_t lastSyncTransformVersion = 0;
        bool shouldUpdateMeshRenderer = true;

        AABB worldBounds;
        uint32_t sceneRenderNodeIndex = UINT32_MAX;
        bool materialDirty = true;
		
        void TryAddtoBatchManager();

        uint32_t renderLayer = 1;
    private:
        ResourceHandle<Material> mShardMatHandler;
        ResourceHandle<Material> mInstanceMatHandler;

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