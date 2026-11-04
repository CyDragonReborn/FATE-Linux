#!/usr/bin/env python3
import os

# 1. Update CMakeLists.txt to ensure the shim directory is included globally
cmake_path = "CMakeLists.txt"
shim_dir = "/mnt/Media2/FATE-Linux/fate_tools/shims"

if os.path.exists(cmake_path):
    with open(cmake_path, "r", encoding="utf-8") as f:
        content = f.read()

    include_entry = f'include_directories("{shim_dir}")'
    if include_entry not in content:
        with open(cmake_path, "w", encoding="utf-8") as f:
            f.write(f'{include_entry}\n' + content)
        print(f"[FIXED] Added include_directories(\"{shim_dir}\") to {cmake_path}")
    else:
        print("[INFO] CMakeLists.txt already contains shim include directory.")

# 2. Ensure d3dx8_shim.cpp includes its main header at the very top
shim_cpp_path = "/mnt/Media2/FATE-Linux/fate_tools/shims/d3dx8_shim.cpp"

if os.path.exists(shim_cpp_path):
    with open(shim_cpp_path, "r", encoding="utf-8") as f:
        cpp_content = f.read()

    if '#include "d3dx8_shim.h"' not in cpp_content:
        with open(shim_cpp_path, "w", encoding="utf-8") as f:
            f.write('#include "d3dx8_shim.h"\n' + cpp_content)
        print(f"[FIXED] Injected #include \"d3dx8_shim.h\" into {shim_cpp_path}")
    else:
        print("[INFO] d3dx8_shim.cpp already includes d3dx8_shim.h.")
