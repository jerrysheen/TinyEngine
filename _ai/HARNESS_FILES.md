# TinyEngine Harness Files

这份清单只列当前工程里和 harness 直接相关、适合后续复制到别的仓库继续演化的文件。

## 入口配置

- `.claude/settings.json`
  作用：注册 `PreToolUse`、`PostToolUse`、`Stop` 三类 hooks。
- `.claude/settings.local.json`
  作用：本地机环境相关覆盖配置；复制前先确认里面是否有机器相关路径或个人设置。

## 任务定义与运行态

- `_ai/tasks/task.template.json`
  作用：新任务模板，定义 `task_id`、`steps`、`acceptance`、重试设置。
- `_ai/tasks/README.md`
  作用：任务 JSON 和运行态的规则说明。
- `_ai/active-task.txt`
  作用：当前活跃任务指针；本地运行文件，通常不需要纳入模板仓库。
- `_ai/runs/README.md`
  作用：说明 `progress.json`、`evidence/` 的目录约定。
- `_ai/progress.schema.json`
  作用：约束 `progress.json` 的结构，包括 `current_step`、`steps`、`file_trace`。

## Harness 脚本

- `_ai/scripts/harness/task_runtime.py`
  作用：统一路径、读写 JSON、初始化 progress、step 游标推进辅助。
- `_ai/scripts/harness/pre_tool_guard.py`
  作用：PreToolUse 防线，拦危险 Bash 命令，并在 step 超重试预算后阻止继续 Bash。
- `_ai/scripts/harness/track_changes.py`
  作用：PostToolUse 记录编辑触达文件，维护 `changed_files` 和 `file_trace`。
- `_ai/scripts/harness/verify_task.py`
  作用：执行 acceptance、写 verify 日志、累计 retries、推进 step、落 `blocked`。
- `_ai/scripts/harness/ensure_task_state.py`
  作用：Stop 阶段提醒任务未完成，避免错误宣布完成。
- `_ai/scripts/harness/update_progress.py`
  作用：手动调整 `progress.json` 的辅助脚本。
- `_ai/scripts/harness/retry_step.py`
  作用：当人已经确认当前 step 的真实失败原因被修复后，将它重置为可再次验证的状态；不是自动恢复器。
- `_ai/scripts/harness/stamp_provenance.py`
  作用：给支持注释的生成文件写入 task/step 来源头注释。

## 建议一起复制的最小集合

- `.claude/settings.json`
- `_ai/tasks/task.template.json`
- `_ai/tasks/README.md`
- `_ai/runs/README.md`
- `_ai/progress.schema.json`
- `_ai/scripts/harness/task_runtime.py`
- `_ai/scripts/harness/pre_tool_guard.py`
- `_ai/scripts/harness/track_changes.py`
- `_ai/scripts/harness/verify_task.py`
- `_ai/scripts/harness/ensure_task_state.py`
- `_ai/scripts/harness/update_progress.py`
- `_ai/scripts/harness/retry_step.py`

## 可选复制

- `_ai/scripts/harness/stamp_provenance.py`
- `CLAUDE.md`

如果目标仓库的运行环境、构建命令、语言类型不同，优先改这几处：

- `.claude/settings.json` 的 hook 注册
- `verify_task.py` 里的 acceptance 实现
- `task.template.json` 的默认字段
- `CLAUDE.md` 里的工作流文案
