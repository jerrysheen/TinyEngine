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
- `[12]` **Runtime/Graphics/MaterialData.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MaterialInstance.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MaterialLayout.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Mesh.h** *(Content Included)*
- `[12]` **Runtime/Graphics/MeshUtils.h** *(Content Included)*
- `[12]` **Runtime/Graphics/RenderTexture.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Shader.h** *(Content Included)*
- `[12]` **Runtime/Graphics/Texture.h** *(Content Included)*
- `[12]` **Runtime/Serialization/AssetHeader.h** *(Content Included)*
- `[12]` **Runtime/Serialization/DDSTextureLoader.h** *(Content Included)*
- `[12]` **Runtime/Serialization/MeshLoader.h** *(Content Included)*
- `[12]` **Runtime/Serialization/SceneLoader.h** *(Content Included)*
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
- `[2]` **Runtime/Managers/WindowManager.h**
- `[2]` **Runtime/Math/AABB.h**
- `[2]` **Runtime/Math/Frustum.h**
- `[2]` **Runtime/Math/Math.h**
- `[2]` **Runtime/Math/Matrix4x4.h**

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
        DXT1, 
        DXT2, 
        DXT3, 
        DXT4, 
        DXT5,
        BC7,
        BC7_SRGB,
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

        uint32_t mipCount = 1;
        uint32_t arraySize = 1; // Texture Array 用
        uint32_t mipOffset[16] = {0}; // 最多支持16级mip
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
```
...
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

### File: `Runtime/Graphics/Material.h`
```cpp
#include "MaterialData.h"

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
        
        MaterialData m_MaterialData;
        void LoadFromMaterialData(const MaterialData& data);

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

        inline MaterailRenderState GetMaterialRenderState() const { return mRenderState;};
        MaterailRenderState mRenderState;
        BufferAllocation materialAllocation;
    private:
        void SetUpRenderState();
        void GetTextureInfoFromShaderReflection();
    };
}
```

### File: `Runtime/Graphics/MaterialData.h`
```cpp
#include "Resources/Asset.h"

namespace EngineCore
{
    // 对应 MaterialLayout::GetDefaultPBRLayout 的内存布局
    // 必须保持 16 字节对齐
    struct MaterialConstants
    {
        Vector4 DiffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 SpecularColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        float Roughness = 0.5f;
        float Metallic = 0.0f;
        Vector2 TilingFactor = Vector2(1.0f, 1.0f);
        float DiffuseTextureIndex = 0.0f; 
        Vector3 Padding = Vector3(0.0f, 0.0f, 0.0f);
    };

    // 序列化到硬盘的数据结构
    struct MaterialData
    {
        MaterialConstants constants;
        AssetID diffuseTexID = 0;
        AssetID normalTexID = 0;
        AssetID specularTexID = 0;
        AssetID emissiveTexID = 0;
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
#include "Graphics/IGPUResource.h"

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
        Mesh(Primitive primitiveType);
        MeshBufferAllocation* vertexAllocation;
        MeshBufferAllocation* indexAllocation;
        void UploadMeshToGPU();

        AABB bounds;
        std::vector<Vertex> vertex;
        std::vector<int> index;
        std::vector<InputLayout> layout;
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
        ShaderReflectionInfo mShaderReflectionInfo;
        vector<InputLayout> mShaderInputLayout;
        Shader(const std::string& path);
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

        //inline const string GetName() const { return mTextureName; };

        inline int GetWidth() { return textureDesc.width; };
        inline int GetHeight() { return textureDesc.height; };
        virtual void OnLoadComplete() override;
    public:
        IGPUTexture*  textureBuffer;
        TextureDesc textureDesc;
        std::vector<uint8_t> cpuData;
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
```
...
```cpp

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
        uint32_t aBitMask;
        uint32_t caps;
        uint32_t caps2;
        uint32_t caps3;
        uint32_t caps4;
        uint32_t reserved2;
    };
```
...
```cpp
    };

    struct DDSLoadResult {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t mipMapCount = 0;
        TextureFormat format;        // DXT1/DXT3/DXT5标识
        uint32_t blockSize = 0;     // 8 for DXT1, 16 for DXT3/DXT5
        std::vector<uint8_t> pixelData;
    };
```
...
```cpp
                uint32_t mipSize = CalculateDXTMipSize(mipWidth, mipHeight, ddsResult.blockSize);
                
                if (i > 0) {
                    tex->textureDesc.mipOffset[i] = offset;  // 记录当前 mip 的 offset
                }
```
...
```cpp
        }
    
        std::vector<uint8_t> LoadMipData(const std::string& realativePath, int mipCount){}
```

### File: `Runtime/Serialization/MeshLoader.h`
```cpp
#include "Settings/ProjectSettings.h"

namespace EngineCore
{
    class MeshLoader : public IResourceLoader
    {
    public:
        virtual ~MeshLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);
            in.seekg(sizeof(AssetHeader));

            Mesh* mesh = new Mesh();
            mesh->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            mesh->SetAssetID(AssetIDGenerator::NewFromFile(path));
            StreamHelper::Read(in, mesh->bounds);
            StreamHelper::ReadVector(in, mesh->vertex);
            StreamHelper::ReadVector(in, mesh->index);
            return mesh;
        }

        void SaveMeshToBin(const Mesh* mesh, const std::string& relativePath, uint32_t id)
        {
            ASSERT(mesh && mesh->vertex.size() > 0 && mesh->index.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 2;
            StreamHelper::Write(out, header);

            StreamHelper::Write(out, mesh->bounds);
            StreamHelper::WriteVector(out, mesh->vertex);
            StreamHelper::WriteVector(out, mesh->index);
        }

    };
}
```

### File: `Runtime/Serialization/SceneLoader.h`
```cpp
#include "Resources/AssetRegistry.h"
#include "Settings/ProjectSettings.h"
namespace EngineCore
{
    struct SceneSerializedNode
    {
        char name[64];         // 固定长度名字
        int32_t parentIndex = -1;
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        uint64_t meshID = 0;
        uint32_t materialID = 0;

    };

    class SceneLoader: public IResourceLoader                       
    {
    public:
        virtual ~SceneLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream in(path, std::ios::binary);

            // todo: 确保进来的文件是Scene类型
            in.seekg(sizeof(AssetHeader));

            Scene* scene = new Scene();
            std::vector<SceneSerializedNode> allnode;
            StreamHelper::ReadVector(in, allnode);
            std::unordered_map<uint32_t, GameObject*> gameObjectMap;
            
            for(int i = 0; i < allnode.size(); i++)
            {
                auto& nodeData = allnode[i];
                
                std::string nodeName = nodeData.name;
                GameObject* go = scene->CreateGameObject(nodeName.empty() ? "Node" : nodeName);

                go->transform->SetLocalPosition(nodeData.position);
                go->transform->SetLocalQuaternion(nodeData.rotation);
                go->transform->SetLocalScale(nodeData.scale);
                if(nodeData.parentIndex != -1)
                {
                    ASSERT(gameObjectMap.count(nodeData.parentIndex) > 0);
                    go->SetParent(gameObjectMap[nodeData.parentIndex]);
                }
                gameObjectMap[i] = go;

                //todo 加入材质的异步加载：
                if(nodeData.meshID != 0)
                {
                    MeshFilter* filter = go->AddComponent<MeshFilter>();
                    filter->mMeshHandle = ResourceManager::GetInstance()->LoadAssetAsync<Mesh>(nodeData.meshID, [=]() 
                        {
                            filter->OnLoadResourceFinished();
                        });
                }
```
...
```cpp
        void SaveSceneToBin(const Scene* scene, const std::string& relativePath, uint32_t id)
        {
            ASSERT(scene && scene->allObjList.size() > 0);
            std::string binPath = PathSettings::ResolveAssetPath(relativePath);
            std::ofstream out(binPath, std::ios::binary);

            AssetHeader header;
            header.assetID =id;
            header.type = 0;
            StreamHelper::Write(out, header);

            std::vector<SceneSerializedNode> linearNode;
            std::unordered_map<GameObject*, uint32_t> gameObjectMap;
            for(int i = 0; i < scene->rootObjList.size(); i++)
            {
                GameObject* gameObject = scene->rootObjList[i];
                
                SerilizedNode(gameObject, gameObjectMap, linearNode);
            }
```
...
```cpp

            // Name Serialization
            memset(node.name, 0, sizeof(node.name));
            const std::string& goName = gameObject->name;
            size_t copyLen = std::min(goName.size(), sizeof(node.name) - 1);
            memcpy(node.name, goName.c_str(), copyLen);

            Transform* parent = gameObject->transform->parentTransform;
            uint64_t meshID = 0;
            MeshFilter* meshfilter = gameObject->GetComponent<MeshFilter>();
            if (meshfilter != nullptr)
            {
                meshID = meshfilter->mMeshHandle->GetAssetID();
            }
```
...
```cpp
            else
            {
                ASSERT(gameObjectMap.count(parent->gameObject) > 0);
                node.parentIndex = gameObjectMap[parent->gameObject];
                gameObjectMap[gameObject] = linearNode.size();
            }
            node.materialID = 0;
            node.meshID = meshID;
            node.position = gameObject->transform->GetLocalPosition();
            node.rotation = gameObject->transform->GetLocalQuaternion();
            node.scale = gameObject->transform->GetLocalScale();
            linearNode.push_back(node);

            for (auto* child : gameObject->GetChildren()) 
            {
```