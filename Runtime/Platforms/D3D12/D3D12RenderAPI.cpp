#include "PreCompiledHeader.h"
#include "D3D12RenderAPI.h"
#include "Managers/WindowManager.h"
#include "Graphics/ResourceStruct.h"

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
        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));
    }
    
    void D3D12RenderAPI::InitRenderTarget()
    {
        auto [width, height] = WindowManager::GetInstance().GetWindowSize();
        mClientWidth = width;
        mClientHeight = height;
        // Flush before changing any resources.
	    WaitForFence();
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mSwapChainBuffer[i].Reset();
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
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
            md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
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
        WaitForRenderFinish();

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
        sd.OutputWindow = static_cast<HWND>(WindowManager::GetInstance().GetWindow());
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
        WaitForRenderFinish();
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
        SignalFence();

        // Wait until frame commands are complete.  This waiting is inefficient and is
        // done for simplicity.  Later we will show how to organize our rendering code
        // so we do not have to wait per frame.
    }

    void D3D12RenderAPI::EndFrame()
    {
        //std::cout << "EndFrame" << std::endl;
        WaitForFence();
        ThrowIfFailed(mSwapChain->Present(0, 0));
        mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
        //WaitForRenderFinish();

    }

    void D3D12RenderAPI::OnResize(int width, int height)
    {
        mClientWidth = width;
        mClientHeight = height;
              // Flush before changing any resources.
        WaitForRenderFinish();
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
            mSwapChainBuffer[i].Reset();
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
            ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
            md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
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
        WaitForRenderFinish();

        // Update the viewport transform to cover the client area.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width    = static_cast<float>(mClientWidth);
        mScreenViewport.Height   = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mScissorRect = { 0, 0, mClientWidth, mClientHeight };
    }

    void D3D12RenderAPI::WaitForRenderFinish()
	{
		SignalFence();
		WaitForFence();
	}

    void D3D12RenderAPI::SignalFence()
	{
		mCurrentFence++;
		ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));
	}

	void D3D12RenderAPI::WaitForFence()
	{
		if (mCurrentFence > 0 && mFence->GetCompletedValue() < mCurrentFence)
		{
			auto event = CreateEventEx(nullptr, NULL, NULL, EVENT_ALL_ACCESS);
			if (event)
			{
				ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, event));
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

    Shader* D3D12RenderAPI::CompileShader(const string& path)
    {

        Shader* shader = new Shader();
        if (!CompileShaderStage(path, "VSMain", "vs_5_1", shader, ShaderStageType::VERTEX_STAGE)) 
        {
            cout << "Shader Stage Vertex Compile fail" << endl;
        }

        if (!CompileShaderStage(path, "PSMain", "ps_5_1", shader, ShaderStageType::FRAGMENT_STAGE))
        {
            cout << "Shader Stage Pixel Shader Compile fail" << endl;
        }
        return shader;
    }


    bool D3D12RenderAPI::CompileShaderStage(const string& path, string entryPoint, string target, Shader* shader, ShaderStageType type)
    {
        ComPtr<ID3DBlob> errorBlob;
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        std::wstring filename(path.begin(), path.end());
        ComPtr<ID3DBlob> m_shaderBlob;

        HRESULT hr = D3DCompileFromFile(
            filename.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.c_str(),
            target.c_str(),
            compileFlags,
            0,
            &m_shaderBlob,
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
        D3DReflect(m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(), IID_PPV_ARGS(&m_reflection));

        if (!m_reflection) return false;

        
        ShaderStageInfo* shaderStageInfo = new ShaderStageInfo();

        D3D12_SHADER_DESC desc;
        m_reflection->GetDesc(&desc);


        std::unordered_map<string, ShaderVariableInfo>  shaderVariableInfoMap;
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
                tempVariable.type = Resources::GetShaderVaribleType((uint32_t)varDesc.Size);
                shaderVariableInfoMap.try_emplace(varDesc.Name, tempVariable);
            }
        }
        shader->mShaderVariableInfoMap = shaderVariableInfoMap;

        std::cout << "获得shader stage info" << std::endl;
        for (UINT i = 0; i < desc.BoundResources; ++i) {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            m_reflection->GetResourceBindingDesc(i, &bindDesc);
            int bufferSize = 0;
            ID3D12ShaderReflectionConstantBuffer* cbReflection;
            switch (bindDesc.Type) {
            case D3D_SIT_CBUFFER:
                cbReflection = m_reflection->GetConstantBufferByName(bindDesc.Name);
                if (cbReflection) {
                    D3D12_SHADER_BUFFER_DESC bufferDesc;
                    HRESULT hr = cbReflection->GetDesc(&bufferDesc);
                    if (SUCCEEDED(hr)) {
                        bufferSize = bufferDesc.Size;
                    }
                }
                shaderStageInfo->mBufferInfo.emplace_back(bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, bindDesc.BindPoint, bufferSize);
                break;
            case D3D_SIT_TEXTURE:
                shaderStageInfo->mTextureInfo.emplace_back(bindDesc.Name, ShaderResourceType::TEXTURE, bindDesc.BindPoint, 0);
                break;
            case D3D_SIT_SAMPLER:
                shaderStageInfo->mSamplerInfo.emplace_back(bindDesc.Name, ShaderResourceType::SAMPLER, bindDesc.BindPoint, 0);
                break;
            default:
                shaderStageInfo->mBufferInfo.emplace_back(bindDesc.Name, ShaderResourceType::CONSTANT_BUFFER, bindDesc.BindPoint, 0);
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

            shader->vsInfo = shaderStageInfo;
        }
        else if(type == ShaderStageType::FRAGMENT_STAGE) 
        {
            shader->psInfo = shaderStageInfo;
        }
        return true;
    }
} // namespace EngineCore
