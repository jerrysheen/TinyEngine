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
- `progress.json.current_step` 才是当前 step 的唯一运行态来源
- `progress.json.file_trace` 用来反查文件与 task/step 的对应关系
- `evidence/` 存放 step 验证日志与构建证据
- 原始日志默认不强制提交，必要时可挑选摘要产物纳入版本控制
- harness 脚本统一放在 `.harness/scripts/`
