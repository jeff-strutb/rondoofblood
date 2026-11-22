/* Lot L4: background layer scroll and the frame's layer work. Declared here, not in routines.h. */
#ifndef RONDO_LAYERS_H
#define RONDO_LAYERS_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

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
Alu AddToListWord(uint8_t amount, uint8_t entry);
uint8_t ScreenMapOfEntry(uint8_t entry);
uint8_t ScreenMapOfEntryB(uint8_t entry);
uint8_t ScreenMapAt(uint8_t row);
uint8_t ScreenMapAtB(uint8_t row);
Alu ScrollXAddWrap(uint8_t amount, uint8_t entry);

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
    Alu sum;
    uint8_t sign;
} AdvanceResult;

Alu ScrollSpeedsFromBands(void);
BitmapBit BitmapBitShifted(uint8_t x, uint8_t y);
Alu CopyWordsToVdc(uint8_t blocks);
Alu ScrollXPlusOffset(uint8_t entry, uint8_t index);
Alu ScrollXPlusOffsetB(uint8_t entry, uint8_t index);
Alu ScrollXPlusOffsetC(uint8_t entry, uint8_t index);
Alu ScrollYPlusOffset(uint8_t entry, uint8_t index);
Alu ScrollYPlusOffsetB(uint8_t entry, uint8_t index);
AdvanceResult AdvanceScrollX(uint8_t entry);
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

#endif
