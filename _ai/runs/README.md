# _ai/runs

每个任务在这里生成独立运行目录：

```text
_ai/runs/<task-id>/
  progress.json
  evidence/
    *.verify.log
    build.log
```

约定：
- `progress.json` 是当前任务状态机快照
- `evidence/` 存放 step 验证日志与构建证据
- 原始日志默认不强制提交，必要时可挑选摘要产物纳入版本控制
- harness 脚本统一放在 `_ai/scripts/harness/`
