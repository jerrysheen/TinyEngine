# TaskComplete hook wrapper for TinyEngine harness
# Windows PowerShell script that calls Python
# Hooks location: .clinerules/hooks/

$ErrorActionPreference = "Continue"

# Get input from stdin
$InputString = [Console]::In.ReadToEnd()

# Call Python harness script
$pythonScript = Join-Path $PSScriptRoot "..\..\_ai\scripts\harness\ensure_task_state.py"
$result = $InputString | & python $pythonScript

# Hooks must emit a single JSON object on stdout.
if ([string]::IsNullOrWhiteSpace(($result -join ""))) {
    Write-Output '{"cancel":false,"contextModification":"","errorMessage":""}'
} else {
    Write-Output ($result -join "")
}
