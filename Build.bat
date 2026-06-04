@echo off
setlocal

call vcvarsall.bat x64 >nul 2>&1

set MODE=debug
if /i "%1"=="release" set MODE=release

if /i "%MODE%"=="debug" (
    set OUT_DIR=Build\Debug
) else (
    set OUT_DIR=Build\Release
)

if not exist %OUT_DIR% mkdir %OUT_DIR%

if /i "%MODE%"=="debug" (
    set FLAGS=/nologo /std:c++17 /W4 /WX /GR- /EHa- /Zi /Fd:%OUT_DIR%\ /Od /DAG_DEBUG
) else (
    set FLAGS=/nologo /std:c++17 /W4 /WX /GR- /EHa- /O2 /DAG_RELEASE
)

set INCLUDES=-I Engine/Include
set DEFINES=

echo [Afterglow] Building engine [%MODE%]...
cl %FLAGS% %INCLUDES% %DEFINES% /c Engine/Src/*.cpp /Fo:%OUT_DIR%\
if %errorlevel% neq 0 goto error

lib /nologo /OUT:%OUT_DIR%\Afterglow.lib %OUT_DIR%\*.obj
if %errorlevel% neq 0 goto error

echo.
echo [Afterglow] Building game [%MODE%]...
cl %FLAGS% %INCLUDES% %DEFINES% /c Game/Src/*.cpp /Fo:%OUT_DIR%\
if %errorlevel% neq 0 goto error

link /nologo %OUT_DIR%\*.obj %OUT_DIR%\Afterglow.lib /OUT:%OUT_DIR%\Game.exe /PDB:%OUT_DIR%\Game.pdb
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