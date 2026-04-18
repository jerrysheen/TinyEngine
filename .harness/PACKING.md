# Packing And Migration

这份文档说明如何把 TinyEngine 当前这套 harness 打包带到别的仓库。

目标不是复制整个 TinyEngine，而是复制一套“可工作的任务状态机 + 编辑器适配层 + 最小文档”。

## 推荐打包结构

优先一起打包这四部分：

- `.harness/`
- `.claude/`
- `.cursor/`
- `CLAUDE.md`

如果目标仓库也要沿用当前任务数据结构，再一起带：

- `_ai/tasks/`
- `_ai/runs/README.md`
- `_ai/progress.schema.json`
- `_ai/active-task.txt` 只在本地需要时创建，不建议作为模板固定提交

## 最小可运行集合

如果你只想最快落地，至少带这些：

- `.harness/scripts/task_runtime.py`
- `.harness/scripts/verify_task.py`
- `.harness/scripts/retry_step.py`
- `.harness/scripts/update_progress.py`
- `.harness/scripts/stamp_provenance.py`
- `.harness/scripts/track_changes_lib.py`
- `.harness/scripts/shell_guard_lib.py`
- `.harness/scripts/claude_pre_tool_guard.py`
- `.harness/scripts/claude_track_changes.py`
- `.harness/scripts/cursor_before_shell_execution.py`
- `.harness/scripts/cursor_after_file_edit.py`
- `.harness/scripts/cursor_stop.py`
- `.claude/settings.json`
- `.cursor/hooks.json`
- `.cursor/rules/harness.mdc`
- `CLAUDE.md`

## 任务数据层

当前 harness 默认把任务数据放在：

- `_ai/active-task.txt`
- `_ai/tasks/<task-id>.json`
- `_ai/runs/<task-id>/progress.json`
- `_ai/runs/<task-id>/evidence/`

这意味着迁移时你有两种模式：

### 模式 1：保持 `_ai/` 目录不变

适用场景：
- 你想最快复用
- 不介意目标仓库继续使用 `_ai/` 命名

做法：
- 直接复制 `_ai/tasks/README.md`
- 复制 `_ai/tasks/task.template.json`
- 复制 `_ai/runs/README.md`
- 复制 `_ai/progress.schema.json`
- 在新仓库里新建 `_ai/tasks/`、`_ai/runs/`

优点：
- 改动最小
- `.harness/scripts/task_runtime.py` 不需要改

### 模式 2：把 `_ai/` 改成别的目录

适用场景：
- 你想做更通用的模板
- 目标仓库不想暴露 `_ai/` 命名

做法：
- 修改 `.harness/scripts/task_runtime.py`
- 主要改这几个常量：
  - `AI_DIR`
  - `TASKS_DIR`
  - `RUNS_DIR`

注意：
- 这一改动会影响所有 harness 脚本
- 迁移后要重新检查 `CLAUDE.md`、`.cursor/rules/harness.mdc` 和任务模板里的路径文案

## Claude Code 迁移步骤

1. 复制 `.harness/`
2. 复制 `.claude/settings.json`
3. 复制 `CLAUDE.md`
4. 准备任务数据目录 `_ai/tasks/`、`_ai/runs/`
5. 在新仓库本地写入 `_ai/active-task.txt`
6. 用 Claude Code 打开仓库
7. 确认 `.claude/settings.json` 的 hook 路径在新仓库仍然有效

检查点：
- Claude 的 `PreToolUse` 是否能调用 `.harness/scripts/claude_pre_tool_guard.py`
- Claude 的 `PostToolUse` 是否能调用 `.harness/scripts/claude_track_changes.py`
- `Stop` 是否能调用 `.harness/scripts/ensure_task_state.py`

## Cursor 迁移步骤

1. 复制 `.harness/`
2. 复制 `.cursor/hooks.json`
3. 复制 `.cursor/rules/harness.mdc`
4. 复制 `CLAUDE.md`
5. 准备任务数据目录 `_ai/tasks/`、`_ai/runs/`
6. 在 Cursor 中打开仓库
7. 确认工作区是 Trusted
8. 打开 `View > Output > Hooks` 检查 hooks 是否已加载

检查点：
- `beforeShellExecution` 是否触发 `.harness/scripts/cursor_before_shell_execution.py`
- `afterFileEdit` 是否触发 `.harness/scripts/cursor_after_file_edit.py`
- `stop` 是否触发 `.harness/scripts/cursor_stop.py`

注意：
- Cursor hooks 在不同版本下有过兼容性问题
- 如果 hooks 没反应，先看 `Output > Hooks`
- 再确认 `.cursor/hooks.json` 是项目级启用而不是被本地策略屏蔽

## 哪些文件通常不用打包

这些通常属于仓库运行时数据，不建议当模板固定带走：

- `_ai/runs/<task-id>/progress.json`
- `_ai/runs/<task-id>/evidence/*`
- `_ai/active-task.txt`
- `.claude/settings.local.json`

## 打包前建议清理

在把 harness 作为模板发给别人之前，建议：

1. 删除具体任务运行产物
2. 只保留 `_ai/tasks/task.template.json` 和说明文档
3. 确认 `.cursor/hooks.json` 和 `.claude/settings.json` 只引用相对路径
4. 确认没有机器私有路径写死在脚本和配置里

## 迁移后第一轮自检

迁移完成后，建议做这组最小验证：

1. 新建一个最小任务 JSON
2. 写入 `_ai/active-task.txt`
3. 随便编辑一个支持注释的文本文件
4. 检查：
   - `progress.json` 是否被自动创建
   - `changed_files` 是否被记录
   - 文件顶部是否出现 provenance 头注释
5. 手动运行：
   - `python .harness/scripts/verify_task.py --task-id <task-id>`
6. 如果故意让它失败，检查：
   - `retries` 是否增加
   - 超过上限后是否进入 `blocked`
7. 再运行：
   - `python .harness/scripts/retry_step.py --task-id <task-id> --confirm-human --clear-last-error`

## 后续可继续抽象的地方

如果后面你想把它做成真正通用模板，优先继续改这几处：

- 让 `task_runtime.py` 支持从环境变量读取数据目录，而不是写死 `_ai`
- 给 `.cursor/hooks.json` 和 `.claude/settings.json` 增加更统一的生成方式
- 把任务模板、schema、运行态目录再从 `_ai/` 抽成 `.harness/template/`
