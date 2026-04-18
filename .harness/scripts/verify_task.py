#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from task_runtime import (
    DEFAULT_MAX_STEP_RETRIES,
    HARD_MAX_STEP_RETRIES,
    ROOT_DIR,
    evidence_dir,
    find_step,
    load_progress,
    load_task,
    next_step_id,
    save_progress,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify current task step and advance progress")
    parser.add_argument("--task-id", required=True)
    return parser.parse_args()


def log_path(task_id: str, current_step: str) -> Path:
    return evidence_dir(task_id) / f"{current_step}.verify.log"


def write_log(task_id: str, current_step: str, lines: list[str]) -> None:
    path = log_path(task_id, current_step)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def find_vsdevcmd() -> str | None:
    candidates = [
        r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
        r"C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat",
        r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat",
        r"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
    ]
    for candidate in candidates:
        if Path(candidate).exists():
            return candidate
    return None


def verify_files_exist(rule: dict) -> tuple[bool, list[str]]:
    missing = []
    for item in rule.get("files", []):
        if not (ROOT_DIR / item).exists():
            missing.append(item)
    if missing:
        return False, [f"Missing: {item}" for item in missing]
    return True, [f"Exists: {item}" for item in rule.get("files", [])]


def verify_changed_files(rule: dict, progress: dict) -> tuple[bool, list[str]]:
    """Pass only when required files were recorded and still exist on disk.

    changed_files is useful to prove that the current task step touched a file,
    but by itself it is not enough to prove that the artifact still exists.
    This combined check avoids the common false-positive where a path remains in
    progress.json even though the file was deleted, replaced incorrectly, or was
    only injected manually into changed_files.
    """
    changed = set(progress.get("changed_files", []))
    required = {item.replace("/", "\\") for item in rule.get("files", [])}

    not_recorded = sorted(required - changed)
    missing_on_disk = sorted(item for item in required if not (ROOT_DIR / item).exists())

    lines: list[str] = []
    if not_recorded:
        lines.extend(f"Not changed yet: {item}" for item in not_recorded)
    if missing_on_disk:
        lines.extend(f"Missing on disk: {item}" for item in missing_on_disk)

    if lines:
        return False, lines

    return True, [f"Changed and exists: {item}" for item in sorted(required)]


def verify_msbuild(task_id: str, step_id: str, rule: dict) -> tuple[bool, list[str]]:
    solution = ROOT_DIR / rule.get("solution", r"Projects\Windows\Visual Studio 2022\TinyEngine.sln")
    generate_projects = bool(rule.get("generate_projects", True))

    if generate_projects and not solution.exists():
        premake = ROOT_DIR / "Tools" / "premake" / "premake5.exe"
        if not premake.exists():
            return False, [f"premake not found: {premake}"]
        result = subprocess.run([str(premake), "vs2022"], cwd=ROOT_DIR, capture_output=True, text=True)
        if result.returncode != 0:
            return False, ["premake failed", result.stdout, result.stderr]

    vsdevcmd = find_vsdevcmd()
    if not vsdevcmd:
        return False, ["VsDevCmd.bat not found"]

    configuration = rule.get("configuration", "Debug")
    platform = rule.get("platform", "x64")
    build_log = evidence_dir(task_id) / "build.log"
    build_log.parent.mkdir(parents=True, exist_ok=True)
    command = (
        f'call "{vsdevcmd}" >nul 2>&1 && '
        f'msbuild "{solution}" /p:Configuration={configuration} /p:Platform={platform} /v:minimal /nologo'
    )
    result = subprocess.run(["cmd.exe", "/c", command], cwd=ROOT_DIR, capture_output=True, text=True)
    build_log.write_text((result.stdout or "") + (result.stderr or ""), encoding="utf-8")
    if result.returncode != 0:
        return False, [f"msbuild failed for step {step_id}", f"See: {build_log}"]
    return True, [f"msbuild passed: {solution}", f"See: {build_log}"]


def verify_manual_check(rule: dict) -> tuple[bool, list[str]]:
    message = rule.get("message", "Manual verification required")
    return False, [message]


def verify_step(task_id: str, step: dict, progress: dict) -> tuple[bool, list[str]]:
    rule = step.get("acceptance", {})
    rule_type = rule.get("type")

    if rule_type == "files_exist":
        return verify_files_exist(rule)
    if rule_type == "changed_files":
        return verify_changed_files(rule, progress)
    if rule_type == "msbuild":
        return verify_msbuild(task_id, step["id"], rule)
    if rule_type == "manual_check":
        return verify_manual_check(rule)

    return False, [f"Unsupported acceptance type: {rule_type}"]


def main() -> int:
    args = parse_args()
    task = load_task(args.task_id)
    progress = load_progress(task)
    current_step_id = progress.get("current_step")

    if not current_step_id:
        print("Task already completed")
        return 0

    step = find_step(task, current_step_id)
    configured_max_retries = int(step.get("max_retries", task.get("max_step_retries", DEFAULT_MAX_STEP_RETRIES)))
    max_retries = min(configured_max_retries, HARD_MAX_STEP_RETRIES)
    step_progress = progress["steps"][current_step_id]

    if step_progress.get("status") == "blocked":
        print(f"BLOCKED {current_step_id}", file=sys.stderr)
        print("Current step is blocked. Create the required artifact or edit task/progress before retrying.", file=sys.stderr)
        if step_progress.get("last_error"):
            print(step_progress["last_error"], file=sys.stderr)
        return 3

    if step_progress.get("status") == "failed":
        print(
            f"Harness note: previous verify for {current_step_id} failed. "
            "Check progress.json last_error and the current step verify log before retrying.",
            file=sys.stderr,
        )

    passed, lines = verify_step(args.task_id, step, progress)
    write_log(args.task_id, current_step_id, lines)

    if passed:
        step_progress["status"] = "done"
        step_progress["last_error"] = ""
        step_progress["verified_at"] = progress["updated_at"]
        next_id = next_step_id(task, current_step_id)
        if next_id is None:
            progress["current_step"] = None
            progress["status"] = "done"
        else:
            progress["current_step"] = next_id
            progress["status"] = "in_progress"
            progress["steps"][next_id]["status"] = "in_progress"
        save_progress(progress)
        print(f"PASS {current_step_id}")
        return 0

    next_retries = int(step_progress.get("retries", 0)) + 1
    step_progress["retries"] = next_retries
    step_progress["last_error"] = " | ".join(lines)
    if next_retries > max_retries:
        step_progress["status"] = "blocked"
        progress["status"] = "blocked"
        lines.append(
            f"Retry limit reached: configured={configured_max_retries}, effective={max_retries}, hard_cap={HARD_MAX_STEP_RETRIES}"
        )
    else:
        step_progress["status"] = "failed"
        progress["status"] = "failed"
    save_progress(progress)
    print(f"FAIL {current_step_id}", file=sys.stderr)
    for line in lines:
        print(line, file=sys.stderr)
    print(
        "Next action: inspect progress.json steps.<current_step>.last_error and "
        f"{log_path(args.task_id, current_step_id)} before retrying.",
        file=sys.stderr,
    )
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
