# Validation B: left for later

## Overlay $8006 vectors with no name yet

Found with the master's rule: `build/rondo/routines.csv` rows whose run address is $8006 and whose size is 3 bytes, crossed with the
six scripts' (boot, attract, menu_files, menu_names, sound_test, raster_effects) `edges.csv`. None of the rows below is reached by
those six scripts. Each is a `jmp` (opcode $4C) to the target shown, read from the disc image (`build/rondo/disc/track02.bin`).

| overlay | disc offset | jumps to | note |
|---|---|---|---|
| 050D:006 | 0x286806 | $967F | |
| 092D:006 | 0x496806 | $967F | bytes equal 050D:006 |
| 116D:006 | 0x8B6806 | $967F | bytes equal 050D:006 |
| 0C9D:006 | 0x64E806 | $9991 | bytes equal StageTileReadVector9991 (05BD:006) |
| 0DFD:006 | 0x6FE806 | $9991 | bytes equal StageTileReadVector9991 |
| 0F5D:006 | 0x7AE806 | $9991 | bytes equal StageTileReadVector9991 |
| 16BD:006 | 0xB5E806 | $9991 | bytes equal StageTileReadVector9991 |

Naming is the game's own form, Vec<Target>; the $9991 four are copies of the filed StageTileReadVector9991 (the hybrid cannot select a
routine whose bytes equal another's, see L4_RECHECK.md), the $967F three need the target's name first.

Filed with the LoadSceneScript fix: VecVramListRun (00C5:006, $8006, a jump to VramListRun at $8169), the entry boot's overlay call takes.
