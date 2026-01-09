# Architecture Digest: CORE_SYSTEM
> Auto-generated. Focus: Runtime/Core, Runtime/Math, Runtime/Serialization, PublicStruct, PublicEnum, Profiler, Job, InstanceID, Allocator

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 核心数据结构、内存分配、标识系统与序列化元数据是其他模块的基础。
- 优先记录公开结构体/枚举/Allocator/Profiler接口。

## Key Files Index
- `[37]` **Runtime/Core/Profiler.h** *(Content Included)*
- `[36]` **Runtime/Core/InstanceID.h** *(Content Included)*
- `[34]` **Runtime/Core/Allocator/LinearAllocator.h** *(Content Included)*
- `[33]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[32]` **Runtime/Core/PublicEnum.h** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[27]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[25]` **Runtime/Renderer/PerDrawAllocator.h** *(Content Included)*
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[16]` **Runtime/Core/Object.h** *(Content Included)*
- `[13]` **Runtime/Serialization/BaseTypeSerialization.h** *(Content Included)*
- `[13]` **Runtime/Serialization/MetaData.h** *(Content Included)*
- `[13]` **Runtime/Serialization/MetaLoader.h** *(Content Included)*
- `[12]` **Runtime/CoreAssert.h** *(Content Included)*
- `[12]` **Runtime/Serialization/MetaFactory.h**
- `[12]` **Runtime/Serialization/JsonSerializer.h**
- `[12]` **Runtime/Serialization/ComponentFactory.h**
- `[12]` **Runtime/Serialization/AssetSerialization.h**
- `[12]` **Runtime/Core/Game.h**
- `[12]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[12]` **Runtime/Math/Quaternion.h**
- `[12]` **Runtime/Math/AABB.h**
- `[12]` **Runtime/Math/Plane.h**
- `[12]` **Runtime/Math/Vector3.h**
- `[12]` **Runtime/Math/Matrix4x4.h**
- `[12]` **Runtime/Math/Vector2.h**
- `[12]` **Runtime/Math/Math.h**
- `[12]` **Runtime/Math/Vector4.h**
- `[12]` **Runtime/Math/Frustum.h**
- `[9]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[8]` **Runtime/Graphics/GPUSceneManager.h**
- `[8]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[8]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[7]` **Runtime/Renderer/RenderCommand.h**
- `[6]` **Runtime/Resources/Resource.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/Resources/Asset.h**
- `[5]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[4]` **Runtime/Renderer/RenderAPI.h**
- `[4]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[3]` **Runtime/PreCompiledHeader.h**
- `[3]` **Runtime/Graphics/MaterialLayout.h**
- `[3]` **Runtime/Graphics/ModelData.h**
- `[3]` **Runtime/Graphics/ComputeShader.h**
- `[3]` **Runtime/Graphics/Material.h**
- `[3]` **Runtime/Graphics/Shader.h**
- `[3]` **Runtime/Scene/Scene.h**
- `[3]` **Runtime/GameObject/Camera.h**
- `[3]` **Runtime/GameObject/MeshFilter.h**
- `[3]` **Runtime/Renderer/BatchManager.h**
- `[3]` **Runtime/Renderer/RenderSorter.h**
- `[3]` **Runtime/Renderer/RenderContext.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/RenderPass.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[3]` **Assets/Shader/SimpleTestShader.hlsl**

## Evidence & Implementation Details

### File: `Runtime/Core/Profiler.h`
```cpp
{
    // 函数打点
    struct ProfilerEvent
    {
        const char* name        = nullptr;
        uint32_t    threadID    = 0;
        float       startMs     = 0.0f;
        float       endMs       = 0.0f;
        uint8_t     depth       = 0;
    };
```
...
```cpp
    };
    
    struct ProfilerFrame
    {
        float frameStartMs = 0;
        const char* frameTag = nullptr;

        std::vector<ProfilerEvent> events;

        std::unordered_map<std::string, uint32_t> counterIndex;

        void Clear()
        {
            events.clear();
            counterIndex.clear();
            frameTag = nullptr;
            frameStartMs = 0.0f;
        }
    };
```
...
```cpp

    
    struct ProfilerZoneScope
    {
        explicit ProfilerZoneScope(const char* name)
            : m_name(name)
        {
            Profiler& p = Profiler::Get();
            m_startMs = p.GetTimeMs();
            m_threadId = GetThreadId();
            //m_depth = p.PushDepth();
        }

        ~ProfilerZoneScope()
        {
            Profiler& p = Profiler::Get();
            float end = p.GetTimeMs();
            p.RecordEvent(m_name, m_threadId, m_startMs, end, m_depth);
            //p.PopDepth();
        }
    private:
        static uint32_t GetThreadId()
        {
            auto id = std::this_thread::get_id();
            std::hash<std::thread::id> hasher;
            return static_cast<uint32_t>(hasher(id));
        }

        const char* m_name;
        float       m_startMs = 0.0f;
        uint32_t    m_threadId = 0;
        uint8_t     m_depth = 0;
    };
```

### File: `Runtime/Core/InstanceID.h`
```cpp
namespace EngineCore
{
    struct InstanceID
    {
        uint64_t v{0};
        explicit operator bool() const { return v != 0; };
    public:
    };
```

### File: `Runtime/Core/Allocator/LinearAllocator.h`
```cpp
namespace EngineCore
{
    class LinearAllocator
    {
    public:
        LinearAllocator(uint32_t size)
        {
            buffer.resize(size);
            currentOffset = 0;
        }

        void Reset()
        {
            currentOffset = 0;
        }

        // align of 得到当前class/struct的对齐方式
        void* allocate(uint32_t size, size_t align = alignof(std::max_align_t))
        {
            // 补齐空位， 下一个位置从当前struct align倍数开始，避免出错
            size_t alignedOffset = (currentOffset + align - 1) & ~(align - 1);
            ASSERT(alignedOffset + size <= buffer.size());
            // 这个地方错的， buffer.resize 会导致前面分配的失效。
            // if(alignedOffset + size > buffer.size())
            // {
            //     size_t newSize = buffer.size() * 2;
            //     while (alignedOffset + size > newSize) newSize *= 2;
            //     buffer.resize(newSize);
            // }

            void* ptr = buffer.data() + alignedOffset;
            currentOffset = alignedOffset + size;
            return ptr;
        }

        template<typename T>
        T* allocArray(int size)
        {
            return (T*)allocate(size * sizeof(T), alignof(T));
        }
    private:
        std::vector<uint8_t> buffer;
        size_t currentOffset = 0;
    };
```

### File: `Runtime/Core/PublicStruct.h`
```cpp
#include "Graphics/RenderTexture.h"

namespace EngineCore
{
    // 用来描述model Input 或者 shader reflection input
    // todos: shader inputlayout 部分的控制， 目前只是简单把值塞过来
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

    // constantbuffer中的变量记录
    struct ShaderConstantInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // shader中通过反射得到的资源名称、类型
    struct ShaderBindingInfo 
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        int space = 0;
        ShaderBindingInfo (const string& resourceName, ShaderResourceType type, int registerSlot, int size, int space)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size),
            space(space)
        {};
    };


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
```
...
```cpp
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};
```
...
```cpp
    };

    struct LightData
    {

    };
```
...
```cpp

    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        MeshRenderer* meshRenderer;
        uint32_t vaoID;
        float distanToCamera = 0;
    };
```
...
```cpp


    struct DrawRecord
    {
        Material* mat;
        uint32_t vaoID;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, uint32_t vaoID)
            :mat(mat), vaoID(vaoID), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, uint32_t vaoID, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), vaoID(vaoID), perDrawHandle(handle), instanceCount(instCount){}
    };
```

### File: `Runtime/Core/PublicEnum.h`
```cpp
#pragma once

namespace EngineCore
{

    enum class ShaderVariableType
    {
        FLOAT, VECTOR2, VECTOR3, VECTOR4, MATRIX4X4, MATRIX3X4, MATRIX3X3, UNKNOWN
    };

    enum class ShaderStageType
    {
        VERTEX_STAGE, FRAGMENT_STAGE
    };

    enum class ShaderResourceType
    {
        CONSTANT_BUFFER,
        TEXTURE,
        SAMPLER,
        UAV
    };

    enum class VertexAttribute
    {
        POSITION,
        NORMAL,
        TANGENT,
        UV0
    };

    enum class Primitive
    {
        Quad,
        Cube,
        Sphere
    };
   
    inline ShaderVariableType InferShaderVaribleTypeBySize(uint32_t size)
    {
        // 以byte为准 比如float为4byte
        switch (size)
        {
        case 4:
            return ShaderVariableType::FLOAT; 
            break;
        case 8:
            return ShaderVariableType::VECTOR2;
            break;
        case 12:
            return ShaderVariableType::VECTOR2;
            break;
        case 16:
            return ShaderVariableType::VECTOR2;
            break;
        case 64:
            return ShaderVariableType::VECTOR2;
            break;
        case 48:
            return ShaderVariableType::VECTOR2;
```
...
```cpp
    }
    
    enum class SortingCriteria
    {
        ComonOpaque,
        ComonTransparent
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

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
namespace EngineCore
{
    class D3D12DescAllocator
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        DescriptorHandle GetNextAvaliableDesc();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
        void Reset();
        DescriptorHandle GetFrameAllocator(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        std::vector<bool> isInUse;

        int currentOffset = 0;
        int currCount = 0;
        uint32_t startIndex = 0;
        uint32_t mDescriptorSize = 0;
        int maxCount = 0;
    };
```

### File: `Runtime/Renderer/PerDrawAllocator.h`
```cpp
{

    class PerDrawAllocator
    {
    public:
        virtual ~PerDrawAllocator() = default;

        virtual PerDrawHandle Allocate(uint32_t size) = 0;
        virtual uint8_t*    GetCPUAddress(PerDrawHandle h) = 0; // 用来 memcpy
        virtual uint64_t    GetGPUBaseAddress() const = 0;  // 大 buffer 的 base GPU VA
        virtual void        ResetFrame() = 0;
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

### File: `Runtime/Core/Object.h`
```cpp
        InstanceID id;
        inline const uint64_t GetInstanceID() const{ return id.v;};
        Object(){ id = InstanceIDGenerator::New();};
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

### File: `Runtime/Serialization/MetaData.h`
```cpp
#include "Graphics/IGPUResource.h"

namespace EngineCore
{

    class Texture;
    class ModelData;
    class Material;
    struct MetaData
    {
        string path;
        AssetType assetType;
        // k,v = <ResouceName, MetaData>
        std::unordered_map<std::string, MetaData> dependentMap;
        MetaData() = default;
        MetaData(const std::string& path, AssetType type)
            : path(path), assetType(type) {};
    };


    struct TextureMetaData : MetaData
    {
        TextureDimension dimension;
        TextureFormat format;
        int width, height;
        // todo
        // warp mode, read write..
        // enable mipmap...
    };

    struct MaterialMetaData : MetaData 
    {
		string shaderPath;       
        unordered_map<string, float> floatData;
        unordered_map<string, Vector2> vec2Data;
        unordered_map<string, Vector3> vec3Data;
        unordered_map<string, Matrix4x4> matrix4x4Data;
        // 无意义的Texture*，只是为了和MaterialData做方便的同步而已。
        unordered_map<string, Texture*> textureData;
    };

    struct ModelMetaData : MetaData 
    {
        
    };

}
```

### File: `Runtime/Serialization/MetaLoader.h`
```cpp
#include "MetaData.h"

namespace EngineCore
{
    class MetaLoader
    {
    public:
        static MaterialMetaData* LoadMaterialMetaData(const std::string& path);
        static ShaderVariableType GetShaderVaribleType(uint32_t size);
        static TextureMetaData* LoadTextureMetaData(const std::string& path);
        static ModelMetaData* LoadModelMetaData(const std::string& path);
        template<typename T>
        static MetaData* LoadMetaData(const std::string& path)
        {
            MetaData* metaData = new MetaData();
            metaData->path = path;
            return metaData;
            // do nothing, for example shader..
        };

        

        template<typename T>
        static T* LoadMeta(const std::string& path)
        {
            std::ifstream file(path);
            
            if(!file.is_open())
            {
                throw std::runtime_error("Can't open Meta File" + path);
            }

            json j = json::parse(file);
            file.close();
            
            return j.get<T>();
        }

    };
    // 只声明特化
    template<>
    MetaData* MetaLoader::LoadMetaData<Texture>(const std::string& path);
    
    template<>
    MetaData* MetaLoader::LoadMetaData<ModelData>(const std::string& path);

    template<>
    MetaData* MetaLoader::LoadMetaData<Material>(const std::string& path);

}
```

### File: `Runtime/CoreAssert.h`
```cpp
    #define ASSERT_MSG(condition, message) \
        do { \
            if (!(condition)) { \
                std::wcout << L"Assert Failed: " << L#condition << L"\n"; \
                std::wcout << L"Message: " << message << L"\n"; \
                std::wcout << L"File: " << __FILEW__ << L", Line: " << __LINE__ << L"\n"; \
                __debugbreak(); \
            } \
```