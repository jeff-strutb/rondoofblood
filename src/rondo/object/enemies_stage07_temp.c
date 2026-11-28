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

static Ended fall_and_leave_when_low_exit_shim(ObjArm r);

enum { S7_SN_STOPPED, S7_SN_FAR, S7_SN_ZERO, S7_SN_LOW_HIGH, S7_SN_LOW };
enum { S7_SH_ZERO, S7_SH_SIZED };
enum { S7_EN_FAR, S7_EN_NEAR };
enum { S7_AF_RUNNING, S7_AF_ENDED };
enum { S7_SA_PLAIN, S7_SA_STARTED };
enum { S7_SR_LATER, S7_SR_START };
enum { S7_RM_FOUND, S7_RM_NONE };
enum { S7_SE_DONE, S7_SE_NO_SLOT };
enum { S7_SP_NO_STATE, S7_SP_NO_FLAG, S7_SP_SPAWNED };
enum { S7_AR_NEAR, S7_AR_STRAIGHT, S7_AR_TURNED };
enum { S7_CT_RECORD, S7_CT_COUNTING, S7_CT_NO_SLOT, S7_CT_SPAWNED };

/*
 * @routine 0BFB:3DC EnterState4IfFlagged
 * @run 70:B3DC
 * WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
 * HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
 */
ObjArm EnterState4IfFlagged(uint8_t slot)
{
    return enter_state_if_flagged(slot, 4);
}

void abi_EnterState4IfFlagged(void)
{
    abi_ended(load_exit_shim(EnterState4IfFlagged(cpu.x)));
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
ObjArm ScrollNotAtRightEnd(void)
{
    ObjArm r = { 0 };

    r.first = RD8(ZP->field82);
    r.arm = S7_SN_STOPPED;
    r.flag = true;
    if (r.first != 0) {
        return r;
    }
    r.fourth = rd8(ScrollXMaxHi);
    r.fifth = RD8(SCROLLE->x_hi_screen[0]);
    r.second = (uint8_t)(r.fourth - r.fifth);
    r.arm = S7_SN_FAR;
    if (r.second >= 2) {
        return r;
    }
    r.arm = S7_SN_ZERO;
    r.flag = false;
    if (r.second == 0) {
        return r;
    }
    r.third = RD8(SCROLL->x_lo[0]);
    r.arm = S7_SN_LOW_HIGH;
    if (r.third >= 0xE0) {
        return r;
    }
    r.arm = S7_SN_LOW;
    r.flag = true;
    return r;
}

static Ended scroll_not_at_right_end_exit_shim(ObjArm r)
{
    bool overflow = sub_overflow(r.fourth, r.fifth, r.second);

    switch (r.arm) {
    case S7_SN_STOPPED:
        return ended_with_carry(ended_load(r.first), true);
    case S7_SN_FAR:
        return ended_with_overflow(ended_compare(r.second, 2), overflow);
    case S7_SN_ZERO:
        return ended_with_overflow(ended_with_carry(ended_compare(r.second, 0), false), overflow);
    case S7_SN_LOW_HIGH:
        return ended_with_overflow(ended_with_carry(ended_compare(r.third, 0xE0), false), overflow);
    default:
        return ended_with_overflow(ended_with_carry(ended_compare(r.third, 0xE0), true), overflow);
    }
}

void abi_ScrollNotAtRightEnd(void)
{
    abi_ended(scroll_not_at_right_end_exit_shim(ScrollNotAtRightEnd()));
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
ObjArm SetHitboxSizeFromFrame(uint8_t slot)
{
    enum { TABLE = 0xBAC9 };
    ObjArm r = { 0 };
    uint8_t value;

    r.first = RD8(OBJ->frame[slot]);
    r.arm = S7_SH_ZERO;
    if (r.first == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return r;
    }
    r.second = (uint8_t)((uint8_t)(r.first - 0x2A) << 1);
    value = rd8((uint16_t)(TABLE + r.second));
    WR8(OBJ->hitbox_half_w[slot], value);
    r.third = rd8((uint16_t)(TABLE + 1 + r.second));
    WR8(OBJ->hitbox_half_h[slot], r.third);
    r.arm = S7_SH_SIZED;
    return r;
}

static Ended set_hitbox_size_from_frame_exit_shim(ObjArm r)
{
    uint8_t diff = (uint8_t)(r.first - 0x2A);

    if (r.arm == S7_SH_ZERO) {
        return ended_load(0);
    }
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(r.third), (diff & 0x80) != 0), r.second),
                               sub_overflow(r.first, 0x2A, diff));
}

void abi_SetHitboxSizeFromFrame(void)
{
    abi_ended(set_hitbox_size_from_frame_exit_shim(SetHitboxSizeFromFrame(cpu.x)));
}

/*
 * @routine 0BFC:3B3 ChangeToType9C
 * @run 70:BBB3
 * WHAT IT DOES: Turns the object into one of type $9C, in state 0.
 * HOW: Stores $9C in its kind byte and zero in its state.
 * WAITS: no.
 * Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
 */
ObjArm ChangeToType9C(uint8_t slot)
{
    return become_type(slot, 0x9C);
}

void abi_ChangeToType9C(void)
{
    abi_ended(load_exit_shim(ChangeToType9C(cpu.x)));
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
ObjArm EnterNextStateWhenPlayerNear(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB3EC);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.first = RD8(OBJ->start_arg[slot]);
    r.arm = S7_EN_FAR;
    if (r.distance.distance >= r.first) {
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->life_timer[slot], 0x20);
    r.second = INC8(OBJ->state[slot]);
    r.arm = S7_EN_NEAR;
    return r;
}

static Ended enter_next_state_when_player_near_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_distance(r.distance), ended_compare(r.distance.distance, r.first));

    if (r.arm == S7_EN_FAR) {
        return e;
    }
    return ended_then(e, ended_with_a(ended_nz(r.second), 0x20));
}

void abi_EnterNextStateWhenPlayerNear(void)
{
    abi_ended(enter_next_state_when_player_near_exit_shim(EnterNextStateWhenPlayerNear(cpu.x)));
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
ObjArm SwingOnCosine(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0, angle, value;

    WR8(ZP->temp10, 0xF6);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xB9D1);
    StepObjectFrames(slot, &exit_y);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->life_timer[slot]);
    jsr_push(0xB9DF);
    ScaleByCosine(angle);
    jsr_pop();
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    jsr_push(0xB9EC);
    ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->life_timer[slot]);
    r.sum = add_bytes(angle, 2, false);
    WR8(OBJ->life_timer[slot], r.sum.value);
    return r;
}

static Ended swing_on_cosine_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.sum.value), r.sum.carry), r.sum.overflow), 0);
}

void abi_SwingOnCosine(void)
{
    abi_ended(swing_on_cosine_exit_shim(SwingOnCosine(cpu.x)));
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
ObjArm AnimateThenFrameRecord(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(ZP->temp10, 0x4B);
    WR8(ZP->temp11, 0xBB);
    jsr_push(0xBB3B);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S7_AF_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->f3358[slot], 2);
    WR8(OBJ->f3300[slot], 0x0E);
    r.first = INC8(OBJ->state[slot]);
    r.arm = S7_AF_ENDED;
    return r;
}

static Ended animate_then_frame_record_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S7_AF_RUNNING) {
        return e;
    }
    return ended_then(e, ended_with_a(ended_nz(r.first), 0x0E));
}

void abi_AnimateThenFrameRecord(void)
{
    abi_ended(animate_then_frame_record_exit_shim(AnimateThenFrameRecord(cpu.x)));
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
ObjArm StartThenAnimate(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t exit_y = 0;

    r.arm = S7_SA_PLAIN;
    if (state == 0) {
        jsr_push(0xD1AB);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->f2a10[slot], 6);
        r.arm = S7_SA_STARTED;
    }
    WR8(ZP->temp11, 0xD1);
    WR8(ZP->temp10, 0xBC);
    jsr_push(0xD1BB);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    return r;
}

static Ended start_then_animate_exit_shim(ObjArm r)
{
    Ended e = ended_kept();

    if (r.arm == S7_SA_STARTED) {
        e = ended_then(record_exit_shim(r), ended_load(6));
    }
    return ended_then(e, frame_step_exit_shim(r));
}

void abi_StartThenAnimate(void)
{
    abi_ended(start_then_animate_exit_shim(StartThenAnimate(cpu.x)));
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
ObjArm StartWithRecordTableRow(uint8_t slot)
{
    enum { ROWS = 0xBBF5 };
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t y;

    if (state != 0) {
        r.arm = S7_SR_LATER;
        r.sub = obj_sub(FallAndLeaveWhenLow(slot));
        return r;
    }
    r.arm = S7_SR_START;
    jsr_push(0xBBC4);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    y = (uint8_t)(RD8(OBJ->life_timer[slot]) << 3);
    push8(y);
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ROWS + y)));
    jsr_push(0xBBD5);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    y = pull8();
    r.third = y;
    WR8(OBJ->frame[slot], rd8((uint16_t)(ROWS + 1 + y)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(ROWS + 3 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(ROWS + 2 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ROWS + 5 + y)));
    r.fourth = rd8((uint16_t)(ROWS + 4 + y));
    WR8(OBJ->vy_lo[slot], r.fourth);
    return r;
}

static Ended start_with_record_table_row_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S7_SR_LATER) {
        return fall_and_leave_when_low_exit_shim(obj_arm(r.sub));
    }
    e = ended_then(ended_then(record_exit_shim(r), ended_with_y(ended_load(r.first), r.third)), ended_of_axis_move(r.move));
    e = ended_then(e, ended_with_y(ended_nz(r.third), r.third));
    return ended_then(e, ended_load(r.fourth));
}

void abi_StartWithRecordTableRow(void)
{
    abi_ended(start_with_record_table_row_exit_shim(StartWithRecordTableRow(cpu.x)));
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
ObjArm FallAndLeaveWhenLow(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBC38);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xBC3B);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first != 0) {
        ClearObject(slot);
    }
    return r;
}

static Ended fall_and_leave_when_low_exit_shim(ObjArm r)
{
    return ended_then(ended_of_axis_move(r.move), ended_load(r.first));
}

void abi_FallAndLeaveWhenLow(void)
{
    abi_ended(fall_and_leave_when_low_exit_shim(FallAndLeaveWhenLow(cpu.x)));
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
ObjArm AnimateThenRemove(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(ZP->temp10, 0xA6);
    WR8(ZP->temp11, 0xBB);
    jsr_push(0xBBA0);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    if (r.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return r;
}

void abi_AnimateThenRemove(void)
{
    abi_ended(frame_step_exit_shim(AnimateThenRemove(cpu.x)));
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
ObjArm RemoveFirstFreeSlotFrom1C(void)
{
    ObjArm r = { 0 };
    uint8_t y, type = 0;

    for (y = 0x1C; y < 0x20; y++) {
        type = RD8(OBJ->kind[y]);
        if (type == 0) {
            jsr_push(0xBA58);
            ClearObjectInY(y);
            jsr_pop();
            r.arm = S7_RM_FOUND;
            r.flag = true;
            r.first = y;
            return r;
        }
    }
    r.arm = S7_RM_NONE;
    r.second = type;
    return r;
}

static Ended remove_first_free_slot_from1c_exit_shim(ObjArm r)
{
    if (r.arm == S7_RM_FOUND) {
        return ended_of_find_slot(r.first, 0, true);
    }
    return ended_of_find_slot(0x20, r.second, false);
}

void abi_RemoveFirstFreeSlotFrom1C(void)
{
    abi_ended(remove_first_free_slot_from1c_exit_shim(RemoveFirstFreeSlotFrom1C()));
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
ObjArm SpawnEightPiecesType37(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t piece = 0, found_slot, value;
    bool found;

    r.arm = S7_SE_DONE;
    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xBA97);
        found_slot = FindFreeSlotFrom30(&r.fifth, &found);
        jsr_pop();
        r.fourth = found_slot;
        if (!found) {
            r.arm = S7_SE_NO_SLOT;
            return r;
        }
        jsr_push(0xBA9E);
        StartObjectAtPlace(0x37, slot, found_slot);
        jsr_pop();
        value = RD8(ZP->hit_slot);
        WR8(OBJ->life_timer[found_slot], value);
        piece = RD8(ZP->hit_slot);
        piece++;
        r.first++;
        r.second = piece;
        r.third = value;
    } while (piece < 8);
    return r;
}

/* Each started piece's pass replaces the exit before it with its own count; a search that found no slot follows the last pass, or LDY #0. */
static Ended spawn_eight_pieces_type37_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_kept(), 0);

    if (r.first != 0) {
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(r.second - 8)), r.third), r.second >= 8), r.second);
    }
    if (r.arm == S7_SE_NO_SLOT) {
        e = ended_then(e, ended_of_find_slot(r.fourth, r.fifth, false));
    }
    return e;
}

void abi_SpawnEightPiecesType37(void)
{
    abi_ended(spawn_eight_pieces_type37_exit_shim(SpawnEightPiecesType37(cpu.x)));
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
ObjArm SpawnPiecesWhenFlaggedHit(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.arm = S7_SP_NO_STATE;
    if (r.first == 0) {
        return r;
    }
    r.second = (uint8_t)(RD8(OBJ->hit_status[slot]) & 0x80);
    r.arm = S7_SP_NO_FLAG;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xBA8B);
    r.sub = obj_sub(SpawnEightPiecesType37(slot));
    jsr_pop();
    WR8(OBJ->state[slot], 4);
    r.arm = S7_SP_SPAWNED;
    return r;
}

static Ended spawn_pieces_when_flagged_hit_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == S7_SP_NO_STATE) {
        return e;
    }
    e = ended_then(e, ended_load(r.second));
    if (r.arm == S7_SP_NO_FLAG) {
        return e;
    }
    e = ended_then(e, spawn_eight_pieces_type37_exit_shim(obj_arm(r.sub)));
    return ended_then(e, ended_load(4));
}

void abi_SpawnPiecesWhenFlaggedHit(void)
{
    abi_ended(spawn_pieces_when_flagged_hit_exit_shim(SpawnPiecesWhenFlaggedHit(cpu.x)));
}

/*
 * @routine 0BFC:2E3 StartAtRandomSpeedTowardPlayer
 * @run 70:BAE3
 * WHAT IT DOES: Loads the record, gives the object an X speed (high byte) from a table at overlay address $BB20
 * by three bits of PlayerDistanceSum and moves it along X. When the player is nearer than $18 along X it removes
 * itself; otherwise it takes a speed from a table of word pairs at $BB28 by two bits of PlayerDistanceSum, faces
 * the player's side (bit 0 of SideOfPlayer into facing_left) and reverses its speed when that bit is set.
 * HOW: LoadObjectRecord, PlayerDistanceSum twice, ApplyVelocityX, DistanceXToPlayer, SideOfPlayer, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, D clear. Exit: removed: A the distance, N, Z, C from its compare with $18; otherwise
 * A, N, Z, C, V from NegateVelocityX when it ran, else A the side bit and N, Z from it.
 */
ObjArm StartAtRandomSpeedTowardPlayer(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBAE6);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xBAE9);
    r.sum = PlayerDistanceSum(slot, r.flag);
    jsr_pop();
    r.third = (uint8_t)(r.sum.value & 7);
    r.fourth = rd8((uint16_t)(0xBB20 + r.third));
    WR8(OBJ->vx_hi[slot], r.fourth);
    jsr_push(0xBAF5);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0xBAF8);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = S7_AR_NEAR;
    if (r.distance.distance < 0x18) {
        ClearObject(slot);
        return r;
    }
    jsr_push(0xBB02);
    r.sum2 = PlayerDistanceSum(slot, true);
    jsr_pop();
    r.fifth = (uint8_t)((r.sum2.value & 3) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xBB29 + r.fifth)));
    r.sixth = rd8((uint16_t)(0xBB28 + r.fifth));
    WR8(OBJ->vx_lo[slot], r.sixth);
    jsr_push(0xBB15);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.seventh = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.seventh);
    r.arm = S7_AR_STRAIGHT;
    if (r.seventh != 0) {
        jsr_push(0xBB1F);
        r.sum3 = NegateVelocityX(slot);
        jsr_pop();
        r.arm = S7_AR_TURNED;
    }
    return r;
}

static Ended start_at_random_speed_toward_player_exit_shim(ObjArm r)
{
    Ended e = record_exit_shim(r);

    e = ended_then(e, ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.fourth), r.third), r.sum.carry), r.sum.overflow));
    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x18));
    if (r.arm == S7_AR_NEAR) {
        return e;
    }
    e = ended_then(e, ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.sixth), r.fifth), false), r.sum2.overflow));
    e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.seventh));
    if (r.arm == S7_AR_STRAIGHT) {
        return e;
    }
    return ended_then(e, ended_of_sum(r.sum3));
}

void abi_StartAtRandomSpeedTowardPlayer(void)
{
    abi_ended(start_at_random_speed_toward_player_exit_shim(StartAtRandomSpeedTowardPlayer(cpu.x)));
}

/*
 * @routine 0BFC:1FF CountTimerPairDownThenSpawn36
 * @run 70:B9FF
 * WHAT IT DOES: In state 0 it loads its record. Later it counts the sixteen bit value life_timer (high) and
 * f3510 (low) down; when it is zero it frees the first empty slot of $1C to $1F, starts an object of type $36
 * there at its place and sets the pair again from a table of word pairs at overlay address $BA3F by two bits of
 * PlayerDistanceSum.
 * HOW: RemoveFirstFreeSlotFrom1C, StartObjectAtPlace, PlayerDistanceSum. The arm with no empty slot ran in no
 * script: the draft stops there, so liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, D clear. Exit: state 0: as LoadObjectRecord; counting: A the new high byte and N, Z, C, V from
 * the borrow subtract; when none of the four slots is empty, as RemoveFirstFreeSlotFrom1C; otherwise A the low
 * byte set, N, Z from it, C and V from the sum, Y the table index.
 */
ObjArm CountTimerPairDownThenSpawn36(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high;
    Sum sum;
    ObjArm freed;

    if (RD8(OBJ->state[slot]) == 0) {
        r.arm = S7_CT_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    r.first = RD8(OBJ->life_timer[slot]);
    r.first = (uint8_t)(r.first | RD8(OBJ->f3510[slot]));
    if (r.first != 0) {
        r.arm = S7_CT_COUNTING;
        low = RD8(OBJ->f3510[slot]);
        sum = sub_bytes(low, 1, true);
        WR8(OBJ->f3510[slot], sum.value);
        high = RD8(OBJ->life_timer[slot]);
        r.sum = sub_bytes(high, 0, sum.carry);
        WR8(OBJ->life_timer[slot], r.sum.value);
        return r;
    }
    jsr_push(0xBA24);
    freed = RemoveFirstFreeSlotFrom1C();
    jsr_pop();
    r.sub = obj_sub(freed);
    r.arm = S7_CT_NO_SLOT;
    if (!freed.flag) {
        return r;
    }
    jsr_push(0xBA2B);
    StartObjectAtPlace(0x36, slot, freed.first);
    jsr_pop();
    jsr_push(0xBA2E);
    r.sum2 = PlayerDistanceSum(slot, true);
    jsr_pop();
    r.second = (uint8_t)((r.sum2.value & 3) << 1);
    WR8(OBJ->life_timer[slot], rd8((uint16_t)(0xBA40 + r.second)));
    r.third = rd8((uint16_t)(0xBA3F + r.second));
    WR8(OBJ->f3510[slot], r.third);
    r.arm = S7_CT_SPAWNED;
    return r;
}

static Ended count_timer_pair_down_then_spawn36_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S7_CT_RECORD) {
        return record_exit_shim(r);
    }
    if (r.arm == S7_CT_COUNTING) {
        return ended_of_sum(r.sum);
    }
    e = ended_then(ended_load(r.first), remove_first_free_slot_from1c_exit_shim(obj_arm(r.sub)));
    if (r.arm == S7_CT_NO_SLOT) {
        return e;
    }
    return ended_then(e, ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.third), r.second), false), r.sum2.overflow));
}

void abi_CountTimerPairDownThenSpawn36(void)
{
    abi_ended(count_timer_pair_down_then_spawn36_exit_shim(CountTimerPairDownThenSpawn36(cpu.x)));
}

/*
 * @routine 0BFC:15F StartFromScreenEdgeByScroll
 * @run 70:B95F
 * WHAT IT DOES: Loads the record, then puts the object at one edge of the screen heading across it: on the left
 * edge (X $FFE0, speed $0140, facing left set) when the player's X low byte is $C0 or more, or between $40 and
 * $BF when the scroll is not at its right end or the player faces left; otherwise on the right (X $0120, speed
 * -$0140). Its Y is the player's, thrown up at -$1000 and moved once, kept at $30 or more.
 * HOW: LoadObjectRecord, ScrollNotAtRightEnd, ApplyVelocityY.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $30 and N, Z clear when the player's Y low byte is below $30; otherwise A the
 * player's Y low byte, N, Z, C from its compare with $30; V from ApplyVelocityY; Y 11.
 */
ObjArm StartFromScreenEdgeByScroll(uint8_t slot)
{
    ObjArm r = { 0 };
    ObjArm scroll;
    uint8_t player_x, nz;
    bool carry, left;

    jsr_push(0xB962);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    player_x = RD8(OBJ->x_lo[0]);
    left = false;
    if (player_x >= 0x40) {
        if (player_x >= 0xC0) {
            left = true;
        } else {
            jsr_push(0xB96F);
            scroll = ScrollNotAtRightEnd();
            jsr_pop();
            left = !scroll.flag || RD8(OBJ->facing_left[0]) != 0;
        }
    }
    if (left) {
        WR8(OBJ->facing_left[slot], 1);
        WR8(OBJ->x_hi[slot], 0xFF);
        WR8(OBJ->x_lo[slot], 0xE0);
        WR8(OBJ->vx_hi[slot], 1);
        WR8(OBJ->vx_lo[slot], 0x40);
    } else {
        WR8(OBJ->x_hi[slot], 1);
        WR8(OBJ->x_lo[slot], 0x20);
        WR8(OBJ->vx_hi[slot], 0xFE);
        WR8(OBJ->vx_lo[slot], 0xC0);
    }
    WR8(OBJ->y_hi[slot], RD8(OBJ->y_hi[0]));
    WR8(OBJ->y_lo[slot], RD8(OBJ->y_lo[0]));
    WR8(OBJ->vy_hi[slot], 0xF0);
    jsr_push(0xB9B9);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_lo[0]);
    if (r.first < 0x30) {
        WR8(OBJ->y_lo[slot], 0x30);
    }
    return r;
}

/* The move and the compare replace the record's exit; LDY #11 stands under them. */
static Ended start_from_screen_edge_by_scroll_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_axis_move(r.move), ended_compare(r.first, 0x30));

    if (r.first < 0x30) {
        e = ended_then(e, ended_load(0x30));
    }
    return ended_then(ended_with_y(ended_kept(), 11), e);
}

void abi_StartFromScreenEdgeByScroll(void)
{
    abi_ended(start_from_screen_edge_by_scroll_exit_shim(StartFromScreenEdgeByScroll(cpu.x)));
}
