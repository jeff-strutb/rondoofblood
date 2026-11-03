#!/usr/bin/env python3
"""precommit.py -- the filing rules, checked on what a commit adds.

The game's code has no ROM address: it is loaded from the disc into RAM, in
pieces that reuse the same RAM. A routine is therefore named by where its
first byte lies on the disc (the sector of track 02, counted from the
track's first user sector as the game's own loader counts, and the offset
in that sector), and says separately where it runs:

    @routine <sector>:<offset> <Name>   e.g. @routine 0043:000 KernelEntry
    @run <bank>:<addr>                  the physical bank and logical address
    WHAT IT DOES:                       its purpose, in game terms
    HOW:                                how it does it

In C the header is the comment block directly above the definition; in the
companion assembly it is the run of `;` comment lines directly above the
label. A routine is filed by responsibility (src/rondo/<area>/,
asm/rondo/<area>/), never in a file named for a sector or address range.
The only such files allowed are the raw intake under asm/rondo/raw/, which
the audit ratchets down to nothing.

The hook judges only staged files that the commit adds or modifies, so a
backlog never blocks an unrelated commit.
"""
import os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TAG = re.compile(r'@routine\s+([0-9A-Fa-f]{4}:[0-9A-Fa-f]{3})\b')
RUN = re.compile(r'@run\s+[0-9A-Fa-f]{2}:[0-9A-Fa-f]{4}\b')
ADDR_NAME = re.compile(r'(^|[_/-])([0-9a-f]{4}[_:-]?[0-9a-f]{3}|[0-9a-f]{2}[_:-]?[0-9a-f]{4})(\.|_|$)'
                       r'|^(slice|batch|bank|chunk|sector)\w*\.(c|h|asm|s|inc)$', re.I)
CODE_DIRS = ('src/rondo/', 'asm/rondo/')
RAW_DIR = 'asm/rondo/raw/'


def git(*a):
    return subprocess.run(['git', '-C', ROOT] + list(a), capture_output=True).stdout


def header_block(lines, i, asm):
    """Collect the comment block that holds line i (the @routine line)."""
    if asm:
        lo = i
        while lo > 0 and lines[lo - 1].lstrip().startswith(';'):
            lo -= 1
        hi = i
        while hi + 1 < len(lines) and lines[hi + 1].lstrip().startswith(';'):
            hi += 1
        return '\n'.join(lines[lo:hi + 1])
    lo = i
    while lo > 0 and '/*' not in lines[lo]:
        lo -= 1
    hi = i
    while hi + 1 < len(lines) and '*/' not in lines[hi]:
        hi += 1
    return '\n'.join(lines[lo:hi + 1])


def main():
    bad = []
    staged = [p for p in git('diff', '--cached', '--name-only', '--diff-filter=ACMR', '-z').decode().split('\0') if p]
    added = set(p for p in git('diff', '--cached', '--name-only', '--diff-filter=A', '-z').decode().split('\0') if p)
    for rel in staged:
        if not rel.startswith(CODE_DIRS):
            continue
        base = os.path.basename(rel)
        if rel in added and not rel.startswith(RAW_DIR) and ADDR_NAME.search(base):
            bad.append('%s: address-named file outside %s; file by responsibility' % (rel, RAW_DIR))
        asm = rel.endswith(('.asm', '.s', '.inc'))
        if not (asm or rel.endswith(('.c', '.h'))):
            continue
        text = git('show', ':' + rel).decode('utf-8', 'replace')
        lines = text.splitlines()
        seen = {}
        for i, line in enumerate(lines):
            m = TAG.search(line)
            if not m:
                continue
            where = m.group(1).upper()
            if where in seen and not rel.startswith(RAW_DIR):
                bad.append('%s:%d: @routine %s also tagged at line %d' % (rel, i + 1, where, seen[where]))
            seen[where] = i + 1
            if rel.startswith(RAW_DIR):
                continue
            block = header_block(lines, i, asm)
            for need in ('WHAT IT DOES:', 'HOW:'):
                if need not in block:
                    bad.append('%s:%d: @routine %s has no %s' % (rel, i + 1, where, need))
            if not RUN.search(block):
                bad.append('%s:%d: @routine %s has no @run <bank>:<addr>' % (rel, i + 1, where))
    for b in bad:
        print(b)
    if bad:
        print('precommit: %d filing finding(s)' % len(bad), file=sys.stderr)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
