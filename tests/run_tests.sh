#!/usr/bin/env sh
# Runs the tests under Wine (64-bit). Usage:
#   tests/run_tests.sh [path/to/a/real/openvr_api.dll]
# The optional real DLL (e.g. the one shipped with the game) enables the
# pass-through test against genuine Valve code.
set -e
cd "$(dirname "$0")"
if command -v zig >/dev/null 2>&1; then ZIG="zig"; else ZIG="python3 -m ziglang"; fi
WINE="${WINE:-$(command -v wine64 || command -v wine || echo /usr/lib/wine/wine64)}"
export WINEDEBUG=-all
T=$(mktemp -d)
../build.sh >/dev/null
$ZIG c++ -target x86_64-windows-gnu -O2 -shared -o "$T/openvr_api_orig.dll" fake_runtime.cpp
$ZIG c++ -target x86_64-windows-gnu -O1 -o "$T/game_sim.exe" game_sim.cpp
cp ../build/openvr_api.dll "$T/"
fail=0
# succeeds when the simulated game never froze while the menu was closed
run() { out=$(cd "$T" && "$WINE" game_sim.exe 2>/dev/null | tr -d '\r'); echo "   $out"; echo "$out" | grep -q ': 0$'; }

echo "1. without the fix (EventFix=none) the bug must reproduce"
printf '[VRFocusFix]\nEventFix=none\n' > "$T/vrfocusfix.ini"
if run; then echo "   FAIL: bug did not reproduce"; fail=1; else echo "   ok"; fi

echo "2. default settings (no ini) must fix it and write no log"
rm -f "$T/vrfocusfix.ini" "$T/vrfocusfix.log"
if run; then echo "   ok"; else echo "   FAIL"; fail=1; fi
if [ -f "$T/vrfocusfix.log" ]; then echo "   FAIL: log written by default"; fail=1; else echo "   ok: no log file"; fi

echo "3. EventFix=swap with Log=1 must fix it and log the swaps"
printf '[VRFocusFix]\nEventFix=swap\nLog=1\n' > "$T/vrfocusfix.ini"
if run; then echo "   ok"; else echo "   FAIL"; fail=1; fi
grep -q "game sees DashboardActivated" "$T/vrfocusfix.log" && echo "   ok: log written" || { echo "   FAIL: no log"; fail=1; }

if [ -n "$1" ]; then
  echo "4. pass-through with a real openvr_api.dll ($1)"
  R=$(mktemp -d)
  $ZIG dlltool -m i386:x86-64 -d imports.def -l "$R/libimp.a"
  $ZIG c++ -target x86_64-windows-gnu -O1 -o "$R/real_dll_test.exe" real_dll_test.cpp "$R/libimp.a"
  cp "$1" "$R/openvr_api.dll"; base=$(cd "$R" && "$WINE" real_dll_test.exe 2>/dev/null | tr -d '\r' | grep -v '^Unable')
  mv "$R/openvr_api.dll" "$R/openvr_api_orig.dll"; cp ../build/openvr_api.dll "$R/"
  prox=$(cd "$R" && "$WINE" real_dll_test.exe 2>/dev/null | tr -d '\r' | grep -v '^Unable')
  echo "$prox" | sed 's/^/   /'
  [ "$base" = "$prox" ] && echo "   ok: identical to the original DLL alone" || { echo "   FAIL: differs from original"; fail=1; }
  rm -rf "$R"
fi
rm -rf "$T"
[ $fail = 0 ] && echo "ALL TESTS PASSED" || { echo "TESTS FAILED"; exit 1; }
