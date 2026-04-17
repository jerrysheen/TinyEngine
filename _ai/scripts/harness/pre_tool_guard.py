#!/usr/bin/env python3
import json
import re
import sys


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


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except json.JSONDecodeError:
        return 0

    tool_name = payload.get("tool_name") or payload.get("toolName") or ""
    if tool_name != "Bash":
        return 0

    tool_input = payload.get("tool_input") or payload.get("parameters") or {}
    command = tool_input.get("command", "")

    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command, flags=re.IGNORECASE):
            print(f"BLOCKED by TinyEngine harness: {pattern}", file=sys.stderr)
            return 2

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
