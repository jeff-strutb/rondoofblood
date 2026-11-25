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

/* object/hitbox_position.c: A at exit, the last value the loop had in A. */
/* A, and the overflow of the last add when one was made; X $10, N 0, Z 1, C 1 follow. */
typedef struct { uint8_t a; bool v_seen; bool v; } HitboxEnd;
HitboxEnd ComputeHitboxPositions(void);

/* system/map_draw.c: the VDC data port's word, masked in place; the high byte read back and returned. */
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

/* system/stage_objects.c: sets a stage's objects up and runs the first frame; leaves what RunObjects leaves. */
void SetUpStageObjects(uint8_t x);
void SetUpStageObjectsB(uint8_t x);
void SetUpStageObjectsC(uint8_t x);
void SetUpStageObjectsD(uint8_t x);
void SetUpStageObjectsE(uint8_t x);
void SetUpStageObjectsF(uint8_t x);
void SetUpStageObjectsG(uint8_t x);
void SetUpStageObjectsH(uint8_t x);
void SetUpStageObjectsI(uint8_t x);
void SetUpStageObjectsJ(uint8_t x);
void SetUpStageObjectsK(uint8_t x);
void SetUpStageObjectsL(uint8_t x);
void SetUpStageObjectsM(uint8_t x);
void SetUpStageObjectsN(uint8_t x);
void SetUpStageObjectsO(uint8_t x);
void SetUpStageObjectsP(uint8_t x);
void SetUpStageObjectsQ(uint8_t x);
void SetUpStageObjectsR(uint8_t x);
void SetUpStageObjectsS(uint8_t x);
void SetUpStageObjectsT(uint8_t x);
void SetUpStageObjectsU(uint8_t x);
void SetUpStageObjectsV(uint8_t x);
void SetUpStageObjectsW(uint8_t x);

/* system/stage_spawn.c: A, X and Y the pair leaves; ran false when nothing was done. */
typedef struct { uint8_t a; uint8_t x; uint8_t y; bool ran; } StageSpawnEnd;
StageSpawnEnd SpawnStageObjectsRows01(void);
StageSpawnEnd SpawnStageObjectsRows01Long(void);
StageSpawnEnd SpawnStageObjectsRows34(void);
StageSpawnEnd SpawnStageObjectsRows34Long(void);

/* system/stage_blend_steps.c: A the restored bank, C and V as left, Y 5 once a blend ran. */
typedef struct { uint8_t a; bool c; bool v; bool y_set; } BlendStepEnd;
BlendStepEnd BlendStageStockRows(uint8_t x, uint8_t y);
BlendStepEnd BlendStageScoreAddRows(uint8_t x, uint8_t y);
BlendStepEnd BlendStageScoreRows(uint8_t x);
BlendStepEnd BlendStageScoreRowsLong(uint8_t x);

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
StageExit StepStageSequence(uint8_t x, uint8_t y);

StageExit StageScriptByWave(void);
StageExit SetHealthBarAddress(void);
StageExit SetHealthBarAddressLong(void);
StageExit StepStageFrame(uint8_t x, uint8_t y);
StageExit StepStageFrameLong(uint8_t x, uint8_t y);

/* object/hit_test.c: the overlay's hit test and its two handlers. */
StageExit HitTestObject(uint8_t x);
StageExit HitTestObjectLong(uint8_t x);
StageExit HitStoreHandler(uint8_t x, uint8_t y);
StageExit HitReportHandler(uint8_t x, uint8_t y);
StageExit HitTestAllObjects(uint8_t x);
StageExit HitTestAllObjectsLong(uint8_t x);

/* object/stage_ending.c: one step of the stage's ending effect. */
StageExit StepStageEnding(uint8_t x, uint8_t y);

/* system/stage_main.c: one frame of the stage. */
StageExit RunStageFrame(uint8_t x, uint8_t y);

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
StageExit EnterStageState(void);

#endif
