# PreToolUse hook wrapper for TinyEngine harness
# Windows PowerShell script that calls Python
# Hooks location: .clinerules/hooks/

$ErrorActionPreference = "Continue"

# Get input from stdin
$InputString = [Console]::In.ReadToEnd()

# Call Python harness script
$pythonScript = Join-Path $PSScriptRoot "..\..\_ai\scripts\harness\cline_pre_tool_guard.py"
$result = $InputString | & python $pythonScript

# Hooks must emit a single JSON object on stdout.
if ([string]::IsNullOrWhiteSpace(($result -join ""))) {
    Write-Output '{"cancel":false,"contextModification":"","errorMessage":""}'
} else {
    Write-Output ($result -join "")
}
