@echo off
setlocal EnableDelayedExpansion

set ROOT=%~dp0
set COOKER=%ROOT%Build\Tools\AgCooker.exe
set SHADERS=%ROOT%Assets\Shaders
set FONTS=%ROOT%Assets\Fonts
set TEXTURES=%ROOT%Assets\Textures
set OUT=%ROOT%Data

if not exist "%COOKER%" (
    echo [Afterglow] Cooker not found, building it...
    call "%ROOT%Tools\Build.bat"
    if !errorlevel! neq 0 exit /b 1
)

if not exist "%OUT%" mkdir "%OUT%"

echo [Afterglow] Cooking shaders...

"%COOKER%" shader "%SHADERS%\Quad.hlsl" VSMain vs_5_0 "%OUT%\QuadVS.aga"
if !errorlevel! neq 0 goto error

"%COOKER%" shader "%SHADERS%\Quad.hlsl" PSMain ps_5_0 "%OUT%\QuadPS.aga"
if !errorlevel! neq 0 goto error

"%COOKER%" shader "%SHADERS%\Quad.hlsl" PSMainAlpha ps_5_0 "%OUT%\QuadPSAlpha.aga"
if !errorlevel! neq 0 goto error

echo [Afterglow] Cooking fonts...

call :CookFonts
if !errorlevel! neq 0 goto error

echo [Afterglow] Cooking textures...

call :CookTextures
if !errorlevel! neq 0 goto error

echo [Afterglow] Cooking succeeded.
goto end

:CookFonts
if not exist "%FONTS%\LMDebug.fnt" (
    echo Skipping UIFont.aga - "%FONTS%\LMDebug.fnt" not found ^(font sources are untracked^)
    exit /b 0
)
if not exist "%FONTS%\LMDebug_0.png" (
    echo Skipping UIFont.aga - "%FONTS%\LMDebug_0.png" not found ^(font sources are untracked^)
    exit /b 0
)
"%COOKER%" font "%FONTS%\LMDebug.fnt" "%FONTS%\LMDebug_0.png" "%OUT%\UIFont.aga"
exit /b !errorlevel!

:CookTextures
if not exist "%TEXTURES%\S.png" (
    echo Skipping S.aga - "%TEXTURES%\S.png" not found ^(texture sources are untracked^)
    exit /b 0
)
"%COOKER%" texture "%TEXTURES%\S.png" "%OUT%\S.aga"
exit /b !errorlevel!

:error
echo.
echo [Afterglow] Cooking failed.
exit /b 1

:end
endlocal
