#include "PreCompiledHeader.h"
#include "GeometryManager.h"
#include "Core/PublicStruct.h"
#include "Renderer/RenderAPI.h"

namespace EngineCore
{
    GeometryManager* GeometryManager::s_Instance = nullptr;

    GeometryManager::GeometryManager()
    {
        BufferDesc vbDesc;
        vbDesc.debugName = L"GlobalVertexBuffer";
        vbDesc.memoryType = BufferMemoryType::Default;
        vbDesc.size = 1024 * 1024 * 200;
        vbDesc.stride = sizeof(Vertex);
        vbDesc.usage = BufferUsage::StructuredBuffer;
        m_GlobalVertexBufferAllocator = new GPUBufferAllocator(vbDesc);

        BufferDesc ibDesc;
        ibDesc.debugName = L"GlobalIndexBuffer";
        ibDesc.memoryType = BufferMemoryType::Default;
        ibDesc.size = 1024 * 1024 * 100;
        ibDesc.stride = sizeof(uint32_t);
        ibDesc.usage = BufferUsage::StructuredBuffer;
        m_GLobalIndexBufferAllocator = new GPUBufferAllocator(ibDesc);
    }

    MeshBufferAllocation *GeometryManager::AllocateVertexBuffer(void *data, int size)
    {
        BufferAllocation alloc = m_GlobalVertexBufferAllocator->Allocate(size);
        RenderAPI::GetInstance()->UploadBuffer(alloc.buffer, alloc.offset, data, size);
        MeshBufferAllocation* meshBufferAllocation = new MeshBufferAllocation(
            alloc.buffer,
            alloc.gpuAddress,
            alloc.offset / sizeof(Vertex),
            size,
            sizeof(Vertex)
        );

        return meshBufferAllocation;
    }

    MeshBufferAllocation *GeometryManager::AllocateIndexBuffer(void *data, int size)
    {
        BufferAllocation alloc = m_GLobalIndexBufferAllocator->Allocate(size);
        RenderAPI::GetInstance()->UploadBuffer(alloc.buffer, alloc.offset, data, size);

        RenderAPI::GetInstance()->UploadBuffer(alloc.buffer, alloc.offset, data, size);
        MeshBufferAllocation* meshBufferAllocation = new MeshBufferAllocation(
            alloc.buffer,
            alloc.gpuAddress,
            alloc.offset / sizeof(uint32_t),
            size,
            sizeof(uint32_t)
        );
        return meshBufferAllocation;
    }

    void GeometryManager::FreeVertexAllocation(MeshBufferAllocation *meshBufferAllocation)
    {
        BufferAllocation allocation;
        allocation.size = meshBufferAllocation->size;
        allocation.offset = meshBufferAllocation->offset * sizeof(Vertex);
        m_GlobalVertexBufferAllocator->Free(allocation);
    }

    void GeometryManager::FreeIndexAllocation(MeshBufferAllocation *meshBufferAllocation)
    {
        BufferAllocation allocation;
        allocation.size = meshBufferAllocation->size;
        allocation.offset = meshBufferAllocation->offset * sizeof(uint32_t);
        m_GLobalIndexBufferAllocator->Free(allocation);
    }
};