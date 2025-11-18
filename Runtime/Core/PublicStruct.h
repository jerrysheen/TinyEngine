#pragma once
#include "PublicEnum.h"
#include "Math/Math.h"
#include "Core/Object.h"
#include "Renderer/RenderCommand.h"
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
        // todo: 确定这个地方是用vector还是直接单个对象
        ShaderStageType type;
        vector<ShaderBindingInfo > mTextureInfo;
        vector<ShaderBindingInfo > mSamplerInfo;
        vector<ShaderBindingInfo > mBufferInfo;
        vector<ShaderBindingInfo > mUavInfo;

        ShaderReflectionInfo(){};
        unordered_map<string, ShaderConstantInfo> mPerDrawConstantBuffferReflectionInfo;
        unordered_map<string, ShaderConstantInfo> mPerMaterialConstantBuffferReflectionInfo;
        uint32_t perDrawBufferSize = 0;
        uint32_t perMaterialBufferSize = 0;
    };

    struct LightData
    {

    };


    // 前向声明，防止循环引用。
    class Transform;
    class MeshRenderer;
    class MeshFilter;

    struct VisibleItem
    {
        // 为了测试，先用直接塞数据的方式。
        MeshRenderer* meshRenderer;
        MeshFilter* meshFilter;
        Transform* transform;
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
        ModelData* model;

        PerDrawHandle perDrawHandle;
        uint32_t instanceCount = 1;

        DrawRecord(Material* mat, ModelData* data)
            :mat(mat), model(data), perDrawHandle{0,0}, instanceCount(1) {}
        DrawRecord(Material* mat, ModelData* data, const PerDrawHandle& handle, uint32_t instCount = 1)
            :mat(mat), model(data), perDrawHandle(handle), instanceCount(instCount){}
    };
    
    //struct PerDrawData
    //{
    //    Matrix4x4 objectToWorldMatrix;
    //    Matrix4x4 worldToViewMatrix;
    //    Matrix4x4 viewToProjectionMatrix;

    //    inline void Reset()
    //    {
    //        objectToWorldMatrix = Matrix4x4::Identity;
    //        worldToViewMatrix = Matrix4x4::Identity;
    //        viewToProjectionMatrix = Matrix4x4::Identity;
    //    };
    //};

    class FrameBufferObject;
    struct RenderPassInfo
    {
        ResourceHandle<FrameBufferObject> colorAttachment;
        ResourceHandle<FrameBufferObject> depthAttachment;
        ClearFlag clearFlag;
        Vector3 clearColorValue;
        float clearDepthValue;

        //PerDrawData perdrawData;
        Vector2 viewportStartPos;
        Vector2 viewportEndPos;
        
        vector<DrawRecord> drawRecordList;
        RenderDataFrenquent mRenderDataFrenquent;
        inline void Reset()
        {
            clearFlag = ClearFlag::None;
            clearColorValue = Vector3::Zero;
            clearDepthValue = 1.0;
            //perdrawData.Reset();
            viewportStartPos = Vector2::Zero;
            viewportStartPos = Vector2::Zero;
            drawRecordList.clear();
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


}