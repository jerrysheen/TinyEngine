#pragma once
#include <iostream>

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
        uint32_t psoId;
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