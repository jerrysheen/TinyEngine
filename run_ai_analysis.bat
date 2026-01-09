@echo off
cd /d "%~dp0"

echo [1/3] Setting up _ai directory...
if not exist "_ai" mkdir "_ai"

echo [2/3] Generating Architecture Digests (Python)...
python _ai\ai_digest.py
if errorlevel 1 (
    echo Error running Python script. Make sure python is installed.
    pause
    exit /b
)

echo [3/3] Preparing Context for AI...

REM 生成一个“聚合文件”或者提示词模板
set PROMPT_FILE=_ai\PROMPT_ARCH_REVIEW.txt
(
echo I am providing you with the "Architecture Digests" of my current C++ Renderer.
echo These files contain the key structs, classes, and flow logic for each subsystem.
echo.
echo Please review the following contexts:
echo [1] @_ai/DIGEST_RENDER_FLOW.md (How we execute a frame)
echo [2] @_ai/DIGEST_MATERIAL.md (How we handle shaders/data)
echo [3] @_ai/DIGEST_RESOURCE.md (How we handle memory/barriers)
echo [4] @_ai/DIGEST_RHI.md (Low level abstraction)
echo.
echo Based strictly on this evidence:
echo 1. Describe my current architecture style (e.g., Immediate mode vs FrameGraph, Bindless vs Slot-based).
echo 2. Identify the top 3 bottlenecks that prevent this from being a "Modern Renderer".
echo 3. Propose a specific refactoring roadmap for the Material System to support [Insert Goal, e.g., GPU-Driven].
) > "%PROMPT_FILE%"

type "%PROMPT_FILE%" | clip

echo.
echo ========================================================
echo DONE!
echo.
echo 1. Digests are in: root/_ai/
echo 2. A prompt has been COPIED to your clipboard.
echo.
echo Just open Cursor/Chat and PASTE (Ctrl+V).
echo ========================================================
pause