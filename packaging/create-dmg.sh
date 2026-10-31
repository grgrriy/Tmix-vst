#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "ERROR: hdiutil is available only on macOS. Run this script on a Mac." >&2
  exit 2
fi
STAGE="packaging/stage"
[[ -d "$STAGE/T-Mix.vst3" ]] || { echo "Run packaging/build-macos.sh first."; exit 2; }
OUT="dist"
mkdir -p "$OUT"
rm -f "$OUT/T-Mix-macOS.dmg"
hdiutil create -volname "T-Mix Installer" -srcfolder "$STAGE" -ov -format UDZO "$OUT/T-Mix-macOS.dmg"
echo "Created: $OUT/T-Mix-macOS.dmg"
