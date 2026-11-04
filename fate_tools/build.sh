#!/usr/bin/env bash
# Configure + build (keeps going after errors) and summarise.   Usage: ./build.sh
set -uo pipefail
TOOLS="$(cd "$(dirname "$0")" && pwd)"
DEV="$(dirname "$TOOLS")/fate_src/DEV"
cmake -S "$DEV" -B "$DEV/build" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLS/mingw-i686.cmake" -DCMAKE_BUILD_TYPE=Release || exit 1
ninja -C "$DEV/build" -k 0 2>&1 | tee "$DEV/build.log"
echo
echo "errors:  $(grep -c ' error: ' "$DEV/build.log" || true)   (log: $DEV/build.log)"
echo "files with errors:"; grep -E ' error: ' "$DEV/build.log" | sed -E 's/:[0-9]+:[0-9]+:.*//' | sort | uniq -c | sort -rn | head -25
[ -f "$(dirname "$DEV")/gamefiles/Fate.exe" ] && echo "BUILT: $(dirname "$DEV")/gamefiles/Fate.exe"
