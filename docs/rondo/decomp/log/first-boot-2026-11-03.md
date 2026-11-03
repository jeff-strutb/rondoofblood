# First boot, 2026-11-03

## System Cards

| | SHA-1 | MD5 |
|---|---|---|
| Super CD-ROM System (Japan) (En) (v3.0), the target | `79f5ff55dd10187c7fd7b8daab0b3ffbd1f56a2c` | `38179df8f4ac870017db21ebcbf53114` |
| TurboGrafx CD Super System Card (USA) (v3.0), a witness | `d02611d99921986147c753df14c7349b31d71950` | `0754f903b52e3b3342202bdafb13efa5` |

Both 262,144 B, no header, stored unreversed (the jump table reads
`4C F3 E0`; reset vector $E0F3). $FFF4 = $68, $FFF5 = $80 on both. They
differ in 11,210 bytes, in banks 0, 1, 3, 4, 22 and 23.

## Core and runner

Patch 0002 adds `rondo_bank`, `rondo_mpr` and `rondo_pc` (read only).
Clean rebuild from the patched checkout: 3.1 s; core SHA-1 prefix
`d179f939d4d8`. pcebox answers pce_cdbios, pce_arcadecard (disabled),
pce_cdspeed (1), pce_cdimagecache, pce_multitap (disabled),
pce_default_joypad_type_p1 (2 Buttons), pce_Turbo_Toggling,
pce_up_down_allowed, pce_disable_softreset, pce_psgrevision (HuC6280),
pce_nospritelimit (disabled) and pce_ocmultiplier (1). The first run left
the last two at their defaults; they were added before any result was
taken. The rest it logs as defaulted: palette, cropping, aspect, blending,
volumes, resampling, ADPCM output precision, pads 2-5, turbo, mouse.

## Boot

First script, no input, 900 frames: PC $C96A-$C976 throughout with MPR6 =
$01 (card code), CD and Super RAM empty: the card's title screen waits for
RUN. With `press RUN 6` at frame 120 (`tools/rondo/scripts/boot.txt`,
0.7 s for 900 frames):

- frame 126: base RAM $2800-$2FFF = sector 0, $3000-$37FF = sector 1.
- frame 180, MPR2-6 = $69 $6A $73 $74 $72: banks $69-$6A = sectors
  $43-$4A, $72 = $4B then $FF fill, $75-$77 = $4D-$56 then $FF fill,
  $78-$79 = $59-$5F then $FF fill: the load table's blocks exactly as
  predicted from the hand reading. Also $6B-$6C = $C1-$C8 (the kernel's
  first read of its own).
- frame 240: $6D-$74 = $C9-$E8; $78-$7C = $A1-$B1.
- frame 480, PC $8062: $78-$87 = $61-$A0 (CD RAM included); $78's $A1-$A4
  of frame 240 overwritten. Konami logo fading in (frame dump).
- frame 900, PC $43DA in bank $69: the same banks; a black frame.

Load checker control: one byte flipped in bank $69 slot 1 of the frame
180 dump turns `sector $0044` into `~$0044 2047/2048`.

USA card, same script: Super and CD RAM identical at frames 180, 480 and
900; base RAM differs by 10, 4 and 4 bytes; PC equal at 480 and 900
($8062, $43DA), $E963 against $E97A (card code) at 180.
