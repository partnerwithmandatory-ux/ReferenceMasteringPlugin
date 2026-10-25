# Reference Master

Early JUCE 8 stereo mastering VST3 and standalone prototype. Not a completed AI mastering product.

Implemented: background WAV/AIFF/FLAC reference analysis, plus MP3 through macOS CoreAudio (RMS, sample peak, crest factor, stereo correlation and side energy); tabbed width, EQ, compression and maximizer controls; mid/side width; 120 Hz and 8 kHz EQ shelves; compressor threshold/ratio/attack/release; drive and limiting; bounded RMS drive suggestion; user preset files and DAW parameter state. The reference picker derives its selectable extensions from the registered platform decoders. MP3 loading still needs confirmation in the macOS host.

Not yet implemented: AI source separation, vocal/drum/instrument balance estimation, perceived-depth processing, unlimited parametric/dynamic EQ, analog compressor models, true-peak lookahead limiting, LUFS metering, spectrum display, factory presets and reference playback. RMS is not LUFS. Apply currently suggests drive only; audition all changes. A stereo reference cannot reveal exact source levels or original mastering settings.

## Build

Requires Git, CMake 3.22+ and a C++17 compiler. CMake fetches JUCE 8.0.6. Review JUCE licensing before distribution.

Ubuntu prerequisites:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build pkg-config libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev
```

Use the existing checkout; no worktree is needed. From the repository directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
c++ -std=c++17 tests/analysis_test.cpp -o build/analysis_test
./build/analysis_test
```

For macOS use Xcode command line tools and CMake. For Windows use Visual Studio 2022 Desktop development with C++, CMake and Git (x64). Build natively for each platform. Outputs appear under `build/ReferenceMaster_artefacts`. Copy the VST3 bundle to your platform's VST3 directory and rescan your DAW. Logic requires a future AU target; Pro Tools requires AAX. Universal DAW compatibility is not established.

Insert on a stereo master bus, load a reference and play a representative section of the mix. Apply level suggests a bounded drive change. Monitor at low volume and audition. Presets store processing parameters; reference analysis is editor-session only. No API key is needed.

## Validation and next steps

The analysis test compiled with GCC 14.2 and passed (silence, mono correlation, opposite-polarity stereo and RMS checks). CMake was installed locally at `/workspace/.reference-tools/cmake/data/bin/cmake`. The cloud image lacks ALSA, X11, FreeType, fontconfig and OpenGL development packages, and sudo is unavailable. Full plugin builds and DAW loading have not been verified. Before production use: validate native builds and host/session/automation behavior; remove allocations on EQ automation; add loudness-matched auditioning, LUFS/true peak and spectral analysis; add dynamic EQ and compressor modes; then integrate licensed source-separation models with confidence indicators. This project does not implement proprietary FabFilter, SSL or iZotope algorithms.
