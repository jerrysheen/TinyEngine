#!/usr/bin/env python3
from __future__ import annotations

import argparse

from task_runtime import get_active_task_id, load_progress, load_task, save_progress


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Reset the current TinyEngine harness step so it can be retried")
    parser.add_argument("--task-id")
    parser.add_argument(
        "--confirm-human",
        action="store_true",
        help="Required. Confirms a human has verified the real failure cause was addressed before retrying.",
    )
    parser.add_argument(
        "--keep-retries",
        action="store_true",
        help="Keep the current retry counter instead of resetting it to 0",
    )
    parser.add_argument(
        "--clear-last-error",
        action="store_true",
        help="Clear last_error for the current step",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not args.confirm_human:
        raise SystemExit("Refusing to reset step state without --confirm-human")

    task_id = args.task_id or get_active_task_id()
    if not task_id:
        raise SystemExit("No task id provided and _ai/active-task.txt is missing")

    task = load_task(task_id)
    progress = load_progress(task)
    current_step_id = progress.get("current_step")
    if not current_step_id:
        raise SystemExit("Task has no current_step. It may already be completed.")

    step_progress = progress.get("steps", {}).get(current_step_id)
    if not step_progress:
        raise SystemExit(f"Current step not found in progress.json: {current_step_id}")

    progress["status"] = "in_progress"
    step_progress["status"] = "in_progress"

    if not args.keep_retries:
        step_progress["retries"] = 0
    if args.clear_last_error:
        step_progress["last_error"] = ""

    save_progress(progress)
    print(f"READY {task_id} {current_step_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
