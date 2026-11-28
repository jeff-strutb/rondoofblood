/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 05CC (enemy kinds not yet identified), by routine. Bodies return
 * the arm they ended on and the values behind it (L8Arm or a struct of this file's section of stage_enemies_l8.h); each routine's exit
 * shim builds how its last instructions left A, Y and the flags (Ended, enemy_objects.h), and the abi_ wrappers show it.
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
enum { BOUNCE_FALLING, BOUNCE_BOUNCED };
enum { LAND_CLEARED, LAND_FALLING, LAND_LANDED };
enum { CLEAR_WHEN_HELD, CLEAR_WHEN_ENDED };
enum { TURN_FALLING, TURN_TURNED };
enum { SPAWN4_NOT_YET, SPAWN4_SPAWNED };
enum { ABOVE_CLEARED, ABOVE_FELL };
enum { FLING_OLD, FLING_NEW };
enum { SCREEN2_NEW, SCREEN2_WAIT, SCREEN2_CLEARED };
enum { BAND_HIGH, BAND_RIGHT, BAND_TESTED };
enum { HOME_NEW, HOME_PAUSED, HOME_WAIT_STATE, HOME_WAIT_STEP, HOME_NO_HIT, HOME_BIG_HIT, HOME_AIMED, HOME_STOPPED, HOME_MOVED };

/*
 * @routine 05CC:103 LoadRecordLaunchByFacing
 * @run 70:B903
 * WHAT IT DOES: Loads the object's record and launches it: the X speed from the tables at $B92E and $B930 by its facing, the Y speed $08 then $06, moved once in between.
 * HOW: LoadObjectRecord, table reads, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: A $06, Y the facing doubled, C and V from the move (l8_launch_by_facing_exit_shim).
 */
L8Arm LoadRecordLaunchByFacing(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t y, value;

    jsr_push(0xB906);
    r.record = l8_record(slot);
    jsr_pop();
    r.first = RD8(OBJ->facing_left[slot]);
    y = r.first;
    r.second = rd8((uint16_t)(0xB92E + y));
    WR8(OBJ->vx_hi[slot], r.second);
    WR8(OBJ->vy_hi[slot], 8);
    jsr_push(0xB917);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.third = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(r.third << 1);
    value = rd8((uint16_t)(0xB931 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB930 + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 6);
    return r;
}

/* What LoadRecordLaunchByFacing leaves in A, Y and the flags (one path). */
static Ended l8_launch_by_facing_exit_shim(L8Arm r)
{
    Ended e = l8_record_ended(r.record);
    uint8_t y = (uint8_t)(r.third << 1);

    e = then(e, ended_with_y(ended_nz(r.first), r.first));
    e = then(e, ended_load(r.second));
    e = then(e, ended_load(8));
    e = then(e, ended_of_axis_move(r.move));
    e = then(ended_with_y(then(e, ended_with_carry(ended_load(y), r.third >= 0x80)), y), ended_kept());
    return then(e, ended_load(6));
}

void abi_LoadRecordLaunchByFacing(void)
{
    abi_ended(l8_launch_by_facing_exit_shim(LoadRecordLaunchByFacing(cpu.x)));
}

/*
 * @routine 05CC:134 StepListBounceOffFloorAtRandom
 * @run 70:B934
 * WHAT IT DOES: Steps the animation list at $B97E and moves; when its Y low byte reaches $C4 it is put back to $C2 and gets a new random speed (X from the table at $B989 by a number to 15, Y from the table at $B9A9 by a number to 7), the animation is cleared and the state advances.
 * HOW: StepObjectFrames, ApplyVelocity, two NextRandom calls, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_bounce_exit_shim).
 */
L8St16BounceArm StepListBounceOffFloorAtRandom(uint8_t slot, uint8_t y_entry)
{
    L8St16BounceArm r = { 0 };
    uint8_t y, value;

    r.step = l8_list_step(slot, 0xB97E, 0xB93F, false, y_entry);
    jsr_push(0xB942);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.ground = RD8(OBJ->y_lo[slot]);
    if (r.ground < 0xC4) {
        r.arm = BOUNCE_FALLING;
        return r;
    }
    WR8(OBJ->y_lo[slot], 0xC2);
    jsr_push(0xB951);
    r.random = NextRandom();
    jsr_pop();
    y = (uint8_t)((r.random.value & 0x0F) << 1);
    value = rd8((uint16_t)(0xB98A + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB989 + y));
    WR8(OBJ->vx_lo[slot], value);
    jsr_push(0xB964);
    r.random2 = NextRandom();
    jsr_pop();
    y = (uint8_t)((r.random2.value & 0x07) << 1);
    value = rd8((uint16_t)(0xB9AA + y));
    WR8(OBJ->vy_hi[slot], value);
    r.value = rd8((uint16_t)(0xB9A9 + y));
    WR8(OBJ->vy_lo[slot], r.value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.counted = INC8(OBJ->state[slot]);
    r.arm = BOUNCE_BOUNCED;
    return r;
}

/* What StepListBounceOffFloorAtRandom leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_bounce_exit_shim(L8St16BounceArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_axis_move(r.move));
    uint8_t pick;

    e = then(e, ended_compare(r.ground, 0xC4));
    if (r.arm == BOUNCE_FALLING) {
        return e;
    }
    e = then(e, ended_load(0xC2));
    e = then(e, ended_of_random(r.random));
    pick = (uint8_t)((r.random.value & 0x0F) << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(pick), false), pick));
    e = then(e, ended_of_random(r.random2));
    pick = (uint8_t)((r.random2.value & 0x07) << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(pick), false), pick));
    e = then(e, ended_load(r.value));
    return then(e, ended_nz(r.counted));
}

void abi_StepListBounceOffFloorAtRandom(void)
{
    abi_ended(l8_bounce_exit_shim(StepListBounceOffFloorAtRandom(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:1B9 StepListFallThenLandOrClear
 * @run 70:B9B9
 * WHAT IT DOES: Steps the animation list at $B9F9 and moves; once the X high byte is not 0 the object is cleared. Otherwise the Y speed gains $0012 a frame, and at Y low $C4 it stops there, clears OBJ_HITBOX_ON and the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit add in memory, a compare, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_land_or_clear_exit_shim).
 */
L8Arm StepListFallThenLandOrClear(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    uint8_t value;
    Sum sum;

    r.step = l8_list_step(slot, 0xB9F9, 0xB9C4, false, y_entry);
    jsr_push(0xB9C7);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->x_hi[slot]);
    if (r.first != 0) {
        ClearObject(slot);
        r.arm = LAND_CLEARED;
        return r;
    }
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x12, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    r.sum = sum;
    r.second = RD8(OBJ->y_lo[slot]);
    if (r.second < 0xC4) {
        r.arm = LAND_FALLING;
        return r;
    }
    WR8(OBJ->y_lo[slot], 0xC4);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.third = INC8(OBJ->state[slot]);
    r.arm = LAND_LANDED;
    return r;
}

/* What StepListFallThenLandOrClear leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_land_or_clear_exit_shim(L8Arm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_axis_move(r.move));

    e = then(e, ended_load(r.first));
    if (r.arm == LAND_CLEARED) {
        return e;
    }
    e = then(e, ended_of_sum(r.sum));
    e = then(e, ended_compare(r.second, 0xC4));
    if (r.arm == LAND_FALLING) {
        return e;
    }
    e = then(e, ended_load(0xC4));
    return then(e, ended_nz(r.third));
}

void abi_StepListFallThenLandOrClear(void)
{
    abi_ended(l8_land_or_clear_exit_shim(StepListFallThenLandOrClear(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:204 StepListThenClearWhenEnded
 * @run 70:BA04
 * WHAT IT DOES: Steps the animation list at $BA15; when it ends the object is cleared.
 * HOW: StepObjectFrames, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (all kept by ClearObject) (l8_clear_when_ended_exit_shim).
 */
L8Arm StepListThenClearWhenEnded(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };

    r.step = l8_list_step(slot, 0xBA15, 0xBA0F, false, y_entry);
    r.arm = CLEAR_WHEN_HELD;
    if (r.step.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        r.arm = CLEAR_WHEN_ENDED;
    }
    return r;
}

/* What StepListThenClearWhenEnded leaves in A, Y and the flags: the step's on both arms (ClearObject keeps them). */
static Ended l8_clear_when_ended_exit_shim(L8Arm r)
{
    return l8_step_ended(r.step);
}

void abi_StepListThenClearWhenEnded(void)
{
    abi_ended(l8_clear_when_ended_exit_shim(StepListThenClearWhenEnded(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:376 CountFramesThenRunByState
 * @run 70:BB76
 * WHAT IT DOES: Counts OBJ_WORK_D up and runs the code a table at $BB8B names for the object's state (391, 3BA or 3FA).
 * HOW: An INC, a table of two bytes to zTemp00-01, a jump through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state (l8_run_by_state_exit_shim).
 */
L8St16StateJump CountFramesThenRunByState(uint8_t slot, L8Layout lay)
{
    L8St16StateJump r = { 0 };
    uint8_t index, low;

    r.counted = INC8(OBJ->f34b8[slot]);
    r.state = RD8(OBJ->state[slot]);
    index = (uint8_t)(r.state << 1);
    low = rd8((uint16_t)(l8_at(lay, 0xBB8B) + index));
    WR8(ZP->temp00, low);
    r.high = rd8((uint16_t)(l8_at(lay, 0xBB8C) + index));
    WR8(ZP->temp01, r.high);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

/* What CountFramesThenRunByState leaves in A, Y and the flags, and the code it jumps to (one path). */
static Jump l8_run_by_state_exit_shim(L8St16StateJump r)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t index = (uint8_t)(r.state << 1);
    Ended e = ended_nz(r.counted);

    e = then(e, ended_with_y(ended_with_carry(ended_load(index), r.state >= 0x80), index));
    jump.end = then(e, ended_load(r.high));
    jump.target = r.target;
    return jump;
}

void abi_CountFramesThenRunByState(void)
{
    Jump jump = l8_run_by_state_exit_shim(CountFramesThenRunByState(cpu.x, L8_LOADED));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CC:391 LoadRecordLaunchSideways
 * @run 70:BB91
 * WHAT IT DOES: Loads the object's record, gives it the X speed from the table at $BBB6 by its facing and the Y speed $08, moves it once, then the X speed from the table at $BBB8, the X low speed 0 and the Y speed $04.
 * HOW: LoadObjectRecord, table reads by the facing, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: A $04, Y the facing, C and V from the move (l8_launch_sideways_exit_shim).
 */
L8Arm LoadRecordLaunchSideways(uint8_t slot, L8Layout lay)
{
    L8Arm r = { 0 };
    uint8_t value;

    jsr_push(l8_at(lay, 0xBB94));
    r.record = l8_record(slot);
    jsr_pop();
    r.y = RD8(OBJ->facing_left[slot]);
    r.first = rd8((uint16_t)(l8_at(lay, 0xBBB6) + r.y));
    WR8(OBJ->vx_hi[slot], r.first);
    WR8(OBJ->vy_hi[slot], 8);
    jsr_push(l8_at(lay, 0xBBA5));
    r.move = ApplyVelocity(slot);
    jsr_pop();
    value = rd8((uint16_t)(l8_at(lay, 0xBBB8) + r.y));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 4);
    return r;
}

/* What LoadRecordLaunchSideways leaves in A, Y and the flags (one path). */
static Ended l8_launch_sideways_exit_shim(L8Arm r)
{
    Ended e = l8_record_ended(r.record);

    e = then(e, ended_with_y(ended_nz(r.y), r.y));
    e = then(e, ended_load(r.first));
    e = then(e, ended_load(8));
    e = then(e, ended_of_axis_move(r.move));
    return then(e, ended_load(4));
}

void abi_LoadRecordLaunchSideways(void)
{
    abi_ended(l8_launch_sideways_exit_shim(LoadRecordLaunchSideways(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CC:3BA StepListFallThenTurnToPlayer
 * @run 70:BBBA
 * WHAT IT DOES: Steps the animation list at $BBF1 and moves; at Y low $C8 it takes the facing of the player's side, the X speed from the table at $BBF7 by it, stops at Y low $C7, clears the frame and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, SideOfPlayer, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_turn_to_player_exit_shim).
 */
L8St16TurnArm StepListFallThenTurnToPlayer(uint8_t slot, uint8_t y_entry)
{
    L8St16TurnArm r = { 0 };
    uint8_t facing, y, value;

    r.step = l8_list_step(slot, 0xBBF1, 0xBBC5, false, y_entry);
    jsr_push(0xBBC8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.ground = RD8(OBJ->y_lo[slot]);
    if (r.ground < 0xC8) {
        r.arm = TURN_FALLING;
        return r;
    }
    jsr_push(0xBBD2);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], facing);
    y = (uint8_t)(facing << 1);
    value = rd8((uint16_t)(0xBBF7 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBBF7 + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->y_lo[slot], 0xC7);
    WR8(OBJ->frame[slot], 0);
    r.counted = INC8(OBJ->state[slot]);
    r.arm = TURN_TURNED;
    return r;
}

/* What StepListFallThenTurnToPlayer leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_turn_to_player_exit_shim(L8St16TurnArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_of_axis_move(r.move));
    uint8_t facing, y;

    e = then(e, ended_compare(r.ground, 0xC8));
    if (r.arm == TURN_FALLING) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    facing = r.side.sides & 0x01;
    e = then(e, ended_load(facing));
    y = (uint8_t)(facing << 1);
    e = then(ended_with_y(then(e, ended_with_carry(ended_load(y), false)), y), ended_kept());
    e = then(e, ended_load(0xC7));
    return then(e, ended_nz(r.counted));
}

void abi_StepListFallThenTurnToPlayer(void)
{
    abi_ended(l8_turn_to_player_exit_shim(StepListFallThenTurnToPlayer(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:419 SoundAndSpawnEveryFourthFrame
 * @run 70:BC19
 * WHAT IT DOES: When the low two bits of OBJ_WORK_D are 0 plays sound $19 and starts an object of type $29 in a free slot of $30 to $3F at the object's place.
 * HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_spawn_fourth_exit_shim).
 */
L8St16SpawnArm SoundAndSpawnEveryFourthFrame(uint8_t slot, uint8_t y_entry)
{
    L8St16SpawnArm r = { 0 };

    r.bits = RD8(OBJ->f34b8[slot]);
    if ((r.bits & 0x03) != 0) {
        r.arm = SPAWN4_NOT_YET;
        return r;
    }
    jsr_push(0xBC25);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBC28);
    r.found = l8_free_slot30();
    jsr_pop();
    jsr_push(0xBC2F);
    r.placed = StartObjectAtPlace(0x29, slot, r.found.slot);
    jsr_pop();
    r.arm = SPAWN4_SPAWNED;
    return r;
}

/* What SoundAndSpawnEveryFourthFrame leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_spawn_fourth_exit_shim(L8St16SpawnArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    uint8_t bits = (uint8_t)(r.bits & 0x03);

    if (r.arm == SPAWN4_NOT_YET) {
        s.end = ended_load(bits);
        return s;
    }
    s.sounded = true;
    s.command = 0x19;
    s.before = then(ended_load(bits), ended_load(0x19));
    s.call = r.call;
    s.end = then(l8_slot_ended(r.found), ended_load(r.placed));
    return s;
}

void abi_SoundAndSpawnEveryFourthFrame(void)
{
    abi_sound_end(l8_spawn_fourth_exit_shim(SoundAndSpawnEveryFourthFrame(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:3FA StepListSpawnAndMoveXOrClear
 * @run 70:BBFA
 * WHAT IT DOES: Steps the animation list at $BC14, runs SoundAndSpawnEveryFourthFrame and moves along X; once the X high byte is not 0 the object is cleared.
 * HOW: StepObjectFrames, a call, ApplyVelocityX, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_move_x_or_clear_exit_shim).
 */
L8St16MoveXArm StepListSpawnAndMoveXOrClear(uint8_t slot, uint8_t y_entry)
{
    L8St16MoveXArm r = { 0 };

    r.step = l8_list_step(slot, 0xBC14, 0xBC05, false, y_entry);
    jsr_push(0xBC08);
    r.spawn = SoundAndSpawnEveryFourthFrame(slot, r.step.y);
    jsr_pop();
    jsr_push(0xBC0B);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.high = RD8(OBJ->x_hi[slot]);
    if (r.high != 0) {
        ClearObject(slot);
    }
    return r;
}

/* What StepListSpawnAndMoveXOrClear leaves in A, Y and the flags, with the sound call of SoundAndSpawnEveryFourthFrame (one path). */
static SoundEnd l8_move_x_or_clear_exit_shim(L8St16MoveXArm r)
{
    SoundEnd s = sound_end_after(l8_step_ended(r.step), l8_spawn_fourth_exit_shim(r.spawn));
    Ended after = then(ended_of_axis_move(r.move), ended_load(r.high));

    return sound_end_then(s, after);
}

void abi_StepListSpawnAndMoveXOrClear(void)
{
    abi_sound_end(l8_move_x_or_clear_exit_shim(StepListSpawnAndMoveXOrClear(cpu.x, cpu.y)));
}

/*
 * @routine 05CC:489 ClearIfAboveOrFall
 * @run 70:BC89
 * WHAT IT DOES: When the object's Y high byte is not 0 the object is cleared; otherwise it falls and moves.
 * HOW: A test, a tail jump into ClearObject, or ApplyGravityUnlimited and a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the load, or as ApplyVelocity (l8_above_or_fall_exit_shim).
 */
L8Arm ClearIfAboveOrFall(uint8_t slot, L8Layout lay)
{
    L8Arm r = { 0 };
    GravityResult gravity;

    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first != 0) {
        ClearObject(slot);
        r.arm = ABOVE_CLEARED;
        return r;
    }
    jsr_push(l8_at(lay, 0xBC94));
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    r.move = ApplyVelocity(slot);
    r.arm = ABOVE_FELL;
    return r;
}

/* What ClearIfAboveOrFall leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_above_or_fall_exit_shim(L8Arm r)
{
    if (r.arm == ABOVE_CLEARED) {
        return ended_load(r.first);
    }
    return ended_of_axis_move(r.move);
}

void abi_ClearIfAboveOrFall(void)
{
    abi_ended(l8_above_or_fall_exit_shim(ClearIfAboveOrFall(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CC:430 LoadRecordRandomFlingOrFall
 * @run 70:BC30
 * @stop BC69
 * WHAT IT DOES: In state 0 loads the record, picks frame $23 or $24 at random and the speed (X and Y) from the table at $BC69 by another random number, and writes $BE to OBJ_Y_LO of the slot the record's Y names. Later it runs ClearIfAboveOrFall.
 * HOW: LoadObjectRecord, two NextRandom calls, table reads, stores, a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_fling_or_fall_exit_shim). OBJ_Y_LO of slot 11 is written because LoadObjectRecord leaves Y 11.
 */
L8St16FlingArm LoadRecordRandomFlingOrFall(uint8_t slot, L8Layout lay)
{
    L8St16FlingArm r = { 0 };
    uint8_t pick, y, value;

    r.state = RD8(OBJ->state[slot]);
    if (r.state != 0) {
        r.fall = ClearIfAboveOrFall(slot, lay);
        r.arm = FLING_OLD;
        return r;
    }
    jsr_push(l8_at(lay, 0xBC38));
    r.record = l8_record(slot);
    jsr_pop();
    y = 11;
    jsr_push(l8_at(lay, 0xBC3B));
    r.random = NextRandom();
    jsr_pop();
    pick = r.random.value & 0x01;
    r.sum = add_bytes(pick, 0x23, false);
    WR8(OBJ->frame[slot], r.sum.value);
    WR8(OBJ->y_lo[y], 0xBE);
    jsr_push(l8_at(lay, 0xBC4B));
    r.random2 = NextRandom();
    jsr_pop();
    y = (uint8_t)((r.random2.value & 0x07) << 2);
    value = rd8((uint16_t)(l8_at(lay, 0xBC6A) + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(l8_at(lay, 0xBC69) + y));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(l8_at(lay, 0xBC6C) + y));
    WR8(OBJ->vy_hi[slot], value);
    r.value = rd8((uint16_t)(l8_at(lay, 0xBC6B) + y));
    WR8(OBJ->vy_lo[slot], r.value);
    r.arm = FLING_NEW;
    return r;
}

/* What LoadRecordRandomFlingOrFall leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_fling_or_fall_exit_shim(L8St16FlingArm r)
{
    Ended e;
    uint8_t pick;

    if (r.arm == FLING_OLD) {
        return then(ended_load(r.state), l8_above_or_fall_exit_shim(r.fall));
    }
    e = l8_record_ended(r.record);
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load((uint8_t)(r.random.value & 0x01)));
    e = then(e, ended_of_sum(r.sum));
    e = then(e, ended_load(0xBE));
    e = then(e, ended_of_random(r.random2));
    pick = (uint8_t)((r.random2.value & 0x07) << 2);
    e = then(ended_with_y(then(e, ended_load(pick)), pick), ended_with_carry(ended_kept(), false));
    return then(e, ended_load(r.value));
}

void abi_LoadRecordRandomFlingOrFall(void)
{
    abi_ended(l8_fling_or_fall_exit_shim(LoadRecordRandomFlingOrFall(cpu.x, L8_LOADED)));
}

/*
 * @routine 05CC:498 LoadRecordThenClearEnemiesAtScreen2
 * @run 70:BC98
 * WHAT IT DOES: In state 0 loads the record. Later, when the scroll's screen byte is 2, it sets the scroll's start high byte to 2, clears every object of kind $A0 whose flags' low nibble is 2 in slots $10 to $53, sets bit 4 of zField8E, plays sound $FE, draws tile script 0 and clears itself.
 * HOW: LoadObjectRecord, a loop over the slots with ClearObject, SoundCommand, DrawTileScript, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_clear_at_screen2_exit_shim).
 */
L8St16ScreenArm LoadRecordThenClearEnemiesAtScreen2(uint8_t slot, uint8_t y_entry)
{
    L8St16ScreenArm r = { 0 };
    uint8_t x;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = SCREEN2_NEW;
        return r;
    }
    r.screen = RD8(SCROLLE->x_hi_screen[0]);
    if (r.screen != 2) {
        r.arm = SCREEN2_WAIT;
        return r;
    }
    wr8(ScrollXMinHi, 2);
    push8(slot);
    x = 0x10;
    do {
        r.kind = RD8(OBJ->kind[x]);
        if (r.kind == 0xA0) {
            r.flags = RD8(OBJ->scroll_band[x]);
            if ((r.flags & 0x0F) == 2) {
                jsr_push(0xBCC3);
                ClearObject(x);
                jsr_pop();
            }
        }
        x++;
    } while (x < 0x54);
    r.x_end = x;
    slot = pull8();
    r.x = slot;
    RMW8(ZP->field8e, old, old | 0x10);
    jsr_push(0xBCD0);
    r.call = SoundCommand(0xFE, slot, y_entry);
    jsr_pop();
    jsr_push(0xBCD5);
    r.run = DrawTileScript(0, 0);
    jsr_pop();
    ClearObject(slot);
    r.arm = SCREEN2_CLEARED;
    return r;
}

/* What LoadRecordThenClearEnemiesAtScreen2 leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_clear_at_screen2_exit_shim(L8St16ScreenArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e;

    if (r.arm == SCREEN2_NEW) {
        s.end = then(ended_load(r.state), l8_record_ended(r.record));
        return s;
    }
    e = ended_compare(r.screen, 2);
    if (r.arm == SCREEN2_WAIT) {
        s.end = e;
        return s;
    }
    e = ended_compare(r.kind, 0xA0);
    if (r.kind == 0xA0) {
        e = ended_compare((uint8_t)(r.flags & 0x0F), 2);
    }
    e = then(e, l8_compare_y(r.x_end, 0x54));
    e = then(e, ended_nz(r.x));
    s.sounded = true;
    s.command = 0xFE;
    s.before = then(e, ended_load(0xFE));
    s.call = r.call;
    s.end = ended_of_tile_run(r.run, 0);
    return s;
}

void abi_LoadRecordThenClearEnemiesAtScreen2(void)
{
    abi_sound_end(l8_clear_at_screen2_exit_shim(LoadRecordThenClearEnemiesAtScreen2(cpu.x, cpu.y)));
}

/* The code at $BB62, which no script ran and which has no row of its own: outside (C 1) when the object's X is not inside the band from $0010 to $00EF on the first screen. */
static L8St16Band inside_edge_band(uint8_t slot)
{
    L8St16Band r = { 0 };

    r.high = RD8(OBJ->x_hi[slot]);
    if (r.high != 0) {
        r.outside = true;
        r.arm = BAND_HIGH;
        return r;
    }
    r.low = RD8(OBJ->x_lo[slot]);
    if (r.low >= 0xF0) {
        r.outside = true;
        r.arm = BAND_RIGHT;
        return r;
    }
    r.outside = r.low < 0x10;
    r.arm = BAND_TESTED;
    return r;
}

/* What the code at $BB62 leaves in A and the flags, from the arm it ended on. */
static Ended l8_band_exit_shim(L8St16Band r)
{
    switch (r.arm) {
    case BAND_HIGH:
        return ended_with_carry(ended_load(r.high), true);
    case BAND_RIGHT:
        return ended_with_carry(ended_compare(r.low, 0xF0), true);
    default:
        return ended_with_carry(ended_compare(r.low, 0x10), r.low < 0x10);
    }
}

/*
 * @routine 05CC:282 LoadRecordThenFollowOrHomeIn
 * @run 70:BA82
 * WHAT IT DOES: In state 0 loads the record and sets the Y speed $1A. Later (unless the game's sub-mode is 4) it takes the place of slot $10 and moves by an X speed from the table at $BA90 by its number and slot $10's facing. While OBJ_WORK_D is 0 and slot $10 is in state 4 with OBJ_SUBSTATE of 2 or 3 it waits for a hit of 1 to 4, then takes the distance to the player as its aim (OBJ_WORK_E, OBJ_WORK_F) and sets OBJ_WORK_D 1. While OBJ_WORK_D is not 0 it homes in: it puts slot 0 at its place and moves it by the aim (turned on a step by the sign of each part), restores the speed of slot $10 and ends.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, ApplyVelocity, CopyObjectPlace, ApplyVelocity with X and Y swapped, compares, table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_follow_or_home_exit_shim). The arm that takes the aim and the whole of the homing arm ran in no script (the draft stops there); the code at $BB62 that the homing arm calls has no row and is written here as inside_edge_band.
 */
L8St16HomeArm LoadRecordThenFollowOrHomeIn(uint8_t slot, L8Layout lay)
{
    L8St16HomeArm r = { 0 };
    uint8_t state, index, y, aim, value, flags;
    Sum diff;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(l8_at(lay, 0xBA8A));
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->vy_hi[slot], 0x1A);
        r.arm = HOME_NEW;
        return r;
    }
    r.mode = RD8(ZP->game_sub_mode);
    if (r.mode == 4) {
        r.arm = HOME_PAUSED;
        return r;
    }
    jsr_push(l8_at(lay, 0xBA9F));
    r.placed = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    r.diff = sub_bytes(slot, 0x11, true);
    index = (uint8_t)(r.diff.value << 1);
    r.facing = RD8(OBJ->facing_left[0x10]);
    r.sum = add_bytes(index, r.facing, false);
    y = r.sum.value;
    r.value = rd8((uint16_t)(l8_at(lay, 0xBA90) + y));
    WR8(OBJ->vx_hi[slot], r.value);
    jsr_push(l8_at(lay, 0xBAB2));
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.held = RD8(OBJ->f34b8[slot]);
    if (r.held == 0) {
        r.other = RD8(OBJ->state[0x10]);
        if (r.other != 4) {
            r.arm = HOME_WAIT_STATE;
            return r;
        }
        r.phase = RD8(OBJ->substate[0x10]);
        if (r.phase != 2) {
            if (r.phase != 3) {
                r.arm = HOME_WAIT_STEP;
                return r;
            }
        }
        WR8(OBJ->hitbox_on[slot], 2);
        r.hits = RD8(OBJ->hit_taken[slot]);
        if (r.hits == 0) {
            r.arm = HOME_NO_HIT;
            return r;
        }
        if (r.hits >= 5) {
            r.arm = HOME_BIG_HIT;
            return r;
        }
        aim = RD8(OBJ->x_lo[0]);
        diff = sub_bytes(aim, RD8(OBJ->x_lo[slot]), true);
        WR8(OBJ->work_e[slot], diff.value);
        aim = RD8(OBJ->y_lo[0]);
        r.aim = sub_bytes(aim, RD8(OBJ->y_lo[slot]), true);
        WR8(OBJ->f3568[slot], r.aim.value);
        WR8(OBJ->f34b8[slot], 1);
        r.arm = HOME_AIMED;
        return r;
    }
    jsr_push(l8_at(lay, 0xBAF5));
    r.band = inside_edge_band(slot);
    jsr_pop();
    if (!r.band.outside) {
        r.other = RD8(OBJ->state[0x10]);
        if (r.other != 4) {
            WR8(OBJ->hitbox_on[slot], 0);
            WR8(OBJ->f34b8[slot], 0);
            r.arm = HOME_STOPPED;
            return r;
        }
    }
    jsr_push(l8_at(lay, 0xBB03));
    r.placed2 = CopyObjectPlace(slot, 0);
    jsr_pop();
    value = RD8(OBJ->work_e[slot]);
    WR8(OBJ->vx_hi[0], value);
    r.vy = RD8(OBJ->f3568[slot]);
    WR8(OBJ->vy_hi[0], r.vy);
    jsr_push(l8_at(lay, 0xBB13));
    r.move2 = ApplyVelocity(0);
    jsr_pop();
    r.aim_x = RD8(OBJ->work_e[slot]);
    if (r.aim_x != 0) {
        index = (uint8_t)((r.aim_x >> 7) & 0x01);
        value = RD8(OBJ->work_e[slot]);
        r.sum_x = add_bytes(value, rd8((uint16_t)(l8_at(lay, 0xBB60) + index)), false);
        WR8(OBJ->work_e[slot], r.sum_x.value);
    }
    r.aim_y = RD8(OBJ->f3568[slot]);
    if (r.aim_y != 0) {
        index = (uint8_t)((r.aim_y >> 7) & 0x01);
        value = RD8(OBJ->f3568[slot]);
        r.sum_y = add_bytes(value, rd8((uint16_t)(l8_at(lay, 0xBB60) + index)), false);
        WR8(OBJ->f3568[slot], r.sum_y.value);
    }
    flags = RD8(OBJ->vx_hi[0x10]);
    WR8(OBJ->vx_hi[0], flags);
    flags = RD8(OBJ->vx_lo[0x10]);
    WR8(OBJ->vx_lo[0], flags);
    WR8(OBJ->vy_hi[0], 0xFF);
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->work_e[0], 2);
    WR8(OBJ->f3568[0], 0);
    r.arm = HOME_MOVED;
    return r;
}

/* What LoadRecordThenFollowOrHomeIn leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_follow_or_home_exit_shim(L8St16HomeArm r)
{
    Ended e;
    uint8_t index;

    if (r.arm == HOME_NEW) {
        return then(l8_record_ended(r.record), ended_load(0x1A));
    }
    e = ended_compare(r.mode, 4);
    if (r.arm == HOME_PAUSED) {
        return e;
    }
    e = then(then(e, ended_with_y(ended_nz(0x10), 0x10)), ended_load(r.placed));
    index = (uint8_t)(r.diff.value << 1);
    e = then(e, ended_with_carry(ended_load(index), (r.diff.value & 0x80) != 0));
    e = ended_with_y(then(e, ended_of_sum(r.sum)), r.sum.value);
    e = then(e, ended_load(r.value));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.held));
    if (r.held == 0) {
        e = then(e, ended_compare(r.other, 4));
        if (r.arm == HOME_WAIT_STATE) {
            return e;
        }
        e = then(e, ended_compare(r.phase, 2));
        if (r.phase != 2) {
            e = then(e, ended_compare(r.phase, 3));
            if (r.arm == HOME_WAIT_STEP) {
                return e;
            }
        }
        e = then(e, ended_load(2));
        e = then(e, ended_load(r.hits));
        if (r.arm == HOME_NO_HIT) {
            return e;
        }
        e = then(e, ended_compare(r.hits, 5));
        if (r.arm == HOME_BIG_HIT) {
            return e;
        }
        e = then(e, ended_of_sum(r.aim));
        return then(e, ended_load(1));
    }
    e = then(e, l8_band_exit_shim(r.band));
    if (!r.band.outside) {
        e = then(e, ended_compare(r.other, 4));
        if (r.arm == HOME_STOPPED) {
            return e;
        }
    }
    e = then(then(e, ended_with_y(ended_nz(0), 0)), ended_load(r.placed2));
    e = then(e, ended_load(r.vy));
    e = then(e, ended_of_axis_move(r.move2));
    e = then(e, ended_load(r.aim_x));
    if (r.aim_x != 0) {
        index = (uint8_t)((r.aim_x >> 7) & 0x01);
        e = then(e, ended_with_y(ended_load(index), index));
        e = then(e, ended_of_sum(r.sum_x));
    }
    e = then(e, ended_load(r.aim_y));
    if (r.aim_y != 0) {
        index = (uint8_t)((r.aim_y >> 7) & 0x01);
        e = then(e, ended_with_y(ended_load(index), index));
        e = then(e, ended_of_sum(r.sum_y));
    }
    return then(e, ended_load(2));
}

void abi_LoadRecordThenFollowOrHomeIn(void)
{
    abi_ended(l8_follow_or_home_exit_shim(LoadRecordThenFollowOrHomeIn(cpu.x, L8_LOADED)));
}
