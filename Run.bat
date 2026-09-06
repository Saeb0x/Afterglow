@echo off
setlocal

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if "%BUILD%"=="release" set BUILD_DIR=%~dp0Build\Release

if exist "%BUILD_DIR%" (
    pushd "%BUILD_DIR%"

    if not exist "AfterglowGame.exe" (
        echo [Afterglow] AfterglowGame.exe not found. Run Build.bat %BUILD% first.
        popd
        endlocal
        exit /b 1
    )

    start "" "AfterglowGame.exe"
    popd
) else (
    echo [Afterglow] AfterglowGame.exe not found. Run Build.bat %BUILD% first.
    endlocal
    exit /b 1
)

endlocal
exit /b 0
