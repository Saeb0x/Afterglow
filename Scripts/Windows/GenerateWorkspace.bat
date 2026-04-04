@echo off
setlocal
cd /d "%~dp0..\.."

:: ANSI escape codes for colors.
for /f %%a in ('echo prompt $E^| cmd /Q') do set "ESC=%%a"
set "GREEN=%ESC%[92m"
set "RED=%ESC%[91m"
set "RESET=%ESC%[0m"

where vcpkg >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% vcpkg not found in PATH.
    echo    Install vcpkg and ensure it is added to your environment variables.
    echo    https://github.com/microsoft/vcpkg
    goto :fail
)
echo %GREEN%[OK]%RESET% vcpkg found.

where premake5 >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% premake5 not found in PATH.
    echo    Install Premake5 and ensure it is added to your environment variables.
    echo    https://github.com/premake/premake-core/
    goto :fail
)
echo %GREEN%[OK]%RESET% premake5 found.
echo.

echo Installing dependencies via vcpkg...
vcpkg install
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% vcpkg install failed.
    goto :fail
)
echo.

echo Generating Visual Studio 2026 solution...
premake5 vs2026
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% premake5 failed to generate the solution.
    goto :fail
)

echo.
echo %GREEN%[DONE]%RESET% Afterglow workspace generated successfully.
pause
goto :end

:fail
echo.
echo %RED%[ERROR]%RESET% Afterglow workspace generation failed. See above for details.
pause
exit /b 1

:end
endlocal