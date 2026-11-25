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

#endif
