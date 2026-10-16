#!/usr/bin/env sh
# Builds the DLL and creates dist/VRFocusFix-<version>.zip for a release.
set -e
cd "$(dirname "$0")/.."
VER=$(sed -n 's/^#define VRFF_VERSION "\(.*\)"/\1/p' src/vrfocusfix.cpp)
./build.sh
rm -rf dist/VRFocusFix && mkdir -p dist/VRFocusFix
cp build/openvr_api.dll dist/VRFocusFix/vrfocusfix_openvr_api.dll
cp scripts/install.bat scripts/uninstall.bat scripts/vrfocusfix.ini scripts/README.txt LICENSE dist/VRFocusFix/
# Windows line endings for files people open in Notepad / cmd
for f in install.bat uninstall.bat vrfocusfix.ini README.txt LICENSE; do
  sed -i.bak 's/\r*$/\r/' "dist/VRFocusFix/$f" && rm -f "dist/VRFocusFix/$f.bak"
done
(cd dist && rm -f "VRFocusFix-$VER.zip" && python3 -c "
import zipfile,os,sys
z=zipfile.ZipFile(sys.argv[1],'w',zipfile.ZIP_DEFLATED)
for r,_,fs in os.walk('VRFocusFix'):
  for f in sorted(fs): z.write(os.path.join(r,f))
" "VRFocusFix-$VER.zip")
echo "Created dist/VRFocusFix-$VER.zip"
