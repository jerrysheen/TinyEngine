# Cursor Adapter

这个目录只放 Cursor 相关入口配置：

- `hooks.json`：把 Cursor 的 hooks 事件映射到 `.harness/scripts/*.py`
- `rules/harness.mdc`：告诉 Cursor Agent 当前仓库必须遵守的 harness 规则

## 说明

- `beforeShellExecution` 用于阻断危险 shell 和超预算重试
- `afterFileEdit` 用于登记 `changed_files`、`file_trace`、自动写 provenance 头
- `stop` 用于会话结束时提醒任务状态
- 如果要从历史 step 重新执行，使用 `.harness/scripts/rewind_step.py`，不要手改 `progress.json`

项目级 hooks 使用 `.cursor/hooks.json`。根据 Cursor 官方说明，hooks 可以放在项目级 `.cursor/hooks.json` 或用户级 `~/.cursor/hooks.json`，而 `beforeShellExecution` / `afterFileEdit` / `stop` 都属于 Agent loop 相关事件。[来源 1](https://cursor.com/blog/enterprise) [来源 2](https://forum.cursor.com/t/enterprise-managed-hooks-not-showing/152082/3)
