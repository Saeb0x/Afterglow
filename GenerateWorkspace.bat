@echo off

set ROOT=%~dp0
set PREMAKE=%ROOT%vendor\Windows\Premake-5.0.0-beta7\premake5.exe

cd /d "%ROOT%"
"%PREMAKE%" --file=AfterglowWorkspace.lua vs2022

pause