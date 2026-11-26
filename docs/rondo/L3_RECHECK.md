# L3: routines divergent or only partly checked

All are landed, image byte exact. "Through" = liftcmp default; "wrappers" = `--callees wrappers`. A5 runs were on stage_10 unless named.

## Divergent in A5 (not by L3's own code)

| routine | file | what |
|---|---|---|
| StepScrollAndRunObjects (024E:24F, 23 copies) | system/stage_frame.c | A5: 13,253 calls identical (RunObjects resident); 6,719 divergent with "C calls 0000". gdb on a breakpoint at call_original with address 0 shows the caller is RunObjects' `jump_shim` (RunObjects -> jump_shim -> call_original), not this routine. Waits on RunObjects (L7b). liftcmp: wrappers mode 399 of 399. |
| RunStageFrame (024E:051) | system/stage_main.c | A5: identical up to the point RunObjects makes the same zero-address call (764 calls unverified, 2 identical, 189 divergent). Waits on RunObjects. |
| SetUpStageObjects (W variant, 18CE:25F) | system/stage_objects.c | A5 call 1 diverges with the same zero-address call (it ends in StepScrollAndRunObjects). Other variants uncovered. |

## Compared only up to a harness limit (liftcmp), needs A5

- ProbeTileSideBit1 (0F6D:324), DroppingPieceStep (0F6C:024), WalkAndTurnAtEdge (0F6C:4A4): the call of the vector at $8006 resolves to another overlay's vector in the harness; events agree up to it (DroppingPieceStep 369 of 369 outside it; WalkAndTurnAtEdge 116 of 116 with wrappers). Cover: attract_89.
- ClearStageBitRowsLong (05BF:2DF): agrees up to its call of SetVramAddressesLong at $968F, which the harness stubs; the other two entries 400 of 400.
- EnterStageState (024E:02B): the 127 cases that call the System Card routine at $E018 are stubbed on the lifted side; the rest match.
- StepStageFrame/Long (36D), RunStageFrame: through mode stubs their callees; A5 identical on 13,537 and 6,718 calls for the frame tick.
- BlendStageStockRows, BlendStageScoreAddRows, BlendStageScoreRows (024F:151, 181, 1AC): all differences are the cases with nothing resident at $8E87 (draft picks SetStageTileVramAddressLong); A5 identical on 13,429 / 13,429 / 13,433 calls.
- StepStageEnding (7C4): 397 of 397 comparable; 3 cases reach untraced fade rows.
- SetUpStageObjects (25F, 22 variants): through mode agrees for the first 19,000 events, then differs inside the RunObjects tail; wrappers mode cannot pass X to the callee.
- RunStageGauge, GaugeStep (05BE:46C, 48B): agree up to the untraced jumps at $8C88 / $8CA4 (draft stub vs the C's direct tail into $8D25, same registers).
- HitTestObjectLongB (18CF:778): 125 comparable cases, 475 reach untraced bytes.

## Arms written from decoded bytes, run in no script (need a unit call or poke)

- LoadStageEntry x23 (152): skip-bit arm, zero-position arm ($F8/$F9 take-over), second-character arm (kind table at $8A32). 472 of 600 cases reach them.
- ExpandStageLists x22 (277): the destination-carry arm (`inc zTemp03`).
- StepStageEnding: fade table rows; StepStageSequence: the stock/score-add restart arm (jsr $9151, jmp $9181).
- StageTriggerObjectB (092E:2FC): negative-number arm; HitTestObject: the arms that run the handlers at $955B, $9580, $9587, $9593 as original code.
- StepStageFrame: the sound arms (zField8E bits) and the jump to $8C74.
- WalkAndTurnAtEdge: the arms that end at the first two answers, the near arm.
- HitStoreHandler (024F:543) has no draft; it is covered through HitTestObject only.

## Not lifted

- 160F:670 is the tail of TerrainLookupAltK (filed); 024F:67F is a byte copy of TerrainTileRead (filed).
- 024E:46C-family and the A-group twins of the gauge (jmp $8C74 reached only by the untraced 36D arm) are not filed for the group that calls SpawnStageObject; no script runs them.
