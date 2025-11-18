#pragma once
#include "Graphics/Shader.h"
#include "Serialization/MetaData.h"
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
        static RenderAPI* GetInstance(){ return s_Instance.get();}
        static bool IsInitialized(){return s_Instance != nullptr;};
        virtual void BeginFrame() = 0;
        virtual void Render() = 0;
        virtual void EndFrame() = 0;
        static void Create();
        virtual Shader* CompileShader(const string& path, Shader* shader) = 0;

        virtual void CreateMaterialConstantBuffers(const Material* mat, uint32_t bufferSize) = 0;
        virtual void CreateMaterialSamplerSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;
        virtual void CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;
        virtual void CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;

        virtual void SetShaderVector(const Material* mat, const ShaderConstantInfo& variableInfo, const Vector3& value) = 0;
        virtual void SetShaderFloat(const Material* mat, const ShaderConstantInfo& variableInfo, float value) = 0;
        virtual void SetShaderVector(const Material* mat, const ShaderConstantInfo& variableInfo, const Vector2& value) = 0;
        virtual void SetShaderMatrix4x4(const Material* mat, const ShaderConstantInfo& variableInfo, const Matrix4x4& value) = 0;
        virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID) = 0;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) = 0;
        virtual void CreateFBO(FrameBufferObject* fbodesc) = 0;
        virtual void CreateTextureBuffer(unsigned char* data, Texture* tbdesc) = 0;

        //virtual void GetOrCreatePSO(const Material& mat, const RenderPassInfo &passinfo) = 0;
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
        virtual void RenderAPISubmit() = 0;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) = 0;
        virtual void RenderAPIPresentFrame() = 0;

        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) = 0;
        virtual void CreateGlobalTexHandler(uint32_t texID) = 0;
        virtual PerDrawHandle AllocatePerDrawData(uint32_t size) = 0;
        
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) = 0;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) = 0;
        template<typename T>
        void SetGlobalValue(uint32_t bufferID, uint32_t offset, T* value)
        {
            uint32_t size = sizeof(T);
            SetGlobalDataImpl(bufferID, offset, size, static_cast<void*>(value));
        }
        
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    private:
        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) = 0;

    };
    
} // namespace  EngineCore

