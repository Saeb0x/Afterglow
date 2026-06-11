@echo off
setlocal

call vcvarsall.bat x64 >nul 2>&1

set MODE=debug
if /i "%1"=="release" set MODE=release

set OUT_DIR=Build\Debug
if /i "%MODE%"=="release" set OUT_DIR=Build\Release

set FLAGS=/nologo /W4 /FS /Zi /Od /DAG_DEBUG
if /i "%MODE%"=="release" set FLAGS=/nologo /W4 /FS /O2 /Ox /Oi /Oy /DAG_RELEASE

set LINK_FLAGS=/nologo /SUBSYSTEM:WINDOWS
if /i "%MODE%"=="debug" set LINK_FLAGS=%LINK_FLAGS% /DEBUG

if not exist %OUT_DIR% mkdir %OUT_DIR%

set INCLUDES=-I "%~dp0Engine\Include"
set LIBS=kernel32.lib user32.lib gdi32.lib
set ROOT=%~dp0

echo [Afterglow] Building engine [%MODE%]...
pushd %OUT_DIR%
cl %FLAGS% %INCLUDES% /c %ROOT%Engine\Src\*.c
if %errorlevel% neq 0 goto error

lib /nologo /OUT:Afterglow.lib *.obj
if %errorlevel% neq 0 goto error
del *.obj

echo.
echo [Afterglow] Building game [%MODE%]...
cl %FLAGS% %INCLUDES% /c %ROOT%Game\Src\*.c
if %errorlevel% neq 0 goto error

echo [Afterglow] Linking...
link %LINK_FLAGS% *.obj Afterglow.lib %LIBS% /OUT:Game.exe /PDB:Game.pdb
if %errorlevel% neq 0 goto error

popd
echo.
echo [Afterglow] Build succeeded.
goto end

:error
popd
echo.
echo [Afterglow] Build failed.
exit /b 1

:end
endlocal
