@echo off
setlocal enabledelayedexpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [Afterglow Tools] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    exit /b 1
)

set BUILD_DIR=%~dp0..\Build\Tools
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

echo [Afterglow Tools] Compiling and linking cooker [debug]...
cl /nologo /std:c++17 /permissive- /MTd /Zi /I "%~dp0Cooker" /I "%~dp0..\Src" /I "%~dp0..\External\SSTL\Include" "%~dp0Cooker\Src\AgCooker.cpp" /Fd"AgCooker.pdb" /Fe"AgCooker.exe" /link /nologo /DEBUG d3dcompiler.lib
if %errorlevel% neq 0 goto error
echo.
echo [Afterglow Tools] Build succeeded.
popd
goto end

:error
echo.
echo [Afterglow Tools] Build failed.
popd
exit /b 1

:end
endlocal
