/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4C (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it; each routine's exit shim builds how its last instructions left A, Y and the flags (Ended,
 * enemy_objects.h), and the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The arms the routines end on (L8Arm, or the routine's own struct). */
enum { ST18_TIMER_COUNTED, ST18_TIMER_PICKED };
enum { ST18_LUCK_COUNTED, ST18_LUCK_STEPPED, ST18_LUCK_PICKED_AT_FRAME, ST18_LUCK_PICKED_AT_END };
enum { ST18_LEAP_STEPPED, ST18_LEAP_SET, ST18_LEAP_REVERSED };
enum { ST18_STATE5_COUNTING, ST18_STATE5_SET };
enum { ST18_STATE5_STEPPED, ST18_STATE5_ENDED };

/*
 * @routine 0B4C:007 CountDownOrPickNextTimer
 * @run 70:B807
 * WHAT IT DOES: Counts OBJ_34B8 down and says no (C 0). At zero it clears the animation and OBJ_3778, goes to state 7, advances OBJ_35C0, takes OBJ_34B8 from the table at $B830 by its low two bits and says yes (C 1).
 * HOW: A test, a memory decrement, stores, an INC, a table read.
 * WAITS: no.
 * Entry: X object. Exit: C 1 when the timer ran out, 0 when not; N and Z from the last load or decrement (l8_count_down_or_pick_exit_shim).
 */
L8Arm CountDownOrPickNextTimer(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t step;

    r.first = RD8(OBJ->f34b8[slot]);
    if (r.first != 0) {
        r.second = DEC8(OBJ->f34b8[slot]);
        r.arm = ST18_TIMER_COUNTED;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 7);
    WR8(OBJ->f3778[slot], 0);
    r.second = INC8(OBJ->f35c0[slot]);
    step = RD8(OBJ->f35c0[slot]);
    r.y = step & 0x03;
    r.third = rd8((uint16_t)(0xB830 + r.y));
    WR8(OBJ->f34b8[slot], r.third);
    r.arm = ST18_TIMER_PICKED;
    return r;
}

/* What CountDownOrPickNextTimer leaves in A, Y and the flags, from the arm it ended on (first the timer, second the DEC or INC, y the pick, third the timer taken). */
Ended l8_count_down_or_pick_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == ST18_TIMER_COUNTED) {
        e = then(e, ended_nz(r.second));
        return ended_with_carry(e, false);
    }
    e = then(e, ended_load(7));
    e = then(e, ended_nz(r.second));
    e = then(e, ended_with_y(ended_load(r.y), r.y));
    return ended_with_carry(then(e, ended_load(r.third)), true);
}

void abi_CountDownOrPickNextTimer(void)
{
    abi_ended(l8_count_down_or_pick_exit_shim(CountDownOrPickNextTimer(cpu.x)));
}

/*
 * @routine 0B4C:051 StepListThenPickStateByLuck
 * @run 70:B851
 * WHAT IT DOES: In state 0, when the frame is 4, advances OBJ_SUBSTATE; otherwise (and in later states when the animation list at $B8D2 ends) it takes frame 2, the facing of the player's side, clears the animation timer and picks the next state and OBJ_SUBSTATE from the tables at $B8A2 and $B89D by OBJ_HEALTH and a random number.
 * HOW: StepObjectFrames, SideOfPlayer, NextRandom, table reads, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_luck_exit_shim). The state 0 arm ran in no script; the draft stops there.
 */
L8LuckArm StepListThenPickStateByLuck(uint8_t slot, uint8_t y_entry)
{
    L8LuckArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.frame = RD8(OBJ->frame[slot]);
        if (r.frame == 4) {
            r.count = INC8(OBJ->substate[slot]);
            r.arm = ST18_LUCK_COUNTED;
            return r;
        }
        r.arm = ST18_LUCK_PICKED_AT_FRAME;
    } else {
        r.step = l8_list_step(slot, 0xB8D2, 0xB86C, true, y_entry);
        if (r.step.step != FRAME_LIST_ENDED) {
            r.arm = ST18_LUCK_STEPPED;
            return r;
        }
        r.arm = ST18_LUCK_PICKED_AT_END;
    }
    WR8(OBJ->frame[slot], 2);
    jsr_push(0xB876);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.side_bit = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.side_bit);
    WR8(OBJ->anim_timer[slot], 0);
    r.shifted = RD8(OBJ->health[slot]);
    r.aim = (uint8_t)((r.shifted >> 1) & 0xF0);
    WR8(ZP->temp00, r.aim);
    jsr_push(0xB889);
    r.random = NextRandom();
    jsr_pop();
    r.pick = r.random.value & 0x0F;
    r.sum = add_bytes(r.pick, RD8(ZP->temp00), false);
    r.index = rd8((uint16_t)(0xB8A2 + r.sum.value));
    r.value = rd8((uint16_t)(0xB89D + r.index));
    WR8(OBJ->state[slot], r.value);
    WR8(OBJ->substate[slot], 0);
    return r;
}

/* What StepListThenPickStateByLuck leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_luck_exit_shim(L8LuckArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == ST18_LUCK_COUNTED) {
        e = then(e, ended_compare(r.frame, 4));
        return then(e, ended_nz(r.count));
    }
    if (r.arm == ST18_LUCK_STEPPED) {
        return then(e, l8_step_ended(r.step));
    }
    if (r.arm == ST18_LUCK_PICKED_AT_FRAME) {
        e = then(e, ended_compare(r.frame, 4));
    } else {
        e = then(e, l8_step_ended(r.step));
    }
    e = then(e, ended_load(2));
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.side_bit));
    e = then(e, ended_with_carry(ended_load(r.aim), (r.shifted & 0x01) != 0));
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load(r.pick));
    e = then(e, ended_with_y(ended_of_sum(r.sum), r.sum.value));
    e = then(e, ended_with_y(ended_load(r.index), r.index));
    return then(e, ended_load(r.value));
}

void abi_StepListThenPickStateByLuck(void)
{
    abi_ended(l8_luck_exit_shim(StepListThenPickStateByLuck(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:0EF StepListThenLeapByDistance
 * @run 70:B8EF
 * WHAT IT DOES: Steps the animation list at $B936; when it ends plays sound $B5, clears the animation, and from the distance to the player in X (eight steps) sets the speed from the table at $B93F, goes to the next OBJ_SUBSTATE and, when facing left, reverses the X speed.
 * HOW: StepObjectFrames, SoundCommand, DistanceXToPlayer, table reads, stores, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_leap_distance_exit_shim). Reversing the speed ran in no script; the draft stops there.
 */
L8LeapDistanceArm StepListThenLeapByDistance(uint8_t slot, uint8_t y_entry)
{
    L8LeapDistanceArm r = { 0 };
    uint8_t value;

    r.step = l8_list_step(slot, 0xB936, 0xB8FA, true, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST18_LEAP_STEPPED;
        return r;
    }
    jsr_push(0xB901);
    r.call = SoundCommand(0xB5, slot, r.step.y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xB90A);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.index = (uint8_t)((r.distance.distance >> 5) << 2);
    value = rd8((uint16_t)(0xB940 + r.index));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB93F + r.index));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xB942 + r.index));
    WR8(OBJ->vy_hi[slot], value);
    r.value = rd8((uint16_t)(0xB941 + r.index));
    WR8(OBJ->vy_lo[slot], r.value);
    r.count = INC8(OBJ->substate[slot]);
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.facing != 0) {
        jsr_push(0xB935);
        r.negated = NegateVelocityX(slot);
        jsr_pop();
        r.arm = ST18_LEAP_REVERSED;
        return r;
    }
    r.arm = ST18_LEAP_SET;
    return r;
}

/* What StepListThenLeapByDistance leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_leap_distance_exit_shim(L8LeapDistanceArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST18_LEAP_STEPPED) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xB5;
    s.before = then(e, ended_load(0xB5));
    s.call = r.call;
    e = ended_of_distance(r.distance);
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.index), false), r.index));
    e = then(e, ended_load(r.value));
    e = then(e, ended_nz(r.count));
    e = then(e, ended_load(r.facing));
    if (r.arm == ST18_LEAP_REVERSED) {
        e = then(e, ended_of_sum(r.negated));
    }
    s.end = e;
    return s;
}

void abi_StepListThenLeapByDistance(void)
{
    abi_sound_end(l8_leap_distance_exit_shim(StepListThenLeapByDistance(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:18A CountDownThenState5
 * @run 70:B98A
 * WHAT IT DOES: Counts OBJ_3510 down; at zero clears the animation and goes to state 5.
 * HOW: A memory decrement, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_state5_count_exit_shim).
 */
L8Arm CountDownThenState5(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = DEC8(OBJ->f3510[slot]);
    if (r.first != 0) {
        r.arm = ST18_STATE5_COUNTING;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    r.arm = ST18_STATE5_SET;
    return r;
}

/* What CountDownThenState5 leaves in A and the flags, from the arm it ended on (first the timer after the DEC). */
static Ended l8_state5_count_exit_shim(L8Arm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == ST18_STATE5_COUNTING) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_CountDownThenState5(void)
{
    abi_ended(l8_state5_count_exit_shim(CountDownThenState5(cpu.x)));
}

/*
 * @routine 0B4C:336 SoundAndTileWriteAtFrame24
 * @run 70:BB36
 * WHAT IT DOES: At frame $24 plays sound $19 and writes two tile words into the background map at the column the object's X names (its position plus the scroll less 2, divided by 8, low six bits).
 * HOW: SoundCommand, SelectVramWriteAddress, a 16-bit add and three shifts, VDC writes, ClearVdcControlHigh, SelectVramData.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: A $51, C from the last rotate, V from the subtract (l8_tile_write_24_exit_shim).
 */
L8TileWriteArm SoundAndTileWriteAtFrame24(uint8_t slot, uint8_t y_entry)
{
    uint8_t low, high, shifted;
    Sum sum;
    L8TileWriteArm r = { 0 };

    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame != 0x24) {
        r.wrote = false;
        return r;
    }
    jsr_push(0xBB42);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBB45);
    SelectVramWriteAddress();
    jsr_pop();
    low = RD8(OBJ->x_lo[slot]);
    sum = add_bytes(low, RD8(SCROLL->x_lo[0]), false);
    WR8(ZP->temp00, sum.value);
    high = RD8(OBJ->x_hi[slot]);
    sum = add_bytes(high, RD8(SCROLLE->x_hi_screen[0]), sum.carry);
    r.diff = sub_bytes(sum.value, 2, true);
    WR8(ZP->temp01, r.diff.value);
    r.carry_out = LSR8(ZP->temp01);
    r.carry_out = ROR8(ZP->temp00, r.carry_out != 0);
    r.carry_out = LSR8(ZP->temp01);
    r.carry_out = ROR8(ZP->temp00, r.carry_out != 0);
    r.carry_out = LSR8(ZP->temp01);
    r.carry_out = ROR8(ZP->temp00, r.carry_out != 0);
    shifted = (uint8_t)(RD8(ZP->temp00) & 0x3F);
    WR8(VDC->data_lo, shifted);
    WR8(VDC->data_hi, 0x06);
    jsr_push(0xBB74);
    ClearVdcControlHigh();
    jsr_pop();
    jsr_push(0xBB77);
    SelectVramData();
    jsr_pop();
    WR8(VDC->data_lo, 0x8F);
    WR8(VDC->data_hi, 0x51);
    WR8(VDC->data_lo, 0x8F);
    WR8(VDC->data_hi, 0x51);
    r.wrote = true;
    return r;
}

/* What SoundAndTileWriteAtFrame24 leaves, the sound call's included, from the arm it ended on. */
SoundEnd l8_tile_write_24_exit_shim(L8TileWriteArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_compare(r.frame, 0x24);

    if (!r.wrote) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x19;
    s.before = then(e, ended_load(0x19));
    s.call = r.call;
    s.end = ended_with_overflow(ended_with_carry(ended_load(0x51), r.carry_out != 0), r.diff.overflow);
    return s;
}

void abi_SoundAndTileWriteAtFrame24(void)
{
    abi_sound_end(l8_tile_write_24_exit_shim(SoundAndTileWriteAtFrame24(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:38C SoundAndSpawnFiveAtFrame27
 * @run 70:BB8C
 * WHAT IT DOES: At frame $27 plays sound $B5 and starts up to five objects of type $27 in free slots of $20 to $3F at its place, each facing as this one and numbered 0 to 4 in OBJ_34B8.
 * HOW: SoundCommand, a loop of FindFreeSlotFrom20 and StartObjectAtPlace with a counter in zTemp00.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_spawn_five_27_exit_shim).
 */
L8SpawnFiveArm SoundAndSpawnFiveAtFrame27(uint8_t slot, uint8_t y_entry)
{
    uint8_t facing;
    L8SpawnFiveArm r = { 0 };

    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame != 0x27) {
        r.arm = L8_FIVE_NOT_FRAME;
        return r;
    }
    jsr_push(0xBB98);
    r.call = SoundCommand(0xB5, slot, y_entry);
    jsr_pop();
    WR8(ZP->temp00, 0);
    for (;;) {
        jsr_push(0xBB9D);
        r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
        jsr_pop();
        if (!r.found.found) {
            r.arm = L8_FIVE_NONE_FREE;
            return r;
        }
        jsr_push(0xBBA4);
        r.placed = StartObjectAtPlace(0x27, slot, r.found.slot);
        jsr_pop();
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[r.found.slot], facing);
        r.number = RD8(ZP->temp00);
        WR8(OBJ->f34b8[r.found.slot], r.number);
        if (r.number == 4) {
            r.arm = L8_FIVE_ALL_STARTED;
            return r;
        }
        INC8(ZP->temp00);
    }
}

/* What SoundAndSpawnFiveAtFrame27 leaves, the sound call's included, from the arm it ended on (the last pass of the loop). */
SoundEnd l8_spawn_five_27_exit_shim(L8SpawnFiveArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_compare(r.frame, 0x27);

    if (r.arm == L8_FIVE_NOT_FRAME) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xB5;
    s.before = then(e, ended_load(0xB5));
    s.call = r.call;
    e = l8_slot_ended(r.found);
    if (r.arm == L8_FIVE_ALL_STARTED) {
        e = then(e, ended_load(r.placed));
        e = then(e, ended_compare(r.number, 4));
    }
    s.end = e;
    return s;
}

void abi_SoundAndSpawnFiveAtFrame27(void)
{
    abi_sound_end(l8_spawn_five_27_exit_shim(SoundAndSpawnFiveAtFrame27(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:409 StepListThenState5
 * @run 70:BC09
 * WHAT IT DOES: Steps the animation list at $BC22; when it ends clears the animation and goes to state 5.
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_state5_step_exit_shim).
 */
L8Arm StepListThenState5(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBC22, 0xBC14, true, y_entry);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = ST18_STATE5_STEPPED;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    r.arm = ST18_STATE5_ENDED;
    return r;
}

/* What StepListThenState5 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_state5_step_exit_shim(L8Arm r)
{
    Ended e = l8_step_ended(r.step);

    if (r.arm == ST18_STATE5_STEPPED) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_StepListThenState5(void)
{
    abi_ended(l8_state5_step_exit_shim(StepListThenState5(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:447 SoundThenLeapByFacing
 * @run 70:BC47
 * WHAT IT DOES: Plays sound $B8, gives the object the X speed from the table at $BC6B by its facing and the Y speed -$0340 (high $FC, low $C0) and goes to the next OBJ_SUBSTATE.
 * HOW: SoundCommand, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_leap_facing_exit_shim).
 */
L8LeapFacingArm SoundThenLeapByFacing(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    L8LeapFacingArm r = { 0 };

    jsr_push(0xBC4C);
    r.call = SoundCommand(0xB8, slot, y_entry);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    r.index = (uint8_t)(r.facing << 1);
    value = rd8((uint16_t)(0xBC6C + r.index));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBC6B + r.index));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0xC0);
    r.count = INC8(OBJ->substate[slot]);
    return r;
}

/* What SoundThenLeapByFacing leaves, the sound call's included: one path. */
static SoundEnd l8_leap_facing_exit_shim(L8LeapFacingArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());

    s.sounded = true;
    s.command = 0xB8;
    s.before = ended_load(0xB8);
    s.call = r.call;
    s.end = then(ended_with_y(ended_with_carry(ended_load(0xC0), r.facing >= 0x80), r.index), ended_nz(r.count));
    return s;
}

void abi_SoundThenLeapByFacing(void)
{
    abi_sound_end(l8_leap_facing_exit_shim(SoundThenLeapByFacing(cpu.x, cpu.y)));
}

/*
 * @routine 0B4C:5BB MaybeSound19AndSpawnPiece
 * @run 70:BDBB
 * WHAT IT DOES: When the low two bits of A are 0 plays sound $19 and starts an object of type $9A in a free slot of $30 to $3F at the object's place at Y low $BE with OBJ_34B8 1 and OBJ_3510 a random number to 7 plus the facing.
 * HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace, NextRandom.
 * WAITS: no.
 * Entry: A the number, X object, Y kept for the sound. Exit: as the last instruction that ran (l8_sound19_piece_exit_shim).
 */
L8SpawnPieceArm MaybeSound19AndSpawnPiece(uint8_t slot, uint8_t number, uint8_t y_entry)
{
    uint8_t facing;
    L8SpawnPieceArm r = { 0 };

    r.bits = number & 0x03;
    if (r.bits != 0) {
        r.spawned = false;
        return r;
    }
    jsr_push(0xBDC4);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBDC7);
    r.found = l8_free_slot30();
    jsr_pop();
    jsr_push(0xBDCE);
    r.placed = StartObjectAtPlace(0x9A, slot, r.found.slot);
    jsr_pop();
    WR8(OBJ->y_lo[r.found.slot], 0xBE);
    WR8(OBJ->f34b8[r.found.slot], 1);
    jsr_push(0xBDDB);
    r.random = NextRandom();
    jsr_pop();
    r.pick = r.random.value & 0x07;
    facing = RD8(OBJ->facing_left[slot]);
    r.sum = add_bytes(r.pick, facing, false);
    WR8(OBJ->f3510[r.found.slot], r.sum.value);
    r.spawned = true;
    return r;
}

/* What MaybeSound19AndSpawnPiece leaves, the sound call's included, from the arm it ended on. */
SoundEnd l8_sound19_piece_exit_shim(L8SpawnPieceArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e;

    if (!r.spawned) {
        s.end = ended_load(r.bits);
        return s;
    }
    s.sounded = true;
    s.command = 0x19;
    s.before = then(ended_load(r.bits), ended_load(0x19));
    s.call = r.call;
    e = l8_slot_ended(r.found);
    e = then(e, ended_load(r.placed));
    e = then(e, ended_load(1));
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load(r.pick));
    s.end = then(e, ended_of_sum(r.sum));
    return s;
}

void abi_MaybeSound19AndSpawnPiece(void)
{
    abi_sound_end(l8_sound19_piece_exit_shim(MaybeSound19AndSpawnPiece(cpu.x, cpu.a, cpu.y)));
}
