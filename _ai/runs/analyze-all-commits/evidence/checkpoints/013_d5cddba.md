# Checkpoint 013: d5cddba

## Commit 标题
Fix： 调整UI位置对不上的问题， 是因为窗口大小和RT size不一致。

## 核心改动
1. **UI显示错位问题修复**：
   - 修改 `Editor/D3D12/D3D12EditorGUIManager.cpp/h` 共33行代码，修复ImGui显示错位问题：
     - 添加窗口大小和渲染目标大小的适配逻辑，处理DPI缩放导致的尺寸不一致问题
     - 新增UI显示区域调整功能，确保ImGui坐标和屏幕坐标正确对应
     - 优化ImGui的视口设置，适配不同分辨率和缩放比例的显示设备
   - 修改 `Runtime/Platforms/D3D12/D3D12RenderAPI.cpp/h` 共11行代码，添加渲染目标尺寸获取接口，供GUI系统查询当前RT的实际大小
   - 修改 `Runtime/Platforms/Windows/WindowManagerWindows.cpp` 16行代码，完善窗口大小查询和DPI感知功能，正确获取窗口的物理像素尺寸

2. **新增D3D12结构定义头文件**：
   - 新增 `Runtime/Platforms/D3D12/D3D12Struct.h` 共19行代码，集中定义D3D12相关的通用结构体和类型别名，优化代码结构，减少重复定义

3. **项目配置更新**：
   - 调整 `Projects/Windows/Visual Studio 2022/EngineCore.vcxproj.filters` 100行代码，优化项目文件目录结构，重新分类整理D3D12相关源码文件，提升项目可维护性
   - 新增 `Projects/Windows/Visual Studio 2022/EngineCore/imgui.ini` 10行配置，保存默认的ImGui窗口布局，提升开发体验

## 涉及模块
- 编辑器GUI系统
- D3D12渲染后端
- 窗口管理器
- 项目构建配置

## 风险点
- DPI缩放适配可能在多显示器不同缩放比例的场景下出现问题
- 窗口大小和渲染目标大小的同步逻辑如果处理不当，可能在窗口最大化、最小化或者快速Resize时出现渲染异常
- 项目文件过滤器调整可能导致部分IDE的智能提示出现暂时异常，重新加载项目即可解决

## 推测错误原因
在高DPI显示器或者用户调整了系统缩放比例的情况下，Win32窗口的逻辑像素大小和实际物理像素大小不一致，而渲染目标使用的是物理像素尺寸，导致ImGui的坐标计算错误，出现UI错位、点击位置不准等问题。本次修复正确处理了逻辑像素和物理像素的转换，同步窗口大小和渲染目标大小，解决了UI显示错位问题。同时优化了项目结构和开发体验配置。
