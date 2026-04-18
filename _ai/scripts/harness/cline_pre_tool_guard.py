#!/usr/bin/env python3
"""Cline PreToolUse hook adapter for TinyEngine harness.

Cline sends JSON with this structure:
{
    "taskId": "abc123",
    "hookName": "PreToolUse",
    "preToolUse": {
        "tool": "Bash",
        "parameters": { "command": "..." }
    }
}

This adapter normalizes it to the format expected by pre_tool_guard.py.
"""
from __future__ import annotations

import json
import re
import sys

from task_runtime import (
    DEFAULT_MAX_STEP_RETRIES,
    HARD_MAX_STEP_RETRIES,
    find_step,
    get_active_task_id,
    load_progress,
    load_task,
)


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


def load_payload() -> dict | None:
    try:
        return json.load(sys.stdin)
    except json.JSONDecodeError:
        return None


def is_bash_tool(payload: dict) -> bool:
    pre_tool = payload.get("preToolUse", {})
    tool_name = pre_tool.get("tool", "")
    return tool_name == "Bash"


def extract_command(payload: dict) -> str:
    pre_tool = payload.get("preToolUse", {})
    params = pre_tool.get("parameters", {})
    return params.get("command", "")


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
            "fix the task state or artifact before running more Bash commands"
        )

    if retries > effective_max_retries:
        return (
            f"task {task_id} exceeded retry budget at step {current_step_id}: "
            f"retries={retries}, effective_max={effective_max_retries}"
        )

    return None


def main() -> int:
    payload = load_payload()
    if not payload or not is_bash_tool(payload):
        print('{"cancel":false,"contextModification":"","errorMessage":""}')
        return 0

    retry_error = check_retry_budget()
    if retry_error:
        print(f"BLOCKED by TinyEngine harness: {retry_error}", file=sys.stderr)
        print('{"cancel":true,"contextModification":"","errorMessage":"Retry budget exceeded or task blocked"}')
        return 0

    command = extract_command(payload)
    blocked_pattern = check_blocked_patterns(command)
    if blocked_pattern:
        print(f"BLOCKED by TinyEngine harness: {blocked_pattern}", file=sys.stderr)
        print('{"cancel":true,"contextModification":"","errorMessage":"Dangerous command pattern blocked"}')
        return 0

    print('{"cancel":false,"contextModification":"","errorMessage":""}')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())