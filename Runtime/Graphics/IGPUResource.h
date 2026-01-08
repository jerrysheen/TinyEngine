#pragma once
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
        STATE_SHADER_RESOURCE,  // SRV (StructuredBuffer, Texture, etc)
        STATE_INDIRECT_ARGUMENT, // Indirect Draw Args
        STATE_COPY_DEST,        // Copy Destination
        STATE_COPY_SOURCE,      // Copy Source
        STATE_GENERIC_READ,      // Generic Read (Vertex, Index, Constant, etc)
        STATE_DEPTH_WRITE,
        STATE_RENDER_TARGET,
        STATE_PRESENT
    };

    struct BufferDesc
    {
        uint64_t size = 0;
        uint32_t stride = 0; 
        BufferUsage usage = BufferUsage::ConstantBuffer;
        BufferMemoryType memoryType = BufferMemoryType::Default;
        const wchar_t* debugName = nullptr;
    };

    struct DescriptorHandle
    {
        uint64_t cpuHandle = UINT64_MAX;
        uint64_t gpuHandle = UINT64_MAX;
        uint32_t descriptorIdx = UINT32_MAX;  // bindless模式下，用的是index而不是地址
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

    class IGPUTexture : public IGPUResource
    {
    public:
        virtual const TextureDesc& GetDesc() const = 0;
    public:
        DescriptorHandle srvHandle;
        DescriptorHandle rtvHandle;
        DescriptorHandle dsvHandle;
    };
};