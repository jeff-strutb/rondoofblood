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
#include "routines.h"

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


/* The exit state of what was before, with every field the later instruction set put over it. */
static inline Ended ended_then(Ended before, Ended after)
{
    if (after.a_set) {
        before.a_set = true;
        before.a = after.a;
    }
    if (after.nz_set) {
        before.nz_set = true;
        before.nz = after.nz;
    }
    if (after.c_set) {
        before.c_set = true;
        before.c = after.c;
    }
    if (after.v_set) {
        before.v_set = true;
        before.v = after.v;
    }
    if (after.y_set) {
        before.y_set = true;
        before.y = after.y;
    }
    return before;
}

/*
 * Callees that are typed C elsewhere and that no header declares. The defining file is named; each
 * ended_ builder below gives the state its abi_ wrapper leaves, from the typed result alone.
 */


/* A routine that may play a sound: what it ended with, and the call the wrapper must show the leak of. */
typedef struct {
    Ended before;   /* what the instructions before the call left (the A that holds the command) */
    Ended end;
    bool sounded;
    uint8_t command;
    CommandCall call;
} SoundEnd;

/* The wrapper of such a routine: the sound call's flags first, then what the routine's own instructions changed. */
static inline void abi_sound_end(SoundEnd r)
{
    if (r.sounded) {
        abi_ended(r.before);
        sound_command_exit_shim(r.command, r.call);
    }
    abi_ended(r.end);
}

/* An add or subtract's result: A the byte, N and Z from it, C and V from the add. */
static inline Ended ended_of_sum(Sum sum)
{
    return ended_with_overflow(ended_with_carry(ended_load(sum.value), sum.carry), sum.overflow);
}

/* NextRandom: A the byte, N and Z from it, C and V from its last add. */
static inline Ended ended_of_random(RandomResult r)
{
    return ended_with_overflow(ended_with_carry(ended_load(r.value), r.carry), r.overflow);
}

/* SideOfPlayer: A the side bits, N and Z from them, C from the last compare when one ran. */
static inline Ended ended_of_side(SideResult r)
{
    Ended e = ended_load(r.sides);

    return r.carry_set ? ended_with_carry(e, r.carry) : e;
}

/* DistanceXToPlayer and DistanceYToPlayer: A the distance; C and V from the subtract when near. */
static inline Ended ended_of_distance(PlayerDistance d)
{
    Ended e = ended_load(d.distance);

    if (!d.far) {
        e = ended_with_carry(e, d.object_further);
        e = ended_with_overflow(e, sub_overflow(d.mine, d.player, (uint8_t)(d.mine - d.player)));
    }
    return e;
}

/* How a routine that may play a sound on the first screen ended: the state before the call, the call, the rest. */
typedef struct {
    Ended before;
    Ended end;
    bool called;
    SoundOnScreen sound;
} ScreenSoundEnd;

static inline void abi_screen_sound_end(ScreenSoundEnd r)
{
    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
    }
    abi_ended(r.end);
}


/* A routine whose exit keeps what NextRandom left in C and V; the wrapper shows that first. */
typedef struct {
    Ended end;
    bool random_used;
    RandomResult random;
} RandomEnd;

static inline void abi_random_end(RandomEnd r)
{
    if (r.random_used) {
        random_exit_shim(r.random);
    }
    abi_ended(r.end);
}

/* The same for SideOfPlayer's carry. */
typedef struct {
    Ended end;
    bool side_used;
    SideResult side;
} SideEnd;

static inline void abi_side_end(SideEnd r)
{
    if (r.side_used) {
        side_of_player_exit_shim(r.side);
    }
    abi_ended(r.end);
}

/* StepObjectFrames: A $FF and C 1 when the frame changed, A 0 and C 0 when the list ended, A $FF and C 0 when held. */
static inline Ended ended_of_frame_step(FrameStep step, bool y_changed, uint8_t y)
{
    Ended e = ended_load(step == FRAME_LIST_ENDED ? 0x00 : 0xFF);

    e = ended_with_carry(e, step == FRAME_CHANGED);
    if (y_changed) {
        e = ended_with_y(e, y);
    }
    return e;
}

/* ApplyVelocity, ApplyVelocityX and ApplyVelocityY: A the position's high byte, N, Z, C and V from its add. */
static inline Ended ended_of_axis_move(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);
    Ended e = ended_load(result);

    e = ended_with_carry(e, sum > 0xFFFFFF);
    return ended_with_overflow(e, (~(sign ^ high) & (sign ^ result) & 0x80) != 0);
}

/* LoadObjectRecord: A the record's last byte, Y 11, N and Z from the new state, C from the record. */
static inline Ended ended_of_record(uint8_t last, uint8_t nz, bool carry)
{
    Ended e = ended_with_y(ended_load(last), 11);

    e.nz = nz;
    return ended_with_carry(e, carry);
}

/* How a routine that steps the animation ended: what the step did and the state it leaves. */
typedef struct {
    FrameStep step;
    Ended end;
} FrameStepEnd;

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
/* object/enemies_common_temp.c */
ScreenSoundEnd PlayHitSoundA(uint8_t slot);
ScreenSoundEnd PlayHitSoundB(uint8_t slot);
ScreenSoundEnd PlayHitSoundC(uint8_t slot);
ScreenSoundEnd PlayHitSoundD(uint8_t slot);
ScreenSoundEnd PlayHitSoundE(uint8_t slot);
ScreenSoundEnd PlayHitSoundF(uint8_t slot);
ScreenSoundEnd PlayHitSoundG(uint8_t slot);
ScreenSoundEnd PlayHitSoundH(uint8_t slot);
ScreenSoundEnd PlayHitSoundI(uint8_t slot);
ScreenSoundEnd PlayHitSoundJ(uint8_t slot);
ScreenSoundEnd PlayHitSoundK(uint8_t slot);
ScreenSoundEnd PlayHitSoundL(uint8_t slot);
Ended DropItemWhenHitA(uint8_t slot);
Ended DropItemWhenHitB(uint8_t slot);
Ended DropItemWhenHitC(uint8_t slot);
Ended DropItemWhenHitD(uint8_t slot);
Ended DropItemWhenHitE(uint8_t slot);
/* object/enemies_stage04_temp.c */
Ended ClampSpeedXToTwo(uint8_t slot);
Ended ThrowUpward(uint8_t slot);
Ended CountdownThenState4(uint8_t slot);
Ended ChangeToType9B(uint8_t slot);
Ended EnterState6IfFlagged(uint8_t slot);
Ended CountdownStopThenState4(uint8_t slot);
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit);
Ended LoadRecordStartAngle40(uint8_t slot);
Ended LoadRecordTimer50(uint8_t slot);
FrameStepEnd StepFramesFacingPlayer(uint8_t slot, uint8_t index);
FrameStepEnd StepFramesFacingPlayerAlways(uint8_t slot, uint8_t index);
Ended WalkWhileFrameChanges(uint8_t slot);
Ended WaitThenState2(uint8_t slot);
Ended BecomeType9CWhenAnimationEnds(uint8_t slot);
/* object/enemies_stage05_temp.c */
Ended SelectRasterStage1(void);
Ended SetHitboxDxFromFrame(uint8_t slot);
Ended PhaseTimerToState1(uint8_t slot);
Ended SetHitboxFromFrameRows(uint8_t slot);
Ended EnterState7OnFlagOrYSignChange(uint8_t slot);
Ended SetHitboxFromFrameRowsIndirect(uint8_t slot);
Ended LaunchHopByF3510(uint8_t slot);
Ended CountdownStopThenState3(uint8_t slot);
Ended StartWithFrame1A(uint8_t slot);
Ended ThrowByNextKind(uint8_t slot);
Ended AnimateThenState1(uint8_t slot);
Ended AnimateThenNextStep(uint8_t slot);
Ended FollowParentYAndLeave(uint8_t slot);
Ended AnimateThenNextStepB(uint8_t slot);
Ended CountdownThenFallOrBecomeType9B(uint8_t slot);
RandomEnd PickTimersAtRandom(uint8_t slot);
RandomEnd PickAngleAtRandom(uint8_t slot);
Ended AnimateMoveAndSag(uint8_t slot);
FrameStepEnd StepFramesFacingPlayerC(uint8_t slot, uint8_t index);
SideEnd FaceThePlayerInSomeStates(uint8_t slot);
Ended NextPhaseAtRandom(uint8_t slot);
Ended ShowOnScreenAndWrapX(uint8_t slot);
Ended StartChargeWhenPlayerNear(uint8_t slot);
Ended WaitThenState2B(uint8_t slot);
Ended ChooseNextMoveWhenFacingAway(uint8_t slot);
/* object/enemies_stage07_temp.c */
Ended EnterState4IfFlagged(uint8_t slot);
Ended ScrollNotAtRightEnd(void);
Ended SetHitboxSizeFromFrame(uint8_t slot);
Ended ChangeToType9C(uint8_t slot);
Ended EnterNextStateWhenPlayerNear(uint8_t slot);
Ended SwingOnCosine(uint8_t slot);
Ended AnimateThenFrameRecord(uint8_t slot);
Ended StartThenAnimate(uint8_t slot);
/* object/enemies_stage08_temp.c */
Ended SetPhase2(uint8_t slot);
Ended SetPhase3(uint8_t slot);
Ended SetPhase4(uint8_t slot);
Ended SetSpeedFromPhaseTable(uint8_t slot);
Ended EnterState6IfFlaggedB(uint8_t slot);
Ended AdvanceState(uint8_t slot);
Ended EnterState3WithTallHitbox(uint8_t slot);
Ended SetHitboxFromFrameRowsB(uint8_t slot);
Ended StartFacingPlayerThenCheck(uint8_t slot);
Ended AnimateThenCountToState3(uint8_t slot);
Ended PickPhaseAtRandomA(uint8_t slot);
Ended PickPhaseAtRandomB(uint8_t slot);
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
FrameStepEnd StepFramesFacingPlayerD(uint8_t slot, uint8_t index);
Ended WaitThenState2C(uint8_t slot);
Ended BecomeType9CWhenAnimationEndsB(uint8_t slot);
Ended LoadRecordStartAngle40B(uint8_t slot);
Ended FrameOneBWhenFallingElseAnimate(uint8_t slot);
SideEnd FaceThePlayerInSomeStatesB(uint8_t slot);
Ended AnimateAlongListC3C2(uint8_t slot);
Ended SwingOnCosineB(uint8_t slot);
Ended PickSpeedAndTurnFromTable(uint8_t slot);
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
