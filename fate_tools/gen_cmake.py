#!/usr/bin/env python3
"""Generate CMakeLists.txt for FATE from Game.vcproj (Release|Win32 config).

Usage:  python3 gen_cmake.py /mnt/Media2/FATE/FATE/Source/DEV
Writes: <DEV>/CMakeLists.txt

- Resolves source paths case-insensitively (vcproj says Direct3d/, disk says DIRECT3D/).
- Takes defines and system libs from the Release|Win32 configuration.
- Leaves DX8 SDK / FMOD 3.75 locations as CMake cache variables.
"""
import os
import sys
import xml.etree.ElementTree as ET

dev = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
tree = ET.parse(os.path.join(dev, "Game.vcproj"))
root = tree.getroot()
CFG = "Release|Win32"


def resolve(rel):
    """Case-insensitive path resolution relative to dev. Returns None if missing."""
    parts = [p for p in rel.replace("\\", "/").split("/") if p not in ("", ".")]
    cur, out = dev, []
    for p in parts:
        try:
            names = os.listdir(cur)
        except OSError:
            return None
        hit = p if p in names else next((n for n in names if n.lower() == p.lower()), None)
        if hit is None:
            return None
        out.append(hit)
        cur = os.path.join(cur, hit)
    return "/".join(out)


# --- configuration ---------------------------------------------------------
cfg = next(c for c in root.iter("Configuration") if c.get("Name") == CFG)
cl = next(t for t in cfg.iter("Tool") if t.get("Name") == "VCCLCompilerTool")
ln = next(t for t in cfg.iter("Tool") if t.get("Name") == "VCLinkerTool")

defs = [d for d in cl.get("PreprocessorDefinitions", "").split(",") if d.strip()]
libs = ln.get("AdditionalDependencies", "").split()
# libs provided by the DX8 SDK / FMOD / Steam get their own search dirs below
rel_incs = [p for p in cl.get("AdditionalIncludeDirectories", "").split(";")
            if p and not os.path.isabs(p.replace("\\", "/")) and ":" not in p]

# --- sources ---------------------------------------------------------------
sources, missing, rc = [], [], []
for f in root.iter("File"):
    rel = f.get("RelativePath", "")
    ext = rel.lower().rsplit(".", 1)[-1]
    if ext not in ("cpp", "c", "rc"):
        continue
    excluded = any(
        fc.get("Name") == CFG and any(t.get("ExcludedFromBuild") == "TRUE" for t in fc.iter("Tool"))
        for fc in f.iter("FileConfiguration")
    )
    if excluded:
        continue
    real = resolve(rel)
    if real is None:
        missing.append(rel)
        continue
    (rc if ext == "rc" else sources).append(real)

sources = sorted(set(sources))
rc = sorted(set(rc))

# --- emit ------------------------------------------------------------------
L = []
L.append("cmake_minimum_required(VERSION 3.20)")
L.append("project(Fate C CXX" + (" RC" if rc else "") + ")")
L.append("")
L.append("# Point these at your old SDKs (headers + 32-bit import libs):")
L.append('set(DX8_INC   "" CACHE PATH "dir with d3d8.h d3dx8.h dinput.h")')
L.append('set(DX8_LIB   "" CACHE PATH "dir with d3d8.lib d3dx8.lib dinput8.lib dxguid.lib")')
L.append('set(FMOD_INC  "" CACHE PATH "dir with fmod.h (FMOD 3.75)")')
L.append('set(FMOD_LIB  "" CACHE PATH "dir with fmodvc.lib (FMOD 3.75)")')
L.append("option(NO_STEAM \"Build with Steam stubbed out (needs #ifdef NO_STEAM in the code)\" ON)")
L.append("")
L.append("add_executable(Fate WIN32")
for s in sources + rc:
    L.append(f"    {s}")
L.append(")")
L.append("")
L.append("target_compile_definitions(Fate PRIVATE")
for d in defs + ["_MBCS"]:
    L.append(f"    {d}")
L.append("    $<$<BOOL:${NO_STEAM}>:NO_STEAM>")
L.append(")")
L.append("")
L.append("target_include_directories(Fate PRIVATE")
L.append("    ${CMAKE_CURRENT_SOURCE_DIR}")
for p in rel_incs:
    L.append("    ${CMAKE_CURRENT_SOURCE_DIR}/" + p.replace("\\", "/").rstrip("/"))
L.append("    ${DX8_INC} ${FMOD_INC}")
L.append(")")
L.append("")
L.append("# VS2003 defaults: wchar_t is a typedef, EH on, 32-bit, static CRT (/MT), MBCS")
L.append("if(MSVC)")
L.append("    target_compile_options(Fate PRIVATE /EHsc /Zc:wchar_t- /MT /W3 /wd4996 /permissive /Zc:strictStrings-)")
L.append("    target_link_options(Fate PRIVATE /SUBSYSTEM:WINDOWS)")
L.append("endif()")
L.append("")
L.append("target_link_directories(Fate PRIVATE ${DX8_LIB} ${FMOD_LIB}")
L.append("    ${CMAKE_CURRENT_SOURCE_DIR}/../steam/lib)")
L.append("target_link_libraries(Fate PRIVATE")
for lib in libs:
    L.append(f"    {lib}")
L.append(")")
L.append("")
L.append('set_target_properties(Fate PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../gamefiles")')

with open(os.path.join(dev, "CMakeLists.txt"), "w") as fh:
    fh.write("\n".join(L) + "\n")

print(f"sources: {len(sources)}  rc: {len(rc)}  defines: {defs}")
print(f"libs: {libs}")
if missing:
    print(f"MISSING ({len(missing)}):")
    for m in missing:
        print("   ", m)
