@echo off
setlocal

for /f "tokens=*" %%i in ('where git 2^>nul') do (
    for %%j in ("%%~dpi..") do set GIT_ROOT=%%~fj
    goto :run
)
echo ERROR: git not found in PATH. Please install Git for Windows.
exit /b 1

:run
"%GIT_ROOT%\bin\bash.exe" "%~dp0sync-assets.sh"
