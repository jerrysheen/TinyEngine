# TinyEngine Portable Harness

这套目录是可迁移的 harness 核心，目标是把编辑器/Agent 适配层和任务状态机拆开：

- `.harness/`：可打包带走的共享脚本与说明
- `.claude/`：Claude Code 的 hook 入口配置
- `.cursor/`：Cursor 的 hook 与 rules 入口配置
- `_ai/`：当前仓库里的任务定义、运行态、证据产物

## 设计原则

- harness 核心尽量不依赖某一个 IDE 的私有目录结构
- 编辑器差异只放在薄适配层
- 任务定义与运行态继续使用 `_ai/tasks`、`_ai/runs`
- `blocked` 后只能人工确认后显式执行 `retry_step.py`

## 目录说明

- `scripts/task_runtime.py`
  共享路径、读写 JSON、progress 初始化等基础能力
- `scripts/verify_task.py`
  当前 step 验证与状态推进
- `scripts/retry_step.py`
  人工确认后的显式解锁入口
- `scripts/rewind_step.py`
  将执行指针回退到指定 step，并重置该 step 及后续 step 的状态
- `scripts/update_progress.py`
  手动调整运行态
- `scripts/stamp_provenance.py`
  按文件类型写入头注释
- `scripts/track_changes_lib.py`
  共享的落盘文件登记逻辑
- `scripts/shell_guard_lib.py`
  共享的命令阻断逻辑
- `scripts/claude_*.py`
  Claude Code 入口适配
- `scripts/cursor_*.py`
  Cursor hooks 入口适配

## 当前绑定的数据目录

这套 portable harness 默认仍然读写：

- `_ai/active-task.txt`
- `_ai/tasks/*.json`
- `_ai/runs/<task-id>/progress.json`
- `_ai/runs/<task-id>/evidence/*`

这样做是为了不打断当前仓库的任务流。将来如果你想把 `_ai` 也抽成可迁移模板，再单独改 `task_runtime.py` 即可。
