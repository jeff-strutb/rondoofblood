#!/usr/bin/env python3
"""Image gate: rebuild the disc's data tracks from user data, compare byte for byte.

The game is the 2,048-byte user data of track 02 (20,785,152 B). This gate
encodes it back into the raw track files the reference set holds and
compares them with the originals:

    track 02  75 sectors of silence (the pregap after audio track 01), 150
              MODE1 pregap sectors of zero data, then every user sector
    track 22  the same pregap, then the first 9,999 user sectors of track
              02 again (the disc's backup copy), then one sector of zeros
              (the disc's last, unwritten in the dump)

    python3 tools/rondo/image.py [--user FILE] [--plant OFFSET]

--user takes the user image to grade (default: the one disc.py extracted;
later the one the build assembles). --plant flips one user byte first, to
prove the gate fails. Exit 1 on any differing byte.
"""
import argparse
import hashlib
import os
import sys
from concurrent.futures import ProcessPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import disc  # noqa: E402
from sector import RAW, USER, encode_mode1  # noqa: E402

SILENCE = 75
PREGAP = 225


def _encode(job):
    lba0, chunk = job
    return b"".join(encode_mode1(lba0 + k, chunk[k * USER:(k + 1) * USER])
                    for k in range(len(chunk) // USER))


def build_track(pool, file_lba, user, tail_zero=0):
    """Raw bytes of a data track starting at file_lba, from its user sectors."""
    n = len(user) // USER
    jobs = []
    step = 256
    data_lba = file_lba + PREGAP
    pre = bytes((PREGAP - SILENCE) * USER)
    jobs.append((file_lba + SILENCE, pre))
    for k in range(0, n, step):
        jobs.append((data_lba + k, user[k * USER:min(n, k + step) * USER]))
    out = bytearray(SILENCE * RAW)
    for part in pool.map(_encode, jobs):
        out += part
    out += bytes(tail_zero * RAW)
    return bytes(out)


def compare(name, built, path, pinned):
    orig = open(path, "rb").read()
    diff = sum(1 for a, b in zip(built, orig) if a != b) if built != orig else 0
    diff += abs(len(built) - len(orig))
    sha = hashlib.sha1(built).hexdigest()
    first = next((i for i, (a, b) in enumerate(zip(built, orig)) if a != b), None)
    where = "" if first is None else f", first at byte {first} (sector {first // RAW})"
    print(f"{name}: {len(built)} B built, SHA-1 {sha} "
          f"({'pinned' if sha == pinned else 'NOT the pinned one'}), "
          f"{diff} differing bytes{where}")
    return diff == 0 and sha == pinned


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--user", default=os.path.join(disc.OUT, "track02.bin"))
    ap.add_argument("--plant", type=int, default=None)
    a = ap.parse_args()
    user = bytearray(open(a.user, "rb").read())
    if len(user) != 10149 * USER:
        print(f"{a.user}: {len(user)} B, expected {10149 * USER}")
        return 1
    if a.plant is not None:
        user[a.plant] ^= 0x01
        print(f"planted: user byte {a.plant} flipped")
    tracks = {t["num"]: t for t in disc.parse_cue(os.path.join(disc.REF, disc.CUE))}
    lba = 0
    for num in sorted(tracks):
        t = tracks[num]
        t["file_lba"] = lba
        lba += os.path.getsize(os.path.join(disc.REF, t["file"])) // RAW
    ok = True
    with ProcessPoolExecutor() as pool:
        for num, src, tail in ((2, user, 0), (22, user[:9999 * USER], 1)):
            t = tracks[num]
            built = build_track(pool, t["file_lba"], bytes(src), tail)
            ok &= compare(f"track {num:02d}", built, os.path.join(disc.REF, t["file"]),
                          disc.PINNED[f"Track {num:02d}"])
    print("image: " + ("0 differing bytes" if ok else "FAILED"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
