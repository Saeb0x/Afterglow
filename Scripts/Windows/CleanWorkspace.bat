@echo off
setlocal
cd /d "%~dp0..\.."

:: ANSI escape codes for colors.
for /f %%a in ('echo prompt $E^| cmd /Q') do set "ESC=%%a"
set "GREEN=%ESC%[92m"
set "RED=%ESC%[91m"
set "RESET=%ESC%[0m"

echo Deleting Build/ directory...
rmdir /s /q Build
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% Failed to delete Build/ directory.
    goto :fail
)
echo %GREEN%[OK]%RESET% Build/ directory deleted.
echo.

echo Deleting .slnx files...
del /s /q *.slnx
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% Failed to delete .slnx files.
    goto :fail
)
echo %GREEN%[OK]%RESET% .slnx files deleted.
echo.

echo Deleting .sln files...
del /s /q *.sln
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% Failed to delete .sln files.
    goto :fail
)
echo %GREEN%[OK]%RESET% .sln files deleted.
echo.

echo Deleting .vcxproj, .vcxproj.user, and .vcxproj.filters files...
del /s /q *.vcxproj *.vcxproj.user *.vcxproj.filters
if %ERRORLEVEL% neq 0 (
    echo %RED%[ERROR]%RESET% Failed to delete .vcxproj, .vcxproj.user, and .vcxproj.filters files.
    goto :fail
)
echo %GREEN%[OK]%RESET% .vcxproj, .vcxproj.user, and .vcxproj.filters files deleted.

echo.
echo %GREEN%[DONE]%RESET% Afterglow workspace cleaned successfully.
pause
goto :end

:fail
echo.
echo %RED%[ERROR]%RESET% Afterglow workspace cleaning failed. See above for details.
pause
exit /b 1

:end
endlocal