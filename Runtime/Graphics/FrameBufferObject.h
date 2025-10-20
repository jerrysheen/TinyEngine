#pragma once
#include "PreCompiledHeader.h"
#include "Resources/Resource.h"
#include "Core/PublicEnum.h"

namespace EngineCore
{
    // todo： 修改，这个地方两个是同样的数据，但是如果直接用FrameBuffer创建又很怪？
    struct FrameBufferDesc
    {
        string name;
        TextureDimension dimension;
        TextureFormat format;
        float width;
        float height;
    };

    class FrameBufferObject : public Resource
    {
    public:
        string name;
        TextureDimension dimension;
        TextureFormat format;
        float width;
        float height;
        FrameBufferObject(const FrameBufferDesc& desc)
            :name(desc.name), dimension(desc.dimension), format(desc.format)
            , width(desc.width), height(desc.height)
        {
        };
        
        FrameBufferObject(const string& name)
            :name(name) 
        {
        };
    };

}