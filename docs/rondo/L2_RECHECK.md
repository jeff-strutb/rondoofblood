# L2 routines to recheck

Routines filed in lot L2 whose check is not a clean liftcmp through mode. Each is rechecked when the callee named is re-lifted.

| routine | check made | waits on |
|---|---|---|
| CountDownThenStand (player_leaves.c) | --callees wrappers, 400 of 400 | RichterStand |
| RichterReadAtOffset (player_leaves.c) | --callees wrappers, 400 of 400 | RichterReadOnScreen |
| ProbeBoxes* and ProbeBoxesFlagged* (player_probe_boxes.c) | curated pins of the handler table (tools/rondo/curated/probe_boxes.txt); the pins assign handlers cyclically, they are not a state a trace shows, so they stand only as a stand-in until the real table (disc bytes, the same in every overlay) is pinned with attributes a trace reaches | untraced overlay handlers at 9A64 to 9ADE and 9C0F to 9C31 (no C) |
| ProbeBoxesFlagged0F60 | first piece only (no whole routine in the intake) | |
