@echo off
setlocal EnableDelayedExpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    exit /b 1
)

set MODE=debug
if /i "%1"=="release" set MODE=release

set OUT_DIR=%~dp0Build\Debug
if /i "%MODE%"=="release" set OUT_DIR=%~dp0Build\Release

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
pushd "%OUT_DIR%"

if /i "%MODE%"=="debug" (
    echo [Afterglow] Compiling and linking game [debug]...
    cl /nologo /Zi /I "%~dp0Src" "%~dp0Src\Platform\Windows\Win32Afterglow.cpp" /Fd"Afterglow.pdb" /Fe"Afterglow.exe" /link /nologo /DEBUG kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
) else (
    echo [Afterglow] Compiling and linking game [release]...
    cl /nologo /O2 /I "%~dp0Src" "%~dp0Src\Platform\Windows\Win32Afterglow.cpp" /Fe"Afterglow.exe" /link /nologo kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
)

echo.
echo [Afterglow] Build succeeded.
popd
goto end

:error
echo.
echo [Afterglow] Build failed.
popd
exit /b 1

:end
endlocal
