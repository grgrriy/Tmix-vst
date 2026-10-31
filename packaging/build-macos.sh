#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "ERROR: Build the macOS plugins on macOS with Xcode. Current OS: $(uname -s)" >&2
  exit 2
fi
if ! command -v cmake >/dev/null 2>&1; then
  echo "Install CMake first: brew install cmake" >&2; exit 2
fi
if [[ ! -d JUCE ]]; then
  echo "JUCE folder not found. Put JUCE 8 source in ./JUCE (next to CMakeLists.txt)." >&2
  exit 2
fi

xcode-select -p >/dev/null 2>&1 || { echo "Install Xcode Command Line Tools: xcode-select --install"; exit 2; }

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --parallel

ART="build/TMix_artefacts/Release"
VST3="$ART/VST3/T-Mix.vst3"
AU="$ART/AU/T-Mix.component"

[[ -d "$VST3" ]] || { echo "VST3 bundle not found at $VST3"; find build -name '*.vst3' -print; exit 3; }
[[ -d "$AU" ]] || { echo "AU bundle not found at $AU"; find build -name '*.component' -print; exit 3; }

mkdir -p packaging/stage
rm -rf packaging/stage/*
cp -R "$VST3" packaging/stage/
cp -R "$AU" packaging/stage/
cp packaging/Install-T-Mix.command packaging/stage/
cp packaging/Uninstall-T-Mix.command packaging/stage/
cp packaging/INSTALL.txt packaging/stage/README-INSTALL.txt
chmod +x packaging/stage/*.command

echo "Build complete. Installer payload is in packaging/stage/"
