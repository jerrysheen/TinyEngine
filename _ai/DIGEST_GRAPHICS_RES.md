# Architecture Digest: GRAPHICS_RES
> Auto-generated. Focus: Runtime/Graphics, Runtime/Resources, Runtime/MaterialLibrary, Material, MaterialLayout, MaterialInstance, MaterialArchetype, MaterialArchytype, Texture, GPUBuffer, Mesh, Model, GPUScene, Vertex, Index, Layout, Bindless

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 资源层是材质、纹理、模型与GPU缓冲的核心纽带。
- 提取资源描述、布局、GPU缓冲与材质接口。

## Key Files Index
- `[90]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[69]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[69]` **Runtime/MaterialLibrary/MaterialInstance.h** *(Content Included)*
- `[67]` **Runtime/Graphics/GPUSceneManager.cpp** *(Content Included)*
- `[56]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[55]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[55]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[52]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[52]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp** *(Content Included)*
- `[48]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[47]` **Runtime/Graphics/MeshUtils.cpp** *(Content Included)*
- `[45]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[44]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[42]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[38]` **Runtime/MaterialLibrary/StandardPBR.h** *(Content Included)*
- `[37]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[37]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[37]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[36]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[36]` **Runtime/Scene/BistroSceneLoader.cpp** *(Content Included)*
- `[35]` **Runtime/Graphics/GPUBufferAllocator.cpp**
- `[35]` **Runtime/Graphics/GPUTexture.h**
- `[35]` **Runtime/Graphics/IGPUBufferAllocator.h**
- `[35]` **Runtime/Graphics/RenderTexture.cpp**
- `[35]` **Runtime/Graphics/Texture.cpp**
- `[33]` **Runtime/GameObject/MeshRenderer.cpp**
- `[33]` **Runtime/GameObject/MeshRenderer.h**
- `[33]` **Runtime/Serialization/MeshLoader.h**
- `[32]` **Runtime/Graphics/GeometryManager.h**
- `[32]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[30]` **Runtime/Renderer/RenderCommand.h**
- `[28]` **Runtime/MaterialLibrary/StandardPBR.cpp**
- `[28]` **Runtime/Serialization/DDSTextureLoader.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Graphics/GeometryManager.cpp**
- `[27]` **Runtime/Resources/ResourceManager.cpp**
- `[27]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[26]` **Runtime/Graphics/IGPUResource.h**
- `[25]` **Runtime/GameObject/MeshFilter.cpp**
- `[25]` **Runtime/Renderer/RenderStruct.h**
- `[24]` **Runtime/Scene/BistroSceneLoader.h**
- `[24]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[22]` **Runtime/Renderer/BatchManager.h**
- `[22]` **Runtime/Renderer/Renderer.cpp**
- `[22]` **Runtime/Resources/ResourceManager.h**
- `[22]` **Runtime/Serialization/TextureLoader.h**
- `[21]` **Runtime/Renderer/RenderAPI.h**
- `[21]` **Runtime/Resources/AssetTypeTraits.h**
- `[21]` **Assets/Shader/SimpleTestShader.hlsl**
- `[21]` **Assets/Shader/StandardPBR.hlsl**
- `[20]` **Runtime/Entry.cpp**
- `[20]` **Runtime/Renderer/Renderer.h**
- `[20]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.cpp**
- `[20]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[20]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[19]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[18]` **Assets/Shader/include/Core.hlsl**
- `[17]` **Runtime/Core/PublicStruct.h**
- `[17]` **Runtime/Renderer/BatchManager.cpp**
- `[17]` **Runtime/Renderer/RenderContext.cpp**

## Evidence & Implementation Details

### File: `Runtime/MaterialLibrary/MaterialLayout.h`
```cpp
{
    // 用来存
    struct TextureAssetInfo
    {
        // 这个地方直接记录
        char name[64];
        uint64_t ASSETID;
    };
```
...
```cpp
    };

    class MaterialLayout
    {
    public:

        void AddProp(const std::string& name, ShaderVariableType type, uint32_t size)
        {
            MaterialPropertyLayout prop;
            prop.name = name;
            prop.type = type;
            prop.size = size;
            prop.offset = m_TotalSize;
            m_PropertyLayout[name] = prop;
            
            m_TotalSize += size;
        }

        void AddTextureToBlockOffset(const std::string& name, uint32_t offset)
        {
            ASSERT(textureToBlockIndexMap.count(name) < 0);
            textureToBlockIndexMap[name] = offset;
            
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}

        std::unordered_map<std::string, MaterialPropertyLayout> m_PropertyLayout;
        std::unordered_map<std::string, uint32_t> textureToBlockIndexMap;
    private:
        uint32_t m_TotalSize = 0;
    };
```

### File: `Runtime/MaterialLibrary/MaterialArchetypeRegistry.h`
```cpp
namespace EngineCore
{
    class MaterialArchetypeRegistry
    {
    public:
        static MaterialArchetypeRegistry& GetInstance()
        {
            static MaterialArchetypeRegistry instance;
            return instance;
        }

        bool RegisMaterial(const std::string name, const MaterialLayout& layout)
        {
            layoutMap[name] = layout;
            return true;
        }

        MaterialLayout GetArchytypeLayout(const std::string& name)
        {
            if (layoutMap.count(name) > 0) 
            {
                return layoutMap[name];
            }
            return MaterialLayout();
        }

        
        std::unordered_map<std::string, MaterialLayout> layoutMap; 
    };
```

### File: `Runtime/MaterialLibrary/MaterialInstance.h`
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

        std::vector<uint8_t> GetInstanceData(){ return m_DataBlob; }
        inline void SetInstanceData(const std::vector<uint8_t>& data){ m_DataBlob = data; }
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
        inline void SetLayout(const MaterialLayout& layout){ m_Layout = layout;};
        
    private:
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
    };
```

### File: `Runtime/Serialization/MaterialLoader.h`
```cpp
namespace EngineCore
{
    struct alignas(16) MetaMaterialHeader
    {
        bool     enable;
        uint64_t shaderID;
    };
```
...
```cpp
    };

    struct alignas(16) MetaTextureToBindlessBlockIndex
    {
        char name[50];
        uint32_t offset;
    };
```
...
```cpp
            in.seekg(sizeof(AssetHeader));
            bool isBindless = false;
            StreamHelper::Read(in, isBindless);

            std::string archyTypeName;
            StreamHelper::ReadString(in, archyTypeName);
            if (archyTypeName.empty())
            {
                archyTypeName = "StandardPBR";
            }
```
...
```cpp

            std::vector<MetatextureDependency> textureDependencyList;
            StreamHelper::ReadVector(in, textureDependencyList);
            for(auto& textureDependency : textureDependencyList)
            {
                std::string texName = std::string(textureDependency.name);
                mat->textureData[texName] = nullptr;
                result.dependencyList.emplace_back
                    (
                        textureDependency.ASSETID, 
                        AssetType::Texture2D,
                        [=](){
                            ResourceHandle<Texture> texID(textureDependency.ASSETID);
                            mat->SetTexture(texName, texID);
                        }
                    );
            }
```
...
```cpp

            std::vector<uint8_t> materialInstanceData;
            StreamHelper::ReadVector(in, materialInstanceData);
            mat->matInstance->SetInstanceData(materialInstanceData);

            if(mat->isBindLessMaterial)
            {
                std::vector<MetaTextureToBindlessBlockIndex> textureToBinlessOffsetList;
                StreamHelper::ReadVector(in, textureToBinlessOffsetList);
                for(auto& textureToBindLessBlockIndex : textureToBinlessOffsetList)
                {
                    mat->matInstance->GetLayout().textureToBlockIndexMap[textureToBindLessBlockIndex.name] = textureToBindLessBlockIndex.offset; 
                }
            }
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

### File: `Runtime/Graphics/Material.h`
```cpp
namespace EngineCore
{
    class Material : public Resource
    {
    public:
        bool isDirty = true;
        bool isBindLessMaterial = false;
        string archyTypeName = "";
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;
        
        //MaterialData m_MaterialData;
        //void LoadFromMaterialData(const MaterialData& data);

        Material() = default;
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

        // only for serialization
        void SetTexture(const string& name, uint64_t asset)
        {
            ResourceHandle<Texture> texHandle;
            texHandle.mAssetID = AssetID(asset);
            textureHandleMap[name] = texHandle;
        }

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
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

### File: `Runtime/MaterialLibrary/StandardPBR.h`
```cpp
#include "Core/PublicEnum.h"

namespace Mat::StandardPBR
{
    using EngineCore::MaterialLayout;
    using EngineCore::ShaderVariableType;


    inline static const string GetArchetypeName()
    {
        return "StandardPBR";
    }

    inline MaterialLayout GetMaterialLayout()
    {   
        MaterialLayout materialLayout;
        materialLayout.AddProp("DiffuseColor", ShaderVariableType::VECTOR4, 16);
        // float4 SpecularColor
        materialLayout.AddProp("SpecularColor", ShaderVariableType::VECTOR4, 16);
        // float Roughness
        materialLayout.AddProp("Roughness", ShaderVariableType::FLOAT, 4);
        // float Metallic
        materialLayout.AddProp("Metallic", ShaderVariableType::FLOAT, 4);
        // float2 TilingFactor (8 bytes)
        materialLayout.AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
        
        materialLayout.AddProp("DiffuseTextureID", ShaderVariableType::FLOAT, 4);
        
        materialLayout.AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);
        
        return materialLayout;
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