#!/usr/bin/env python3
import os

gen_script = "/mnt/Media2/FATE-Linux/fate_tools/gen_cmake.py"

if os.path.exists(gen_script):
    with open(gen_script, "r", encoding="utf-8") as f:
        content = f.read()

    # Ensure MSVC builds convert absolute shim paths to relative CMake paths
    patch = (
        '\n# Convert absolute paths to relative paths for msvc-wine compatibility\n'
        'if(MSVC)\n'
        '  file(RELATIVE_PATH SHIMS_REL "${CMAKE_CURRENT_SOURCE_DIR}" "${SHIMS_DIR}")\n'
        '  include_directories("${SHIMS_REL}" "${SHIMS_REL}/d3d8_headers")\n'
        'endif()\n'
    )

    if "SHIMS_REL" not in content:
        if "include_directories(" in content:
            content = content.replace("include_directories(", patch + "\ninclude_directories(", 1)

    with open(gen_script, "w", encoding="utf-8") as f:
        f.write(content)
    print("[SUCCESS] Patched gen_cmake.py for relative include paths.")
