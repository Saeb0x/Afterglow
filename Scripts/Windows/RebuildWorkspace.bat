call CleanWorkspace.bat
if %ERRORLEVEL% neq 0 exit /b 1

call GenerateWorkspace.bat
if %ERRORLEVEL% neq 0 exit /b 1