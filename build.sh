#!/usr/bin/env sh
# Builds dist/openvr_api.dll (64-bit Windows) with Zig's bundled clang/MinGW.
# Works on Linux, macOS and Windows (Git Bash / WSL). Needs Zig 0.11+ on PATH,
# or Python with `pip install ziglang`.
set -e
cd "$(dirname "$0")"
if command -v zig >/dev/null 2>&1; then ZIG="zig"; else ZIG="python3 -m ziglang"; fi
mkdir -p build
$ZIG c++ -target x86_64-windows-gnu -O2 -shared -fno-exceptions -fno-rtti -s \
    -o build/openvr_api.dll src/vrfocusfix.cpp src/openvr_api.def -luser32
rm -f build/*.lib build/*.pdb
echo "Built build/openvr_api.dll"
