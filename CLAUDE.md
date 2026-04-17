# CLAUDE.md

TinyEngine 的 Claude Code 薄入口。目标是让任务定义短、执行路径稳定、关键节点由程序化规则判定。

## 基本规则
- 本仓库默认使用中文沟通，必要时保留英文技术术语。
- 优先遵循仓库内可执行约束：`task.json`、`progress.json`、Python verifier、证据产物。
- 任务进行中，优先读取 `_ai/tasks/<task-id>.json` 与 `_ai/runs/<task-id>/progress.json`，而不是把所有过程写回对话。
- 执行者只处理当前 step；当前 step 未通过 verifier 时，不进入下一步。

## Harness 入口
- 项目级配置：`.claude/settings.json`
- 任务定义：`_ai/tasks/<task-id>.json`
- 当前任务指针：`_ai/active-task.txt`（本地文件，可不提交）
- 运行状态：`_ai/runs/<task-id>/progress.json`
- 证据产物：`_ai/runs/<task-id>/evidence/`
- Hook 脚本：`_ai/scripts/harness/`
- 任务模板与说明：`_ai/tasks/README.md`

## 工作流
1. 如果存在 `_ai/active-task.txt`，先读取其中的 `task-id`。
2. 打开 `_ai/tasks/<task-id>.json`，只关注当前 `current_step`。
3. 修改代码后，PostToolUse hook 会把触达文件写入 `progress.json`。
4. 当前 step 完成后，显式运行 `python _ai/scripts/harness/verify_task.py --task-id <task-id>`。
5. verifier 通过：当前 step 标记为 `done` 并推进到下一步；失败：保持当前 step，不允许推进。
6. 会话准备结束时，Stop hook 只检查任务是否真的完成，避免错误宣布完成。

## 常用命令
- 生成 VS2022 工程：`Tools/premake/premake5.exe vs2022`
- 解决方案路径：`Projects/Windows/Visual Studio 2022/TinyEngine.sln`
- 一键构建并运行：`WinBuildAndRun.bat`
- 手动构建：`msbuild "Projects\Windows\Visual Studio 2022\TinyEngine.sln" /p:Configuration=Debug /p:Platform=x64`

## 项目速记
- 语言与构建：C++17 + Premake5 + Visual Studio 2022
- 运行时代码：`Runtime/`
- 编辑器代码：`Editor/`
- 资源目录：`Assets/`
- 设计与记录：`Docs/`、`Note/`
- AI 摘要与任务制品：`_ai/`

## 架构边界
- `Runtime/Graphics` 和 `Runtime/Platforms` 负责底层图形 API 与平台适配。
- `Runtime/Renderer` 负责上层渲染管线与场景渲染逻辑。
- `Runtime/Scene`、`Runtime/Resources`、`Runtime/Managers` 负责场景、资源和管理器体系。
- `Editor/` 只放编辑器 UI 与工具代码，不把平台底层实现混进来。

## 开发注意事项
- 保持与周边代码风格一致：4 空格缩进、PascalCase 类型/函数、局部变量 camelCase。
- 所有包含路径相对于 `Runtime/` 和 `Editor/` 目录。
- Debug 模式使用绝对路径资源，Dist 模式使用相对路径资源。
- 当前仓库没有独立测试框架；关键门禁默认以任务 JSON 中声明的验收规则为准。
