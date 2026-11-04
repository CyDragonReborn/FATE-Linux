/*
 * d3d8_loader.cpp -- Direct3DCreate8 without needing a d3d8.lib import library.
 * Loads d3d8.dll at runtime (Wine/DXVK ships one).  Works with MSVC and MinGW.
 */
#include <d3d8.h>

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    typedef IDirect3D8* (WINAPI *Fn)(UINT);
    static HMODULE mod = LoadLibraryA("d3d8.dll");
    Fn fn = mod ? (Fn)GetProcAddress(mod, "Direct3DCreate8") : 0;
    return fn ? fn(SDKVersion) : 0;
}
