#!/usr/bin/env python3
"""Add NO_STEAM support to StatsAndAchievements.{h,cpp} (edits in place, keeps CRLF)."""
import re, sys, os

d = sys.argv[1]

def load(p):
    raw = open(p, 'rb').read().decode('latin-1')
    crlf = '\r\n' in raw
    return raw.replace('\r\n', '\n'), crlf

def save(p, s, crlf):
    if crlf: s = s.replace('\n', '\r\n')
    open(p, 'wb').write(s.encode('latin-1'))

def once(s, old, new, what):
    assert s.count(old) == 1, f"{what}: expected 1 match, got {s.count(old)}"
    return s.replace(old, new)

# ---------------- header ----------------
hp = os.path.join(d, 'StatsAndAchievements.h')
h, crlf = load(hp)
if 'NO_STEAM' in h:
    sys.exit("header already patched")
h, n = re.subn(r'(\t//Steam Callback Declarations\n(?:\tSTEAM_CALLBACK[^\n]*\n){3})',
               r'#ifndef NO_STEAM\n\1#endif\n', h)
assert n == 1, "callback decls"
h, n = re.subn(r'(\t// our GameID\n.*?ISteamUserStats \*m_pSteamUserStats;\n)',
               r'#ifndef NO_STEAM\n\1#endif\n', h, flags=re.S)
assert n == 1, "steam members"
save(hp, h, crlf)

# ---------------- cpp -------------------
cp = os.path.join(d, 'StatsAndAchievements.cpp')
c, crlf = load(cp)
if 'NO_STEAM' in c:
    sys.exit("cpp already patched")

init = ("\tm_pSteamUser( NULL ),\n\tm_pSteamUserStats( NULL ),\n"
        "\tm_GameID( SteamUtils()->GetAppID() ),\n"
        "\tm_CallbackUserStatsReceived( this, &CStatsAndAchievements::OnUserStatsReceived ),\n"
        "\tm_CallbackUserStatsStored( this, &CStatsAndAchievements::OnUserStatsStored ),\n"
        "\tm_CallbackAchievementStored( this, &CStatsAndAchievements::OnAchievementStored ),\n")
c = once(c, init, "#ifndef NO_STEAM\n" + init + "#endif\n", "ctor init list")

body = "\tm_pSteamUser = SteamUser();\n\tm_pSteamUserStats = SteamUserStats();\n"
c = once(c, body, "#ifndef NO_STEAM\n" + body + "#endif\n", "ctor body")

# Initialize()
c = once(c, "void CStatsAndAchievements::Initialize()\n",
         "#ifndef NO_STEAM\nvoid CStatsAndAchievements::Initialize()\n", "Initialize start")
end = "\t\tm_bRequestedStats = bSuccess;\n\t}\n}\n"
c = once(c, end, end + "#else\nvoid CStatsAndAchievements::Initialize()\n{\n\tm_bRequestedStats = true;\n}\n#endif\n",
         "Initialize end")

# UnlockAchievement()
c = once(c, "void CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n",
         "#ifndef NO_STEAM\nvoid CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n",
         "Unlock start")
end = "\t// Store stats end of frame\n\tm_bStoreStats = true;\n}\n"
c = once(c, end, end + "#else\nvoid CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )\n"
         "{\n\tachievement.m_bAchieved = true;\n}\n#endif\n", "Unlock end")

# StoreStatsIfNecessary()
c = once(c, "void CStatsAndAchievements::StoreStatsIfNecessary()\n",
         "#ifndef NO_STEAM\nvoid CStatsAndAchievements::StoreStatsIfNecessary()\n", "Store start")
end = "\t\tm_bStoreStats = !bSuccess;\n\t}\n}\n"
c = once(c, end, end + "#else\nvoid CStatsAndAchievements::StoreStatsIfNecessary()\n{\n\tm_bStoreStats = false;\n}\n#endif\n",
         "Store end")

# Steam callbacks (OnUserStatsReceived / Stored / AchievementStored)
start = ("//-----------------------------------------------------------------------------\n"
         "// Purpose: We have stats data from Steam.")
c = once(c, start, "#ifndef NO_STEAM\n" + start, "callbacks start")
stop = "//------------------------------------------------------\n// Stats Accessors\n"
c = once(c, stop, "#endif // NO_STEAM\n\n" + stop, "callbacks end")

save(cp, c, crlf)
print("patched StatsAndAchievements.h / .cpp")
