# Bootstrap, 2026-11-03

## Reference set

22 tracks and a cue. All 23 files hashed (SHA-1 pinned in
`tools/rondo/disc.py`). Track layout from the cue, placed by file size:

| Track | Mode | First LBA | INDEX 01 LBA | Sectors |
|---|---|---:|---:|---:|
| 01 | AUDIO | 0 | 0 | 3,665 |
| 02 | MODE1/2352 | 3,665 | 3,890 | 10,374 |
| 03-21 | AUDIO | 14,039 | | 196,998 in all |
| 22 | MODE1/2352 | 211,037 | 211,262 | 10,225 |

221,262 sectors on the disc.

## Sector check (`disc.py`, 3 s)

Track 02: 10,374 sectors, 75 all-zero pregap, 0 bad, 20,785,152 user
bytes, SHA-1 `abbfd5d62a775a4f63adbdc03a574b012790a85f`.
Track 22: 10,225 sectors, 75 all-zero pregap, 1 all-zero at the disc's
end (LBA 221,261), 0 bad, 20,480,000 user bytes.

Before the zero-sector rules, the check reported track 22 sector 10,224
"sync": a dump of it is 2,352 zero bytes; sector 10,223 has good sync and
header 49:12:10 (LBA 221,260, its place). Pregap sectors 0-74 of both
tracks are zeros; 75 on carry MODE1 headers at their places (track 02
sector 75: 00:51:65 = LBA 3,740).

Track 22 against track 02, user data: 10,000 sectors compared, 0 differ.

Negative control: a real sector passes; one data bit flipped fails the
EDC; one header address bit flipped fails the address.

## Image gate (`image.py`, 2 s on 14 cores)

Both tracks rebuilt from track 02's user data:

```
track 02: 24399648 B built, SHA-1 c9b092fb15af2a7436f425bf29ed5d948f9c9640 (pinned), 0 differing bytes
track 22: 24049200 B built, SHA-1 7edaa39557dcb62d58b5c9dfbcade46475963e57 (pinned), 0 differing bytes
```

Negative controls: `--plant 4100` (user sector 2) fails both tracks, 27
bytes differing from raw sector 227 (225 + 2); `--plant 20783109` (the
last user sector) fails track 02 at raw sector 10,373 with 32 bytes
differing and leaves track 22, which does not hold that sector, pinned.

## Boot

User sector 1, $800 into the track: `00 00 02 | 01 | 00 30 | 00 30 |
00 01 02 03 04 | 00`: boot program at sector 2, 1 sector, loaded to $3000,
run at $3000, MPR2-6 = $80-$84. Then "PC Engine CD-ROM SYSTEM", Hudson
and NEC's copyright, and the title field "SAMPLE PROGRAM". User sector 0
is Shift-JIS text.

The boot program ($3000-$312E, then its table to $3148): sets VDC
registers, clears VRAM, calls $E006 twice and $E05A; X >= 3 takes the
3.0 path:

- $E0DE; carry set falls back to the path below; otherwise its A is the
  base bank (kept at <$DB);
- $E0BD with <$F8 = 0, <$FA = 5 leaves an offset in <$FC-<$FD that
  indexes the table at $312F (not yet identified; the reading below
  assumes 0, to be confirmed by a trace);
- the table: a 24-bit value $000F75, less $000F32, is the base sector
  $43; then, through the word at $3132 ($3134), records (0, three MPR
  banks) and (1, sector offset, count) to $FF: 10 sectors from $43, 12
  from $4D, 8 from $59, each read to $8000 with MPR4-6 set to the
  record's banks plus the base bank;
- MPR2-3 set to the base bank plus 1 and 2, then `JMP $4000`.

Below 3.0, or when $E0DE returns carry set: MPR2-5 from the byte at
$FFF5 and up, 20 sectors from sector $23 read to $4000, `JMP $4000`.

Sector $43 at $4000: `JMP $40F9`, three zero bytes, then 24 `JMP`s into
$4393-$5BA0: the kernel's entry vectors. Sector $23 at $4000: a short
program calling $E069 among others. Sectors $4C, $58 and $60 start with
$FF filler; $4D starts with code-shaped bytes (`LDA <$31; ASL; TAX`).

## Toolchain (fetch approved by the maintainer)

cc65 pinned at `77b6484615427746e55fda2755c9cbc320a1fb51` and Beetle PCE
at `b96c11e095b6a40a412d2da02766ae1c3f4fd539` (both upstream master
today), fetched shallow into `tools/toolchains/src/`.

ca65 and ld65: built in 4 s (`ca65 V2.19 - Git 77b6484`).

Assembler control, first run: all 192 BBR and BBS lines refused `z:` on
their zero-page operand, and two TMA lines refused operands $5A and $FF
("Argument of TMA must be a power of two"). With those fixed, the second
run failed on 232 lines, every branch: the 192 BBR and BBS lines and the
40 plain ones (10 opcodes at 4 reaches), each "Range error (Address size
2 does not match fragment size 1)". A four-line probe showed BNE to a
constant fails the same way, and to a label passes. With branches written as
`img+$offset` from a label at the image's start:

```
910 instructions over 234 defined opcodes, 22 undefined as data; 2388 B expected, 2388 B assembled, 0 differing bytes
```

`--plant` (`tsb a:$0012` written without `a:`) assembles TSB as zero page,
moves every later byte down one and pushes 58 branches out of range: ca65
fails, exit 1.

Beetle PCE, first build: exit 2, `deps/zlib-1.2.11/zutil.o`: the SDK's
`_stdio.h` line 322 declaration of `fdopen` broken by zutil.h line 133,
which defines `fdopen(fd,mode)` as `NULL` when `TARGET_OS_MAC` is
defined. Patch `tools/toolchains/patches/pce/0001-zlib-darwin-fdopen.patch`
drops that test. Clean build from the patched checkout: 2.7 s on 14
cores, `mednafen_pce_libretro.dylib`, arm64, 54 `retro_` exports, SHA-1
prefix `d1dce514a128`, identical to the incremental build before it.
