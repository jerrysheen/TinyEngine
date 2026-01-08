#include "PreCompiledHeader.h"
#include "D3D12RenderAPI.h"
#include "Managers/WindowManager.h"
#include "Serialization/MetaData.h"
#include "Serialization/MetaLoader.h"
#include "Core/PublicStruct.h"
#include "D3D12DescManager.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类
#include "D3D12PSO.h"
#include "Renderer/RenderUniforms.h"
#include "Renderer/RenderStruct.h"
#include "D3D12RootSignature.h"
#include "Graphics/GPUSceneManager.h"
#include "D3D12Buffer.h"
#include "D3D12ShaderUtils.h"
#include "Renderer/RenderEngine.h"
#include "Graphics/IGPUResource.h"
#include "D3D12Texture.h"

namespace EngineCore
{
    D3D12RenderAPI::D3D12RenderAPI()
    {
        InitDirect3D();
        InitFence();
        InitDescritorHeap();
        InitCommandObject();
        InitSwapChain();

        InitRenderTarget();
        D3D12DescManager::Create(md3dDevice);
        //m_DataMap = unordered_map<uint32_t, TD3D12MaterialData>();


    }

    bool D3D12RenderAPI::InitDirect3D()
    {
        #if defined(DEBUG) || defined(_DEBUG) 
        // Enable the D3D12 debug layer.
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
    #endif

        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

        // Try to create hardware device.
        HRESULT hardwareResult = D3D12CreateDevice(
            nullptr,             // default adapter
            D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&md3dDevice));

        // Fallback to WARP device.
        if(FAILED(hardwareResult))
        {
            ComPtr<IDXGIAdapter> pWarpAdapter;
            ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

            ThrowIfFailed(D3D12CreateDevice(
                pWarpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&md3dDevice)));
        }

    #ifdef _DEBUG
        //LogAdapters();
    #endif
        return true;
    }

    void D3D12RenderAPI::InitFence()
    {
        mFrameFence = new TD3D12Fence();
        mImediatelyFence = new TD3D12Fence();
        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFrameFence->mFence)));
        mFrameFence->mCurrentFence = 0;

        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mImediatelyFence->mFence)));
        mFrameFence->mCurrentFence = 0;

    }
    
    void D3D12RenderAPI::InitRenderTarget()
    {
        auto [width, height] = WindowManager::GetInstance()->GetWindowSize();
        mClientWidth = width;
        mClientHeight = height;
        // Flush before changing any resources.
	    WaitForFence(mFrameFence);
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mBackBuffer[i].m_Resource.Reset();
        mDepthStencilBuffer.Reset();
        
        // Resize the swap chain.
        ThrowIfFailed(mSwapChain->ResizeBuffers(
            SwapChainBufferCount, 
            mClientWidth, mClientHeight, 
            mBackBufferFormat, 
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        mCurrBackBuffer = mSwapChain->GetCurrentBackBufferIndex();
    
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < SwapChainBufferCount; i++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i].m_Resource)));
            md3dDevice->CreateRenderTargetView(mBackBuffer[i].m_Resource.Get(), nullptr, rtvHeapHandle);
            mBackBuffer[i].rtvHandle.cpuHandle = rtvHeapHandle.ptr;
            mBackBuffer[i].SetState(BufferResourceState::STATE_PRESENT);
            mBackBuffer[i].SetName(L"BackBuffer");
            rtvHeapHandle.Offset(1, mRtvDescriptorSize);

            // Init RenderTexture Wrapper
        }

        // Create the depth/stencil buffer and view.
        D3D12_RESOURCE_DESC depthStencilDesc;
        depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment = 0;
        depthStencilDesc.Width = mClientWidth;
        depthStencilDesc.Height = mClientHeight;
        depthStencilDesc.DepthOrArraySize = 1;
        depthStencilDesc.MipLevels = 1;

        // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
        // the depth buffer.  Therefore, because we need to create two views to the same resource:
        //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
        //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
        // we need to create the depth buffer resource with a typeless format.  
        depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

        depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
        depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
        depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format = mDepthStencilFormat;
        optClear.DepthStencil.Depth = 1.0f;
        optClear.DepthStencil.Stencil = 0;
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &optClear,
            IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()))); 
        mDepthStencilBuffer->SetName(L"BackDepthBuffer");
        // Create descriptor to mip level 0 of entire resource using the format of the resource.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = mDepthStencilFormat;
        dsvDesc.Texture2D.MipSlice = 0;
        md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

        // Transition the resource from its initial state to be used as a depth buffer.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        
        // Execute the resize commands.
        ThrowIfFailed(mCommandList->Close());
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // Wait until resize is complete.
        WaitForRenderFinish(mFrameFence);

        // Update the viewport transform to cover the client area.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width    = static_cast<float>(mClientWidth);
        mScreenViewport.Height   = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mScissorRect = { 0, 0, mClientWidth, mClientHeight };
        mBackBufferProxyRenderTexture.textureBuffer = &mBackBufferProxy;
        mBackBufferProxy.m_Desc.name = "LogicalBackBufferProxy";
    }

    void D3D12RenderAPI::InitDescritorHeap()
    {
        mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
        rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
            &rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));


        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
            &dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

    }
    
    void D3D12RenderAPI::InitCommandObject()
    {
        // init commandQueue;
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

        ThrowIfFailed(md3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));
        mDirectCmdListAlloc->SetName(L"DirectCommandList");
        ThrowIfFailed(md3dDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            mDirectCmdListAlloc.Get(), // Associated command allocator
            nullptr,                   // Initial PipelineStateObject
            IID_PPV_ARGS(mCommandList.GetAddressOf())));

        // Start off in a closed state.  This is because the first time we refer 
        // to the command list we will Reset it, and it needs to be closed before
        // calling Reset.
        mCommandList->Close();

        ThrowIfFailed(md3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(mImediatelyCmdListAlloc.GetAddressOf())));
        mImediatelyCmdListAlloc->SetName(L"ImediatelyCommandList");

        ThrowIfFailed(md3dDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            mImediatelyCmdListAlloc.Get(), // Associated command allocator
            nullptr,                   // Initial PipelineStateObject
            IID_PPV_ARGS(mImediatelyCommandList.GetAddressOf())));
        // Start off in a closed state.  This is because the first time we refer 
        // to the command list we will Reset it, and it needs to be closed before
        // calling Reset.
        mImediatelyCommandList->Close();


        // 创建IndirectDrawArgs的commandSignature，并不是RootSignature，只是用来描述
        // DrawIndirectArgs
        D3D12_INDIRECT_ARGUMENT_DESC argumentDescs = {};

        argumentDescs.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
        D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
        commandSignatureDesc.pArgumentDescs = &argumentDescs;
        commandSignatureDesc.NumArgumentDescs = 1;
        commandSignatureDesc.ByteStride = sizeof(DrawIndirectArgs);
        ThrowIfFailed(md3dDevice->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&mCommandSignature)));
    
    }
    
    void D3D12RenderAPI::InitSwapChain()
    {
        // Release the previous swapchain we will be recreating.
        mSwapChain.Reset();

        DXGI_SWAP_CHAIN_DESC sd;
        sd.BufferDesc.Width = mClientWidth;
        sd.BufferDesc.Height = mClientHeight;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = mBackBufferFormat;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
        sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = SwapChainBufferCount;
        sd.OutputWindow = static_cast<HWND>(WindowManager::GetInstance()->GetWindow());
        sd.Windowed = true;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // Note: Swap chain uses queue to perform flush.
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
        ThrowIfFailed(mdxgiFactory->CreateSwapChain(
            mCommandQueue.Get(),
            &sd, 
            swapChain.GetAddressOf()));

        ThrowIfFailed(swapChain.As(&mSwapChain));
    }

    void D3D12RenderAPI::WaitForRenderFinish(TD3D12Fence* mFence)
	{
		SignalFence(mFence);
		WaitForFence(mFence);
	}

    void D3D12RenderAPI::SignalFence(TD3D12Fence* mFence)
	{
		mFence->mCurrentFence++;
		ThrowIfFailed(mCommandQueue->Signal(mFence->mFence.Get(), mFence->mCurrentFence));
	}

	void D3D12RenderAPI::WaitForFence(TD3D12Fence* mFence)
	{
		if (mFence->mCurrentFence > 0 && mFence->mFence->GetCompletedValue() < mFence->mCurrentFence)
		{
			auto event = CreateEventEx(nullptr, NULL, NULL, EVENT_ALL_ACCESS);
			if (event)
			{
				ThrowIfFailed(mFence->mFence->SetEventOnCompletion(mFence->mCurrentFence, event));
				// �ȴ��¼�
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
			else
			{
				std::cout << "Create event failed !" << std::endl;
			}
		}
	}

    void D3D12RenderAPI::CompileShader(const string& path, Shader* shader)
    {
        D3D12ShaderUtils::CompileShaderAndGetReflection(path, shader);
        D3D12RootSignature::GetOrCreateARootSig(md3dDevice, shader);
    }

    void D3D12RenderAPI::CompileComputeShader(const string &path, ComputeShader *csShader)
    {
        D3D12ShaderUtils::CompileComputeShaderAndGetReflection(path, csShader);
        D3D12RootSignature::GetOrCreateAComputeShaderRootSig(md3dDevice, csShader);
    }


    IGPUTexture* D3D12RenderAPI::CreateTextureBuffer(unsigned char* data, const TextureDesc& textureDesc)
    {
        D3D12Texture* texture = new D3D12Texture(textureDesc);

        // 创建默认堆 贴图资源， CPU不可见
        CD3DX12_HEAP_PROPERTIES textureProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC d3D12TextureDesc(CD3DX12_RESOURCE_DESC::Tex2D(mDefaultImageFormat, 
            textureDesc.width, textureDesc.height, 1, 1));
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &textureProps,
            D3D12_HEAP_FLAG_NONE,
            &d3D12TextureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&texture->m_Resource)
        ));

        // 创建上传堆，从CPU拷贝资源， COPY到GPU
        // todo: 这个是不是可以放到帧上传堆里面？
        UINT64 uploadHeapSize;
        md3dDevice->GetCopyableFootprints(&d3D12TextureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadHeapSize);
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC uploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadHeapSize);
        ComPtr<ID3D12Resource> uploadHeap;
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadHeapDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadHeap)
        ));

        // �ϴ���������
        ImmediatelyExecute([&](ComPtr<ID3D12GraphicsCommandList> cmdList)
            {
                D3D12_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pData = data;
                subresourceData.RowPitch = static_cast<LONG_PTR>(textureDesc.width * 4);
                subresourceData.SlicePitch = subresourceData.RowPitch * textureDesc.height;

                UpdateSubresources(cmdList.Get(),
                    texture->m_Resource.Get(),
                    uploadHeap.Get(),
                    0, 0, 1, &subresourceData);

                // ת������״̬
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    texture->m_Resource.Get(),
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
                );
                
                cmdList->ResourceBarrier(1, &barrier);
                texture->SetState(BufferResourceState::STATE_SHADER_RESOURCE);
            });
        // 在函数结束前等待GPU完成
        // todo： 优化成 std::vector<ComPtr<ID3D12Resource>> m_PendingUploadResources;，
        // 持有这个资源到帧尾，然后Clear，这样能够让资源不立即销毁， 也不用CPU同步等待
        WaitForRenderFinish(mImediatelyFence);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = mDefaultImageFormat;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        
        texture->srvHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource.Get(), srvDesc);
        return texture;
    }

    IGPUTexture *D3D12RenderAPI::CreateRenderTexture(const TextureDesc& textureDesc)
    {
        D3D12Texture* texture = new D3D12Texture(textureDesc);
        // 1. 设置资源描述符
        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.Dimension = d3dUtil::GetFBOD3D12Dimesnsion(textureDesc.dimension);
        resourceDesc.Alignment = 0;
        resourceDesc.Width = textureDesc.width;
        resourceDesc.Height = textureDesc.height;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = d3dUtil::GetFBOD3D12Format(textureDesc.format);
        // todo: 加上mipmap 和 msaa
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

         // 设置资源标志
         if (textureDesc.format == TextureFormat::D24S8)
         {
             resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
         }
         else
         {
             resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
         }

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = resourceDesc.Format;
        if (textureDesc.format == TextureFormat::D24S8)
        {
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;
        }
        else
        {
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 1.0f;
        }

        
        // 根据格式选择正确的初始资源状态
        D3D12_RESOURCE_STATES initialState;
        if (textureDesc.format == TextureFormat::D24S8)
        {
            initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            texture->SetState(BufferResourceState::STATE_DEPTH_WRITE);
        }
        else
        {
            initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
            texture->SetState(BufferResourceState::STATE_RENDER_TARGET);
        }

        // 3. 创建资源
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            &clearValue,
            IID_PPV_ARGS(&texture->m_Resource)));
        std::wstring debugName = std::wstring(textureDesc.name.begin(), textureDesc.name.end());
        texture->m_Resource->SetName(debugName.c_str());
        // Create Descriptor:...
        if(textureDesc.format == TextureFormat::R8G8B8A8)
        {
            DescriptorHandle descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, D3D12_RENDER_TARGET_VIEW_DESC{});
            texture->rtvHandle = descHandle;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // SRV格式
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, srvDesc);
            texture->srvHandle = descHandle;
        }
        else if (textureDesc.format == TextureFormat::D24S8)
        {
            DescriptorHandle descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, D3D12_DEPTH_STENCIL_VIEW_DESC{});
            texture->dsvHandle = descHandle;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // SRV格式
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(texture->m_Resource, srvDesc);
            texture->srvHandle = descHandle;
        }

        return texture;
    }

    // 初始化一次， 同步信息
    void D3D12RenderAPI::CreateMaterialTextureSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos)
    {
        // uint32_t matID = mat->GetInstanceID();
        // if (m_DataMap.count(matID) <= 0) m_DataMap.try_emplace(matID, TD3D12MaterialData());
        // auto iter = m_DataMap.find(matID);
        // TD3D12MaterialData data = iter->second;
        // data.mTextureBufferArray.clear();
        // for each(auto bufferInfo in resourceInfos)
        // {
        //     // 创建buffer resource + handle
        //     // 初始化时没有Handler信息，后续就是对应Texture的InstanceID
        //     TD3D12TextureHander textureHandler;
        //     textureHandler.textureID = 0;
        //     data.mTextureBufferArray.push_back(textureHandler);
        // }
        // m_DataMap[matID] = data;
    }

    void D3D12RenderAPI::CreateMaterialUAVSlots(const Material* mat, const vector<ShaderBindingInfo >& resourceInfos)
    {

    }
    

    void D3D12RenderAPI::ImmediatelyExecute(std::function<void(ComPtr<ID3D12GraphicsCommandList> cmdList)>&& function)
    {
        WaitForRenderFinish(mImediatelyFence);
        // Reuse the memory associated with command recording.
        // We can only reset when the associated command lists have finished execution on the GPU.
        ThrowIfFailed(mImediatelyCmdListAlloc->Reset());

        // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
        // Reusing the command list reuses memory.
        ThrowIfFailed(mImediatelyCommandList->Reset(mImediatelyCmdListAlloc.Get(), nullptr));

        function(mImediatelyCommandList);

        // Done recording commands.
        ThrowIfFailed(mImediatelyCommandList->Close());
    
        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdsLists[] = { mImediatelyCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
        
        // swap the back and front buffers
        SignalFence(mImediatelyFence);
    }

    IGPUBuffer* D3D12RenderAPI::CreateBuffer(const BufferDesc &desc, void *data)
    {
        // Align size to 256 bytes (D3D12 Constant Buffer requirement)
        uint64_t alignedSize = (desc.size + 255) & ~255;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = alignedSize;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;

        switch(desc.memoryType)
        {
            case BufferMemoryType::Default:
                heapType = D3D12_HEAP_TYPE_DEFAULT;
                initialState = D3D12_RESOURCE_STATE_COMMON;
                if(desc.usage == BufferUsage::StructuredBuffer || desc.usage == BufferUsage::ByteAddressBuffer || desc.usage == BufferUsage::IndirectArgument)
                {
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                }
                break;
            case BufferMemoryType::Upload:
                heapType = D3D12_HEAP_TYPE_UPLOAD;
                initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
                break;
            case BufferMemoryType::ReadBack:
                heapType = D3D12_HEAP_TYPE_READBACK;
                initialState = D3D12_RESOURCE_STATE_COPY_DEST;
                break;
        }

        ComPtr<ID3D12Resource> bufferResource;
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(heapType),
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            nullptr,
            IID_PPV_ARGS(&bufferResource)));

        if(desc.debugName) bufferResource->SetName(desc.debugName);

        if(data)
        {
            if(desc.memoryType == BufferMemoryType::Upload)
            {
                void* mappedData;
                // 第0个子资源， nullptr表示read range为整个buffer
                bufferResource->Map(0, nullptr, &mappedData);
                memcpy(mappedData, data, desc.size);
                bufferResource->Unmap(0, nullptr);
            }
            else if(desc.memoryType == BufferMemoryType::Default)
            {
                // 创建临时的UploadBuffer
                ComPtr<ID3D12Resource> uploadBuffer;
                ThrowIfFailed(md3dDevice->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(desc.size),
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&uploadBuffer)
                ));

                void* mappedData;
                uploadBuffer->Map(0, nullptr, &mappedData);
                memcpy(mappedData, data, desc.size);
                uploadBuffer->Unmap(0, nullptr);

                // todo 加入pendingtoList， 保持索引，在下一帧首释放。
                ImmediatelyExecute([&](ComPtr<ID3D12GraphicsCommandList> cmdList)
                {
                    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferResource.Get(), initialState, D3D12_RESOURCE_STATE_COPY_DEST));
                    cmdList->CopyBufferRegion(bufferResource.Get(), 0, uploadBuffer.Get(), 0, desc.size);
                    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, initialState));
                });
                WaitForRenderFinish(mImediatelyFence);
            }
        }

        return new D3D12Buffer(bufferResource, desc, initialState);
    }

    void D3D12RenderAPI::UploadBuffer(IGPUBuffer *bufferResource, uint32_t offset, void *data, uint32_t size)
    {
        const BufferDesc& desc = bufferResource->GetDesc();
        if(data)
        {
            if(desc.memoryType == BufferMemoryType::Upload)
            {
                void* mappedData = bufferResource->Map();
                memcpy(static_cast<char*>(mappedData) + offset, data, size);
            }
            else if(desc.memoryType == BufferMemoryType::Default)
            {
                // 创建临时的UploadBuffer
                ComPtr<ID3D12Resource> uploadBuffer;
                ThrowIfFailed(md3dDevice->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(size),
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&uploadBuffer)
                ));

                void* mappedData;
                uploadBuffer->Map(0, nullptr, &mappedData);
                memcpy(mappedData, data, size);
                uploadBuffer->Unmap(0, nullptr);

                // todo 加入pendingtoList， 保持索引，在下一帧首释放。
                ID3D12Resource* nativeHandle = static_cast<ID3D12Resource*>(bufferResource->GetNativeHandle());
                ImmediatelyExecute([&](ComPtr<ID3D12GraphicsCommandList> cmdList)
                {
                    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(nativeHandle, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
                    cmdList->CopyBufferRegion(nativeHandle, offset, uploadBuffer.Get(), 0, size);
                    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(nativeHandle, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
                });
                WaitForRenderFinish(mImediatelyFence);
            }
        }
    }

    D3D12_RESOURCE_STATES D3D12RenderAPI::GetResourceState(BufferResourceState state)
    {
        switch (state)
        {
        case BufferResourceState::STATE_COMMON:
            return D3D12_RESOURCE_STATE_COMMON;
        case BufferResourceState::STATE_UNORDERED_ACCESS:
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case BufferResourceState::STATE_SHADER_RESOURCE:
            return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        case BufferResourceState::STATE_INDIRECT_ARGUMENT:
            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        case BufferResourceState::STATE_COPY_DEST:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case BufferResourceState::STATE_COPY_SOURCE:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case BufferResourceState::STATE_GENERIC_READ:
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        case BufferResourceState::STATE_DEPTH_WRITE:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case BufferResourceState::STATE_RENDER_TARGET:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case BufferResourceState::STATE_PRESENT:
            return D3D12_RESOURCE_STATE_PRESENT;
        default:
            ASSERT("Wrongt State");
            return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    // void D3D12RenderAPI::SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID)
    // {
    //     uint32_t matID = mat->GetInstanceID();
    //     if(m_DataMap.count(matID) <= 0)
    //     {
    //         cout << "SetShaderVector: matID not found" << endl;
    //         return;
    //     }
    //     auto iter = m_DataMap.find(matID);
    //     TD3D12MaterialData& data = iter->second;
    //     // 找到对应的插槽然后替换
    //     data.mTextureBufferArray[slotIndex].textureID = texInstanceID;
    // }

    void D3D12RenderAPI::SetUpMesh(ModelData* data, bool isStatic)
    {
        TD3D12VAO vao;
        Microsoft::WRL::ComPtr<ID3D12Resource> uploadBufferVertex;
        Microsoft::WRL::ComPtr<ID3D12Resource> uploadBufferIndex;
        ImmediatelyExecute([&](ComPtr<ID3D12GraphicsCommandList> cmdList) 
        {        
            vao.VertexBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), cmdList.Get(), data->vertex.data(), data->vertex.size() * sizeof(float) * 8, uploadBufferVertex);
            vao.IndexBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), cmdList.Get(), data->index.data(), data->index.size() * sizeof(int), uploadBufferIndex);
        });

        WaitForRenderFinish(mImediatelyFence);
        D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vao.VertexBuffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = sizeof(Vertex);
		vbv.SizeInBytes = data->vertex.size() * sizeof(float)* 8;

        D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = vao.IndexBuffer->GetGPUVirtualAddress();
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = data->index.size() * sizeof(int);

        vao.vertexBufferView = vbv;
        vao.indexBufferView = ibv;

        VAOMap.try_emplace(data->GetInstanceID(), std::move(vao));
    }

    int D3D12RenderAPI::GetNextVAOIndex()
    {
        return 0;
    }
    


    // DescriptorHandle EngineCore::D3D12RenderAPI::GetTextureSrvHanle(uint32_t textureID)
    // {
    //     ASSERT_MSG(m_TextureBufferMap.count(textureID) > 0, "Texture not find in m_TextureBuffer!!");
    //     DescriptorHandle handle;
    //     handle.cpuHandle = m_TextureBufferMap[textureID].srvHandle.ptr;
    //     return handle;
    // }

    void D3D12RenderAPI::CreateGlobalConstantBuffer(uint32_t bufferID, uint32_t size)
    {
        mGlobalConstantBufferMap.try_emplace(bufferID, CreateConstantBuffer(size));
    }

    RenderTexture* D3D12RenderAPI::GetCurrentBackBuffer()
    {
        // return &mBackBufferRT[mCurrBackBuffer];
        // 延迟绑定，逻辑线程只获取一个Proxy对象
        return &mBackBufferProxyRenderTexture;
    }
    

    void D3D12RenderAPI::SetGlobalDataImpl(uint32_t bufferID, uint32_t offset, uint32_t size, const void *value)
    {
        ASSERT(mGlobalConstantBufferMap.count(bufferID) > 0);
        TD3D12ConstantBuffer buffer = mGlobalConstantBufferMap[bufferID];
        void* baseAddress = buffer.mCpuAddress;
        void* targetAddress = reinterpret_cast<char*>(baseAddress) + offset;
        memcpy(targetAddress, value, size);
    }


    TD3D12ConstantBuffer D3D12RenderAPI::CreateConstantBuffer(uint32_t size)
    {
        int alignedSize = (size + 255) & ~255;
        TD3D12ConstantBuffer constantBuffer;
        constantBuffer.mSize = alignedSize;
        // only slot 0 for PerFrame, PerPass Data
        constantBuffer.registerSlot = 0;
        ImmediatelyExecute([&](ComPtr<ID3D12GraphicsCommandList> cmdList)
        {
             // 1. 创建constant buffer resource
            ThrowIfFailed(md3dDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),  // 使用UPLOAD heap以便CPU写入
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(alignedSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,  // Upload heap的默认状态
                nullptr,
                IID_PPV_ARGS(constantBuffer.mBufferResource.GetAddressOf())));
            
            constantBuffer.mGPUAddress = constantBuffer.mBufferResource->GetGPUVirtualAddress();
            // 2. 映射内存以便CPU写入
            ThrowIfFailed(constantBuffer.mBufferResource->Map(0, nullptr, &constantBuffer.mCpuAddress));
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = constantBuffer.mBufferResource->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = alignedSize;
            constantBuffer.handleCBV = D3D12DescManager::GetInstance()->CreateDescriptor(cbvDesc);
        });
        return constantBuffer;
    }

    void D3D12RenderAPI::RenderAPIBeginFrame()
    {
        // 给定默认的PSO，
        WaitForRenderFinish(mFrameFence);
        ThrowIfFailed(mDirectCmdListAlloc->Reset());
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), psoObj.Get()));

        // 重置状态缓存，因为Reset后CommandList的状态被清空了
        currentRootSignature = nullptr;
        currentPSO = psoObj;

        ID3D12DescriptorHeap* heaps[] = {
            D3D12DescManager::GetInstance()->GetFrameCbvSrvUavHeap().Get(),
            // 如果用了采样器表，这里再加上 sampler heap
        };
        mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

    }

    void D3D12RenderAPI::RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT)
    {
        // Clear the back buffer and depth buffer.
        D3D12Texture* colorBuffer = static_cast<D3D12Texture*>(payloadConfigureRT.colorAttachment);
        
        // [Proxy Pattern] Check if it is the proxy object, if so, replace with the real back buffer.
        if (colorBuffer->m_Desc.name == mBackBufferProxy.m_Desc.name)
        {
            colorBuffer = static_cast<D3D12Texture*>(&mBackBuffer[mCurrBackBuffer]);
        }

        D3D12Texture* depthBuffer = static_cast<D3D12Texture*>(payloadConfigureRT.depthAttachment);
        
        // 根据状态判断当前RT是否需要切换渲染状态， 一般不用的，可能需要从资源切换到RenderTarget
        if (colorBuffer && colorBuffer->GetState() != BufferResourceState::STATE_RENDER_TARGET)
        {
            mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                colorBuffer->m_Resource.Get(),
                GetResourceState(colorBuffer->GetState()), D3D12_RESOURCE_STATE_RENDER_TARGET));
            colorBuffer->SetState(BufferResourceState::STATE_RENDER_TARGET);
        }

        if (depthBuffer && depthBuffer->GetState() != BufferResourceState::STATE_DEPTH_WRITE)
        {
            mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                depthBuffer->m_Resource.Get(),
                GetResourceState(depthBuffer->GetState()), D3D12_RESOURCE_STATE_DEPTH_WRITE));
            depthBuffer->SetState(BufferResourceState::STATE_DEPTH_WRITE);
        }
        
        // Set ClearFlag.
        // Set Rendertarget.
        ClearValue& state = payloadConfigureRT.clearValue;
        float color[4] = { state.colorValue.x, state.colorValue.y, state.colorValue.z, 1.0f };
        
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
        if (colorBuffer) 
        {
            rtvHandle.ptr = colorBuffer->rtvHandle.cpuHandle;
        }
        if (colorBuffer && (state.flags == ClearFlag::All || state.flags == ClearFlag::Color))
        {
            mCommandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
        }
                    
        // todo: Reverse Z?
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};
        if (depthBuffer) 
        {
            dsvHandle.ptr = depthBuffer->dsvHandle.cpuHandle;
        }
        if (depthBuffer && (state.flags == ClearFlag::All || state.flags == ClearFlag::Depth))
        {
            mCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, state.depthValue, 0, 0, nullptr);
        }
        
        
        mCommandList->OMSetRenderTargets(1, 
            colorBuffer ? &rtvHandle : nullptr, 
            true, 
            depthBuffer ? &dsvHandle : nullptr);

    }

    void D3D12RenderAPI::RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand)
    {
        auto& vao = VAOMap[payloadDrawCommand.vaoID];
        mCommandList->DrawIndexedInstanced(vao.indexBufferView.SizeInBytes / sizeof(int), payloadDrawCommand.count, 0, 0, 0);
    }

    void D3D12RenderAPI::RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial)
    {
        //TD3D12MaterialData& matData = m_DataMap[payloadSetMaterial.matId];
        Shader* shader = payloadSetMaterial.shader;
        Material* mat = payloadSetMaterial.mat;
        // todo： 重写buffer 绑定逻辑
        uint64_t gpuAddr = GPUSceneManager::GetInstance()->allObjectDataBuffer->GetBaseGPUAddress();
        mCommandList->SetGraphicsRootShaderResourceView((UINT)RootSigSlot::AllObjectData, gpuAddr);

        gpuAddr = GPUSceneManager::GetInstance()->allMaterialDataBuffer->GetBaseGPUAddress();
        mCommandList->SetGraphicsRootShaderResourceView((UINT)RootSigSlot::AllMaterialData, gpuAddr);
        
        gpuAddr = GPUSceneManager::GetInstance()->visibilityBuffer->GetBaseGPUAddress();
        mCommandList->SetGraphicsRootShaderResourceView((UINT)RootSigSlot::PerDrawInstanceObjectsList, gpuAddr);
        
        // === 5. 绑定纹理 (Root Param 5+) ===
        vector<ShaderBindingInfo > textureInfo = shader->mShaderReflectionInfo.mTextureInfo;
        if (textureInfo.size() > 0)
        {
            DescriptorHandle tableHandle = 
                D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(textureInfo.size());
            
            for (int i = 0; i < textureInfo.size(); i++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE dest = {
                    tableHandle.cpuHandle + i * mCbvSrvUavDescriptorSize
                };
                
                ASSERT(mat->textureData.count(textureInfo[i].resourceName) > 0);
                D3D12Texture* texture = static_cast<D3D12Texture*>(mat->textureData[textureInfo[i].resourceName]);
                DescriptorHandle texSRVHandle = texture->srvHandle;
                ASSERT(texSRVHandle.cpuHandle != UINT64_MAX);
                D3D12_CPU_DESCRIPTOR_HANDLE srcHandle;
                srcHandle.ptr = texSRVHandle.cpuHandle;
                md3dDevice->CopyDescriptorsSimple(
                    1, dest, 
                    srcHandle,
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );
            }
            
            // ⭐ 纹理使用 Root Param 5（固定，因为 Slot 4 留给了 UAV）
            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
            gpuHandle.ptr = tableHandle.gpuHandle;
            mCommandList->SetGraphicsRootDescriptorTable((UINT)RootSigSlot::Textures, gpuHandle);
        }
    }

    
    void D3D12RenderAPI::RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState)
    {
        // rootsig 只要和 pso匹配的，pso切换 rootsig不一定要切换
        // 这个payload里面记录了对应的rootsigKey，只需要判断和current的是否一致。
        // 不一致就刷新， 没必要管pso是什么，状态已经提前算完了，pso创建的时候绑定的也是对应的rootSig.
        PSODesc& psoDesc = payloadSetRenderState.psoDesc;
       
        ComPtr<ID3D12RootSignature> rootSig = D3D12RootSignature::GetOrCreateARootSig(psoDesc.matRenderState.rootSignatureKey);
        if(currentRootSignature != rootSig)
        {
            mCommandList->SetGraphicsRootSignature(rootSig.Get());
            // todo： 这个地方应该还是需要一个状态， 比如需要更新的全局通知？
            uint32_t perFrameBufferID = (uint32_t)UniformBufferType::PerFrameData;
            if (mGlobalConstantBufferMap.count(perFrameBufferID) > 0)
            {
                TD3D12ConstantBuffer& buffer = mGlobalConstantBufferMap[perFrameBufferID];
                uint64_t gpuAddr = buffer.mGPUAddress;
                mCommandList->SetGraphicsRootConstantBufferView(
                    (UINT)RootSigSlot::PerFrameData,
                    gpuAddr
                );
            }

            uint32_t perPassBufferID = (uint32_t)UniformBufferType::PerPassData_Foraward;
            if (mGlobalConstantBufferMap.count(perPassBufferID) > 0)
            {
                TD3D12ConstantBuffer& buffer = mGlobalConstantBufferMap[perPassBufferID];
                uint64_t gpuAddr = buffer.mGPUAddress;
                mCommandList->SetGraphicsRootConstantBufferView(
                    (UINT)RootSigSlot::PerPassData,
                    gpuAddr
                );
            }
            currentRootSignature = rootSig;
        }

        ComPtr<ID3D12PipelineState> pso = D3D12PSO::GetOrCreatePSO(md3dDevice, psoDesc);  // 添加这行
        if(currentPSO != pso)
        {
            mCommandList->SetPipelineState(pso.Get()); 
            currentPSO = pso;
        }
        
    }

    void D3D12RenderAPI::RenderAPISetSissorRect(Payload_SetSissorRect payloadSetSissorrect)
    {
        mScissorRect = { (int)payloadSetSissorrect.x, (int)payloadSetSissorrect.y, (int)payloadSetSissorrect.w, (int)payloadSetSissorrect.h };
        mCommandList->RSSetScissorRects(1, &mScissorRect);
    }

    void D3D12RenderAPI::RenderAPISetVBIB(Payload_SetVBIB payloadSetVBIB)
    {
        auto& vao = VAOMap[payloadSetVBIB.vaoId];
        mCommandList->IASetVertexBuffers(0, 1, &vao.vertexBufferView);
        mCommandList->IASetIndexBuffer(&vao.indexBufferView);
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void D3D12RenderAPI::RenderAPISetViewPort(Payload_SetViewPort payloadSetViewport)
    {
        // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
        mScreenViewport.TopLeftX = payloadSetViewport.x;
        mScreenViewport.TopLeftY = payloadSetViewport.y;
        mScreenViewport.Width = payloadSetViewport.w;
        mScreenViewport.Height = payloadSetViewport.h;
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;
        mCommandList->RSSetViewports(1, &mScreenViewport);
    }


    void D3D12RenderAPI::RenderAPIWindowResize(Payload_WindowResize payloadWindowResize)
    {
        mClientWidth = payloadWindowResize.width;
        mClientHeight = payloadWindowResize.height;
                          // Flush before changing any resources.
        WaitForRenderFinish(mFrameFence);
        ThrowIfFailed(mDirectCmdListAlloc->Reset());
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // 重置状态缓存
        currentRootSignature = nullptr;
        currentPSO = nullptr;

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mBackBuffer[i].m_Resource.Reset();
        mDepthStencilBuffer.Reset();
        
        // Resize the swap chain.
        ThrowIfFailed(mSwapChain->ResizeBuffers(
            SwapChainBufferCount, 
            mClientWidth, mClientHeight, 
            mBackBufferFormat, 
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        mCurrBackBuffer = mSwapChain->GetCurrentBackBufferIndex();
    
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < SwapChainBufferCount; i++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i].m_Resource)));
            md3dDevice->CreateRenderTargetView(mBackBuffer[i].m_Resource.Get(), nullptr, rtvHeapHandle);
            // Resize后是全新的资源，初始状态为 COMMON/PRESENT，逻辑上我们将其标记为 PRESENT
            mBackBuffer[i].SetState(BufferResourceState::STATE_PRESENT);
            mBackBuffer[i].SetName(L"BackBuffer");
            rtvHeapHandle.Offset(1, mRtvDescriptorSize);

        }

        // Create the depth/stencil buffer and view.
        D3D12_RESOURCE_DESC depthStencilDesc;
        depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment = 0;
        depthStencilDesc.Width = mClientWidth;
        depthStencilDesc.Height = mClientHeight;
        depthStencilDesc.DepthOrArraySize = 1;
        depthStencilDesc.MipLevels = 1;

        // Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
        // the depth buffer.  Therefore, because we need to create two views to the same resource:
        //   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
        //   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
        // we need to create the depth buffer resource with a typeless format.  
        depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

        depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
        depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
        depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format = mDepthStencilFormat;
        optClear.DepthStencil.Depth = 1.0f;
        optClear.DepthStencil.Stencil = 0;
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &optClear,
            IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

        // Create descriptor to mip level 0 of entire resource using the format of the resource.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = mDepthStencilFormat;
        dsvDesc.Texture2D.MipSlice = 0;
        md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

        // Transition the resource from its initial state to be used as a depth buffer.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        
        // Execute the resize commands.
        ThrowIfFailed(mCommandList->Close());
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // Wait until resize is complete.
        WaitForRenderFinish(mFrameFence);

        // Update the viewport transform to cover the client area.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width    = static_cast<float>(mClientWidth);
        mScreenViewport.Height   = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mScissorRect = { 0, 0, mClientWidth, mClientHeight };

    }



    void D3D12RenderAPI::RenderAPISubmit()
    {
        // 需要切换RT -> present Frame.
        // todo 切换backbuffer
        // 最后一定是backbuffer的格式切换。 中间过程中涉及到 RT-> ShaderResource，就直接根据资源判断
        {
            if (mBackBuffer[mCurrBackBuffer].GetState() != BufferResourceState::STATE_PRESENT)
            {
                mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                    mBackBuffer[mCurrBackBuffer].m_Resource.Get(),
                    GetResourceState(mBackBuffer[mCurrBackBuffer].GetState()),
                    D3D12_RESOURCE_STATE_PRESENT));
                mBackBuffer[mCurrBackBuffer].SetState(BufferResourceState::STATE_PRESENT);
            }
        }

        // Done recording commands.
        ThrowIfFailed(mCommandList->Close());

        PROFILER_EVENT_BEGIN("GPU Process::WaitForGPUFinished");
        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
        // swap the back and front buffers
        SignalFence(mFrameFence);

        PROFILER_EVENT_END("GPU Process::WaitForGPUFinished");
    }

    // todo 这个分类似乎不合理
    void D3D12RenderAPI::RenderAPIPresentFrame()
    {
        ThrowIfFailed(mSwapChain->Present(0, 0));
        mCurrBackBuffer = mSwapChain->GetCurrentBackBufferIndex();
        D3D12DescManager::GetInstance()->ResetFrameAllocator();
    }

    // todo: Obsolute
    void D3D12RenderAPI::RenderAPISetPerDrawData(Payload_SetPerDrawData setPerDrawData)
    {
    }

    void D3D12RenderAPI::RenderAPIDrawInstanceCmd(Payload_DrawInstancedCommand setDrawInstanceCmd)
    {
        auto& vao = VAOMap[setDrawInstanceCmd.vaoID];
        mCommandList->SetGraphicsRoot32BitConstants((UINT)RootSigSlot::DrawIndiceConstant, 1, &setDrawInstanceCmd.perDrawOffset, 0);
        mCommandList->DrawIndexedInstanced(vao.indexBufferView.SizeInBytes / sizeof(int), setDrawInstanceCmd.count, 0, 0, setDrawInstanceCmd.perDrawOffset);
        //ASSERT_MSG(false, "Not Implemented!");
    }

    void D3D12RenderAPI::RenderAPISetPerPassData(Payload_SetPerPassData setPerPassData)
    {
        currentPerPassBufferID = setPerPassData.perPassBufferID;
    }

    void D3D12RenderAPI::RenderAPISetPerFrameData(Payload_SetPerFrameData setPerFrameData)
    {
        currentPerFrameBufferID = setPerFrameData.perFrameBufferID;
    }

    void D3D12RenderAPI::RenderAPICopyRegion(Payload_CopyBufferRegion copyBufferRegion)
    {
        ASSERT(copyBufferRegion.count > 0);
        ID3D12Resource* destHandle = static_cast<ID3D12Resource*>(copyBufferRegion.destDefaultBuffer->GetNativeHandle());
        ID3D12Resource* srcHandle = static_cast<ID3D12Resource*>(copyBufferRegion.srcUploadBuffer->GetNativeHandle());
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            destHandle,
            D3D12_RESOURCE_STATE_COMMON,    
            D3D12_RESOURCE_STATE_COPY_DEST  
        );
        mCommandList->ResourceBarrier(1, &barrier);
        uint32_t count = copyBufferRegion.count;
        CopyOp* op = copyBufferRegion.copyList;
        for (int i = 0; i < count; i++)
        {
            mCommandList->CopyBufferRegion(
                destHandle,                      // pDstBuffer
                (UINT64)op->dstOffset,     // DstOffset (注意转为UINT64防止溢出)
                srcHandle,                       // pSrcBuffer
                (UINT64)op->srcOffset,     // SrcOffset (紧凑排列的源数据)
                op->size                   // NumBytes
            );
            op++;
        }

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            destHandle,
            D3D12_RESOURCE_STATE_COPY_DEST,  
            D3D12_RESOURCE_STATE_COMMON   
        );
        mCommandList->ResourceBarrier(1, &barrier);

    }

    void D3D12RenderAPI::RenderAPIDispatchComputeShader(Payload_DispatchComputeShader dispatchComputeShader)
    {
        ComputeShader* csShader = dispatchComputeShader.csShader;
        // 1. set PSO
        ComPtr<ID3D12RootSignature> rootSig = D3D12RootSignature::GetOrCreateAComputeShaderRootSig(md3dDevice, csShader);
        if(currentRootSignature != rootSig)
        {
            mCommandList->SetComputeRootSignature(rootSig.Get());
            currentRootSignature = rootSig;
        }

        PSODesc psoDesc = {};
        psoDesc.matRenderState.rootSignatureKey = csShader->mShaderReflectionInfo.mRootSigKey;
        psoDesc.matRenderState.shaderInstanceID = csShader->GetInstanceID();
        ComPtr<ID3D12PipelineState> pso = D3D12PSO::GetOrCreateComputeShaderPSO(md3dDevice, psoDesc);  // 添加这行
        if(currentPSO != pso)
        {
            mCommandList->SetPipelineState(pso.Get()); 
            currentPSO = pso;
        }
        // 2. bind rootSign
        std::vector<ShaderBindingInfo> cbvs = csShader->mShaderReflectionInfo.mConstantBufferInfo;
        std::vector<ShaderBindingInfo> srvs = csShader->mShaderReflectionInfo.mTextureInfo;
        std::vector<ShaderBindingInfo> uavs = csShader->mShaderReflectionInfo.mUavInfo;
        std::vector<ShaderBindingInfo> samplersInfo = csShader->mShaderReflectionInfo.mSamplerInfo;
        int rootParamIndex = 0;
        for (const auto& cbv : cbvs)
        {
            D3D12Buffer* buffer = static_cast<D3D12Buffer*>(csShader->GetBufferResource(cbv.resourceName));
            mCommandList->SetComputeRootConstantBufferView(rootParamIndex++, buffer->GetGPUVirtualAddress());
        }

        // Note: Root SRV only supports buffers (e.g., StructuredBuffer/ByteAddressBuffer), not textures.
        // This matches current engine's ComputeShader::SetBuffer(name, gpuVA) usage.
        for (const auto& srv : srvs)
        {
            D3D12Buffer* buffer = static_cast<D3D12Buffer*>(csShader->GetBufferResource(srv.resourceName));
            mCommandList->SetComputeRootShaderResourceView(rootParamIndex++, buffer->GetGPUVirtualAddress());
        }

        for (const auto& uav : uavs)
        {
            D3D12Buffer* buffer = static_cast<D3D12Buffer*>(csShader->GetBufferResource(uav.resourceName));
            mCommandList->SetComputeRootUnorderedAccessView(rootParamIndex++, buffer->GetGPUVirtualAddress());  
        }
        
        // 3. dispatch
        mCommandList->Dispatch(
            dispatchComputeShader.groupX,
            dispatchComputeShader.groupY,
            dispatchComputeShader.groupZ
        );
    }

    void D3D12RenderAPI::RenderAPISetBufferResourceState(Payload_SetBufferResourceState bufferResourceState)
    {
        IGPUResource* resource = bufferResourceState.resource;
        if(resource->GetState() == bufferResourceState.state) return;
        D3D12_RESOURCE_STATES fromState = GetResourceState(resource->GetState());
        D3D12_RESOURCE_STATES toState = GetResourceState(bufferResourceState.state);
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(static_cast<ID3D12Resource*>(resource->GetNativeHandle()),
        fromState, toState));
        resource->SetState(bufferResourceState.state);
    }

    void D3D12RenderAPI::RenderAPIExecuteIndirect(Payload_DrawIndirect drawIndirect)
    {
        D3D12Buffer* argsBuffer = static_cast<D3D12Buffer*>(drawIndirect.indirectArgsBuffer);
        uint32_t stride = sizeof(DrawIndirectArgs);
        mCommandList->SetGraphicsRoot32BitConstants((UINT)RootSigSlot::DrawIndiceConstant, 1, &drawIndirect.startIndexInInstanceDataBuffer, 0);
        mCommandList->ExecuteIndirect(
            mCommandSignature.Get(),
            drawIndirect.count,   // 绘制几个
            static_cast<ID3D12Resource*>(argsBuffer->GetNativeHandle()),
            drawIndirect.startIndex * stride,  // 从哪个offset 开始，按照byte算
            nullptr,
            0
        );
    }

} // namespace EngineCore

