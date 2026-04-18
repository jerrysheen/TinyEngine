#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from task_runtime import ROOT_DIR, get_active_task_id, load_progress, load_task


COMMENT_STYLES = {
    ".py": ("# ", ""),
    ".ps1": ("# ", ""),
    ".bat": (":: ", ""),
    ".cmd": (":: ", ""),
    ".sh": ("# ", ""),
    ".md": ("<!-- ", " -->"),
    ".h": ("// ", ""),
    ".hpp": ("// ", ""),
    ".hh": ("// ", ""),
    ".c": ("// ", ""),
    ".cc": ("// ", ""),
    ".cpp": ("// ", ""),
    ".cxx": ("// ", ""),
    ".cs": ("// ", ""),
    ".lua": ("-- ", ""),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Stamp task provenance header into a generated file")
    parser.add_argument("path", help="Path to generated file, relative to repo root or absolute")
    parser.add_argument("--task-id")
    parser.add_argument("--step-id")
    parser.add_argument("--agent", default="TinyEngine harness")
    return parser.parse_args()


def resolve_path(raw_path: str) -> Path:
    candidate = Path(raw_path)
    if not candidate.is_absolute():
        candidate = ROOT_DIR / candidate
    return candidate.resolve(strict=False)


def build_header(path: Path, task_id: str, step_id: str | None, agent: str) -> str:
    prefix, suffix = COMMENT_STYLES[path.suffix.lower()]
    parts = [f"Created by {agent}", f"task {task_id}"]
    if step_id:
        parts.append(f"step {step_id}")
    line = ", ".join(parts)
    return f"{prefix}{line}{suffix}\n"


def supports_provenance_header(path: Path) -> bool:
    return path.suffix.lower() in COMMENT_STYLES


def ensure_provenance_header(path: Path, task_id: str, step_id: str | None, agent: str) -> bool:
    if not supports_provenance_header(path) or not path.exists():
        return False

    content = path.read_text(encoding="utf-8")
    header = build_header(path, task_id, step_id, agent)
    path.write_text(header + content, encoding="utf-8")
    return True


def main() -> int:
    args = parse_args()
    task_id = args.task_id or get_active_task_id()
    if not task_id:
        raise SystemExit("No task id provided and _ai/active-task.txt is missing")

    task = load_task(task_id)
    progress = load_progress(task)
    step_id = args.step_id if args.step_id is not None else progress.get("current_step")
    path = resolve_path(args.path)

    if not supports_provenance_header(path):
        raise SystemExit(f"Unsupported comment style for suffix: {path.suffix}")
    if not path.exists():
        raise SystemExit(f"File not found: {path}")

    ensure_provenance_header(path, task_id, step_id, args.agent)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
