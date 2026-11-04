#!/usr/bin/env python3
import os
import re

CMAKE_FILE = "CMakeLists.txt"
SHIM_HEADER = "d3dx8_shim.h"  # Replace with your actual shim header filename if different

def fix_cmake():
    if not os.path.exists(CMAKE_FILE):
        print(f"Error: {CMAKE_FILE} not found in the current directory.")
        return

    with open(CMAKE_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    # Check if forced include is already configured
    if "/FI" in content or SHIM_HEADER in content:
        print(f"Forced include already present in {CMAKE_FILE}.")
        return

    # Look for add_executable or add_library targets
    match = re.search(r'(add_executable|add_library)\s*\(\s*([a-zA-Z0-9_\-\$]+)', content)

    if match:
        target_name = match.group(2)
        print(f"Found build target: '{target_name}'")
    else:
        # Fallback to variable if specific target call isn't matched easily
        target_name = "${PROJECT_NAME}"
        print(f"Could not auto-detect target name, defaulting to '{target_name}'")

    cmake_snippet = f"""
# Force include DirectX/shim header across all MSVC translation units
if (MSVC)
    target_compile_options({target_name} PRIVATE /FI"{SHIM_HEADER}")
endif()
"""

    with open(CMAKE_FILE, "a", encoding="utf-8") as f:
        f.write(cmake_snippet)

    print(f"[SUCCESS] Appended MSVC forced include block to {CMAKE_FILE}")

if __name__ == "__main__":
    fix_cmake()
