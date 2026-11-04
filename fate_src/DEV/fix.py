import os
import re

PROJECT_ROOT = "."

HEADERS_NEEDING_D3D = [
    "vertexdefinitions.h",
    "material.h",
    "datapoint.h",
    "datalight.h",
    "path.h"
]

FILES_NEEDING_SHIM_INC = [
    "d3dx8_shim.cpp",
    "texture.cpp",
    "material.cpp",
    "cubemap.cpp",
    "subscene.cpp",
    "movie.cpp"
]

D3DX_FALLBACK_BLOCK = """
// --- Added by build fix script ---
#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT ((UINT)-1)
#endif

#ifndef D3DX_FILTER_NONE
#define D3DX_FILTER_NONE      (1 << 0)
#define D3DX_FILTER_POINT     (2 << 0)
#define D3DX_FILTER_LINEAR    (3 << 0)
#define D3DX_FILTER_TRIANGLE  (4 << 0)
#define D3DX_FILTER_BOX       (5 << 0)
#endif

#ifndef D3DXIFF_BMP
typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP         = 0,
    D3DXIFF_JPG         = 1,
    D3DXIFF_TGA         = 2,
    D3DXIFF_PNG         = 3,
    D3DXIFF_DDS         = 4,
    D3DXIFF_PPM         = 5,
    D3DXIFF_DIB         = 6,
    D3DXIFF_HDR         = 7,
    D3DXIFF_PFM         = 8,
    D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;
#endif

#ifndef D3DXMESH_SYSTEMMEM
#define D3DXMESH_SYSTEMMEM 0x010
#endif
// ---------------------------------
"""

def find_file(filename, search_root):
    """Find the full relative path of a file in the project tree."""
    for root, _, files in os.walk(search_root):
        if filename in files:
            return os.path.join(root, filename)
    return None

def inject_includes(filepath, includes_to_add):
    """Inject missing include headers at the top of the file after pragma once / header guards."""
    if not os.path.exists(filepath):
        print(f"[!] Warning: File not found: {filepath}")
        return False

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    needed_includes = [inc for inc in includes_to_add if inc not in content]
    if not needed_includes:
        print(f"[=] {filepath}: Includes already present.")
        return False

    inc_str = "\n".join(needed_includes) + "\n"

    if "#pragma once" in content:
        new_content = content.replace("#pragma once", f"#pragma once\n\n{inc_str}", 1)
    else:
        new_content = inc_str + content

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"[+] Updated {filepath} with includes: {needed_includes}")
    return True

def ensure_d3dx_constants(search_root):
    """Ensure missing D3DX constants are present in d3dx8.h or d3dx8_shim.h."""
    target_path = find_file("d3dx8.h", search_root) or find_file("d3dx8_shim.h", search_root)
    if not target_path:
        print("[!] Warning: Neither d3dx8.h nor d3dx8_shim.h found.")
        return

    with open(target_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    if "D3DX_DEFAULT" not in content:
        with open(target_path, 'a', encoding='utf-8') as f:
            f.write(D3DX_FALLBACK_BLOCK)
        print(f"[+] Appended missing D3DX constants to {target_path}")
    else:
        print(f"[=] {target_path} already contains D3DX constants.")

def main():
    print("--- Applying Direct3D & Header Fixes ---")

    d3d_includes = [
        '#include <d3d8.h>',
        '#include <d3d8types.h>',
        '#include "d3dx8.h"'
    ]
    for header in HEADERS_NEEDING_D3D:
        path = find_file(header, PROJECT_ROOT)
        if path:
            inject_includes(path, d3d_includes)

    shim_includes = ['#include "d3dx8.h"']
    for file in FILES_NEEDING_SHIM_INC:
        path = find_file(file, PROJECT_ROOT)
        if path:
            inject_includes(path, shim_includes)

    ensure_d3dx_constants(PROJECT_ROOT)

    print("--- Script Completed. Re-run Ninja build. ---")

if __name__ == "__main__":
    main()
