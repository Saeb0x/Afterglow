@echo off
setlocal enabledelayedexpansion

call vcvarsall.bat x64 >nul 2>&1
if !errorlevel! neq 0 (
    echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    endlocal
    exit /b 1
)

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if "!BUILD!"=="release" set BUILD_DIR=%~dp0Build\Release

if not exist "!BUILD_DIR!" mkdir "!BUILD_DIR!"
pushd "!BUILD_DIR!"

if "!BUILD!"=="debug" (
    echo [Afterglow] Compiling and linking game [debug]...
    cl /nologo /std:c++17 /permissive- /MTd /Od /Zi ^
    /I "%~dp0Src" /I "%~dp0External\SSTL\Include" ^
    "%~dp0Src\Engine\Platform\Windows\Win32Main.cpp" ^
    "%~dp0Src\Engine\Platform\Windows\Window\Win32Window.cpp" ^
    "%~dp0Src\Engine\Platform\Windows\Input\Win32Input.cpp" ^
    /Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
    /link /nologo /DEBUG kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
) else (
    echo [Afterglow] Compiling and linking game [release]...
    cl /nologo /std:c++17 /permissive- /MT /O2 ^
    /I "%~dp0Src" /I "%~dp0External\SSTL\Include" ^
    "%~dp0Src\Engine\Platform\Windows\Win32Main.cpp" ^
    "%~dp0Src\Engine\Platform\Windows\Window\Win32Window.cpp" ^
    "%~dp0Src\Engine\Platform\Windows\Input\Win32Input.cpp" ^
    /Fe"Afterglow.exe" ^
    /link /nologo kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
)

echo.
echo [Afterglow] Build succeeded.
popd

call "%~dp0Cook.bat"
if !errorlevel! neq 0 (
    endlocal
    exit /b 1
)

endlocal
exit /b 0

:error
echo.
echo [Afterglow] Build failed.
popd
endlocal
exit /b 1
