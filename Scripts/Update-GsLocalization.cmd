@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-GsLocalization.ps1" -ProjectFile "%~1" -EngineRoot "%~2"
exit /b %ERRORLEVEL%
