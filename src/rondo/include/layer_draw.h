/* Lot L4: drawing the frame's objects. Declared here, not in routines.h. */
#ifndef RONDO_LAYER_DRAW_H
#define RONDO_LAYER_DRAW_H

#include <stdbool.h>
#include <stdint.h>

/* The registers and flags EmitObjectSprites leaves: its callers read N (the room ran out) and nothing else. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v, tail;
} SpriteExit;

SpriteExit EmitObjectSprites(uint8_t object);
void abi_EmitObjectSprites(void);

#endif
