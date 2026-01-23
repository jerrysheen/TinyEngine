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
- `[64]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[52]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[48]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[44]` **Runtime/Graphics/MaterialInstance.h** *(Content Included)*
- `[42]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[38]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[37]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[37]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[37]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[36]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[35]` **Runtime/Graphics/GPUTexture.h** *(Content Included)*
- `[35]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[33]` **Runtime/GameObject/MeshRenderer.h**
- `[33]` **Runtime/Serialization/MeshLoader.h**
- `[32]` **Runtime/Graphics/GeometryManager.h**
- `[32]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[25]` **Runtime/Renderer/RenderCommand.h**
- `[24]` **Runtime/Renderer/RenderStruct.h**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[23]` **Runtime/Graphics/IGPUResource.h**
- `[22]` **Runtime/Renderer/BatchManager.h**
- `[22]` **Runtime/Serialization/TextureLoader.h**
- `[21]` **Runtime/Resources/AssetTypeTraits.h**
- `[18]` **Assets/Shader/include/Core.hlsl**
- `[17]` **Runtime/Core/PublicStruct.h**
- `[17]` **Runtime/Renderer/RenderAPI.h**
- `[17]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[17]` **Assets/Shader/SimpleTestShader.hlsl**
- `[17]` **Assets/Shader/StandardPBR.hlsl**
- `[16]` **Runtime/Resources/Asset.h**
- `[16]` **Runtime/Resources/ResourceManager.h**
- `[16]` **Runtime/Scene/Scene.h**
- `[16]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[15]` **Runtime/Graphics/ComputeShader.h**
- `[15]` **Runtime/Scene/BistroSceneLoader.h**
- `[15]` **Editor/Panel/EditorMainBar.h**
- `[14]` **Runtime/Graphics/Shader.h**
- `[14]` **Runtime/Renderer/Renderer.h**
- `[14]` **Runtime/Serialization/SceneLoader.h**
- `[13]` **Runtime/Scene/SceneManager.h**
- `[13]` **Runtime/Serialization/BaseTypeSerialization.h**
- `[12]` **Runtime/Resources/AssetRegistry.h**
- `[12]` **Runtime/Resources/IResourceLoader.h**
- `[12]` **Runtime/Resources/Resource.h**
- `[12]` **Runtime/Resources/ResourceHandle.h**
- `[12]` **Runtime/Serialization/MetaLoader.h**
- `[11]` **Runtime/Serialization/MetaData.h**
- `[10]` **Assets/Shader/BlitShader.hlsl**
- `[8]` **Runtime/Renderer/RenderSorter.h**
- `[8]` **Runtime/Serialization/AssetSerialization.h**
- `[8]` **Assets/Shader/GPUCulling.hlsl**
- `[7]` **Runtime/Core/Profiler.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[6]` **Runtime/GameObject/Camera.h**
- `[6]` **Runtime/Renderer/RenderEngine.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[5]` **Runtime/Core/PublicEnum.h**
- `[5]` **Runtime/Renderer/RenderUniforms.h**

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
            AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
            
            AddProp("DiffuseTextureIndex", ShaderVariableType::FLOAT, 4);
            
            AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);

            // 此时 offset = 16+16+4+4+8 = 48 bytes
            // 还需要补齐到 16 字节对齐吗？HLSL cbuffer 是 16 字节对齐的
            // 目前 48 刚好是 16 的倍数，完美。
            
            layout.m_TotalSize = currentOffset;
            return layout;
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
```

### File: `Runtime/Graphics/Mesh.h`
```cpp
{
    // 用来描述model Input 或者 shader reflection input
    struct InputLayout
    {
        VertexAttribute type;
        int size;
        int dimension;
        int stride;
        int offset;
        InputLayout(VertexAttribute _type, int _size, int _dimension, int _stride, int _offset)
        {
            type = _type; size = _size; dimension = _dimension; stride = _stride; offset = _offset;
        };
        InputLayout() = default;
        InputLayout(VertexAttribute type) : type(type) {};
    };
```
...
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
        Mesh(MetaData* metaData);
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

### File: `Assets/Shader/StandardPBR_VertexPulling.hlsl`
```hlsl

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
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

        Texture(MetaData* textureMetaData);
        //inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
    };
```

### File: `Runtime/Graphics/MeshUtils.h`
```cpp
namespace EngineCore
{
    class MeshUtils
    {
    public:
        static void GetFullScreenQuad(Mesh* modelData);
    private:

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