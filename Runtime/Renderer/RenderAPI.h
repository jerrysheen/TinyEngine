#pragma once
#include "PreCompiledHeader.h"
#include "Graphics/Shader.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"
#include "Graphics/Material.h"
#include "Graphics/ModelData.h"


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
        virtual void SetShaderVector(const Material* mat, const ShaderVariableInfo& variableInfo, const Vector2& value) = 0;
        virtual void SetShaderMatrix4x4(const Material* mat, const ShaderVariableInfo& variableInfo, const Matrix4x4& value) = 0;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) = 0;
        virtual FrameBufferObject* CreateFBO(const string& name, const FrameBufferObject& fbodesc) = 0;

    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    };
    
} // namespace  EngineCore

