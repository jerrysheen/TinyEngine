#pragma once
#include <iostream>
#include "Utils/HashCombine.h"
#include "Core/PublicEnum.h"
#include "Renderer/RenderStruct.h"
#include <tuple>
#include <vector>
#include "Graphics/IGPUResource.h"
#include "Graphics/Mesh.h"
#include "Renderer/FrameTicket.h"

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
        //kDrawIndexed = 7,
        kSetMaterial = 8,
        kConfigureRT = 9,
        kWindowResize = 10,
        kIssueEditorGUIDraw = 11,
        kSetPerDrawData = 12,
        kDrawInstanced = 13,
        kSetPerFrameData = 14,
        kSetPerPassData = 15,
        kCopyBufferRegion = 16,
        kDispatchComputeShader = 17,
        kSetBufferResourceState = 18,
        kDrawIndirect = 19,
        kSetBindlessMat = 20,
        kSetBindLessMeshIB = 21,
        kSetFrame = 22,
        kCopyBufferStaged = 23,
        kUAVBarrier = 24,
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
        uint32_t shaderInstanceID = 0;
        RootSignatureKey rootSignatureKey;
        // depth stencil state:
        bool enableDepthTest = true;
        bool enableDepthWrite = true;
#ifdef REVERSE_Z
        DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::GREATEQUAL;
#else
        DepthComparisonFunc depthComparisonFunc = DepthComparisonFunc::LEQUAL;
#endif
        bool isBindLessMaterial = false;
        // blend mode
        bool enableBlend = false;
        BlendState srcBlend = (BlendState)0;
        BlendState destBlend = (BlendState)0;
        uint32_t hashID = 0;
        uint32_t GetHash()
        {
            if(hashID != 0) return hashID;
            hashID = 0;
            HashCombine(hashID, shaderInstanceID);
            HashCombine(hashID, static_cast<uint32_t>(enableDepthTest));
            HashCombine(hashID, static_cast<uint32_t>(enableDepthWrite));
            HashCombine(hashID, static_cast<uint32_t>(depthComparisonFunc));
            HashCombine(hashID, static_cast<uint32_t>(enableBlend));
            HashCombine(hashID, static_cast<uint32_t>(srcBlend));
            HashCombine(hashID, static_cast<uint32_t>(destBlend));
            return hashID;
        }

        void Reset() 
        {
            shaderInstanceID = 0;
            rootSignatureKey;
            enableDepthTest = true;
            enableDepthWrite = true;
#ifdef REVERSE_Z
            depthComparisonFunc = DepthComparisonFunc::GREATEQUAL;
#else
            depthComparisonFunc = DepthComparisonFunc::LEQUAL;
#endif
            enableBlend = false;
            srcBlend = (BlendState)0;
            destBlend = (BlendState)0;
            hashID = 0;
        }
    };

    struct PSODesc
    {
        MaterailRenderState matRenderState;

        TextureFormat colorAttachment = TextureFormat::EMPTY;
        TextureFormat depthAttachment = TextureFormat::EMPTY;
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
        ClearValue() : colorValue(Vector3::Zero), depthValue(1.0f), flags(ClearFlag::None) {};
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
        Mesh* mesh;
    };

    struct Payload_SetPerFrameData
    {
        UINT perFrameBufferID;
    };

    struct Payload_SetPerPassData
    {
        UINT perPassBufferID;
    };

    struct Payload_SetFrame
    {
        FrameTicket* frameTicket = nullptr;
        uint32_t frameID = 0;
    };

    struct alignas(16) CopyOp
    {
        IGPUBuffer* srcUploadBuffer;
        IGPUBuffer* destDefaultBuffer;
        uint32_t srcOffset;
        uint32_t dstOffset;
        uint32_t size;
    };

    struct Payload_CopyBufferRegion
    {
        IGPUBuffer* srcUploadBuffer;
        IGPUBuffer* destDefaultBuffer;
        CopyOp* copyList;
        uint32_t count;
    };
    

    // 绑定渲染材质，
    class Shader;
    struct Payload_SetMaterial 
    {
        Material* mat;
        Shader* shader;
    };
    
    struct Payload_ConfigureRT
    {
        IGPUTexture* colorAttachment = 0;
        IGPUTexture* depthAttachment = 0;
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
        Mesh* mesh;
        int count;
    };

    struct Payload_WindowResize 
    {
        int width;
        int height;
    };

    struct Payload_IssueEditorGUIDraw
    {
    };

    struct SissorRect
    {
        float x, y, w, h;
    };

    struct Payload_SetPerDrawData
    {
        uint32_t perDrawOffset;
        uint32_t perDrawSize;
    };

    struct Payload_DrawInstancedCommand
    {
        Mesh* mesh;
        int count;
        uint32_t perDrawOffset;
        uint32_t perDrawStride;
    };

    class ComputeShader;
    struct ComputeBufferDispatchBindingSpan
    {
        IGPUBuffer** buffers = nullptr;
        uint32_t count = 0;
    };

    struct TextureBinding
    {
        IGPUTexture* texture;
        bool isUAV = false;
        uint32_t mipLevel = 0;
        TextureBinding() {}
        TextureBinding(IGPUTexture* texture, bool isuav, uint32_t mip) :
            texture(texture), isUAV(isuav), mipLevel(mip) {
        }
    };

    struct ComputeTextureDispatchBindingSpan
    {
        TextureBinding* bindingInfo = nullptr;
        uint32_t count = 0;
    };

    struct Payload_DispatchComputeShader
    {
        ComputeShader* csShader = nullptr;
        uint32_t frameID = 0;
        ComputeBufferDispatchBindingSpan bufferBindings;
        ComputeTextureDispatchBindingSpan textureBindings;
        uint32_t groupX = 0;
        uint32_t groupY = 0;
        uint32_t groupZ = 0;
    };

    struct Payload_SetBufferResourceState
    {
        IGPUResource* resource;
        BufferResourceState state;
    };

    struct Payload_UAVBarrier
    {
        IGPUResource* resource = nullptr;
    };

    struct Payload_DrawIndirect
    {
        // 这个payload只关心， 我绘制哪几个IndirectDraw，怎么找到，
        IGPUBuffer* indirectArgsBuffer;
        IGPUBuffer* indirectDrawCountBuffer;
        uint32_t startIndex;
        uint32_t count;
        uint32_t startIndexInInstanceDataBuffer;
    };

    struct Payload_SetBindlessMat
    {
        Material* mat;
    };

    struct Payload_SetBindLessMeshIB
    {
        uint32_t id;
    };

    struct Payload_CopyBufferStaged
    {
        CopyOp copyOp;
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
        Payload_WindowResize onWindowResize;
        Payload_IssueEditorGUIDraw onGUIDraw;
        Payload_SetPerDrawData setPerDrawData;
        Payload_DrawInstancedCommand setDrawInstanceCmd;
        Payload_SetPerFrameData setPerFrameData;
        Payload_SetPerPassData setPerPassData;
        Payload_SetFrame setFrame;
        Payload_CopyBufferRegion copyBufferRegion;
        Payload_DispatchComputeShader dispatchComputeShader;
        Payload_SetBufferResourceState setBufferResourceState;
        Payload_UAVBarrier uavBarrier;
        Payload_DrawIndirect setDrawIndirect;
        Payload_SetBindlessMat setBindlessMat;
        Payload_SetBindLessMeshIB SetBindLessMeshIB;
        Payload_CopyBufferStaged copyBufferStaged;
        CommandData() {};
    };

    struct DrawCommand 
    {
        RenderOp op{ RenderOp::kInvalid };
        CommandData data;
    public:
        DrawCommand(){};
    };

    struct CommandStream
    {
        std::vector<DrawCommand> commads;
    };

};
