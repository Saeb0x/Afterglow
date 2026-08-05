@echo off
setlocal

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if /i "%BUILD%"=="release" set BUILD_DIR=%~dp0Build\Release

if exist "%BUILD_DIR%" (
    pushd "%BUILD_DIR%"

    if not exist "Afterglow.exe" (
        echo [Afterglow] Afterglow.exe not found. Run Build.bat %BUILD% first.
        popd
        exit /b 1
    )

    start "" "Afterglow.exe"
    popd
) else (
    echo [Afterglow] Afterglow.exe not found. Run Build.bat %BUILD% first.
)

endlocal
