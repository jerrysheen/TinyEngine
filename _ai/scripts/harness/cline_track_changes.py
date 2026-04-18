#!/usr/bin/env python3
"""Cline PostToolUse hook adapter for TinyEngine harness.

Cline sends JSON with this structure:
{
    "taskId": "abc123",
    "hookName": "PostToolUse",
    "postToolUse": {
        "tool": "write_to_file",
        "parameters": { "path": "..." },
        "result": "...",
        "success": true,
        "durationMs": 100
    }
}

This adapter tracks file changes and stamps provenance headers.
"""
from __future__ import annotations

import json
import sys
from datetime import datetime, timezone
from pathlib import Path

from task_runtime import ROOT_DIR, get_active_task_id, load_progress, load_task, save_progress
from stamp_provenance import ensure_provenance_header, supports_provenance_header


DEBUG_LOG = ROOT_DIR / "_ai" / "runs" / "cline_posttool_debug.log"
RAW_DEBUG_LOG = ROOT_DIR / "_ai" / "runs" / "cline_posttool_raw.log"


def now_iso() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec="seconds")


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


def append_debug(payload: dict, note: str, raw_paths: list[str] | None = None, normalized: list[str] | None = None) -> None:
    try:
        DEBUG_LOG.parent.mkdir(parents=True, exist_ok=True)
        record = {
            "time": now_iso(),
            "note": note,
            "payload": payload,
            "raw_paths": raw_paths or [],
            "normalized": normalized or [],
        }
        with DEBUG_LOG.open("a", encoding="utf-8") as f:
            f.write(json.dumps(record, ensure_ascii=False) + "\n")
    except Exception:
        pass


def append_raw_debug(note: str, raw_input: str) -> None:
    try:
        RAW_DEBUG_LOG.parent.mkdir(parents=True, exist_ok=True)
        record = {
            "time": now_iso(),
            "note": note,
            "raw_len": len(raw_input),
            "raw_preview": raw_input[:2000],
        }
        with RAW_DEBUG_LOG.open("a", encoding="utf-8") as f:
            f.write(json.dumps(record, ensure_ascii=False) + "\n")
    except Exception:
        pass


def main() -> int:
    task_id = get_active_task_id()
    if not task_id:
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    raw_input = sys.stdin.read()
    if raw_input.startswith("\ufeff"):
        raw_input = raw_input.lstrip("\ufeff")
    append_raw_debug("stdin_received", raw_input)

    try:
        payload = json.loads(raw_input)
    except json.JSONDecodeError:
        append_raw_debug("json_decode_error", raw_input)
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    task = load_task(task_id)
    progress = load_progress(task)
    current_step = progress.get("current_step")

    post_tool = payload.get("postToolUse", {})
    tool_name = post_tool.get("tool") or post_tool.get("toolName", "")
    success = post_tool.get("success")
    
    # Only track file edit tools
    if success is False:
        append_debug(payload, "skip_unsuccessful")
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    if tool_name not in ("write_to_file", "replace_in_file", "Edit", "Write", "MultiEdit"):
        append_debug(payload, f"skip_tool:{tool_name}")
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    params = post_tool.get("parameters", {})
    raw_paths: list[str] = []
    
    for key in ("path", "file_path", "absolutePath"):
        value = params.get(key)
        if value:
            raw_paths.append(value)

    for key in ("target_file", "targetFile", "output_path", "outputPath"):
        value = params.get(key)
        if value:
            raw_paths.append(value)

    for value in params.get("file_paths", []) or []:
        if value:
            raw_paths.append(value)

    normalized = [item for item in (normalize_path(path) for path in raw_paths) if item]
    if not normalized:
        append_debug(payload, "no_normalized_paths", raw_paths=raw_paths, normalized=normalized)
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    changed_files = list(progress.get("changed_files", []))
    file_trace = dict(progress.get("file_trace", {}))
    touched_at = now_iso()

    for item in normalized:
        if item not in changed_files:
            changed_files.append(item)

        resolved_path = Path(ROOT_DIR) / item
        ensure_provenance_header(resolved_path, task_id, current_step, "TinyEngine harness")

        trace = dict(file_trace.get(item, {}))
        if not trace:
            trace["first_seen_at"] = touched_at
            trace["created_by_task"] = task_id
            trace["created_by_step"] = current_step
        trace["last_seen_at"] = touched_at
        trace["last_touched_by_task"] = task_id
        trace["last_touched_by_step"] = current_step
        file_trace[item] = trace

    progress["changed_files"] = changed_files
    progress["file_trace"] = file_trace
    save_progress(progress)
    append_debug(payload, "tracked", raw_paths=raw_paths, normalized=normalized)

    print('{"cancel":false,"contextModification":"","errorMessage":""}')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
