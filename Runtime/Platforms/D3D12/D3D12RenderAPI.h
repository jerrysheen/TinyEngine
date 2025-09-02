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



namespace EngineCore
{
    class D3D12RenderAPI : public RenderAPI
    {
    public:
        virtual void BeginFrame() override;
        virtual void Render() override;
        virtual void EndFrame() override;
       
        D3D12RenderAPI();
        ~D3D12RenderAPI(){};

        Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;
        const int MAX_FRAME_INFLIAGHT = 3;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT mCurrBackBuffer = 0;
        static const int SwapChainBufferCount = 3;
        Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

        UINT64 mCurrentFence = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

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
            return mSwapChainBuffer[mCurrBackBuffer].Get();
        }

        void SignalFence();
        void WaitForFence();
        void WaitForRenderFinish();
    private:
        bool InitDirect3D();
        void InitFence();
        void InitDescritorHeap();
        void InitCommandObject();
        void InitSwapChain();
        void InitRenderTarget();




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
        
        DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        bool      m4xMsaaState = false;    // 4X MSAA enabled
        UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
        
        Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

        D3D12_VIEWPORT mScreenViewport; 
        D3D12_RECT mScissorRect;
        int mClientWidth = 1280;
        int mClientHeight = 720;
    };

}