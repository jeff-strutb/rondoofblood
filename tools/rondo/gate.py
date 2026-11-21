#!/usr/bin/env python3
"""gate.py -- the whole-game run, done only when asked.

    python3 tools/rondo/gate.py [runner options]     every script, A5 and A7, into the records

This is runner.py once --sweep: every script measured at main's tip in a private tree, the records and
the ledger written and committed under the landing lock. The day-to-day certification is runner.py
alone (it measures only the scripts that reach a routine whose compiled C changed). A whole-game run
is for after a checker change that must be applied everywhere, or a doubt about the records.
It replaces the old gate.py, whose config/rondo/gate_*.csv records the runner no longer writes; those
files are superseded by config/rondo/runs/ (records.py has the rule for when a result counts).
Forced runs while the runner is up are refused by the runner's own lock.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import runner  # noqa: E402

if __name__ == "__main__":
    opts = runner.parse(["once", "--sweep"] + sys.argv[1:])
    sys.exit(runner.cycle(opts["main"], opts))
