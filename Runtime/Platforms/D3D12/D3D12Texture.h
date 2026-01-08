#pragma once
#include "Graphics/IGPUResource.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类
#include "d3dUtil.h"

namespace EngineCore
{
    // 只是一个资源的壳，IGPUTexture的实现，持有指针
    class D3D12Texture : public IGPUTexture
    {
    public:
        D3D12Texture() = default;

        D3D12Texture(const TextureDesc& desc) 
            : m_Desc(desc)
        {
        }

        D3D12Texture(ComPtr<ID3D12Resource> resource, const TextureDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            switch(initialState)
            {
                case D3D12_RESOURCE_STATE_COMMON:
                m_ResourceState = BufferResourceState::STATE_COMMON;
                break;
                case D3D12_RESOURCE_STATE_GENERIC_READ:
                m_ResourceState = BufferResourceState::STATE_GENERIC_READ;
                break;
                case D3D12_RESOURCE_STATE_COPY_DEST:
                m_ResourceState = BufferResourceState::STATE_COPY_DEST;
                break;
                default:
                    ASSERT("Wrong InitialState");
                break;
            }
        }

        virtual ~D3D12Texture()
        {
        }

        virtual const TextureDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}


        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }

        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}
    public:
        ComPtr<ID3D12Resource> m_Resource;
        TextureDesc m_Desc;
    };
}