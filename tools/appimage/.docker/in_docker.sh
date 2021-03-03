#!/usr/bin/env bash

set -euo pipefail

DEST=$(mktemp -d)
JOBS=$(nproc --ignore 4)
cd "$DEST"
cmake /source -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make -j"${JOBS}"
./valeronoi-tests
make install DESTDIR=AppDir
LD_LIBRARY_PATH=/usr/local/lib PATH=/usr/local/lib/qt6/bin:$PATH /tools/linuxdeploy-x86_64.AppImage --appdir AppDir -i /source/assets/icon/icon-256.png -d AppDir/usr/share/applications/valeronoi.desktop --output appimage --plugin qt
mkdir -p /source/dist
mv Valeronoi-x86_64.AppImage /source/dist/Valeronoi-"$VARIANT"-x86_64.AppImage
