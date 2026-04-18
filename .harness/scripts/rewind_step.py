#!/usr/bin/env python3
from __future__ import annotations

import argparse

from task_runtime import find_step, get_active_task_id, load_progress, load_task, save_progress


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Rewind TinyEngine harness progress to a specified step")
    parser.add_argument("--task-id")
    parser.add_argument("--to", required=True, help="Target step id to rewind to")
    parser.add_argument(
        "--confirm-human",
        action="store_true",
        help="Required. Confirms a human intentionally wants to replay work from the target step.",
    )
    parser.add_argument(
        "--clear-last-error",
        action="store_true",
        help="Clear last_error for the rewound step and all later steps",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not args.confirm_human:
        raise SystemExit("Refusing to rewind task state without --confirm-human")

    task_id = args.task_id or get_active_task_id()
    if not task_id:
        raise SystemExit("No task id provided and _ai/active-task.txt is missing")

    task = load_task(task_id)
    progress = load_progress(task)
    target_step = find_step(task, args.to)
    target_step_id = target_step["id"]
    step_ids = [step["id"] for step in task["steps"]]
    target_index = step_ids.index(target_step_id)

    progress["current_step"] = target_step_id
    progress["status"] = "in_progress"

    for index, step_id in enumerate(step_ids):
        step_progress = progress["steps"][step_id]
        if index < target_index:
            continue

        step_progress["status"] = "in_progress" if index == target_index else "pending"
        step_progress["retries"] = 0
        if args.clear_last_error:
            step_progress["last_error"] = ""
        step_progress.pop("verified_at", None)

    save_progress(progress)
    print(f"REWOUND {task_id} {target_step_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
