#pragma once
#include <iostream>
#include "GPUBUfferAllocator.h"
#include "Renderer/RenderStruct.h"
#include "Renderer/RenderAPI.h"

// 创建并且管理multi frame buffer data
namespace EngineCore
{
    class PerFrameBufferRing
    {
    public:
        PerFrameBufferRing(const BufferDesc& inDesc)
        {
            BufferDesc desc = inDesc;
            for(int i = 0; i < kMaxFrames; i++)
            {
                std::wstring name = desc.debugName + std::to_wstring(i + 1);
                desc.debugName = name.c_str();
                m_Buffers[i] = new GPUBufferAllocator(desc);
            }
        }

        ~PerFrameBufferRing()
        {
            for(int i = 0; i < kMaxFrames; i++)
            {
                delete m_Buffers[i];
            }
        }

        inline GPUBufferAllocator* GetCurrentBufferByFrameID(uint32_t frameID)
        {
            return m_Buffers[frameID % 3];
        }
        //inline BufferAllocation* GetCurrentBufferAllocationByFrameID();

    private:
        static const uint32_t kMaxFrames = 3;
        GPUBufferAllocator* m_Buffers[kMaxFrames] = {};
        //BufferAllocation mPersistantAllocs[kMaxFrames] = {};
    };
}