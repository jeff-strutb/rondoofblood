/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0F6B, second part (enemy kinds not yet identified), by routine. Bodies return the
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

/* The arms the routines end on. */
enum { ST22_START_NONE_FREE, ST22_START_STARTED };
enum { ST22_FALL_LOADED, ST22_FALL_FELL, ST22_FALL_COUNTING, ST22_FALL_OTHER_KIND, ST22_FALL_SOUNDED };

/* The ending the two orbit routines share: the angle byte steps by $FF90 and, when it lands on $80 coming from another value, the object is placed at Y $40, X $B0 less the scroll and goes on. */
static L8OrbitEnd st22_orbit_step(uint8_t slot)
{
    uint8_t low, high, scroll;
    L8OrbitEnd r = { 0 };

    r.before = RD8(OBJ->f37d0[slot]);
    WR8(ZP->temp00, r.before);
    low = RD8(OBJ->f3778[slot]);
    r.sum = add_bytes(0x90, low, false);
    WR8(OBJ->f3778[slot], r.sum.value);
    high = RD8(OBJ->f37d0[slot]);
    r.sum = add_bytes(0xFF, high, r.sum.carry);
    WR8(OBJ->f37d0[slot], r.sum.value);
    r.other = RD8(ZP->temp00);
    if (r.sum.value == r.other) {
        r.arm = L8_ORBIT_SAME;
        return r;
    }
    if (r.sum.value != 0x80) {
        r.arm = L8_ORBIT_NOT_80;
        return r;
    }
    WR8(OBJ->y_lo[slot], 0x40);
    WR8(OBJ->y_frac[slot], 0);
    WR8(OBJ->f3778[slot], 0);
    scroll = RD8(SCROLL->x_lo[0]);
    r.diff = sub_bytes(0xB0, scroll, true);
    WR8(OBJ->x_lo[slot], r.diff.value);
    WR8(OBJ->x_frac[slot], 0);
    r.arm = L8_ORBIT_PLACED;
    return r;
}

/* For the orbit shims: what st22_orbit_step leaves. */
static Ended st22_orbit_ended(L8OrbitEnd r)
{
    Ended e = ended_load(r.before);

    e = then(then(e, ended_load(0xFF)), ended_of_sum(r.sum));
    e = then(e, ended_compare(r.sum.value, r.other));
    if (r.arm == L8_ORBIT_SAME) {
        return e;
    }
    e = then(e, ended_compare(r.sum.value, 0x80));
    if (r.arm == L8_ORBIT_NOT_80) {
        return e;
    }
    return then(e, ended_of_sum(r.diff));
}

/*
 * @routine 0F6B:52F OrbitCosineSineThenSettle
 * @run 70:B52F
 * WHAT IT DOES: Moves the object on an ellipse: X speed from the cosine of OBJ_37D0 scaled by $0120, Y speed from the sine scaled by $0050; turns OBJ_37D0 on by -$0070 a frame (a 16-bit add with OBJ_3778); when OBJ_37D0 reaches $80 it is placed at Y $0040, X $B0 less the scroll and goes to the next OBJ_SUBSTATE.
 * HOW: ScaleByCosine, ScaleBySine, ApplyVelocityX and Y, a 16-bit add in memory, two compares.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_orbit_settle_exit_shim).
 */
L8OrbitArm OrbitCosineSineThenSettle(uint8_t slot)
{
    uint8_t value;
    L8OrbitArm r = { 0 };

    WR8(ZP->temp00, 0x20);
    WR8(ZP->temp01, 1);
    r.angle1 = RD8(OBJ->f37d0[slot]);
    jsr_push(0xB53D);
    r.scale1 = ScaleByCosine(r.angle1);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    r.value1 = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], r.value1);
    jsr_push(0xB54A);
    r.move1 = ApplyVelocityX(slot);
    jsr_pop();
    WR8(ZP->temp00, 0x50);
    WR8(ZP->temp01, 0);
    r.angle2 = RD8(OBJ->f37d0[slot]);
    jsr_push(0xB558);
    r.scale2 = ScaleBySine(r.angle2);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    r.value2 = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], r.value2);
    jsr_push(0xB565);
    r.move2 = ApplyVelocityY(slot);
    jsr_pop();
    r.orbit = st22_orbit_step(slot);
    if (r.orbit.arm == L8_ORBIT_PLACED) {
        r.count = INC8(OBJ->substate[slot]);
    }
    return r;
}

/* What OrbitCosineSineThenSettle leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_orbit_settle_exit_shim(L8OrbitArm r)
{
    Ended e = ended_load(r.angle1);

    e = then(e, l8_of_scale(r.scale1));
    e = then(e, ended_load(r.value1));
    e = then(e, ended_of_axis_move(r.move1));
    e = then(e, ended_load(r.angle2));
    e = then(e, l8_of_scale(r.scale2));
    e = then(e, ended_load(r.value2));
    e = then(e, ended_of_axis_move(r.move2));
    e = then(e, st22_orbit_ended(r.orbit));
    if (r.orbit.arm == L8_ORBIT_PLACED) {
        e = then(e, ended_nz(r.count));
    }
    return e;
}

void abi_OrbitCosineSineThenSettle(void)
{
    abi_ended(l8_orbit_settle_exit_shim(OrbitCosineSineThenSettle(cpu.x)));
}

/*
 * @routine 0F6B:59E ResetOrbitAndAdvance
 * @run 70:B59E
 * WHAT IT DOES: Clears OBJ_3778, sets OBJ_37D0 and OBJ_35C0 $80 and goes to the next OBJ_SUBSTATE.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from the new OBJ_SUBSTATE (l8_reset_orbit_exit_shim).
 */
uint8_t ResetOrbitAndAdvance(uint8_t slot)
{
    WR8(OBJ->f3778[slot], 0);
    WR8(OBJ->f37d0[slot], 0x80);
    WR8(OBJ->f35c0[slot], 0x80);
    return INC8(OBJ->substate[slot]);
}

/* What ResetOrbitAndAdvance leaves: one path, the new OBJ_SUBSTATE behind N and Z. */
static Ended l8_reset_orbit_exit_shim(uint8_t count)
{
    return then(ended_load(0x80), ended_nz(count));
}

void abi_ResetOrbitAndAdvance(void)
{
    abi_ended(l8_reset_orbit_exit_shim(ResetOrbitAndAdvance(cpu.x)));
}

/*
 * @routine 0F6B:5B1 OrbitWithRisingSineThenSettle
 * @run 70:B5B1
 * WHAT IT DOES: As OrbitCosineSineThenSettle, but the Y speed comes from the sine of OBJ_3828 scaled by $0100, which turns on by $01C0 a frame; at $80 the object goes to OBJ_SUBSTATE 2.
 * HOW: ScaleBySine, ScaleByCosine, ApplyVelocityY and X, two 16-bit adds in memory, two compares.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_orbit_rising_exit_shim).
 */
L8OrbitArm OrbitWithRisingSineThenSettle(uint8_t slot)
{
    uint8_t value, low, high;
    L8OrbitArm r = { 0 };

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    r.angle1 = RD8(OBJ->f3828[slot]);
    jsr_push(0xB5BF);
    r.scale1 = ScaleBySine(r.angle1);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    r.value1 = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], r.value1);
    jsr_push(0xB5CC);
    r.move1 = ApplyVelocityY(slot);
    jsr_pop();
    low = RD8(OBJ->f3880[slot]);
    r.sum = add_bytes(0xC0, low, false);
    WR8(OBJ->f3880[slot], r.sum.value);
    high = RD8(OBJ->f3828[slot]);
    r.sum = add_bytes(1, high, r.sum.carry);
    WR8(OBJ->f3828[slot], r.sum.value);
    WR8(ZP->temp00, 0x20);
    WR8(ZP->temp01, 1);
    r.angle2 = RD8(OBJ->f37d0[slot]);
    jsr_push(0xB5EB);
    r.scale2 = ScaleByCosine(r.angle2);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], value);
    r.value2 = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], r.value2);
    jsr_push(0xB5F8);
    r.move2 = ApplyVelocityX(slot);
    jsr_pop();
    r.orbit = st22_orbit_step(slot);
    if (r.orbit.arm == L8_ORBIT_PLACED) {
        WR8(OBJ->substate[slot], 2);
    }
    return r;
}

/* What OrbitWithRisingSineThenSettle leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_orbit_rising_exit_shim(L8OrbitArm r)
{
    Ended e = ended_load(r.angle1);

    e = then(e, l8_of_scale(r.scale1));
    e = then(e, ended_load(r.value1));
    e = then(e, ended_of_axis_move(r.move1));
    e = then(then(e, ended_load(1)), ended_of_sum(r.sum));
    e = then(e, ended_load(r.angle2));
    e = then(e, l8_of_scale(r.scale2));
    e = then(e, ended_load(r.value2));
    e = then(e, ended_of_axis_move(r.move2));
    e = then(e, st22_orbit_ended(r.orbit));
    if (r.orbit.arm == L8_ORBIT_PLACED) {
        e = then(e, ended_load(2));
    }
    return e;
}

void abi_OrbitWithRisingSineThenSettle(void)
{
    abi_ended(l8_orbit_rising_exit_shim(OrbitWithRisingSineThenSettle(cpu.x)));
}

/*
 * @routine 0F6B:711 StepListByNumberTable
 * @run 70:B711
 * WHAT IT DOES: Steps the animation list that the table at $BA0A names for OBJ_34B8.
 * HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (l8_number_step_exit_shim).
 */
L8NumberStep StepListByNumberTable(uint8_t slot, uint8_t y_entry)
{
    L8NumberStep r = { 0 };

    (void)y_entry;
    r.number = RD8(OBJ->f34b8[slot]);
    r.index = (uint8_t)(r.number << 1);
    r.lo = rd8((uint16_t)(0xBA0A + r.index));
    WR8(ZP->temp10, r.lo);
    r.hi = rd8((uint16_t)(0xBA0B + r.index));
    WR8(ZP->temp11, r.hi);
    r.y = r.index;
    r.step = StepObjectFrames(slot, &r.y);
    return r;
}

/* What StepListByNumberTable leaves in A, Y and the flags: one path, the step's. */
Ended l8_number_step_exit_shim(L8NumberStep r)
{
    Ended e = ended_with_y(ended_with_carry(ended_load(r.index), r.number >= 0x80), r.index);

    e = then(e, ended_load(r.lo));
    e = then(e, ended_load(r.hi));
    return then(e, ended_of_frame_step(r.step, r.step != FRAME_HELD, r.y));
}

void abi_StepListByNumberTable(void)
{
    abi_ended(l8_number_step_exit_shim(StepListByNumberTable(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:70B ClearAnimationThenStepListByNumber
 * @run 70:B70B
 * WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTable.
 * HOW: Two stores, running into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTable (l8_number_step_exit_shim).
 */
L8NumberStep ClearAnimationThenStepListByNumber(uint8_t slot, uint8_t y_entry)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return StepListByNumberTable(slot, y_entry);
}

void abi_ClearAnimationThenStepListByNumber(void)
{
    abi_ended(l8_number_step_exit_shim(ClearAnimationThenStepListByNumber(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:723 StartPieceABInFreeSlot
 * @run 70:B723
 * WHAT IT DOES: Starts an object of type $AB in a free slot of $30 to $3F at the object's place and says yes (C 1); with no free slot says no (C 0).
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: C 1 started, 0 none (l8_start_ab_exit_shim).
 */
L8Arm StartPieceABInFreeSlot(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xB726);
    r.found = l8_free_slot30();
    jsr_pop();
    if (!r.found.found) {
        r.arm = ST22_START_NONE_FREE;
        return r;
    }
    jsr_push(0xB72D);
    r.first = StartObjectAtPlace(0xAB, slot, r.found.slot);
    jsr_pop();
    r.arm = ST22_START_STARTED;
    return r;
}

/* What StartPieceABInFreeSlot leaves in A, Y and the flags, from the arm it ended on (first the start's result). */
static Ended l8_start_ab_exit_shim(L8Arm r)
{
    Ended e = l8_slot_ended(r.found);

    if (r.arm == ST22_START_NONE_FREE) {
        return e;
    }
    return ended_with_carry(then(e, ended_load(r.first)), true);
}

void abi_StartPieceABInFreeSlot(void)
{
    abi_ended(l8_start_ab_exit_shim(StartPieceABInFreeSlot(cpu.x)));
}

/*
 * @routine 0F6B:72F LoadRecordFrameByKindThenFall
 * @run 70:B72F
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 4 and takes the frame and OBJ_3510 from tables at $B76C and $B771 by OBJ_3040. Later, while OBJ_3510 counts down, kind 4 sets bit 4 of zField8E and plays sound $FE at zero; at zero it moves in Y and falls.
 * HOW: LoadObjectRecord, table reads, a memory decrement, SoundCommand, ApplyVelocityY, ApplyGravity.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_frame_by_kind_exit_shim).
 */
L8FrameByKindArm LoadRecordFrameByKindThenFall(uint8_t slot, uint8_t y_entry)
{
    uint8_t value;
    L8FrameByKindArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB737);
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 4);
        r.kind = RD8(OBJ->f3040[slot]);
        value = rd8((uint16_t)(0xB76C + r.kind));
        WR8(OBJ->frame[slot], value);
        r.value = rd8((uint16_t)(0xB771 + r.kind));
        WR8(OBJ->f3510[slot], r.value);
        r.arm = ST22_FALL_LOADED;
        return r;
    }
    r.timer = RD8(OBJ->f3510[slot]);
    if (r.timer == 0) {
        jsr_push(0xB768);
        r.move = ApplyVelocityY(slot);
        jsr_pop();
        jsr_push(0xB76B);
        r.gravity = ApplyGravity(slot);
        jsr_pop();
        r.arm = ST22_FALL_FELL;
        return r;
    }
    r.now = DEC8(OBJ->f3510[slot]);
    if (r.now != 0) {
        r.arm = ST22_FALL_COUNTING;
        return r;
    }
    r.kind = RD8(OBJ->f3040[slot]);
    if (r.kind != 4) {
        r.arm = ST22_FALL_OTHER_KIND;
        return r;
    }
    RMW8(ZP->field8e, old, old | 0x10);
    jsr_push(0xB764);
    r.call = SoundCommand(0xFE, slot, y_entry);
    jsr_pop();
    r.arm = ST22_FALL_SOUNDED;
    return r;
}

/* What LoadRecordFrameByKindThenFall leaves, the sound call's included, from the arm it ended on. */
static SoundEnd l8_frame_by_kind_exit_shim(L8FrameByKindArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.state);

    if (r.arm == ST22_FALL_LOADED) {
        e = then(e, l8_record_ended(r.record));
        e = then(e, ended_load(4));
        e = then(e, ended_with_y(ended_nz(r.kind), r.kind));
        s.end = then(e, ended_load(r.value));
        return s;
    }
    e = then(e, ended_load(r.timer));
    if (r.arm == ST22_FALL_FELL) {
        e = then(e, ended_of_axis_move(r.move));
        s.end = then(e, l8_of_gravity(r.gravity));
        return s;
    }
    e = then(e, ended_nz(r.now));
    if (r.arm == ST22_FALL_COUNTING) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.kind, 4));
    if (r.arm == ST22_FALL_OTHER_KIND) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xFE;
    s.before = then(e, ended_load(0xFE));
    s.call = r.call;
    return s;
}

void abi_LoadRecordFrameByKindThenFall(void)
{
    abi_sound_end(l8_frame_by_kind_exit_shim(LoadRecordFrameByKindThenFall(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:45D StepListThenStartFivePieces
 * @run 70:B45D
 * WHAT IT DOES: Runs StepListByNumberTable; when the list ends starts objects of type $A9 in the slots $11 to $15 (each cleared first) at the object's place, numbered 0 to 4 in OBJ_3040, sets frame 2 and OBJ_3510 $C0 and goes to the next state.
 * HOW: StepListByNumberTable, a loop of ClearObjectInY and StartObjectAtPlace, a counter in zTemp00.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_five_pieces_exit_shim).
 */
L8FivePiecesArm StepListThenStartFivePieces(uint8_t slot, uint8_t y_entry)
{
    uint8_t y, number;
    L8FivePiecesArm r = { 0 };

    jsr_push(0xB460);
    r.step = StepListByNumberTable(slot, y_entry);
    jsr_pop();
    if (r.step.step != FRAME_LIST_ENDED) {
        r.started = false;
        return r;
    }
    WR8(ZP->temp00, 0);
    y = 0x11;
    do {
        jsr_push(0xB469);
        ClearObjectInY(y);
        jsr_pop();
        jsr_push(0xB46E);
        r.placed[y - 0x11] = StartObjectAtPlace(0xA9, slot, y);
        jsr_pop();
        number = RD8(ZP->temp00);
        WR8(OBJ->f3040[y], number);
        INC8(ZP->temp00);
        y++;
    } while (y != 0x16);
    WR8(OBJ->frame[slot], 2);
    WR8(OBJ->f3510[slot], 0xC0);
    r.count = INC8(OBJ->state[slot]);
    r.started = true;
    return r;
}

/* What StepListThenStartFivePieces leaves in A, Y and the flags, from the arm it ended on (each pass of the loop replayed). */
static Ended l8_five_pieces_exit_shim(L8FivePiecesArm r)
{
    Ended e = l8_number_step_exit_shim(r.step);
    uint8_t y;

    if (!r.started) {
        return e;
    }
    for (y = 0x12; y <= 0x16; y++) {
        e = then(e, ended_with_y(ended_with_a(l8_compare_y(y, 0x16), r.placed[y - 0x12]), y));
    }
    e = then(e, ended_load(0xC0));
    return then(e, ended_nz(r.count));
}

void abi_StepListThenStartFivePieces(void)
{
    abi_ended(l8_five_pieces_exit_shim(StepListThenStartFivePieces(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:4C9 StepListThenRunPhaseTable
 * @run 70:B4C9
 * WHAT IT DOES: Runs StepListByNumberTable and counts OBJ_3568 down; at zero tries StartPieceABInFreeSlot and, when it started one, keeps OBJ_SUBSTATE in OBJ_38D8, clears it, sets OBJ_3510 $40 and takes OBJ_3568 from OBJ_35C0. Then runs the code the table at $B501 names for OBJ_SUBSTATE (50B, 51C, 52F, 59E, 5B1).
 * HOW: Two calls, a table of two bytes to zTemp00-01, a jump through them.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE (l8_phase_table_exit_shim).
 */
L8PhaseTableArm StepListThenRunPhaseTable(uint8_t slot, uint8_t y_entry)
{
    uint8_t now, value, low;
    L8PhaseTableArm r = { 0 };

    jsr_push(0xB4CC);
    r.step = StepListByNumberTable(slot, y_entry);
    jsr_pop();
    r.timer = RD8(OBJ->f3568[slot]);
    now = r.timer;
    if (r.timer != 0) {
        r.now = DEC8(OBJ->f3568[slot]);
        now = r.now;
    }
    if (now == 0) {
        r.tried = true;
        jsr_push(0xB4D9);
        r.started = StartPieceABInFreeSlot(slot);
        jsr_pop();
        if (r.started.found.found) {
            value = RD8(OBJ->substate[slot]);
            WR8(OBJ->f38d8[slot], value);
            WR8(OBJ->substate[slot], 0);
            WR8(OBJ->f3510[slot], 0x40);
            r.value = RD8(OBJ->f35c0[slot]);
            WR8(OBJ->f3568[slot], r.value);
        }
    }
    r.phase = RD8(OBJ->substate[slot]);
    r.index = (uint8_t)(r.phase << 1);
    low = rd8((uint16_t)(0xB501 + r.index));
    WR8(ZP->temp00, low);
    r.high = rd8((uint16_t)(0xB502 + r.index));
    WR8(ZP->temp01, r.high);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

/* What StepListThenRunPhaseTable leaves and where it jumps, from the arm it ended on. */
static Jump l8_phase_table_exit_shim(L8PhaseTableArm r)
{
    Jump jump = { { 0 }, 0, true };
    Ended e = l8_number_step_exit_shim(r.step);

    e = then(e, ended_load(r.timer));
    if (r.timer != 0) {
        e = then(e, ended_nz(r.now));
    }
    if (r.tried) {
        e = then(e, l8_start_ab_exit_shim(r.started));
        if (r.started.found.found) {
            e = then(e, ended_load(0x40));
            e = then(e, ended_load(r.value));
        }
    }
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.index), r.phase >= 0x80), r.index));
    jump.end = then(e, ended_load(r.high));
    jump.target = r.target;
    return jump;
}

void abi_StepListThenRunPhaseTable(void)
{
    Jump jump = l8_phase_table_exit_shim(StepListThenRunPhaseTable(cpu.x, cpu.y));

    jump_shim(jump.end, jump.target);
}
