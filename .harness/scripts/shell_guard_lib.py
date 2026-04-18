#!/usr/bin/env python3
from __future__ import annotations

import re

from task_runtime import DEFAULT_MAX_STEP_RETRIES, HARD_MAX_STEP_RETRIES, find_step, get_active_task_id, load_progress, load_task


BLOCKED_PATTERNS = [
    r"rm\s+-r[fF]\b",
    r"\brd\s+/s\s+/q\b",
    r"\bdel\s+/[a-z]*[sq][a-z]*\b",
    r"\bRemove-Item\b.*-Recurse\b.*-Force\b",
    r"\bformat\s+[a-z]:",
    r"\bgit\s+push\s+.*\bmain\b",
    r"\bgit\s+push\s+.*\bmaster\b",
    r"\bcurl\b.*\|\s*(?:bash|sh|pwsh|powershell)\b",
    r"\bInvoke-WebRequest\b.*\|\s*iex\b",
]


def check_blocked_patterns(command: str) -> str | None:
    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command, flags=re.IGNORECASE):
            return pattern
    return None


def check_retry_budget() -> str | None:
    task_id = get_active_task_id()
    if not task_id:
        return None

    task = load_task(task_id)
    progress = load_progress(task)
    current_step_id = progress.get("current_step")
    if not current_step_id:
        return None

    step = find_step(task, current_step_id)
    configured_max_retries = int(step.get("max_retries", task.get("max_step_retries", DEFAULT_MAX_STEP_RETRIES)))
    effective_max_retries = min(configured_max_retries, HARD_MAX_STEP_RETRIES)
    step_progress = progress.get("steps", {}).get(current_step_id, {})
    step_status = step_progress.get("status")
    retries = int(step_progress.get("retries", 0))

    if progress.get("status") == "blocked" or step_status == "blocked":
        return (
            f"task {task_id} is blocked at step {current_step_id}; "
            "fix the task state or artifact before running more shell commands"
        )

    if retries > effective_max_retries:
        return (
            f"task {task_id} exceeded retry budget at step {current_step_id}: "
            f"retries={retries}, effective_max={effective_max_retries}"
        )
    return None


def evaluate_shell_command(command: str) -> str | None:
    retry_error = check_retry_budget()
    if retry_error:
        return retry_error

    blocked_pattern = check_blocked_patterns(command)
    if blocked_pattern:
        return blocked_pattern

    return None
