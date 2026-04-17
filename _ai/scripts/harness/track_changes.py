#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path

from task_runtime import ROOT_DIR, get_active_task_id, load_progress, load_task, save_progress


def normalize_path(raw_path: str) -> str | None:
    if not raw_path:
        return None

    candidate = Path(raw_path)
    if not candidate.is_absolute():
        candidate = ROOT_DIR / candidate

    try:
        resolved = candidate.resolve(strict=False)
        return str(resolved.relative_to(ROOT_DIR)).replace("/", "\\")
    except Exception:
        return None


def main() -> int:
    task_id = get_active_task_id()
    if not task_id:
        return 0

    try:
        payload = json.load(sys.stdin)
    except json.JSONDecodeError:
        return 0

    task = load_task(task_id)
    progress = load_progress(task)

    tool_input = payload.get("tool_input") or payload.get("parameters") or {}
    raw_paths: list[str] = []

    for key in ("file_path", "path"):
        value = tool_input.get(key)
        if value:
            raw_paths.append(value)

    for value in tool_input.get("file_paths", []) or []:
        if value:
            raw_paths.append(value)

    normalized = [item for item in (normalize_path(path) for path in raw_paths) if item]
    if not normalized:
        return 0

    changed_files = list(progress.get("changed_files", []))
    for item in normalized:
        if item not in changed_files:
            changed_files.append(item)

    progress["changed_files"] = changed_files
    save_progress(progress)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
