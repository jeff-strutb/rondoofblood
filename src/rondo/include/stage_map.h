/* Lot L3 (stage map, setup, scroll, terrain): the typed functions the other lots call. */
#ifndef RONDO_STAGE_MAP_H
#define RONDO_STAGE_MAP_H

#include "machine.h"
#include "routines.h"

/* system/stage_mode.c: N and Z come from the byte it returns. */
uint8_t StageWaitTimerStep(void);
void StageWaitTimerToStep2(void);

/* system/stage_setup.c */
void ClearStageStateBytes(void);
Sum SaveAndMapStageBanks(void);  /* the add that mapped the second bank: C and V at exit */
Sum MapStageBanks(void);
uint8_t RestoreStageBanks(void);
Sum SaveAndMapStageBanksLong(void);
Sum MapStageBanksLong(void);
uint8_t RestoreStageBanksLong(void);
/* The bank routines a group of stage overlays' blends call: 024F's at $933F and $9354, 05BF's at $9651 and $9666. */
typedef struct { Sum (*save_and_map)(void); uint8_t (*restore)(void); } StageBanks;
extern const StageBanks stage_banks_024f;
extern const StageBanks stage_banks_05bf;

/* object/hitbox_position.c: A at exit, the last value the loop had in A. */
/* A, and the overflow of the last add when one was made; X $10, N 0, Z 1, C 1 follow. */
typedef struct { uint8_t a; bool v_seen; bool v; } HitboxEnd;
HitboxEnd ComputeHitboxPositions(void);
HitboxEnd ComputeHitboxPositionsLong(void);

/* system/map_draw.c: the VDC data port's word, masked in place; the high byte read back and returned. */
void set_vram_addresses(uint8_t lo, uint8_t hi, uint16_t first_ret, uint16_t second_ret);
void SetVramAddresses(uint8_t lo, uint8_t hi);
void SetVramAddressesLong(uint8_t lo, uint8_t hi);
void SetStageTileVramAddress(void);
void SetStageTileVramAddressLong(void);
/* What a blend leaves: A, X and the overflow; Y is 5, N 0, Z 1, C 1. */
typedef struct { uint8_t a; uint8_t x; bool v; } TileRowsExit;
TileRowsExit BlendStageTileRows(uint8_t phase, uint8_t row_a, uint8_t row_b);
TileRowsExit BlendStageRowPair(uint8_t x, uint8_t a, uint8_t y);
/* What a changes blend leaves: A and X; Y 5 and V from the blend only when y_set and v_set. */
typedef struct { uint8_t a; uint8_t x; bool y_set; bool v_set; bool v; } RowChangesExit;
RowChangesExit BlendStageRowChanges(uint8_t x, uint8_t new_byte);
uint8_t AndVramData(uint8_t lo_mask, uint8_t hi_mask);
uint8_t OrVramData(uint8_t lo_mask, uint8_t hi_mask);
uint8_t AndVramDataLong(uint8_t lo_mask, uint8_t hi_mask);
uint8_t OrVramDataLong(uint8_t lo_mask, uint8_t hi_mask);

/* system/stage_mask.c: x the row's base offset in the mask buffer; y is kept on the stack and handed back. */
/* The last cell written and the last offset add: A, C and V at exit. */
typedef struct { uint8_t a; Sum last; } MaskExit;
MaskExit MaskStageRowCopy(uint8_t x, uint8_t y);
MaskExit MaskStageRow(uint8_t x, uint8_t y);

/* system/stage_vram_mask.c: A at exit (the first byte of the mask buffer). */
/* What the routine leaves at its end: A and the carry and overflow if a masking set them; Y 0 or the row count. */
typedef struct {
    uint8_t a;
    bool flags_from_mask;
    Sum mask;
    uint8_t y;
} VramMaskExit;
VramMaskExit MaskVramRowsRandom(void);
VramMaskExit MaskVramRowsRandomLong(void);

/* system/stage_setup.c: the slot the object went to, and the Y low it was given (A at exit). */
typedef struct { uint8_t slot; uint8_t y_lo; } SpawnedObject;
SpawnedObject SpawnStageObject(uint8_t row);
SpawnedObject SpawnStageObjectLong(uint8_t row);

/* system/stage_vram_runs.c: where the walk stopped: X the buffer index, Y the list position (A $FF, N 0, Z 1, C 1). */
typedef struct { uint8_t x; uint8_t y; } StageRunsEnd;
StageRunsEnd expand_vram_runs(uint16_t table);
void runs_exit_shim(StageRunsEnd end);
StageRunsEnd ExpandStageVramRuns(void);
StageRunsEnd ExpandStageVramRunsB(void);
StageRunsEnd ExpandStageVramRunsC(void);
StageRunsEnd ExpandStageVramRunsD(void);
StageRunsEnd ExpandStageVramRunsE(void);
StageRunsEnd ExpandStageVramRunsF(void);
StageRunsEnd ExpandStageVramRunsG(void);
StageRunsEnd ExpandStageVramRunsH(void);
StageRunsEnd ExpandStageVramRunsI(void);
StageRunsEnd ExpandStageVramRunsJ(void);
StageRunsEnd ExpandStageVramRunsK(void);
StageRunsEnd ExpandStageVramRunsL(void);
StageRunsEnd ExpandStageVramRunsM(void);
StageRunsEnd ExpandStageVramRunsN(void);
StageRunsEnd ExpandStageVramRunsO(void);
StageRunsEnd ExpandStageVramRunsP(void);
StageRunsEnd ExpandStageVramRunsQ(void);
StageRunsEnd ExpandStageVramRunsR(void);
StageRunsEnd ExpandStageVramRunsS(void);
StageRunsEnd ExpandStageVramRunsT(void);
StageRunsEnd ExpandStageVramRunsU(void);
StageRunsEnd ExpandStageVramRunsV(void);

/* system/stage_lists.c: where the walk stopped: Y the last index, X if a run was stored, V from the last add. */
typedef struct { uint8_t x; bool x_set; uint8_t y; bool v; } StageListEnd;
StageListEnd expand_stage_lists(uint16_t table);
void lists_exit_shim(StageListEnd end);
StageListEnd ExpandStageLists(void);
StageListEnd ExpandStageListsB(void);
StageListEnd ExpandStageListsC(void);
StageListEnd ExpandStageListsD(void);
StageListEnd ExpandStageListsE(void);
StageListEnd ExpandStageListsF(void);
StageListEnd ExpandStageListsG(void);
StageListEnd ExpandStageListsH(void);
StageListEnd ExpandStageListsI(void);
StageListEnd ExpandStageListsJ(void);
StageListEnd ExpandStageListsK(void);
StageListEnd ExpandStageListsL(void);
StageListEnd ExpandStageListsM(void);
StageListEnd ExpandStageListsN(void);
StageListEnd ExpandStageListsO(void);
StageListEnd ExpandStageListsP(void);
StageListEnd ExpandStageListsQ(void);
StageListEnd ExpandStageListsR(void);
StageListEnd ExpandStageListsS(void);
StageListEnd ExpandStageListsT(void);
StageListEnd ExpandStageListsU(void);
StageListEnd ExpandStageListsV(void);

/* system/stage_scroll.c: as StreamScrollStep (routines.h), and the V of the bank offset add made before it. */
typedef struct { StreamWalk walk; bool v_before; } StageScrollEnd;
StageScrollEnd StageScrollStep(uint8_t x);
StageScrollEnd StageScrollStepB(uint8_t x);
StageScrollEnd StageScrollStepC(uint8_t x);
StageScrollEnd StageScrollStepD(uint8_t x);
StageScrollEnd StageScrollStepE(uint8_t x);
StageScrollEnd StageScrollStepF(uint8_t x);
StageScrollEnd StageScrollStepG(uint8_t x);
StageScrollEnd StageScrollStepH(uint8_t x);
StageScrollEnd StageScrollStepI(uint8_t x);
StageScrollEnd StageScrollStepJ(uint8_t x);
StageScrollEnd StageScrollStepK(uint8_t x);
StageScrollEnd StageScrollStepL(uint8_t x);
StageScrollEnd StageScrollStepM(uint8_t x);
StageScrollEnd StageScrollStepN(uint8_t x);
StageScrollEnd StageScrollStepO(uint8_t x);
StageScrollEnd StageScrollStepP(uint8_t x);
StageScrollEnd StageScrollStepQ(uint8_t x);
StageScrollEnd StageScrollStepR(uint8_t x);
StageScrollEnd StageScrollStepS(uint8_t x);
StageScrollEnd StageScrollStepT(uint8_t x);
StageScrollEnd StageScrollStepU(uint8_t x);
StageScrollEnd StageScrollStepV(uint8_t x);
StageScrollEnd StageScrollStepW(uint8_t x);

/* system/stage_start.c: the script started and the carry and overflow of the table index add. */
typedef struct { uint8_t script; Sum index; } StageStartEnd;
StageStartEnd StartStageLists(void);
StageStartEnd StartStageListsB(void);
StageStartEnd StartStageListsC(void);
StageStartEnd StartStageListsD(void);
StageStartEnd StartStageListsE(void);
StageStartEnd StartStageListsF(void);
StageStartEnd StartStageListsG(void);
StageStartEnd StartStageListsH(void);
StageStartEnd StartStageListsI(void);
StageStartEnd StartStageListsJ(void);
StageStartEnd StartStageListsK(void);
StageStartEnd StartStageListsL(void);
StageStartEnd StartStageListsM(void);
StageStartEnd StartStageListsN(void);
StageStartEnd StartStageListsO(void);
StageStartEnd StartStageListsP(void);
StageStartEnd StartStageListsQ(void);
StageStartEnd StartStageListsR(void);
StageStartEnd StartStageListsS(void);
StageStartEnd StartStageListsT(void);
StageStartEnd StartStageListsU(void);
StageStartEnd StartStageListsV(void);
StageStartEnd StartStageListsW(void);
StageScrollEnd StageScrollStepResident(uint8_t x);

/* system/stage_frame.c: the frame set-up's end; leaves what RunObjects leaves. */
void StepScrollAndRunObjects(uint8_t x);
/* What a stage object set-up leaves for its tail jump. */
typedef struct { uint8_t x; Sum count; } SetUpEnd;

/* system/stage_objects.c: sets a stage's objects up and runs the first frame; leaves what RunObjects leaves. */
SetUpEnd SetUpStageObjects(uint8_t x);
SetUpEnd SetUpStageObjectsB(uint8_t x);
SetUpEnd SetUpStageObjectsC(uint8_t x);
SetUpEnd SetUpStageObjectsD(uint8_t x);
SetUpEnd SetUpStageObjectsE(uint8_t x);
SetUpEnd SetUpStageObjectsF(uint8_t x);
SetUpEnd SetUpStageObjectsG(uint8_t x);
SetUpEnd SetUpStageObjectsH(uint8_t x);
SetUpEnd SetUpStageObjectsI(uint8_t x);
SetUpEnd SetUpStageObjectsJ(uint8_t x);
SetUpEnd SetUpStageObjectsK(uint8_t x);
SetUpEnd SetUpStageObjectsL(uint8_t x);
SetUpEnd SetUpStageObjectsM(uint8_t x);
SetUpEnd SetUpStageObjectsN(uint8_t x);
SetUpEnd SetUpStageObjectsO(uint8_t x);
SetUpEnd SetUpStageObjectsP(uint8_t x);
SetUpEnd SetUpStageObjectsQ(uint8_t x);
SetUpEnd SetUpStageObjectsR(uint8_t x);
SetUpEnd SetUpStageObjectsS(uint8_t x);
SetUpEnd SetUpStageObjectsT(uint8_t x);
SetUpEnd SetUpStageObjectsU(uint8_t x);
SetUpEnd SetUpStageObjectsV(uint8_t x);
SetUpEnd SetUpStageObjectsW(uint8_t x);

/* system/stage_spawn.c: A, X and Y the pair leaves; ran false when nothing was done. */
typedef struct { uint8_t a; uint8_t x; uint8_t y; bool ran; } StageSpawnEnd;
StageSpawnEnd SpawnStageObjectsRows01(void);
StageSpawnEnd SpawnStageObjectsRows01Long(void);
StageSpawnEnd SpawnStageObjectsRows34(void);
StageSpawnEnd SpawnStageObjectsRows34Long(void);

/* system/stage_blend_steps.c: A the restored bank, C and V as left, Y 5 once a blend ran. */
typedef struct { uint8_t a; bool c; bool v; bool y_set; } BlendStepEnd;
BlendStepEnd BlendStageScoreRows(uint8_t x);
BlendStepEnd BlendStageScoreRowsLong(uint8_t x);
/* system/stage_copies.c: the copy of a stage helper resident at its address, called as the JSR there reaches it (024F's
 * group's when 05BF's group's is not loaded). */
BlendStepEnd blend_stock_step_loaded(uint8_t x, uint8_t y);
BlendStepEnd blend_score_add_step_loaded(uint8_t x, uint8_t y);
BlendStepEnd blend_score_rows_loaded(uint8_t x);
uint8_t and_vram_data_loaded(uint8_t lo_mask, uint8_t hi_mask);
uint8_t or_vram_data_loaded(uint8_t lo_mask, uint8_t hi_mask);
Sum map_stage_banks_loaded(void);

/* system/stage_sequence.c: the registers and flags a stage routine with several exits sets; the set mask names which; N and Z are those of nz, which a compare leaves as its difference. */
enum { EXIT_X = 1, EXIT_Y = 2, EXIT_C = 4, EXIT_V = 8, EXIT_CPU = 16 };
typedef struct { uint8_t a; uint8_t x; uint8_t y; bool c; bool v; uint8_t nz; uint8_t set; } StageExit;
void stage_exit_shim(StageExit end);
StageExit stage_exit_from_cpu_shim(void);
/* The registers and flags a stage routine holds while it calls pieces that take them from the machine. */
typedef struct { uint8_t a; uint8_t x; uint8_t y; bool c; bool v; uint8_t nz; } StageRegs;
StageRegs stage_regs_after(StageExit end, StageRegs before);
void stage_regs_load_shim(StageRegs r);
StageRegs stage_regs_from_cpu_shim(StageRegs before);
/* The registers and flags a routine is entered with, for one whose callee sees them unchanged. */
StageRegs stage_regs_entry_shim(void);
StageExit StepStageSequence(uint8_t x, uint8_t y);

StageExit StageScriptByWave(void);
StageExit SetHealthBarAddress(void);
StageExit SetHealthBarAddressLong(void);
StageExit StepStageFrame(uint8_t x, uint8_t y, bool v);
/* system/stage_tick.c: the RTS before it that it branches back to (024E:36C). */
void StepStageFrameIdle(void);
void abi_StepStageFrameIdle(void);
StageExit StepStageFrameLong(uint8_t x, uint8_t y, bool v);

/* object/hit_test.c: the overlay's hit test and its two handlers. */
StageExit HitTestObject(uint8_t x);
StageExit HitTestObjectLong(uint8_t x);
StageExit HitTestObjectLongB(uint8_t x);
/* The columns of a build of the hit store handler: kind (by Y), taken (by X, and its first byte), from_a and from_b
 * (by X), to_b (one byte), and gate (a byte tested first when it is not 0). */
typedef struct { uint16_t kind, taken, from_a, from_b, to_b, gate; } HitStoreColumns;
StageExit HitStoreHandlerAt(uint8_t x, uint8_t y, const HitStoreColumns *c);
StageExit HitStoreHandler(uint8_t x, uint8_t y);
StageExit HitReportHandler(uint8_t x, uint8_t y);
StageExit HitStoreHandlerLong(uint8_t x, uint8_t y);
StageExit HitReportHandlerLong(uint8_t x, uint8_t y);
/* A build's two hit handlers that have C: their run addresses, which its jump table names, and their C. A handler
 * the table names that is neither, or every handler when handlers is NULL, goes to the routine loaded at its address. */
typedef struct {
    uint16_t store, report;
    StageExit (*store_c)(uint8_t x, uint8_t y);
    StageExit (*report_c)(uint8_t x, uint8_t y);
} HitHandlers;
/* The hit test's body (hit_test.c) for a build with its jump table at table, and the hit-test-all body with the
 * build's two callees and the return addresses of its two JSRs. */
StageExit HitTestObjectAt(uint8_t x, uint16_t table, const HitHandlers *handlers);
StageExit HitTestAllObjectsAt(uint8_t entry_x, HitboxEnd (*compute)(void), StageExit (*test)(uint8_t x),
                              uint16_t compute_return, uint16_t test_return);
StageExit HitTestAllObjects(uint8_t x);
StageExit HitTestAllObjectsLong(uint8_t x);

/* object/stage_ending.c: one step of the stage's ending effect. */
StageExit StepStageEnding(uint8_t x, uint8_t y);
/* object/stage_ending.c: its fade, from the pick (024E:866). */
StageExit StepStageEndingFade(uint8_t pick);
void abi_StepStageEndingFade(void);

/* system/stage_main.c: one frame of the stage. */
StageExit RunStageFrame(uint8_t a, uint8_t x, uint8_t y, bool c, bool v);

/* system/stage_entry.c: loads the stage section's entry record. */
StageExit LoadStageEntry(void);
StageExit LoadStageEntryB(void);
StageExit LoadStageEntryC(void);
StageExit LoadStageEntryD(void);
StageExit LoadStageEntryE(void);
StageExit LoadStageEntryF(void);
StageExit LoadStageEntryG(void);
StageExit LoadStageEntryH(void);
StageExit LoadStageEntryI(void);
StageExit LoadStageEntryJ(void);
StageExit LoadStageEntryK(void);
StageExit LoadStageEntryL(void);
StageExit LoadStageEntryM(void);
StageExit LoadStageEntryN(void);
StageExit LoadStageEntryO(void);
StageExit LoadStageEntryP(void);
StageExit LoadStageEntryQ(void);
StageExit LoadStageEntryR(void);
StageExit LoadStageEntryS(void);
StageExit LoadStageEntryT(void);
StageExit LoadStageEntryU(void);
StageExit LoadStageEntryV(void);
StageExit LoadStageEntryW(void);

StageExit LoadStageEntryResident(void);

/* system/stage_init.c: starts a stage. */
StageExit InitStage(uint8_t x, uint8_t y);
StageExit InitStageB(uint8_t x, uint8_t y);
StageExit InitStageC(uint8_t x, uint8_t y);

/* system/stage_state_start.c: the state a stage starts in. */
StageExit EnterStageState(StageRegs entry);

/* system/stage_clear_bits.c: masks rows of VRAM words by the stage's cleared bit. */
StageExit ClearStageBitRows(uint8_t x);
StageExit ClearStageBitRowsLong(uint8_t x);
StageExit ClearStageBitRowsFour(uint8_t x);

/* object/stage_gauge.c: a stage's gauge. */
StageExit SetGaugeAddress(void);
StageExit GaugeStepUp(void);
StageExit GaugeStep(void);
StageExit RunStageGauge(void);

/*
 * The stage overlay's bodies, with what moves between the overlay's copies given as a layout: the return addresses of the
 * calls (run address after the call's last byte) and the tables. The wrappers of each overlay pass its layout; 024E's values
 * are the ones its wrappers use.
 */
typedef struct {
    uint16_t clear_screen_return, first_load_return, map_return, first_code_return, second_load_return, second_code_return;
    uint16_t effect_table, first_code, second_code;
} InitStageLayout;
StageExit InitStageBody(uint8_t x, uint8_t y, const InitStageLayout *layout);
StageExit LoadStageEntryBody(uint16_t table, uint16_t character_offsets);
typedef struct { uint16_t second_offset_at, step_return; } ScrollStepLayout;
StageScrollEnd ScrollStepBody(uint8_t x, const ScrollStepLayout *layout);
typedef struct { uint16_t record_table, second_offset_at, clear_return, map_return, walk_return; } SetUpLayout;
SetUpEnd SetUpObjectsBody(uint8_t x, const SetUpLayout *layout);
typedef SpawnedObject (*SpawnRow)(uint8_t row);
typedef struct { uint16_t first_return_1, first_return_2, second_return_1, second_return_2; uint8_t list_channel; } SpawnPairLayout;
StageSpawnEnd SpawnFirstPairBody(SpawnRow spawn, const SpawnPairLayout *layout);
StageSpawnEnd SpawnSecondPairBody(SpawnRow spawn, const SpawnPairLayout *layout);
typedef void (*SetBarAddress)(uint8_t lo, uint8_t hi);
typedef struct {
    uint16_t sequence_return, stock_return, add_return, score_return, wave_return, bar_down_return, and_return, bar_up_return,
             or_return, sound_up_return, sound_down_return, map_return, blend_return_1, blend_return_2;
    uint16_t frame_table, tail_run;
    uint8_t sound_up, sound_down;   /* the health bar's sound commands */
} StepFrameLayout;
StageExit StepStageFrameBody(uint8_t x, uint8_t y, bool v, SetBarAddress set_bar, const StepFrameLayout *layout);

StageExit EnterStageStateAt(StageRegs entry, uint16_t card_return);

#endif
