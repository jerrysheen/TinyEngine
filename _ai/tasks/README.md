# JSON Task Flow

任务定义只使用一个 JSON 文件：

```text
_ai/tasks/<task-id>.json
```

运行状态写到：

```text
_ai/runs/<task-id>/progress.json
_ai/runs/<task-id>/evidence/
```

## 设计目标

- 任务短而硬，不靠长上下文
- 执行者只看当前 step
- 每个 step 都有固定验收规则
- 验收失败时不允许推进到下一步
- `current_step` 属于运行态，只保存在 `_ai/runs/<task-id>/progress.json`
- verifier 有代码级硬重试上限，保证最终会停在 `blocked`
- PostToolUse 会尝试给支持注释的文本文件自动写入任务来源头注释

## 最小流程

1. 创建 `_ai/tasks/<task-id>.json`
2. 把 `<task-id>` 写入本地 `_ai/active-task.txt`
3. 读取 `_ai/runs/<task-id>/progress.json` 中的 `current_step`，执行当前 step
4. 运行：

```bat
python .harness/scripts/verify_task.py --task-id <task-id>
```

5. 通过后再做下一步；失败则修复并重试

## 运行态文件

`progress.json` 至少包含这些关键字段：

- `status`：任务整体状态
- `current_step`：当前 step id
- `changed_files`：本轮任务触达过的文件
- `steps.<step-id>.retries`：当前 step 已失败次数
- `file_trace`：文件追踪索引，记录首次/最后一次由哪个 task step 触达

## 重试策略

- task 级默认值来自 `max_step_retries`
- step 可以用 `max_retries` 覆盖
- 但最终仍受 Python 代码里的硬上限约束，超过后 step 会变成 `blocked`
- `blocked` 后 verifier 不会自动重置，必须先由人确认真实原因已处理，再手动运行 `retry_step.py`
- `changed_files` 验收现在要求“路径被记录且文件仍然存在于磁盘”

## 失败后如何重试

1. 看 `_ai/runs/<task-id>/progress.json` 里的 `current_step`、`status`、`retries`、`last_error`
2. 看 `_ai/runs/<task-id>/evidence/<current_step>.verify.log`
3. 只修当前 step 的问题
4. 如果当前 step 已 `blocked`，先由人确认真实原因已处理，再手动运行 `python .harness/scripts/retry_step.py --task-id <task-id> --confirm-human --clear-last-error`
5. 然后再运行 `python .harness/scripts/verify_task.py --task-id <task-id>`
6. 如果再次失败，先回头看 `last_error` 和 `verify.log`，不要机械连续重跑 verify
7. 如果再次进入 `blocked`，继续排查真实原因，不要循环执行 reset/retry

## 支持的 acceptance 类型

- `files_exist`
- `changed_files`
- `msbuild`
- `manual_check`

## 模板

参考：

```text
_ai/tasks/task.template.json
```
