@echo off

set ROOT=%~dp0
set PREMAKE=%ROOT%vendor\Windows\Premake-5.0.0-beta7\premake5.exe
set VCPKG=%ROOT%vendor\vcpkg

REM Bootstrap vcpkg if it doesn't exist.
if not exist "%VCPKG%\vcpkg.exe" (
	echo [1/3] Bootstrapping vcpkg...
	cd /d "%VCPKG%"
	call bootstrap-vcpkg.bat
	if errorlevel 1 (
		echo Failed to bootstrap vcpkg!
		pause
		exit /b 1
	)
	echo Vcpkg bootstrapped successfully!
) else (
	echo [1/3] Vcpkg already bootstrapped.
)

REM Install dependencies from vcpkg.json.
echo [2/3] Installing dependencies...
cd /d "%ROOT%"
"%VCPKG%\vcpkg.exe" install --triplet=x64-windows-static
if errorlevel 1 (
	echo Failed to install dependencies!
	pause
	exit /b 1
)
echo Dependencies installed successfully!

REM Generate Visual Studio 2022 project files.
echo [3/3] Generating Visual Studio 2022 solution...
"%PREMAKE%" --file=AfterglowWorkspace.lua vs2022
if errorlevel 1 (
	echo Failed to generate project files!
	pause
	exit /b 1
)

echo =================================
echo Completed, you're good to go :)
echo =================================

pause