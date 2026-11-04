#!/usr/bin/env python3
"""Make quoted #includes resolve on a case-sensitive filesystem.

Run on a COPY of the tree:  python3 fix_case.py /path/to/copy/Source/DEV
For every include that only fails because of letter case, creates a symlink
with the wrong-case name pointing at the real file/dir (e.g. spawn.h -> Spawn.h,
utilities -> UTILITIES). Nothing is modified or deleted.
"""
import os
import re
import sys

root = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
inc_re = re.compile(r'#\s*include\s*"([^"]+)"')
made = set()


def link_case_variants(base, rel):
    """Walk rel from base; wherever a component only matches case-insensitively,
    symlink the wanted name to the real one. Returns True if it now resolves."""
    cur = base
    for p in os.path.normpath(rel.replace("\\", "/")).split("/"):
        if p in ("", "."):
            continue
        if p == "..":
            cur = os.path.dirname(cur)
            continue
        try:
            names = os.listdir(cur)
        except OSError:
            return False
        if p in names:
            cur = os.path.join(cur, p)
            continue
        hit = next((n for n in names if n.lower() == p.lower()), None)
        if hit is None:
            return False
        dst = os.path.join(cur, p)
        if dst not in made:
            os.symlink(hit, dst)
            made.add(dst)
        cur = os.path.join(cur, hit)
    return True


unresolved = {}
for d, _, files in os.walk(root, followlinks=False):
    for f in files:
        if not f.lower().endswith((".cpp", ".c", ".h", ".inl")):
            continue
        path = os.path.join(d, f)
        if os.path.islink(path):
            continue
        for line in open(path, errors="replace"):
            m = inc_re.search(line)
            if not m:
                continue
            inc = m.group(1).replace("\\", "/")
            if any(os.path.exists(os.path.join(b, inc)) for b in (d, root)):
                continue
            if not (link_case_variants(d, inc) or link_case_variants(root, inc)):
                unresolved[inc] = unresolved.get(inc, 0) + 1

print(f"created {len(made)} symlinks")
print(f"still unresolved (SDKs / other dirs / uncompiled code): {len(unresolved)}")
for k, v in sorted(unresolved.items(), key=lambda kv: -kv[1]):
    print(f"  {v:3d} {k}")
