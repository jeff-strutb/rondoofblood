# Rondo of Blood handbook

What this decomp learns about the hardware, the disc, the code and the
method, one rule per finding, each linked to the dated note that measured
it. New findings go here when they are made.

## Disc

- The set is 22 tracks: 02 and 22 are MODE1/2352 data, the other 20 are
  CD-DA. Every MODE1 sector of both data tracks has good sync, header
  address, mode and EDC
  ([decomp/log/bootstrap-2026-11-03](decomp/log/bootstrap-2026-11-03.md)).
- Each data track's pregap (225 sectors, after an audio track) opens with
  75 sectors of all zeros, no sync, then 150 MODE1 sectors of zero data.
- The disc's last sector (track 22, LBA 221,261) is 2,352 zero bytes in
  the dump; the sector before it is good. It is kept as zeros, allowed
  only there.
- Track 22 is a copy of track 02's first 9,999 user sectors (its 10,000th
  is the zero sector above, and track 02's is zeros too). Track 02 is the
  target.
- Both data tracks rebuild byte for byte from track 02's user data by
  re-encoding every sector (sync, header, EDC, P and Q parity). The
  bit-exact target is the user data; the sector layer adds nothing.

## Code

- Boot block (user sector 1): load 1 sector from sector 2 to $3000, run
  at $3000, MPR2-6 = $80-$84. Title field "SAMPLE PROGRAM", as Hudson's
  template left it.
- The boot program (sector 2, $3000) calls the System Card through its
  $E000 jump table. Card below 3.0, or $E0DE returning carry set: reads
  20 sectors from sector $23 to $4000 and jumps there (a separate
  program, by all appearances the old-card warning). Card 3.0 or later:
  takes the Super System Card RAM base bank from $E0DE's A and reads three
  blocks from the load table at $312F (10 sectors from $43, 12 from $4D,
  8 from $59, each to $8000 under its own MPR4-6 banks), then jumps to
  $4000. Which table record is used depends on $E0BD (not yet
  identified); the reading assumes offset 0 until a trace confirms it.
- The loader's sector numbers count from track 02's first user sector:
  sector $43 opens with `JMP $40F9` and a table of `JMP`s, the resident
  kernel's entry vectors.
- BIOS names ($E006 cd_base, $E009 cd_read, $E05A ex_getver, $E0DE
  ex_memopen) are from public System Card notes: hints until a trace
  confirms them.
- The boot confirmed in RAM: at frame 180 the load table's three blocks
  sit exactly where the hand reading put them (base bank $68 from $E0DE,
  table offset 0 from $E0BD, MPR2 = $69). The four slots read as filler
  ($4C, $57, $58, $60) are $FF sectors
  ([decomp/log/first-boot-2026-11-03](decomp/log/first-boot-2026-11-03.md)).
- The kernel loads on at once (sectors $C1-$E8 into $6B-$74 by frame 240)
  and reuses RAM for other sectors within four seconds (bank $78: $A1-$A4
  at frame 240, $61-$64 at frame 480). By frame 480 CD RAM ($80-$87)
  holds sectors $81-$A0.

## System Card

- The System Card stops on its own title screen until RUN is pressed; no
  disc sector is read before it. Every script starts `wait 120`,
  `press RUN 6`.
- After RUN the card reads user sectors 0 and 1 (the warning text and the
  boot block) to $2800-$37FF before the boot program.
- Both 3.0 cards hold $68 at $FFF4 (first Super RAM bank) and $80 at
  $FFF5 (first CD RAM bank); the boot program's fallback maps $FFF5's.
- The Japan and USA 3.0 cards differ in 11,210 bytes over banks 0, 1, 3,
  4, 22 and 23, so traces of card code are card specific. The game loads
  identical Super and CD RAM on both; base RAM differs by 4 to 10 bytes.

## Toolchain

- The HuC6280 defines 234 opcodes: the 65C02's, the Rockwell bit
  instructions and Hudson's own; the 22 others run as NOPs. `huc.py` and
  ca65 agree on all 910 forms of the control
  ([decomp/log/bootstrap-2026-11-03](decomp/log/bootstrap-2026-11-03.md)).
- ca65 refuses a constant branch target in a relocatable segment ("Range
  error (Address size 2 does not match fragment size 1)"), BNE as much as
  BBR. Branches go to labels, never to addresses.
- ca65 takes no `z:` on BBR and BBS (their operand is always zero page),
  but needs `z:` and `a:` everywhere else to fix the operand size.
- ca65 takes TMA's operand only as a single MPR bit or 0 (TAM takes any):
  any other TMA is written as `.byte $43,$nn`.
- Beetle PCE leaves the Arcade Card on and multitap on by default; a
  frontend that answers no option gets a machine the game never ran on.
  pcebox answers every option that bears on emulation and logs the rest.
- Beetle PCE's bundled zlib 1.2.11 defines `fdopen` away whenever
  `TARGET_OS_MAC` is set, which breaks the macOS SDK's own declaration;
  patch 0001 keeps only the `MACOS` test, as later zlib does.

## Method

- ASM is the bit-exact decompilation, C the behaviour-exact one, and the C
  is proper C: registers and flags appear only in each routine's `abi_`
  wrapper. Register-shaped C is a stepping stone, never counted as
  decompiled.

- Routines are named by disc location, `@routine SSSS:OOO`, with
  `@run BB:AAAA` for where they execute: overlays reuse run addresses, a
  disc location is unique. The pre-commit hook enforces both.
