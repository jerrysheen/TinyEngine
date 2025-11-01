#pragma once
#include "PreCompiledHeader.h"
#include "Platforms/D3D12/d3dUtil.h"
#include "EditorGUIManager.h"
#include "imgui.h"

namespace EngineEditor
{
    class D3D12EditorGUIManager : public EditorGUIManager
    {
    public:
        // 渲染线程beginFrame，重置CommandList
        virtual void BeginFrame() override;
        // 渲染线程EndFrame，提交指令
        virtual void EndFrame() override;
        static void ApplyDpiScale(float scale)
        {
            // 1) 缩放 ImGui Style
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(scale);

            // 2) 重新构建字体（把像素大小乘上 scale）
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();
            // 举例：原 16px 字体，按比例放大
            io.Fonts->AddFontFromFileTTF("c:/Windows/Fonts/segoeui.ttf", 16.0f * scale);
            io.Fonts->Build();

            // 3) 如果你不想重建字体，也可以简单用全局缩放(体验略差)
            // io.FontGlobalScale = scale; // 简易方案（字体不会变清晰度）
        }
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