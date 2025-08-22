@echo off
pushd %~dp0\..\
CALL Vendor\Premake\premake5.exe vs2022
PAUSE