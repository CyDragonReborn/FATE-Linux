#!/usr/bin/env bash
# Configure + build (keeps going after errors) and summarize.
#   bash build.sh            # MSVC via msvc-wine (default, recommended)
#   TOOLCHAIN=mingw bash build.sh
set -uo pipefail
TOOLS="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$TOOLS")"
DEV="$ROOT/fate_src/DEV"
TOOLCHAIN="${TOOLCHAIN:-msvc}"

if [ "$TOOLCHAIN" = msvc ]; then
  MSVC_DIR="${MSVC_DIR:-}"
  if [ -z "$MSVC_DIR" ]; then
    for d in "$ROOT/msvc" /mnt/Media2/msvc; do
      [ -x "$d/bin/x86/cl" ] && { MSVC_DIR="$d"; break; };
    done;
  fi
  [ -x "${MSVC_DIR:-/nonexistent}/bin/x86/cl" ] || { echo "msvc-wine not found (set MSVC_DIR=...)"; exit 1; }

  # Source environment setup script if available, or export paths directly
  if [ -f "$MSVC_DIR/msvchooks.sh" ]; then
    source "$MSVC_DIR/msvchooks.sh"
  else
    export PATH="$MSVC_DIR/bin/x86:$PATH"
  fi

  # Prefer the bin/x86/rc wrapper script in MSVC_DIR if present,
  # or locate rc.exe safely with double quotes handling spaces
  if [ -x "$MSVC_DIR/bin/x86/rc" ]; then
    RC_COMPILER="$MSVC_DIR/bin/x86/rc"
  else
    RC_COMPILER="$(find "$MSVC_DIR" -type f \( -name "rc.exe" -o -name "rc" \) -path "*/x86/*" | head -n 1)"
  fi

  B="$DEV/build-msvc"
  cmake -S "$DEV" -B "$B" -G Ninja \
        -DCMAKE_SYSTEM_NAME=Windows \
        -DCMAKE_SYSTEM_PROCESSOR=X86 \
        -DCMAKE_C_COMPILER="$MSVC_DIR/bin/x86/cl" \
        -DCMAKE_CXX_COMPILER="$MSVC_DIR/bin/x86/cl" \
        -DCMAKE_RC_COMPILER="$RC_COMPILER" \
        -DCMAKE_BUILD_TYPE=Release || exit 1
else
  B="$DEV/build-mingw"
  cmake -S "$DEV" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLS/mingw-i686.cmake" -DCMAKE_BUILD_TYPE=Release || exit 1
fi

ninja -C "$B" -k 0 2>&1 | tee "$DEV/build.log"
echo
echo "errors: $(grep -cE ' (error|fatal error)[: ]|error C[0-9]+' "$DEV/build.log" || true)   (log: $DEV/build.log)"
echo "files with errors:"
grep -E ' error: |fatal error|error C[0-9]+' "$DEV/build.log" | sed -E 's/[:(][0-9]+[:,)].*//' | sort | uniq -c | sort -rn | head -25
[ -f "$ROOT/fate_src/gamefiles/Fate.exe" ] && echo "BUILT: $ROOT/fate_src/gamefiles/Fate.exe"
