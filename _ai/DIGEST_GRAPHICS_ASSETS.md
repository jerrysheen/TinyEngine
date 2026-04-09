# Architecture Digest: GRAPHICS_ASSETS
> Auto-generated. Focus: Runtime/Graphics, Runtime/Resources, Runtime/MaterialLibrary, Runtime/MaterialLibrary/MaterialLayout.h, Runtime/MaterialLibrary/MaterialInstance.h, Runtime/MaterialLibrary/MaterialArchetypeRegistry.h, Material, MaterialLayout, MaterialInstance, MaterialArchetypeRegistry, Texture, GPUBuffer, Mesh, Vertex, Index, Layout, Bindless, GeometryManager, MeshUtils, StandardPBR, IGPUResource, GPUBufferAllocator

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
- 统一的图形资源模块，包含材质、纹理、模型、GPU缓冲、材质系统的核心纽带。
- 提取资源描述、布局、GPU缓冲、MaterialLayout/MaterialInstance/Bindless材质系统接口，Mesh/GeometryManager几何系统。

## Key Files Index
- `[100]` **Runtime/MaterialLibrary/MaterialLayout.h** *(Content Included)*
- `[80]` **Runtime/MaterialLibrary/MaterialInstance.h** *(Content Included)*
- `[79]` **Runtime/MaterialLibrary/MaterialArchetypeRegistry.h** *(Content Included)*
- `[70]` **Runtime/Graphics/MeshUtils.cpp** *(Content Included)*
- `[64]` **Runtime/MaterialLibrary/StandardPBR.h** *(Content Included)*
- `[63]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[62]` **Assets/Shader/StandardPBR_VertexPulling.hlsl** *(Content Included)*
- `[61]` **Runtime/Graphics/Material.cpp** *(Content Included)*
- `[60]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[60]` **Runtime/Graphics/GPUBufferAllocator.cpp** *(Content Included)*
- `[60]` **Runtime/Serialization/MaterialLoader.h** *(Content Included)*
- `[58]` **Runtime/Graphics/Mesh.cpp** *(Content Included)*
- `[56]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[56]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[54]` **Runtime/Graphics/GeometryManager.cpp** *(Content Included)*
- `[52]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[50]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[50]` **Runtime/MaterialLibrary/StandardPBR.cpp** *(Content Included)*
- `[46]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[44]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[41]` **Assets/Shader/StandardPBR.hlsl**
- `[38]` **Runtime/Graphics/RenderTexture.h**
- `[38]` **Runtime/Graphics/Texture.h**
- `[37]` **Runtime/GameObject/MeshRenderer.h**
- `[36]` **Runtime/Graphics/GPUTexture.h**
- `[36]` **Runtime/Scene/BistroSceneLoader.cpp**
- `[35]` **Runtime/Graphics/RenderTexture.cpp**
- `[35]` **Runtime/Graphics/Texture.cpp**
- `[34]` **Runtime/Renderer/RenderCommand.h**
- `[33]` **Runtime/Serialization/MeshLoader.h**
- `[31]` **Runtime/GameObject/MeshRenderer.cpp**
- `[29]` **Runtime/Resources/ResourceManager.cpp**
- `[29]` **Runtime/Scene/GPUScene.cpp**
- `[29]` **Runtime/Serialization/DDSTextureLoader.h**
- `[28]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[27]` **Runtime/GameObject/MeshFilter.h**
- `[27]` **Runtime/Renderer/RenderBackend.cpp**
- `[27]` **Runtime/Renderer/RenderStruct.h**
- `[25]` **Runtime/GameObject/MeshFilter.cpp**
- `[24]` **Runtime/Renderer/BatchManager.h**
- `[24]` **Runtime/Resources/ResourceManager.h**
- `[23]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.cpp**
- `[22]` **Runtime/Renderer/RenderAPI.h**
- `[22]` **Runtime/Serialization/TextureLoader.h**
- `[22]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[22]` **Runtime/Platforms/D3D12/D3D12RootSignature.cpp**
- `[21]` **Runtime/Core/Game.cpp**
- `[21]` **Runtime/Renderer/RenderBackend.h**
- `[21]` **Runtime/Resources/AssetTypeTraits.h**
- `[21]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.cpp**
- `[21]` **Assets/Shader/SimpleTestShader.hlsl**
- `[20]` **Runtime/Entry.cpp**
- `[20]` **Runtime/Graphics/PerFrameBufferRing.h**
- `[20]` **Runtime/Renderer/BatchManager.cpp**
- `[19]` **Runtime/Core/PublicStruct.h**
- `[19]` **Runtime/Scene/BistroSceneLoader.h**
- `[19]` **Runtime/Scene/GPUScene.h**
- `[19]` **Assets/Shader/include/Core.hlsl**
- `[17]` **Runtime/Scene/SceneManager.cpp**
- `[17]` **Runtime/Serialization/SceneLoader.h**

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
            ASSERT(textureToBlockIndexMap.count(name) <= 0);
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

        inline void SetValueAtOffset(uint32_t offset, void* data, uint32_t size)
        {
            memcpy(m_DataBlob.data() + offset, data, size);
        }

        std::vector<uint8_t> GetInstanceData(){ return m_DataBlob; }
        inline void SetInstanceData(const std::vector<uint8_t> data){ m_DataBlob = data; }
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
        inline void SetLayout(const MaterialLayout& layout){ m_Layout = layout;};
        
    private:
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
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
        materialLayout.AddProp("NormalTextureID", ShaderVariableType::FLOAT, 4);
        materialLayout.AddProp("MetallicTextureID", ShaderVariableType::FLOAT, 4);
        materialLayout.AddProp("EmissiveTextureID", ShaderVariableType::FLOAT, 4);
        
        materialLayout.AddTextureToBlockOffset("DiffuseTexture", 48);
        materialLayout.AddTextureToBlockOffset("NormalTexture", 52);
        materialLayout.AddTextureToBlockOffset("MetallicTexture", 56);
        materialLayout.AddTextureToBlockOffset("EmissiveTexture", 60);

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
        virtual void UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size) override;
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

### File: `Assets/Shader/StandardPBR_VertexPulling.hlsl`
```hlsl

// 纹理资源
Texture2D g_Textures[1024] : register(t0, space0);


// 采样器
SamplerState LinearSampler : register(s0, space0);
SamplerState PointSampler : register(s1, space0);
SamplerState AnisotropicSampler : register(s2,space0);
SamplerComparisonState ShadowSampler : register(s3, space0);

// 顶点着色器输入
struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Tangent : TANGENT;
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
                //mat->SetTexture(texName, ResourceHandle<Texture>(ResourceManager::GetInstance()->mDefaultTexture->GetAssetID()));
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
        virtual void UploadBufferStaged(const BufferAllocation& alloc, void* data, uint32_t size) = 0;
        virtual IGPUBuffer* GetGPUBuffer() = 0;
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
        std::vector<DescriptorHandle> uavHandles;  
    };
```

### File: `Runtime/Graphics/Material.h`
```cpp
    };

    class Material : public Resource
    {
    public:
        bool isDirty = true;
        bool isBindLessMaterial = false;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        float transmissionFactor = 0.0f;
        string archyTypeName = "";
        std::unique_ptr<MaterialInstance> matInstance;
        ResourceHandle<Shader> mShader;
        unordered_map<string, IGPUTexture*> textureData;
        unordered_map<std::string, ResourceHandle<Texture>> textureHandleMap;
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;

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
        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState; };
        
        virtual void OnLoadComplete() override;

    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
        void FlushBindlessIndicesToInstance();
    };
}
```