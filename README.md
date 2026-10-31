# T-Mix v0.2 — JUCE audio-effect prototype

Target: macOS first, with VST3 as the recommended format for FL Studio. AU is also configured for compatible macOS hosts.

## What's new in v0.2
- Two operating modes: AUTO MIX and AI ASSIST. Assist currently applies a deliberately gentler amount of processing; it is not a generative AI model.
- Style selector: Neutral, Modern Pop, Electronic, Rock, Cinematic, Hip-Hop.
- Stereo width control with Mid/Side width scaling (enabled by Stereo Control).
- Stereo-linked compressor detector to keep the stereo image more stable.
- Stereo correlation meter in the UI.
- Expanded live meter display and updated 620x500 minimal interface.
- Persistent APVTS parameters and track role selection.

## Track roles
Guitar, Bass, Synth, Piano, Orchestra, Vocal, Vocal Chop, Kick, Snare, Hi-Hat, Percussion, Drum Bus, Master, FX, Custom.

## Build on macOS
1. Install Xcode Command Line Tools: `xcode-select --install`.
2. Install CMake (for example with Homebrew): `brew install cmake`.
3. Download JUCE 8 from https://juce.com/get-juce/.
4. Put the JUCE source folder at `T-Mix/JUCE` (next to this file).
5. Configure and build:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
6. In FL Studio, open Plugin Manager, add the generated VST3 location to the scan paths, then rescan.

The CMake project declares both VST3 and AU. AU is macOS-only. VST3 is the recommended FL Studio target.

## Honest scope / not yet implemented
This remains a DSP prototype, not a finished AI mixing product. It does not yet perform neural instrument recognition, source separation, true adaptive spectral masking, reference-track matching, LUFS/true-peak mastering, or mix-wide context analysis. The genre selector applies small tonal-intent offsets; it is not genre classification. The analysis button is a UI cue; meters are continuously updated. Mix-context mode requires a companion bus/sidechain feed or a shared analysis mechanism across plugin instances; a normal insert cannot hear unrelated mixer channels by itself.

Before commercial use: compile and test against the selected JUCE release and target DAWs; add parameter smoothing, robust state/version migration, latency reporting, oversampling options, automation stress tests, preset management, CPU profiling, and audio QA. No compiled binary is included in this source archive.


## Create a macOS DMG installer

The repository now includes macOS packaging scripts in `packaging/`.
A real Apple disk image must be created on macOS (`hdiutil` is not available
on Linux), after compiling the AU/VST3 bundles.

1. Put JUCE 8 source in `T-Mix/JUCE`.
2. On a Mac, run `./packaging/build-macos.sh`.
3. Run `./packaging/create-dmg.sh`.
4. The disk image will be `dist/T-Mix-macOS.dmg`.

The DMG payload includes an installer command and an uninstaller command.
It installs into the current user's `~/Library/Audio/Plug-Ins` folders,
so administrator privileges are not required. This developer build is not
Apple Developer ID signed or notarized; distribution outside your own Mac
requires signing and notarization.


## Build a DMG without a local Mac
See `BUILD-WITHOUT-A-MAC.md`. The included GitHub Actions workflow uses a hosted macOS runner to fetch JUCE, build universal VST3/AU bundles, and produce a DMG artifact.
