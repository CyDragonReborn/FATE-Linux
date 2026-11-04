#!/usr/bin/env python3
import os

cmake_path = "CMakeLists.txt"

if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Prepend relative and Wine drive paths directly at the top of CMakeLists.txt
    header_block = (
        '# --- MSVC Wine Cross-Compilation Include Fix ---\n'
        'include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../fate_tools/shims")\n'
        'include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../fate_tools/shims/d3d8_headers")\n'
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims")\n'
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims/d3d8_headers")\n'
        '# -----------------------------------------------\n\n'
    )

    if '# --- MSVC Wine Cross-Compilation Include Fix ---' not in content:
        content = header_block + content
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(content)
        print("[FIXED] Prepend relative shim include paths to CMakeLists.txt.")
    else:
        print("[INFO] MSVC Wine include paths are already present.")
