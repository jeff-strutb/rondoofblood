# Catalog entries inside filed C (untagged callees the C inlines)

From build/rondo/routines.csv (the census's entries) against the filed spans of every @routine tag. Each is an entry the record opens as a call of its own (kind in the third column); the C that holds it makes its accesses inline, which A5 counts as EXTRA, with a MISSING at the caller. Each needs its own tag and C, called between jsr_push and jsr_pop.

| entry | bytes | kind | inside routine | its tag | file | class |
|---|---|---|---|---|---|---|
| 0053:4AF | 6 | jump | SoundStartVoice | 0053:3F0 | audio/sound_driver.c | piece of its routine: fine while A5 judges it inside |
| 0111:358 | 25 | jump | EndingEffectList | 0111:2F8 | menu/ending.c | piece of its routine: fine while A5 judges it inside |
| 00E0:3F1 | 25 | call | StartScriptSlotKind7B | 00E0:3E5 | menu/file_select.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0047:681 | 8 | call | PlayerAndCameraFrame | 0047:608 | object/camera.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 051B:7CF | 74 | jump | StepListB819 | 051B:7C4 | object/enemies_l6_stage04.c | piece of its routine: fine while A5 judges it inside |
| 01BB:69C | 21 | call | RichterSetAnimation | 01BB:67B | object/player_input.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01BC:113 | 7 | call | RichterLatchButtons | 01BB:909 | object/player_input.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01BC:13D | 5 | table | RichterUseItem | 01BC:129 | object/player_input.c | piece of its routine: fine while A5 judges it inside |
| 0720:07A | 31 | table | ReportHitUnlessTwo | 0720:076 | object/player_leaves.c | piece of its routine: fine while A5 judges it inside |
| 01BC:0F2 | 23 | call | RichterLadderCheck | 01BC:093 | object/player_terrain.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01BC:250 | 20 | call | RichterLedgeCheck | 01BC:241 | object/player_terrain.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01BC:264 | 22 | call | RichterLedgeCheck | 01BC:241 | object/player_terrain.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01B9:1B6 | 8 | call | RichterDie | 01B9:1A4 | object/richter_state.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 01B9:4E4 | 57 | jump | RichterAttack | 01B9:4CD | object/richter_state.c | piece of its routine: fine while A5 judges it inside |
| 05BE:474 | 23 | jump | RunStageGauge | 05BE:46C | object/stage_gauge.c | piece of its routine: fine while A5 judges it inside |
| 0045:533 | 8 | call | StartScriptSlotKind1 | 0045:52E | system/channels.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0049:01F | 12 | jump | LoadSceneScript | 0048:7D8 | system/disc_load.c | piece of its routine: fine while A5 judges it inside |
| 0515:6E2 | 18 | call | ScreenMapOfEntry | 0515:6DA | system/layers.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 07D5:132 | 19 | call | HalveScrollStepFromFirst | 07D5:131 | system/layers.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0043:77D | 30 | jump | RasterEffect8 | 0043:76A | system/raster.c | piece of its routine: fine while A5 judges it inside |
| 0043:7E2 | 81 | jump | RasterEffect8 | 0043:76A | system/raster.c | piece of its routine: fine while A5 judges it inside |
| 0044:527 | 11 | jump | RasterEffect20 | 0044:479 | system/raster.c | piece of its routine: fine while A5 judges it inside |
| 0111:151 | 24 | call | VramListFromTableB | 0111:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0111:169 | 100 | jump | VramListFromTableB | 0111:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0111:1CD | 52 | call | VramListFromTableB | 0111:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0111:201 | 13 | jump | VramListFromTableB | 0111:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0251:151 | 24 | call | VramListFromTableC | 0251:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0251:169 | 100 | jump | VramListFromTableC | 0251:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0251:1CD | 52 | call | VramListFromTableC | 0251:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0251:201 | 13 | jump | VramListFromTableC | 0251:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0511:151 | 24 | call | VramListFromTableD | 0511:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0511:169 | 100 | jump | VramListFromTableD | 0511:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0511:1CD | 52 | call | VramListFromTableD | 0511:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0511:201 | 13 | jump | VramListFromTableD | 0511:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 05C1:151 | 124 | call | VramListFromTableE | 05C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 05C1:1CD | 52 | call | VramListFromTableE | 05C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 05C1:201 | 13 | jump | VramListFromTableE | 05C1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0721:151 | 124 | call | VramListFromTableF | 0721:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0721:1CD | 52 | call | VramListFromTableF | 0721:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0721:201 | 13 | jump | VramListFromTableF | 0721:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 07D1:151 | 124 | call | VramListFromTableG | 07D1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 07D1:1CD | 52 | call | VramListFromTableG | 07D1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 07D1:201 | 13 | jump | VramListFromTableG | 07D1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0881:151 | 124 | call | VramListFromTableH | 0881:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0881:1CD | 52 | call | VramListFromTableH | 0881:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0881:201 | 13 | jump | VramListFromTableH | 0881:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0931:151 | 124 | call | VramListFromTableI | 0931:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0931:1CD | 52 | call | VramListFromTableI | 0931:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0931:201 | 13 | jump | VramListFromTableI | 0931:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 09E1:151 | 124 | call | VramListFromTableJ | 09E1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 09E1:1CD | 52 | call | VramListFromTableJ | 09E1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 09E1:201 | 13 | jump | VramListFromTableJ | 09E1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0A91:151 | 124 | call | VramListFromTableK | 0A91:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0A91:1CD | 52 | call | VramListFromTableK | 0A91:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0A91:201 | 13 | jump | VramListFromTableK | 0A91:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0B41:151 | 124 | call | VramListFromTableL | 0B41:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0B41:1CD | 52 | call | VramListFromTableL | 0B41:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0B41:201 | 13 | jump | VramListFromTableL | 0B41:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0BF1:151 | 124 | call | VramListFromTableM | 0BF1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0BF1:1CD | 52 | call | VramListFromTableM | 0BF1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0BF1:201 | 13 | jump | VramListFromTableM | 0BF1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0D51:151 | 24 | call | VramListFromTableN | 0D51:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0D51:169 | 100 | jump | VramListFromTableN | 0D51:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0D51:1CD | 52 | call | VramListFromTableN | 0D51:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0D51:201 | 13 | jump | VramListFromTableN | 0D51:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0E01:151 | 124 | call | VramListFromTableO | 0E01:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0E01:1CD | 52 | call | VramListFromTableO | 0E01:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0E01:201 | 13 | jump | VramListFromTableO | 0E01:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0EB1:151 | 24 | call | VramListFromTableP | 0EB1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0EB1:169 | 100 | jump | VramListFromTableP | 0EB1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0EB1:1CD | 52 | call | VramListFromTableP | 0EB1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0EB1:201 | 13 | jump | VramListFromTableP | 0EB1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 0F61:151 | 124 | call | VramListFromTableQ | 0F61:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0F61:1CD | 52 | call | VramListFromTableQ | 0F61:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 0F61:201 | 13 | jump | VramListFromTableQ | 0F61:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 1011:151 | 124 | call | VramListFromTableR | 1011:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 1011:1CD | 52 | call | VramListFromTableR | 1011:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 1011:201 | 13 | jump | VramListFromTableR | 1011:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 10C1:151 | 124 | call | VramListFromTableS | 10C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 10C1:1CD | 52 | call | VramListFromTableS | 10C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 10C1:201 | 13 | jump | VramListFromTableS | 10C1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 1171:151 | 24 | call | VramListFromTableT | 1171:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 1171:169 | 100 | jump | VramListFromTableT | 1171:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 1171:1CD | 52 | call | VramListFromTableT | 1171:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 1171:201 | 13 | jump | VramListFromTableT | 1171:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 1611:151 | 189 | call | VramListFromTableU | 1611:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 16C1:151 | 124 | call | VramListFromTableV | 16C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 16C1:1CD | 52 | call | VramListFromTableV | 16C1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 16C1:201 | 13 | jump | VramListFromTableV | 16C1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
| 18D1:151 | 124 | call | VramListFromTableW | 18D1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 18D1:1CD | 52 | call | VramListFromTableW | 18D1:142 | system/vram_update.c | NEEDS TAG (a JSR or BSR target the record opens as a call) |
| 18D1:201 | 13 | jump | VramListFromTableW | 18D1:142 | system/vram_update.c | piece of its routine: fine while A5 judges it inside |
