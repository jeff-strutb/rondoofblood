/* The registers and flags a stage routine is entered with and leaves (StageRegs), for the stage writer's files. */
#ifndef RONDO_STAGE_REGS_H
#define RONDO_STAGE_REGS_H

#include "machine.h"
#include "stage_map.h"

/* The registers and flags the routine is entered with, N and Z as the flags hold them. */
static inline StageRegs regs_entry_shim(void)
{
    StageRegs r = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, 0 };

    r.nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    return r;
}

/* A load into A: N and Z from the value. */
static inline StageRegs regs_loaded(StageRegs r, uint8_t value)
{
    r.a = value;
    r.nz = value;
    return r;
}

/* A compare with A, X or Y: C from no borrow, N and Z from the difference. */
static inline StageRegs regs_compared(StageRegs r, uint8_t reg, uint8_t value)
{
    r.c = reg >= value;
    r.nz = (uint8_t)(reg - value);
    return r;
}

/* system/stage_flow.c: StageStart (024D:02D) with the section order table at order_table, for the copies whose
 * only difference is that operand. */
StageRegs StageStartAt(StageRegs r, uint16_t order_table);

/* system/stage_flow.c: StagePauseToggle (024D:1A2) with ret the address after its JSR to StagePauseSet. */
StageRegs StagePauseToggleAt(StageRegs r, uint16_t ret);

/* system/stage_flow.c: StageScrollSpeedsOff (024D:0C5) and StageCameraStep (024D:0BF) with ret the address after
 * their JSR to MapOverlayBank7. */
StageRegs StageScrollSpeedsOffAt(StageRegs r, uint16_t ret);
StageRegs StageCameraStepAt(StageRegs r, uint16_t ret);


/* system/stage_flow.c: StageChooseClearKind (024D:260) with ret the address after its JSR to StageScrollSpeedsOff
 * and scroll_off the address that JSR goes to. */
StageRegs StageChooseClearKindAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepDeath (024D:23D) with ret the address after its JSR to StageScrollSpeedsOff and
 * scroll_off the address that JSR goes to. */
StageRegs StageSubStepDeathAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepTally (024D:2BA) with ret the address after its JSR to StageScrollSpeedsOff and
 * scroll_off the address that JSR goes to. */
StageRegs StageSubStepTallyAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepFadeOut (024D:2A2) with site the address of its tail JMP to StageScrollSpeedsOff and
 * scroll_off the address that JMP goes to. */
StageRegs StageSubStepFadeOutAt(StageRegs r, uint16_t site, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepFadeOutLate (024D:2AD) with site the address of its tail JMP to StageScrollSpeedsOff and
 * scroll_off the address that JMP goes to. */
StageRegs StageSubStepFadeOutLateAt(StageRegs r, uint16_t site, uint16_t scroll_off);


/* system/stage_flow.c: StageForcedWalk and StageForcedWalkB (024D:31D, 024D:340) with init_ret the address after their
 * JSR to InitStage, init the address it goes to, and pads their table of forced pads. */
StageRegs StageForcedWalkAt(StageRegs r, uint16_t init_ret, uint16_t init, uint16_t pads);


/* system/stage_flow.c: StageFallPose (024D:373) run at `at`. */
StageRegs StageFallPoseAt(StageRegs r, uint16_t at);


/* system/stage_flow.c: StageFallRightLand (024D:380) run at `at`. */
StageRegs StageFallRightLandAt(StageRegs r, uint16_t at);


/* system/stage_flow.c: StageFallRightDone (024D:39C) with init_ret the address after its JSR to InitStage and init the
 * address it goes to. */
StageRegs StageFallRightDoneAt(StageRegs r, uint16_t init_ret, uint16_t init);


/* system/stage_flow.c: StageFallLeftDone (024D:3EC) with init_ret the address after its JSR to InitStage and init the
 * address it goes to. */
StageRegs StageFallLeftDoneAt(StageRegs r, uint16_t init_ret, uint16_t init);


/* system/stage_flow.c: StageFallLeftLand (024D:3D0) run at `at`. */
StageRegs StageFallLeftLandAt(StageRegs r, uint16_t at);


/* system/stage_flow.c: StageFallRight (024D:361) with the steps it goes on into. */
StageRegs StageFallRightAt(StageRegs r, StageRegs (*land)(StageRegs), StageRegs (*done)(StageRegs),
                           StageRegs (*pose)(StageRegs));


/* system/stage_flow.c: StageFallLeft (024D:3B0) with the steps it goes on into and its two calls' return addresses. */
StageRegs StageFallLeftAt(StageRegs r, StageRegs (*land)(StageRegs), StageRegs (*done)(StageRegs),
                          StageRegs (*pose)(StageRegs), uint16_t step_ret, uint16_t pose_ret);


/* system/stage_flow.c: StageRestartInPlace (024D:413) run at `at`, with where its five calls into the overlay go. */
StageRegs StageRestartInPlaceAt(StageRegs r, uint16_t at, uint16_t lists, uint16_t runs, uint16_t init, uint16_t spawn,
                                uint16_t setup);


/* system/stage_flow.c: StageShowStartFrame (024D:448) run at `at`, with its table of frames. */
StageRegs StageShowStartFrameAt(StageRegs r, uint16_t at, uint16_t frames);


/* system/stage_flow.c: StageClearedWaitScroll (024D:4C7) with ret the address after its JSR to StepScrollAndRunObjects
 * and step the address it goes to. */
StageRegs StageClearedWaitScrollAt(StageRegs r, uint16_t ret, uint16_t step);


/* system/stage_flow.c: StageClearedScrollOn (024D:695) with ret the address after its JSR to StepScrollAndRunObjects
 * and step the address it goes to. */
StageRegs StageClearedScrollOnAt(StageRegs r, uint16_t ret, uint16_t step);


/* system/stage_steps.c: StageWalkOutScroll (024D:48D) run at `at`, with its tables by route and where its three calls
 * go. */
StageRegs StageWalkOutScrollAt(StageRegs r, uint16_t at, uint16_t scroll_steps, uint16_t player_steps, uint16_t stop_x,
                                 uint16_t camera, uint16_t step, uint16_t scroll_off);


/* system/stage_steps.c: StageStopAndStep (024D:68C) run at `at`, with where its two calls go. */
StageRegs StageStopAndStepAt(StageRegs r, uint16_t at, uint16_t scroll_off, uint16_t step);


/* system/stage_tick.c: StageScriptByWave (024E:422) with its table of eight script numbers. */
StageExit StageScriptByWaveAt(uint16_t scripts);


/* object/stage_boss_flash.c: BossFlashMaskCell (024E:564) run at `at`, with its two VRAM calls' targets and its masks. */
StageRegs BossFlashMaskCellAt(StageRegs r, uint16_t at, uint16_t point, uint16_t and_bits, uint16_t masks);


/* object/stage_boss_flash.c: BossGaugeFlash (024E:525) run at `at`, with its table of cells and its mask's address. */
StageRegs BossGaugeFlashAt(StageRegs r, uint16_t at, uint16_t addresses, uint16_t mask_cell);


/* system/stage_tile_rows.c: BlendStageTileRows (024E:584), its code moved by `moved` from the loaded overlays', with
 * the C of its tile address setter. */
TileRowsExit BlendStageTileRowsAt(uint8_t phase, uint8_t row_a, uint8_t row_b, uint16_t moved, void (*set)(void));

/* system/stage_tile_rows.c: what a tile row blend leaves, set on the registers. */
void blend_exit_shim(TileRowsExit out);


/* system/stage_tile_rows.c: BlendStageRowPair (024F:266) run at `at`, with the C of its tile row blend. */
TileRowsExit BlendStageRowPairAt(uint8_t x, uint8_t a, uint8_t y, uint16_t at,
                                 TileRowsExit (*tile_rows)(uint8_t, uint8_t, uint8_t));


/* system/stage_tile_rows.c: BlendStageRowChanges (024F:236) run at `at`, with the C of its tile row blend, and what it
 * leaves set on the registers. */
RowChangesExit BlendStageRowChangesAt(uint8_t x, uint8_t new_byte, uint16_t at,
                                      TileRowsExit (*tile_rows)(uint8_t, uint8_t, uint8_t));
void row_changes_exit_shim(RowChangesExit out);


/* system/stage_blend_steps.c: BlendStageScoreRows (024F:1AC) run at `at`, with its banks and its row changes blend; and
 * what a blend step leaves, set on the registers. */
BlendStepEnd BlendStageScoreRowsAt(uint8_t x, const StageBanks *banks, uint16_t at,
                                   RowChangesExit (*row_changes)(uint8_t, uint8_t));
void blend_step_exit_shim(BlendStepEnd end);


/* system/stage_vram_mask.c: MaskVramRowsRandom (024F:0B1) run at `at`, with its VRAM address setter and row masks;
 * and what it leaves, set on the registers. */
VramMaskExit MaskVramRowsRandomAt(uint16_t at, void (*point)(uint8_t lo, uint8_t hi),
                                  MaskExit (*row_copy)(uint8_t x, uint8_t y), MaskExit (*row)(uint8_t x, uint8_t y));
void mask_vram_rows_exit_shim(VramMaskExit out);


/* system/stage_clear_bits.c: ClearStageBitRows (024F:2DF) run at `at`, with its VRAM address setter and its tables of
 * the stage's bit. */
StageExit ClearStageBitRowsAt(uint8_t x, void (*set)(uint8_t lo, uint8_t hi), uint16_t at, uint16_t bit_lo,
                              uint16_t bit_hi);


/* system/stage_clear_bits.c: ClearStageBitRowsFour (024F:2D9) run at `at`, with the C of the rows it runs. */
StageExit ClearStageBitRowsFourAt(uint8_t x, uint16_t at, StageExit (*run)(uint8_t x));


/* system/stage_play.c: StagePlayerFrame and StagePlayerObjects (024D:0AB, 024D:0B1) with ret the address after their
 * JSR to MapOverlayBanks5And6. */
StageRegs StagePlayerFrameAt(StageRegs r, uint16_t ret);
StageRegs StagePlayerObjectsAt(StageRegs r, uint16_t ret);

/* system/stage_play.c: StageDemoPadRecord (024D:0F8) run at `at`. */
StageRegs StageDemoPadRecordAt(StageRegs r, uint16_t at);

StageRegs StageHitReportTopAt(StageRegs r, uint8_t side_two_power);

StageRegs StagePauseCdThenStockAt(StageRegs r, uint16_t at);

StageRegs StagePlayerToCentreAt(StageRegs r, uint16_t at, uint16_t player_frame, uint16_t camera_step);

#endif
