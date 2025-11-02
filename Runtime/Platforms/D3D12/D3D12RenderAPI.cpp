#include "PreCompiledHeader.h"
#include "D3D12RenderAPI.h"
#include "Managers/WindowManager.h"
#include "Resources/MetaFile.h"
#include "Core/PublicStruct.h"
#include "D3D12DescManager.h"
#include "d3dx12.h"  // 确保包含D3D12辅助类
#include "Renderer/FrameBufferManager.h"
#include "D3D12PSOManager.h"
#include "Graphics/FrameBufferObject.h"

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
        m_DataMap = unordered_map<uint32_t, TD3D12MaterialData>();

        D3D12DescManager::Create(md3dDevice);
        D3D12PSOManager::Create(md3dDevice);
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
            mBackBuffer[i].resource.Reset();
        mDepthStencilBuffer.Reset();
        
        // Resize the swap chain.
        ThrowIfFailed(mSwapChain->ResizeBuffers(
            SwapChainBufferCount, 
            mClientWidth, mClientHeight, 
            mBackBufferFormat, 
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        mCurrBackBuffer = 0;
    
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < SwapChainBufferCount; i++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i].resource)));
            md3dDevice->CreateRenderTargetView(mBackBuffer[i].resource.Get(), nullptr, rtvHeapHandle);
            mBackBuffer[i].rtvHandle = rtvHeapHandle;
            mBackBuffer[i].state = D3D12_RESOURCE_STATE_PRESENT;
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
        ThrowIfFailed(mdxgiFactory->CreateSwapChain(
            mCommandQueue.Get(),
            &sd, 
            mSwapChain.GetAddressOf()));
    }

    void D3D12RenderAPI::BeginFrame()
    {
        //std::cout << "beginFrame" << std::endl;
    }

    void D3D12RenderAPI::Render()
    {
        WaitForRenderFinish(mFrameFence);
        // Reuse the memory associated with command recording.
        // We can only reset when the associated command lists have finished execution on the GPU.
        ThrowIfFailed(mDirectCmdListAlloc->Reset());

        // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
        // Reusing the command list reuses memory.
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // Indicate a state transition on the resource usage.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
        mCommandList->RSSetViewports(1, &mScreenViewport);
        mCommandList->RSSetScissorRects(1, &mScissorRect);

        float color[4] = {0.434153706f, 0.552011609f, 0.730461001f, 1.f};
        // Clear the back buffer and depth buffer.
        mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
        mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        
        // Specify the buffers we are going to render to.
        mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
        
        // Indicate a state transition on the resource usage.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Done recording commands.
        ThrowIfFailed(mCommandList->Close());
    
        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
        
        // swap the back and front buffers
        SignalFence(mFrameFence);

        // Wait until frame commands are complete.  This waiting is inefficient and is
        // done for simplicity.  Later we will show how to organize our rendering code
        // so we do not have to wait per frame.
    }

    void D3D12RenderAPI::EndFrame()
    {
        // //std::cout << "EndFrame" << std::endl;
        // WaitForFence(mFrameFence);
        // ThrowIfFailed(mSwapChain->Present(0, 0));
        // mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
        // //WaitForRenderFinish();
        // D3D12DescManager::GetInstance()->ResetFrameAllocator();

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

    Shader* D3D12RenderAPI::CompileShader(const string& path, Shader* shader)
    {

        shader->name = path;
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        if (!CompileShaderStage(path, "VSMain", "vs_5_1", shader, ShaderStageType::VERTEX_STAGE, vsBlob))
        {
            cout << "Shader Stage Vertex Compile fail" << endl;
        }
        vsBlobMap.try_emplace(shader->GetInstanceID(), vsBlob);
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        if (!CompileShaderStage(path, "PSMain", "ps_5_1", shader, ShaderStageType::FRAGMENT_STAGE, psBlob))
        {
            cout << "Shader Stage Pixel Shader Compile fail" << endl;
        }
        psBlobMap.try_emplace(shader->GetInstanceID(), psBlob);

        // 创建PSO
        CreateRootSignatureByShaderReflection(shader);
        return shader;
    }

    void D3D12RenderAPI::CreateRootSignatureByShaderReflection(Shader* shader)
    {
        
        int cbCount = shader->mShaderBindingInfo.mBufferInfo.size();
        int texCount = shader->mShaderBindingInfo.mTextureInfo.size();
        int samplerCount = shader->mShaderBindingInfo.mSamplerInfo.size();

        int tableCount = 0;
        if(cbCount > 0) tableCount++;
        if(texCount > 0) tableCount++;
        //if(samplerCount > 0) tableCount++;


        vector<CD3DX12_ROOT_PARAMETER> slotRootParameter(tableCount);
        vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;

        // 防止扩容导致的内存地址失效！！！！
        descriptorRanges.reserve(tableCount);
        // 先尝试绑定constantbuffer
        int paramIndex = 0;
        if(cbCount > 0)
        {
            CD3DX12_DESCRIPTOR_RANGE cbvRange;
            cbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbCount, 0);
            descriptorRanges.push_back(cbvRange);
            slotRootParameter[paramIndex++].InitAsDescriptorTable(1, &descriptorRanges.back());
        }
        
        if(texCount > 0)
        {
            CD3DX12_DESCRIPTOR_RANGE srvRange;
            srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, texCount, 0);
            descriptorRanges.push_back(srvRange);
            slotRootParameter[paramIndex++].InitAsDescriptorTable(1,&descriptorRanges.back());
        }

		// todo: 使用动态采样器
        //if(samplerCount > 0)
        //{
        //    CD3DX12_DESCRIPTOR_RANGE samplerRange;
        //    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, samplerCount, 0);
        //    descriptorRanges.push_back(samplerRange);
        //    slotRootParameter[paramIndex++].InitAsDescriptorTable(1,&descriptorRanges.back());
        //}

        // 在第 526 行之前添加
        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0,  // shaderRegister: register(s0)
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,   // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP    // addressW
        );

        ComPtr<ID3D12RootSignature> tempRootSignature;
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            static_cast<UINT>(slotRootParameter.size()), slotRootParameter.data(),
            1, &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;

        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig, &errorBlob));

        ThrowIfFailed(md3dDevice->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&tempRootSignature)));

        if(shaderRootSignatureMap.count(shader->GetInstanceID()) == 0)
        {
            shaderRootSignatureMap.try_emplace(shader->GetInstanceID(), tempRootSignature);
        }

        // // todo: InputLayout完善：
        // std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
        // mInputLayout =
        // {
        //     { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //     { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //     { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        // };
        // // pipeline state object:
        // D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
        // pso.pRootSignature = tempRootSignature.Get();
        // pso.VS = {reinterpret_cast<BYTE*>(vsBlob->GetBufferPointer()), vsBlob->GetBufferSize()};
        // pso.PS = {reinterpret_cast<BYTE*>(psBlob->GetBufferPointer()), psBlob->GetBufferSize()};
        // pso.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
        // // todo: 调整depth stencil逻辑
        // pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // pso.BlendState      = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        // D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        // depthStencilDesc.DepthEnable = TRUE;
        // depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        // depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;  // 这里设置比较函数
        // depthStencilDesc.StencilEnable = FALSE;
        // pso.DepthStencilState = depthStencilDesc;
        // //pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

        // pso.SampleMask     = UINT_MAX;
        // pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // pso.NumRenderTargets = 1;
        // pso.RTVFormats[0]    = DXGI_FORMAT_R8G8B8A8_UNORM;
        // if (shader->name == "D:/GitHubST/TinyEngine/Assets/Shader/BlitShader.hlsl") 
        // {
        //     pso.DSVFormat = DXGI_FORMAT_UNKNOWN;
        //     pso.DepthStencilState.DepthEnable = FALSE;
        //     pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        // }
        // else 
        // {
        //     pso.DSVFormat        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        // }
        // pso.SampleDesc.Count = 1;

        
        // if (FAILED(md3dDevice->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&tempPsoObj))))
        //     throw std::runtime_error("CreateGraphicsPipelineState failed");

        // TD3D12ShaderPSO psoData(std::move(tempPsoObj), std::move(tempRootSignature), std::move(mInputLayout));
        // m_PipeLineStateObjectMap.try_emplace(shader->GetInstanceID(), std::move(psoData));
    }

    bool D3D12RenderAPI::CompileShaderStage(const string& path, string entryPoint, string target, Shader* shader, ShaderStageType type, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        ComPtr<ID3DBlob> errorBlob;
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        std::wstring filename(path.begin(), path.end());

        HRESULT hr = D3DCompileFromFile(
            filename.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.c_str(),
            target.c_str(),
            compileFlags,
            0,
            &blob,
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorBlob)
            {
                std::cout << "Compilation Error:\n" << (char*)errorBlob->GetBufferPointer() << std::endl;
            }
            return false;
        }


        ComPtr<ID3D12ShaderReflection>  m_reflection;
        D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_reflection));

        if (!m_reflection) return false;

        
        //if (shader->mShaderBindingInfo == nullptr) 
        //{
        //    shader->mShaderBindingInfo = new ShaderReflectionInfo();
        //}
        ShaderReflectionInfo* ShaderReflectionInfo = &shader->mShaderBindingInfo;

        D3D12_SHADER_DESC desc;
        m_reflection->GetDesc(&desc);


        auto& shaderVariableInfoMap = shader->mShaderBindingInfo.mShaderStageVariableInfoMap;
        ShaderVariableInfo tempVariable;
        std::cout << "\n=== Constant Buffers ===\n";

        for (UINT i = 0; i < desc.ConstantBuffers; ++i) {
            auto cbReflection = m_reflection->GetConstantBufferByIndex(i);

            D3D12_SHADER_BUFFER_DESC bufferDesc;
            cbReflection->GetDesc(&bufferDesc);

            for (UINT j = 0; j < bufferDesc.Variables; ++j) {
                auto varReflection = cbReflection->GetVariableByIndex(j);

                D3D12_SHADER_VARIABLE_DESC varDesc;
                varReflection->GetDesc(&varDesc);

                tempVariable.variableName = varDesc.Name;
                tempVariable.bufferIndex = i;
                tempVariable.offset = varDesc.StartOffset;
                tempVariable.size = varDesc.Size;
                tempVariable.type = MetaLoader::GetShaderVaribleType((uint32_t)varDesc.Size);
                shaderVariableInfoMap.try_emplace(varDesc.Name, tempVariable);
            }
        }


        std::cout << "获得shader stage info" << std::endl;
        for (UINT i = 0; i < desc.BoundResources; ++i) {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            m_reflection->GetResourceBindingDesc(i, &bindDesc);
            int bufferSize = 0;
            HRESULT hr;
            ID3D12ShaderReflectionConstantBuffer* cbReflection;
            switch (bindDesc.Type) {
            case D3D_SIT_CBUFFER:
                cbReflection = m_reflection->GetConstantBufferByName(bindDesc.Name);
                for (auto& x : ShaderReflectionInfo->mBufferInfo) 
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                
                D3D12_SHADER_BUFFER_DESC bufferDesc;
                hr = cbReflection->GetDesc(&bufferDesc);
                if (SUCCEEDED(hr)) {
                    bufferSize = bufferDesc.Size;
                }
                ShaderReflectionInfo->mBufferInfo.emplace_back(bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, bindDesc.BindPoint, bufferSize);
                break;

            case D3D_SIT_TEXTURE:
                for (auto& x : ShaderReflectionInfo->mTextureInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                ShaderReflectionInfo->mTextureInfo.emplace_back(bindDesc.Name, ShaderResourceType::TEXTURE, bindDesc.BindPoint, 0);
                break;
            case D3D_SIT_SAMPLER:
                for (auto& x : ShaderReflectionInfo->mSamplerInfo)
                {
                    if (x.resourceName == bindDesc.Name) break;
                }
                ShaderReflectionInfo->mSamplerInfo.emplace_back(bindDesc.Name, ShaderResourceType::SAMPLER, bindDesc.BindPoint, 0);
                break;
            default:
                std::cout << " Not find any exites shader resource type " << std::endl;
                break;
            }
        }
        
        if (type == ShaderStageType::VERTEX_STAGE) 
        {
            std::cout << "\n=== Input Layout ===\n";
            for (UINT i = 0; i < desc.InputParameters; ++i) {
                D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                m_reflection->GetInputParameterDesc(i, &paramDesc);
                if (strcmp(paramDesc.SemanticName, "POSITION") == 0) 
                {
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::POSITION);
                }
                else if (strcmp(paramDesc.SemanticName, "NORMAL") == 0) 
                {
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::NORMAL);
                }
                else if (strcmp(paramDesc.SemanticName, "TEXCOORD") == 0) 
                {
                    // 目前先比较 TexCoord一个， 没有涉及index
                    shader->mShaderInputLayout.emplace_back(VertexAttribute::UV0);
                }
            }
        }
        return true;
    }

    void D3D12RenderAPI::CreateBuffersResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos)
    {
        uint32_t matID = mat->GetInstanceID();
        if(m_DataMap.count(matID) <= 0) m_DataMap.try_emplace(matID, TD3D12MaterialData());
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData data = iter->second;
        data.mConstantBufferArray.clear();
        for each(auto bufferInfo in resourceInfos)
        {   
            int size = bufferInfo.size;
            int alignedSize = (size + 255) & ~255;
            //  创建buffer resource + handle
            TD3D12ConstantBuffer constantBuffer;
            constantBuffer.mSize = alignedSize;
            constantBuffer.registerSlot = bufferInfo.registerSlot;
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
            data.mConstantBufferArray.push_back(constantBuffer);
        }
        m_DataMap[matID] = data;
    }

    void D3D12RenderAPI::CreateFBO(FrameBufferObject* fbodesc)
    {
        if(m_TextureBufferMap.count(fbodesc->GetInstanceID()) > 0)
        {
            ASSERT_MSG(false, "Already Create FBO Here");
        }

        // 1. 设置资源描述符
        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.Dimension = d3dUtil::GetFBOD3D12Dimesnsion(fbodesc->mDimension);
        resourceDesc.Alignment = 0;
        resourceDesc.Width = fbodesc->mWidth;
        resourceDesc.Height = fbodesc->mHeight;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = d3dUtil::GetFBOD3D12Format(fbodesc->mFormat);
        // todo: 加上mipmap 和 msaa
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

         // 设置资源标志
         if (fbodesc->mFormat == TextureFormat::D24S8)
         {
             resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
         }
         else
         {
             resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
         }

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = resourceDesc.Format;
        if (fbodesc->mFormat == TextureFormat::D24S8)
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

        TD3D12FrameBuffer d3DFrameBufferObject;
        
        // 根据格式选择正确的初始资源状态
        D3D12_RESOURCE_STATES initialState;
        if (fbodesc->mFormat == TextureFormat::D24S8)
        {
            initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }
        else
        {
            initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        }
        d3DFrameBufferObject.state = initialState; 

        // 3. 创建资源
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            &clearValue,
            IID_PPV_ARGS(&d3DFrameBufferObject.resource)));
        std::wstring debugName = std::wstring(fbodesc->mTextureName.begin(), fbodesc->mTextureName.end());
        d3DFrameBufferObject.resource->SetName(debugName.c_str());
        // Create Descriptor:...
        if(fbodesc->mFormat == TextureFormat::R8G8B8A8)
        {
            TD3D12DescriptorHandle descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(d3DFrameBufferObject.resource, D3D12_RENDER_TARGET_VIEW_DESC{});
            d3DFrameBufferObject.rtvHandle = descHandle.cpuHandle;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // SRV格式
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(d3DFrameBufferObject.resource, srvDesc);
            d3DFrameBufferObject.srvHandle = descHandle.cpuHandle;
        }
        else if (fbodesc->mFormat == TextureFormat::D24S8)
        {
            TD3D12DescriptorHandle descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(d3DFrameBufferObject.resource, D3D12_DEPTH_STENCIL_VIEW_DESC{});
            d3DFrameBufferObject.dsvHandle = descHandle.cpuHandle;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // SRV格式
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            descHandle = D3D12DescManager::GetInstance()->CreateDescriptor(d3DFrameBufferObject.resource, srvDesc);
            d3DFrameBufferObject.srvHandle = descHandle.cpuHandle;
        }

        m_TextureBufferMap.insert({fbodesc->GetInstanceID(), d3DFrameBufferObject});
    }

    void D3D12RenderAPI::CreateTextureBuffer(unsigned char* data, Texture* tbdesc)
    {
        TD3D12TextureBuffer buffer;


        // 创建默认堆 贴图资源， CPU不可见
        CD3DX12_HEAP_PROPERTIES textureProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC textureDesc(CD3DX12_RESOURCE_DESC::Tex2D(mDefaultImageFormat, 
            tbdesc->GetWidth(), tbdesc->GetHeight(), 1, 1));
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &textureProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&buffer.resource)
        ));

        // 创建上传堆，从CPU拷贝资源， COPY到GPU
        UINT64 uploadHeapSize;
        md3dDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadHeapSize);
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
                subresourceData.RowPitch = static_cast<LONG_PTR>(tbdesc->GetWidth() * 4);
                subresourceData.SlicePitch = subresourceData.RowPitch * tbdesc->GetHeight();

                UpdateSubresources(cmdList.Get(),
                    buffer.resource.Get(),
                    uploadHeap.Get(),
                    0, 0, 1, &subresourceData);

                // ת������״̬
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    buffer.resource.Get(),
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
                );

                cmdList->ResourceBarrier(1, &barrier);
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
        TD3D12DescriptorHandle handle = D3D12DescManager::GetInstance()->CreateDescriptor(buffer.resource.Get(), srvDesc);

        buffer.srvHandle = handle.cpuHandle;

        m_TextureBufferMap.insert({tbdesc->GetInstanceID(), buffer});
        return;
    }

    //void D3D12RenderAPI::GetOrCreatePSO(const Material &mat, const RenderPassInfo &passinfo)
    //{
    //    
    //}

    void D3D12RenderAPI::CreateSamplerResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos)
    {
    }

    // 初始化一次， 同步信息
    void D3D12RenderAPI::CreateTextureResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos)
    {
        uint32_t matID = mat->GetInstanceID();
        if (m_DataMap.count(matID) <= 0) m_DataMap.try_emplace(matID, TD3D12MaterialData());
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData data = iter->second;
        data.mTextureBufferArray.clear();
        for each(auto bufferInfo in resourceInfos)
        {
            // 创建buffer resource + handle
            // 初始化时没有Handler信息，后续就是对应Texture的InstanceID
            TD3D12TextureHander textureHandler;
            textureHandler.textureID = 0;
            data.mTextureBufferArray.push_back(textureHandler);
        }
        m_DataMap[matID] = data;
    }

    void D3D12RenderAPI::CreateUAVResource(const Material* mat, const vector<ShaderResourceInfo>& resourceInfos)
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

    void D3D12RenderAPI::SetShaderFloat(const Material *mat, const ShaderVariableInfo &variableInfo, float value)
    {
        uint32_t matID = mat->GetInstanceID();
        if(m_DataMap.count(matID) <= 0)
        {
            cout << "SetShaderVector: matID not found" << endl;
            return;
        }
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData data = iter->second;
        void* baseAddress = data.mConstantBufferArray[variableInfo.bufferIndex].mCpuAddress;
        void* targetAddress = reinterpret_cast<char*>(baseAddress) + variableInfo.offset;
        memcpy(targetAddress, &value, sizeof(float));
    }

    void D3D12RenderAPI::SetShaderVector(const Material *mat, const ShaderVariableInfo &variableInfo, const Vector3 &value)
    {
        uint32_t matID = mat->GetInstanceID();
        if(m_DataMap.count(matID) <= 0)
        {
            cout << "SetShaderVector: matID not found" << endl;
            return;
        }
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData data = iter->second;
        void* baseAddress = data.mConstantBufferArray[variableInfo.bufferIndex].mCpuAddress;
        void* targetAddress = reinterpret_cast<char*>(baseAddress) + variableInfo.offset;
        memcpy(targetAddress, &value, sizeof(Vector3));
        
    }
    
    void D3D12RenderAPI::SetShaderMatrix4x4(const Material* mat, const ShaderVariableInfo& variableInfo, const Matrix4x4& value)
    {
        uint32_t matID = mat->GetInstanceID();
        if(m_DataMap.count(matID) <= 0)
        {
            cout << "SetShaderVector: matID not found" << endl;
            return;
        }
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData data = iter->second;
        void* baseAddress = data.mConstantBufferArray[variableInfo.bufferIndex].mCpuAddress;
        void* targetAddress = reinterpret_cast<char*>(baseAddress) + variableInfo.offset;
        memcpy(targetAddress, &value, sizeof(Matrix4x4));
        
    }

    void D3D12RenderAPI::SetShaderTexture(const Material* mat, const string& slotName, int slotIndex, uint32_t texInstanceID)
    {
        uint32_t matID = mat->GetInstanceID();
        if(m_DataMap.count(matID) <= 0)
        {
            cout << "SetShaderVector: matID not found" << endl;
            return;
        }
        auto iter = m_DataMap.find(matID);
        TD3D12MaterialData& data = iter->second;
        // 找到对应的插槽然后替换
        data.mTextureBufferArray[slotIndex].textureID = texInstanceID;
    }

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
    

    // submit the drawCommand to render thread..
    void D3D12RenderAPI::Submit(const vector<RenderPassInfo*>& renderPassInfos)
    {
        //WaitForRenderFinish(mFrameFence);
        //// Reuse the memory associated with command recording.
        //// We can only reset when the associated command lists have finished execution on the GPU.
        //ThrowIfFailed(mDirectCmdListAlloc->Reset());

        //// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
        //// Reusing the command list reuses memory.
        //ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), psoObj.Get()));


        //// Specify the buffers we are going to render to.


        //for (auto& renderpassInfo : renderPassInfos) 
        //{
        //    // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
        //    mScreenViewport.TopLeftX = renderpassInfo->viewportStartPos.x;
        //    mScreenViewport.TopLeftY = renderpassInfo->viewportStartPos.y;
        //    mScreenViewport.Width = renderpassInfo->viewportEndPos.x;
        //    mScreenViewport.Height = renderpassInfo->viewportEndPos.y;
        //    mScreenViewport.MinDepth = 0.0f;
        //    mScreenViewport.MaxDepth = 1.0f;
        //    mCommandList->RSSetViewports(1, &mScreenViewport);
        //    mCommandList->RSSetScissorRects(1, &mScissorRect);


        //    // Clear the back buffer and depth buffer.
        //    TD3D12FrameBuffer* colorBuffer = nullptr;
        //    colorBuffer = GetFrameBuffer(renderpassInfo->colorAttachment->name);
        //    TD3D12FrameBuffer* depthBuffer = nullptr;
        //    depthBuffer = GetFrameBuffer(renderpassInfo->depthAttachment->name);

        //    // 根据状态判断当前RT是否需要切换渲染状态， 一般不用的，可能需要从资源切换到RenderTarget
        //    if (colorBuffer && colorBuffer->state != D3D12_RESOURCE_STATE_RENDER_TARGET)
        //    {
        //        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        //            colorBuffer->resource.Get(),
        //            colorBuffer->state, D3D12_RESOURCE_STATE_RENDER_TARGET));
        //        colorBuffer->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        //    }

        //    if (depthBuffer && depthBuffer->state != D3D12_RESOURCE_STATE_DEPTH_WRITE)
        //    {
        //        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        //            depthBuffer->resource.Get(),
        //            depthBuffer->state, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        //        depthBuffer->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        //    }

        //    // Set ClearFlag.
        //    // Set Rendertarget.
        //    float color[4] = { renderpassInfo->clearColorValue.x, renderpassInfo->clearColorValue.y, renderpassInfo->clearColorValue.z, 1.f };
        //    if (colorBuffer && (renderpassInfo->clearFlag == ClearFlag::ALL || renderpassInfo->clearFlag == ClearFlag::COLOR))
        //    {
        //        mCommandList->ClearRenderTargetView(colorBuffer->rtvHandle, color, 0, nullptr);
        //    }
        //    
        //    // todo: Reverse Z?
        //    if (depthBuffer && (renderpassInfo->clearFlag == ClearFlag::ALL || renderpassInfo->clearFlag == ClearFlag::DEPTH))
        //    {
        //        mCommandList->ClearDepthStencilView(depthBuffer->dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, renderpassInfo->clearDepthValue, 0, 0, nullptr);
        //    }


        //    mCommandList->OMSetRenderTargets(1, 
        //        colorBuffer ? &colorBuffer->rtvHandle : nullptr, 
        //        true, 
        //        depthBuffer ? &depthBuffer->dsvHandle : nullptr);

        //    for (auto& drawRecord : renderpassInfo->drawRecordList) 
        //    {
        //        auto& vao = mVAOList[drawRecord.model->VAO];
        //        mCommandList->IASetVertexBuffers(0, 1, &vao.vertexBufferView);
        //        mCommandList->IASetIndexBuffer(&vao.indexBufferView);
        //        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //        Shader* shader = drawRecord.mat->shader;
        //        // 3. 设置root signature
        //        ComPtr<ID3D12RootSignature> rootSig = m_ShaderPSOMap[shader->name].rootSignature;


        //        ID3D12DescriptorHeap* heaps[] = {
        //            D3D12DescManager::GetInstance()->GetFrameCbvSrvUavHeap().Get(),
        //            // 如果用了采样器表，这里再加上 sampler heap
        //        };
        //        mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);
        //        mCommandList->SetGraphicsRootSignature(rootSig.Get());
        //        
        //        int rootParamIndex = 0;
        //        int cbvCount = shader->mShaderBindingInfo->mBufferInfo.size();

        //        TD3D12MaterialData& matData = m_DataMap[drawRecord.mat->GetID()];
        //        // 4. 处理CBV描述符表
        //        if (cbvCount > 0 && matData.mConstantBufferArray.size() > 0)
        //        {
        //            // 分配CBV表空间
        //            TD3D12DescriptorHandle handle = D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(cbvCount);
        //            D3D12_CPU_DESCRIPTOR_HANDLE cbvTableStart = handle.cpuHandle;
        //            
        //            // 拷贝CBV描述符
        //            for (int i = 0; i < matData.mConstantBufferArray.size(); i++)
        //            {
        //                // 计算目标位置
        //                D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {
        //                    cbvTableStart.ptr + i * mCbvSrvUavDescriptorSize
        //                };
        //                md3dDevice->CopyDescriptorsSimple(1, destHandle, matData.mConstantBufferArray[i].handleCBV.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        //            }
        //            
        //            // 绑定CBV描述符表
        //            D3D12_GPU_DESCRIPTOR_HANDLE cbvTableGPU = handle.gpuHandle;
        //            mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, cbvTableGPU);
        //        }
        //        
        //         // 5. 处理SRV(Texture)描述符表
        //        int srvCount = shader->mShaderBindingInfo->mTextureInfo.size();
        //        if (srvCount > 0 && matData.mTextureBufferArray.size() > 0)
        //        {
        //            // 分配CBV表空间
        //            TD3D12DescriptorHandle handle = D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(srvCount);
        //            D3D12_CPU_DESCRIPTOR_HANDLE srvTableStart = handle.cpuHandle;

        //            for (int i = 0; i < matData.mTextureBufferArray.size(); i++)
        //            {
        //                // 计算目标位置
        //                D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {
        //                    srvTableStart.ptr + i * mCbvSrvUavDescriptorSize
        //                };

        //                // 找到资源对应的SRV，然后copy
        //                TD3D12DescriptorHandle texSRVHandle = GetTextureSrvHanle(matData.mTextureBufferArray[i].textureID);
        //                md3dDevice->CopyDescriptorsSimple(1, destHandle, texSRVHandle.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        //            }

        //            // 绑定CBV描述符表
        //            D3D12_GPU_DESCRIPTOR_HANDLE srvTableGPU = handle.gpuHandle;
        //            mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, srvTableGPU);
        //           
        //        }
        //        //     // 绑定SRV描述符表
        //        //     D3D12_GPU_DESCRIPTOR_HANDLE srvTableGPU = frameHeap.GetCbvSrvUavGPUHandle(srvTableStart);
        //        //     mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, srvTableGPU);
        //            
        //        //     std::cout << "绑定SRV表，参数索引: " << rootParamIndex-1 << ", 描述符数量: " << srvCount << std::endl;
        //        // }
        //        
        //        // // 6. 处理Sampler描述符表
        //        // if (samplerCount > 0 && matData.mSamplerArray.size() > 0)  // 需要添加mSamplerArray到TD3D12MaterialData
        //        // {
        //        //     // 分配Sampler表空间
        //        //     D3D12_CPU_DESCRIPTOR_HANDLE samplerTableStart = frameHeap.AllocateSamplerTable(samplerCount);
        //            
        //        //     // 拷贝Sampler描述符
        //        //     for (int i = 0; i < matData.mSamplerArray.size(); i++)
        //        //     {
        //        //         // 计算目标位置
        //        //         D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {
        //        //             samplerTableStart.ptr + i * mSamplerDescriptorSize
        //        //         };
        //                
        //        //         // 拷贝现有的Sampler描述符
        //        //         D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = matData.mSamplerArray[i].samplerHandle;
        //                
        //        //         md3dDevice->CopyDescriptorsSimple(
        //        //             1,                                   // 描述符数量
        //        //             destHandle,                          // 目标
        //        //             srcHandle,                           // 源
        //        //             D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER   // Sampler堆类型
        //        //         );
        //        //     }
        //            
        //        //     // 绑定Sampler描述符表
        //        //     D3D12_GPU_DESCRIPTOR_HANDLE samplerTableGPU = frameHeap.GetSamplerGPUHandle(samplerTableStart);
        //        //     mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, samplerTableGPU);
        //            
        //        //     std::cout << "绑定Sampler表，参数索引: " << rootParamIndex-1 << ", 描述符数量: " << samplerCount << std::endl;
        //        // }
        //        ComPtr<ID3D12PipelineState> pso = m_ShaderPSOMap[shader->name].pso;  // 添加这行
        //        mCommandList->SetPipelineState(pso.Get()); 
        //        mCommandList->DrawIndexedInstanced(
        //            vao.indexBufferView.SizeInBytes / sizeof(int),
        //            1, 0, 0, 0);
        //    }

        //    if (colorBuffer)
        //    {
        //        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        //            colorBuffer->resource.Get(),
        //            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
        //        colorBuffer->state = D3D12_RESOURCE_STATE_PRESENT;
        //    }

        //    //if (depthBuffer)
        //    //{
        //    //    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        //    //        depthBuffer->resource.Get(),
        //    //        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
        //    //    depthBuffer->state = D3D12_RESOURCE_STATE_PRESENT;

        //    //}
        //}

        //// Done recording commands.
        //ThrowIfFailed(mCommandList->Close());

        //// Add the command list to the queue for execution.
        //ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        //mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        //// swap the back and front buffers
        //SignalFence(mFrameFence);
    }

    TD3D12DescriptorHandle EngineCore::D3D12RenderAPI::GetTextureSrvHanle(uint32_t textureID)
    {
        ASSERT_MSG(m_TextureBufferMap.count(textureID) > 0, "Texture not find in m_TextureBuffer!!");
        TD3D12DescriptorHandle handle;
        handle.cpuHandle = m_TextureBufferMap[textureID].srvHandle;
        return handle;
    }

    ComPtr<ID3D12PipelineState> D3D12RenderAPI::GetOrCreatePSO(PSODesc& psodesc)
    {
        uint32_t psoHash = psodesc.GetHash();
        if(shaderPSOMap.count(psoHash) > 0)
        {
            return shaderPSOMap[psoHash];
        }
        TD3D12PSO pso;
        pso.desc = psodesc;
        pso.inputLayout =         
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        ASSERT(psBlobMap.count(psodesc.matRenderState.shaderInstanceID) > 0);
        ASSERT(vsBlobMap.count(psodesc.matRenderState.shaderInstanceID) > 0);
        pso.psBlob = psBlobMap[psodesc.matRenderState.shaderInstanceID];
        pso.vsBlob = vsBlobMap[psodesc.matRenderState.shaderInstanceID];
        pso.rootSignature = shaderRootSignatureMap[psodesc.matRenderState.shaderInstanceID];
        ComPtr<ID3D12PipelineState> temp = D3D12PSOManager::GetInstance()->CreatePSO(pso);
        shaderPSOMap.try_emplace(psoHash, temp);
        return temp;
    }

    TD3D12FrameBuffer* D3D12RenderAPI::GetFrameBuffer(uint32_t bufferID, bool isBackBuffer)
    {
        if (bufferID == 0) return nullptr;
        if (isBackBuffer)
        {
            return &mBackBuffer[mCurrBackBuffer];
        }
        else 
        {
            ASSERT_MSG(m_TextureBufferMap.count(bufferID) > 0, "This FBO doesnt exits!!!");
            return &m_TextureBufferMap[bufferID];
        }

        return nullptr;
    }


    void D3D12RenderAPI::RenderAPIBeginFrame()
    {
        // 给定默认的PSO，
        WaitForRenderFinish(mFrameFence);
        ThrowIfFailed(mDirectCmdListAlloc->Reset());
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), psoObj.Get()));
        ID3D12DescriptorHeap* heaps[] = {
            D3D12DescManager::GetInstance()->GetFrameCbvSrvUavHeap().Get(),
            // 如果用了采样器表，这里再加上 sampler heap
        };
        mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

    }

    void D3D12RenderAPI::RenderAPIConfigureRT(Payload_ConfigureRT payloadConfigureRT)
    {
        // Clear the back buffer and depth buffer.
        TD3D12FrameBuffer* colorBuffer = nullptr;
        colorBuffer = GetFrameBuffer(payloadConfigureRT.colorAttachment, payloadConfigureRT.isBackBuffer);
        TD3D12FrameBuffer* depthBuffer = nullptr;
        depthBuffer = GetFrameBuffer(payloadConfigureRT.depthAttachment);
        
        // 根据状态判断当前RT是否需要切换渲染状态， 一般不用的，可能需要从资源切换到RenderTarget
        if (colorBuffer && colorBuffer->state != D3D12_RESOURCE_STATE_RENDER_TARGET)
        {
            mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                colorBuffer->resource.Get(),
                colorBuffer->state, D3D12_RESOURCE_STATE_RENDER_TARGET));
            colorBuffer->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        }

        if (depthBuffer && depthBuffer->state != D3D12_RESOURCE_STATE_DEPTH_WRITE)
        {
            mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                depthBuffer->resource.Get(),
                depthBuffer->state, D3D12_RESOURCE_STATE_DEPTH_WRITE));
            depthBuffer->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }
        
        // Set ClearFlag.
        // Set Rendertarget.
        ClearValue& state = payloadConfigureRT.clearValue;
        float color[4] = { state.colorValue.x, state.colorValue.y, state.colorValue.z, 1.0f };
        if (colorBuffer && (state.flags == ClearFlag::All || state.flags == ClearFlag::Color))
        {
            mCommandList->ClearRenderTargetView(colorBuffer->rtvHandle, color, 0, nullptr);
        }
                    
        // todo: Reverse Z?
        if (depthBuffer && (state.flags == ClearFlag::All || state.flags == ClearFlag::Depth))
        {
            mCommandList->ClearDepthStencilView(depthBuffer->dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, state.depthValue, 0, 0, nullptr);
        }
        
        
        mCommandList->OMSetRenderTargets(1, 
            colorBuffer ? &colorBuffer->rtvHandle : nullptr, 
            true, 
            depthBuffer ? &depthBuffer->dsvHandle : nullptr);

    }

    void D3D12RenderAPI::RenderAPIDrawIndexed(Payload_DrawCommand payloadDrawCommand)
    {
        auto& vao = VAOMap[payloadDrawCommand.vaoID];
        mCommandList->DrawIndexedInstanced(
            vao.indexBufferView.SizeInBytes / sizeof(int),
            payloadDrawCommand.count, 0, 0, 0);
    }

    void D3D12RenderAPI::RenderAPISetMaterial(Payload_SetMaterial payloadSetMaterial)
    {
        int rootParamIndex = 0;

        TD3D12MaterialData& matData = m_DataMap[payloadSetMaterial.matId];
        int cbvCount = matData.mConstantBufferArray.size();
        // 4. 处理CBV描述符表
        if (cbvCount > 0)
        {
            // 分配CBV表空间
            TD3D12DescriptorHandle handle = D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(cbvCount);
            D3D12_CPU_DESCRIPTOR_HANDLE cbvTableStart = handle.cpuHandle;
            
            // 拷贝CBV描述符
            for (int i = 0; i < matData.mConstantBufferArray.size(); i++)
            {
                // 计算目标位置
                D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {
                    cbvTableStart.ptr + i * mCbvSrvUavDescriptorSize
                };
                md3dDevice->CopyDescriptorsSimple(1, destHandle, matData.mConstantBufferArray[i].handleCBV.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
            
            // 绑定CBV描述符表
            D3D12_GPU_DESCRIPTOR_HANDLE cbvTableGPU = handle.gpuHandle;
            mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, cbvTableGPU);
        }
        
         // 5. 处理SRV(Texture)描述符表
        int srvCount = matData.mTextureBufferArray.size();
        if (srvCount > 0)
        {
            // 分配CBV表空间
            TD3D12DescriptorHandle handle = D3D12DescManager::GetInstance()->GetFrameCbvSrvUavAllocator(srvCount);
            D3D12_CPU_DESCRIPTOR_HANDLE srvTableStart = handle.cpuHandle;

            for (int i = 0; i < matData.mTextureBufferArray.size(); i++)
            {
                // 计算目标位置
                D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {
                    srvTableStart.ptr + i * mCbvSrvUavDescriptorSize
                };

                // 找到资源对应的SRV，然后copy
                TD3D12DescriptorHandle texSRVHandle = GetTextureSrvHanle(matData.mTextureBufferArray[i].textureID);
                md3dDevice->CopyDescriptorsSimple(1, destHandle, texSRVHandle.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            // 绑定CBV描述符表
            D3D12_GPU_DESCRIPTOR_HANDLE srvTableGPU = handle.gpuHandle;
            mCommandList->SetGraphicsRootDescriptorTable(rootParamIndex++, srvTableGPU);
           
        }

    }

    void D3D12RenderAPI::RenderAPISetRenderState(Payload_SetRenderState payloadSetRenderState)
    {
        // 这个地方正确的操作应该是怎么样的？
        // 这个地方其实就是传入一个ShaderID + passInfo，来得到一个渲染状态。
        // 所以那么这个地方，需要的参数就是 渲染状态，这个状态有当前pass的，
        // 有设置的， 比如blendmode这种， DepthCompare，
        // 也有shader的，比如inputlayout，rootsignature，
        // psoManager负责根据这个找到对应的， 如果没有就创建一个新的。
        // 这个地方顺便想一下合理的快速找到对应的操作。
        PSODesc& psoDesc = payloadSetRenderState.psoDesc;
       
        // set pso and Root Signature:
        // 3. 设置root signature
        ComPtr<ID3D12RootSignature> rootSig = shaderRootSignatureMap[psoDesc.matRenderState.shaderInstanceID];
        mCommandList->SetGraphicsRootSignature(rootSig.Get());
        ComPtr<ID3D12PipelineState> pso = GetOrCreatePSO(psoDesc);  // 添加这行
        mCommandList->SetPipelineState(pso.Get()); 
        
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
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mBackBuffer[i].resource.Reset();
        mDepthStencilBuffer.Reset();
        
        // Resize the swap chain.
        ThrowIfFailed(mSwapChain->ResizeBuffers(
            SwapChainBufferCount, 
            mClientWidth, mClientHeight, 
            mBackBufferFormat, 
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        mCurrBackBuffer = 0;
    
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT i = 0; i < SwapChainBufferCount; i++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffer[i].resource)));
            md3dDevice->CreateRenderTargetView(mBackBuffer[i].resource.Get(), nullptr, rtvHeapHandle);
            mBackBuffer[i].state = D3D12_RESOURCE_STATE_PRESENT;
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
            mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                mBackBuffer[mCurrBackBuffer].resource.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
            mBackBuffer[mCurrBackBuffer].state = D3D12_RESOURCE_STATE_PRESENT;
        }

        // Done recording commands.
        ThrowIfFailed(mCommandList->Close());

        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // swap the back and front buffers
        SignalFence(mFrameFence);

        WaitForFence(mFrameFence);
     
        
    }

    // todo 这个分类似乎不合理
    void D3D12RenderAPI::RenderAPIPresentFrame()
    {
        ThrowIfFailed(mSwapChain->Present(0, 0));
        mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
        //WaitForRenderFinish();
        D3D12DescManager::GetInstance()->ResetFrameAllocator();
    }

} // namespace EngineCore
