# VRFocusFix

**Fixes DiRT Rally 2.0 freezing in VR on Steam Frame when you close the Steam menu.**

> **AI disclosure:** this project was written with the help of an AI model,
> Claude (Anthropic). The code, the diagnosis and this documentation were
> produced in a conversation between the author and Claude; the author tested
> every stage on a real Steam Frame and provided the logs that pinned down the
> cause. See [How this was made](#how-this-was-made).

---

## The problem

When you play DiRT Rally 2.0 in SteamVR mode on a Steam Frame:

- the picture in the headset freezes (audio keeps playing) as soon as the
  Steam menu is **closed**, often right after launch;
- opening the Steam menu (Steam button on the controller) makes the game run
  again, but darkened behind the menu and without controller input;
- closing the menu freezes it again.

It looks as if the game's "pause while the Steam menu is open" rule is
reversed, and that is what turned out to be happening.

## What it does

SteamVR tells a game when its menu (the *dashboard*) opens and closes with two
events: `DashboardActivated` and `DashboardDeactivated`. Logs recorded on a
Steam Frame showed that DiRT Rally 2.0 stops sending frames to the headset
within one frame of receiving `DashboardDeactivated` (menu closed), and starts
again within one frame of `DashboardActivated` (menu opened), which is the
opposite of what the events mean.

VRFocusFix is a small replacement for the game's `openvr_api.dll` that swaps
those two events before the game sees them. Result:

- **Steam menu open:** the game pauses its VR picture (SteamVR shows its own
  environment behind the menu).
- **Steam menu closed:** the game runs normally.

Every other OpenVR call is passed through unchanged to the game's original
`openvr_api.dll`, which the installer keeps as `openvr_api_orig.dll`. Nothing
is injected into Steam or SteamVR, and no game files other than that one DLL
are touched.

Whether the game or the Steam Frame's SteamVR has the meaning backwards is
not known; the swap works either way.

## Install

1. Download `VRFocusFix-1.0.0.zip` from the
   [Releases](../../releases) page.
2. In Steam, right-click **DiRT Rally 2.0 → Manage → Browse local files**.
3. Extract the zip there, so you get a `VRFocusFix` folder inside the game
   folder.
4. Double-click `VRFocusFix\install.bat`.
5. Start the game in SteamVR mode as usual.

`install.bat` renames the game's `openvr_api.dll` to `openvr_api_orig.dll`
and copies the fix in as `openvr_api.dll`. Running it again is safe: it never
overwrites the backup.

### Manual install

In the game folder:

1. Rename `openvr_api.dll` to `openvr_api_orig.dll`.
2. Copy `vrfocusfix_openvr_api.dll` from the zip into the game folder and
   rename it to `openvr_api.dll`.
3. Optionally copy `vrfocusfix.ini` next to it.

The fix ships as `vrfocusfix_openvr_api.dll` on purpose. If it were called
`openvr_api.dll`, copying it into the game folder would overwrite the game's
original DLL before it could be backed up, and the game would no longer start.

## Uninstall

Double-click `VRFocusFix\uninstall.bat`, or in Steam use **Properties →
Installed Files → Verify integrity of game files**. Either one restores the
original `openvr_api.dll`.

A game update or "Verify integrity" also removes the fix. Run `install.bat`
again afterwards.

## Settings

`vrfocusfix.ini` in the game folder (optional; these are the defaults):

```ini
[VRFocusFix]
EventFix=swap
Log=0
```

| Setting    | Values                 | Meaning |
|------------|------------------------|---------|
| `EventFix` | `swap` (default)       | Swap "menu opened" and "menu closed". This is the fix. |
|            | `none`                 | Change nothing (the original bug). |
|            | `drop`                 | Hide both events from the game. Did **not** fix DiRT Rally 2.0 on Steam Frame; kept for experiments. |
| `Log`      | `0` (default) / `1`    | Write `vrfocusfix.log` next to the DLL. Useful when reporting a problem. |

## Troubleshooting

- **The game doesn't start / "could not load openvr_api_orig.dll":** the
  original DLL is missing. Run `uninstall.bat` or verify the game files in
  Steam, then run `install.bat` again.
- **Still freezing:** set `Log=1`, start the game, open and close the Steam
  menu a couple of times, then open an issue with `vrfocusfix.log` attached.
  With logging on, each menu event is logged with what the game was told.
  If the log is empty or missing, the game didn't load the fix.
- **Online play:** the fix only replaces the VR runtime loader DLL and doesn't
  change game code, but use it online at your own discretion.

## Compatibility

- **Tested:** DiRT Rally 2.0 (Steam, SteamVR mode) on Steam Frame, Windows,
  with the event swap confirmed working by the author. The game ships OpenVR
  1.0.10's `openvr_api.dll` and uses the `IVRSystem_019` interface.
- **Other games:** the DLL works with any 64-bit OpenVR game that keeps
  `openvr_api.dll` in its own folder (it supports IVRSystem versions 011–026),
  but swapping these events only makes sense for games with the same reversed
  behaviour. Most games don't have it.
- 64-bit Windows 10/11 only.

## How it works (technical)

- All exports of `openvr_api.dll` are implemented as small assembly thunks
  that load `openvr_api_orig.dll` **by full path** from the DLL's own folder
  and jump to the matching function, so every signature is passed through
  untouched. No PE export forwarding is used, because forwarding depends on
  the Windows DLL search order.
- `VR_GetGenericInterface` is intercepted. When the game asks for
  `IVRSystem_0xx` (C++ or `FnTable:` C API), the two event-polling functions
  `PollNextEvent` and `PollNextEventWithPose` are patched in the runtime's
  vtable. The original function pointer is recorded before the slot is
  overwritten, so concurrent calls are safe.
- Each event the game polls is checked: `DashboardActivated` (502) and
  `DashboardDeactivated` (503) are swapped, and everything else is returned
  as-is.
- The vtable slot numbers come from the public
  [OpenVR SDK headers](https://github.com/ValveSoftware/openvr) for every
  published `IVRSystem` version.

Source: [`src/vrfocusfix.cpp`](src/vrfocusfix.cpp), with export thunks in
[`src/thunks.h`](src/thunks.h).

## Building from source

The DLL is built with [Zig](https://ziglang.org/)'s bundled clang/MinGW
toolchain, which cross-compiles for Windows from Linux, macOS or Windows:

```sh
pip install ziglang     # or install Zig 0.11+ and put it on PATH
./build.sh              # -> build/openvr_api.dll
scripts/package.sh      # -> dist/VRFocusFix-<version>.zip
```

MSVC isn't supported because the export thunks use GCC-style inline assembly.

### Tests

`tests/run_tests.sh` runs under Wine (64-bit). It uses a fake OpenVR runtime
and a small program that mimics the game's observed behaviour, and checks
that:

1. without the fix, the simulated game freezes while the menu is closed;
2. with default settings it never does, and no log file is written;
3. with `Log=1` the swaps are logged.

Pass the path of a real `openvr_api.dll` (for example the game's) to also
check that calls through the proxy behave exactly like the original DLL:

```sh
tests/run_tests.sh "/path/to/DiRT Rally 2.0/openvr_api.dll"
```

GitHub Actions (`.github/workflows/build.yml`) runs the tests and builds the
release zip on every push. Pushing a `v*` tag publishes a GitHub release.

## How this was made

This project was built in a single conversation between the author and
**Claude**, an AI model made by Anthropic. Claude wrote the code, the
installer scripts, the tests and this documentation, and analysed the logs.
The author ran each build on a Steam Frame, reported what happened, and sent
the logs back.

It took several rounds to find the real cause:

1. The first guess was a known OpenVR change: the "is input focus captured?"
   query was replaced by "is input available?" with the opposite meaning.
   Logs showed the game never calls either function.
2. A diagnostic build that logged every OpenVR call showed the game keeps
   running but stops submitting frames right after `DashboardDeactivated`,
   and starts again after `DashboardActivated`.
3. Three event strategies were then tested on the headset: re-sending a
   legacy "input focus released" event (no effect), hiding the dashboard
   events (no effect), and swapping them, which fixed it.

The AI-written code was reviewed and tested as described above, but it has
not been audited by a human security or graphics expert. Read the source
(about 300 lines) if you want to know exactly what runs in your game.

## License

[MIT](LICENSE). No warranty.

Not affiliated with or endorsed by Valve, Codemasters or Electronic Arts.
DiRT Rally, Steam, SteamVR and Steam Frame are trademarks of their respective
owners. This project does not include or redistribute Valve's
`openvr_api.dll`; it uses the copy that ships with your game.
