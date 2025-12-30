#pragma once
#include "PublicEnum.h"
#include "Math/Math.h"
#include "Core/Object.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/RenderStruct.h"
//#include "Graphics/FrameBufferObject.h"
#include "Resources/ResourceHandle.h"


namespace EngineCore
{
    // 用来描述model Input 或者 shader reflection input
    // todos: shader inputlayout 部分的控制， 目前只是简单把值塞过来
    struct InputLayout
    {
        VertexAttribute type;
        int size;
        int dimension;
        int stride;
        int offset;
        InputLayout(VertexAttribute _type, int _size, int _dimension, int _stride, int _offset)
        {
            type = _type; size = _size; dimension = _dimension; stride = _stride; offset = _offset;
        };
        InputLayout() = default;
        InputLayout(VertexAttribute type) : type(type) {};
    };

    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
    };

    // constantbuffer中的变量记录
    struct ShaderConstantInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // shader中通过反射得到的资源名称、类型
    struct ShaderBindingInfo 
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        int space = 0;
        ShaderBindingInfo (const string& resourceName, ShaderResourceType type, int registerSlot, int size, int space)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size),
            space(space)
        {};
    };


    struct ShaderReflectionInfo
    {
        RootSignatureKey mRootSigKey;

        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mConstantBufferInfo;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};


        // 定义偏移量常量 (方便修改)
        static const int BIT_OFFSET_CBV = 0;
        static const int BIT_OFFSET_SRV = 16;
        static const int BIT_OFFSET_UAV = 48;
        static const int BIT_OFFSET_SAMPLER = 56;
    };

    struct LightData
    {

    };


    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct RenderPacket
    {
        // 为了测试，先用直接塞数据的方式。
        uint64_t sortingKey = 0;
        MeshRenderer* meshRenderer;
        uint32_t vaoID;
        float distanToCamera = 0;
    };

    struct PerDrawHandle
    {
        uint8_t* destPtr;
        uint32_t offset;
        uint32_t size;
    };


    struct DrawRecord
    {
        Material* mat;
        uint32_t vaoID;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, uint32_t vaoID)
            :mat(mat), vaoID(vaoID), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, uint32_t vaoID, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), vaoID(vaoID), perDrawHandle(handle), instanceCount(instCount){}
    };

    class FrameBufferObject;
    struct RenderPassInfo
    {
        std::string passName;
        ResourceHandle<FrameBufferObject> colorAttachment;
        ResourceHandle<FrameBufferObject> depthAttachment;
        ClearFlag clearFlag;
        Vector3 clearColorValue;
        float clearDepthValue;

        //PerDrawData perdrawData;
        Vector2 viewportStartPos;
        Vector2 viewportEndPos;
        
        bool enableBatch = true;
        bool enableIndirectDrawCall = false;
        // drawRecord支持单个绘制，比如后处理等，没有走复杂的逻辑，直接用drawRecord绘制。
        vector<DrawRecord> drawRecordList;
        vector<RenderBatch> renderBatchList;
        RootSigSlot mRootSigSlot;
        inline void Reset()
        {
            clearFlag = ClearFlag::None;
            clearColorValue = Vector3::Zero;
            clearDepthValue = 1.0;
            //perdrawData.Reset();
            viewportStartPos = Vector2::Zero;
            viewportStartPos = Vector2::Zero;
            drawRecordList.clear();
            renderBatchList.clear();
        };
    };

    class RenderPass;
    struct RenderPassAsset
    {
        vector<RenderPass*> renderPasses;
        inline void Clear()
        {
             for (RenderPass* pass : renderPasses) 
                delete pass;
        };
    };

    struct ContextDrawSettings
    {
        SortingCriteria sortingCriteria;
    };

    struct ContextFilterSettings
    {

    };

    struct PerObjectCPUHandler
    {
        uint32_t perObejectIndex = UINT32_MAX;
        inline bool isValid() const {return perObejectIndex != UINT32_MAX;}
    };

    //struct PerObjectCPUData
    //{
    //    AssetID meshID = {};
    //    AssetID matID = {};
    //    uint32_t layermask = 0;
    //    Matrix4x4 worldMatrix = Matrix4x4::Identity;
    //    AABB bounds;
    //    BufferAllocation gpuAllocation;
    //    bool active = false;
    //};



}
