#include "PreCompiledHeader.h"
#include "D3D12EditorGUIManager.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include "Platforms/D3D12/D3D12RenderAPI.h"
#include "Renderer/RenderAPI.h"
#include "Managers/WindowManager.h"
#include "Platforms/Windows/WindowManagerWindows.h"

namespace EngineEditor
{
    D3D12EditorGUIManager::D3D12EditorGUIManager()
    {
        std::cout << "Create Editor GUI" << std::endl;
        IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
        InitForDirectX12();

    }
    
    D3D12EditorGUIManager::~D3D12EditorGUIManager()
    {
        auto renderAPI = static_cast<EngineCore::D3D12RenderAPI*>(&EngineCore::RenderAPI::GetInstance());


		// todo::
        //renderAPI->WaitForRenderFinish();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		descriptorHeap.Reset();
    }

    void D3D12EditorGUIManager::BeginFrame()
    {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
    }

    void D3D12EditorGUIManager::Update()
    {
    }

    void D3D12EditorGUIManager::EndFrame()
    {
        auto renderAPI = static_cast<EngineCore::D3D12RenderAPI*>(&EngineCore::RenderAPI::GetInstance());


		commandAllocators[renderAPI->mCurrBackBuffer]->Reset();
		commandList->Reset(commandAllocators[renderAPI->mCurrBackBuffer].Get(), NULL);

        auto currentBackBuffer = renderAPI->mBackBuffer[renderAPI->mCurrBackBuffer].resource;
		auto toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &toRenderTarget);

		auto rtv = renderAPI->CurrentBackBufferView();
		commandList->OMSetRenderTargets(1, &rtv, FALSE, NULL);

		ID3D12DescriptorHeap* curDescriptorHeaps[] = { descriptorHeap.Get() };
		commandList->SetDescriptorHeaps(1, curDescriptorHeaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

		auto toPresent = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &toPresent);

		commandList->Close();

		ID3D12CommandList* cmdsLists[] = { commandList.Get() };
		renderAPI->mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		renderAPI->SignalFence(renderAPI->GetFrameFence());
    }
    

    void D3D12EditorGUIManager::Render()
    {
        bool alwaystrue = true;
        ImGui::Begin("Another Window", &alwaystrue);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                alwaystrue = false;
        ImGui::End();
		
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(1920, 1080);
		// Rendering
		
		ImGui::Render();
    }
    
    
    void D3D12EditorGUIManager::InitForDirectX12()
    {
		auto renderAPI = static_cast<EngineCore::D3D12RenderAPI*>(&EngineCore::RenderAPI::GetInstance());

		ImGui_ImplWin32_Init(static_cast<EngineCore::WindowManagerWindows*>(&EngineCore::WindowManager::GetInstance())->GetWindow());

		// ���ﴴ�����������ѣ���һ���������ǹ̶���ImGui���ı�Texture�õģ�ʣ�µ��Ǹ�ͼƬ��Ⱦ�õ�
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = maxDescriptorNum;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (renderAPI->md3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)) != S_OK)
			assert(false);
		descriptorUseState.resize(maxDescriptorNum, false);
		descriptorUseState[0] = true;
		descriptorSize = renderAPI->mCbvSrvUavDescriptorSize;

		ImGui_ImplDX12_Init(renderAPI->md3dDevice.Get(),  renderAPI->MAX_FRAME_INFLIAGHT, renderAPI->mBackBufferFormat, descriptorHeap.Get(),
			descriptorHeap->GetCPUDescriptorHandleForHeapStart(), descriptorHeap->GetGPUDescriptorHandleForHeapStart());

		commandAllocators.resize(renderAPI->MAX_FRAME_INFLIAGHT);
		for (uint32_t i = 0; i < renderAPI->MAX_FRAME_INFLIAGHT; i++)
		{
			if (renderAPI->md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])) != S_OK)
				assert(false);
			commandAllocators[i]->SetName(L"EditorCommandAllocator");
		}
		renderAPI->md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList));
		commandList->Close();
    }

}