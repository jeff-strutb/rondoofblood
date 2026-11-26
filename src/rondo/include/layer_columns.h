/* Layer columns (system/layer_columns.c): a column of tile blocks drawn to the VRAM. Declared here, not in routines.h. */
#ifndef RONDO_LAYER_COLUMNS_H
#define RONDO_LAYER_COLUMNS_H

#include "machine.h"
#include "routines.h"
#include "layers.h"

/* What DrawBlockColumn leaves that its last call made: Y the block's index pulled back, V of the block copy. */
typedef struct {
    uint8_t y;
    bool overflow;
} ColumnEnd;

ColumnEnd DrawBlockColumn(uint8_t entry, uint8_t index);
void abi_DrawBlockColumn(void);
BitmapBit BlockIndexForPlace(uint8_t entry, uint8_t y);
void abi_BlockIndexForPlace(void);

#endif
