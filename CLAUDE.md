# CLAUDE.md

TinyEngine 的 Claude Code 薄入口。目标是让任务定义短、执行路径稳定、关键节点由程序化规则判定。

## 基本规则
- 本仓库默认使用中文沟通，必要时保留英文技术术语。
- 优先遵循仓库内可执行约束：`task.json`、`progress.json`、Python verifier、证据产物。
- 任务进行中，优先读取 `_ai/tasks/<task-id>.json` 与 `_ai/runs/<task-id>/progress.json`，而不是把所有过程写回对话。
- 执行者只处理当前 step；当前 step 未通过 verifier 时，不进入下一步。

## Harness 入口
- 项目级配置：`.claude/settings.json`
- Cursor 配置：`.cursor/hooks.json`、`.cursor/rules/harness.mdc`
- 可迁移 harness：`.harness/`
- 任务定义：`_ai/tasks/<task-id>.json`
- 当前任务指针：`_ai/active-task.txt`（本地文件，可不提交）
- 运行状态：`_ai/runs/<task-id>/progress.json`
- 证据产物：`_ai/runs/<task-id>/evidence/`
- Hook 脚本：`.harness/scripts/`
- 手动重试：`python .harness/scripts/retry_step.py --task-id <task-id> --confirm-human --clear-last-error`
- 回退到指定 step 重新执行：`python .harness/scripts/rewind_step.py --task-id <task-id> --to <step-id> --confirm-human --clear-last-error`
- 任务模板与说明：`_ai/tasks/README.md`

## 工作流
1. 如果存在 `_ai/active-task.txt`，先读取其中的 `task-id`。
2. 打开 `_ai/tasks/<task-id>.json` 读取 step 定义、验收规则和 `max_step_retries`；当前运行态只认 `_ai/runs/<task-id>/progress.json`。
3. 当前 step 以 `_ai/runs/<task-id>/progress.json` 里的 `current_step` 为准；`task.json` 不保存运行中的 step 游标。
4. 修改代码后，PostToolUse hook 会把触达文件写入 `progress.json.changed_files`，为支持注释的文本文件自动写入或刷新任务头注释，并补充文件追踪信息。
5. 当前 step 完成后，显式运行 `python .harness/scripts/verify_task.py --task-id <task-id>`。
6. verifier 通过：当前 step 标记为 `done` 并推进到下一步；失败：重试次数加一，超过 Python 代码内硬编码上限后进入 `blocked`，不允许继续推进。
7. 会话准备结束时，Stop hook 只检查任务是否真的完成，避免错误宣布完成。
8. 如果需要从更早的 step 重新执行，不要手改 `progress.json`；显式运行 `python .harness/scripts/rewind_step.py --task-id <task-id> --to <step-id> --confirm-human --clear-last-error`。

## Hook 行为
- `PreToolUse`：拦截 Bash 命令，阻止明显高风险操作，例如递归删除、格式化盘符、把内容通过管道直接喂给 shell、直接 push 到 `main/master`。
- `PostToolUse`：在 `Edit|Write|MultiEdit` 后记录触达文件，自动给支持注释的文本文件写入/刷新任务头注释，并更新 `_ai/runs/<task-id>/progress.json` 中的 `changed_files` 与文件追踪索引。
- `Stop`：会话结束前读取 `progress.json`，如果任务未完成则打印当前 step 和状态，防止错误宣称“任务已完成”。

## 重试边界
- `task.json` 可以声明 `max_step_retries` 或 step 级 `max_retries`，作为任务期望值。
- Python verifier 还会应用代码中的硬上限；即使 JSON 配置写得更大，也会被截断，保证 harness 一定停止，不会无限重试。

## 失败后重试
1. 先打开 `_ai/runs/<task-id>/progress.json`，确认 `current_step`、`steps.<current_step>.status`、`steps.<current_step>.retries` 和 `last_error`。
2. 再打开 `_ai/runs/<task-id>/evidence/<current_step>.verify.log`，看 verifier 失败的直接原因。
3. 只修当前 `current_step` 对应的问题，不要跳到后续 step。
4. 失败后先看 `progress.json` 的 `last_error` 和当前 step 的 `verify.log`，确认原因后再决定是否重试。
5. 修完后重新运行 `python .harness/scripts/verify_task.py --task-id <task-id>`。
6. 如果 verifier 通过，harness 会自动推进到下一个 step；如果失败但未超上限，继续停在当前 step。
7. 如果 step 进入 `blocked`，不要继续盲试；停下来排查真实原因，由人确认问题已经修复后，才允许手动运行 `python .harness/scripts/retry_step.py --task-id <task-id> --confirm-human --clear-last-error`。
8. `retry_step.py` 不是自动恢复机制，而是人工确认后的显式解锁动作；未带 `--confirm-human` 不允许执行。
9. `PreToolUse` 会在 step 已 `blocked` 或重试次数已超有效上限时直接拦截新的 Bash 命令。
10. 如果需要从历史 step 重做，使用 `rewind_step.py` 回退执行指针；默认只重置状态，不自动删除已有 evidence 文件。

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
