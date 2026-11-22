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
void ScrollYFromLevelWord(void);
Alu AddToListWord(uint8_t amount, uint8_t entry);
uint8_t ScreenMapOfEntry(uint8_t entry);
uint8_t ScreenMapOfEntryB(uint8_t entry);
uint8_t ScreenMapOfEntryC(uint8_t entry);
uint8_t ScreenMapAt(uint8_t row);
Alu ScrollXAddWrap(uint8_t amount, uint8_t entry);

void abi_ClearScrollSpeeds(void);
void abi_ClearScrollSpeedsB(void);
void abi_ClearScrollSpeedsC(void);
void abi_ScrollYFromLevelWord(void);
void abi_AddToListWord(void);
void abi_ScreenMapOfEntry(void);
void abi_ScreenMapOfEntryB(void);
void abi_ScreenMapOfEntryC(void);
void abi_ScreenMapAt(void);
void abi_ScrollXAddWrap(void);

#endif
