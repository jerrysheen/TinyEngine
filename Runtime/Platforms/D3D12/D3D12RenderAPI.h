#pragma once
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#include "Renderer/RenderAPI.h"
#include "d3dUtil.h"
#include "Graphics/Shader.h"
#include "Graphics/ComputeShader.h"
#include "PreCompiledHeader.h"
#include "D3D12Struct.h"
#include "Core/PublicStruct.h"
#include "Graphics/Texture.h"
#include "Core/InstanceID.h"
#include "Core/PublicEnum.h"
#include "D3D12PerDrawAllocator.h"
#include "Graphics/IGPUBuffer.h"

namespace EngineCore
{
    class D3D12RenderAPI : public RenderAPI
    {
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        virtual void CompileShader(const string& path, Shader* shader) override;
        virtual void CompileComputeShader(const string& path, ComputeShader* csShader) override;
        virtual void CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) override;
        virtual void CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos) override;

        inline TD3D12Fence* GetFrameFence() { return mFrameFence; };
        // todo: maybe可以清理成模板。
        virtual void SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID) override;
        virtual void SetUpMesh(ModelData* data, bool isStatic = true) override;
        virtual void CreateFBO(FrameBufferObject* fbodesc) override;
        virtual void CreateTextureBuffer(unsigned char* data, Texture* tbdesc) override;
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
        virtual void RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState) override;
        virtual void RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect) override;
        virtual void RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB) override;
        virtual void RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport) override;
        virtual void RenderAPIWindowResize(Payload_WindowResize payloadWindowResize) override;
        virtual void RenderAPISubmit() override;
        virtual void RenderAPIPresentFrame() override;
        virtual void RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData) override;
        virtual void RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd) override;
        virtual void RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData) override;
        virtual void RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;

        TD3D12DescriptorHandle GetTextureSrvHanle(uint32_t textureID);
        TD3D12FrameBuffer* GetFrameBuffer(uint32_t bufferID, bool isBackBuffer = false);
        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) override;
        virtual void CreateGlobalTexHandler(uint32_t texID) override;


        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) override;
        virtual PerDrawHandle AllocatePerDrawData(uint32_t size) override;

        TD3D12ConstantBuffer CreateConstantBuffer(uint32_t size);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        //Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
        TD3D12FrameBuffer mBackBuffer[SwapChainBufferCount];
        //UINT64 mCurrentFence = 0;
        //Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
        TD3D12Fence* mFrameFence;
        TD3D12Fence* mImediatelyFence;

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::CurrentBackBufferView()const
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
                mCurrBackBuffer,
                mRtvDescriptorSize);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderAPI::DepthStencilView()const
        {
            return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
        }

        ID3D12Resource* D3D12RenderAPI::CurrentBackBuffer()const
        {
            return mBackBuffer[mCurrBackBuffer].resource.Get();
        }

        void SignalFence(TD3D12Fence* mFence);
        void WaitForFence(TD3D12Fence* mFence);
        void WaitForRenderFinish(TD3D12Fence* mFence);
        virtual void WaitForGpuFinished() override { WaitForFence(mFrameFence); }

        ComPtr<ID3D12PipelineState> psoObj;
        ComPtr<ID3D12RootSignature> rootSignature;
        
        void ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function);
        
        virtual IGPUBuffer* CreateBuffer(const BufferDesc& desc, void* data) override;
        virtual void UploadBuffer(IGPUBuffer* bufferResource, uint32_t offset, void* data, uint32_t size) override;
        static D3D12_RESOURCE_STATES GetResourceState(BufferResourceState state);
    private:

        bool InitDirect3D();
        void InitFence();
        void InitDescritorHeap();
        void InitCommandObject();
        void InitSwapChain();
        void InitRenderTarget();
        void InitPerDrawLargeBuffer();

        int GetNextVAOIndex();
        TD3D12VAO& GetAvaliableModelDesc();


        Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
        Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
        
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;




        // 屏幕后台缓冲区图像格式
		DXGI_FORMAT mPresentBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// 默认的纹理和FrameBuffer色彩空间
		const DXGI_FORMAT mDefaultImageFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// 4X MSAA质量等级
		UINT m4xMSAAQuality = 0;
		UINT msaaSamplesCount = 4;


        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mImediatelyCmdListAlloc;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mImediatelyCommandList;
        
        DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        bool      m4xMsaaState = false;    // 4X MSAA enabled
        UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
        
        Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

        D3D12_VIEWPORT mScreenViewport; 
        D3D12_RECT mScissorRect;
        int mClientWidth = 1280;
        int mClientHeight = 720;

        unordered_map<uint32_t, TD3D12MaterialData> m_DataMap;
        vector<ComPtr<ID3D12RootSignature>> mRootSignatureList;
        unordered_map<uint32_t, TD3D12TextureBuffer> m_TextureBufferMap;
        ComPtr<ID3D12CommandSignature> mCommandSignature;
        unordered_map<uint32_t, TD3D12VAO> VAOMap;

        unordered_map<uint32_t, TD3D12ConstantBuffer> mGlobalConstantBufferMap;
        unordered_map<uint32_t, TD3D12TextureHander> mGlobalTexHandlerMap;

        std::unique_ptr<D3D12PerDrawAllocator> mPerDrawAllocator;

        ComPtr<ID3D12PipelineState> currentPSO;
        ComPtr<ID3D12RootSignature> currentRootSignature;
        UINT currentPerFrameBufferID;
        UINT currentPerPassBufferID;
    };

}