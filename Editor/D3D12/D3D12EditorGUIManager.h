#pragma once
#include "PreCompiledHeader.h"
#include "Platforms/D3D12/d3dUtil.h"
#include "Windows/EditorGUIManager.h"
#include "imgui.h"

namespace EngineEditor
{
    class D3D12EditorGUIManager : public EditorGUIManager
    {
    public:
        virtual void BeginFrame() override;
        virtual void EndFrame() override;
        virtual void Update() override;
        virtual void Render() override;

        D3D12EditorGUIManager();
        ~D3D12EditorGUIManager();
	private:

		const uint32_t maxDescriptorNum = 256;

		UINT descriptorSize;
		std::vector<bool> descriptorUseState;
		ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
		ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
		std::vector<ComPtr<ID3D12CommandAllocator>> commandAllocators;

		void InitForDirectX12();
		UINT GetNextAvailablePos();
    };
}