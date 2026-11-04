#!/usr/bin/env python3
import os

def fix_file(filepath, header_line):
    if os.path.exists(filepath):
        with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
        if header_line not in content:
            with open(filepath, "w", encoding="utf-8") as f:
                f.write(f'{header_line}\n' + content)
            print(f"[FIXED] Injected `{header_line}` into {filepath}")
        else:
            print(f"[INFO] Header already present in {filepath}")

# 1. Inject d3dx8_shim.h header into movie.cpp and astar.cpp
fix_file("MEDIA/movie.cpp", '#include "d3dx8_shim.h"')
fix_file("PATHFINDING/astar.cpp", '#include "d3dx8_shim.h"')

# 2. Update CMakeLists.txt to pass relative include paths alongside absolute ones
cmake_path = "CMakeLists.txt"
if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8") as f:
        content = f.read()

    # Ensure both relative path and explicit paths are present for MSVC drive-letter shim mapping
    shims_rel_include = 'include_directories("../../../fate_tools/shims")'
    shims_abs_include = 'include_directories("/mnt/Media2/FATE-Linux/fate_tools/shims")'

    modified = False
    if shims_rel_include not in content:
        content = f'{shims_rel_include}\n' + content
        modified = True
    if shims_abs_include not in content:
        content = f'{shims_abs_include}\n' + content
        modified = True

    if modified:
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(content)
        print("[FIXED] Updated CMakeLists.txt include directories.")
