#!/usr/bin/env python3
from __future__ import annotations

import json
import sys


def load_payload() -> dict | None:
    try:
        return json.load(sys.stdin)
    except json.JSONDecodeError:
        return None


def is_claude_bash_tool(payload: dict) -> bool:
    tool_name = payload.get("tool_name") or payload.get("toolName") or ""
    return tool_name == "Bash"


def extract_tool_input(payload: dict) -> dict:
    return payload.get("tool_input") or payload.get("parameters") or {}


def extract_shell_command(payload: dict) -> str:
    tool_input = extract_tool_input(payload)
    if tool_input.get("command"):
        return tool_input["command"]
    if payload.get("command"):
        return payload.get("command", "")
    return ""


def extract_file_paths(payload: dict) -> list[str]:
    tool_input = extract_tool_input(payload)
    raw_paths: list[str] = []

    for container in (tool_input, payload):
        for key in ("file_path", "path", "relative_workspace_path"):
            value = container.get(key)
            if value:
                raw_paths.append(value)

        for key in ("file_paths", "paths"):
            values = container.get(key) or []
            for value in values:
                if value:
                    raw_paths.append(value)

    for edit in payload.get("edits", []) or []:
        value = edit.get("file_path") or edit.get("path")
        if value:
            raw_paths.append(value)

    deduped: list[str] = []
    seen: set[str] = set()
    for item in raw_paths:
        if item in seen:
            continue
        seen.add(item)
        deduped.append(item)
    return deduped


def emit_cursor_permission(permission: str, user_message: str = "", agent_message: str = "") -> None:
    response: dict[str, object] = {
        "continue": True,
        "permission": permission,
    }
    if user_message:
        response["user_message"] = user_message
    if agent_message:
        response["agent_message"] = agent_message
    print(json.dumps(response, ensure_ascii=False))
