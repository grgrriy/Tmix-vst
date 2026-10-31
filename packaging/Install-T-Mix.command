#!/bin/bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
VST_SRC="$HERE/T-Mix.vst3"
AU_SRC="$HERE/T-Mix.component"
VST_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

if [[ ! -d "$VST_SRC" ]]; then
  echo "T-Mix.vst3 is missing beside this installer."; exit 1
fi
mkdir -p "$VST_DIR" "$AU_DIR"
rm -rf "$VST_DIR/T-Mix.vst3"
cp -R "$VST_SRC" "$VST_DIR/"
if [[ -d "$AU_SRC" ]]; then
  rm -rf "$AU_DIR/T-Mix.component"
  cp -R "$AU_SRC" "$AU_DIR/"
fi
echo
echo "T-Mix installed for this macOS user."
echo "VST3: $VST_DIR/T-Mix.vst3"
[[ ! -d "$AU_SRC" ]] || echo "AU:   $AU_DIR/T-Mix.component"
echo
echo "In FL Studio: Options > Manage plugins > Find plugins."
echo "Make sure the VST3 scan path is enabled, then run Find plugins."
