@echo off
setlocal enabledelayedexpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
	echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
	endlocal
	exit /b 1
)

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if "%BUILD%"=="release" set BUILD_DIR=%~dp0Build\Release

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

if "%BUILD%"=="debug" (
	echo [Afterglow] Compiling and linking [debug]...
	cl /nologo /std:c++20 /permissive- /MTd /Od /Zi /DAG_DEBUG=1 /W4 /WX /wd4100 /wd4505 /GR- /EHa- /FC ^
	/I "%~dp0Src" ^
	"%~dp0Src\Platform\Windows\Win32Main.cpp" ^
	/Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
	/link /nologo /INCREMENTAL:NO /DEBUG Kernel32.lib User32.lib Gdi32.lib
	if !errorlevel! neq 0 goto error
) else (
	echo [Afterglow] Compiling and linking [release]...
	cl /nologo /std:c++20 /permissive- /MT /O2 /Zi /DAG_DEBUG=0 /W4 /WX /wd4100 /wd4505 /GR- /EHa- /FC ^
	/I "%~dp0Src" ^
	"%~dp0Src\Platform\Windows\Win32Main.cpp" ^
	/Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
	/link /nologo /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF Kernel32.lib User32.lib Gdi32.lib
	if !errorlevel! neq 0 goto error
)

echo.
echo [Afterglow] Build succeeded.
popd
endlocal
exit /b 0

:error
echo.
echo [Afterglow] Build failed.
popd
endlocal
exit /b 1