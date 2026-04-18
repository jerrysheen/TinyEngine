#!/usr/bin/env python3
from __future__ import annotations

import argparse

from task_runtime import get_active_task_id, load_progress, load_task, save_progress


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Update TinyEngine harness progress.json")
    parser.add_argument("--task-id")
    parser.add_argument("--status")
    parser.add_argument("--add-file", action="append", default=[])
    parser.add_argument("--current-step")
    return parser.parse_args()


def merge_unique(existing: list[str], new_items: list[str]) -> list[str]:
    merged: list[str] = []
    seen: set[str] = set()

    for item in existing + new_items:
        if not item or item in seen:
            continue
        seen.add(item)
        merged.append(item)
    return merged


def main() -> int:
    args = parse_args()
    task_id = args.task_id or get_active_task_id()
    if not task_id:
        raise SystemExit("No task id provided and _ai/active-task.txt is missing")

    task = load_task(task_id)
    progress = load_progress(task)

    if args.status:
        progress["status"] = args.status
    progress["changed_files"] = merge_unique(progress.get("changed_files", []), args.add_file)
    if args.current_step is not None:
        progress["current_step"] = args.current_step

    save_progress(progress)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
