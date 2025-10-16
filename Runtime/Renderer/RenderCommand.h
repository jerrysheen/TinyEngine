#pragma once
#include <iostream>
#include "Utils/HashCombine.h"
#include "Core/PublicEnum.h"


namespace EngineCore
{
    enum class RenderOp : uint8_t
    {
        kInvalid = 0,
        kBeginFrame = 1,
        kEndFrame = 2,
        kSetRenderState = 3,
        kSetVBIB = 4,
        kSetViewPort = 5,
        kSetSissorRect = 6,
        kDrawIndexed = 7,
        kSetMaterial = 8,
        kConfigureRT = 9,
    };

    enum class DepthComparisonFunc : uint8_t
    {
        LESS = 0,
        LEQUAL = 1,
        EQUAL = 2,
        GREATEQUAL = 3,
        GREAT = 4
    };

    enum class CullMode : uint8_t
    {
        CULLOFF = 0,
        CULLBACK = 1,
        CULLFRONT = 2,
    };

    enum class BlendState : uint8_t
    {
        SRCALHPHA = 0,
        ONEMINUSSRCALPHA = 2,
        ONE = 3,
        ZERO = 4,
    };

    // 和材质相关，关联材质后可以做合批操作。
    struct MaterailRenderState
    {
        uint32_t shaderInstanceID;

        // depth stencil state:
        bool enableDepthTest = true;
        DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::LEQUAL;
        bool enableDepthWrite = true;

        // blend mode
        bool enableBlend = false;
        BlendState srcBlend;
        BlendState destBlend;
    };

    struct PSODesc
    {
        MaterailRenderState matRenderState;

        TextureFormat colorAttachment;
        TextureFormat depthAttachment;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, matRenderState.shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.enableBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.srcBlend));
            HashCombine(hashID, static_cast<uint32_t>(matRenderState.destBlend));
            HashCombine(hashID, static_cast<uint32_t>(colorAttachment));
            HashCombine(hashID, static_cast<uint32_t>(depthAttachment));
            return hashID;
        };
    private:
        uint32_t hashID = 0;
    };

    enum class ClearFlag : uint8_t
    {
        None = 0,
        Color = 1,
        Depth = 2,
        All = 3
    };

    struct ClearValue
    {
        // todo :: vector3 -> vector4
        Vector3 colorValue;
        float depthValue;
        ClearFlag flags;
        ClearValue(Vector3 color, float depth, ClearFlag flag):colorValue(color), depthValue(depth), flags(flag){};
    };

    struct Payload_BeginFrame { uint64_t frameIndex; };
    struct Payload_EndFrame { uint64_t frameIndex; };

    // todo : mrt渲染，
    struct Payload_SetRenderState 
    {
        PSODesc psoDesc;
    };

    struct Payload_SetVBIB 
    {
        uint32_t vaoId;
    };

    // 绑定渲染材质，
    struct Payload_SetMaterial 
    {
        uint32_t matId;
    };
    
    struct Payload_ConfigureRT
    {
        uint32_t colorAttachment = 0;
        uint32_t depthAttachment = 0;
        bool isBackBuffer;
        ClearValue clearValue;
    };

    struct Payload_SetViewPort 
    {
        float x, y, w, h, minDepth, maxDepth;
    };

    struct Payload_SetSissorRect 
    {
        float x, y, w, h;
    };

    struct Payload_DrawCommand 
    {
        uint32_t vaoID;
        int count;
    };

    struct SissorRect
    {
        float x, y, w, h;
    };

    union CommandData 
    {
        Payload_BeginFrame beginFrame;
        Payload_EndFrame endFrame;
        Payload_SetRenderState setRenderState;
        Payload_SetVBIB setVBIB;
        Payload_SetViewPort setViewPort;
        Payload_SetMaterial setMaterial;
        Payload_ConfigureRT configureRT;
        Payload_DrawCommand setDrawCmd;
        Payload_SetSissorRect setSissorRect;
        CommandData() {};
    };

    struct DrawCommand 
    {
        RenderOp op{ RenderOp::kInvalid };
        CommandData data;
    public:
        DrawCommand(){};
    };

};