#pragma once
#include "MaterialLayout.h"
#include "iostream"
#include "Graphics/IGPUResource.h"

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

        std::vector<uint8_t> GetInstanceData(){ return m_DataBlob; }
        inline void SetInstanceData(const std::vector<uint8_t> data){ m_DataBlob = data; }
        uint32_t GetSize(){return m_Layout.GetSize();}
        inline MaterialLayout GetLayout(){return m_Layout;};
        inline void SetLayout(const MaterialLayout& layout){ m_Layout = layout;};
        
    private:
        MaterialLayout m_Layout;
        std::vector<uint8_t> m_DataBlob;
        bool m_Dirty = true;
    };
}