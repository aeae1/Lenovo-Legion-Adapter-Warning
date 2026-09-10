@echo off
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0Read-SecureBootState.ps1"
if errorlevel 1 echo Reading failed. Right-click this file and choose Run as administrator.
pause
