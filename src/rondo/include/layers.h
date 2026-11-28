/* Lot L4: background layer scroll and the frame's layer work. Declared here, not in routines.h. */
#ifndef RONDO_LAYERS_H
#define RONDO_LAYERS_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "routines.h"

/* What HalveScrollStep leaves in A and C: the low byte it wrote, and the bit shifted out of it. */
typedef struct {
    uint8_t low;
    bool carry;
} ScrollStepHalved;

ScrollStepHalved HalveScrollStep(uint8_t from, uint8_t to);
ScrollStepHalved HalveScrollStepFromFirst(uint8_t to);
ScrollStepHalved HalveScrollStepFirstToSecond(void);

void abi_HalveScrollStep(void);
void abi_HalveScrollStepFromFirst(void);
void abi_HalveScrollStepFirstToSecond(void);

void ClearScrollSpeeds(void);
void ClearScrollSpeedsB(void);
void ClearScrollSpeedsC(void);
uint8_t ScrollYFromLevelWord(void);
Sum AddToListWord(uint8_t amount, uint8_t entry);
uint8_t ScreenMapOfEntry(uint8_t entry);
uint8_t ScreenMapOfEntryB(uint8_t entry);
uint8_t ScreenMapAt(uint8_t row);
uint8_t ScreenMapAtB(uint8_t row);
Sum ScrollXAddWrap(uint8_t amount, uint8_t entry);

void abi_ClearScrollSpeeds(void);
void abi_ClearScrollSpeedsB(void);
void abi_ClearScrollSpeedsC(void);
void abi_ScrollYFromLevelWord(void);
void abi_AddToListWord(void);
void abi_ScreenMapOfEntry(void);
void abi_ScreenMapOfEntryB(void);
void abi_ScreenMapAt(void);
void abi_ScreenMapAtB(void);
void abi_ScrollXAddWrap(void);


/* What BitmapBitShifted leaves: A, X and Y, and the bit in the carry. */
typedef struct {
    uint8_t a, x, y;
    bool carry;
} BitmapBit;

/* What AdvanceScroll leaves: the last add (A, N, Z, C, V) and the step's sign, which is Y. */
typedef struct {
    Sum sum;
    uint8_t sign;
} AdvanceResult;

Sum ScrollSpeedsFromBands(void);
BitmapBit BitmapBitShifted(uint8_t x, uint8_t y);
Sum CopyWordsToVdc(uint8_t blocks);
Sum ScrollXPlusOffset(uint8_t entry, uint8_t index);
Sum ScrollXPlusOffsetB(uint8_t entry, uint8_t index);
Sum ScrollXPlusOffsetC(uint8_t entry, uint8_t index);
Sum ScrollYPlusOffset(uint8_t entry, uint8_t index);
Sum ScrollYPlusOffsetB(uint8_t entry, uint8_t index);
AdvanceResult AdvanceScrollX(uint8_t entry);
/* The bodies of AdvanceScrollX, ScrollXPlusOffset and ScrollYPlusOffset with the overlay's table given (copies in other overlays differ only there). */
AdvanceResult AdvanceScrollXAt(uint8_t entry, uint16_t table);
Sum ScrollXPlusOffsetAt(uint8_t entry, uint8_t index, uint16_t table);
Sum ScrollYPlusOffsetAt(uint8_t entry, uint8_t index, uint16_t table);
AdvanceResult AdvanceScrollXB(uint8_t entry);
AdvanceResult AdvanceScrollXC(uint8_t entry);
AdvanceResult AdvanceScrollY(uint8_t entry);
AdvanceResult AdvanceScrollYB(uint8_t entry);

void abi_ScrollSpeedsFromBands(void);
void abi_BitmapBitShifted(void);
void abi_CopyWordsToVdc(void);
void abi_ScrollXPlusOffset(void);
void abi_ScrollXPlusOffsetB(void);
void abi_ScrollXPlusOffsetC(void);
void abi_ScrollYPlusOffset(void);
void abi_ScrollYPlusOffsetB(void);
void abi_AdvanceScrollX(void);
void abi_AdvanceScrollXB(void);
void abi_AdvanceScrollXC(void);
void abi_AdvanceScrollY(void);
void abi_AdvanceScrollYB(void);

/* The copies in the programs at 10C5 and 12D5. */
Sum ScrollSpeedsFromBands10C5(void);
void abi_ScrollSpeedsFromBands10C5(void);
uint8_t ScrollYFromLevelWord10C5(void);
void abi_ScrollYFromLevelWord10C5(void);
BitmapBit BitmapBitShifted10C5(uint8_t x, uint8_t y);
void abi_BitmapBitShifted10C5(void);
Sum ScrollXAddWrap10C5(uint8_t amount, uint8_t entry);
void abi_ScrollXAddWrap10C5(void);
Sum ScrollSpeedsFromBands12D5(void);
void abi_ScrollSpeedsFromBands12D5(void);
uint8_t ScrollYFromLevelWord12D5(void);
void abi_ScrollYFromLevelWord12D5(void);
uint8_t ScreenMapAt12D5(uint8_t row);
void abi_ScreenMapAt12D5(void);
AdvanceResult AdvanceScrollY12D5(uint8_t entry);
void abi_AdvanceScrollY12D5(void);
BitmapBit BitmapBitShifted12D5(uint8_t x, uint8_t y);
void abi_BitmapBitShifted12D5(void);
Sum ScrollYPlusOffset12D5(uint8_t entry, uint8_t index);
void abi_ScrollYPlusOffset12D5(void);
Sum ScrollXAddWrap12D5(uint8_t amount, uint8_t entry);
void abi_ScrollXAddWrap12D5(void);

#endif
