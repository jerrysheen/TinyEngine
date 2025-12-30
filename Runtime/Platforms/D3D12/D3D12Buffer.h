#pragma once
#include "Graphics/IGPUBuffer.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类
#include "d3dUtil.h"

namespace EngineCore
{
    class D3D12Buffer : public IGPUBuffer
    {
    public:
        D3D12Buffer(ComPtr<ID3D12Resource> resource, const BufferDesc& desc, D3D12_RESOURCE_STATES initialState)
            : m_Resource(resource), m_Desc(desc) 
        {
            if(desc.memoryType == BufferMemoryType::Upload)
            {
                Map();
            }

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

        virtual ~D3D12Buffer()
        {
            if(m_Desc.memoryType != BufferMemoryType::Default) UnMap();
        }

        virtual const BufferDesc& GetDesc() const override {return m_Desc;};
        virtual void* GetNativeHandle() const override {return m_Resource.Get();}

    
        virtual uint64_t GetGPUVirtualAddress() const override
        {
            return m_Resource->GetGPUVirtualAddress();
        }
    
        virtual void* Map() override
        {
            if(!m_MappedData){
                m_Resource->Map(0, nullptr, &m_MappedData);
            }
            return m_MappedData;
        }

        virtual void UnMap() override
        {
            if(m_MappedData)
            {
                m_Resource->Unmap(0, nullptr);
                m_MappedData = nullptr;
            }
        }
    
        virtual void SetName(const wchar_t* name) override {m_Resource->SetName(name);}

        BufferResourceState m_ResourceState;
    private:
        ComPtr<ID3D12Resource> m_Resource;
        BufferDesc m_Desc;
        void* m_MappedData = nullptr;
    };
}