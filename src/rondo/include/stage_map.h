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
void MapStageBanks(void);
uint8_t RestoreStageBanks(void);

/* object/hitbox_position.c: A at exit, the last value the loop had in A. */
uint8_t ComputeHitboxPositions(void);

/* system/map_draw.c: the VDC data port's word, masked in place; the high byte read back and returned. */
void SetVramAddresses(uint8_t lo, uint8_t hi);
void SetVramAddressesLong(uint8_t lo, uint8_t hi);
void SetStageTileVramAddress(void);
void SetStageTileVramAddressLong(void);
/* What a blend leaves: A, X and the overflow; Y is 5, N 0, Z 1, C 1. */
typedef struct { uint8_t a; uint8_t x; bool v; } TileRowsExit;
TileRowsExit BlendStageTileRows(uint8_t phase, uint8_t row_a, uint8_t row_b);
TileRowsExit BlendStageRowPair(uint8_t x, uint8_t a, uint8_t y);
TileRowsExit BlendStageRowChanges(uint8_t x, uint8_t new_byte);
uint8_t AndVramData(uint8_t lo_mask, uint8_t hi_mask);
uint8_t OrVramData(uint8_t lo_mask, uint8_t hi_mask);

/* system/stage_mask.c: x the row's base offset in the mask buffer; y is kept on the stack and handed back. */
/* The last cell written and the last offset add: A, C and V at exit. */
typedef struct { uint8_t a; Sum last; } MaskExit;
MaskExit MaskStageRowCopy(uint8_t x, uint8_t y);
MaskExit MaskStageRow(uint8_t x, uint8_t y);

/* system/stage_vram_mask.c: A at exit (the first byte of the mask buffer). */
uint8_t MaskVramRowsRandom(void);
uint8_t MaskVramRowsRandomLong(void);

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

#endif
