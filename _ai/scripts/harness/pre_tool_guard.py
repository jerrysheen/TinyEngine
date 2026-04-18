#!/usr/bin/env python3
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


# These command patterns are denied before execution because they are either
# destructive or bypass normal review boundaries.
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
    """Read the hook payload from stdin.

    Claude sends a JSON object to hook scripts. If stdin is empty or malformed,
    the safest fallback here is to no-op instead of crashing the hook layer.
    """
    try:
        return json.load(sys.stdin)
    except json.JSONDecodeError:
        return None


def is_bash_tool(payload: dict) -> bool:
    """PreToolUse only guards Bash invocations in the current harness setup."""
    tool_name = payload.get("tool_name") or payload.get("toolName") or ""
    return tool_name == "Bash"


def extract_command(payload: dict) -> str:
    """Normalize the incoming command field across payload shapes."""
    tool_input = payload.get("tool_input") or payload.get("parameters") or {}
    return tool_input.get("command", "")


def check_blocked_patterns(command: str) -> str | None:
    """Return the matched deny rule if the command hits a blocked pattern."""
    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command, flags=re.IGNORECASE):
            return pattern
    return None


def check_retry_budget() -> str | None:
    """Stop new Bash execution when the current task step has exhausted retries.

    verify_task.py is still the source of truth for mutating retry counters and
    marking states. This guard is a second line of defense: once a step is
    blocked, or its retry counter already exceeds the effective cap, the agent
    should stop issuing more Bash commands for that task.
    """
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
        return 0

    retry_error = check_retry_budget()
    if retry_error:
        print(f"BLOCKED by TinyEngine harness: {retry_error}", file=sys.stderr)
        return 2

    command = extract_command(payload)
    blocked_pattern = check_blocked_patterns(command)
    if blocked_pattern:
        print(f"BLOCKED by TinyEngine harness: {blocked_pattern}", file=sys.stderr)
        return 2

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
