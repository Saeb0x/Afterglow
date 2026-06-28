@echo off
setlocal EnableDelayedExpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [Afterglow Tools] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    exit /b 1
)

set OUT_DIR=%~dp0..\Build\Tools
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
pushd "%OUT_DIR%"

echo [Afterglow Tools] Compiling and linking cooker...
cl /nologo /Zi /I "%~dp0Cooker" /I "%~dp0..\Src" "%~dp0Cooker\Src\AgCooker.cpp" /Fd"AgCooker.pdb" /Fe"AgCooker.exe"
if !errorlevel! neq 0 goto error

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
