#!/usr/bin/env python3
from __future__ import annotations

import sys

# 清空命令行参数，避免 ENAMETOOLONG 错误
# hooks 系统会自动传递文件路径作为参数，但我们从 stdin 的 payload 读取
sys.argv = [sys.argv[0]] if sys.argv else ['cursor_after_file_edit.py']

from payload_adapter import extract_file_paths, load_payload
from task_runtime import get_active_task_id
from track_changes_lib import record_paths


def main() -> int:
    task_id = get_active_task_id()
    if not task_id:
        return 0

    payload = load_payload()
    if not payload:
        return 0

    record_paths(task_id, extract_file_paths(payload))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
