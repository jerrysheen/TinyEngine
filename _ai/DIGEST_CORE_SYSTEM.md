# Architecture Digest: CORE_SYSTEM
> Auto-generated. Focus: Runtime/Core, Runtime/Math, Runtime/Serialization, PublicStruct, PublicEnum, Profiler, Job, InstanceID, Allocator

## Key Files Index
- `[37]` **Runtime/Core/Profiler.h** *(Content Included)*
- `[36]` **Runtime/Core/InstanceID.h** *(Content Included)*
- `[34]` **Runtime/Core/Allocator/LinearAllocator.h** *(Content Included)*
- `[33]` **Runtime/Core/PublicStruct.h** *(Content Included)*
- `[32]` **Runtime/Core/PublicEnum.h** *(Content Included)*
- `[31]` **Runtime/Core/PublicEnum.cpp** *(Content Included)*
- `[31]` **Runtime/Core/PublicStruct.cpp** *(Content Included)*
- `[27]` **Runtime/Graphics/GPUBufferAllocator.h** *(Content Included)*
- `[27]` **Runtime/Platforms/D3D12/D3D12DescAllocator.h** *(Content Included)*
- `[25]` **Runtime/Graphics/GPUBufferAllocator.cpp** *(Content Included)*
- `[25]` **Runtime/Renderer/PerDrawAllocator.h** *(Content Included)*
- `[25]` **Runtime/Platforms/D3D12/D3D12DescAllocator.cpp** *(Content Included)*
- `[23]` **Runtime/Graphics/IGPUBufferAllocator.h** *(Content Included)*
- `[16]` **Runtime/Core/Object.h** *(Content Included)*
- `[14]` **Runtime/Core/Game.cpp** *(Content Included)*
- `[13]` **Runtime/Serialization/BaseTypeSerialization.h** *(Content Included)*
- `[13]` **Runtime/Serialization/MetaData.h** *(Content Included)*
- `[13]` **Runtime/Serialization/MetaLoader.h** *(Content Included)*
- `[13]` **Runtime/Platforms/D3D12/D3D12RenderAPI.cpp** *(Content Included)*
- `[12]` **Runtime/CoreAssert.h** *(Content Included)*
- `[12]` **Runtime/Core/Game.h**
- `[12]` **Runtime/Math/AABB.h**
- `[12]` **Runtime/Math/Frustum.h**
- `[12]` **Runtime/Math/Math.h**
- `[12]` **Runtime/Math/Matrix4x4.h**
- `[12]` **Runtime/Math/Plane.h**
- `[12]` **Runtime/Math/Quaternion.h**
- `[12]` **Runtime/Math/Vector2.h**
- `[12]` **Runtime/Math/Vector3.h**
- `[12]` **Runtime/Math/Vector4.h**
- `[12]` **Runtime/Serialization/AssetSerialization.h**
- `[12]` **Runtime/Serialization/ComponentFactory.h**
- `[12]` **Runtime/Serialization/JsonSerializer.h**
- `[12]` **Runtime/Serialization/MetaFactory.h**
- `[12]` **Runtime/Core/Concurrency/CpuEvent.h**
- `[10]` **Runtime/Math/AABB.cpp**
- `[10]` **Runtime/Math/Frustum.cpp**
- `[10]` **Runtime/Math/Matrix4x4.cpp**
- `[10]` **Runtime/Math/Plane.cpp**
- `[10]` **Runtime/Math/Quaternion.cpp**
- `[10]` **Runtime/Math/Vector2.cpp**
- `[10]` **Runtime/Math/Vector3.cpp**
- `[10]` **Runtime/Math/Vector4.cpp**
- `[10]` **Runtime/Serialization/MetaFactory.cpp**
- `[10]` **Runtime/Serialization/MetaLoader.cpp**
- `[10]` **Runtime/Core/Concurrency/CpuEvent.cpp**
- `[9]` **Runtime/Renderer/RenderPath/GPUSceneRenderPath.h**
- `[8]` **Runtime/Graphics/GPUSceneManager.h**
- `[8]` **Runtime/Renderer/Renderer.h**
- `[8]` **Runtime/Platforms/D3D12/D3D12RenderAPI.h**

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
    {
    public:
        static Profiler& Get()
        {
            static Profiler s_Instance;
            return s_Instance;
        }
```
...
```cpp
        }

        ProfilerFrame& GetCurrentFrame()
        {
            return m_frams[m_currentFrameIndex];  // 使用传入的 index
        }
```
...
```cpp
        }

        static uint32_t GetThreadIdForProfiler()
        {
            auto id = std::this_thread::get_id();
            std::hash<std::thread::id> hasher;
            return static_cast<uint32_t>(hasher(id));
        }
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
...
```cpp
    {
    public:
        static InstanceID New()
        {
            // 从 1 开始，保留 0 代表“无效”
            const auto next = sCount.fetch_add(1, std::memory_order_relaxed) + 1;
            return InstanceID{ next };
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