@echo off
rem ## CL9 project file generation - engine 5.8 pinned.
rem ## HKLM registry only lists 5.4/5.6, so UnrealVersionSelector cannot resolve
rem ## the .uproject "EngineAssociation": "5.8". Engine path is pinned here instead.
rem ## If the engine moves, edit the ENGINE line below.
rem ## (inherited from D:\Work\unreal\CCL8\GenerateProjectFiles.bat)

setlocal

set "ENGINE=D:\Program Files\Epic Games\UE_5.8"
set "PROJECT=D:\Work\unreal\CL9\CL9.uproject"

if not exist "%ENGINE%\Engine\Build\BatchFiles\Build.bat" (
    echo ERROR: engine not found: %ENGINE%
    echo        Fix the ENGINE variable at the top of this file.
    set ExitCode=1
    goto Done
)

call "%ENGINE%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -engine -progress
set ExitCode=%ERRORLEVEL%

:Done
rem ## Scripts/CI: pass any argument to skip the pause:  GenerateProjectFiles.bat -nopause
if not "%~1"=="" goto SkipPause
pause
:SkipPause
exit /B %ExitCode%
