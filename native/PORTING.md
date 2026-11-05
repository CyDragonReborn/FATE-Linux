# Native Linux port status

The native build is a platform backend and the first extracted engine modules.
It is **not a complete or playable port**. The original Win32 executable remains
an optional cross-build; its success does not validate native gameplay.

## Implemented

- SDL3 window creation, events, keyboard/mouse/text state, focus handling, and timing.
- SDL3 gamepad discovery, hotplug handling, axes with dead zones, buttons and rumble.
  Physical controller behavior still needs manual verification.
- OpenGL 2.1 indexed textured drawing, transforms, depth testing, alpha blending,
  framebuffer resizing, fullscreen changes, and image loading through stb_image.
- Left-handed projection compatible with the game's coordinate convention, with
  OpenGL clip-depth conversion.
- Original engine math, text-file/data-group parser, and weighted randomizer built
  natively. The existing header-only D3DX math is reused with numeric types that
  do not import Windows or Direct3D interfaces.
- Fixed-width engine integers on Linux, plus corrected string-position handling
  so `std::string::npos` survives a 64-bit build.
- Windows-style asset separators and case-insensitive lookup within an explicit
  data root; assets do not depend on changing the process working directory.
- Per-user SDL preference directory and atomic binary save replacement.
- Existing miniaudio-backed FSOUND implementation builds as a Linux library.

## Required before a native game executable

1. Replace `CDX8App` lifecycle/window handling with SDL and connect `CGame` frame,
   render, focus and shutdown callbacks. Wire existing menu/input code to SDL events.
2. Migrate the engine's D3D resource and render-state calls to the native renderer:
   vertex/index buffers, texture formats (including DDS and cubemaps), material
   stages, lighting, fog, shadows, render targets, skinning and mesh optimization.
   The present backend covers basic drawing only. Some existing Windows D3DX
   shims return success without implementing their operation; those cannot serve
   as working native implementations.
3. Route existing asset access/enumeration through the native asset root. Remove
   Windows locale/resource-DLL loading and use the bundled language data.
4. Connect game serialization to the native save directory and atomic writes;
   retain the existing 32-bit on-disk field widths. Validate existing saves.
5. Replace AVI/Video-for-Windows playback, DirectInput integration and remaining
   Win32 utility calls. Connect and verify native audio playback.
6. Link the complete game with the native libraries, then validate startup, menus,
   character creation, gameplay, sound, controller input, saves and shutdown.

## Verification

`ctest --preset linux-native` runs filesystem/input/engine-core tests and an
actual offscreen OpenGL render/readback test. These tests require SDL3 and Mesa;
they do not invoke Wine. The demo binary can be inspected with `file` and `ldd`
to verify its Linux ELF format and native dependencies.
