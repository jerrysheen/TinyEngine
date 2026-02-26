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
#include "Graphics/IGPUResource.h"
#include "D3D12Texture.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类


namespace EngineCore
{
    class FrameContext;

    class D3D12RenderAPI : public RenderAPI
    {
    public:

        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        virtual void CompileShader(const string& path, Shader* shader) override;
        virtual void CompileComputeShader(const string& path, ComputeShader* csShader) override;

        inline TD3D12Fence* GetFrameFence() { return mFrameFence; };
        virtual IGPUTexture* CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc) override;
        virtual IGPUTexture* CreateRenderTexture(const TextureDesc& textureDesc) override;
        
        // 渲染线程调用接口
        virtual void RenderAPIBeginFrame() override;
        virtual void RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT) override;
        virtual void RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand) override;
        virtual void RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial) override;
        virtual void RenderAPISetBindlessMat(Payload_SetBindlessMat payloadSetBindlessMat) override;
        virtual void RenderAPISetBindLessMeshIB() override;
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
        virtual void RenderAPISetFrameContext(Payload_SetFrameContext setFrameContext) override;
        virtual void RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion) override;
        virtual void RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader) override;
        virtual void RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState) override;
        virtual void RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect) override;

        
        virtual void CreateGlobalConstantBuffer(uint32_t enumID, uint32_t size) override;
        virtual RenderTexture* GetCurrentBackBuffer() override;

        virtual void SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void* value) override;

        TD3D12ConstantBuffer CreateConstantBuffer(uint32_t size);

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
        // 给底层使用的具体资源
        D3D12Texture mBackBuffer[SwapChainBufferCount];
        // 一个壳，上层用，IGPUTexture = mBackBuffer
        RenderTexture mBackBufferProxyRenderTexture;
        D3D12Texture mBackBufferProxy;
        
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
            return mBackBuffer[mCurrBackBuffer].m_Resource.Get();
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

        int GetNextVAOIndex();

        DXGI_FORMAT ConvertD3D12Format(TextureFormat format);
        inline bool IsCompressedFormat(TextureFormat format)
        {
            return format >= TextureFormat::DXT1 && format <= TextureFormat::BC7_SRGB;
        }
        
        // 获取每像素字节数
        inline uint32_t GetBytesPerPixel(TextureFormat format)
        {
            switch(format)
            {
                case TextureFormat::R8G8B8A8: return 4;
                case TextureFormat::D24S8: return 4;
                default: ASSERT(false);
            }
        }

        // RowPitch计算的是一行像素的字节数
        // 比如压缩图，4x4块会被压缩成 xxByte 比如8B
        inline uint32_t CalculateCompressedRowPitch(TextureFormat format, uint32_t width)
        {
            uint32_t blockWidth = (width + 3) / 4;  // 4x4 块对齐
            
            switch(format)
            {
                case TextureFormat::DXT1:  // BC1: 8 bytes per 4x4 block
                    return blockWidth * 8;
                case TextureFormat::DXT3:  // BC2: 16 bytes per 4x4 block
                case TextureFormat::DXT5:  // BC3: 16 bytes per 4x4 block
                case TextureFormat::BC7:
                case TextureFormat::BC7_SRGB:
                    return blockWidth * 16;
                default:
                    ASSERT_MSG(false, "Not Implemented yet");
            }
        }

        // 计算整个切片/mipmap level的总字节数
        inline uint32_t CalculateCompressedSlicePitch(TextureFormat format, uint32_t width, uint32_t height)
        {
            uint32_t blockHeight = (height + 3) / 4;
            return CalculateCompressedRowPitch(format, width) * blockHeight;
        }

        Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
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

        //unordered_map<uint32_t, TD3D12MaterialData> m_DataMap;
        vector<ComPtr<ID3D12RootSignature>> mRootSignatureList;
        ComPtr<ID3D12CommandSignature> mCommandSignature;

        unordered_map<uint32_t, TD3D12ConstantBuffer> mGlobalConstantBufferMap;


        ComPtr<ID3D12PipelineState> currentPSO;
        ComPtr<ID3D12RootSignature> currentRootSignature;
        TD3D12MaterialStateCache materialStateCache;
        UINT currentPerFrameBufferID;
        UINT currentPerPassBufferID;
        FrameContext* mCurrentFrameContext = nullptr;
        uint32_t mCurrentFrameID = 0;
    };

}
