#pragma once
#include "Buffer.h"

namespace EngineCore
{
    // todo： 修改，这个地方两个是同样的数据，但是如果直接用FrameBuffer创建又很怪？
    struct FrameBufferDesc
    {
        string name;
        TextureDimension dimension;
        TextureFormat format;
        int width;
        float height;
    };

    class FrameBufferObject : public Buffer2D
    {
    public:

        FrameBufferObject(const FrameBufferDesc& desc)
        {
            mTextureName = desc.name;
            mDimension = desc.dimension;
            mFormat = desc.format;
            mWidth = desc.width;
            mHeight = desc.height;
        };
        
        FrameBufferObject(const string& name)
        {
            mTextureName = name;
        };
    };

}