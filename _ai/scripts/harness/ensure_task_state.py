#!/usr/bin/env python3
from __future__ import annotations

import sys

from task_runtime import get_active_task_id, load_progress, load_task


def main() -> int:
    task_id = get_active_task_id()
    if not task_id:
        return 0

    task = load_task(task_id)
    progress = load_progress(task)
    status = progress.get("status")
    current_step = progress.get("current_step")

    if status == "done" and current_step is None:
        return 0

    print(f"Task not complete: {task_id}", file=sys.stderr)
    if current_step:
        print(f"Current step: {current_step}", file=sys.stderr)
    print("Run verify_task.py before ending the session.", file=sys.stderr)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
