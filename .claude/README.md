# Claude Adapter

这个目录只放 Claude Code 相关入口配置。

- `settings.json`：把 Claude 的 `PreToolUse` / `PostToolUse` / `Stop` 映射到 `.harness/scripts/*.py`
- `settings.local.json`：本机覆盖配置，通常不需要打包给别人

迁移时，优先一起带走：

- `.claude/settings.json`
- `.harness/`

如果目标仓库不需要 Claude Code，可以只保留 `.cursor/` 和 `.harness/`。
