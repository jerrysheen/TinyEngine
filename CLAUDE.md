# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供本代码库的操作指引。

---

## 交互规则
1. 本工程所有回答优先使用中文
2. 交流内容以实现方案讨论和范例代码说明为主，除非明确要求实现代码，否则仅输出方案和代码片段到窗口即可，不需要实际修改文件

---

## 常用命令
### 项目生成
- **生成 VS2022 项目**：运行 `Win-GenProjects.bat`，会自动调用 premake 生成 `Projects/Windows/Visual Studio 2022/TinyEngine.sln`
- 支持 VS2019、Xcode、gmake2 等生成器，可直接运行 `Tools/premake/premake5.exe <生成器名称>`

### 构建与运行
- **一键构建并运行**：运行 `WinBuildAndRun.bat`，会自动配置VS环境，增量编译后启动编辑器
- **手动构建**：
  - 打开生成的 sln 文件，选择 Debug/Release/Dist 配置，x64 平台
  - 直接编译运行即可，调试目录已配置为项目根目录
- 构建产物输出路径：`Projects/Windows/Visual Studio 2022/bin/<配置>-windows-x86_64/EngineCore/`

### 技术栈
- C++17
- 渲染 API：默认 DirectX12（可配置 OpenGL）
- 构建系统：Premake5

---

## 代码架构
### 核心目录结构
```
├── Runtime/          # 引擎核心运行时代码
│   ├── Core/         # 基础核心功能（内存、日志、断言等）
│   ├── Graphics/     # 图形硬件抽象层（RHI），包含各渲染API实现
│   ├── Platforms/    # 平台相关代码（Windows、D3D12 等平台/API 专属实现）
│   ├── Renderer/     # 上层渲染器实现（渲染管线、Pass、场景渲染逻辑等）
│   ├── Scene/        # 场景管理（GameObject、组件、ECS等）
│   ├── Resources/    # 资源管理（模型、纹理、材质等加载与管理）
│   ├── Managers/     # 各类管理器（输入、事件、时间、资源等）
│   └── Math/         # 数学库（向量、矩阵、几何计算等）
├── Editor/           # 编辑器功能代码
├── Vendor/           # 第三方依赖库（Assimp 等）
├── Assets/           # 项目资源文件（模型、纹理、着色器等）
└── Projects/         # 生成的工程文件与构建产物
```

### 架构分层
1. **平台抽象层**：Platforms 目录下实现不同操作系统与渲染API的适配，上层代码与平台无关
2. **RHI 层**：Graphics 目录下的图形硬件抽象层，统一封装 DirectX12/OpenGL 等渲染API的底层操作
3. **核心层**：Core、Math、Utils 等基础模块，为整个引擎提供基础能力
4. **渲染层**：Renderer 目录下的上层渲染逻辑，实现渲染管线、Pass、场景剔除、光照等高级渲染功能
5. **场景层**：Scene、GameObject 等模块，负责游戏对象与场景的管理
6. **编辑器层**：Editor 目录下实现编辑器界面与交互功能

### 核心特性
- 支持多渲染后端（当前重点开发 DirectX12 后端）
- GPU Driven 渲染管线（正在开发 HIZ、GPU Scene 等特性）
- 基于 Precompiled Header 加速构建
- 支持 Debug/Release/Dist 三种构建配置

---

## 开发注意事项
- 所有包含路径相对于 `Runtime/` 和 `Editor/` 目录
- 中文编码统一使用 UTF-8，build 选项已配置
- Debug 模式使用绝对路径访问资源，Dist 模式使用相对路径
