#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path

from stamp_provenance import ensure_provenance_header, resolve_path
from task_runtime import ROOT_DIR, get_active_task_id, load_progress, load_task, now_iso, save_progress


def normalize_path(raw_path: str) -> str | None:
    """Convert a hook-reported file path into a repo-relative Windows-style path.

    The PostToolUse payload may contain absolute or relative paths depending on
    which editor operation fired the hook. progress.json stores normalized
    project-relative paths so later acceptance checks can compare them directly.
    """
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
    """Record files touched by edit tools into the active task runtime state.

    Flow:
    1. Resolve the active task from _ai/active-task.txt.
    2. Read the hook payload from stdin and extract one or more file paths.
    3. Normalize paths into repo-relative strings.
    4. Append new paths to progress.json.changed_files.
    5. Auto-stamp supported text files with a provenance header.
    6. Update progress.json.file_trace so we can answer which task/step first
       created or most recently touched a file.

    This script intentionally does not decide whether a step is complete. It
    only accumulates edit evidence for verify_task.py to consume later.
    """
    task_id = get_active_task_id()
    if not task_id:
        return 0

    try:
        payload = json.load(sys.stdin)
    except json.JSONDecodeError:
        return 0

    task = load_task(task_id)
    progress = load_progress(task)
    current_step = progress.get("current_step")

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

    # changed_files is the coarse-grained "was this file touched at all during
    # the task?" list used by changed_files acceptance checks.
    changed_files = list(progress.get("changed_files", []))

    # file_trace is the fine-grained reverse index used for auditability. It
    # records both the first task/step that touched a file and the latest one.
    file_trace = dict(progress.get("file_trace", {}))
    touched_at = now_iso()
    for item in normalized:
        if item not in changed_files:
            changed_files.append(item)

        # Best-effort provenance stamping for supported text files. Headers are
        # intentionally append-only so the file keeps a visible local edit trail.
        # Unsupported types still remain traceable through progress.json.file_trace.
        resolved_path = resolve_path(item)
        ensure_provenance_header(resolved_path, task_id, current_step, "TinyEngine harness")

        trace = dict(file_trace.get(item, {}))
        if not trace:
            # The first writer wins for provenance so we can trace origin later.
            trace["first_seen_at"] = touched_at
            trace["created_by_task"] = task_id
            trace["created_by_step"] = current_step
        # The last-touch fields are updated on every edit to show the latest
        # task/step that modified the file.
        trace["last_seen_at"] = touched_at
        trace["last_touched_by_task"] = task_id
        trace["last_touched_by_step"] = current_step
        file_trace[item] = trace

    progress["changed_files"] = changed_files
    progress["file_trace"] = file_trace
    save_progress(progress)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
