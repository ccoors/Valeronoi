#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
docker build --pull -t valeronoi-appimage-current .
