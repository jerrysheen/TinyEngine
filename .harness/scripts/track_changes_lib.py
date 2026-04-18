#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from stamp_provenance import ensure_provenance_header, resolve_path
from task_runtime import ROOT_DIR, load_progress, load_task, now_iso, save_progress


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


def record_paths_batch(task_id: str, raw_paths: list[str], agent_name: str = "TinyEngine harness", batch_size: int = 20) -> int:
    """分批处理文件路径，避免命令行过长"""
    total = 0
    for i in range(0, len(raw_paths), batch_size):
        batch = raw_paths[i:i + batch_size]
        total += record_paths_single_batch(task_id, batch, agent_name)
    return total


def record_paths_single_batch(task_id: str, raw_paths: list[str], agent_name: str = "TinyEngine harness") -> int:
    """单批次处理文件路径"""
    task = load_task(task_id)
    progress = load_progress(task)
    current_step = progress.get("current_step")

    normalized = [item for item in (normalize_path(path) for path in raw_paths) if item]
    if not normalized:
        return 0

    changed_files = list(progress.get("changed_files", []))
    file_trace = dict(progress.get("file_trace", {}))
    touched_at = now_iso()

    for item in normalized:
        if item not in changed_files:
            changed_files.append(item)

        resolved_path = resolve_path(item)
        ensure_provenance_header(resolved_path, task_id, current_step, agent_name)

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
    return len(normalized)


def record_paths(task_id: str, raw_paths: list[str], agent_name: str = "TinyEngine harness") -> int:
    """主入口：自动分批处理大量文件"""
    # 如果文件数量超过20个，使用分批处理
    if len(raw_paths) > 20:
        return record_paths_batch(task_id, raw_paths, agent_name, batch_size=20)
    return record_paths_single_batch(task_id, raw_paths, agent_name)
