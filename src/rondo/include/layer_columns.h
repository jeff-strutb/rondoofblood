/* Layer columns (system/layer_columns.c): a column of tile blocks drawn to the VRAM. Declared here, not in routines.h. */
#ifndef RONDO_LAYER_COLUMNS_H
#define RONDO_LAYER_COLUMNS_H

#include "machine.h"
#include "routines.h"
#include "layers.h"

/* What DrawBlockColumn leaves that its last call made: Y the block's index pulled back, V of the block copy. */
typedef struct {
    uint8_t y;
    bool carry;
    bool overflow;
    bool from_original;   /* the last block copy ran as original code: A is as it left it */
} ColumnEnd;

/* What BlockIndexForPlace leaves: BitmapBitShifted's bit, X and Y, and V from SelectBlockTables' add. */
typedef struct {
    BitmapBit bit;
    bool overflow;
} BlockIndex;

ColumnEnd DrawBlockColumn(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockColumn(void);
BlockIndex BlockIndexForPlace(uint8_t entry, uint8_t y, bool overflow);
void abi_BlockIndexForPlace(void);

#endif
