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

set "COMMON_COMPILE_FLAGS=/nologo /std:c++14 /permissive- /Zi /W4 /WX /wd4505 /GR- /EHa- /FC"
set "INCLUDES=/I"%~dp0Src" /I"%~dp0External""
set "COMMON_LINKER_FLAGS=/link /nologo /INCREMENTAL:NO /DEBUG"
set "COMMON_MACROS=/DAG_RHI_OPENGL=1"
set "COMMON_LIBS=Kernel32.lib User32.lib Gdi32.lib Opengl32.lib"

if "%BUILD%"=="debug" (
	echo [Afterglow] Compiling and linking [debug]...
	cl %COMMON_COMPILE_FLAGS% /MTd /Od %COMMON_MACROS% /DAG_DEBUG=1 ^
	%INCLUDES% ^
	"%~dp0Src\Platform\Windows\Win32Main.cpp" ^
	/Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
	%COMMON_LINKER_FLAGS% %COMMON_LIBS%
	if !errorlevel! neq 0 goto error
) else (
	echo [Afterglow] Compiling and linking [release]...
	cl %COMMON_COMPILE_FLAGS% /MT /O2 %COMMON_MACROS% /DAG_DEBUG=0 ^
	%INCLUDES% ^
	"%~dp0Src\Platform\Windows\Win32Main.cpp" ^
	/Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
	%COMMON_LINKER_FLAGS% %COMMON_LIBS%
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