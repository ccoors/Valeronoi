#!/usr/bin/env bash

set -euxo pipefail

# Install
## Install brew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew analytics off

## Install Valeronoi dependencies
brew install cgal qt@5 cmake create-dmg
## Add Qt to PATH (because it's not Qt 6)
export PATH="/usr/local/opt/qt@5/bin:$PATH"

# Build
git clone https://github.com/ccoors/Valeronoi.git
cd Valeronoi && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
./valeronoi-tests

# Package
macdeployqt valeronoi.app
mkdir dist
mv valeronoi.app dist/Valeronoi.app

# This needs UI/Permissions
create-dmg --volname "Valeronoi Installer" --volicon "../assets/icon/valeronoi.icns" --window-pos 200 120 --window-size 800 400 --icon-size 100 --icon "Valeronoi.app" 200 190 --hide-extension "Valeronoi.app" --app-drop-link 600 185 "Valeronoi-macOS-Intel.dmg" "dist/"

# Alternatively, generate a zip archive
zip -r Valeronoi-macOS-Intel.zip Valeronoi.app
