# TinyEngine Harness Rule

## 入口

- 当前任务指针：`_ai/active-task.txt`
- 任务定义：`_ai/tasks/<task-id>.json`
- 运行状态：`_ai/runs/<task-id>/progress.json`
- 核心脚本：`.harness/scripts/`

## 必须遵守的流程

1. 先读取 `_ai/active-task.txt`
2. 当前 step 只以 `_ai/runs/<task-id>/progress.json.current_step` 为准
3. 一次只处理当前 step，不允许跳 step
4. 完成当前 step 后，只运行一次：
   `python .harness/scripts/verify_task.py --task-id <task-id>`
5. verifier 失败后，不允许偷偷推进到下一个 step
6. verifier 失败后，先查看 `progress.json.last_error` 和对应 `verify.log`，只修当前 step 的直接原因
7. 如果 step 已进入 `blocked`，停止自动尝试；只能由人确认真实原因已修复后，显式运行：
   `python .harness/scripts/retry_step.py --task-id <task-id> --confirm-human --clear-last-error`
8. 如果 acceptance 是 `changed_files`，即使目标文件已经存在，也不能直接运行 verifier；必须先对该文件做一次真实写入或改写，让 `PostToolUse` 把它登记进 `progress.json.changed_files`
9. 只做 `read_file`、`search_files`、`execute_command` 不会满足 `changed_files`；当前 step 的目标文件未被写入前，不要重复运行 verifier
10. 如果需要从更早的 step 重新执行，不要手改 `progress.json`；显式运行：
   `python .harness/scripts/rewind_step.py --task-id <task-id> --to <step-id> --confirm-human --clear-last-error`

## 约束

- 不要把 `current_step` 写回 task.json
- 不要手动声称任务完成，除非 `progress.json.status == done` 且 `current_step == null`
- 不要因为 verifier 失败而反复重跑同一条命令；必须先改变代码、配置或证据产物
- 看到 checkpoint 文件“已经存在”时，不要把它当成当前 step 已完成；只有当前 step 重新写入后才允许 verify
- 不要手动编辑 `progress.json` 来回退任务；统一使用 `rewind_step.py`
- 支持注释的文本文件需要带任务来源头注释
- 无法自动注释的文件，至少要通过 `progress.json.file_trace` 可反查

## 落盘与追踪

- `PostToolUse` hook 会自动登记 `changed_files`
- `PostToolUse` hook 会自动维护 `file_trace`
- `PostToolUse` hook 会为支持注释的文本文件自动追加 provenance 头注释
