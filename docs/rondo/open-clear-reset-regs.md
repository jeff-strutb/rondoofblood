# Open: X and Y that ClearScreenState and ResetGameState save (validation A)

The block moves in both routines (TAI) push Y, A and X on the stack. The C leaves X and Y as the live
registers, so the stack bytes differ from the original's wherever the C caller's registers are stale.

A5 evidence (all 34 scripts, be14573 tree): VALUE at 1F01ED (Y) and 1F01EB (X) in
- AttractStep -> AttractBegin / AttractEndCountdown -> ResetGameState: Y 04 against 00 (the original has the dispatch's
  X = doubled sub-mode, Y = the kept Y);
- ScriptStep -> ScriptStartListAndClear -> ClearScreenState: Y 05 against 03, X 02 against 00 (X 0 from
  StartListChannel0Kind2, Y 3 from ScriptStep);
- the same in EndingCommandStartList (X 0, Y 3).

Fix, tried and not landed: give both routines (kept_x, kept_y) as SoundStopAll has. The callers above then pass theirs.
The other callers (nine in game_mode.c, EndingBegin, StageLoad) need their own X and Y from their entry and from the
exits of what ran before (FrameOverlay8000, StageClearBit, ...); passing the live registers there makes every routine
above them non-proper C (landcheck proper follows callees), so the change waits for those values.

## Status (validation A, after the landing of ClearScreenStateAt and ResetGameStateAt)

Done, A5 on boot, attract, attract_82, menu_files: the AttractStep, ScriptStep and EndingStep defects at 1F01ED/EB are gone
(AttractBegin, AttractEndCountdown, TitleScreenBegin, ScriptStartListAndClear, EndingCommandStartList and EndingBegin pass
their X and Y to ClearScreenStateAt and ResetGameStateAt; ResetGameState and ClearScreenState keep the live registers for
the callers not yet traced).

Left, with evidence: ModeAttract's screens path (ModeAttract -> load_screens -> ResetGameStateAt with the registers the
overlay left): attract_82 call 8, frame 20160, the TAI's push at 1F01EF is FF in the C and 01 in the original. The registers
passed are cpu after the overlay's original code ran, so the Y the hybrid holds after the overlay at $8006 is not the
original's there; the routine that ended the overlay frame (AttractEntry/AttractStep path) is the next place to look.
Untraced callers still on the live registers: ModeAttractPlay, ModeStageLoadScreens (through load_screens), ModeStageDone,
ModeStage, ModeStageCleared's continue path, ModeScreen, ModeEnding, StageLoad.
