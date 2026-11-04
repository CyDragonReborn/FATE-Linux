from pathlib import Path
h = Path("/mnt/Media2/FATE-Linux/fate_tools/shims/d3dx8_shim.h")
c = Path("/mnt/Media2/FATE-Linux/fate_tools/shims/d3dx8_shim.cpp")

h_text = h.read_text().replace(
    "typedef void (WINAPI *LPD3DXFILL2D)(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexScale, void* pData);",
    "typedef void (*LPD3DXFILL2D)(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexScale, void* pData);"
)
h.write_text(h_text)
print("Updated LPD3DXFILL2D to __cdecl")
