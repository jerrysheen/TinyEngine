#pragma once
#include "PreCompiledHeader.h"
#include "Graphics/Shader.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"
#include "Graphics/Material.h"
#include "Graphics/ModelData.h"
#include "Graphics/Texture.h"


namespace  EngineCore
{
    class RenderAPI
    {
    public:
        static RenderAPI& GetInstance(){ return *s_Instance.get();}
        static bool IsInitialized(){return s_Instance != nullptr;};
        virtual void BeginFrame() = 0;
        virtual void Render() = 0;
        virtual void EndFrame() = 0;
        virtual void OnResize(int width, int height) = 0;
        static void Create();
        virtual Shader* CompileShader(const string& path) = 0;

        virtual void CreateBuffersResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos) = 0;
        virtual void CreateSamplerResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos) = 0;
        virtual void CreateTextureResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos) = 0;
        virtual void CreateUAVResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos) = 0;

        virtual void SetShaderVector(const Material* mat, const ShaderVariableInfo& variableInfo, const Vector3& value) = 0;
        virtual void SetShaderFloat(const Material* mat, const ShaderVariableInfo& variableInfo, float value) = 0;
        virtual void SetShaderVector(const Material* mat, const ShaderVariableInfo& variableInfo, const Vector2& value) = 0;
        virtual void SetShaderMatrix4x4(const Material* mat, const ShaderVariableInfo& variableInfo, const Matrix4x4& value) = 0;
        virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, const Texture& value) = 0;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) = 0;
        virtual void CreateFBO(const string& name, FrameBufferObject* fbodesc) = 0;
        virtual void CreateTextureBuffer(unsigned char* data, Texture* tbdesc) = 0;
        virtual void Submit(const vector<RenderPassInfo*>& renderPassInfos) = 0;

        inline void AddRenderPassInfo(const RenderPassInfo& renderPassInfo){ mRenderPassInfoList.push_back(renderPassInfo); };
        inline void ClearRenderPassInfo(){ mRenderPassInfoList.clear(); };
        
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    };
    
} // namespace  EngineCore

