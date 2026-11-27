#!/usr/bin/env python3
"""Estimate how much distinct code the disc holds, with measured error.

Labels 256-byte chunks of track 02 from the merged trace (build/rondo/
trace/discmap.bin): a chunk at least half executed is code, one never
executed and at least half read as data is data. Scores every chunk by the
share of its bytes that decode, linearly, into instructions whose opcodes
are among those making 95% of the traced code's opcode sites. Picks the
threshold that best separates the labelled chunks, reports its accuracy,
recall and false-positive rate, applies it to every distinct non-fill
chunk on the disc, and corrects the count for recall and false positives,
with the range the false-positive rate's two-sigma error gives.

An estimate, not a census: chunks that mix code and data, and code no
trace has labelled, bound its accuracy.

    python3 tools/rondo/estimate.py
"""
import os
import sys
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import huc  # noqa: E402

CHUNK = 256


def common_opcodes(user, flags):
    """The opcodes that make 95% of the traced code's opcode sites."""
    ops = Counter(user[i] for i, v in enumerate(flags) if v & 1)
    total = sum(ops.values())
    common, seen = set(), 0
    for op, n in ops.most_common():
        common.add(op)
        seen += n
        if seen >= 0.95 * total:
            break
    return common


def score(chunk, common):
    """Share of the chunk's bytes inside a linear decode of common opcodes."""
    good = i = 0
    while i < len(chunk):
        n, name, _, _, _ = huc.decode(chunk, i, 0)
        if name is not None and chunk[i] in common and i + n <= len(chunk):
            good += n
            i += n
        else:
            i += 1
    return good / len(chunk)


def fill(chunk):
    return chunk.count(chunk[0]) == len(chunk)


def main():
    user = open(os.path.join(TOP, "build/rondo/disc/track02.bin"), "rb").read()
    flags = open(os.path.join(TOP, "build/rondo/trace/discmap.bin"), "rb").read()
    common = common_opcodes(user, flags)

    labelled = []
    for k in range(len(user) // CHUNK):
        f = flags[k * CHUNK:(k + 1) * CHUNK]
        c = user[k * CHUNK:(k + 1) * CHUNK]
        if fill(c):
            continue
        ran = sum(1 for v in f if v & 3)
        read = sum(1 for v in f if v == 4)
        if ran >= CHUNK // 2:
            labelled.append((score(c, common), True))
        elif ran == 0 and read >= CHUNK // 2:
            labelled.append((score(c, common), False))

    accuracy, threshold = max(
        (sum(1 for s, code in labelled if (s >= t) == code) / len(labelled), t)
        for t in (x / 100 for x in range(30, 100)))
    tp = sum(1 for s, code in labelled if s >= threshold and code)
    fn = sum(1 for s, code in labelled if s < threshold and code)
    fp = sum(1 for s, code in labelled if s >= threshold and not code)
    tn = sum(1 for s, code in labelled if s < threshold and not code)
    recall, fpr = tp / (tp + fn), fp / (fp + tn)
    print(f"common opcodes {len(common)}; labelled chunks {len(labelled)} "
          f"(code {tp + fn}, data {fp + tn})")
    print(f"threshold {threshold}: accuracy {accuracy:.3f}; code recall {recall:.3f}; "
          f"data false-positive rate {fpr:.4f}")

    seen = set()
    n_all = n_code = 0
    for k in range(len(user) // CHUNK):
        c = user[k * CHUNK:(k + 1) * CHUNK]
        if fill(c) or c in seen:
            continue
        seen.add(c)
        n_all += 1
        n_code += score(c, common) >= threshold
    print(f"distinct non-fill chunks on the disc {n_all}; scored as code {n_code} = "
          f"{n_code * CHUNK} B")

    kb = lambda f: (n_code - f * n_all) / (recall - f) * CHUNK / 1024
    sigma = (fpr * (1 - fpr) / (fp + tn)) ** 0.5
    print(f"corrected for recall and false positives: {kb(fpr):.0f} KB")
    print(f"range for the false-positive rate within two sigma: "
          f"{kb(fpr + 2 * sigma):.0f} to {kb(fpr - 2 * sigma):.0f} KB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
