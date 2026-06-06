@echo off
setlocal

call vcvarsall.bat x64 >nul 2>&1

set MODE=debug
if /i "%1"=="release" set MODE=release

set OUT_DIR=Build\Debug
if /i "%MODE%"=="release" set OUT_DIR=Build\Release

set FLAGS=/nologo /W4 /WX /FS /Zi /Fd:%OUT_DIR%\ /Od /DAG_DEBUG
if /i "%MODE%"=="release" set FLAGS=/nologo /W4 /WX /FS /O2 /Ox /Oi /Oy /DAG_RELEASE

if not exist %OUT_DIR% mkdir %OUT_DIR%

set INCLUDES=-I "%cd%\Engine\Include"
set LIBS=kernel32.lib user32.lib gdi32.lib

echo [Afterglow] Building engine [%MODE%]...
cl %FLAGS% %INCLUDES% /c Engine/Src/*.c /Fo:%OUT_DIR%\
if %errorlevel% neq 0 goto error

lib /nologo /OUT:%OUT_DIR%\Afterglow.lib %OUT_DIR%\*.obj
if %errorlevel% neq 0 goto error

echo.
echo [Afterglow] Building game [%MODE%]...
cl %FLAGS% %INCLUDES% /c Game/Src/*.c /Fo:%OUT_DIR%\
if %errorlevel% neq 0 goto error

link /nologo %OUT_DIR%\*.obj %OUT_DIR%\Afterglow.lib %LIBS% /OUT:%OUT_DIR%\Game.exe /PDB:%OUT_DIR%\Game.pdb /SUBSYSTEM:CONSOLE
if %errorlevel% neq 0 goto error

echo.
echo [Afterglow] Build succeeded.
pause
goto end

:error
echo.
echo [Afterglow] Build failed.
pause
exit /b 1

:end
endlocal