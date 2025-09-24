#pragma once
#include "PublicEnum.h"
#include "Math/Math.h"

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
        
        InputLayout(VertexAttribute type) : type(type) {};
    };

    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
    };

    // constantbuffer中的变量记录
    struct ShaderVariableInfo
    {
        string variableName;
        ShaderVariableType type;
        int bufferIndex;
        int size;
        int offset;
    };

    // shader中通过反射得到的资源名称、类型
    struct ShaderResourceInfo
    {
        string resourceName;
        ShaderResourceType type;
        int registerSlot;              
        int size = 0;                  // 对CB有意义，其他资源可为0
        struct ShaderResourceInfo(const string& resourceName, ShaderResourceType type, int registerSlot, int size)
            : resourceName(resourceName), type(type), registerSlot(registerSlot), size(size)
        {};
    };

    struct ShaderReflectionInfo
    {
        ShaderStageType type;
        vector<ShaderResourceInfo> mTextureInfo;
        vector<ShaderResourceInfo> mSamplerInfo;
        vector<ShaderResourceInfo> mBufferInfo;
        vector<ShaderResourceInfo> mUavInfo;

        ShaderReflectionInfo(){};
        unordered_map<string, ShaderVariableInfo> mShaderStageVariableInfoMap;

    };

    struct FrameBufferObject
    {
        string name;
        TextureDimension dimension;
        TextureFormat format;
        float width;
        float height;
    };


    struct LightData
    {

    };


    // 前向声明，防止循环引用。
    class Material;
    class ModelData;

    struct VisibleItem
    {
        // 为了测试，先用直接塞数据的方式。
        Material* mat;
        ModelData* model;
    };


    struct DrawRecord
    {
        // temp 方案：
        // 为了测试，先用直接塞数据的方式。
        Material* mat;
        ModelData* model;
        DrawRecord(Material* mat, ModelData* data):mat(mat),model(data){};
    };
    
    struct PerDrawData
    {
        Matrix4x4 objectToWorldMatrix;
        Matrix4x4 worldToViewMatrix;
        Matrix4x4 viewToProjectionMatrix;

        inline void Reset()
        {
            objectToWorldMatrix = Matrix4x4::Identity;
            worldToViewMatrix = Matrix4x4::Identity;
            viewToProjectionMatrix = Matrix4x4::Identity;
        };
    };

    struct RenderPassInfo
    {
        FrameBufferObject* colorAttachment;
        FrameBufferObject* depthAttachment;
        ClearFlag clearFlag;
        Vector3 clearColorValue;
        float clearDepthValue;

        PerDrawData perdrawData;
        Vector2 viewportStartPos;
        Vector2 viewportEndPos;
        
        vector<DrawRecord> drawRecordList;

        inline void Reset()
        {
            colorAttachment = nullptr;
            depthAttachment = nullptr;
            clearFlag = ClearFlag::NONE;
            clearColorValue = Vector3::Zero;
            clearDepthValue = 1.0;
            perdrawData.Reset();
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