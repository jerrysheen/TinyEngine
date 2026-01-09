# Architecture Digest: GRAPHICS_RES
> Auto-generated. Focus: Runtime/Graphics, Runtime/Resources, Material, Texture, GPUBuffer, Mesh, Model, GpuScene, Vertex, Index, Layout

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 资源层是材质、纹理、模型与GPU缓冲的核心纽带。
- 提取资源描述、布局、GPU缓冲与材质接口。

## Key Files Index
- `[62]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[48]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[46]` **Runtime/Graphics/ModelData.h** *(Content Included)*
- `[44]` **Runtime/Graphics/MaterialInstance.h** *(Content Included)*
- `[42]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[37]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[37]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[37]` **Runtime/Graphics/ModelUtils.h** *(Content Included)*
- `[37]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GPUTexture.h** *(Content Included)*
- `[35]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[33]` **Runtime/GameObject/MeshRenderer.h**
- `[32]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[29]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Core/PublicStruct.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/Graphics/IGPUResource.h**
- `[21]` **Runtime/Renderer/BatchManager.h**
- `[20]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[20]` **Runtime/Renderer/RenderCommand.h**
- `[19]` **Runtime/Renderer/RenderAPI.h**
- `[18]` **Runtime/Renderer/RenderStruct.h**
- `[17]` **Runtime/Resources/ResourceManager.h**
- `[17]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[17]` **Assets/Shader/SimpleTestShader.hlsl**
- `[16]` **Runtime/Scene/Scene.h**
- `[16]` **Runtime/Resources/Asset.h**
- `[15]` **Runtime/Graphics/ComputeShader.h**
- `[15]` **Editor/Panel/EditorMainBar.h**
- `[14]` **Runtime/Graphics/Shader.h**
- `[13]` **Runtime/Scene/SceneManager.h**
- `[13]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[12]` **Runtime/Resources/Resource.h**
- `[12]` **Runtime/Resources/ResourceHandle.h**
- `[12]` **Runtime/Serialization/MetaData.h**
- `[11]` **Runtime/Serialization/MetaLoader.h**
- `[11]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[10]` **Runtime/Renderer/Renderer.h**
- `[10]` **Assets/Shader/BlitShader.hlsl**
- `[10]` **Assets/Shader/include/Core.hlsl**
- `[8]` **Runtime/Serialization/AssetSerialization.h**
- `[8]` **Assets/Shader/GPUCulling.hlsl**
- `[7]` **Runtime/Core/Profiler.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[6]` **Runtime/GameObject/Camera.h**
- `[6]` **Runtime/Renderer/RenderSorter.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[5]` **Runtime/Core/PublicEnum.h**
- `[5]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[4]` **premake5.lua**
- `[4]` **Runtime/Serialization/MetaFactory.h**
- `[4]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[4]` **Runtime/GameObject/GameObject.h**
- `[4]` **Runtime/GameObject/ComponentType.h**
- `[4]` **Runtime/Renderer/RenderUniforms.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12Buffer.h**
- `[3]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[3]` **Runtime/Renderer/RenderContext.h**

## Evidence & Implementation Details

### File: `Runtime/Graphics/MaterialLayout.h`
```cpp
namespace EngineCore
{
    struct MaterialPropertyLayout
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        ShaderVariableType type;
    };
```
...
```cpp
        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}
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
    };
```

### File: `Runtime/Graphics/ModelData.h`
```cpp
{
    ModelData* GetFullScreenQuad();
    class ModelData : public Resource
    {
    public:
        // todo: 先这么写，后续或许抽成单独Component
        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
        ModelData() = default;
        ModelData(MetaData* metaData);
        ModelData(Primitive primitiveType);
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);
    };
```

### File: `Runtime/Graphics/MaterialInstance.h`
```cpp
namespace EngineCore
{
    class MaterialInstance
    {
    public:
        MaterialInstance(const MaterialLayout& layout)
            :m_Layout(layout)
        {
            m_DataBlob.resize(m_Layout.GetSize());    
        }
        
        void SetValue(const std::string& name, void* data, uint32_t size)
        {
            uint32_t offset = m_Layout.GetPropertyOffset(name);
            memcpy(m_DataBlob.data() + offset, data, size);
        }

        void* GetData(){return m_DataBlob.data();}
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
    private:
        unordered_map<std::string, IGPUTexture*> textureMap;
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
    };
```

### File: `Runtime/Graphics/Material.h`
```cpp
namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;


        Material() = default;
        Material(MetaData* metaData);
        Material(ResourceHandle<Shader> shader);
        Material(const Material& other);
        void UploadDataToGpu();
        ~Material();


        void SetValue(const string& name, void* data, uint32_t size) 
        {
            ASSERT(matInstance != nullptr);
            matInstance->SetValue(name, data, size);
        }

        // 通用设置材质texture的接口
        void SetTexture(const string& name, IGPUTexture* texture)
        {
            ASSERT(textureData.count(name) > 0);
            if(textureHandleMap.count(name))
            {
                textureHandleMap.erase(name);
            }
            textureData[name] = texture;
        }

        // 运行时关联一个临时资源，建立一个引用， 防止资源因为0引用被销毁
        void SetTexture(const string& name, ResourceHandle<Texture> texture)
        {
            ASSERT(textureData.count(name) > 0);
            textureHandleMap[name] = texture;
            if(texture.IsValid())
            {
                textureData[name] = texture.Get()->textureBuffer;
            }
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void LoadDependency(const std::unordered_map<std::string, MetaData>& dependentMap);
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
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

        Texture(MetaData* textureMetaData);
        //inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
    };
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

### File: `Runtime/Graphics/ModelUtils.h`
```cpp
namespace EngineCore
{
    class ModelUtils
    {
    public:
        static void GetFullScreenQuad(ModelData* modelData);
    private:

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

### File: `Runtime/Graphics/GPUTexture.h`
```cpp
namespace EngineCore
{
    class GPUTexture
    {
    public:
        IGPUTexture* texture;
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