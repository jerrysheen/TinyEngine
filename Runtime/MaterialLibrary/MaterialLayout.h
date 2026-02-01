#pragma once
#include "iostream"
#include "Core/PublicEnum.h"
#include <unordered_map>
#include "Assert.h"

namespace EngineCore
{
    // 用来存
    struct TextureAssetInfo
    {
        // 这个地方直接记录
        char name[64];
        uint64_t ASSETID;
    };

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

        void AddProp(const std::string& name, ShaderVariableType type, uint32_t size)
        {
            MaterialPropertyLayout prop;
            prop.name = name;
            prop.type = type;
            prop.size = size;
            prop.offset = m_TotalSize;
            m_PropertyLayout[name] = prop;
            
            m_TotalSize += size;
        }

        void AddTextureToBlockOffset(const std::string& name, uint32_t offset)
        {
            ASSERT(textureToBlockIndexMap.count(name) < 0);
            textureToBlockIndexMap[name] = offset;
            
        }

        uint32_t GetPropertyOffset(const std::string& name)
        {
            ASSERT(m_PropertyLayout.count(name) > 0);
            return m_PropertyLayout[name].offset;
        };

        uint32_t GetSize(){ return m_TotalSize;}

        std::unordered_map<std::string, MaterialPropertyLayout> m_PropertyLayout;
        std::unordered_map<std::string, uint32_t> textureToBlockIndexMap;
    private:
        uint32_t m_TotalSize = 0;
    };

}