#!/usr/bin/env python3
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[2]
AI_DIR = ROOT_DIR / "_ai"
TASKS_DIR = AI_DIR / "tasks"
RUNS_DIR = AI_DIR / "runs"
DEFAULT_MAX_STEP_RETRIES = 2
HARD_MAX_STEP_RETRIES = 3


def now_iso() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec="seconds")


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def get_active_task_id() -> str | None:
    active_path = AI_DIR / "active-task.txt"
    if not active_path.exists():
        return None

    value = active_path.read_text(encoding="utf-8").strip()
    return value or None


def task_path(task_id: str) -> Path:
    return TASKS_DIR / f"{task_id}.json"


def run_dir(task_id: str) -> Path:
    return RUNS_DIR / task_id


def progress_path(task_id: str) -> Path:
    return run_dir(task_id) / "progress.json"


def evidence_dir(task_id: str) -> Path:
    return run_dir(task_id) / "evidence"


def load_task(task_id: str) -> dict:
    path = task_path(task_id)
    if not path.exists():
        raise FileNotFoundError(f"Task file not found: {path}")
    task = read_json(path)
    if task.get("task_id") and task["task_id"] != task_id:
        raise ValueError(f"task_id mismatch in {path}")
    task["task_id"] = task_id
    return task


def initial_progress(task: dict) -> dict:
    steps = task.get("steps", [])
    if not steps:
        raise ValueError("Task has no steps")

    progress_steps: dict[str, dict] = {}
    first_step_id = steps[0]["id"]

    for index, step in enumerate(steps):
        progress_steps[step["id"]] = {
            "title": step["title"],
            "status": "in_progress" if index == 0 else "pending",
            "retries": 0,
            "last_error": "",
        }

    return {
        "schema_version": 2,
        "task_id": task["task_id"],
        "status": "in_progress",
        "current_step": first_step_id,
        "changed_files": [],
        "file_trace": {},
        "steps": progress_steps,
        "updated_at": now_iso(),
    }


def load_progress(task: dict) -> dict:
    path = progress_path(task["task_id"])
    if path.exists():
        progress = read_json(path)
        if progress.get("schema_version") == 2:
            return progress
    progress = initial_progress(task)
    save_progress(progress)
    return progress


def save_progress(progress: dict) -> None:
    progress["updated_at"] = now_iso()
    write_json(progress_path(progress["task_id"]), progress)


def find_step(task: dict, step_id: str) -> dict:
    for step in task["steps"]:
        if step["id"] == step_id:
            return step
    raise KeyError(f"Step not found: {step_id}")


def next_step_id(task: dict, current_step_id: str) -> str | None:
    step_ids = [step["id"] for step in task["steps"]]
    idx = step_ids.index(current_step_id)
    if idx + 1 >= len(step_ids):
        return None
    return step_ids[idx + 1]
