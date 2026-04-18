#!/usr/bin/env python3
from __future__ import annotations

import sys

from payload_adapter import extract_shell_command, is_claude_bash_tool, load_payload
from shell_guard_lib import evaluate_shell_command


def main() -> int:
    payload = load_payload()
    if not payload or not is_claude_bash_tool(payload):
        return 0

    reason = evaluate_shell_command(extract_shell_command(payload))
    if reason:
        print(f"BLOCKED by TinyEngine harness: {reason}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
