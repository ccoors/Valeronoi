#!/usr/bin/env bash

set -euo pipefail

DEST=$(mktemp -d)
JOBS=$(nproc --ignore 4)
cd "$DEST"
x86_64-w64-mingw32-cmake /source -DCMAKE_BUILD_TYPE=Release
make -j"${JOBS}"
mkdir dist
cp /source/COPYING dist
mv valeronoi.exe dist
cp /source/tools/windows/install.nsi dist
cp /source/assets/appinfo.rc dist
mkdir dist/icon
cp /source/assets/icon/appicon.ico dist/icon
cd dist
cp -rv /usr/x86_64-w64-mingw32/lib/qt6/plugins/platforms .
cp -rv /usr/x86_64-w64-mingw32/lib/qt6/plugins/styles .
python /source/tools/windows/.docker/copy_dlls.py
makensis install.nsi

mkdir -p /source/dist
mv Valeronoi-installer-x86_64.exe /source/dist
rm install.nsi appinfo.rc
cp -r . /source/dist/Valeronoi-Windows-portable-x86_64
