# Changelog

## 1.0.0 - 2026-10-16

First public release.

- Swaps SteamVR's `DashboardActivated` / `DashboardDeactivated` events for the
  game, so DiRT Rally 2.0 pauses while the Steam menu is open and resumes when
  it closes (instead of the reverse, which left the headset frozen).
- All other OpenVR calls are passed through to the game's original
  `openvr_api.dll` unchanged.
- Optional settings file (`vrfocusfix.ini`) with `EventFix=swap|none|drop` and
  `Log=0|1`. Logging is off by default.
- `install.bat` / `uninstall.bat` that back up and restore the original DLL.
