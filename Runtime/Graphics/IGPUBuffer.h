#pragma once
#include <iostream>

namespace EngineCore
{
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
        STATE_COMMON,
        STATE_UNORDERED_ACCESS, // UAV (RWStructuredBuffer, etc)
        STATE_SHADER_RESOURCE,  // SRV (StructuredBuffer, Texture, etc)
        STATE_INDIRECT_ARGUMENT, // Indirect Draw Args
        STATE_COPY_DEST,        // Copy Destination
        STATE_COPY_SOURCE,      // Copy Source
        STATE_GENERIC_READ      // Generic Read (Vertex, Index, Constant, etc)
    };

    struct BufferDesc
    {
        uint64_t size = 0;
        uint32_t stride = 0; 
        BufferUsage usage = BufferUsage::ConstantBuffer;
        BufferMemoryType memoryType = BufferMemoryType::Default;
        const wchar_t* debugName = nullptr;
    };

    class IGPUBuffer
    {
    public:
        virtual ~IGPUBuffer() = default;

        virtual const BufferDesc& GetDesc() const = 0;

        virtual void* GetNativeHandle() const = 0;

        virtual uint64_t GetGPUVirtualAddress() const = 0;

        virtual void* Map() = 0;
        virtual void UnMap() = 0;

        virtual void SetName(const wchar_t* name) = 0;
        // Helper to track current state if needed, but for now we might pass it explicitly in barriers
        //virtual void SetResourceState(BufferResourceState state) = 0;
        //virtual BufferResourceState GetResourceState() const = 0;
    };
};