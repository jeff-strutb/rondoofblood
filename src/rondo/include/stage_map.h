/* Lot L3 (stage map, setup, scroll, terrain): the typed functions the other lots call. */
#ifndef RONDO_STAGE_MAP_H
#define RONDO_STAGE_MAP_H

#include "machine.h"

/* system/stage_setup.c */
void ClearStageStateBytes(void);

/* system/map_draw.c: the VDC data port's word, masked in place; the high byte read back and returned. */
uint8_t AndVramData(uint8_t lo_mask, uint8_t hi_mask);
uint8_t OrVramData(uint8_t lo_mask, uint8_t hi_mask);

#endif
