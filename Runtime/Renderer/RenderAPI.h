#pragma once
#include "PreCompiledHeader.h"
#include "Graphics/Shader.h"
#include "Core/Resources.h"
#include "Core/PublicStruct.h"
#include "Graphics/Material.h"
#include "Graphics/ModelData.h"
#include "Graphics/Texture.h"
#include "Renderer/RenderCommand.h"

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
        virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID) = 0;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) = 0;
        virtual void CreateFBO(FrameBufferObject* fbodesc) = 0;
        virtual void CreateTextureBuffer(unsigned char* data, Texture* tbdesc) = 0;
        virtual void Submit(const vector<RenderPassInfo*>& renderPassInfos) = 0;

        virtual void GetOrCreatePSO(const Material& mat, const RenderPassInfo &passinfo) = 0;
        inline void AddRenderPassInfo(const RenderPassInfo& renderPassInfo){ mRenderPassInfoList.push_back(renderPassInfo); };
        inline void ClearRenderPassInfo(){ mRenderPassInfoList.clear(); };
        
        virtual void RenderAPIBeginFrame() = 0;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) = 0;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) = 0;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) = 0;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) = 0;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) = 0;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) = 0;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) = 0;
        virtual void RenderAPIEndFrame() = 0;

    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    };
    
} // namespace  EngineCore

