#!/usr/bin/env bash
# One-shot prep: copy the source tree (if needed), fix case, patch Steam out, generate CMake.
# Safe to re-run.   Usage:  ./setup.sh [path to original Source dir]
set -euo pipefail
TOOLS="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$TOOLS")"
SRC="$ROOT/fate_src"
ORIG="${1:-/mnt/Media2/FATE/FATE/Source}"
echo "tools: $TOOLS"; echo "src:   $SRC"

echo "== checking tools"
miss=0
for t in python3 cmake ninja i686-w64-mingw32-g++ i686-w64-mingw32-gcc i686-w64-mingw32-windres; do
  command -v "$t" >/dev/null || { echo "  MISSING: $t"; miss=1; }
done
[ $miss = 0 ] || { echo "install: sudo pacman -S --needed mingw-w64-gcc cmake ninja python"; exit 1; }
SYS=/usr/i686-w64-mingw32/lib
for l in d3d8 dinput8 dxguid winmm odbc32 version; do
  [ -e "$SYS/lib$l.a" ] || echo "  note: lib$l.a not in $SYS (build may fail to link)"
done
for l in vfw32 avifil32 msvfw32; do [ -e "$SYS/lib$l.a" ] && echo "  have lib$l.a"; done

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
echo; echo "Done. Next:  $TOOLS/build.sh"
