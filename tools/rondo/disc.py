#!/usr/bin/env python3
"""Split the reference disc and check every data sector.

Reads the cue sheet, places every track on the disc (absolute LBA), and for
each MODE1/2352 track checks every sector from INDEX 00: the 12-byte sync
pattern, the header's address against the sector's place on the disc, the
mode byte and the EDC. The user data (2048 B a sector, from INDEX 01) goes
to build/rondo/disc/trackNN.bin, and a manifest of every track to
build/rondo/disc/tracks.csv.

    python3 tools/rondo/disc.py            check and extract
    python3 tools/rondo/disc.py --check    check only

Exit 1 on any bad sector or a reference file whose hash is not the pinned one.
"""
import csv
import hashlib
import os
import re
import sys

TOP = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REF = os.path.join(TOP, "references", "Akumajou Dracula X - Chi no Rondo (Japan)")
CUE = "Akumajou Dracula X - Chi no Rondo (Japan).cue"
OUT = os.path.join(TOP, "build", "rondo", "disc")

# SHA-1 of every reference file: the Redump set, 22 tracks and its cue.
PINNED = {
    CUE: "ebb1350121815493f7179556ca81b8a904a4b05f",
    "Track 01": "db237c0efed295e8c2b92707de56c30d3c765047",
    "Track 02": "c9b092fb15af2a7436f425bf29ed5d948f9c9640",
    "Track 03": "d224c6c2a4a3aa27faaf89406455c82eaaa56b4e",
    "Track 04": "2734a4320856c33b6b48337fe95b8b2d2d3a8900",
    "Track 05": "ba0f0a81ccafa10f98701cac3bf730e926dde169",
    "Track 06": "62fe954a65a30a86aaa9b9ab0d491a06d9b44d99",
    "Track 07": "1e60fe3dfb31f6a2e03415bfec227c4607e4321b",
    "Track 08": "774a35debb93f99d9d31a9799096bb6a48cb1d10",
    "Track 09": "e9fbb84495168de8aefdb68eff9d58f799d35f31",
    "Track 10": "885a5f1be6d51b6c53feed153a501e523281eb3b",
    "Track 11": "f9ba6b206454a7fe171ce661dc9e61d2a0bf41a0",
    "Track 12": "01fdb752eada21a0b868713bd4ef71a9e62c5c8b",
    "Track 13": "6ea07d12871d9321ad8d06e0833add822572e8db",
    "Track 14": "2fdcb5699e87500815a3c0ca06b2463e0448b3c0",
    "Track 15": "1fb871710fb4e39bbf906b05461063790ede4a53",
    "Track 16": "a07d99a9e9efbafcd7d3d6a3508459d327b56613",
    "Track 17": "52f99af778d63f17f583333ddf5e8c0fbe54eedf",
    "Track 18": "83e81c58fa3a863c97a177d9a3125051aca7d08f",
    "Track 19": "ed8b404cf7dc0fb751012fd9db04dcc0914aa2ac",
    "Track 20": "b20e6e74ed06083b094c890bafa6c44e6d8d6259",
    "Track 21": "2fc52b7382fa4916253a1be2d1fba8ab92f9715d",
    "Track 22": "7edaa39557dcb62d58b5c9dfbcade46475963e57",
}

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sector import RAW, USER, SYNC, edc, msf as msf_bytes  # noqa: E402


def msf_frames(s):
    m, sec, f = (int(x) for x in s.split(":"))
    return (m * 60 + sec) * 75 + f


def parse_cue(path):
    tracks = []
    cur_file = None
    for line in open(path, encoding="ascii"):
        line = line.strip()
        m = re.match(r'FILE "(.+)" BINARY', line)
        if m:
            cur_file = m.group(1)
            continue
        m = re.match(r"TRACK (\d+) (\S+)", line)
        if m:
            tracks.append({"num": int(m.group(1)), "mode": m.group(2),
                           "file": cur_file, "index": {}})
            continue
        m = re.match(r"INDEX (\d+) (\d+:\d+:\d+)", line)
        if m:
            tracks[-1]["index"][int(m.group(1))] = msf_frames(m.group(2))
    return tracks


# The System Card images, in references/ beside the disc. Japan is the
# target (the card the game shipped against); the USA card is a witness.
CARDS = {
    "[BIOS] Super CD-ROM System (Japan) (En) (v3.0).pce":
        ("syscard3.pce", "79f5ff55dd10187c7fd7b8daab0b3ffbd1f56a2c"),
    "[BIOS] TurboGrafx CD Super System Card (USA) (v3.0).pce":
        ("syscard3u.pce", "d02611d99921986147c753df14c7349b31d71950"),
}
SYSTEM = os.path.join(TOP, "build", "rondo", "system")


def stage_cards(check_only):
    """Check the System Card images; link them where the core looks."""
    bad = 0
    for name, (core_name, pin) in CARDS.items():
        path = os.path.join(os.path.dirname(REF), name)
        if not os.path.exists(path):
            print(f"{name}: missing")
            bad += 1
            continue
        if sha1(path) != pin:
            print(f"{name}: SHA-1 differs from the pinned one")
            bad += 1
            continue
        print(f"{core_name}: {name}, SHA-1 pinned")
        if not check_only:
            os.makedirs(SYSTEM, exist_ok=True)
            link = os.path.join(SYSTEM, core_name)
            if os.path.lexists(link):
                os.remove(link)
            os.symlink(path, link)
    return bad


def sha1(path):
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    check_only = "--check" in sys.argv[1:]
    bad = 0
    cue = os.path.join(REF, CUE)
    if sha1(cue) != PINNED[CUE]:
        print(f"{CUE}: SHA-1 differs from the pinned one")
        return 1
    tracks = parse_cue(cue)
    lba = 0
    rows = []
    for t in tracks:
        path = os.path.join(REF, t["file"])
        size = os.path.getsize(path)
        if size % RAW:
            print(f"{t['file']}: size {size} is not a whole number of sectors")
            return 1
        n = size // RAW
        start = t["index"].get(1, 0)
        t.update(file_lba=lba, sectors=n, start_lba=lba + start)
        key = f"Track {t['num']:02d}"
        if key in PINNED and sha1(path) != PINNED[key]:
            print(f"{key}: SHA-1 differs from the pinned one")
            bad += 1
        row = {"track": t["num"], "mode": t["mode"], "file_lba": lba,
               "index01_lba": lba + start, "sectors": n,
               "pregap": start, "user_bytes": 0, "bad_sectors": 0}
        if t["mode"] == "MODE1/2352":
            errs = 0
            zero = 0
            tail = 0
            user = bytearray()
            with open(path, "rb") as f:
                for i in range(n):
                    s = f.read(RAW)
                    why = None
                    # The pregap after an audio track opens with silence:
                    # whole sectors of zeros, no sync. Only there.
                    if i < start and s == bytes(RAW) and zero == i:
                        zero += 1
                        continue
                    # The disc's last sector is all zeros in this dump (the
                    # one before it is good MODE1 at the right address). It
                    # is counted, kept as zeros, and allowed only there.
                    if t is tracks[-1] and i == n - 1 and s == bytes(RAW):
                        tail += 1
                        user += bytes(USER)
                        continue
                    if s[:12] != SYNC:
                        why = "sync"
                    elif s[12:15] != msf_bytes(lba + i):
                        why = f"address {s[12:15].hex()} != {msf_bytes(lba + i).hex()}"
                    elif s[15] != 1:
                        why = f"mode {s[15]}"
                    elif edc(s[:0x810]) != int.from_bytes(s[0x810:0x814], "little"):
                        why = "EDC"
                    if why:
                        errs += 1
                        if errs <= 10:
                            print(f"track {t['num']:02d} sector {i} (LBA {lba + i}): {why}")
                    if i >= start:
                        user += s[16:16 + USER]
            row["bad_sectors"] = errs
            row["user_bytes"] = len(user)
            row["user_sha1"] = hashlib.sha1(user).hexdigest()
            bad += errs
            row["zero_pregap"] = zero
            row["zero_tail"] = tail
            print(f"track {t['num']:02d} {t['mode']}: {n} sectors ({zero} zero pregap, "
                  f"{tail} zero at the disc's end), {errs} bad, "
                  f"{len(user)} user bytes from LBA {lba + start}, SHA-1 {row['user_sha1']}")
            if not check_only:
                os.makedirs(OUT, exist_ok=True)
                tmp = os.path.join(OUT, f".track{t['num']:02d}.bin")
                with open(tmp, "wb") as f:
                    f.write(user)
                os.replace(tmp, os.path.join(OUT, f"track{t['num']:02d}.bin"))
        rows.append(row)
        lba += n
    if not check_only:
        os.makedirs(OUT, exist_ok=True)
        with open(os.path.join(OUT, "tracks.csv"), "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=["track", "mode", "file_lba", "index01_lba",
                                              "sectors", "pregap", "zero_pregap", "zero_tail", "user_bytes",
                                              "bad_sectors", "user_sha1"])
            w.writeheader()
            w.writerows(rows)
    bad += stage_cards(check_only)
    print(f"{len(tracks)} tracks, {lba} sectors on the disc, {bad} problems")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
