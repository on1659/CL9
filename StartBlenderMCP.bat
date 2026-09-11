@echo off
setlocal
set "CL9_BLENDER=C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"
if not exist "%CL9_BLENDER%" (
    echo Blender not found: %CL9_BLENDER%
    pause
    exit /b 1
)
rem Do not launch a second server when the local MCP port is already listening.
powershell -NoProfile -Command "if (Get-NetTCPConnection -LocalPort 9876 -State Listen -ErrorAction SilentlyContinue) { exit 10 }"
if "%ERRORLEVEL%"=="10" (
    echo Port 9876 is already listening. No new Blender instance was started.
    pause
    exit /b 0
)
rem Optional: drag a .blend file onto this BAT to open it before starting MCP.
if "%~1"=="" (
    start "CL9 Blender MCP" "%CL9_BLENDER%" --python "%~dp0Tools\Blender\start_mcp.py"
) else (
    start "CL9 Blender MCP" "%CL9_BLENDER%" "%~f1" --python "%~dp0Tools\Blender\start_mcp.py"
)
