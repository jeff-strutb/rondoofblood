/* Lot L3 (stage map, setup, scroll, terrain): the typed functions the other lots call. */
#ifndef RONDO_STAGE_MAP_H
#define RONDO_STAGE_MAP_H

#include "machine.h"

/* system/stage_setup.c */
void ClearStageStateBytes(void);
void SaveAndMapStageBanks(void);
void MapStageBanks(void);
uint8_t RestoreStageBanks(void);

/* object/hitbox_position.c: A at exit, the last value the loop had in A. */
uint8_t ComputeHitboxPositions(void);

/* system/map_draw.c: the VDC data port's word, masked in place; the high byte read back and returned. */
uint8_t AndVramData(uint8_t lo_mask, uint8_t hi_mask);
uint8_t OrVramData(uint8_t lo_mask, uint8_t hi_mask);

/* system/stage_mask.c: x the row's base offset in the mask buffer; y is kept on the stack and handed back. */
void MaskStageRowCopy(uint8_t x, uint8_t y);
void MaskStageRow(uint8_t x, uint8_t y);

/* system/stage_setup.c: the slot the object went to, and the Y low it was given (A at exit). */
typedef struct { uint8_t slot; uint8_t y_lo; } SpawnedObject;
SpawnedObject SpawnStageObject(uint8_t row);
SpawnedObject SpawnStageObjectLong(uint8_t row);

#endif
