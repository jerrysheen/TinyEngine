#pragma once
#include "Graphics/Shader.h"
#include "Graphics/ComputeShader.h"
#include "Serialization/MetaData.h"
#include "Core/PublicStruct.h"
#include "Graphics/Material.h"
#include "Graphics/ModelData.h"
#include "Graphics/Texture.h"
#include "Renderer/RenderCommand.h"
#include "Graphics/IGPUBuffer.h"

namespace  EngineCore
{
    class RenderAPI
    {
    public:
        static RenderAPI* GetInstance(){ return s_Instance.get();}
        static bool IsInitialized(){return s_Instance != nullptr;};
       
        static void Create();
        virtual Shader* CompileShader(const string& path, Shader* shader) = 0;
        virtual ComputeShader* CompileComputeShader(const string& path, ComputeShader* csShader) = 0;
        
        virtual void CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;
        virtual void CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) = 0;

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
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) = 0;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) = 0;
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) = 0;
        virtual void CreateGlobalTexHandler(uint32_t texID) = 0;
        virtual PerDrawHandle AllocatePerDrawData(uint32_t size) = 0;
        
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) = 0;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) = 0;
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) = 0;
        virtual void UploadBuffer(IGPUBuffer* buffer, uint32_t offset, void* data, uint32_t size) = 0;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) = 0;

        template<typename T>
        void SetGlobalValue(uint32_t bufferID, uint32_t offset, T* value)
        {
            uint32_t size = sizeof(T);
            SetGlobalDataImpl(bufferID, offset, size, static_cast<void*>(value));
        }
        virtual void WaitForGpuFinished() = 0;
    public:
        static std::unique_ptr<RenderAPI> s_Instance;
    protected:
        vector<RenderPassInfo> mRenderPassInfoList;
    private:
        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) = 0;

    };
    
} // namespace  EngineCore

