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

/* What DrawBlockRow leaves: A zTemp10's new value, Y the last block's index, C and V from its add. */
typedef struct {
    uint8_t a;
    uint8_t y;
    bool carry;
    bool overflow;
} RowEnd;

/* What a row check leaves: the masked compare (A, N, Z) when nothing was drawn, else the row's end. */
typedef struct {
    uint8_t masked;
    bool drawn;
    RowEnd row;
} RowDue;

RowEnd DrawBlockRow(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
/* The column and row drawing with their camera module `moved` on from $A000. */
RowEnd DrawBlockRowMoved(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow, uint16_t moved);
ColumnEnd DrawBlockColumnMoved(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow, uint16_t moved);
BlockIndex BlockIndexForPlaceMoved(uint8_t entry, uint8_t y, bool overflow, uint16_t moved);
RowDue RowDueUpMoved(uint8_t entry, uint8_t index, bool carry, bool overflow, uint16_t moved);
RowDue RowDueAtMoved(uint8_t entry, uint8_t index, bool carry, bool overflow, uint16_t moved);
void abi_DrawBlockRow(void);
RowDue RowDueUp(uint8_t entry, uint8_t index, bool carry, bool overflow);
RowDue RowDueAt(uint8_t entry, uint8_t index, bool carry, bool overflow);
void abi_RowDueUp(void);
void abi_RowDueAt(void);
ColumnEnd DrawBlockColumn(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockColumn(void);
BlockIndex BlockIndexForPlace(uint8_t entry, uint8_t y, bool overflow);
void abi_BlockIndexForPlace(void);

/* The copies in the programs at 10C5 and 12D5. */
ColumnEnd DrawBlockColumn10C5(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockColumn10C5(void);
BlockIndex BlockIndexForPlace10C5(uint8_t entry, uint8_t y, bool overflow);
void abi_BlockIndexForPlace10C5(void);
RowDue RowDueUp10C5(uint8_t entry, uint8_t index, bool carry, bool overflow);
void abi_RowDueUp10C5(void);
RowDue RowDueAt10C5(uint8_t entry, uint8_t index, bool carry, bool overflow);
void abi_RowDueAt10C5(void);
RowEnd DrawBlockRow10C5(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockRow10C5(void);
ColumnEnd DrawBlockColumn12D5(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockColumn12D5(void);
BlockIndex BlockIndexForPlace12D5(uint8_t entry, uint8_t y, bool overflow);
void abi_BlockIndexForPlace12D5(void);
RowDue RowDueUp12D5(uint8_t entry, uint8_t index, bool carry, bool overflow);
void abi_RowDueUp12D5(void);
RowDue RowDueAt12D5(uint8_t entry, uint8_t index, bool carry, bool overflow);
void abi_RowDueAt12D5(void);
RowEnd DrawBlockRow12D5(uint8_t entry, uint8_t index, bool entry_carry, bool entry_overflow);
void abi_DrawBlockRow12D5(void);

#endif
