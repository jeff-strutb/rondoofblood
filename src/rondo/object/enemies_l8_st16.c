/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 05CC (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/*
 * @routine 05CC:103 LoadRecordLaunchByFacing
 * @run 70:B903
 * WHAT IT DOES: Loads the object's record and launches it: the X speed from the tables at $B92E and $B930 by its facing, the Y speed $08 then $06, moved once in between.
 * HOW: LoadObjectRecord, table reads, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: A $06, Y the facing doubled, C and V from the move.
 */
Ended LoadRecordLaunchByFacing(uint8_t slot)
{
    uint8_t last, nz, facing, y, value;
    bool carry;
    AxisMove move;
    Ended e;

    jsr_push(0xB906);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    facing = RD8(OBJ->facing_left[slot]);
    y = facing;
    e = then(e, ended_with_y(ended_nz(facing), y));
    value = rd8((uint16_t)(0xB92E + y));
    WR8(OBJ->vx_hi[slot], value);
    e = then(e, ended_load(value));
    WR8(OBJ->vy_hi[slot], 8);
    e = then(e, ended_load(8));
    jsr_push(0xB917);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    facing = RD8(OBJ->facing_left[slot]);
    y = (uint8_t)(facing << 1);
    e = then(ended_with_y(then(e, ended_with_carry(ended_load(y), facing >= 0x80)), y), ended_kept());
    value = rd8((uint16_t)(0xB931 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB930 + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->vy_hi[slot], 6);
    return then(e, ended_load(6));
}

void abi_LoadRecordLaunchByFacing(void)
{
    abi_ended(LoadRecordLaunchByFacing(cpu.x));
}

/*
 * @routine 05CC:134 StepListBounceOffFloorAtRandom
 * @run 70:B934
 * WHAT IT DOES: Steps the animation list at $B97E and moves; when its Y low byte reaches $C4 it is put back to $C2 and gets a new random speed (X from the table at $B989 by a number to 15, Y from the table at $B9A9 by a number to 7), the animation is cleared and the state advances.
 * HOW: StepObjectFrames, ApplyVelocity, two NextRandom calls, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListBounceOffFloorAtRandom(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, ground, pick, value;
    FrameStep step;
    AxisMove move;
    RandomResult random;
    Ended e = l8_step_list(slot, 0xB97E, 0xB93F, false, &y, &step);

    jsr_push(0xB942);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    ground = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(ground, 0xC4));
    if (ground < 0xC4) {
        return e;
    }
    WR8(OBJ->y_lo[slot], 0xC2);
    e = then(e, ended_load(0xC2));
    jsr_push(0xB951);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)((random.value & 0x0F) << 1);
    y = pick;
    e = then(e, ended_with_y(ended_with_carry(ended_load(pick), false), y));
    value = rd8((uint16_t)(0xB98A + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xB989 + y));
    WR8(OBJ->vx_lo[slot], value);
    jsr_push(0xB964);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)((random.value & 0x07) << 1);
    y = pick;
    e = then(e, ended_with_y(ended_with_carry(ended_load(pick), false), y));
    value = rd8((uint16_t)(0xB9AA + y));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xB9A9 + y));
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListBounceOffFloorAtRandom(void)
{
    abi_ended(StepListBounceOffFloorAtRandom(cpu.x, cpu.y));
}

/*
 * @routine 05CC:1B9 StepListFallThenLandOrClear
 * @run 70:B9B9
 * WHAT IT DOES: Steps the animation list at $B9F9 and moves; once the X high byte is not 0 the object is cleared. Otherwise the Y speed gains $0012 a frame, and at Y low $C4 it stops there, clears OBJ_3250 and the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit add in memory, a compare, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListFallThenLandOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, high, value, ground;
    FrameStep step;
    AxisMove move;
    Sum sum;
    Ended e = l8_step_list(slot, 0xB9F9, 0xB9C4, false, &y, &step);

    jsr_push(0xB9C7);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    high = RD8(OBJ->x_hi[slot]);
    e = then(e, ended_load(high));
    if (high != 0) {
        ClearObject(slot);
        return e;
    }
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x12, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    ground = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(ground, 0xC4));
    if (ground < 0xC4) {
        return e;
    }
    WR8(OBJ->y_lo[slot], 0xC4);
    e = then(e, ended_load(0xC4));
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListFallThenLandOrClear(void)
{
    abi_ended(StepListFallThenLandOrClear(cpu.x, cpu.y));
}

/*
 * @routine 05CC:204 StepListThenClearWhenEnded
 * @run 70:BA04
 * WHAT IT DOES: Steps the animation list at $BA15; when it ends the object is cleared.
 * HOW: StepObjectFrames, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (all kept by ClearObject).
 */
Ended StepListThenClearWhenEnded(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xBA15, 0xBA0F, false, &y, &step);

    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return e;
}

void abi_StepListThenClearWhenEnded(void)
{
    abi_ended(StepListThenClearWhenEnded(cpu.x, cpu.y));
}

/*
 * @routine 05CC:376 CountFramesThenRunByState
 * @run 70:BB76
 * WHAT IT DOES: Counts OBJ_34B8 up and runs the code a table at $BB8B names for the object's state (391, 3BA or 3FA).
 * HOW: An INC, a table of two bytes to zTemp00-01, a jump through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump CountFramesThenRunByState(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;
    Ended e;

    e = ended_nz(INC8(OBJ->f34b8[slot]));
    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), state >= 0x80), index));
    low = rd8((uint16_t)(0xBB8B + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBB8C + index));
    WR8(ZP->temp01, high);
    jump.end = then(e, ended_load(high));
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_CountFramesThenRunByState(void)
{
    Jump jump = CountFramesThenRunByState(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CC:391 LoadRecordLaunchSideways
 * @run 70:BB91
 * WHAT IT DOES: Loads the object's record, gives it the X speed from the table at $BBB6 by its facing and the Y speed $08, moves it once, then the X speed from the table at $BBB8, the X low speed 0 and the Y speed $04.
 * HOW: LoadObjectRecord, table reads by the facing, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: A $04, Y the facing, C and V from the move.
 */
Ended LoadRecordLaunchSideways(uint8_t slot)
{
    uint8_t last, nz, y, value;
    bool carry;
    AxisMove move;
    Ended e;

    jsr_push(0xBB94);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    y = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    value = rd8((uint16_t)(0xBBB6 + y));
    WR8(OBJ->vx_hi[slot], value);
    e = then(e, ended_load(value));
    WR8(OBJ->vy_hi[slot], 8);
    e = then(e, ended_load(8));
    jsr_push(0xBBA5);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = rd8((uint16_t)(0xBBB8 + y));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 4);
    return then(e, ended_load(4));
}

void abi_LoadRecordLaunchSideways(void)
{
    abi_ended(LoadRecordLaunchSideways(cpu.x));
}

/*
 * @routine 05CC:3BA StepListFallThenTurnToPlayer
 * @run 70:BBBA
 * WHAT IT DOES: Steps the animation list at $BBF1 and moves; at Y low $C8 it takes the facing of the player's side, the X speed from the table at $BBF7 by it, stops at Y low $C7, clears the frame and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, SideOfPlayer, table reads, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended StepListFallThenTurnToPlayer(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, ground, facing, value;
    FrameStep step;
    AxisMove move;
    SideResult side;
    Ended e = l8_step_list(slot, 0xBBF1, 0xBBC5, false, &y, &step);

    jsr_push(0xBBC8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    ground = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(ground, 0xC8));
    if (ground < 0xC8) {
        return e;
    }
    jsr_push(0xBBD2);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    facing = side.sides & 0x01;
    e = then(e, ended_load(facing));
    WR8(OBJ->facing_left[slot], facing);
    y = (uint8_t)(facing << 1);
    e = then(ended_with_y(then(e, ended_with_carry(ended_load(y), false)), y), ended_kept());
    value = rd8((uint16_t)(0xBBF7 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBBF7 + y));
    WR8(OBJ->vx_lo[slot], value);
    WR8(OBJ->y_lo[slot], 0xC7);
    WR8(OBJ->frame[slot], 0);
    e = then(e, ended_load(0xC7));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_StepListFallThenTurnToPlayer(void)
{
    abi_ended(StepListFallThenTurnToPlayer(cpu.x, cpu.y));
}

/*
 * @routine 05CC:419 SoundAndSpawnEveryFourthFrame
 * @run 70:BC19
 * WHAT IT DOES: When the low two bits of OBJ_34B8 are 0 plays sound $19 and starts an object of type $29 in a free slot of $30 to $3F at the object's place.
 * HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd SoundAndSpawnEveryFourthFrame(uint8_t slot, uint8_t y_entry)
{
    uint8_t bits, found_slot, last, placed;
    bool found;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    bits = (uint8_t)(RD8(OBJ->f34b8[slot]) & 0x03);
    if (bits != 0) {
        r.end = ended_load(bits);
        return r;
    }
    r.sounded = true;
    r.command = 0x19;
    r.before = then(ended_load(bits), ended_load(0x19));
    jsr_push(0xBC25);
    r.call = SoundCommand(0x19, slot, y_entry);
    jsr_pop();
    jsr_push(0xBC28);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_of_find_slot(found_slot, last, found);
    jsr_push(0xBC2F);
    placed = StartObjectAtPlace(0x29, slot, found_slot);
    jsr_pop();
    r.end = then(e, ended_load(placed));
    return r;
}

void abi_SoundAndSpawnEveryFourthFrame(void)
{
    abi_sound_end(SoundAndSpawnEveryFourthFrame(cpu.x, cpu.y));
}

/*
 * @routine 05CC:3FA StepListSpawnAndMoveXOrClear
 * @run 70:BBFA
 * WHAT IT DOES: Steps the animation list at $BC14, runs SoundAndSpawnEveryFourthFrame and moves along X; once the X high byte is not 0 the object is cleared.
 * HOW: StepObjectFrames, a call, ApplyVelocityX, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListSpawnAndMoveXOrClear(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, high;
    FrameStep step;
    AxisMove move;
    SoundEnd r;
    Ended e = l8_step_list(slot, 0xBC14, 0xBC05, false, &y, &step), after;

    jsr_push(0xBC08);
    r = SoundAndSpawnEveryFourthFrame(slot, y);
    jsr_pop();
    r = sound_end_after(e, r);
    jsr_push(0xBC0B);
    move = ApplyVelocityX(slot);
    jsr_pop();
    after = ended_of_axis_move(move);
    high = RD8(OBJ->x_hi[slot]);
    after = then(after, ended_load(high));
    if (high != 0) {
        ClearObject(slot);
    }
    return sound_end_then(r, after);
}

void abi_StepListSpawnAndMoveXOrClear(void)
{
    abi_sound_end(StepListSpawnAndMoveXOrClear(cpu.x, cpu.y));
}

/*
 * @routine 05CC:489 ClearIfAboveOrFall
 * @run 70:BC89
 * WHAT IT DOES: When the object's Y high byte is not 0 the object is cleared; otherwise it falls and moves.
 * HOW: A test, a tail jump into ClearObject, or ApplyGravityUnlimited and a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the load, or as ApplyVelocity.
 */
Ended ClearIfAboveOrFall(uint8_t slot)
{
    uint8_t high;
    GravityResult gravity;
    AxisMove move;
    Ended e;

    high = RD8(OBJ->y_hi[slot]);
    e = ended_load(high);
    if (high != 0) {
        ClearObject(slot);
        return e;
    }
    jsr_push(0xBC94);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    (void)gravity;
    move = ApplyVelocity(slot);
    return ended_of_axis_move(move);
}

void abi_ClearIfAboveOrFall(void)
{
    abi_ended(ClearIfAboveOrFall(cpu.x));
}

/*
 * @routine 05CC:430 LoadRecordRandomFlingOrFall
 * @run 70:BC30
 * @stop BC69
 * WHAT IT DOES: In state 0 loads the record, picks frame $23 or $24 at random and the speed (X and Y) from the table at $BC69 by another random number, and writes $BE to OBJ_Y_LO of the slot the record's Y names. Later it runs ClearIfAboveOrFall.
 * HOW: LoadObjectRecord, two NextRandom calls, table reads, stores, a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. OBJ_Y_LO of slot 11 is written because LoadObjectRecord leaves Y 11.
 */
Ended LoadRecordRandomFlingOrFall(uint8_t slot)
{
    uint8_t state, last, nz, pick, y, value;
    bool carry;
    RandomResult random;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return then(ended_load(state), ClearIfAboveOrFall(slot));
    }
    jsr_push(0xBC38);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    y = 11;
    jsr_push(0xBC3B);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x01;
    e = then(e, ended_load(pick));
    sum = add_bytes(pick, 0x23, false);
    WR8(OBJ->frame[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    WR8(OBJ->y_lo[y], 0xBE);
    e = then(e, ended_load(0xBE));
    jsr_push(0xBC4B);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)((random.value & 0x07) << 2);
    y = pick;
    e = then(ended_with_y(then(e, ended_load(pick)), y), ended_with_carry(ended_kept(), false));
    value = rd8((uint16_t)(0xBC6A + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBC69 + y));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xBC6C + y));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xBC6B + y));
    WR8(OBJ->vy_lo[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadRecordRandomFlingOrFall(void)
{
    abi_ended(LoadRecordRandomFlingOrFall(cpu.x));
}

/*
 * @routine 05CC:498 LoadRecordThenClearEnemiesAtScreen2
 * @run 70:BC98
 * WHAT IT DOES: In state 0 loads the record. Later, when the scroll's screen byte is 2, it sets the scroll's start high byte to 2, clears every object of kind $A0 whose flags' low nibble is 2 in slots $10 to $53, sets bit 4 of zField8E, plays sound $FE, draws tile script 0 and clears itself.
 * HOW: LoadObjectRecord, a loop over the slots with ClearObject, SoundCommand, DrawTileScript, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd LoadRecordThenClearEnemiesAtScreen2(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, screen, x, kind, flags, masked;
    bool carry;
    TileRun run;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        r.end = then(ended_load(state), ended_of_record(last, nz, carry));
        return r;
    }
    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 2);
    if (screen != 2) {
        r.end = e;
        return r;
    }
    wr8(ScrollXMinHi, 2);
    push8(slot);
    x = 0x10;
    do {
        kind = RD8(OBJ->kind[x]);
        e = ended_compare(kind, 0xA0);
        if (kind == 0xA0) {
            flags = RD8(OBJ->flags[x]);
            masked = flags & 0x0F;
            e = ended_compare(masked, 2);
            if (masked == 2) {
                jsr_push(0xBCC3);
                ClearObject(x);
                jsr_pop();
            }
        }
        x++;
        e = then(e, l8_compare_y(x, 0x54));
    } while (x < 0x54);
    slot = pull8();
    e = then(e, ended_nz(slot));
    RMW8(ZP->field8e, old, old | 0x10);
    r.sounded = true;
    r.command = 0xFE;
    r.before = then(e, ended_load(0xFE));
    jsr_push(0xBCD0);
    r.call = SoundCommand(0xFE, slot, y_entry);
    jsr_pop();
    jsr_push(0xBCD5);
    run = DrawTileScript(0, 0);
    jsr_pop();
    ClearObject(slot);
    r.end = ended_of_tile_run(run, 0);
    return r;
}

void abi_LoadRecordThenClearEnemiesAtScreen2(void)
{
    abi_sound_end(LoadRecordThenClearEnemiesAtScreen2(cpu.x, cpu.y));
}

/* The code at $BB62, which no script ran and which has no row of its own: C 0 when the object's X is inside the band from $0010 to $00EF on the first screen, C 1 when not. */
static Ended inside_edge_band(uint8_t slot)
{
    uint8_t high, low;
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    e = ended_load(high);
    if (high != 0) {
        return ended_with_carry(e, true);
    }
    low = RD8(OBJ->x_lo[slot]);
    e = ended_compare(low, 0xF0);
    if (low >= 0xF0) {
        return ended_with_carry(e, true);
    }
    e = ended_compare(low, 0x10);
    return ended_with_carry(e, low < 0x10);
}

/*
 * @routine 05CC:282 LoadRecordThenFollowOrHomeIn
 * @run 70:BA82
 * WHAT IT DOES: In state 0 loads the record and sets the Y speed $1A. Later (unless the game's sub-mode is 4) it takes the place of slot $10 and moves by an X speed from the table at $BA90 by its number and slot $10's facing. While OBJ_34B8 is 0 and slot $10 is in state 4 with OBJ_2800 of 2 or 3 it waits for a hit of 1 to 4, then takes the distance to the player as its aim (OBJ_3510, OBJ_3568) and sets OBJ_34B8 1. While OBJ_34B8 is not 0 it homes in: it puts slot 0 at its place and moves it by the aim (turned on a step by the sign of each part), restores the speed of slot $10 and ends.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, ApplyVelocity, CopyObjectPlace, ApplyVelocity with X and Y swapped, compares, table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The arm that takes the aim and the whole of the homing arm ran in no script (the draft stops there); the code at $BB62 that the homing arm calls has no row and is written here as inside_edge_band.
 */
Ended LoadRecordThenFollowOrHomeIn(uint8_t slot)
{
    uint8_t state, mode, facing, index, value, held, other, flags, hits, aim, y;
    uint8_t last, nz, placed;
    bool carry;
    AxisMove move;
    Sum sum, diff;
    Ended e, band;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xBA8A);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->vy_hi[slot], 0x1A);
        return then(ended_of_record(last, nz, carry), ended_load(0x1A));
    }
    mode = RD8(ZP->game_sub_mode);
    e = ended_compare(mode, 4);
    if (mode == 4) {
        return e;
    }
    jsr_push(0xBA9F);
    placed = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x10), 0x10)), ended_load(placed));
    diff = sub_bytes(slot, 0x11, true);
    index = (uint8_t)(diff.value << 1);
    e = then(e, ended_with_carry(ended_load(index), (diff.value & 0x80) != 0));
    facing = RD8(OBJ->facing_left[0x10]);
    sum = add_bytes(index, facing, false);
    y = sum.value;
    e = ended_with_y(then(e, ended_of_sum(sum)), y);
    value = rd8((uint16_t)(0xBA90 + y));
    WR8(OBJ->vx_hi[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xBAB2);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    held = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(held));
    if (held == 0) {
        other = RD8(OBJ->state[0x10]);
        e = then(e, ended_compare(other, 4));
        if (other != 4) {
            return e;
        }
        other = RD8(OBJ->f2800[0x10]);
        e = then(e, ended_compare(other, 2));
        if (other != 2) {
            e = then(e, ended_compare(other, 3));
            if (other != 3) {
                return e;
            }
        }
        WR8(OBJ->hitbox_on[slot], 2);
        e = then(e, ended_load(2));
        hits = RD8(OBJ->hit_taken[slot]);
        e = then(e, ended_load(hits));
        if (hits == 0) {
            return e;
        }
        e = then(e, ended_compare(hits, 5));
        if (hits >= 5) {
            return e;
        }
        aim = RD8(OBJ->x_lo[0]);
        diff = sub_bytes(aim, RD8(OBJ->x_lo[slot]), true);
        WR8(OBJ->f3510[slot], diff.value);
        aim = RD8(OBJ->y_lo[0]);
        diff = sub_bytes(aim, RD8(OBJ->y_lo[slot]), true);
        WR8(OBJ->f3568[slot], diff.value);
        e = then(e, ended_of_sum(diff));
        WR8(OBJ->f34b8[slot], 1);
        return then(e, ended_load(1));
    }
    jsr_push(0xBAF5);
    band = inside_edge_band(slot);
    jsr_pop();
    e = then(e, band);
    if (!band.c) {
        other = RD8(OBJ->state[0x10]);
        e = then(e, ended_compare(other, 4));
        if (other != 4) {
            WR8(OBJ->hitbox_on[slot], 0);
            WR8(OBJ->f34b8[slot], 0);
            return e;
        }
    }
    jsr_push(0xBB03);
    placed = CopyObjectPlace(slot, 0);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0), 0)), ended_load(placed));
    value = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[0], value);
    value = RD8(OBJ->f3568[slot]);
    WR8(OBJ->vy_hi[0], value);
    e = then(e, ended_load(value));
    jsr_push(0xBB13);
    move = ApplyVelocity(0);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    value = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load(value));
    if (value != 0) {
        index = (uint8_t)((value >> 7) & 0x01);
        e = then(e, ended_with_y(ended_load(index), index));
        value = RD8(OBJ->f3510[slot]);
        sum = add_bytes(value, rd8((uint16_t)(0xBB60 + index)), false);
        WR8(OBJ->f3510[slot], sum.value);
        e = then(e, ended_of_sum(sum));
    }
    value = RD8(OBJ->f3568[slot]);
    e = then(e, ended_load(value));
    if (value != 0) {
        index = (uint8_t)((value >> 7) & 0x01);
        e = then(e, ended_with_y(ended_load(index), index));
        value = RD8(OBJ->f3568[slot]);
        sum = add_bytes(value, rd8((uint16_t)(0xBB60 + index)), false);
        WR8(OBJ->f3568[slot], sum.value);
        e = then(e, ended_of_sum(sum));
    }
    flags = RD8(OBJ->vx_hi[0x10]);
    WR8(OBJ->vx_hi[0], flags);
    flags = RD8(OBJ->vx_lo[0x10]);
    WR8(OBJ->vx_lo[0], flags);
    WR8(OBJ->vy_hi[0], 0xFF);
    WR8(OBJ->vy_lo[0], 0);
    WR8(OBJ->f3510[0], 2);
    WR8(OBJ->f3568[0], 0);
    return then(e, ended_load(2));
}

void abi_LoadRecordThenFollowOrHomeIn(void)
{
    abi_ended(LoadRecordThenFollowOrHomeIn(cpu.x));
}
