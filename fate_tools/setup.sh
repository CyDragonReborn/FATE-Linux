#!/usr/bin/env bash
# One-shot prep: copy the source tree (if needed), fix case, patch Steam out, generate CMake.
# Safe to re-run.   Usage:  bash setup.sh [path to original Source dir]
set -euo pipefail
TOOLS="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$TOOLS")"
SRC="$ROOT/fate_src"
ORIG="${1:-/mnt/Media2/FATE/FATE/Source}"
echo "tools: $TOOLS"; echo "src:   $SRC"

echo "== checking tools"
miss=0
for t in python3 cmake ninja wine; do command -v "$t" >/dev/null || { echo "  MISSING: $t"; miss=1; }; done
[ $miss = 0 ] || { echo "install: sudo pacman -S --needed wine cmake ninja python"; exit 1; }

# MSVC (msvc-wine) is the recommended compiler: the code is VS2003-era.
MSVC_DIR="${MSVC_DIR:-}"
if [ -z "$MSVC_DIR" ]; then
  for d in "$ROOT/msvc" /mnt/Media2/msvc; do [ -x "$d/bin/x86/cl" ] && { MSVC_DIR="$d"; break; }; done
fi
if [ -n "$MSVC_DIR" ] && [ -x "$MSVC_DIR/bin/x86/cl" ]; then
  echo "  MSVC: $MSVC_DIR  ($("$MSVC_DIR/bin/x86/cl" 2>&1 | head -1))"
else
  echo "  MSVC not found. Install it (several GB):"
  echo "    git clone https://github.com/mstorsjo/msvc-wine $ROOT/msvc-wine && cd $ROOT/msvc-wine"
  echo "    ./vsdownload.py --accept-license --dest $ROOT/msvc && ./install.sh $ROOT/msvc"
fi

# d3d8 headers: the Windows SDK has none, MinGW-w64 / Wine do.
H="$TOOLS/shims/d3d8_headers"; mkdir -p "$H"
if [ ! -f "$H/d3d8.h" ]; then
  for d in /usr/i686-w64-mingw32/include /usr/x86_64-w64-mingw32/include /usr/include/wine/windows /usr/include/wine/wine/windows; do
    if [ -f "$d/d3d8.h" ] && [ -f "$d/d3d8types.h" ] && [ -f "$d/d3d8caps.h" ]; then
      cp -L "$d/d3d8.h" "$d/d3d8types.h" "$d/d3d8caps.h" "$H/"; echo "  d3d8 headers copied from $d"; break
    fi
  done
fi
[ -f "$H/d3d8.h" ] || { echo "ERROR: no d3d8.h found. sudo pacman -S mingw-w64-headers  (or put d3d8.h, d3d8types.h, d3d8caps.h in $H)"; exit 1; }

if [ ! -f "$SRC/DEV/Game.vcproj" ]; then
  echo "== copying $ORIG -> $SRC"
  [ -f "$ORIG/DEV/Game.vcproj" ] || { echo "ERROR: $ORIG/DEV/Game.vcproj not found; pass the original Source dir as argument"; exit 1; }
  mkdir -p "$SRC"; cp -r "$ORIG"/. "$SRC"/
fi
echo "== removing .svn metadata"
find "$SRC" -name .svn -type d -prune -exec rm -rf {} + 2>/dev/null || true

echo "== fixing include case"
python3 "$TOOLS/fix_case.py" "$SRC/DEV"
echo "== patching Steam out (NO_STEAM)"
python3 "$TOOLS/patch_steam.py" "$SRC/DEV"
echo "== generating CMakeLists.txt"
python3 "$TOOLS/gen_cmake.py" "$SRC/DEV" "$TOOLS/shims"
echo; echo "Done. Next:  bash $TOOLS/build.sh"
