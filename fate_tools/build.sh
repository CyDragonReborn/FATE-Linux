#!/usr/bin/env bash
# Configure and build on Linux.
#   bash fate_tools/build.sh          # Native Linux platform and core
#   TOOLCHAIN=mingw bash fate_tools/build.sh
#   TOOLCHAIN=msvc bash fate_tools/build.sh
set -euo pipefail
TOOLS="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$TOOLS")"
DEV="$ROOT/fate_src/DEV"
TOOLCHAIN="${TOOLCHAIN:-native}"

if [ "$TOOLCHAIN" = native ]; then
  B="$ROOT/build/native"
  cmake -S "$ROOT" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release
elif [ "$TOOLCHAIN" = msvc ]; then
  MSVC_DIR="${MSVC_DIR:-}"
  if [ -z "$MSVC_DIR" ]; then for d in "$ROOT/msvc" /mnt/Media2/msvc; do [ -x "$d/bin/x86/cl" ] && { MSVC_DIR="$d"; break; }; done; fi
  [ -x "${MSVC_DIR:-/nonexistent}/bin/x86/cl" ] || { echo "msvc-wine not found (set MSVC_DIR=...)"; exit 1; }
  export PATH="$MSVC_DIR/bin/x86:$PATH"
  B="$DEV/build-msvc"
  cmake -S "$DEV" -B "$B" -G Ninja -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_SYSTEM_PROCESSOR=X86 \
        -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_RC_COMPILER=rc -DCMAKE_BUILD_TYPE=Release || exit 1
elif [ "$TOOLCHAIN" = mingw ]; then
  B="$ROOT/build/linux-mingw"
  cmake -S "$ROOT" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLS/mingw-i686.cmake" -DCMAKE_BUILD_TYPE=Release || exit 1
else
  echo "Unknown TOOLCHAIN: $TOOLCHAIN (use native, mingw or msvc)" >&2
  exit 2
fi

cmake --build "$B" --parallel "${JOBS:-4}" 2>&1 | tee "$B/build.log"
if [ "$TOOLCHAIN" = native ]; then
  ctest --test-dir "$B" --output-on-failure
  echo "BUILT native platform demo: $B/bin/fate-platform-demo (game integration pending)"
else
  echo "BUILT: $B/bin/Fate.exe"
fi
