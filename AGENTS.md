# Repository Guidelines

## Project Structure & Module Organization
`Runtime/` 为引擎核心代码（渲染、资源、序列化、平台层等）；`Editor/` 为编辑器 UI 与工具；`Assets/` 存放纹理等资源；`Projects/Windows/Visual Studio 2022/` 为生成的 VS 解决方案与中间产物；`Tools/premake/` 提供本地 premake；`Vendor/` 为第三方依赖；`Docs/` 和 `Note/` 为设计与记录文档；`_ai/` 为 AI 生成的摘要与配置。

## Build, Test, and Development Commands
常用流程以 Windows + VS2022 为主：
```bat
Win-GenProjects.bat
```
生成 `Projects/Windows/Visual Studio 2022/TinyEngine.sln`（基于 `premake5.lua`）。

```bat
WinBuildAndRun.bat
```
用 `msbuild` 构建 Debug x64，并运行 `EngineCore.exe`。

也可直接打开 `Projects/Windows/Visual Studio 2022/TinyEngine.sln` 使用 VS 编译运行。

## Coding Style & Naming Conventions
项目无统一格式化工具，请保持与周边代码一致：4 空格缩进、花括号独立成行、`namespace EngineCore` 风格。类型/类名采用 PascalCase（如 `RenderEngine`），函数名 PascalCase，局部变量多为 camelCase，静态成员常用 `s_` 前缀（如 `s_Instance`）。新增文件尽量沿用现有目录与命名模式（如 `Runtime/Renderer/`）。

## Testing Guidelines
当前未看到独立测试框架或测试目录。改动后请至少运行 `WinBuildAndRun.bat`，并在关键场景（例如加载 Bistro 场景或资源加载路径）做手动验证。若新增功能可被自动化验证，欢迎补充简单测试或运行脚本。

## Commit & Pull Request Guidelines
近期提交信息通常使用前缀：`D:`（功能/开发）、`F:`（修复）、`Docs:`（文档），示例：`F: typo fix`、`D：场景序列化`。提交信息保持简短，清晰描述改动点。

PR 建议包含：变更概述、关键文件列表、运行/测试方式；涉及渲染或编辑器 UI 变更时附截图或录屏；如有关联任务或缺陷请注明。

## Agent-Specific Instructions
在本仓库范围内默认用中文沟通，必要时用英文术语；若缺少构建或测试细节，请先在 PR 或说明中补充。
