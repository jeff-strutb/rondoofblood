/* The registers and flags the code holds where a routine waits or makes a block move (system/video.c). */
#ifndef RONDO_WAIT_REGS_H
#define RONDO_WAIT_REGS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * An interrupt at a wait or inside a block move pushes A, X, Y and P, so a caller in C hands a routine that waits what the
 * original holds at the call.
 */
typedef struct { uint8_t a, x, y; bool n, z, c, v; } WaitRegs;

WaitRegs wait_regs_live_shim(void);                 /* what the machine holds: for a routine entered from original code */
WaitRegs wait_regs_xy_shim(uint8_t x, uint8_t y);   /* the live A and flags, X and Y as given */

#endif
