#!/usr/bin/env python3
from __future__ import annotations

from payload_adapter import emit_cursor_permission, extract_shell_command, load_payload
from shell_guard_lib import evaluate_shell_command


def main() -> int:
    payload = load_payload() or {}
    reason = evaluate_shell_command(extract_shell_command(payload))
    if reason:
        emit_cursor_permission(
            "deny",
            "Blocked by TinyEngine harness.",
            f"TinyEngine harness blocked shell execution: {reason}",
        )
    else:
        emit_cursor_permission("allow")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
