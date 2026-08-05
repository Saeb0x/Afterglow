@echo off
setlocal enabledelayedexpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    exit /b 1
)

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if /i "%BUILD%"=="release" set BUILD_DIR=%~dp0Build\Release

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

if /i "%BUILD%"=="debug" (
    echo [Afterglow] Compiling and linking game [debug]...
    cl /nologo /std:c++17 /permissive- /DAG_DEBUG /DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00 /MTd /Zi /I "%~dp0Src" /I "%~dp0External\SSTL\Include" "%~dp0Src\Platform\Windows\Win32Main.cpp" /Fd"Afterglow.pdb" /Fe"Afterglow.exe" /link /nologo /DEBUG kernel32.lib user32.lib d3d11.lib dxgi.lib
    if !errorlevel! neq 0 goto error
) else (
    echo [Afterglow] Compiling and linking game [release]...
    cl /nologo /std:c++17 /permissive- /DAG_RELEASE /DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00 /MT /O2 /I "%~dp0Src" /I "%~dp0External\SSTL\Include" "%~dp0Src\Platform\Windows\Win32Main.cpp" /Fe"Afterglow.exe" /link /nologo kernel32.lib user32.lib d3d11.lib dxgi.lib
    if !errorlevel! neq 0 goto error
)

echo.
echo [Afterglow] Build succeeded.
popd

call "%~dp0Cook.bat"
if %errorlevel% neq 0 (
    exit /b 1
)

goto end

:error
echo.
echo [Afterglow] Build failed.
popd
exit /b 1

:end
endlocal
