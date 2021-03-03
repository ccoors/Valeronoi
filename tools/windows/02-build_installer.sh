#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
docker run --rm -it --init -v "${PWD}/../../":/source valeronoi-windows /source/tools/windows/.docker/in_docker.sh
