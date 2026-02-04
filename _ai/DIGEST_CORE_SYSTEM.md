# Architecture Digest: CORE_SYSTEM
> Auto-generated. Focus: Runtime/Core, Runtime/Core/Allocator, Runtime/Core/Concurrency, Runtime/Math, Runtime/Serialization, Runtime/Utils, PublicStruct, PublicEnum, Profiler, Job, JobSystem, InstanceID, Allocator

## Project Intent
目标：构建现代化渲染器与工具链，强调GPU驱动渲染、资源管理、可扩展渲染管线与编辑器协作。

## Digest Guidance
- 优先提取头文件中的接口定义与系统契约，避免CPP实现噪音。
- 如果某子系统缺少头文件，可在索引中保留关键.cpp以建立结构视图。
- 突出GPU驱动渲染、资源生命周期、管线调度、序列化与工具链。
- 关注可扩展性：Pass/Path、RHI封装、资源描述、线程与任务系统。

## Understanding Notes
- 核心数据结构、内存分配、标识系统与序列化元数据是其他模块的基础。
- 优先记录公开结构体/枚举/Allocator/Profiler接口。

## Key Files Index
- `[67]` **Runtime/Core/Concurrency/JobSystem.h** *(Content Included)*
- `[65]` **Runtime/Core/Concurrency/JobSystem.cpp** *(Content Included)*
- `[44]` **Runtime/Core/Allocator/LinearAllocator.h** *(Content Included)*
- `[37]` **Runtime/Core/Profiler.h** *(Content Included)*
- `[36]` **Runtime/Core/InstanceID.h** *(Content Included)*
- `[33]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[32]` **Runtime/Core/PublicEnum.h** *(Content Included)*
- `[31]` **Runtime/Core/PublicEnum.cpp** *(Content Included)*
- `[31]` **Runtime/Core/PublicStruct.cpp** *(Content Included)*
- `[28]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[26]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[25]` **Runtime/Graphics/GPUBufferAllocator.cpp** *(Content Included)*
- `[25]` **Runtime/Renderer/PerDrawAllocator.h** *(Content Included)*
- `[25]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp** *(Content Included)*
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[22]` **Runtime/Core/Concurrency/CpuEvent.h** *(Content Included)*
- `[20]` **Runtime/Entry.cpp** *(Content Included)*
- `[20]` **Runtime/Core/Concurrency/CpuEvent.cpp** *(Content Included)*
- `[16]` **Runtime/Core/Object.h** *(Content Included)*
- `[16]` **Runtime/Renderer/RenderEngine.cpp**
- `[13]` **Runtime/Graphics/GPUSceneManager.cpp**
- `[13]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp**
- `[12]` **Runtime/CoreAssert.h**
- `[12]` **Runtime/Core/Game.h**
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
- `[12]` **Runtime/Renderer/Renderer.h**
- `[12]` **Runtime/Serialization/AssetHeader.h**
- `[12]` **Runtime/Serialization/DDSTextureLoader.h**
- `[12]` **Runtime/Serialization/MaterialLoader.h**
- `[12]` **Runtime/Serialization/MeshLoader.h**
- `[12]` **Runtime/Serialization/SceneLoader.h**
- `[12]` **Runtime/Serialization/ShaderLoader.h**
- `[12]` **Runtime/Serialization/StreamHelper.h**
- `[12]` **Runtime/Serialization/TextureLoader.h**
- `[12]` **Runtime/Utils/HashCombine.h**
- `[10]` **Runtime/Math/AABB.cpp**
- `[10]` **Runtime/Math/Frustum.cpp**
- `[10]` **Runtime/Math/Matrix4x4.cpp**
- `[10]` **Runtime/Math/Plane.cpp**
- `[10]` **Runtime/Math/Quaternion.cpp**
- `[10]` **Runtime/Math/Vector2.cpp**
- `[10]` **Runtime/Math/Vector3.cpp**
- `[10]` **Runtime/Math/Vector4.cpp**
- `[9]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[9]` **Runtime/Platforms/D3D12/d3dUtil.h**
- `[8]` **Runtime/Graphics/GPUSceneManager.h**
- `[8]` **Runtime/Renderer/RenderCommand.h**
- `[8]` **Runtime/Scene/SceneManager.cpp**
- `[8]` **Assets/Shader/StandardPBR_VertexPulling.hlsl**
- `[7]` **Runtime/Graphics/GeometryManager.h**

## Evidence & Implementation Details

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

### File: `Runtime/Core/Concurrency/JobSystem.cpp`
```cpp
#include "JobSystem.h"

namespace EngineCore
{
    JobSystem* JobSystem::s_Instance = nullptr;

    JobSystem::JobSystem()
    {
        isRunning = true;

        unsigned int numThreads = std::thread::hardware_concurrency();
        ASSERT(numThreads >= 2);
        numThreads = numThreads - 2;

        // job会优先起在空闲核内， 如果要指定核，要用别的接口
        for(unsigned int i = 0; i < numThreads; i++)
        {
            m_Workers.emplace_back(&JobSystem::WorkerThreadLoop, this);
        }
    }

    JobSystem::~JobSystem()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            isRunning = false;
        }

        wakeWorker.notify_all();

        for(std::thread& worker : m_Workers)
        {
            if(worker.joinable())
            {
                worker.join();
            }
        }

        while(!counterQueue.empty())
        {
            delete counterQueue.front();
            counterQueue.pop_front();
        }
    }

    void JobSystem::Create()
    {
        if(s_Instance != nullptr)
        {
            return;
        }
        s_Instance = new JobSystem();
    }

    void JobSystem::Shutdown()
    {
    }

    JobSystem *JobSystem::GetInstance()
    {
        if(s_Instance == nullptr)
        {
            Create();
        }
        return s_Instance;
    }

    void JobSystem::WaitForJob(JobHandle handle)
    {
        while(handle.counter->value > 0)
        {
            bool executed = TryExecuteOneJob();
        }
    }

    void JobSystem::InternalKickJob(void (*function)(void *, void *), void *JobData, void *rawCounter)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            jobQueue.push_front({function, JobData, rawCounter});
```
...
```cpp
            InternalJob job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                // wait 为true，就是不sleep，继续走的条件是，
                // isRunning为false了，需要往下走逻辑， 或者是jobQueue有东西了
                // 需要往下pop()； 不然这个线程关不掉？
                wakeWorker.wait(lock, [&]{ return !isRunning || !jobQueue.empty();});
```
...
```cpp
        InternalJob job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if(jobQueue.empty()) return false;
            job = jobQueue.front();
            jobQueue.pop_front();
        }
        job.function(job.JobData, job.rawCounter);
    }

    JobCounter *JobSystem::GetAvaliableCounter()
    {
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


    // 前向声明，防止循环引用。
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

    struct PerDrawHandle
    {
        uint8_t* destPtr;
        uint32_t offset;
        uint32_t size;
    };
```
...
```cpp
            :mat(mat), mesh(mesh), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, Mesh* mesh, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), mesh(mesh), perDrawHandle(handle), instanceCount(instCount){}
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
...
```cpp
    {
        uint32_t perObejectIndex = UINT32_MAX;
        inline bool isValid() const {return perObejectIndex != UINT32_MAX;}
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
            break;
        case 36:
            return ShaderVariableType::VECTOR2;
            break;
        default:
            break;
        }
    }
    
    enum class SortingCriteria
    {
        ComonOpaque,
        ComonTransparent
    };
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

### File: `Runtime/Core/Concurrency/CpuEvent.h`
```cpp
#include <condition_variable>

namespace EngineCore
{
    class CpuEvent
    {
    public:
        static CpuEvent& RenderThreadSubmited();
        static CpuEvent& MainThreadSubmited();

        CpuEvent(bool startCondition = false)
            : m_signaled(startCondition){}

        void Signal()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_signaled = true;
            }
            m_cv.notify_all();
        }

        void Wait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]{ return m_signaled;});
            m_signaled = false;
        }

    private:
        std::mutex  m_mutex;
        std::condition_variable m_cv;
        bool    m_signaled = false;
    };
}
```

### File: `Runtime/Core/Object.h`
```cpp
        InstanceID id;
        inline const uint64_t GetInstanceID() const{ return id.v;};
        Object(){ id = InstanceIDGenerator::New();};
```