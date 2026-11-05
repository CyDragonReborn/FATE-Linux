# FATE-Linux

A native Linux port of the original Windows FATE source is in progress.
**The full game is not playable natively yet.** The default build now produces
native Linux platform libraries, selected original engine modules, tests, and
a platform demo. It does not use Wine, MinGW, Windows headers, or Direct3D.

## Native Linux build

On Arch Linux / CachyOS:

```sh
sudo pacman -S --needed base-devel cmake ninja sdl3 mesa libgl
bash fate_tools/build.sh
```

Or use CMake directly (3.21+ for presets):

```sh
cmake --preset linux-native
cmake --build --preset linux-native --parallel 4
ctest --preset linux-native
```

The renderer test uses SDL's offscreen driver and software Mesa, so a visible
window is unnecessary. It verifies pixels rendered by an indexed textured mesh.
The other tests cover input, asset paths, save replacement, image decoding, and
original engine parsing/math/randomization. The native FSOUND compatibility
library also builds, but audio playback has not yet been tested.

Launch the platform demo:

```sh
./build/native/bin/fate-platform-demo
```

Optionally display an original PNG, JPEG, BMP or TGA game asset:

```sh
./build/native/bin/fate-platform-demo --assets fate_src/gamefiles --image IMAGES/example.png
```

Replace `IMAGES/example.png` with an existing asset. Escape closes the demo.
This demonstrates the new platform backend, not the game's menus or gameplay.
See [native/PORTING.md](native/PORTING.md) for implemented components and remaining
engine integration work.

## Existing Windows cross-build

The Windows build remains available as a separate, optional target:

```sh
TOOLCHAIN=mingw bash fate_tools/build.sh
```

It requires `mingw-w64-gcc` on Arch and produces `build/linux-mingw/bin/Fate.exe`.
To run it, install Wine and place it beside the game data (the game sets its
working directory to the executable's directory):

```sh
cp build/linux-mingw/bin/Fate.exe fate_src/gamefiles/Fate-linux.exe
cd fate_src/gamefiles
wine Fate-linux.exe
```

Steam is disabled by default. The legacy preparation scripts are unnecessary for
this checkout. The original game assets are required for the game itself.
