/* Lot L4: background layer scroll and the frame's layer work. Declared here, not in routines.h. */
#ifndef RONDO_LAYERS_H
#define RONDO_LAYERS_H

#include <stdbool.h>
#include <stdint.h>

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

#endif
