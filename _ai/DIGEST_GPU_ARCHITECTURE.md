# Architecture Digest: GPU_ARCHITECTURE
> Auto-generated. Focus: Runtime/Platforms/D3D12, Runtime/Graphics, Runtime/Serialization

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Key Files Index
- `[12]` **Runtime/Graphics/ComputeShader.h** *(Content Included)*
- `[12]` **Runtime/Graphics/GeometryManager.h** *(Content Included)*
- `[12]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[12]` **Runtime/Graphics/GPUSceneManager.h** *(Content Included)*
- `[12]` **Runtime/Graphics/GPUTexture.h** *(Content Included)*
- `[12]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[12]` **Runtime/Graphics/IGPUResource.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Material.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MaterialInstance.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[12]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Shader.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[12]` **Runtime/Serialization/AssetHeader.h** *(Content Included)*
- `[12]` **Runtime/Serialization/AssetSerialization.h** *(Content Included)*
- `[12]` **Runtime/Serialization/BaseTypeSerialization.h** *(Content Included)*
- `[12]` **Runtime/Serialization/ComponentFactory.h** *(Content Included)*
- `[12]` **Runtime/Serialization/JsonSerializer.h** *(Content Included)*
- `[12]` **Runtime/Serialization/MeshLoader.h**
- `[12]` **Runtime/Serialization/MetaData.h**
- `[12]` **Runtime/Serialization/MetaFactory.h**
- `[12]` **Runtime/Serialization/MetaLoader.h**
- `[12]` **Runtime/Serialization/SceneLoader.h**
- `[12]` **Runtime/Serialization/StreamHelper.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12Buffer.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12PSO.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12RootSignature.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12ShaderUtils.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[12]` **Runtime/Platforms/D3D12/D3D12Texture.h**
- `[12]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[12]` **Runtime/Platforms/D3D12/d3dx12.h**
- `[4]` **premake5.lua**
- `[2]` **Editor/EditorGUIManager.h**
- `[2]` **Editor/EditorSettings.h**
- `[2]` **Runtime/CoreAssert.h**
- `[2]` **Runtime/EngineCore.h**
- `[2]` **Runtime/PreCompiledHeader.h**
- `[2]` **Runtime/Core/Game.h**
- `[2]` **Runtime/Core/InstanceID.h**
- `[2]` **Runtime/Core/Object.h**
- `[2]` **Runtime/Core/Profiler.h**
- `[2]` **Runtime/Core/PublicEnum.h**
- `[2]` **Runtime/Core/PublicStruct.h**
- `[2]` **Runtime/Core/ThreadSafeQueue.h**
- `[2]` **Runtime/GameObject/Camera.h**
- `[2]` **Runtime/GameObject/Component.h**
- `[2]` **Runtime/GameObject/ComponentType.h**
- `[2]` **Runtime/GameObject/GameObject.h**
- `[2]` **Runtime/GameObject/MeshFilter.h**
- `[2]` **Runtime/GameObject/MeshRenderer.h**
- `[2]` **Runtime/GameObject/MonoBehaviour.h**
- `[2]` **Runtime/GameObject/Transform.h**
- `[2]` **Runtime/Managers/Manager.h**

## Evidence & Implementation Details

### File: `Runtime/Graphics/ComputeShader.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    class ComputeShader : public Resource
    {
    public:
        IGPUBuffer* GetBufferResource(const std::string& slotName)
        {
            ASSERT(resourceMap.count(slotName) > 0);
            return resourceMap[slotName];
        }

        ComputeShader(const string& path);
        ShaderReflectionInfo mShaderReflectionInfo;
        std::unordered_map<std::string, IGPUBuffer*> resourceMap;
        void SetBuffer(const std::string& name, IGPUBuffer* buffer);
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

### File: `Runtime/Graphics/GPUBufferAllocator.h`
```cpp
#include <mutex>

namespace EngineCore
{
    // A unified GPU buffer allocator.
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
}
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

### File: `Runtime/Graphics/GPUTexture.h`
```cpp
#include "IGPUResource.h"

namespace EngineCore
{
    class GPUTexture
    {
    public:
        IGPUTexture* texture;
    };

};
```

### File: `Runtime/Graphics/IGPUBufferAllocator.h`
```cpp
#include "Renderer/RenderStruct.h"

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
}
```

### File: `Runtime/Graphics/IGPUResource.h`
```cpp
#include <iostream>

namespace EngineCore
{
    enum class TextureDimension
    {
        TEXTURE2D,
        TEXTURE3D,
        TEXTURECUBE,
        TEXTURE2DARRAY
    };

    enum class TextureFormat
    {
        R8G8B8A8,
        D24S8,
        EMPTY,
    };


    enum class TextureUsage
    {
        ShaderResource,
        RenderTarget,
        DepthStencil,
        UnorderedAccess   // 预留，现在反正没用处
    };

    struct TextureDesc
    {
        std::string name;
        int width;
        int height;
        TextureFormat format;
        TextureDimension dimension;
        TextureUsage texUsage;
    };


    enum class BufferMemoryType
    {
        Default,
        Upload,
        ReadBack // 注意这个buffer表示CPU可读，gpu可写
    };

    enum class BufferUsage
    {
        ConstantBuffer,
        Vertex,
        Index,
        StructuredBuffer,
        ByteAddressBuffer,
        IndirectArgument
    };

    enum class BufferResourceState 
    {
        STATE_COMMON = 0,
        STATE_UNORDERED_ACCESS, // UAV (RWStructuredBuffer, etc)
```
...
```cpp
    };

    struct BufferDesc
    {
        uint64_t size = 0;
        uint32_t stride = 0; 
        BufferUsage usage = BufferUsage::ConstantBuffer;
        BufferMemoryType memoryType = BufferMemoryType::Default;
        const wchar_t* debugName = nullptr;
    };
```
...
```cpp
        uint64_t gpuHandle = UINT64_MAX;
        uint32_t descriptorIdx = UINT32_MAX;  // bindless模式下，用的是index而不是地址
        bool isValid() const { return cpuHandle != UINT64_MAX; }
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

### File: `Runtime/Graphics/Material.h`
```cpp
#include "MaterialInstance.h"

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

### File: `Runtime/Graphics/MaterialInstance.h`
```cpp
#include "IGPUResource.h"

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
}
```

### File: `Runtime/Graphics/MaterialLayout.h`
```cpp
#include "Assert.h"

namespace EngineCore
{
    struct MaterialPropertyLayout
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        ShaderVariableType type;
    };

    class MaterialLayout
    {
    public:
        // 后续用json加载
        void BuildFromJson(){};

        // temp;
        // 【新增】硬编码构建一个标准布局，模拟未来的 JSON 加载结果
        // 对应 Shader/SimpleTestShader.hlsl 里的 cbuffer 结构
        static MaterialLayout GetDefaultPBRLayout()
        {
            MaterialLayout layout;
            uint32_t currentOffset = 0;

            // 辅助 Lambda，模拟 JSON 遍历过程
            auto AddProp = [&](const std::string& name, ShaderVariableType type, uint32_t size) 
            {
                MaterialPropertyLayout prop;
                prop.name = name;
                prop.type = type;
                prop.size = size;
                prop.offset = currentOffset;
                layout.m_PropertyLayout[name] = prop;
                
                currentOffset += size;
            };

            // --- Chunk 0 ---
            // float4 DiffuseColor
            AddProp("DiffuseColor", ShaderVariableType::VECTOR4, 16);
            
            // float4 SpecularColor
            AddProp("SpecularColor", ShaderVariableType::VECTOR4, 16);

            // float Roughness
            AddProp("Roughness", ShaderVariableType::FLOAT, 4);
            
            // float Metallic
            AddProp("Metallic", ShaderVariableType::FLOAT, 4);

            // float2 TilingFactor (8 bytes)
            AddProp("TilingFactor", ShaderVariableType::VECTOR2, 8);
            
            AddProp("DiffuseTextureIndex", ShaderVariableType::FLOAT, 4);
            
            AddProp("PaddingLast", ShaderVariableType::VECTOR3, 12);

            // 此时 offset = 16+16+4+4+8 = 48 bytes
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

### File: `Runtime/Graphics/Mesh.h`
```cpp
#include "Math/AABB.h"

namespace EngineCore
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


    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
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

    class Mesh : public Resource
    {
    public:
        // todo: 先这么写，后续或许抽成单独Component

        Mesh() = default;
        Mesh(MetaData* metaData);
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
```
...
```cpp
        virtual void OnLoadComplete() override { UploadMeshToGPU(); };
    private:
        void ProcessNode(aiNode* node, const aiScene* scene);
        void LoadAiMesh(const string& path);
        void ProcessMesh(aiMesh* aiMesh, const aiScene* scene);

    };

}
```

### File: `Runtime/Graphics/MeshUtils.h`
```cpp
#include "Graphics/Mesh.h"

namespace EngineCore
{
    class MeshUtils
    {
    public:
        static void GetFullScreenQuad(Mesh* modelData);
    private:

    };
}
```

### File: `Runtime/Graphics/RenderTexture.h`
```cpp
#include "Graphics/IGPUResource.h"

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

}
```

### File: `Runtime/Graphics/Shader.h`
```cpp


namespace EngineCore
{
    class Shader : public Resource
    {
    public:
        Shader(MetaData* metaFile);
        ShaderReflectionInfo mShaderReflectionInfo;
        vector<InputLayout> mShaderInputLayout;

        Shader();
        ~Shader();
        string name;
    };
}
```

### File: `Runtime/Graphics/Texture.h`
```cpp
#include "Resources/Resource.h"

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

}
```

### File: `Runtime/Serialization/AssetHeader.h`
```cpp
#include <iostream>

namespace EngineCore
{
    // type 0 = Scene
    // type 1 = Texture
    // type 2 = Mesh
    struct AssetHeader
    {
        char magic[4] = {'E', 'N', 'G', 'N'};
        uint32_t version = 1;
        uint32_t assetID = 0;
        uint32_t type = 0; 
    };
}
```

### File: `Runtime/Serialization/AssetSerialization.h`
```cpp


namespace EngineCore
{
    // Meta
    inline void to_json(json& j, const EngineCore::MetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
            };
    }

    inline void from_json(const json& j, EngineCore::MetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
    }


    // TextureMetaData
    inline void to_json(json& j, const EngineCore::TextureMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"Dimension", v.dimension}, 
                {"Format", v.format},
                {"Width", v.width},
                {"Height", v.height}
            };
    }

    inline void from_json(const json& j, EngineCore::TextureMetaData& v)
    {
        j.at("Path").get_to(v.path);
        j.at("AssetType").get_to(v.assetType);
        j.at("Dependencies").get_to(v.dependentMap);
        j.at("Dimension").get_to(v.dimension);
        j.at("Format").get_to(v.format);
        j.at("Width").get_to(v.width);
        j.at("Height").get_to(v.height);
    }

    // MaterialMetaData
    inline void to_json(json& j, const EngineCore::MaterialMetaData& v)
    {
        j = {  
                {"Path", v.path}, 
                {"AssetType", v.assetType}, 
                {"Dependencies", v.dependentMap}, 
                {"FloatData", v.floatData}, 
                {"Vec2Data", v.vec2Data},
                {"Vec3Data", v.vec3Data},
                {"Matrix4x4Data", v.matrix4x4Data}
            };
    }
```

### File: `Runtime/Serialization/BaseTypeSerialization.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{
    // // Vector3
    using json = nlohmann::json;
    inline void to_json(json& j, const Vector3& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
    }

    inline void from_json(const json& j, EngineCore::Vector3& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
    }

    //vector2
    inline void to_json(json& j, const EngineCore::Vector2& v)
    {
        j = json{ {"x", v.x}, {"y", v.y}};
    }

    inline void from_json(const json& j, EngineCore::Vector2& v)
    {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
    }

    // Matrix4x4
    inline void to_json(json& j, const EngineCore::Matrix4x4& m)
    {
        j = json{
            {"m00", m.m00}, {"m01", m.m01}, {"m02", m.m02},{"m03", m.m03},
            {"m10", m.m10}, {"m11", m.m11}, {"m12", m.m12}, {"m13", m.m13},
            {"m20", m.m20}, {"m21", m.m21}, {"m22", m.m22}, {"m23", m.m23},
            {"m30", m.m30}, {"m31", m.m31}, {"m32", m.m32}, {"m33", m.m33}
        };
    }

    inline void from_json(const json& j, EngineCore::Matrix4x4& m)
    {
        j.at("m00").get_to(m.m00); j.at("m01").get_to(m.m01); j.at("m02").get_to(m.m02); j.at("m03").get_to(m.m03);
        j.at("m10").get_to(m.m10); j.at("m11").get_to(m.m11); j.at("m12").get_to(m.m12); j.at("m13").get_to(m.m13);
        j.at("m20").get_to(m.m20); j.at("m21").get_to(m.m21); j.at("m22").get_to(m.m22); j.at("m23").get_to(m.m23);
        j.at("m30").get_to(m.m30); j.at("m31").get_to(m.m31); j.at("m32").get_to(m.m32); j.at("m33").get_to(m.m33);
    }

    // Quaternion
    inline void to_json(json& j, const EngineCore::Quaternion& m)
    {
        j = json{
            {"x", m.x},
            {"y", m.y},
            {"z", m.z},
            {"w", m.w},
        };
    }

```

### File: `Runtime/Serialization/ComponentFactory.h`
```cpp

// 注册所有Component类， 运行时根据名字生成对应的Component
namespace EngineCore
{
    class ComponentFactory
    {
    public :
        using CreateFunc = std::function<Component*(GameObject*)>;
        inline static Component* Create(const std::string& componentName, GameObject* go)
        {
            auto& registry = GetRegistry();
            if(registry.count(componentName) > 0)
            {
                return registry[componentName](go);
            }

            ASSERT_MSG(false, "Can't find this script");
            return nullptr;
        }

        inline static void Register(const std::string& componentName, CreateFunc createFunc)
        {
            GetRegistry()[componentName] = createFunc;
        }
    private :

        // 这样写的好处， 不用在cpp调用， 初次调用的时候创建，一定保证时序
        static std::unordered_map<std::string, CreateFunc>& GetRegistry()
        {
            static std::unordered_map<std::string, CreateFunc> registry;
            return registry;
        }
    };
}
```
...
```cpp
#define REGISTER_SCRIPT(ComponentClass)\
    namespace { \
        struct ComponentClass##_Register { \
            ComponentClass##_Register() { \
                EngineCore::ComponentFactory::Register( #ComponentClass, \
                    [](EngineCore::GameObject* go) -> EngineCore::Component* {\
                        return new EngineCore::ComponentClass(go); \
                    }); \
            } \
        } ComponentClass##_instance; \
```

### File: `Runtime/Serialization/JsonSerializer.h`
```cpp

// 这个类负责读取和写回json，不负责别的功能。
namespace EngineCore
{
    using json = nlohmann::json;

    class JsonSerializer
    {
    public:
        template<typename T>
        static void SvaeAsJson(const T* data, const string& relativePath)
        {
            string metaPath = PathSettings::ResolveAssetPath(relativePath);

            // 防御：如果路径中有扩展名，去掉扩展名
            size_t dotPos = metaPath.find_last_of('.');
            if (dotPos != string::npos) {
                metaPath = metaPath.substr(0, dotPos);
            }

            metaPath += ".meta";
            
            json j = MetaFactory::ConvertToJson<T>(data);

            // 确保目录存在
            std::filesystem::path filePath(metaPath);
            std::filesystem::path dirPath = filePath.parent_path();
            if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
                std::filesystem::create_directories(dirPath);
            }

            // 写入文件（不存在创建，存在覆盖）
            std::ofstream file(metaPath, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {  
                ASSERT_MSG(false, "Failed to open/create file!");
                return;
            }
            
            file << j.dump(4);
            file.close();
            
            return;
        };

    
        static json ReadFromJson(const string& path)
        {
			string metaPath = EngineCore::PathSettings::ResolveAssetPath(path);
    
			// 读取JSON文件并解析
			std::ifstream file(metaPath);

			if(!file.is_open())
			{
				// 错误处理：文件打不开
				std::cerr << "无法打开文件: " << metaPath << std::endl;
                return json{};
			}
			
			json j = json::parse(file);
```