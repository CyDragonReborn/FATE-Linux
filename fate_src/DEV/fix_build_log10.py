#!/usr/bin/env python3
import os

cmake_path = "CMakeLists.txt"

if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Paths needed for MSVC wrapper under Wine / cross-compilation
    include_entries = [
        'include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../fate_tools/shims")',
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims")',
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims/d3d8_headers")',
    ]

    modified = False
    for entry in include_entries:
        if entry not in content:
            content = f"{entry}\n" + content
            modified = True

    if modified:
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(content)
        print("[FIXED] Updated CMakeLists.txt with Windows drive letter and relative include paths.")
    else:
        print("[INFO] CMakeLists.txt already contains necessary include paths.")
