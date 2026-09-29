@echo off
setlocal enabledelayedexpansion

call vcvarsall.bat x64 >nul 2>&1
if !errorlevel! neq 0 (
    echo [Afterglow] Failed to call vcvarsall.bat. Ensure Microsoft C/C++ build tools are installed and vcvarsall.bat is accessible from the current environment.
    endlocal
    exit /b 1
)

set BUILD=debug
if /i "%1"=="release" set BUILD=release

set BUILD_DIR=%~dp0Build\Debug
if "!BUILD!"=="release" set BUILD_DIR=%~dp0Build\Release

if not exist "!BUILD_DIR!" mkdir "!BUILD_DIR!"
pushd "!BUILD_DIR!"

if "!BUILD!"=="debug" (
    if not exist "box2d.lib" (
        echo [Afterglow] Compiling Box2D [debug]...
        if not exist "Box2D" mkdir "Box2D"
        cl /nologo /c /std:c17 /utf-8 /MTd /Od /Zi ^
        /I "%~dp0External\Box2D\include" ^
        "%~dp0External\Box2D\src\*.c" ^
        /Fo:"Box2D\\" /Fd:"Box2D\box2d.pdb"
        if !errorlevel! neq 0 goto error
        lib /nologo /OUT:"box2d.lib" "Box2D\*.obj"
        if !errorlevel! neq 0 goto error
    )

    echo [Afterglow] Compiling and linking game [debug]...
    cl /nologo /std:c++20 /permissive- /utf-8 /MTd /Od /Zi ^
    /I "%~dp0Src" /I "%~dp0External\SSTL\Include" /I "%~dp0External\DirectXMath\Inc" /I "%~dp0External\Box2D\include" ^
    "%~dp0Src\Afterglow.cpp" ^
    "%~dp0Src\Sandbox\Sandbox.cpp" ^
    /Fd"Afterglow.pdb" /Fe"Afterglow.exe" ^
    /link /nologo /DEBUG Kernel32.lib User32.lib D3D11.lib DXGI.lib DXGUID.lib box2d.lib
    if !errorlevel! neq 0 goto error

    echo [Afterglow] Compiling and linking cooker [debug]...
    cl /nologo /std:c++20 /permissive- /utf-8 /MTd /Od /Zi ^
    /I "%~dp0Src" /I "%~dp0External\stb" /I "%~dp0External\SSTL\Include" ^
    "%~dp0Src\AfterglowCooker.cpp" ^
    /Fd"AfterglowCooker.pdb" /Fe"AfterglowCooker.exe" ^
    /link /nologo /DEBUG Kernel32.lib D3DCompiler.lib
    if !errorlevel! neq 0 goto error
) else (
    if not exist "box2d.lib" (
        echo [Afterglow] Compiling Box2D [release]...
        if not exist "Box2D" mkdir "Box2D"
        cl /nologo /c /std:c17 /utf-8 /MT /O2 ^
        /I "%~dp0External\Box2D\include" ^
        "%~dp0External\Box2D\src\*.c" ^
        /Fo:"Box2D\\"
        if !errorlevel! neq 0 goto error
        lib /nologo /OUT:"box2d.lib" "Box2D\*.obj"
        if !errorlevel! neq 0 goto error
    )

    echo [Afterglow] Compiling and linking game [release]...
    cl /nologo /std:c++20 /permissive- /utf-8 /MT /O2 ^
    /I "%~dp0Src" /I "%~dp0External\SSTL\Include" /I "%~dp0External\DirectXMath\Inc" /I "%~dp0External\Box2D\include" ^
    "%~dp0Src\Afterglow.cpp" ^
    "%~dp0Src\Sandbox\Sandbox.cpp" ^
    /Fe"Afterglow.exe" ^
    /link /nologo Kernel32.lib User32.lib D3D11.lib DXGI.lib DXGUID.lib box2d.lib
    if !errorlevel! neq 0 goto error

    echo [Afterglow] Compiling and linking cooker [release]...
    cl /nologo /std:c++20 /permissive- /utf-8 /MT /O2 ^
    /I "%~dp0Src" /I "%~dp0External\stb" /I "%~dp0External\SSTL\Include" ^
    "%~dp0Src\AfterglowCooker.cpp" ^
    /Fe"AfterglowCooker.exe" ^
    /link /nologo Kernel32.lib D3DCompiler.lib
    if !errorlevel! neq 0 goto error
)

set COOK_FLAGS=
if "!BUILD!"=="debug" set COOK_FLAGS=--debug

echo [Afterglow] Cooking assets [!BUILD!]...
"!BUILD_DIR!\AfterglowCooker.exe" "%~dp0Data" "!BUILD_DIR!\Data" !COOK_FLAGS!
if !errorlevel! neq 0 goto error

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
