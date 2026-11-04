#!/usr/bin/env python3
import os
import re

SHIM_HEADER_INCLUDE = '#include "d3dx8_shim.h"'  # Update path if needed

TRIGGER_PATTERNS = re.compile(
    r'\b(D3DXVECTOR[234]|D3DXMATRIX|D3DXQUATERNION|D3DX_DEFAULT|D3DXIMAGE_INFO|D3DXCreateTexture)\b'
)

modified = 0
for root, _, files in os.walk("."):
    for file in files:
        if file.endswith((".h", ".hpp")):
            filepath = os.path.join(root, file)
            with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            if SHIM_HEADER_INCLUDE in content:
                continue

            if TRIGGER_PATTERNS.search(content):
                lines = content.splitlines(True)
                insert_pos = 0
                for idx, line in enumerate(lines[:15]):
                    if line.strip().startswith("#pragma once") or line.strip().startswith("#ifndef"):
                        insert_pos = idx + 1
                        if idx + 1 < len(lines) and lines[idx+1].strip().startswith("#define"):
                            insert_pos = idx + 2
                        break

                lines.insert(insert_pos, f"\n{SHIM_HEADER_INCLUDE}\n")
                with open(filepath, "w", encoding="utf-8") as f:
                    f.writelines(lines)
                print(f"[PATCHED] {filepath}")
                modified += 1

print(f"\nFinished. Injected include into {modified} files.")
