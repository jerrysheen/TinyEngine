@echo off
setlocal enabledelayedexpansion

:: 设置VS环境
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" >nul 2>&1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" >nul 2>&1
)

:: 保存原始目录
set "ORIGINAL_DIR=%CD%"

cd "Projects\Windows\Visual Studio 2022"

:: 检查可执行文件是否存在
set "EXE_PATH=bin\Debug-windows-x86_64\EngineCore\EngineCore.exe"
set "EXE_DIR=bin\Debug-windows-x86_64\EngineCore"

if exist "%EXE_PATH%" (
    echo Executable found. Checking if rebuild is needed...
    
    :: 快速检查（只在必要时编译）
    msbuild TinyEngine.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo
    
    if !ERRORLEVEL! == 0 (
        echo Starting Editor...
        
        :: 关键修改：切换到 exe 所在目录再运行
        cd "%EXE_DIR%"
        "EngineCore.exe"
        
        :: 返回原始目录
        cd "%ORIGINAL_DIR%"
    ) else (
        echo Build failed 1!
        pause
    )
) else (
    echo Executable not found. Full build required...
    msbuild TinyEngine.sln /p:Configuration=Debug /p:Platform=x64
    
    if !ERRORLEVEL! == 0 (
        echo Build successful! Starting TinyEngine...
        
        :: 关键修改：切换到 exe 所在目录再运行
        cd "%EXE_DIR%"
        "EngineCore.exe"
        
        :: 返回原始目录
        cd "%ORIGINAL_DIR%"
    ) else (
        echo Build failed 2!
        pause
    )
)

pause