#!/usr/bin/env python3
"""Add NO_STEAM support to the game sources (idempotent, keeps CRLF).

  python3 patch_steam.py <path to Source/DEV>

 - GAME/game.cpp:  InitializeSteam() returns true; SteamAPI_RunCallbacks / SteamAPI_Shutdown skipped
 - UTILITIES/StatsAndAchievements.{h,cpp}: Steam callbacks/interfaces compiled out, no-op stats
"""
import os
import re
import sys

dev = sys.argv[1] if len(sys.argv) > 1 else "."


def load(p):
    raw = open(p, "rb").read().decode("latin-1")
    return raw.replace("\r\n", "\n"), "\r\n" in raw


def save(p, s, crlf):
    open(p, "wb").write((s.replace("\n", "\r\n") if crlf else s).encode("latin-1"))


def find(rel):
    """case-insensitive path lookup under dev"""
    cur = dev
    for part in rel.split("/"):
        hit = next((n for n in os.listdir(cur) if n.lower() == part.lower()), None)
        if hit is None:
            return None
        cur = os.path.join(cur, hit)
    return cur


def once(s, old, new, what):
    if s.count(old) != 1:
        raise SystemExit(f"{what}: expected exactly 1 match, found {s.count(old)} -- file differs from what I saw")
    return s.replace(old, new)


# ------------------------------------------------------------- game.cpp
p = find("GAME/game.cpp")
if p:
    s, crlf = load(p)
    changed = False
    m = re.search(r"bool CGame::InitializeSteam\(\)\s*\{\n", s)
    if not m:
        raise SystemExit("game.cpp: InitializeSteam() not found")
    body_start = m.end()
    if "NO_STEAM" not in s[body_start:body_start + 200]:
        s = s[:body_start] + "#ifdef NO_STEAM\n    m_bSteamInitialized = true;\n    return true;\n#endif\n" + s[body_start:]
        changed = True
    lines = s.split("\n")
    out = []
    for i, line in enumerate(lines):
        t = line.strip()
        if t in ("SteamAPI_RunCallbacks();", "SteamAPI_Shutdown();"):
            prev = out[-1].strip() if out else ""
            if prev != "#ifndef NO_STEAM":
                out.append("#ifndef NO_STEAM")
                out.append(line)
                out.append("#endif")
                changed = True
                continue
        out.append(line)
    s = "\n".join(out)
    if changed:
        save(p, s, crlf)
        print("patched GAME/game.cpp")
    else:
        print("GAME/game.cpp already patched")
else:
    print("WARNING: GAME/game.cpp not found")

# ------------------------------------------------- StatsAndAchievements
hp, cp = find("UTILITIES/StatsAndAchievements.h"), find("UTILITIES/StatsAndAchievements.cpp")
if hp and cp:
    h, hcrlf = load(hp)
    if "NO_STEAM" in h:
        print("StatsAndAchievements already patched")
    else:
        h, n = re.subn(r"(\t//Steam Callback Declarations\n(?:\tSTEAM_CALLBACK[^\n]*\n){3})", r"#ifndef NO_STEAM\n\1#endif\n", h)
        if n != 1:
            raise SystemExit("StatsAndAchievements.h: callback declarations not found")
        h, n = re.subn(r"(\t// our GameID\n.*?ISteamUserStats \*m_pSteamUserStats;\n)", r"#ifndef NO_STEAM\n\1#endif\n", h, flags=re.S)
        if n != 1:
            raise SystemExit("StatsAndAchievements.h: Steam members not found")

        c, ccrlf = load(cp)
        init = ("\tm_pSteamUser( NULL ),\n\tm_pSteamUserStats( NULL ),\n"
                "\tm_GameID( SteamUtils()->GetAppID() ),\n"
                "\tm_CallbackUserStatsReceived( this, &CStatsAndAchievements::OnUserStatsReceived ),\n"
                "\tm_CallbackUserStatsStored( this, &CStatsAndAchievements::OnUserStatsStored ),\n"
                "\tm_CallbackAchievementStored( this, &CStatsAndAchievements::OnAchievementStored ),\n")
        c = once(c, init, "#ifndef NO_STEAM\n" + init + "#endif\n", "ctor init list")
        body = "\tm_pSteamUser = SteamUser();\n\tm_pSteamUserStats = SteamUserStats();\n"
        c = once(c, body, "#ifndef NO_STEAM\n" + body + "#endif\n", "ctor body")
        c = once(c, "void CStatsAndAchievements::Initialize()\n", "#ifndef NO_STEAM\nvoid CStatsAndAchievements::Initialize()\n", "Initialize start")
        end = "\t\tm_bRequestedStats = bSuccess;\n\t}\n}\n"
        c = once(c, end, end + "#else\nvoid CStatsAndAchievements::Initialize()\n{\n\tm_bRequestedStats = true;\n}\n#endif\n", "Initialize end")
        c = once(c, "void CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n",
                 "#ifndef NO_STEAM\nvoid CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n", "Unlock start")
        end = "\t// Store stats end of frame\n\tm_bStoreStats = true;\n}\n"
        c = once(c, end, end + "#else\nvoid CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n{\n\tachievement.m_bAchieved = true;\n}\n#endif\n", "Unlock end")
        c = once(c, "void CStatsAndAchievements::StoreStatsIfNecessary()\n", "#ifndef NO_STEAM\nvoid CStatsAndAchievements::StoreStatsIfNecessary()\n", "Store start")
        end = "\t\tm_bStoreStats = !bSuccess;\n\t}\n}\n"
        c = once(c, end, end + "#else\nvoid CStatsAndAchievements::StoreStatsIfNecessary()\n{\n\tm_bStoreStats = false;\n}\n#endif\n", "Store end")
        start = "//-----------------------------------------------------------------------------\n// Purpose: We have stats data from Steam."
        c = once(c, start, "#ifndef NO_STEAM\n" + start, "callbacks start")
        stop = "//------------------------------------------------------\n// Stats Accessors\n"
        c = once(c, stop, "#endif // NO_STEAM\n\n" + stop, "callbacks end")
        save(hp, h, hcrlf)
        save(cp, c, ccrlf)
        print("patched UTILITIES/StatsAndAchievements.h/.cpp")
else:
    print("WARNING: StatsAndAchievements files not found")
