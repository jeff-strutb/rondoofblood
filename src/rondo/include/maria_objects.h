/*
 * Maria's objects (her overlay's $C000-$DFFF, bank $6E: sectors 01E1-01E4): what object/maria_objects*.c share.
 *
 * The exits use maria.h's Regs and MExit. A callee's result that is a plain register set (Ended, Sum, ScaleResult,
 * SoleSlot) becomes Regs here. Before a call that goes to the routine at a run address (a stage overlay's entry at
 * $8006 or $8009), the exit so far is given to the processor (ox_flush): that routine is entered with the registers
 * the assembly has there, and what it leaves is in the processor; what follows is counted from it.
 */
#ifndef RONDO_MARIA_OBJECTS_H
#define RONDO_MARIA_OBJECTS_H

#include "maria.h"
#include "enemy_objects.h"

/* Ended (A, N and Z, C, V, Y; X kept) as Regs. */
static inline Regs regs_of_ended(Ended e)
{
    Regs r = regs_kept();

    if (e.a_set) {
        r = r_a_only(r, e.a);
    }
    if (e.nz_set) {
        r = r_nz(r, e.nz);
    }
    if (e.c_set) {
        r = r_c(r, e.c);
    }
    if (e.v_set) {
        r = r_v(r, e.v);
    }
    if (e.y_set) {
        r = r_y_only(r, e.y);
    }
    return r;
}

/* An add's or a subtract's result in A: N and Z from it, C and V. */
static inline Regs regs_of_sum(Sum s)
{
    return r_v(r_c(r_lda(regs_kept(), s.value), s.carry), s.overflow);
}

/* A velocity from a table (MariaSetVelocityXFromTable, MariaSetVelocityYFromTable): A the high byte, C the entry's
 * bit 7, Y the doubled entry. */
static inline Regs regs_of_velocity_table(ObjArm r)
{
    return r_c(r_y_only(r_lda(regs_kept(), r.first), r.y), r.flag);
}

/* SetVelocityFromAngle: A, N and Z, C, V when an add ran, Y 0. */
static inline Regs regs_of_scale(ScaleResult s)
{
    Regs r = r_c(r_y_only(r_a_only(regs_kept(), s.a), 0), s.carry);

    r = r_nz(r, s.nz);
    if (s.v_set) {
        r = r_v(r, s.overflow);
    }
    return r;
}

/* StartSoleScriptSlot: Y the script; A the script started or slot 3's kind refused, N and Z from it, C started. */
static inline Regs regs_of_sole_slot(SoleSlot s)
{
    return r_c(r_y_only(r_lda(regs_kept(), s.started ? s.script : s.busy_kind), s.script), s.started);
}

/* The exit so far, into the processor, before a call to the routine at a run address. */
static inline void ox_flush(MExit *e)
{
    mx_exit(*e);
    *e = mx_none();
}

/* Defined elsewhere, typed, and not declared in routines.h or maria.h. */
uint8_t MariaObjectNextState(uint8_t object);                       /* object/richter_objects.c */
Sum MariaObjectAtPlayer(uint8_t object);                            /* object/richter_objects.c */
AnimationStep MariaObjectSetAnimation(uint8_t object, uint8_t animation); /* object/richter_objects.c */
AnimationStep MariaObjectAnimationTick(uint8_t object);             /* object/richter_objects.c */
AnimationStep MariaObjectAnimationRunning(uint8_t object);          /* object/richter_objects.c */
ObjArm MariaSetVelocityYFromTable(uint8_t slot, uint8_t entry);     /* object/player_leaves.c */
ObjArm MariaSetVelocityXFromTable(uint8_t slot, uint8_t entry);     /* object/player_leaves.c */
Ended MariaObjectFacingVelocity(uint8_t object);                    /* object/richter_objects.c */
Ended MariaObjectFallCapped(uint8_t object);                        /* object/richter_objects.c */
AxisMove MariaObjectMove(uint8_t object);                           /* object/richter_objects.c */
AxisMove MariaObjectApplyVelocity(uint8_t object);                  /* object/richter_objects.c */
ObjectFromPlayer MariaObjectFromPlayer(uint8_t object);             /* object/richter_objects.c */
uint8_t MariaObjectMarkOn(uint8_t object);                          /* object/richter_objects.c */
void MariaObjectClear(uint8_t object);                              /* object/richter_objects.c */
Ended MariaObjectAccelerate(uint8_t object);                        /* object/richter_objects.c */
Ended MariaObjectNearPlayer(uint8_t object);                        /* object/richter_objects.c */
void RichterObjectStopX(uint8_t slot);                              /* object/richter_objects.c */
void RichterObjectStopY(uint8_t slot);                              /* object/richter_objects.c */

/* object/maria_objects.c */
void MariaObjectStopX(uint8_t object);
void MariaObjectStop(uint8_t object);
void MariaObjectStopY(uint8_t object);
Regs MariaObjectMarkOff(uint8_t object);
Regs MariaClearObjectsBut(void);
MExit MariaObjectPlaceByState(uint8_t object);
Regs MariaClaimKind3Slot(uint8_t object);
MExit MariaObjectStartAnimation(uint8_t object, uint8_t animation);
Regs MariaObjectHitByChecked(uint8_t object);
Regs MariaObjectAbovePlayer(uint8_t object);
Regs MariaObjectFrameFromPlayer(uint8_t object);
Regs MariaObjectVelocityFromWork(uint8_t object);
Regs MariaStartScriptSlot3(uint8_t object, uint8_t script);
Regs MariaObjectFarOffScreen(uint8_t object);
Regs MariaObjectFrame24Next(uint8_t object);
Regs MariaObjectFrameByPlayerFrame(uint8_t object);
Regs MariaObjectFrame25(uint8_t object);

#endif
