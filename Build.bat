@echo off
setlocal EnableDelayedExpansion

call vcvarsall.bat x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    exit /b 1
)

set MODE=debug
if /i "%1"=="release" set MODE=release

set OUT_DIR=%~dp0Build\Debug
if /i "%MODE%"=="release" set OUT_DIR=%~dp0Build\Release

if not exist "%OUT_DIR%\Obj\Engine" mkdir "%OUT_DIR%\Obj\Engine"
if not exist "%OUT_DIR%\Obj\Game" mkdir "%OUT_DIR%\Obj\Game"

pushd "%OUT_DIR%"

if /i "%MODE%"=="debug" (
    echo [Afterglow] Compiling engine [debug]...
    cl /c /nologo /Zi /I "%~dp0Engine" "%~dp0Engine\Src\*.cpp" /Fo"Obj\Engine\\" /Fd"Obj\Engine\Afterglow.pdb"
    if !errorlevel! neq 0 goto error

    echo.
    echo [Afterglow] Archiving engine...
    lib /nologo /OUT:Afterglow.lib "Obj\Engine\*.obj"
    if !errorlevel! neq 0 goto error

    echo.
    echo [Afterglow] Compiling and linking game [debug]...
    cl /nologo /Zi /I "%~dp0Engine\Include" "%~dp0Game\Src\*.cpp" /Fo"Obj\Game\\" /Fd"Game.pdb" /Fe"Game.exe" /link /nologo /DEBUG Afterglow.lib kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
) else (
    echo [Afterglow] Compiling engine [release]...
    cl /c /nologo /O2 /I "%~dp0Engine" "%~dp0Engine\Src\*.cpp" /Fo"Obj\Engine\\"
    if !errorlevel! neq 0 goto error

    echo.
    echo [Afterglow] Archiving engine...
    lib /nologo /OUT:Afterglow.lib "Obj\Engine\*.obj"
    if !errorlevel! neq 0 goto error

    echo.
    echo [Afterglow] Compiling and linking game [release]...
    cl /nologo /O2 /I "%~dp0Engine\Include" "%~dp0Game\Src\*.cpp" /Fo"Obj\Game\\" /Fe"Game.exe" /link /nologo Afterglow.lib kernel32.lib user32.lib gdi32.lib
    if !errorlevel! neq 0 goto error
)

echo.
echo [Afterglow] Build succeeded.
popd
goto end

:error
echo.
echo [Afterglow] Build failed.
popd
exit /b 1

:end
endlocal
