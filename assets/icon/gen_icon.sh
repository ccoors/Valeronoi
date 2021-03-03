#!/usr/bin/env bash

set -euxo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
sizes=(16 24 32 48 64 128 144 180 192 256 512 1024)
input="icon"

for size in "${sizes[@]}"; do
	outfile="${input}-${size}.png"
	inkscape --export-filename "$outfile" -w "$size" -h "$size" "${input}.svg"
	optipng -o7 "$outfile"
done

magick convert icon-16.png icon-32.png icon-256.png appicon.ico
cp icon-256.png ../../src/res/valeronoi.png
