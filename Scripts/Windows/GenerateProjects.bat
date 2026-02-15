@echo off
setlocal enabledelayedexpansion

REM Check if premake5 is available
where premake5 >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] premake5 not found in PATH
    echo Install premake5 and add it to your PATH
    echo See: https://github.com/premake/premake-core
    echo.
    pause
    exit /b 1
)

echo [OK] premake5 found
premake5 --version
echo.

REM Check if vcpkg is available
where vcpkg >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] vcpkg not found in PATH
    echo Install vcpkg and add it to your PATH
    echo See: https://github.com/microsoft/vcpkg
    echo.
    pause
    exit /b 1
)

echo [OK] vcpkg found
vcpkg --version | find "vcpkg package"
echo.

REM Navigate to root directory (2 levels up from Scripts/Windows)
pushd %~dp0\..\..

REM Install dependencies using vcpkg manifest mode
echo [INFO] Installing dependencies from vcpkg.json...
echo.

vcpkg install --triplet=x64-windows

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] vcpkg dependency installation failed
    popd
    pause
    exit /b 1
)

echo.
echo [OK] All dependencies installed
echo.

echo Select Visual Studio version:
echo 1. Visual Studio 2026 (default)
echo 2. Visual Studio 2022
echo.
set /p choice="Enter choice (1-2): "

if "%choice%"=="" set choice=1
if "%choice%"=="2" (
    set vs_version=vs2022
) else (
    set choice=1
    set vs_version=vs2026
)

call premake5 %vs_version%

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] premake5 generation failed
    popd
    pause
    exit /b 1
)

popd
pause