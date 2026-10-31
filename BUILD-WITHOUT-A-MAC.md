# Build the T-Mix DMG without owning a Mac

You do not need to install CMake or Xcode on your own computer if you use
GitHub Actions. GitHub provides a hosted macOS runner that runs the build.

## Steps

1. Create a GitHub account if you do not have one.
2. Create a new repository, for example `T-Mix`.
3. Upload the complete contents of this project ZIP to that repository.
   Make sure `.github/workflows/build-macos.yml` is included.
4. Open the repository's **Actions** tab.
5. Select **Build T-Mix macOS installer**.
6. Click **Run workflow** and wait for the job to finish.
7. Open the completed workflow run. Under **Artifacts**, download
   `T-Mix-macOS-DMG`.

The workflow downloads JUCE 8.0.6, configures CMake, compiles universal
Apple Silicon + Intel VST3/AU bundles, and packages them into a DMG.

## Important

- This workflow builds the source currently in this repository. It does not
  magically add unfinished AI capabilities.
- The DMG is not Developer ID signed or Apple-notarized. Gatekeeper may warn
  when opening the installer. Public distribution requires signing and
  notarization with an Apple Developer account.
- The first build may fail if the source has compile errors or if the chosen
  JUCE version exposes API incompatibilities. Read the failed build log;
  it gives the exact compiler error.
- FL Studio should use the VST3 copy. AU is included for other compatible hosts.
