#include "PreCompiledHeader.h"
#include "EditorConsolePanel.h"
#include "imgui.h"
#include "EditorSettings.h"
#include "Core/Profiler.h"
#include <cstring> // 顶部加一下
namespace EngineEditor
{
    // Console 透明度设置为0.0， 只用来接受以及处理一些SceneView事件
    void EditorConsolePanel::DrawGUI()
    {
        ImGui::SetNextWindowPos(ImVec2((float)EditorSettings::GetConsolePanelStartPos().x, (float)EditorSettings::GetConsolePanelStartPos().y));
        ImGui::SetNextWindowSize(ImVec2((float)EditorSettings::GetConsolePanelSize().x, (float)EditorSettings::GetConsolePanelSize().y));

        if (ImGui::Begin("Console - Profiler", NULL,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize))
        {
#if ENABLE_PROFILER
            // 获取上一帧的性能数据（当前帧可能还在写入）
            EngineCore::Profiler& profiler = EngineCore::Profiler::Get();
            uint32_t currentFrameIndex = profiler.GetCurrentFrameIndex();
            // 获取前一帧的索引
            uint32_t prevFrameIndex = (currentFrameIndex > 0) ? (currentFrameIndex - 1) : (299); // kMaxFrames - 1
            const EngineCore::ProfilerFrame& frame = profiler.GetFrame(prevFrameIndex);

            // 显示帧信息
            float frameTimeMs = profiler.GetLastFrameTimeMs();
            float fps = profiler.GetFPS();
            ImGui::Text("Frame: %u | Frame Time: %.2f ms | FPS: %.1f", prevFrameIndex, frameTimeMs, fps);
            ImGui::Separator();

            // 创建一个可滚动的区域来显示所有事件
            ImGui::BeginChild("ProfilerEvents", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            // 表头
            ImGui::Columns(4, "ProfilerColumns");
            ImGui::SetColumnWidth(0, 300.0f);
            ImGui::SetColumnWidth(1, 100.0f);
            ImGui::SetColumnWidth(2, 100.0f);
            ImGui::SetColumnWidth(3, 80.0f);

            ImGui::Text("Event Name"); ImGui::NextColumn();
            ImGui::Text("Duration (ms)"); ImGui::NextColumn();
            ImGui::Text("Thread ID"); ImGui::NextColumn();
            ImGui::Text("Depth"); ImGui::NextColumn();
            ImGui::Separator();

            // 遍历所有打点事件
            for (const auto& event : frame.events)
            {
                if (event.name != nullptr)
                {
                    float duration = event.endMs - event.startMs;

                    // 根据耗时着色
                    if (duration > 10.0f)
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // 红色：慢
                    else
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // 绿色：快

                    // 根据 depth 添加缩进
                    std::string indentedName = std::string(event.depth * 2, ' ') + event.name;
                    ImGui::Text("%s", indentedName.c_str()); ImGui::NextColumn();
                    ImGui::Text("%.3f", duration); ImGui::NextColumn();
                    ImGui::Text("%u", event.threadID); ImGui::NextColumn();
                    ImGui::Text("%u", event.depth); ImGui::NextColumn();

                    ImGui::PopStyleColor();
                }
            }

     
            ImGui::Columns(1);
            ImGui::EndChild();
#else
            ImGui::Text("Profiler is disabled. Enable ENABLE_PROFILER to see performance data.");
#endif
            ImGui::End();
        }
    }
    
    EditorConsolePanel::~EditorConsolePanel()
    {
    }
}