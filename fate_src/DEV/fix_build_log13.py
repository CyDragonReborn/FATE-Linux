#!/usr/bin/env python3
import os

cmake_path = "CMakeLists.txt"

if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Clean up previous bad include attempt if present
    content = content.replace(
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims")\n', ''
    ).replace(
        'include_directories("Z:/mnt/Media2/FATE-Linux/fate_tools/shims/d3d8_headers")\n', ''
    )

    # Inject proper Windows path flags via force-include or CMake path conversion
    header_block = (
        '# --- MSVC Wine Cross-Compilation Include Fix ---\n'
        'if(MSVC)\n'
        '    # Force MSVC to resolve include paths using Wine Z: drive or relative path\n'
        '    add_compile_options("/I" "Z:/mnt/Media2/FATE-Linux/fate_tools/shims")\n'
        '    add_compile_options("/I" "Z:/mnt/Media2/FATE-Linux/fate_tools/shims/d3d8_headers")\n'
        'endif()\n'
        '# -----------------------------------------------\n\n'
    )

    if '# --- MSVC Wine Cross-Compilation Include Fix ---' not in content:
        content = header_block + content
    else:
        # Replace existing section with updated compiler options
        start_marker = '# --- MSVC Wine Cross-Compilation Include Fix ---'
        end_marker = '# -----------------------------------------------\n'
        start_idx = content.find(start_marker)
        end_idx = content.find(end_marker) + len(end_marker)
        if start_idx != -1 and end_idx != -1:
            content = content[:start_idx] + header_block + content[end_idx:]

    with open(cmake_path, "w", encoding="utf-8") as f:
        f.write(content)
    print("[FIXED] Applied MSVC compile options /I Z:/mnt/Media2/... to CMakeLists.txt.")
