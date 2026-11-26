# A5 at be14573: divergent routines by cause (validation A)

Source: the runner's out-be14573 (34 scripts). Game routines: 1528 identical, 249 divergent (96 with a divergent call, 153 access-order only), 2 unverified, 86 uncovered. Card: 87 identical, 7 divergent, 41 uncovered.

## Cause 1: the stage overlay's tile read at $8006 run as original code (fixed in terrain.c, StageTileRead)
about 45 routines, text 'to 8006: C A.. original A..' or 'C calls 8006; the original made no such call'.

## Player and Richter divergents (for L2): first defect line

| routine | script | kind | first defect |
|---|---|---|---|
| CameraModeFollow | attract_82 | MISSING | call 1 frame 20172: the original's read 1F00C9 (00) at access 14 of the call is not made |
| CameraModeVertical | attract_83 | MOVED | call 1 frame 20171: C write 1F1E6E placed at access 12, past 2 of the original's accesses it has not made first: read 1F00C9 (00) at 7 (a read the C s |
| MovePlayerDown | attract_83 | MOVED | call 1 frame 20275: C read 1F0E88 placed at access 2, past 2 of the original's accesses it has not made first: write 1F1E6E (00) at 0 (a read the C sk |
| MovePlayerLeft | attract_82 | MOVED | call 1 frame 20904: C read 1F0DD8 placed at access 4, past 2 of the original's accesses it has not made first: write 1F01E9 (69) at 0 (a read the C sk |
| MovePlayerLeftOrWall | gameover | MISSING | call 46 frame 14300: the original's read 1F01E7 (43) at access 1122 of the call is not made |
| MovePlayerRight | attract_82 | MOVED | call 1 frame 20226: C read 1F0DD8 placed at access 6, past 4 of the original's accesses it has not made first: write 1F1E7E (00) at 0 (a read the C sk |
| MovePlayerRightOrWall | play_s0_r1 | MISSING | call 1034 frame 17885: the original's read 1F01E7 (74) at access 1017 of the call is not made |
| MovePlayerUp | attract_83 | MOVED | call 1 frame 20253: C read 1F0E30 placed at access 3, past 2 of the original's accesses it has not made first: read 1F0D28 (00) at 1 (a read the C ski |
| MovePlayerY | attract_82 | MOVED | call 1 frame 20974: C read 1F0E30 placed at access 3, past 2 of the original's accesses it has not made first: read 1F0D28 (00) at 1 (a read the C ski |
| MovePlayerYIfMoving | attract_82 | MOVED | call 1 frame 20172: C read 1F0E88 placed at access 4, past 2 of the original's accesses it has not made first: write 1F01E9 (67) at 0 (a read the C sk |
| PlayerAndCameraFrame | attract_82 | MOVED | call 1 frame 20172: C read 1F0D80 placed at access 2, past 1 of the original's accesses it has not made first: write 1F01EF (00) at 1 (a read the C sk |
| PlayerNextX | attract_82 | MOVED | call 1 frame 20226: C read 1F0D80 placed at access 3, past 2 of the original's accesses it has not made first: read 1F0C20 (00) at 1 (a read the C ski |
| PlayerWallLeft | attract_82 | EXTRA | call 1 frame 20904: C read 1F00A0; the original's next is write 1F01E7 (68) at access 0 of the call |
| PlayerWallRight | attract_82 | EXTRA | call 1 frame 20226: C read 1F00A0; the original's next is write 1F01E7 (67) at access 0 of the call |
| PlayerXFromNext | attract_82 | EXTRA | call 1 frame 20226: C read 1F0005; the original made no more accesses in the call |
| PlayerXFromNextAlt | attract_82 | EXTRA | call 1 frame 20904: C read 1F0005; the original made no more accesses in the call |
| PlayerXOnScreen | attract_83 | MOVED | call 1 frame 20171: C read 1F0D80 placed at access 1, past 1 of the original's accesses it has not made first: read 1F0DD8 (00) at 0 (a read the C ski |
| PlayerXPastMiddle | attract_82 | EXTRA | call 1 frame 20329: C read 1F0BC8; the original made no more accesses in the call |
| PlayerYFromNext | attract_83 | EXTRA | call 1 frame 20253: C read 1F0005; the original made no more accesses in the call |
| PlayerYFromNextWord | attract_83 | EXTRA | call 1 frame 20292: C read 1F0006; the original made no more accesses in the call |
| PlayerYPastMiddle | attract_83 | EXTRA | call 1 frame 20258: C read 1F0CD0; the original made no more accesses in the call |
| RichterAnimationRunning | attract_82 | EXTRA | call 1 frame 20259: C read 1F0F90; the original made no more accesses in the call |
| RichterAnimationTick | attract_82 | EXTRA | call 1 frame 20172: C read 1F0EE0; the original made no more accesses in the call |
| RichterAnimationWait | attract_82 | EXTRA | call 1 frame 20259: C read 1F01EB; the original made no more accesses in the call |
| RichterApplyMoveAndPush | gameover | EXTRA | call 1 frame 12809: C read 1F0000; the original made no more accesses in the call |
| RichterApplyMoveAndPushGoTo | attract_82 | DIVERGENT | to 8006: C A6E X00 Y05 P00, original AD0 X01 Y02 P80 |
| RichterAttack | attract_82 | EXTRA | call 1 frame 20258: C write 1F0800; the original's next is write 1F01EA (B6) at access 44 of the call |
| RichterCheckItemHearts | attract_82 | MOVED | call 1 frame 20172: C read 1F0093 placed at access 1, past 1 of the original's accesses it has not made first: write 1F1BE0 (00) at 0 (a read the C sk |
| RichterCheckTurnWindow | attract_82 | MOVED | call 1 frame 20172: C read 1F0AC0 placed at access 6, past 3 of the original's accesses it has not made first: read 1F07A8 (00) at 3 (a read the C ski |
| RichterClimbStart | attract_85 | MOVED | call 1 frame 20571: C read 1F0CD0 placed at access 28, past 7 of the original's accesses it has not made first: write 1F01EC (03) at 16 (a read the C  |
| RichterColumnOffset | attract_82 | EXTRA | call 1 frame 20172: C read 1F0AC0; the original's next is read 0DBB4D (00) at access 1 of the call |
| RichterColumnsBlocked | attract_84 | EXTRA | call 94 frame 20265: C read 1F1BC4; the original made no more accesses in the call |
| RichterCrouch | attract_83 | MISSING | call 1 frame 23535: the original's read 1F0EE0 (0C) at access 23 of the call is not made |
| RichterCrouchAttack | attract_83 | EXTRA | call 1 frame 23550: C write 1F0800; the original's next is write 1F01EA (B6) at access 41 of the call |
| RichterCrouchStep0 | gameover | EXTRA | call 1 frame 12812: C read 1F0000; the original made no more accesses in the call |
| RichterCrouchStep1 | play_s0_r1 | EXTRA | call 1 frame 16131: C read 1F0F90; the original's next is write 1F01E6 (B6) at access 720 of the call |
| RichterCrouchStep3 | gameover | EXTRA | call 1 frame 12810: C read 1F0F90; the original's next is write 1F01E6 (B6) at access 854 of the call |
| RichterCrouchStep4 | gameover | EXTRA | call 1 frame 14253: C read 1F0F90; the original's next is write 1F01E6 (B6) at access 735 of the call |
| RichterFaceByPad | attract_82 | EXTRA | call 1 frame 20258: C read 1F00C0; the original's next is write 1F0AC0 (00) at access 2 of the call |
| RichterFloor | attract_82 | EXTRA | call 1 frame 21001: C read 1F1BD3; the original's next is write 1F01EB (66) at access 11 of the call [in the call of PlayerAndCameraFrame] |
| RichterFloorCheck | attract_82 | MOVED | call 1 frame 20172: C read 1F00C9 placed at access 2, past 2 of the original's accesses it has not made first: read 1F0B70 (00) at 0 (a read the C ski |
| RichterFrame | attract_82 | VALUE | call 1 frame 20172: C wrote 1F01EF with A0, the original's write 1F01EF (01) |
| RichterHasNoFooting | attract_84 | MISSING | call 298 frame 21420: the original's read 1F00C9 (00) at access 11 of the call is not made |
| RichterHitWhilePushed | attract_82 | MOVED | call 1 frame 20172: C read 1F1EB6 placed at access 56, past 1 of the original's accesses it has not made first: write 1F01ED (79) at 53 (a read the C  |
| RichterItemCrashFive | attract_8E | EXTRA | call 1 frame 23451: C write 1F01EC; the original made no more accesses in the call |
| RichterJumpOrClimb | play_s0_r2 | EXTRA | call 75 frame 20844: C read 1F0CD0; the original made no more accesses in the call |
| RichterKnockBack | attract_82 | EXTRA | call 1 frame 20987: C read 1F0AC0; the original made no more accesses in the call |
| RichterLadderCheck | attract_87 | MOVED | call 1 frame 21341: C read 1F0C78 placed at access 3, past 1 of the original's accesses it has not made first: read 1F1EB6 (00) at 1 (a read the C ski |
| RichterLandOnFloor | attract_82 | EXTRA | call 1 frame 20172: C read 1F00CA; the original made no more accesses in the call |
| RichterLatchButtons | attract_84 | EXTRA | call 1 frame 20970: C read 1F1BDF; the original's next is read 1F00C1 (00) at access 0 of the call |
| RichterLeapClearingOthers | attract_82 | EXTRA | call 1 frame 23755: C read 1F0AC0; the original made no more accesses in the call |
| RichterLedgeCheck | gameover | MISSING | call 1 frame 12809: the original's write 1F01EA (BA) at access 210 of the call is not made |
| RichterLiftByColumn | attract_82 | MOVED | call 1 frame 20172: C read 1F1BC8 placed at access 2, past 1 of the original's accesses it has not made first: write 1F0010 (00) at 0 (a read the C sk |
| RichterLowColumnsBlocked | attract_82 | MOVED | call 1 frame 20172: C read 1F0CD0 placed at access 12, past 6 of the original's accesses it has not made first: write 1F01E6 (BB) at 0 (a read the C s |
| RichterNoteItemPress | attract_82 | EXTRA | call 1 frame 20976: C read 1F00C1; the original made no more accesses in the call |
| RichterNudgeBJump | play_s0_r2 | EXTRA | call 1 frame 24501: C write 1F0E88; the original's next is write 1F01E7 (B6) at access 62 of the call |
| RichterReadAtOffset | attract_84 | DIVERGENT | to 8006: C A80 X00 Y00 PC0, original A00 X00 Y00 P02 |
| RichterReadBelow | attract_82 | MOVED | call 5 frame 20172: C write 1F0002 placed at access 50, past 16 of the original's accesses it has not made first: read 1F0002 (D0) at 34 (a read the C |
| RichterReadColumns | attract_82 | MOVED | call 1 frame 20172: C read 1F0CD0 placed at access 12, past 6 of the original's accesses it has not made first: write 1F01EA (BB) at 1 (a read the C s |
| RichterReadOnScreen | attract_84 | DIVERGENT | to 8006: C A80 X00 Y00 PC0, original A00 X00 Y00 P02 |
| RichterReadPad | attract_82 | MOVED | call 1 frame 20172: C read 1F1BFA placed at access 2, past 1 of the original's accesses it has not made first: write 1F00C2 (00) at 1 (a read the C sk |
| RichterReadUnderFeet | attract_82 | MOVED | call 1 frame 21002: C read 1F0CD0 placed at access 2, past 2 of the original's accesses it has not made first: read 1F0BC8 (7D) at 0 (a read the C ski |
| RichterSetAnimation | attract_82 | EXTRA | call 1 frame 20170: C read 1F0EE0; the original made no more accesses in the call |
| RichterSolidUnderColumn | attract_82 | MOVED | call 1 frame 20172: C read 1F0CD0 placed at access 11, past 6 of the original's accesses it has not made first: write 1F01E8 (AA) at 0 (a read the C s |
| RichterSpikeBelow | attract_83 | MOVED | call 1 frame 20423: C read 1F0CD0 placed at access 11, past 2 of the original's accesses it has not made first: read 1F0BC8 (58) at 9 (a read the C sk |
| RichterStand | attract_82 | MISSING | call 1 frame 20170: the original's read 1F0EE0 (01) at access 15 of the call is not made |
| RichterStandStep0 | attract_82 | EXTRA | call 1 frame 20172: C write 1F01ED; the original's next is write 1F01EC (A3) at access 131 of the call [in the call of RichterFrame] |
| RichterStandStep1 | play_s0_r1 | EXTRA | call 1 frame 14352: C read 1F0F90; the original's next is write 1F01E6 (B6) at access 728 of the call |
| RichterStandStep2 | attract_8E | EXTRA | call 1 frame 23452: C write 1F01ED; the original's next is write 1F01EC (A3) at access 103 of the call [in the call of RichterFrame] |
| RichterStart | attract_82 | MISSING | call 1 frame 20170: the original's read 1F0EE0 (01) at access 311 of the call is not made |
| RichterStartClear | attract_82 | MISSING | call 1 frame 20170: the original's read 1F0EE0 (01) at access 306 of the call is not made |
| RichterStartHealth | gameover | MISSING | call 1 frame 12804: the original's read 1F0EE0 (01) at access 309 of the call is not made |
| RichterStartNew | gameover | MISSING | call 1 frame 12804: the original's read 1F0EE0 (01) at access 314 of the call is not made |
| RichterStartStand | attract_82 | MISSING | call 1 frame 20170: the original's read 1F0EE0 (01) at access 20 of the call is not made |
| RichterStartWalk | attract_82 | MISSING | call 1 frame 20225: the original's read 1F0EE0 (00) at access 24 of the call is not made |
| RichterStateCrouch | attract_82 | EXTRA | call 1 frame 21083: C write 1F01ED; the original's next is write 1F01EC (A8) at access 85 of the call [in the call of RichterFrame] |
| RichterStateFrame | attract_82 | MOVED | call 1 frame 20172: C read 1F1BF4 placed at access 65, past 1 of the original's accesses it has not made first: write 1F01ED (7C) at 62 (a read the C  |
| RichterStateStand | attract_82 | EXTRA | call 1 frame 20172: C write 1F01ED; the original's next is write 1F01EC (A3) at access 119 of the call [in the call of RichterFrame] |
| RichterStateWalk | attract_82 | EXTRA | call 1 frame 20226: C write 1F01ED; the original's next is write 1F01EC (A7) at access 85 of the call [in the call of RichterFrame] |
| RichterToAir | attract_82 | MOVED | call 1 frame 20974: C write 1F01EC placed at access 42, past 2 of the original's accesses it has not made first: write 1F0EE0 (0A) at 39 (a read the C |
| RichterToFall | attract_83 | EXTRA | call 1 frame 20421: C write 1F0E88; the original made no more accesses in the call |
| RichterTryItemCrash | attract_8E | MOVED | call 2134 frame 23451: C read 0DB85B placed at access 865, past 1 of the original's accesses it has not made first: read 1F0093 (05) at 864 (a read th |
| RichterUpWhipLeft | attract_87 | EXTRA | call 1 frame 20747: C write 1F0E88; the original's next is write 1F01E7 (B6) at access 77 of the call |
| RichterUpWhipRight | play_s0_r2 | EXTRA | call 1 frame 20844: C read 1F0CD0; the original made no more accesses in the call |
| RichterUseItem | gameover | EXTRA | call 1 frame 12809: C read 1F00C1; the original's next is write 1F01E6 (BB) at access 744 of the call [in the call of RichterStandStep0] |
| RichterUseThrownItem | attract_82 | MOVED | call 1 frame 21237: C read 0DBA28 placed at access 43, past 1 of the original's accesses it has not made first: read 1F0093 (02) at 42 (a read the C s |
| RichterVecC023 | attract_82 | MISSING | call 1 frame 23755: the original's read 1F0EE0 (22) at access 1175 of the call is not made |
| RichterVecFrame | gameover | MISSING | call 1 frame 12809: the original's write 1F01E9 (4B) at access 1042 of the call is not made |
| RichterVecStart | attract_82 | MISSING | call 1 frame 20170: the original's read 1F0EE0 (01) at access 313 of the call is not made |
| RichterWall | attract_82 | MOVED | call 1 frame 20226: C read 1F0CD0 placed at access 14, past 4 of the original's accesses it has not made first: read 1F0BC8 (20) at 8 (a read the C sk |

## After the $9991 tile-read fix (3b31e17): attract_82, 100 divergent, the 64 that are not Player/Richter/Camera

Cause 1 above took attract_82 from 113 to 100 divergent (388 verdicts over twelve scripts, none worse). The families left, by first defect:

- Ending* (13 routines): exit 'A C 00 orig 82/84/87/88, P C 00 orig 80' and EndingScriptStep VALUE write 1F01EF C 00 orig 80: one exit-flags cause in EndingScriptStep's callers (check what A and P the original's script step leaves).
- Title* (TitleEntry, TitleSetUp, TitleStep): 'A C 00 orig 10'.
- Card calls from the game (CdPlayOnce/Again/Repeat, CdSearchTrack, LoadSceneScript): EXTRA read 1F00FC with the original making no more accesses: the card routine's C (card writer).
- Sound driver (SoundCommand, SoundDriverCommand, SoundStartVoice, KernelAdpcmPlay): EXTRA/MOVED in the driver (L5, card writer for ad_play).
- Routines of L8 that call a sound or jump table (BossFrameRoutinesC, CountDownThenSound84AndAppear, StepListThenLeapWithSound, StepListThenBurstAtFrame14, StepListSlowDownThenPickRandom, StepListFallThenMusicAgain): the same drivers; StepListSlowDownThenPickRandom EXTRA read 1F0E40 to look at.
- Probe and landing exits (ProbeTerrain, ProbeTerrainAlt, SettleOnLanding, StopOnLanding, VecStopOnLanding): P differs by V ($40) after the V-exit fix of a29ff94 was not in this run; rerun.
- Other: RunObjects 'C calls B13F' and StepScrollAndRunObjects 'to B13F' (L7b's RunObjects fix), ObjectHit, EnterStageState 'to E018', FrameModeDispatch, VramListFromIndex 1FE002, ProbeWalls.

| routine | first defect |
|---|---|
| AttractStep | verdict only |
| AttractWaitStart | verdict only |
| BossFrameRoutinesC | EXTRA call 1 frame 20171: C read 1F15D0; the original made no more accesses in the call |
| CdPlayAgain | EXTRA call 1 frame 21227: C read 1F00FC; the original made no more accesses in the call |
| CdPlayOnce | EXTRA call 1 frame 937: C read 1F00FC; the original made no more accesses in the call |
| CdPlayRepeat | EXTRA call 1 frame 21227: C read 1F00FC; the original made no more accesses in the call |
| CdSearchTrack | EXTRA call 1 frame 20160: C read 1F00FC; the original made no more accesses in the call |
| CountDownThenSound84AndAppear | verdict only |
| EndingCreditScrollStart |  A C 01 orig 82; P C 00 orig 80 (entry 80); |
| EndingDispatchKind |  A C 00 orig 88; P C 00 orig 80 (entry 01); |
| EndingFrame | EXTRA call 1 frame 8333: C write 1F01F1; the original made no more accesses in the call |
| EndingHoldThenScript19 |  A C 00 orig 88; P C 00 orig 80 (entry 80); |
| EndingHoldTimer |  A C 00 orig 88; P C 00 orig 80 (entry 02); |
| EndingRiseFrame31 |  A C 00 orig 87; |
| EndingRiseFrame32A |  A C 00 orig 87; |
| EndingRiseFrame32B |  A C 00 orig 88; |
| EndingRiseWithList |  A C 00 orig 87; P C 00 orig 80 (entry 80); |
| EndingScriptAdvance | EXTRA call 1 frame 8333: C read 1F007E; the original made no more accesses in the call [in the call of EndingScriptStep] |
| EndingScriptStep | VALUE call 2 frame 8333: C wrote 1F01EF with 00, the original's write 1F01EF (80) |
| EndingScrollWindowSteps |  A C 00 orig 84; P C 00 orig 80 (entry 80); |
| EndingScrollsOn |  P C 00 orig 80 (entry 80); |
| EndingSplitScrolls |  P C 00 orig 80 (entry 80); |
| EndingStartList3 |  P C 00 orig 80 (entry 80); |
| EndingStartScript18 |  P C 00 orig 80 (entry 80); |
| EndingStep | MISSING call 2 frame 8333: the original's write 1F01ED (81) at access 30728 of the call is not made |
| EnterStageState | to E018: C A82 X00 Y00 P80, original A82 X08 YFF P80 |
| FrameModeDispatch | code called at 8009 does not come back in this call, from a routine the call did not enter |
| KernelAdpcmPlay | MOVED call 11 frame 23754: C write 1F00FE placed at access 272, past 1 of the original's accesses it has not made first: write 1F01E7 (A |
| LoadSceneScript | EXTRA call 1 frame 218: C read 1F00FC; the original made no more accesses in the call |
| ModeAttract | verdict only |
| ModeBackToTitle | verdict only |
| ModeBoot | verdict only |
| ModeGameOverWait | verdict only |
| ObjectHit |  A C 80 orig 6B; Y C 08 orig 00; P C 80 orig 41 (entry 01); 1F11B0 C 80 orig 00; 1F0000 C 00 orig 07; 1F0000 C 00 orig 07; 1F12B8  |
| ProbeTerrain | verdict only |
| ProbeTerrainAlt | verdict only |
| ProbeWalls | EXTRA call 1 frame 20269: C read 1F0012; the original made no more accesses in the call [in the call of ProbeTerrain] |
| RunObjects | C calls B13F; the original made no such call |
| SaveVramToRam | verdict only |
| SceneReadSectors | verdict only |
| ScriptPlayTrack | verdict only |
| ScriptSound | verdict only |
| ScriptStep | verdict only |
| SettleOnLanding | to 800F: C A00 X21 Y04 P02, original A00 X21 Y04 P42 |
| SoundCommand | EXTRA call 10 frame 20258: C read 1F0034; the original made no more accesses in the call [in the call of RichterAttack] |
| SoundDriverCommand | EXTRA call 10 frame 20258: C write 1F01EA; the original made no more accesses in the call [in the call of RichterAttack] |
| SoundStartVoice | EXTRA call 11 frame 23754: C write 1F01E9; the original's next is write 1F01E8 (B3) at access 121 of the call [in the call of RichterFra |
| StageEnter | verdict only |
| StageLoad | verdict only |
| StepListFallThenMusicAgain | verdict only |
| StepListSlowDownThenPickRandom | EXTRA call 7 frame 21505: C read 1F0E40; the original made no more accesses in the call |
| StepListThenBurstAtFrame14 | verdict only |
| StepListThenLeapWithSound | verdict only |
| StepScrollAndRunObjects | to B13F: C AB1 X10 Y46 P80, original AB1 X10 Y46 PC0 |
| StopOnLanding | to 800F: C A00 X21 Y04 P02, original A00 X21 Y04 P42 |
| TitleEntry |  A C 00 orig 10; |
| TitleSetUp |  A C 00 orig 10; |
| TitleStep |  A C 00 orig 10; |
| VecCdPlayAgain | verdict only |
| VecCdPlayOnce | verdict only |
| VecPlayerAndCameraFrame | verdict only |
| VecSoundCommand | verdict only |
| VecStopOnLanding | to 800F: C A00 X21 Y04 P02, original A00 X21 Y04 P42 |
| VramListFromIndex | hardware write 1FE002: C AD, original 00 |
