# L8: what is left, and rechecks

Last landing of this branch: aa490b0 (about 240 of 249 routines). Image byte exact at every landing.

## Not written

| routine | bytes | why it is left |
|---|---|---|
| 0F6D:324 | 51 | calls the stage vector $8006 (0F5D:006, a 3-byte JMP entry; row 05BD:006 of lot L1 with 12 copies); the master gave 0F5D:006 to L8 after the 17:00 reset; write it first |
| 0F6C:024 | 177 | calls the same vector; mostly untraced |
| 0F6C:4A4 | 47 | calls 0F6D:324; its arms past the first calls ran in no script |
| 0F5D:006 | 3 | the vector itself (above) |
| 05C9:05E, 05C9:079, 1619:080, 0C0, 0D1, 0E2, 097, 18D9:05E | about 130 | pieces of the whole 05C9:000 (copies at 1619:000 and 18D9:000): L7b lifts 05C9:000 with its pieces |

## Rechecks (written and landed, but matched only with wrapper callees)

In through mode these differ inside a callee that is not in this lot: the terrain lookup of the stage overlay, the sound driver's ad_play, or a jump through a table to arbitrary code for random states. With `--callees wrappers` all cases match, except the first line.

- 0B4B:7CC MoveXAndProbeWallBounce: 4 of 686 cases differ in V after the terrain probe (open; sent to L2).
- Terrain probes: 0B4B:77C FallToGroundAt98, 0B4B:76E, 0B4B:7B6, 0B4B:79B, 0B4C:034, 0B4C:15F, 0B4C:3B8, 0B4C:46F, 0EBE:768.
- Sound driver: 025B:4FF, 025C:2EA, 05CB:2A6, 0B4C:0EF, 0B4C:447.
- Table jumps: 025B:471, 0F6B:38E, 0B4B:556, 0F6B:4C9, 0B4C:376, 05CB:13F, 0F6C:39C.

## Arms no script ran

See l8-untraced-arms.md: C written from the decoded bytes, not compared. Needs a unit call or a poke.

## Small open points

- A few exotic arms in 025B:266, 025B:761 and 0F6B:395 are not reached by the generated cases.
- Names to request from the typed access owner: bare zero page bytes $D9, $DA (0B4A:608, 39F, 025C:7ED), the byte $3BF0, $26F3 and $26F4, $3BC3.

## Handed over by L4, not taken (master's plan changed)

0255:0DB, 0255:3C5, 0255:7A5 (alternate entry of 0515:797), 0256:03D, 0256:14C with 0256:16B, 0256:1B1: L4 has not begun them; the master did not give them to L8.
