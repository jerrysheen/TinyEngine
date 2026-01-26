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
- `[37]` **Runtime/Core/Concurrency/JobSystem.h** *(Content Included)*
- `[36]` **Runtime/Core/InstanceID.h** *(Content Included)*
- `[34]` **Runtime/Core/Allocator/LinearAllocator.h** *(Content Included)*
- `[33]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[32]` **Runtime/Core/PublicEnum.h** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[26]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[25]` **Runtime/Renderer/PerDrawAllocator.h** *(Content Included)*
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[16]` **Runtime/Core/Object.h** *(Content Included)*
- `[12]` **Runtime/CoreAssert.h** *(Content Included)*
- `[12]` **Runtime/Core/Game.h** *(Content Included)*
- `[12]` **Runtime/Core/ThreadSafeQueue.h**
- `[12]` **Runtime/Math/AABB.h**
- `[12]` **Runtime/Math/Frustum.h**
- `[12]` **Runtime/Math/Math.h**
- `[12]` **Runtime/Math/Matrix4x4.h**
- `[12]` **Runtime/Math/Plane.h**
- `[12]` **Runtime/Math/Quaternion.h**
- `[12]` **Runtime/Math/Vector2.h**
- `[12]` **Runtime/Math/Vector3.h**
- `[12]` **Runtime/Math/Vector4.h**
- `[12]` **Runtime/Serialization/AssetHeader.h**
- `[12]` **Runtime/Serialization/DDSTextureLoader.h**
- `[12]` **Runtime/Serialization/MeshLoader.h**
- `[12]` **Runtime/Serialization/SceneLoader.h**
- `[12]` **Runtime/Serialization/StreamHelper.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[9]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[8]` **Runtime/Graphics/GPUSceneManager.h**
- `[8]` **Runtime/Renderer/RenderCommand.h**
- `[8]` **Runtime/Renderer/Renderer.h**
- `[7]` **Runtime/Graphics/GeometryManager.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12DescManager.h**
- `[7]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**
- `[6]` **Runtime/Resources/Resource.h**
- `[6]` **Runtime/Renderer/RenderPath/LagacyRenderPath.h**
- `[5]` **premake5.lua**
- `[5]` **Runtime/Resources/Asset.h**
- `[5]` **Runtime/Platforms/D3D12/D3D12Struct.h**
- `[5]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[4]` **Editor/D3D12/D3D12EditorGUIManager.h**
- `[4]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[3]` **Runtime/PreCompiledHeader.h**
- `[3]` **Runtime/GameObject/Camera.h**
- `[3]` **Runtime/GameObject/MeshFilter.h**
- `[3]` **Runtime/Graphics/ComputeShader.h**
- `[3]` **Runtime/Graphics/Material.h**
- `[3]` **Runtime/Graphics/MaterialLayout.h**
- `[3]` **Runtime/Graphics/Mesh.h**
- `[3]` **Runtime/Graphics/Shader.h**
- `[3]` **Runtime/Renderer/BatchManager.h**
- `[3]` **Runtime/Renderer/RenderAPI.h**
- `[3]` **Runtime/Renderer/RenderContext.h**
- `[3]` **Runtime/Renderer/RenderSorter.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/FinalBlitPass.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/GPUSceneRenderPass.h**
- `[3]` **Runtime/Renderer/RenderPipeLine/OpaqueRenderPass.h**

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

### File: `Runtime/Core/Concurrency/JobSystem.h`
```cpp
namespace EngineCore
{
    struct JobCounter
    {
        std::atomic<int> value;
    };
```
...
```cpp
    };

    struct InternalJob
    {
        void (*function)(void*, void*);
        void* JobData;
        void* rawCounter;
    };
```
...
```cpp
    {
    public:
        JobSystem();
        ~JobSystem();
        static void Create();
        static void Shutdown();
        static JobSystem* GetInstance();

        template<typename CallableJob>
        void KickJob(CallableJob job, JobHandle& handler, JobCounter* counter)
        {
            if(counter == nullptr)
            {
                counter = GetAvaliableCounter();
            }

            void* jobData = new CallableJob(job);

            counter->value.fetch_add(1);
            auto lambda = [](void* jobData, void* rawCounter)
            {
                CallableJob* job = (CallableJob*)jobData;
                job();
                JobCounter* counter = (JobCounter*)rawCounter;
                counter->value.fetch_sub(1);
                delete jobData;
            }
            handler.counter = counter;
            InternalKickJob(lambda, jobData, counter);
        }
```
...
```cpp
        std::vector<std::thread> m_Workers;

        void WaitForJob(JobHandle handle);

    private:
        void InternalKickJob(void (*function)(void*, void*), void* JobData, void* rawCounter);
        void WorkerThreadLoop(); 
        bool TryExecuteOneJob();
        static JobSystem* s_Instance;
        JobCounter* GetAvaliableCounter();
        std::deque<JobCounter*> counterQueue;
    };

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
        int bindCount = 1;             // 绑定数量，数组时 > 1
        ShaderBindingInfo (const string& resourceName, ShaderResourceType type, int registerSlot, int size, int space, int bindCount = 1)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size),
            space(space), bindCount(bindCount)
        {};
    };


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};


        // 定义偏移量常量 (方便修改)
        static const int BIT_OFFSET_CBV = 0;
        static const int BIT_OFFSET_SRV = 16;
        static const int BIT_OFFSET_UAV = 48;
        static const int BIT_OFFSET_SAMPLER = 56;
    };

    struct LightData
    {

    };


```
...
```cpp
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        MeshRenderer* meshRenderer;
        MeshFilter* meshFilter;
        float distanToCamera = 0;
    };
```
...
```cpp


    struct DrawRecord
    {
        Material* mat;
        Mesh* mesh;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, Mesh* mesh)
            :mat(mat), mesh(mesh), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, Mesh* mesh, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), mesh(mesh), perDrawHandle(handle), instanceCount(instCount){}
    };
```
...
```cpp
    };

    class RenderPass;
    struct RenderPassAsset
    {
        vector<RenderPass*> renderPasses;
        inline void Clear()
        {
             for (RenderPass* pass : renderPasses) 
                delete pass;
        };
    };
```
...
```cpp
    };

    struct ContextFilterSettings
    {

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

### File: `Runtime/Platforms/D3D12/D3D12DescAllocator.h`
```cpp
namespace EngineCore
{
    class D3D12DescAllocator
    {
    public:
        D3D12DescAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap, bool isShaderVisible = false);
        ~D3D12DescAllocator(){};
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType(){ return mHeapType;};

        DescriptorHandle CreateDescriptor(const D3D12_SAMPLER_DESC& desc){};
		DescriptorHandle CreateDescriptor(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
		DescriptorHandle CreateDescriptor(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
        DescriptorHandle AllocateStaticHandle();
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;

        void Reset();
        void CleanPerFrameData();

        // 设置动态分配的起始位置（用于混合 Heap 模式）
        void SetDynamicStartOffset(int offset) 
        { 
            dynamicStartOffset = offset;
            currDynamicoffset = offset;
        }
        // 专门用于 Global Heap 的动态分配
        DescriptorHandle AllocateDynamicSpace(int count);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS GetHeapVisible(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool isFrameHeap);
        int ConfigAllocatorDescSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        //std::vector<bool> isInUse;
        std::vector<int> freeIndexList;

        int dynamicStartOffset = 0; // 记录动态分配的起始点，Reset 时回到这里
        int currDynamicoffset = 0;
        
        int currentOffset = 0;
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

### File: `Runtime/Core/Game.h`
```cpp


namespace EngineCore
{
    class Game
    {
    public:
        static std::unique_ptr<Game> m_Instance;
        // 回传的是一个对象的引用，所以返回*ptr
        static Game* GetInstance()
        {
            if(m_Instance == nullptr)
            {
                m_Instance = std::make_unique<Game>();
            }
            return m_Instance.get();
        };
        Game(){};
        ~Game(){};

        void Launch();
    private:
        void Update();
        void Render();
        void EndFrame();
        void Shutdown();
    };

}
```