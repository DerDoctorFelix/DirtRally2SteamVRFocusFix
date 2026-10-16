@echo off
setlocal
cd /d "%~dp0"
echo VRFocusFix installer
echo.
if not exist "%~dp0vrfocusfix_openvr_api.dll" (
  echo vrfocusfix_openvr_api.dll is missing. Keep all files from the zip together.
  pause & exit /b 1
)
rem Find the game's openvr_api.dll in the parent folder (the game folder) or below.
set "TARGET="
for /f "delims=" %%F in ('dir /s /b "%~dp0..\openvr_api.dll" "%~dp0openvr_api.dll" 2^>nul') do (
  if not defined TARGET set "TARGET=%%~dpF"
)
if not defined TARGET (
  echo Could not find openvr_api.dll.
  echo Put the VRFocusFix folder INSIDE the game's install folder
  echo ^(Steam: right-click the game, Manage, Browse local files^) and run this again.
  pause & exit /b 1
)
echo Game folder: %TARGET%
echo.
if exist "%TARGET%openvr_api_orig.dll" (
  echo The original DLL is already backed up as openvr_api_orig.dll - updating VRFocusFix only.
) else (
  ren "%TARGET%openvr_api.dll" openvr_api_orig.dll
)
if not exist "%TARGET%openvr_api_orig.dll" (echo Could not back up the original DLL. Is the game running? & pause & exit /b 1)
copy /y "%~dp0vrfocusfix_openvr_api.dll" "%TARGET%openvr_api.dll" >nul
fc /b "%~dp0vrfocusfix_openvr_api.dll" "%TARGET%openvr_api.dll" >nul 2>&1
if errorlevel 1 (echo Could not copy VRFocusFix. Is the game running? & pause & exit /b 1)
rem Keep the user's settings, but replace settings files from pre-release test builds.
if not exist "%TARGET%vrfocusfix.ini" goto copyini
findstr /i /c:"Trace=" "%TARGET%vrfocusfix.ini" >nul 2>&1
if errorlevel 1 goto inidone
:copyini
copy /y "%~dp0vrfocusfix.ini" "%TARGET%vrfocusfix.ini" >nul
:inidone
echo Installed. Start the game in SteamVR mode as usual.
echo Settings: %TARGET%vrfocusfix.ini
pause
