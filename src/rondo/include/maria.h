/*
 * Maria's player overlay (sectors 01DD-01E4, loaded by scene $01 at $A000-$DFFF: $A000-$BFFF bank $6D, $C000-$DFFF
 * bank $6E): what her own files (object/maria_*.c) share.
 *
 * Regs is what one of her routines leaves in the registers and flags: each field counts only when its bit is in
 * `set`; the others are as they stood when the routine was entered, or as the call it ended in left them (the
 * abi_ wrapper gives that call's exit first, then this). It is the register file the assembly keeps, carried as a
 * value: a body sets it where the assembly's instruction sets the register, and nowhere else.
 */
#ifndef RONDO_MARIA_H
#define RONDO_MARIA_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum { R_A = 0x01, R_X = 0x02, R_Y = 0x04, R_NZ = 0x08, R_C = 0x10, R_V = 0x20 };

typedef struct {
    uint8_t a, x, y;
    uint8_t nz;     /* the value N and Z come from */
    bool n_on;      /* N set although nz is zero (a TST whose memory had bit 7) */
    bool c, v;
    uint8_t set;
} Regs;

/* Nothing set: every register and flag as it was. */
static inline Regs regs_kept(void)
{
    Regs r = { 0 };

    return r;
}

/* N and Z from a value (an INC, a DEC, an AND, a compare's difference). */
static inline Regs r_nz(Regs r, uint8_t value)
{
    r.nz = value;
    r.n_on = false;
    r.set |= R_NZ;
    return r;
}

/* LDA, PLA, TXA and the like: A, with N and Z from it. */
static inline Regs r_lda(Regs r, uint8_t value)
{
    r.a = value;
    r.set |= R_A;
    return r_nz(r, value);
}

/* LDX, INX, TAX: X, with N and Z from it. */
static inline Regs r_ldx(Regs r, uint8_t value)
{
    r.x = value;
    r.set |= R_X;
    return r_nz(r, value);
}

/* LDY, INY, TAY: Y, with N and Z from it. */
static inline Regs r_ldy(Regs r, uint8_t value)
{
    r.y = value;
    r.set |= R_Y;
    return r_nz(r, value);
}

/* CLA, CLX, CLY: the register only; the flags stand. */
static inline Regs r_a_only(Regs r, uint8_t value)
{
    r.a = value;
    r.set |= R_A;
    return r;
}

static inline Regs r_x_only(Regs r, uint8_t value)
{
    r.x = value;
    r.set |= R_X;
    return r;
}

static inline Regs r_y_only(Regs r, uint8_t value)
{
    r.y = value;
    r.set |= R_Y;
    return r;
}

static inline Regs r_c(Regs r, bool c)
{
    r.c = c;
    r.set |= R_C;
    return r;
}

static inline Regs r_v(Regs r, bool v)
{
    r.v = v;
    r.set |= R_V;
    return r;
}

/* CMP, CPX, CPY: N and Z from the difference, C from no borrow. */
static inline Regs r_cmp(Regs r, uint8_t reg, uint8_t operand)
{
    return r_c(r_nz(r, (uint8_t)(reg - operand)), reg >= operand);
}

/* TST #mask, memory: Z from the AND, N and V bits 7 and 6 of the memory. */
static inline Regs r_tst(Regs r, uint8_t mask, uint8_t memory)
{
    bool zero = (mask & memory) == 0;

    r = r_nz(r, zero ? 0 : (uint8_t)((memory & 0x80) | 0x01));
    r.n_on = zero && (memory & 0x80) != 0;
    return r_v(r, (memory & 0x40) != 0);
}

/* What `after` left over what `before` left: a field `after` did not set is `before`'s. */
static inline Regs r_then(Regs before, Regs after)
{
    if (!(after.set & R_A)) {
        after.a = before.a;
    }
    if (!(after.set & R_X)) {
        after.x = before.x;
    }
    if (!(after.set & R_Y)) {
        after.y = before.y;
    }
    if (!(after.set & R_NZ)) {
        after.nz = before.nz;
        after.n_on = before.n_on;
    }
    if (!(after.set & R_C)) {
        after.c = before.c;
    }
    if (!(after.set & R_V)) {
        after.v = before.v;
    }
    after.set |= before.set;
    return after;
}

/* The wrapper's last step: what the routine set, into the processor. */
static inline void regs_exit(Regs r)
{
    if (r.set & R_A) {
        cpu.a = r.a;
    }
    if (r.set & R_X) {
        cpu.x = r.x;
    }
    if (r.set & R_Y) {
        cpu.y = r.y;
    }
    if (r.set & R_NZ) {
        abi_nz(r.nz);
        if (r.n_on) {
            abi_flag(P_N, true);
        }
    }
    if (r.set & R_C) {
        abi_flag(P_C, r.c);
    }
    if (r.set & R_V) {
        abi_flag(P_V, r.v);
    }
}

/* Her disc tables, read in the bank her overlay maps them in (TBL8, access.h). */
#define MariaAnimationSounds_TBL MariaAnimationSounds          /* $C1A8, bank $6E: the list by animation */
#define MariaFootYByState_TBL MariaFootYByState                /* $A118, bank $6D: the feet's row by state */
#define MariaStartHandlers_TBL MariaStartHandlers              /* $A131, bank $6D: four words by zField8A */
#define MariaAnimationHitboxIndex_TBL MariaAnimationHitboxIndex /* $C200, bank $6E: hitbox index by animation */
#define MariaAnimationHitbox_TBL MariaAnimationHitbox          /* $C258, bank $6E: four bytes a hitbox */

/* Defined elsewhere, typed, and not declared in routines.h. */
typedef struct { uint8_t keep; CommandCall sound; } FadeStop;      /* system/game_mode.c */
FadeStop FadeAndStopSound(uint8_t kept_x, uint8_t kept_y);         /* system/game_mode.c */
void abi_FadeAndStopSound(void);                                   /* system/game_mode.c */
void ClearObject(uint8_t object);                                  /* object/lifecycle.c */
void MariaClearWork(uint8_t a, uint8_t x, uint8_t y);              /* object/richter_state.c */
uint8_t MariaClearKeepPlace(void);                                 /* object/richter_state.c */
void MariaStartHitScript(uint8_t script);                          /* object/richter_state.c */
void hit_script_exit_shim(uint8_t script);                         /* object/richter_state.c */
void MariaPadForcesOff(void);                                      /* object/richter_state.c */
AnimationStep MariaSetAnimation(uint8_t animation);                /* object/player_input.c */
AnimationStep MariaAnimationTick(void);                            /* object/player_input.c */
ColumnOffset MariaColumnOffset(uint8_t column);                    /* object/player_terrain.c */
TileRead MariaReadAbove(uint8_t row, bool on_the_map);             /* object/player_terrain.c */

/* object/maria_frame.c */
/* What an animation set left, then the registers set after it (its wrapper gives the animation's exit first). */
typedef struct { AnimationStep animation; Regs regs; } AnimThen;
Regs MariaAnimationSound(void);
void abi_MariaAnimationSound(void);
void abi_MariaStart(void);
AnimationStep MariaVecSetAnimation(uint8_t animation);
AnimationStep MariaVecAnimationTick(void);
Regs MariaVecAnimationSound(void);
void MariaStopX(void);
void MariaStopMoving(void);
AnimThen MariaStand(void);
AnimThen MariaCrouch(void);
AnimThen MariaToFall(void);
AnimThen MariaStartStand(void);
AnimThen MariaStartClear(uint8_t a, uint8_t x, uint8_t y);
AnimThen MariaStartHealth(uint8_t x, uint8_t y);
AnimThen MariaStartNew(uint8_t x, uint8_t y);
CommandCall MariaClearSlot5AsKind9(uint8_t y);
CommandCall MariaDeathSound(uint8_t x, uint8_t y);
CommandCall MariaDie(uint8_t x, uint8_t y);
void abi_MariaObjectApplyVelocity(void);                           /* object/richter_objects.c */
void abi_MariaApplyMoveAndPush(void);                              /* object/richter_state.c */

#endif
