/* Lot L4: drawing the frame's objects. Declared here, not in routines.h. */
#ifndef RONDO_LAYER_DRAW_H
#define RONDO_LAYER_DRAW_H

#include <stdbool.h>
#include <stdint.h>

/* The registers and flags EmitObjectSprites leaves; nz holds the P bits N and Z. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
} SpriteExit;

SpriteExit EmitObjectSprites(uint8_t object);

/* What WriteObjectSprites leaves in A, X, Y and the flags. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
} ObjectSpritesExit;

ObjectSpritesExit WriteObjectSprites(bool carry, bool overflow);
void abi_WriteObjectSprites(void);
void abi_EmitObjectSprites(void);

#endif
