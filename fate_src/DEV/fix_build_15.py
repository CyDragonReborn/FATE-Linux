#!/usr/bin/env python3
import os

cmake_path = "CMakeLists.txt"

if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Clean up standard include_directories attempts that expand to Unix paths
    lines = content.splitlines()
    cleaned_lines = [
        line for line in lines 
        if "fate_tools/shims" not in line or "add_compile_options" in line
    ]
    content = "\n".join(cleaned_lines)

    # Force MSVC /I flags with Wine drive letter 'Z:'
    msvc_fix = (
        "\n# --- MSVC Wine Cross-Compilation Include Fix ---\n"
        "if(MSVC)\n"
        '    add_compile_options("/IZ:/mnt/Media2/FATE-Linux/fate_tools/shims")\n'
        '    add_compile_options("/IZ:/mnt/Media2/FATE-Linux/fate_tools/shims/d3d8_headers")\n'
        "endif()\n"
        "# -----------------------------------------------\n\n"
    )

    if "# --- MSVC Wine Cross-Compilation Include Fix ---" not in content:
        content = msvc_fix + content

    with open(cmake_path, "w", encoding="utf-8") as f:
        f.write(content)
    print("[FIXED] Added explicit /IZ:/... compiler options for Wine MSVC.")