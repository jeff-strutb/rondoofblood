/*
 * Lot L5: the stage-specific object code of the stage overlays, as typed C. A body takes the
 * object's slot and returns, as a plain value, the byte its last instruction left N and Z from;
 * its abi_ wrapper turns that into the flags the assembly leaves.
 */
#ifndef RONDO_ENEMY_OBJECTS_H
#define RONDO_ENEMY_OBJECTS_H

#include "machine.h"
#include "ram.h"
#include "access.h"

/*
 * How a routine's last instructions left the registers and flags the caller can see: a plain
 * value a body returns beside its work, turned into the processor's state by abi_ended. Only
 * what the routine changed is marked.
 */
typedef struct {
    bool a_set;      /* A holds a */
    uint8_t a;
    bool nz_set;     /* N and Z come from nz */
    uint8_t nz;
    bool c_set;      /* C is c */
    bool c;
    bool v_set;      /* V is v */
    bool v;
    bool y_set;      /* Y holds y */
    uint8_t y;
} Ended;

/* Nothing was changed: the caller's A, flags and Y stand. */
static inline Ended ended_kept(void)
{
    Ended e = { 0 };

    return e;
}

/* LDA, AND, ORA: A and N, Z from the value. */
static inline Ended ended_load(uint8_t value)
{
    Ended e = { 0 };

    e.a_set = true;
    e.a = value;
    e.nz_set = true;
    e.nz = value;
    return e;
}

/* INC, DEC of memory, LDY: N and Z from the value only. */
static inline Ended ended_nz(uint8_t value)
{
    Ended e = { 0 };

    e.nz_set = true;
    e.nz = value;
    return e;
}

/* CMP: A is a; N, Z and C from the compare. */
static inline Ended ended_compare(uint8_t a, uint8_t operand)
{
    Ended e = { 0 };

    e.a_set = true;
    e.a = a;
    e.nz_set = true;
    e.nz = (uint8_t)(a - operand);
    e.c_set = true;
    e.c = a >= operand;
    return e;
}

/* The same state with C as given (a SEC, a CLC, or the carry out of an add or a shift). */
static inline Ended ended_with_carry(Ended e, bool carry)
{
    e.c_set = true;
    e.c = carry;
    return e;
}

/* The same state with V as given (the overflow of an add or a subtract). */
static inline Ended ended_with_overflow(Ended e, bool overflow)
{
    e.v_set = true;
    e.v = overflow;
    return e;
}

/* The same state with A as given (an LDA that came before the last INC or DEC). */
static inline Ended ended_with_a(Ended e, uint8_t a)
{
    e.a_set = true;
    e.a = a;
    return e;
}

/* The same state with Y as given. */
static inline Ended ended_with_y(Ended e, uint8_t y)
{
    e.y_set = true;
    e.y = y;
    return e;
}

/* The abi_ wrappers' one place that meets the registers. */
static inline void abi_ended(Ended e)
{
    if (e.a_set) {
        cpu.a = e.a;
    }
    if (e.nz_set) {
        abi_nz(e.nz);
    }
    if (e.c_set) {
        abi_flag(P_C, e.c);
    }
    if (e.v_set) {
        abi_flag(P_V, e.v);
    }
    if (e.y_set) {
        cpu.y = e.y;
    }
}

/* The V an add or a subtract leaves: a, b the operands, result the byte it gave. */
static inline bool add_overflow(uint8_t a, uint8_t b, uint8_t result)
{
    return (((a ^ result) & (b ^ result)) & 0x80) != 0;
}

static inline bool sub_overflow(uint8_t a, uint8_t b, uint8_t result)
{
    return (((a ^ b) & (a ^ result)) & 0x80) != 0;
}

/*
 * Bodies several stage files share (one C body for every near copy). Each takes the object's
 * slot and says how it ended.
 */

/* The timer counts down; at zero the object enters `state`. */
static inline Ended countdown_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    if (left != 0) {
        return ended_nz(left);
    }
    WR8(OBJ->state[slot], state);
    return ended_load(state);
}

/* The same, and at zero the object's f3408 and f3460 are cleared first. */
static inline Ended countdown_stop_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    if (left != 0) {
        return ended_nz(left);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], state);
    return ended_load(state);
}

/*
 * A hop: X speed from a byte table by an index the caller read (which way the object faces), Y speed -3.00, the frame, f35c0
 * set to 8 and the step byte advanced. Returns N, Z from the step; A 8; Y the index byte.
 */
static inline Ended hop_from_index(uint8_t slot, uint8_t index, uint16_t table, uint8_t frame)
{
    uint8_t facing = index;
    uint8_t step;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(table + facing)));
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->frame[slot], frame);
    WR8(OBJ->f35c0[slot], 8);
    step = RD8(OBJ->f2800[slot]);
    step = (uint8_t)(step + 1);
    WR8(OBJ->f2800[slot], step);
    return ended_with_y(ended_with_a(ended_nz(step), 8), facing);
}

/* The object becomes `type` in state 0. */
static inline Ended become_type(uint8_t slot, uint8_t type)
{
    WR8(OBJ->kind[slot], type);
    WR8(OBJ->state[slot], 0);
    return ended_load(type);
}

/* The object enters `state` when bit 7 of its f31a0 is set. */
static inline Ended enter_state_if_flagged(uint8_t slot, uint8_t state)
{
    uint8_t flags = RD8(OBJ->status_flags[slot]);
    uint8_t bit = (uint8_t)(flags & 0x80);

    if (bit == 0) {
        return ended_load(0);
    }
    WR8(OBJ->state[slot], state);
    return ended_load(state);
}

/* PROTOTYPES BEGIN */
/* object/enemies_stage04_temp.c */
Ended ClampSpeedXToTwo(uint8_t slot);
Ended ThrowUpward(uint8_t slot);
Ended CountdownThenState4(uint8_t slot);
Ended ChangeToType9B(uint8_t slot);
Ended EnterState6IfFlagged(uint8_t slot);
Ended CountdownStopThenState4(uint8_t slot);
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit);
/* object/enemies_stage05_temp.c */
Ended SelectRasterStage1(void);
Ended SetHitboxDxFromFrame(uint8_t slot);
Ended PhaseTimerToState1(uint8_t slot);
Ended SetHitboxFromFrameRows(uint8_t slot);
Ended EnterState7OnFlagOrYSignChange(uint8_t slot);
Ended SetHitboxFromFrameRowsIndirect(uint8_t slot);
Ended LaunchHopByF3510(uint8_t slot);
Ended CountdownStopThenState3(uint8_t slot);
/* object/enemies_stage07_temp.c */
Ended EnterState4IfFlagged(uint8_t slot);
Ended ScrollNotAtRightEnd(void);
Ended SetHitboxSizeFromFrame(uint8_t slot);
Ended ChangeToType9C(uint8_t slot);
/* object/enemies_stage08_temp.c */
Ended SetPhase2(uint8_t slot);
Ended SetPhase3(uint8_t slot);
Ended SetPhase4(uint8_t slot);
Ended SetSpeedFromPhaseTable(uint8_t slot);
Ended EnterState6IfFlaggedB(uint8_t slot);
Ended AdvanceState(uint8_t slot);
Ended EnterState3WithTallHitbox(uint8_t slot);
Ended SetHitboxFromFrameRowsB(uint8_t slot);
/* object/enemies_stage0b_temp.c */
Ended CountdownThenState4B(uint8_t slot);
Ended ChangeToType9BB(uint8_t slot);
Ended SixteenBitTimerToState8(uint8_t slot);
Ended NoOpStateA(void);
Ended SetXSpeedFromFacing(uint8_t slot);
Ended EnterState6WhenHitHard(uint8_t slot);
Ended EnterState7IfYHighOrFlagged(uint8_t slot);
Ended NoOpStateB(void);
Ended LaunchHopByFacing(uint8_t slot);
Ended CountdownStopThenState2(uint8_t slot);
/* object/state_countdown.c */
uint8_t CountdownNextStepA(uint8_t slot);
uint8_t CountdownNextStateA(uint8_t slot);
uint8_t CountdownNextStateB(uint8_t slot);
uint8_t CountdownNextStepB(uint8_t slot);
uint8_t CountdownNextStateC(uint8_t slot);
uint8_t CountdownNextStateD(uint8_t slot);
uint8_t CountdownNextStateE(uint8_t slot);
uint8_t CountdownNextStepC(uint8_t slot);
/* PROTOTYPES END */

#endif
