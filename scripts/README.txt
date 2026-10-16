VRFocusFix 1.0.0
Fixes DiRT Rally 2.0 freezing in VR on Steam Frame when you close the Steam menu.
Project page, source code and documentation: see README.md in the repository.

This tool was created with the help of an AI model (Claude, by Anthropic).
See "How this was made" in README.md.

INSTALL
1. Steam: right-click DiRT Rally 2.0 > Manage > Browse local files.
2. Copy this whole VRFocusFix folder into that folder.
3. Double-click VRFocusFix\install.bat.
   It renames the game's openvr_api.dll to openvr_api_orig.dll and puts the
   fix in its place.
4. Start the game in SteamVR mode as usual.

Now the game pauses while the Steam menu is open and continues as soon as you
close it.

UNINSTALL
Double-click VRFocusFix\uninstall.bat, or in Steam use Properties >
Installed Files > Verify integrity of game files.
After a game update or "Verify integrity", run install.bat again.

SETTINGS
vrfocusfix.ini in the game folder. Set Log=1 to write vrfocusfix.log when
reporting a problem.

No warranty - see LICENSE. Not affiliated with Valve, Codemasters or EA.
