@echo off
setlocal

set MODE=debug
if /i "%1"=="release" set MODE=release

if /i "%MODE%"=="debug" (
    set EXE=%~dp0Build\Debug\Game.exe
) else (
    set EXE=%~dp0Build\Release\Game.exe
)

if not exist "%EXE%" (
    echo [Afterglow] "%EXE%" not found. Run Build.bat %MODE% first.
    exit /b 1
)

echo [Afterglow] Running...
"%EXE%"

endlocal
