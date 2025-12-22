#pragma once
#include <iostream>
#include "Math/AABB.h"
#include "Math/Vector2.h"

namespace EngineCore
{
    class IGPUBuffer;
    struct BufferAllocation
    {
        IGPUBuffer* buffer = nullptr;
        uint64_t gpuAddress = 0;
        uint8_t* cpuAddress = nullptr;
        uint64_t offset =0;
        uint64_t size = 0;
        bool isValid = false;
    };

    struct RootSigBindingInfo
    {
        uint32_t RegisterIndex;
        uint32_t RegisterSpace;
    };

    class Material;
    class ModelData;
    struct RenderBatch
    {
        BufferAllocation alloc;
        
        Material* mat;
        uint32_t vaoID;
        uint32_t index;
        uint32_t instanceCount;
    };

    enum class RootSigSlot : UINT
    {
        DrawIndiceConstant = 0,
        PerFrameData = 1,
        PerPassData = 2,
        AllObjectData = 3,
        AllMaterialData = 4, // SRV t1 (StructuredBuffer) - 替代原来的 PerMaterialData
        PerDrawInstanceObjectsList = 5, // SRV t1 (StructuredBuffer) - 替代原来的 PerMaterialData
        Textures        = 6, // Descriptor Table (t2...)
        Count
    };

    inline constexpr RootSigBindingInfo GetRootSigBinding(RootSigSlot slot)
    {
        switch(slot)
        {
            case RootSigSlot::DrawIndiceConstant : return {0, 0};
            case RootSigSlot::PerFrameData : return {1, 0};
            case RootSigSlot::PerPassData : return {2, 0};
            case RootSigSlot::AllObjectData : return {0, 1};
            case RootSigSlot::AllMaterialData : return {1, 1};
            case RootSigSlot::PerDrawInstanceObjectsList : return {2, 1};
            //case RootSigSlot::Textures           : return {0, 2};
            default: return {0, 0};
        }
    }

    struct RootSignatureKey
    {
        // Space 0: 用于传统绑定
        uint64_t Space0Mask = 0;
        // Space 1: 通常用于 bindless 资源
        uint64_t Space1Mask = 0;

        RootSignatureKey() = default;

        bool operator==(const RootSignatureKey& other) const
        {
            return Space0Mask == other.Space0Mask && Space1Mask == other.Space1Mask;
        }

        std::size_t operator()(const RootSignatureKey& k) const
        {
            // 获取两个 uint64 的 hash 值
            std::size_t h1 = std::hash<uint64_t>{}(k.Space0Mask);
            std::size_t h2 = std::hash<uint64_t>{}(k.Space1Mask);

            // 【关键】Hash Combine 算法 (参考 Boost 实现)
            // 将 h2 混合进 h1，防止比如 (1, 1) 和 (0, 0) 这种简单异或产生的碰撞
            // 0x9e3779b9 是黄金分割比，能让 bit 分布更均匀
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct GPUCullingDesc
    {
        // frustum cb
        void* cullingParamsCB;
        // aabb structuredBuffer;
        void* aabbBufferSRV;
        // instandeData uav
        void* instanceDataUav;
        // indirectArgsUAV uav
        void* indirectArgsUAV;
    };

}