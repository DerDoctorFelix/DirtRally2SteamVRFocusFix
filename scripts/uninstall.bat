@echo off
setlocal
cd /d "%~dp0"
set "TARGET="
for /f "delims=" %%F in ('dir /s /b "%~dp0..\openvr_api_orig.dll" "%~dp0openvr_api_orig.dll" 2^>nul') do (
  if not defined TARGET set "TARGET=%%~dpF"
)
if not defined TARGET ( echo Nothing to uninstall - openvr_api_orig.dll not found. & pause & exit /b 0 )
del /q "%TARGET%openvr_api.dll" 2>nul
ren "%TARGET%openvr_api_orig.dll" openvr_api.dll
if not exist "%TARGET%openvr_api.dll" (echo Could not restore the original DLL. Is the game running? & pause & exit /b 1)
del /q "%TARGET%vrfocusfix.ini" "%TARGET%vrfocusfix.log" 2>nul
echo Restored the game's original openvr_api.dll in %TARGET%
pause
