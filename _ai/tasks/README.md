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

## 最小流程

1. 创建 `_ai/tasks/<task-id>.json`
2. 把 `<task-id>` 写入本地 `_ai/active-task.txt`
3. 执行当前 step
4. 运行：

```bat
python _ai/scripts/harness/verify_task.py --task-id <task-id>
```

5. 通过后再做下一步；失败则修复并重试

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
