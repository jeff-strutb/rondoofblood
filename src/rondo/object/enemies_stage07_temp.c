/*
 * TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/*
 * @routine 0BFB:3DC EnterState4IfFlagged
 * @run 70:B3DC
 * WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
 * HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
 */
Ended EnterState4IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 4);
}

void abi_EnterState4IfFlagged(void)
{
    abi_ended(EnterState4IfFlagged(cpu.x));
}

/*
 * @routine 0BFC:141 ScrollNotAtRightEnd
 * @run 70:B941
 * WHAT IT DOES: Says, in the carry, that the scroll has not come to the right end of the level:
 * carry set when zField82 is set, when two or more screens remain, or when one screen remains
 * and the scroll's low byte is under $E0; clear otherwise.
 * HOW: Subtracts the first scroll entry's high screen byte from the scroll's limit
 * (ScrollXMaxHi) and tests the difference.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A and N, Z from the last load or compare; V from the
 * subtract. Y and X kept. The arms past the screen difference of 0 or 1 ran in no script: the
 * draft stops there and liftcmp judges only the first tests; the rest waits for coverage
 * (PROCEDURE section 6).
 */
Ended ScrollNotAtRightEnd(void)
{
    uint8_t stop = RD8(ZP->field82);
    uint8_t limit, screen, remaining, low;
    bool overflow;

    if (stop != 0) {
        return ended_with_carry(ended_load(stop), true);
    }
    limit = rd8(ScrollXMaxHi);
    screen = RD8(SCROLLE->x_hi_screen[0]);
    remaining = (uint8_t)(limit - screen);
    overflow = sub_overflow(limit, screen, remaining);
    if (remaining >= 2) {
        return ended_with_overflow(ended_compare(remaining, 2), overflow);
    }
    if (remaining == 0) {
        return ended_with_overflow(ended_with_carry(ended_compare(remaining, 0), false), overflow);
    }
    low = RD8(SCROLL->x_lo[0]);
    if (low >= 0xE0) {
        return ended_with_overflow(ended_with_carry(ended_compare(low, 0xE0), false), overflow);
    }
    return ended_with_overflow(ended_with_carry(ended_compare(low, 0xE0), true), overflow);
}

void abi_ScrollNotAtRightEnd(void)
{
    abi_ended(ScrollNotAtRightEnd());
}

/*
 * @routine 0BFC:2AB SetHitboxSizeFromFrame
 * @run 70:BAAB
 * WHAT IT DOES: Sets the object's hit rectangle's half width and half height from a two byte
 * row of a table by its frame minus $2A; frame 0 clears both.
 * HOW: The table is at overlay address $BAC9.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 7 of the frame minus $2A
 * (the shift); V from the subtract; Y the row's offset; X kept.
 */
Ended SetHitboxSizeFromFrame(uint8_t slot)
{
    enum { TABLE = 0xBAC9 };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t diff, y, value;
    bool overflow;

    if (frame == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return ended_load(0);
    }
    diff = (uint8_t)(frame - 0x2A);
    overflow = sub_overflow(frame, 0x2A, diff);
    y = (uint8_t)(diff << 1);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (diff & 0x80) != 0), y),
                               overflow);
}

void abi_SetHitboxSizeFromFrame(void)
{
    abi_ended(SetHitboxSizeFromFrame(cpu.x));
}

/*
 * @routine 0BFC:3B3 ChangeToType9C
 * @run 70:BBB3
 * WHAT IT DOES: Turns the object into one of type $9C, in state 0.
 * HOW: Stores $9C in its kind byte and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
 */
Ended ChangeToType9C(uint8_t slot)
{
    return become_type(slot, 0x9C);
}

void abi_ChangeToType9C(void)
{
    abi_ended(ChangeToType9C(cpu.x));
}

/*
 * @routine 0BFB:3E9 EnterNextStateWhenPlayerNear
 * @run 70:B3E9
 * WHAT IT DOES: When the player is nearer along X than the distance in the object's next_kind byte,
 * sets f3250 to 2 and life_timer to $20 and moves the object on to its next state.
 * HOW: DistanceXToPlayer compared with next_kind (read once).
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the compare left them (V from the distance's subtract), or
 * A $20 and N, Z from the new state; X, Y kept.
 */
Ended EnterNextStateWhenPlayerNear(uint8_t slot)
{
    PlayerDistance distance;
    uint8_t limit, state;
    Ended e;

    jsr_push(0xB3EC);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    limit = RD8(OBJ->next_kind[slot]);
    e = ended_then(ended_of_distance(distance), ended_compare(distance.distance, limit));
    if (distance.distance >= limit) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->life_timer[slot], 0x20);
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 0x20));
}

void abi_EnterNextStateWhenPlayerNear(void)
{
    abi_ended(EnterNextStateWhenPlayerNear(cpu.x));
}

/*
 * @routine 0BFC:1C6 SwingOnCosine
 * @run 70:B9C6
 * WHAT IT DOES: Animates the object along the list at overlay address $B9F6, gives it a Y speed
 * of its life_timer's angle's cosine (speed 1.0), moves it, and turns life_timer on by 2.
 * HOW: StepObjectFrames, ScaleByCosine with zTemp00-01 set to 256, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new life_timer, N, Z, C and V from the add of 2; Y 0.
 * Clobbers zTemp00-01, zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
 */
Ended SwingOnCosine(uint8_t slot)
{
    uint8_t exit_y = 0, angle, value;
    ScaleResult scaled;
    Sum sum;

    WR8(ZP->temp10, 0xF6);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xB9D1);
    StepObjectFrames(slot, &exit_y);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->life_timer[slot]);
    jsr_push(0xB9DF);
    scaled = ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    jsr_push(0xB9EC);
    ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->life_timer[slot]);
    sum = add_bytes(angle, 2, false);
    WR8(OBJ->life_timer[slot], sum.value);
    (void)scaled;
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(sum.value), sum.carry), sum.overflow), 0);
}

void abi_SwingOnCosine(void)
{
    abi_ended(SwingOnCosine(cpu.x));
}

/*
 * @routine 0BFC:330 AnimateThenFrameRecord
 * @run 70:BB30
 * WHAT IT DOES: Animates the object along the list at overlay address $BB4B; when the list has
 * ended, sets f3358 to 2 and f3300 to $0E and moves the object on to its next state.
 * HOW: StepObjectFrames; its result says the list ended.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames, or at the end A $0E and N, Z from the new
 * state; X kept. Clobbers zTemp10-11.
 */
Ended AnimateThenFrameRecord(uint8_t slot)
{
    uint8_t exit_y = 0, state;
    FrameStep step;
    Ended e;

    WR8(ZP->temp10, 0x4B);
    WR8(ZP->temp11, 0xBB);
    jsr_push(0xBB3B);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f3358[slot], 2);
    WR8(OBJ->f3300[slot], 0x0E);
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 0x0E));
}

void abi_AnimateThenFrameRecord(void)
{
    abi_ended(AnimateThenFrameRecord(cpu.x));
}

/*
 * @routine 0BFF:1A3 StartThenAnimate
 * @run 71:D1A3
 * WHAT IT DOES: The first time the object runs (state 0), starts it from its record and sets its
 * f2a10 to 6. Every time, it animates it along the list at overlay address $D1BC.
 * HOW: LoadObjectRecord when the state is 0, then StepObjectFrames as the last call.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames (Y 11 from the record start when the
 * animation does not change it); X and V kept. (A JSR then RTS, not a tail jump.) Clobbers zTemp10-11.
 */
Ended StartThenAnimate(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t exit_y = 0, nz, last;
    bool carry;
    FrameStep step;
    Ended e = ended_kept();

    if (state == 0) {
        jsr_push(0xD1AB);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = ended_of_record(last, nz, carry);
        WR8(OBJ->f2a10[slot], 6);
        e = ended_then(e, ended_load(6));
    }
    WR8(ZP->temp11, 0xD1);
    WR8(ZP->temp10, 0xBC);
    jsr_push(0xD1BB);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    return ended_then(e, ended_of_frame_step(step, step != FRAME_HELD, exit_y));
}

void abi_StartThenAnimate(void)
{
    abi_ended(StartThenAnimate(cpu.x));
}


/*
 * @routine 0BFC:3BC StartWithRecordTableRow
 * @run 70:BBBC
 * @stop BBF5
 * WHAT IT DOES: The first time (state 0) starts the object from its record and gives it a row of
 * a table at overlay address $BBF5 (eight bytes, picked by its life_timer): a first Y speed (the
 * object is moved once by it), its frame, and its X and Y speeds. Past state 0 it runs
 * FallAndLeaveWhenLow, which it jumps to.
 * HOW: LoadObjectRecord; the row offset is kept on the stack across ApplyVelocityY.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the last row byte, N, Z from it, C and V from ApplyVelocityY,
 * Y the row offset; past state 0 as FallAndLeaveWhenLow.
 */
Ended StartWithRecordTableRow(uint8_t slot)
{
    enum { ROWS = 0xBBF5 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t timer, y, value, nz, last;
    bool carry;
    AxisMove move;
    Ended e;

    if (state != 0) {
        return FallAndLeaveWhenLow(slot);
    }
    jsr_push(0xBBC4);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    timer = RD8(OBJ->life_timer[slot]);
    y = (uint8_t)(timer << 3);
    push8(y);
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ROWS + y)));
    jsr_push(0xBBD5);
    move = ApplyVelocityY(slot);
    jsr_pop();
    y = pull8();
    e = ended_then(ended_then(e, ended_with_y(ended_load(last), y)), ended_of_axis_move(move));
    e = ended_then(e, ended_with_y(ended_nz(y), y));
    WR8(OBJ->frame[slot], rd8((uint16_t)(ROWS + 1 + y)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(ROWS + 3 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(ROWS + 2 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ROWS + 5 + y)));
    value = rd8((uint16_t)(ROWS + 4 + y));
    WR8(OBJ->vy_lo[slot], value);
    return ended_then(e, ended_load(value));
}

void abi_StartWithRecordTableRow(void)
{
    abi_ended(StartWithRecordTableRow(cpu.x));
}

/*
 * @routine 0BFC:435 FallAndLeaveWhenLow
 * @run 70:BC35
 * WHAT IT DOES: Pulls the object down with gravity, moves it, and removes it when its Y high
 * byte is not zero (it has fallen off the screen).
 * HOW: ApplyGravityUnlimited, ApplyVelocity, a test; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the Y high byte, N, Z from it; C and V from ApplyVelocity.
 */
Ended FallAndLeaveWhenLow(uint8_t slot)
{
    uint8_t high;
    AxisMove move;
    Ended e;

    jsr_push(0xBC38);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xBC3B);
    move = ApplyVelocity(slot);
    jsr_pop();
    high = RD8(OBJ->y_hi[slot]);
    e = ended_then(ended_of_axis_move(move), ended_load(high));
    if (high != 0) {
        ClearObject(slot);
    }
    return e;
}

void abi_FallAndLeaveWhenLow(void)
{
    abi_ended(FallAndLeaveWhenLow(cpu.x));
}

/*
 * @routine 0BFC:395 AnimateThenRemove
 * @run 70:BB95
 * WHAT IT DOES: Animates the object along the list at overlay address $BBA6; when the list has
 * ended the object removes itself.
 * HOW: StepObjectFrames; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames. Clobbers zTemp10-11.
 */
Ended AnimateThenRemove(uint8_t slot)
{
    uint8_t exit_y = 0;
    FrameStep step;

    WR8(ZP->temp10, 0xA6);
    WR8(ZP->temp11, 0xBB);
    jsr_push(0xBBA0);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return ended_of_frame_step(step, step != FRAME_HELD, exit_y);
}

void abi_AnimateThenRemove(void)
{
    abi_ended(AnimateThenRemove(cpu.x));
}

/*
 * @routine 0BFC:247 RemoveFirstFreeSlotFrom1C
 * @run 70:BA47
 * WHAT IT DOES: Finds the first empty object slot from $1C to $1F and empties it fully (it is
 * already empty; this is the slot search's tail without its start), carry set when it found one.
 * HOW: A loop on the type byte; ClearObjectInY through its vector.
 * WAITS: no.
 * Entry: nothing, D clear. Exit: found: C 1, Y the slot, A 0, N 0, Z 1; none: C 0, Y $20, A the
 * last type, N 0, Z 1. X kept.
 */
Ended RemoveFirstFreeSlotFrom1C(void)
{
    uint8_t y, type = 0;

    for (y = 0x1C; y < 0x20; y++) {
        type = RD8(OBJ->kind[y]);
        if (type == 0) {
            jsr_push(0xBA58);
            ClearObjectInY(y);
            jsr_pop();
            return ended_of_find_slot(y, 0, true);
        }
    }
    return ended_of_find_slot(0x20, type, false);
}

void abi_RemoveFirstFreeSlotFrom1C(void)
{
    abi_ended(RemoveFirstFreeSlotFrom1C());
}

/*
 * @routine 0BFC:291 SpawnEightPiecesType37
 * @run 70:BA91
 * WHAT IT DOES: Starts up to eight objects of type $37 at the object's place, each in a free slot
 * from $30, each with its piece number in life_timer.
 * HOW: A loop over pieces 0 to 7 (kept in zHitSlot): FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: after eight pieces A the last piece number, N, Z, C from the
 * compare of Y with 8, Y 8; when no slot was free as FindFreeSlotFrom30 left it. X kept.
 */
Ended SpawnEightPiecesType37(uint8_t slot)
{
    uint8_t piece = 0, found_slot, last, value;
    bool found;
    Ended e = ended_with_y(ended_kept(), 0);

    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xBA97);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = ended_then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            return e;
        }
        jsr_push(0xBA9E);
        StartObjectAtPlace(0x37, slot, found_slot);
        jsr_pop();
        value = RD8(ZP->hit_slot);
        WR8(OBJ->life_timer[found_slot], value);
        piece = RD8(ZP->hit_slot);
        piece++;
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(piece - 8)), value), piece >= 8), piece);
    } while (piece < 8);
    return e;
}

void abi_SpawnEightPiecesType37(void)
{
    abi_ended(SpawnEightPiecesType37(cpu.x));
}

/*
 * @routine 0BFC:27C SpawnPiecesWhenFlaggedHit
 * @run 70:BA7C
 * WHAT IT DOES: When the object is past state 0 and bit 7 of its status flags is set, spawns its
 * eight pieces (SpawnEightPiecesType37) and enters state 4.
 * HOW: Two tests, then the spawn.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 4 and N, Z clear, C, V and Y as the spawn left them; or A, N,
 * Z from the failing test.
 */
Ended SpawnPiecesWhenFlaggedHit(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t flags, bit;
    Ended e;

    e = ended_load(state);
    if (state == 0) {
        return e;
    }
    flags = RD8(OBJ->status_flags[slot]);
    bit = (uint8_t)(flags & 0x80);
    e = ended_then(e, ended_load(bit));
    if (bit == 0) {
        return e;
    }
    jsr_push(0xBA8B);
    e = ended_then(e, SpawnEightPiecesType37(slot));
    jsr_pop();
    WR8(OBJ->state[slot], 4);
    return ended_then(e, ended_load(4));
}

void abi_SpawnPiecesWhenFlaggedHit(void)
{
    abi_ended(SpawnPiecesWhenFlaggedHit(cpu.x));
}
