/*
 * The shared object pool's state helpers: what the stage overlays' object handlers call at the
 * head of a state, before they jump through the object's state table (the byte OBJ->state is the
 * state, an index into a table of handlers).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* The arms a dispatch by kind ends on. */
enum { DK_NONE, DK_JUMP };

enum { HIT_STATE = 5, HIT_LIMIT = 5, BLINK_LIMIT = 0x18, BLINK_FLAG = 0x86, BLINK_WAVE_MASK = 0x02 };

/*
 * @routine 0519:46A EnterHurtStateOnHit
 * @run 70:A46A
 * WHAT IT DOES: Sends an object that is in one of its first four states and has taken a hit
 * of 1 to 4 into state 5, the state its handler table uses for being hurt.
 * HOW: Reads the state and the hit byte, each once, and writes the state only when both are
 * from 1 to 4.
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare or load left them (see the wrapper);
 * X and Y kept.
 */
HurtCheck EnterHurtStateOnHit(uint8_t slot)
{
    HurtCheck result;
    uint8_t state, hit;

    state = RD8(OBJ->state[slot]);
    result.value = state;
    if (state == 0) {
        result.end = HURT_NOT_ACTIVE;
        return result;
    }
    if (state >= HIT_LIMIT) {
        result.end = HURT_STATE_PAST;
        return result;
    }
    hit = RD8(OBJ->hit_taken[slot]);
    result.value = hit;
    if (hit == 0) {
        result.end = HURT_NOT_HIT;
        return result;
    }
    if (hit >= HIT_LIMIT) {
        result.end = HURT_HIT_PAST;
        return result;
    }
    WR8(OBJ->state[slot], HIT_STATE);
    result.end = HURT_ENTERED;
    return result;
}

/* A and flags as the hurt check's last compare or load left them. */
void hurt_exit_shim(HurtCheck result)
{
    switch (result.end) {
    case HURT_NOT_ACTIVE:
        cpu.a = 0;
        abi_nz(0);
        break;
    case HURT_NOT_HIT:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);  /* the state's compare against 5 left C clear */
        break;
    case HURT_STATE_PAST:
    case HURT_HIT_PAST:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - HIT_LIMIT));
        abi_flag(P_C, true);
        break;
    case HURT_ENTERED:
        cpu.a = HIT_STATE;
        abi_nz(HIT_STATE);
        abi_flag(P_C, false);
        break;
    }
}

void abi_EnterHurtStateOnHit(void)
{
    hurt_exit_shim(EnterHurtStateOnHit(cpu.x));
}

/*
 * @routine 0519:74E SetBlinkFlash
 * @run 70:A74E
 * WHAT IT DOES: Sets an object's flash flag (the palette flash its sprite shows) to $86 on
 * alternate beats of the wave counter once its timer has reached $18, and to 0 otherwise.
 * HOW: Clears the flag first, then reads the timer and, if it is $18 or more and bit 1 of the
 * wave counter is clear, writes $86.
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare, AND or load left them (see the
 * wrapper); X and Y kept.
 */
BlinkCheck SetBlinkFlash(uint8_t slot)
{
    BlinkCheck result;
    uint8_t timer, wave;

    WR8(OBJ->flash_flags[slot], 0);
    timer = RD8(OBJ->timer[slot]);
    result.value = timer;
    if (timer < BLINK_LIMIT) {
        result.end = BLINK_TIMER_LOW;
        return result;
    }
    wave = RD8(ZP->wave_counter);
    result.value = (uint8_t)(wave & BLINK_WAVE_MASK);
    if (result.value != 0) {
        result.end = BLINK_WAVE_BIT;
        return result;
    }
    WR8(OBJ->flash_flags[slot], BLINK_FLAG);
    result.end = BLINK_SET;
    return result;
}

/* A and flags as the blink check's last compare, AND or load left them. */
void blink_exit_shim(BlinkCheck result)
{
    switch (result.end) {
    case BLINK_TIMER_LOW:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - BLINK_LIMIT));
        abi_flag(P_C, false);
        break;
    case BLINK_WAVE_BIT:
        cpu.a = result.value;
        abi_nz(result.value);
        abi_flag(P_C, true);
        break;
    case BLINK_SET:
        cpu.a = BLINK_FLAG;
        abi_nz(BLINK_FLAG);
        abi_flag(P_C, true);
        break;
    }
}

void abi_SetBlinkFlash(void)
{
    blink_exit_shim(SetBlinkFlash(cpu.x));
}

/*
 * @routine 0519:482 InitObjectFromRecord
 * @run 70:A482
 * WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
 * OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
 * its timer to $60.
 * HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
 * WAITS: no.
 * Entry: X object. Exit: A $60, N and Z from it, C as LoadObjectRecord left it; Y 11; X kept. D clear.
 */
bool InitObjectFromRecord(uint8_t slot)
{
    uint8_t shared, nz;
    bool carry;

    jsr_push(0xA485);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    shared = RD8(OBJ->f2fe8[0]);
    WR8(OBJ->f2fe8[slot], shared);
    WR8(OBJ->hitbox_half_w[slot], 4);
    WR8(OBJ->hitbox_half_h[slot], 4);
    WR8(OBJ->f3510[slot], 4);
    WR8(OBJ->life_timer[slot], 0x60);
    return carry;
}

void abi_InitObjectFromRecord(void)
{
    bool carry = InitObjectFromRecord(cpu.x);

    cpu.a = 0x60;
    cpu.y = 11;
    abi_nz(0x60);
    abi_flag(P_C, carry);
}

/* The carry out of the 24-bit position add ApplyVelocity ended with. */
static bool axis_carry(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (move.before + added) > 0xFFFFFF;
}

/*
 * The second half of FallAndBounce (from $A4D6), which 0519:519 also branches into: the object's X
 * speed is the cosine of its angle, its Y speed $0080, and its angle turns on by $0280. Returns the
 * add of the angle's high byte, which is what the caller's exit shows; Y is left 0 by the scale.
 */
Sum BounceFromAngle(uint8_t slot)
{
    uint8_t angle, product_hi, product_mid, angle_low, angle_high;
    Sum sum;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xA4E4);
    ScaleByCosine(angle);
    jsr_pop();
    product_hi = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], product_hi);
    product_mid = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], product_mid);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0x80);
    angle_low = RD8(OBJ->angle_lo[slot]);
    sum = add_bytes(angle_low, 0x80, false);
    WR8(OBJ->angle_lo[slot], sum.value);
    angle_high = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(angle_high, 0x02, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    return sum;
}

/*
 * @routine 0519:4BD FallAndBounce
 * @run 70:A4BD
 * WHAT IT DOES: Moves an object by its velocity and asks the ground under it; on floor the
 * object enters its next state. Unless the ground is deadly, the object's X speed is set
 * from its angle (the full cosine) and its Y speed to $80, and its angle is turned on by
 * $0280.
 * HOW: Calls ApplyVelocity and ProbeTerrain (floor, height 0, with the carry ApplyVelocity
 * left), then ScaleByCosine with a multiplier of 256; the angle is the two bytes OBJ_35C0 (low)
 * and OBJ_3568 (high).
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: A and flags from the last AND or add (see the
 * wrapper); X kept; Y as ScaleByCosine left it. D clear.
 */
FallResult FallAndBounce(uint8_t slot, uint8_t column)
{
    FallResult result;
    AxisMove move;
    uint8_t deadly;

    jsr_push(0xA4C0);
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(ZP->product.mid, 0);
    jsr_push(0xA4C9);
    result.probe = ProbeTerrain(1, slot, column, axis_carry(move));
    jsr_pop();
    if ((result.probe.answer & 0x01) != 0) {
        INC8(OBJ->state[slot]);
    }
    deadly = RD8(ZP->product.lo);
    deadly = (uint8_t)(deadly & 0x10);
    result.deadly = deadly;
    if (deadly != 0) {
        result.bounced = false;
        return result;
    }
    result.angle = BounceFromAngle(slot);
    result.bounced = true;
    return result;
}

void abi_FallAndBounce(void)
{
    FallResult result = FallAndBounce(cpu.x, cpu.y);

    probe_exit_shim(result.probe);
    if (!result.bounced) {
        cpu.a = result.deadly;
        abi_nz(result.deadly);
    } else {
        cpu.a = result.angle.value;
        abi_nz(result.angle.value);
        abi_flag(P_C, result.angle.carry);
        abi_flag(P_V, result.angle.overflow);
        cpu.y = 0;
    }
}

/*
 * @routine 051A:055 BitMaskFromIndex
 * @run 70:A855
 * WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more).
 * HOW: Shifts a 1 left, counting Y down to 0.
 * WAITS: no.
 * Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear (the last compare).
 */
uint8_t BitMaskFromIndex(uint8_t count)
{
    uint8_t mask = 1;

    while (count != 0) {
        mask = (uint8_t)(mask << 1);
        count--;
    }
    return mask;
}

void abi_BitMaskFromIndex(void)
{
    cpu.a = BitMaskFromIndex(cpu.y);
    cpu.y = 0;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 0519:329 StepReleaseOnHit
 * @run 70:A329
 * WHAT IT DOES: An object that waits to be hit. It starts from its record; each frame after, it
 * steps its animation list (the list at $A365); once it has taken a hit of 5 or more it records
 * the event, takes its next kind and a timer of $10, goes back to state 0, and starts an object of
 * kind $9B at its place in the first free slot from $50 to $53.
 * Untraced: the arm where no slot from $50 to $53 is free (FindFreeSlotFrom50 ran in no script that far).
 * HOW: LoadObjectRecord and a frame of 1 on state 0; else StepObjectFrames with the list's pointer
 * put in zTemp10-11, then the hit byte compared with 5. The spawn is a tail jump into
 * StartObjectAtPlace. The list is read from the disc's table at $A365 by the step (bank 70).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A 1, C from
 * the record; stepped: A the hit byte, C clear and N, Z from hit less 5; no slot: as
 * FindFreeSlotFrom50; spawned: as StartObjectAtPlace with the slot found in Y).
 */
ReleaseResult StepReleaseOnHit(uint8_t slot, uint8_t y)
{
    ReleaseResult result = { RELEASE_STEPPED, false, y, 0, 0, 0, 0 };
    uint8_t state, hit, kind, found_slot;
    bool found;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        uint8_t nz;

        jsr_push(0xA331);
        LoadObjectRecord(slot, &nz, &result.carry);
        jsr_pop();
        WR8(OBJ->frame[slot], 1);
        result.end = RELEASE_STARTED;
        return result;
    }
    WR8(ZP->temp11, 0xA3);
    WR8(ZP->temp10, 0x65);
    jsr_push(0xA342);
    StepObjectFrames(slot, &result.y);
    jsr_pop();
    hit = RD8(OBJ->hit_taken[slot]);
    result.hit = hit;
    if (hit < 5) {
        return result;
    }
    {
        uint8_t stored, event, byte;

        jsr_push(0xA34C);
        RecordObjectEvent(slot, &stored, &event, &byte);
        jsr_pop();
    }
    kind = RD8(OBJ->next_kind[slot]);
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    jsr_push(0xA35D);
    found_slot = FindFreeSlotFrom50(&result.last_type, &found);
    jsr_pop();
    result.slot = found_slot;
    if (!found) {
        result.end = RELEASE_NO_SLOT;
        return result;
    }
    result.placed = StartObjectAtPlace(0x9B, slot, found_slot);
    result.end = RELEASE_SPAWNED;
    return result;
}

void abi_StepReleaseOnHit(void)
{
    ReleaseResult result = StepReleaseOnHit(cpu.x, cpu.y);

    switch (result.end) {
    case RELEASE_STARTED:
        cpu.y = 11;
        cpu.a = 1;
        abi_nz(1);
        abi_flag(P_C, result.carry);
        break;
    case RELEASE_STEPPED:
        cpu.y = result.y;
        cpu.a = result.hit;
        abi_nz((uint8_t)(result.hit - 5));
        abi_flag(P_C, false);
        break;
    case RELEASE_NO_SLOT:
        find_slot_exit_shim(result.slot, result.last_type, false);
        break;
    case RELEASE_SPAWNED:
        find_slot_exit_shim(result.slot, result.last_type, true);
        copy_place_exit_shim(result.placed);
        break;
    }
}

/*
 * @routine 0519:36D StepFallingItem
 * @run 70:A36D
 * WHAT IT DOES: A small thing that falls and then goes away. It starts from its record with its
 * speed set from a two-byte row of the table at $A3B1 picked by its kind less $9B; each frame after,
 * it moves down by its Y speed, clears its flash flag and steps the animation list the second table
 * ($A3B9, by the same index) names; when the list ends the object is cleared.
 * HOW: ApplyVelocityY, then StepObjectFrames with zTemp10-11 loaded from the table (bank 70); the
 * clear is a tail jump into ClearObject, which leaves every register as it was.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A the speed's low
 * byte, Y the table index, C bit 7 of kind less $9B; else as StepObjectFrames).
 */
FallingItem StepFallingItem(uint8_t slot, uint8_t y)
{
    FallingItem result = { false, FRAME_HELD, y, 0, false, false };
    uint8_t state, kind, diff, index, byte;
    Sum sum;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        uint8_t nz;
        bool carry;

        jsr_push(0xA375);
        LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        kind = RD8(OBJ->kind[slot]);
        sum = sub_bytes(kind, 0x9B, true);
        diff = sum.value;
        index = (uint8_t)(diff << 1);
        result.carry = (diff & 0x80) != 0;
        result.overflow = sum.overflow;
        byte = rd8((uint16_t)(0xA3B2 + index));
        WR8(OBJ->vy_hi[slot], byte);
        byte = rd8((uint16_t)(0xA3B1 + index));
        WR8(OBJ->vy_lo[slot], byte);
        result.started = true;
        result.y = index;
        result.last = byte;
        return result;
    }
    jsr_push(0xA393);
    ApplyVelocityY(slot);
    jsr_pop();
    WR8(OBJ->flash_flags[slot], 0);
    kind = RD8(OBJ->kind[slot]);
    sum = sub_bytes(kind, 0x9B, true);
    index = (uint8_t)(sum.value << 1);
    result.y = index;
    result.overflow = sum.overflow;
    byte = rd8((uint16_t)(0xA3B9 + index));
    WR8(ZP->temp10, byte);
    byte = rd8((uint16_t)(0xA3BA + index));
    WR8(ZP->temp11, byte);
    jsr_push(0xA3AB);
    result.step = StepObjectFrames(slot, &result.y);
    jsr_pop();
    if (result.step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return result;
}

void abi_StepFallingItem(void)
{
    FallingItem result = StepFallingItem(cpu.x, cpu.y);

    if (result.started) {
        cpu.y = result.y;
        cpu.a = result.last;
        abi_nz(result.last);
        abi_flag(P_C, result.carry);
    } else {
        step_frames_exit_shim(result.step, result.y);
    }
    abi_flag(P_V, result.overflow);
}

/*
 * @routine 0519:5EF ChangeKindFromTable
 * @run 70:A5EF
 * WHAT IT DOES: An object changes into another kind. Sound $0B plays, the player's byte at $3BD6
 * becomes 1, and zField93 takes a byte of the table at $A650 picked by the kind less $80 (its old
 * value is the index into the table at $A668 that names the new kind). A zero there clears the object.
 * Otherwise the object takes the new kind and state 0 with a timer of $60, is put at the player's place,
 * moved 8 up with a borrow, and thrown up at a speed of $FEC0 and sideways by a table pair picked by the
 * player's facing.
 * HOW: The old zField93 is kept on the stack across the table read; the place copy is made with X
 * and Y swapped (source slot 0, destination the object); the clear is a tail jump into ClearObject.
 * Tables are read from the disc (bank 70) by their addresses until the data layer names them; $3BD6
 * has no name yet (a row is requested).
 * WAITS: no.
 * Entry: X object, Y pushed by the sound driver and pulled back. Exit: see the wrapper (cleared: A 0, Y the old zField93, C and V from the kind
 * less $80; changed: A the sideways speed's low byte, Y the facing's table index, C the facing's top
 * bit, V from the borrow into the Y position's high byte).
 */
KindChange ChangeKindFromTable(uint8_t slot, uint8_t y)
{
    KindChange result;
    uint8_t old, kind, index, next, low, high, facing, byte;
    Sum sum, borrow;

    jsr_push(0xA5F4);
    SoundCommand(0x0B, slot, y);
    jsr_pop();
    old = RD8(ZP->field93);
    push8(old);
    wr8(0x3BD6, 1);
    kind = RD8(OBJ->kind[slot]);
    sum = sub_bytes(kind, 0x80, true);
    result.table = sum;
    byte = rd8((uint16_t)(0xA650 + sum.value));
    WR8(ZP->field93, byte);
    index = pull8();
    result.y = index;
    next = rd8((uint16_t)(0xA668 + index));
    if (next == 0) {
        result.cleared = true;
        ClearObject(slot);
        return result;
    }
    result.cleared = false;
    WR8(OBJ->kind[slot], next);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->timer[slot], 0x60);
    jsr_push(0xA622);
    CopyObjectPlace(0, slot);
    jsr_pop();
    low = RD8(OBJ->y_lo[slot]);
    sum = sub_bytes(low, 0x08, true);
    WR8(OBJ->y_lo[slot], sum.value);
    high = RD8(OBJ->y_hi[slot]);
    borrow = sub_bytes(high, 0x00, sum.carry);
    WR8(OBJ->y_hi[slot], borrow.value);
    result.borrow = borrow;
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0xC0);
    facing = RD8(OBJ->facing_left[0]);
    result.carry = (facing & 0x80) != 0;
    index = (uint8_t)(facing << 1);
    result.y = index;
    byte = rd8((uint16_t)(0xA672 + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xA671 + index));
    WR8(OBJ->vx_lo[slot], byte);
    result.last = byte;
    return result;
}

void abi_ChangeKindFromTable(void)
{
    KindChange result = ChangeKindFromTable(cpu.x, cpu.y);

    cpu.y = result.y;
    if (result.cleared) {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, result.table.carry);
        abi_flag(P_V, result.table.overflow);
    } else {
        cpu.a = result.last;
        abi_nz(result.last);
        abi_flag(P_C, result.carry);
        abi_flag(P_V, result.borrow.overflow);
    }
}

/*
 * @routine 0519:72C AnimateKind88
 * @run 70:A72C
 * WHAT IT DOES: For an object of kind $88 only: steps its animation list (at $A745) and keeps the
 * frame it shows in OBJ_3510. Any other kind is left alone.
 * HOW: Compares the kind with $88; then StepObjectFrames with the list's pointer in zTemp10-11 and a
 * copy of the frame byte.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: other kind: A the kind, flags of its compare
 * with $88; kind $88: A the frame, N and Z from it, C as the step left it (changed), Y as it left it.
 */
Kind88Step AnimateKind88(uint8_t slot, uint8_t y)
{
    Kind88Step result = { false, 0, y, FRAME_HELD };
    uint8_t kind, frame;

    kind = RD8(OBJ->kind[slot]);
    result.value = kind;
    if (kind != 0x88) {
        return result;
    }
    WR8(ZP->temp11, 0xA7);
    WR8(ZP->temp10, 0x45);
    jsr_push(0xA73E);
    result.step = StepObjectFrames(slot, &result.y);
    jsr_pop();
    frame = RD8(OBJ->frame[slot]);
    WR8(OBJ->f3510[slot], frame);
    result.value = frame;
    result.stepped = true;
    return result;
}

/* A the kind compared or the frame copied, with the flags of that compare or load; Y and C as the step left them. */
void kind88_exit_shim(Kind88Step result)
{
    cpu.a = result.value;
    if (!result.stepped) {
        abi_nz((uint8_t)(result.value - 0x88));
        abi_flag(P_C, result.value >= 0x88);
        return;
    }
    cpu.y = result.y;
    abi_nz(result.value);
    abi_flag(P_C, result.step == FRAME_CHANGED);
}

void abi_AnimateKind88(void)
{
    kind88_exit_shim(AnimateKind88(cpu.x, cpu.y));
}

/*
 * @routine 0519:764 StartKindFromCharacter
 * @run 70:A764
 * WHAT IT DOES: Starts an object whose kind is a pair: a kind $93 with zField90 set becomes kind $84,
 * and an object whose kind is already the one the table at $A668 gives for zField93 becomes $84; then
 * the record is loaded, slot 0's byte of the OBJ_2FE8 column is copied into the object's, and OBJ_3510
 * is set from the table at $A79E, row of two bytes by the kind less $84, the second by which character
 * plays.
 * HOW: Compares and, on equality, stores kind $84; LoadObjectRecord; the row by an add with the
 * character.
 * Untraced: the two arms that store $84 (the bytes at A76B and A77E) ran in no script.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, N and Z from it, C and V from the add of the character,
 * Y the table index.
 */
KindStart StartKindFromCharacter(uint8_t slot)
{
    KindStart result;
    uint8_t kind, flag, field93, wanted, shared, character, index, byte, nz;
    bool carry;
    Sum diff, sum;

    kind = RD8(OBJ->kind[slot]);
    if (kind == 0x93) {
        flag = RD8(ZP->field90);
        if (flag != 0) {
            WR8(OBJ->kind[slot], 0x84);
        }
    }
    field93 = RD8(ZP->field93);
    wanted = rd8((uint16_t)(0xA668 + field93));
    kind = RD8(OBJ->kind[slot]);
    if (wanted == kind) {
        WR8(OBJ->kind[slot], 0x84);
    }
    jsr_push(0xA786);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    shared = RD8(OBJ->f2fe8[0]);
    WR8(OBJ->f2fe8[slot], shared);
    kind = RD8(OBJ->kind[slot]);
    diff = sub_bytes(kind, 0x84, true);
    index = (uint8_t)(diff.value << 1);
    character = RD8(ZP->character);
    sum = add_bytes(index, character, false);
    byte = rd8((uint16_t)(0xA79E + sum.value));
    WR8(OBJ->f3510[slot], byte);
    result.last = byte;
    result.index = sum.value;
    result.sum = sum;
    return result;
}

void abi_StartKindFromCharacter(void)
{
    KindStart result = StartKindFromCharacter(cpu.x);

    cpu.a = result.last;
    cpu.y = result.index;
    abi_nz(result.last);
    abi_flag(P_C, result.sum.carry);
    abi_flag(P_V, result.sum.overflow);
}

/*
 * @routine 0519:49E ClearOnLanding
 * @run 70:A49E
 * WHAT IT DOES: One frame of an object that falls away: while it is not rising (its Y speed's top bit
 * clear) it asks the floor with a probe height of 2 and, if the probe says floor, the object is cleared.
 * Otherwise it takes its frame from OBJ_3510 and moves on one state.
 * HOW: ProbeTerrain with the carry and column the caller left; the clear is a tail jump into
 * ClearObject.
 * Untraced: the arm that clears (the jump at A4B0) ran in no script.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed to the lookup. Exit: cleared: as the probe left it,
 * A 1; else A the frame, N and Z from the new state, C and V as the probe or entry left them.
 */
LandClear ClearOnLanding(uint8_t slot, uint8_t column, bool carry_in)
{
    LandClear result = { false, false };
    uint8_t speed, frame, landed;

    speed = RD8(OBJ->vy_hi[slot]);
    if ((speed & 0x80) == 0) {
        WR8(ZP->product.mid, 2);
        jsr_push(0xA4AC);
        result.probe = ProbeTerrain(1, slot, column, carry_in);
        jsr_pop();
        result.probed = true;
        landed = (uint8_t)(result.probe.answer & 0x01);
        if (landed != 0) {
            ClearObject(slot);
            result.cleared = true;
            return result;
        }
    }
    frame = RD8(OBJ->f3510[slot]);
    WR8(OBJ->frame[slot], frame);
    result.frame = frame;
    result.state = INC8(OBJ->state[slot]);
    return result;
}

void abi_ClearOnLanding(void)
{
    LandClear result = ClearOnLanding(cpu.x, cpu.y, (cpu.p & P_C) != 0);

    if (result.probed) {
        probe_exit_shim(result.probe);
    }
    if (result.cleared) {
        cpu.a = 1;
        abi_nz(1);
        return;
    }
    cpu.a = result.frame;
    abi_nz(result.state);
}

/* A fall's exit: the probe's registers when it was asked, then what the routine's own instructions changed. */
void floor_fall_exit_shim(FloorFall fall)
{
    if (fall.probed) {
        probe_exit_shim(fall.probe);
    }
    abi_ended(fall.end);
}

/*
 * @routine 0519:519 FallUntilFloor
 * @run 70:A519
 * WHAT IT DOES: One frame of an object that drops until it finds floor. Only while the object is on
 * the first screen down (its Y high byte 0) it asks the floor with a probe height of 0; deadly ground
 * (bit 4) ends the frame by moving it down with ApplyVelocityY; when the probe does not report floor
 * (bit 0) the object is moved on both axes and bounces off by its angle (the bounce the second half of
 * FallAndBounce makes).
 * HOW: ProbeTerrain with the carry and column the caller left. The deadly arm is a tail jump into
 * ApplyVelocityY; the not-floor arm calls ApplyVelocity and branches into FallAndBounce at $A4D6.
 * Untraced: the deadly arm and the not-floor arm (A52B, A534) ran in no script.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed to the lookup. Exit: see the wrapper.
 */
FloorFall FallUntilFloor(uint8_t slot, uint8_t column, bool carry_in)
{
    FloorFall fall = { false, { 0 }, { 0 } };
    uint8_t y_high, masked, floor;
    AxisMove move;
    Sum bounce;

    y_high = RD8(OBJ->y_hi[slot]);
    if (y_high != 0) {
        fall.end = ended_load(y_high);
        return fall;
    }
    WR8(ZP->product.mid, 0);
    jsr_push(0xA527);
    fall.probe = ProbeTerrain(1, slot, column, carry_in);
    jsr_pop();
    fall.probed = true;
    masked = (uint8_t)(fall.probe.answer & 0x10);
    fall.end = ended_load(masked);
    if (masked != 0) {
        move = ApplyVelocityY(slot);
        fall.end = ended_of_axis_move(move);
        return fall;
    }
    floor = RD8(ZP->product.lo);
    masked = (uint8_t)(floor & 0x01);
    fall.end = ended_load(masked);
    if (masked != 0) {
        return fall;
    }
    jsr_push(0xA537);
    move = ApplyVelocity(slot);
    jsr_pop();
    bounce = BounceFromAngle(slot);
    fall.end = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(bounce.value), bounce.carry),
                                                bounce.overflow), 0);
    (void)move;
    return fall;
}

void abi_FallUntilFloor(void)
{
    floor_fall_exit_shim(FallUntilFloor(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0519:508 FallThenWait
 * @run 70:A508
 * WHAT IT DOES: A falling object's wait: it falls as FallUntilFloor does, and counts its life timer
 * down; at zero the timer becomes $40 and the object moves on one state.
 * HOW: FallUntilFloor, then DEC of the timer.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: see the wrapper (the fall's registers,
 * then the timer's N and Z; at zero A $40 and N, Z from the new state).
 */
FloorFall FallThenWait(uint8_t slot, uint8_t column, bool carry_in)
{
    FloorFall fall;
    uint8_t left, state;

    jsr_push(0xA50B);
    fall = FallUntilFloor(slot, column, carry_in);
    jsr_pop();
    left = DEC8(OBJ->life_timer[slot]);
    if (left != 0) {
        fall.end = ended_then(fall.end, ended_nz(left));
        return fall;
    }
    WR8(OBJ->life_timer[slot], 0x40);
    state = INC8(OBJ->state[slot]);
    fall.end = ended_then(fall.end, ended_with_a(ended_nz(state), 0x40));
    return fall;
}

void abi_FallThenWait(void)
{
    floor_fall_exit_shim(FallThenWait(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0519:53A FallThenClear
 * @run 70:A53A
 * WHAT IT DOES: A falling object that goes away: it falls as FallUntilFloor does and counts its life
 * timer down; at zero the object is cleared. Until then its frame is 0, and it shows the frame in
 * OBJ_3510 on the beats where bit 1 of the timer is set (a flicker).
 * HOW: FallUntilFloor, DEC of the timer; the clear is a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: see the wrapper.
 */
FloorFall FallThenClear(uint8_t slot, uint8_t column, bool carry_in)
{
    FloorFall fall;
    uint8_t left, timer, bit, frame;

    jsr_push(0xA53D);
    fall = FallUntilFloor(slot, column, carry_in);
    jsr_pop();
    left = DEC8(OBJ->life_timer[slot]);
    if (left == 0) {
        ClearObject(slot);
        fall.end = ended_then(fall.end, ended_nz(left));
        return fall;
    }
    WR8(OBJ->frame[slot], 0);
    timer = RD8(OBJ->life_timer[slot]);
    bit = (uint8_t)(timer & 0x02);
    fall.end = ended_then(fall.end, ended_load(bit));
    if (bit == 0) {
        return fall;
    }
    frame = RD8(OBJ->f3510[slot]);
    WR8(OBJ->frame[slot], frame);
    fall.end = ended_then(fall.end, ended_load(frame));
    return fall;
}

void abi_FallThenClear(void)
{
    floor_fall_exit_shim(FallThenClear(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/* The jump a table handler ends with: the registers it leaves, then the code run as the original. */
void jump_shim(Ended end, uint16_t target)
{
    abi_ended(end);
    call_original(target);
}

/*
 * @routine 0519:556 JumpByKindTable
 * @run 70:A556
 * WHAT IT DOES: Goes to the code a table at $A56B names for the object's kind less $83 (the handler
 * of that kind's state).
 * HOW: Reads the table's two bytes into zTemp00-01 and jumps through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the table's high byte, Y the index, N and Z from A,
 * C and V from the kind's subtract and shift.
 */
ObjArm JumpByKindTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t kind, low;

    kind = RD8(OBJ->kind[slot]);
    r.sum = sub_bytes(kind, 0x83, true);
    r.first = (uint8_t)(r.sum.value << 1);
    low = rd8((uint16_t)(0xA56B + r.first));
    WR8(ZP->temp00, low);
    r.second = rd8((uint16_t)(0xA56C + r.first));
    WR8(ZP->temp01, r.second);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_kind_table_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_with_y(ended_load(r.second), r.first);
    jump.end = ended_with_carry(ended_with_overflow(jump.end, r.sum.overflow), (r.sum.value & 0x80) != 0);
    return jump;
}

void abi_JumpByKindTable(void)
{
    Jump jump = jump_by_kind_table_exit_shim(JumpByKindTable(cpu.x));

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0519:703 RunObjectState
 * @run 70:A703
 * WHAT IT DOES: One frame of a common object: animates kind $88, takes a pending hit, blinks, then
 * runs the handler of the object's state from the table at $A71E.
 * HOW: AnimateKind88, EnterHurtStateOnHit, SetBlinkFlash, then the state doubled indexes the table
 * (read into zTemp00-01 and jumped through).
 * WAITS: no.
 * Entry: X object, Y kept unless AnimateKind88 sets it. Exit: as the handler leaves it; at the jump A
 * is the table's high byte, Y the state doubled, N and Z from A, C and V as SetBlinkFlash and the
 * earlier calls left them.
 */
StateRun RunObjectState(uint8_t slot, uint8_t y)
{
    StateRun run;
    uint8_t state, index, low, high;

    jsr_push(0xA706);
    run.kind88 = AnimateKind88(slot, y);
    jsr_pop();
    jsr_push(0xA709);
    run.hurt = EnterHurtStateOnHit(slot);
    jsr_pop();
    jsr_push(0xA70C);
    run.blink = SetBlinkFlash(slot);
    jsr_pop();
    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xA71E + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xA71F + index));
    WR8(ZP->temp01, high);
    run.end = ended_with_y(ended_load(high), index);
    run.end = ended_with_carry(run.end, (state & 0x80) != 0);
    run.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return run;
}

void abi_RunObjectState(void)
{
    StateRun run = RunObjectState(cpu.x, cpu.y);

    kind88_exit_shim(run.kind88);
    hurt_exit_shim(run.hurt);
    blink_exit_shim(run.blink);
    jump_shim(run.end, run.target);
}

/*
 * @routine 051A:180 DropOffScreen
 * @run 70:A980
 * WHAT IT DOES: A thrown object's frame: when it is below the first screen (Y high byte above 0 and
 * not negative) it is cleared; otherwise gravity takes its speed up and it moves by its velocity.
 * HOW: The clear and the move are tail jumps (ClearObject, ApplyVelocity); gravity is
 * ApplyGravityUnlimited.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the Y high byte, N and Z from it; else as ApplyVelocity.
 */
ThrowStep DropOffScreen(uint8_t slot)
{
    ThrowStep step = { false, { 0 }, { 0 } };
    uint8_t y_high;
    AxisMove move;

    y_high = RD8(OBJ->y_hi[slot]);
    if (y_high != 0 && (y_high & 0x80) == 0) {
        ClearObject(slot);
        step.end = ended_load(y_high);
        return step;
    }
    jsr_push(0xA98D);
    step.gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    step.gravity_run = true;
    move = ApplyVelocity(slot);
    step.end = ended_of_axis_move(move);
    return step;
}

void throw_step_exit_shim(ThrowStep step)
{
    if (step.gravity_run) {
        gravity_exit_shim(step.gravity);
    }
    abi_ended(step.end);
}

void abi_DropOffScreen(void)
{
    throw_step_exit_shim(DropOffScreen(cpu.x));
}

/*
 * @routine 051A:12E StartThrownItem
 * @run 70:A92E
 * @stop A95E
 * WHAT IT DOES: A thrown item: from state 0 it loads its record, takes its frame from a table by its
 * life timer and its speeds from a four-byte row of the table at $A95E picked by OBJ_3510; in any other
 * state it goes on as DropOffScreen.
 * HOW: Tables read from the disc (bank 70) by address; the other states are a tail branch into
 * DropOffScreen.
 * WAITS: no.
 * Entry: X object. Exit: started: A the last speed byte, Y the row's start, C bit 6 of OBJ_3510, N and
 * Z from A; else as DropOffScreen.
 */
ThrowStep StartThrownItem(uint8_t slot)
{
    ThrowStep step = { false, { 0 }, { 0 } };
    uint8_t state, timer, frame, row, index, byte, last, nz;
    bool carry;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return DropOffScreen(slot);
    }
    jsr_push(0xA936);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    timer = RD8(OBJ->life_timer[slot]);
    frame = rd8((uint16_t)(0xA97E + timer));
    WR8(OBJ->frame[slot], frame);
    row = RD8(OBJ->f3510[slot]);
    index = (uint8_t)(row << 2);
    byte = rd8((uint16_t)(0xA95F + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xA95E + index));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xA961 + index));
    WR8(OBJ->vy_hi[slot], byte);
    last = rd8((uint16_t)(0xA960 + index));
    WR8(OBJ->vy_lo[slot], last);
    step.end = ended_with_carry(ended_with_y(ended_load(last), index), (row & 0x40) != 0);
    return step;
}

void abi_StartThrownItem(void)
{
    throw_step_exit_shim(StartThrownItem(cpu.x));
}

/*
 * @routine 051A:1EB FallThenState
 * @run 70:A9EB
 * WHAT IT DOES: Moves an object down by its Y speed; OBJ_3510 counts down and at zero becomes $10
 * while the object moves on one state.
 * HOW: ApplyVelocityY, then DEC of OBJ_3510.
 * WAITS: no.
 * Entry: X object. Exit: the move's registers, then N and Z of the count; at zero A $10 and N, Z of
 * the new state.
 */
ThrowStep FallThenState(uint8_t slot)
{
    ThrowStep step = { false, { 0 }, { 0 } };
    AxisMove move;
    uint8_t left, state;

    jsr_push(0xA9EE);
    move = ApplyVelocityY(slot);
    jsr_pop();
    step.end = ended_of_axis_move(move);
    left = DEC8(OBJ->f3510[slot]);
    if (left != 0) {
        step.end = ended_then(step.end, ended_nz(left));
        return step;
    }
    WR8(OBJ->f3510[slot], 0x10);
    state = INC8(OBJ->state[slot]);
    step.end = ended_then(step.end, ended_with_a(ended_nz(state), 0x10));
    return step;
}

void abi_FallThenState(void)
{
    throw_step_exit_shim(FallThenState(cpu.x));
}

/*
 * @routine 051A:1FC ClearWhenCountEnds
 * @run 70:A9FC
 * WHAT IT DOES: OBJ_3510 counts down; at zero the object is cleared.
 * HOW: DEC; the clear is a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: N and Z of the count; A, Y and the rest kept.
 */
uint8_t ClearWhenCountEnds(uint8_t slot)
{
    uint8_t left;

    left = DEC8(OBJ->f3510[slot]);
    if (left == 0) {
        ClearObject(slot);
    }
    return left;
}

void abi_ClearWhenCountEnds(void)
{
    abi_nz(ClearWhenCountEnds(cpu.x));
}

/*
 * The body of the kind dispatch every overlay carries (its copies differ only in the address of the
 * table): an object with a kind goes to the code the table names for its kind less one; kind 0 does
 * nothing.
 */
static ObjArm dispatch_by_kind(uint8_t slot, uint16_t table)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.first = RD8(OBJ->kind[slot]);
    r.arm = DK_NONE;
    if (r.first == 0) {
        return r;
    }
    r.second = (uint8_t)(r.first - 1);
    r.third = (uint8_t)(r.second << 1);
    low = rd8((uint16_t)(table + r.third));
    WR8(ZP->temp00, low);
    r.fourth = rd8((uint16_t)(table + 1 + r.third));
    WR8(ZP->temp01, r.fourth);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    r.arm = DK_JUMP;
    return r;
}

/* The jump of a dispatch: the registers it leaves, and the code if there is one. */
void dispatch_exit_shim(ObjArm r)
{
    Ended end;

    if (r.arm == DK_NONE) {
        abi_ended(ended_load(0));
        return;
    }
    end = ended_with_carry(ended_with_y(ended_load(r.fourth), r.third), (r.second & 0x80) != 0);
    jump_shim(end, r.target);
}

/*
 * @routine 0259:313 DispatchByKind0259
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AB3A holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0259's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind0259(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAB3A);
}

void abi_DispatchByKind0259(void)
{
    dispatch_exit_shim(DispatchByKind0259(cpu.x));
}

/*
 * @routine 07D9:313 DispatchByKind07D9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AA72 holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 07D9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind07D9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAA72);
}

void abi_DispatchByKind07D9(void)
{
    dispatch_exit_shim(DispatchByKind07D9(cpu.x));
}

/*
 * @routine 0889:313 DispatchByKind0889
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AA35 holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0889's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind0889(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAA35);
}

void abi_DispatchByKind0889(void)
{
    dispatch_exit_shim(DispatchByKind0889(cpu.x));
}

/*
 * @routine 0A99:313 DispatchByKind0A99
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AB77 holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0A99's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind0A99(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAB77);
}

void abi_DispatchByKind0A99(void)
{
    dispatch_exit_shim(DispatchByKind0A99(cpu.x));
}

/*
 * @routine 0CA9:313 DispatchByKind0CA9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AB9D holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0CA9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind0CA9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAB9D);
}

void abi_DispatchByKind0CA9(void)
{
    dispatch_exit_shim(DispatchByKind0CA9(cpu.x));
}

/*
 * @routine 0E09:313 DispatchByKind0E09
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AAAA holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0E09's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind0E09(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAAAA);
}

void abi_DispatchByKind0E09(void)
{
    dispatch_exit_shim(DispatchByKind0E09(cpu.x));
}

/*
 * @routine 10C9:313 DispatchByKind10C9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AB4A holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 10C9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind10C9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAB4A);
}

void abi_DispatchByKind10C9(void)
{
    dispatch_exit_shim(DispatchByKind10C9(cpu.x));
}

/*
 * @routine 12D9:313 DispatchByKind12D9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $ABAF holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 12D9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind12D9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xABAF);
}

void abi_DispatchByKind12D9(void)
{
    dispatch_exit_shim(DispatchByKind12D9(cpu.x));
}

/*
 * @routine 16C9:313 DispatchByKind16C9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AB0D holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 16C9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind16C9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAB0D);
}

void abi_DispatchByKind16C9(void)
{
    dispatch_exit_shim(DispatchByKind16C9(cpu.x));
}

/*
 * @routine 18D9:313 DispatchByKind18D9
 * @run 70:A313
 * WHAT IT DOES: Sends an object to the code its kind names: the table at $AAD5 holds one address per
 * kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 18D9's copy.
 * HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
 * zTemp00-01.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
 * N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
 */
ObjArm DispatchByKind18D9(uint8_t slot)
{
    return dispatch_by_kind(slot, 0xAAD5);
}

void abi_DispatchByKind18D9(void)
{
    dispatch_exit_shim(DispatchByKind18D9(cpu.x));
}

/* A landing wait's exit: the mover's registers when the landing was asked, then what the routine's own instructions left. */
void land_wait_exit_shim(LandWait wait)
{
    if (wait.asked) {
        mover_exit_shim(wait.mover, 0x01);
    }
    abi_ended(wait.end);
}

/*
 * @routine 0519:7BE SettleOnLanding
 * @run 70:A7BE
 * WHAT IT DOES: One frame of a thrown object that stops where it lands: StopOnLanding with a probe height of 0; on landing its life timer is set
 * to $F0 and it enters its next state.
 * HOW: StopOnLanding, then a store and an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: the mover's registers, then A $F0 and N, Z from the new state when landed.
 */
LandWait SettleOnLanding(uint8_t slot, uint8_t column, bool carry_in)
{
    LandWait wait = { true, { 0 }, { 0 } };
    uint8_t state;

    jsr_push(0xA7C3);
    wait.mover = StopOnLanding(0, slot, column, carry_in);
    jsr_pop();
    if (!mover_landed(wait.mover)) {
        return wait;
    }
    WR8(OBJ->life_timer[slot], 0xF0);
    state = INC8(OBJ->state[slot]);
    wait.end = ended_with_a(ended_nz(state), 0xF0);
    return wait;
}

void abi_SettleOnLanding(void)
{
    land_wait_exit_shim(SettleOnLanding(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/* The start two waiting handlers share: on the first screen row the object is asked to land. */
static LandWait land_if_on_row(uint8_t slot, uint8_t column, bool carry_in, uint16_t ret)
{
    LandWait wait = { false, { 0 }, { 0 } };
    uint8_t y_high;

    y_high = RD8(OBJ->y_hi[slot]);
    wait.end = ended_load(y_high);
    if (y_high != 0) {
        return wait;
    }
    wait.end = ended_kept();
    jsr_push(ret);
    wait.mover = StopOnLanding(0, slot, column, carry_in);
    jsr_pop();
    wait.asked = true;
    return wait;
}

/*
 * @routine 0519:7CE LandThenWait
 * @run 70:A7CE
 * WHAT IT DOES: A thrown object on the first screen row is asked to land (StopOnLanding, height 0); its life timer counts down and at zero
 * becomes $50 while the object enters its next state.
 * HOW: StopOnLanding, DEC, INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: the mover's registers when asked, then the timer's N and Z; at zero A $50 and N, Z from the state.
 */
LandWait LandThenWait(uint8_t slot, uint8_t column, bool carry_in)
{
    LandWait wait;
    uint8_t left, state;

    wait = land_if_on_row(slot, column, carry_in, 0xA7D8);
    left = DEC8(OBJ->life_timer[slot]);
    if (left != 0) {
        wait.end = ended_then(wait.end, ended_nz(left));
        return wait;
    }
    WR8(OBJ->life_timer[slot], 0x50);
    state = INC8(OBJ->state[slot]);
    wait.end = ended_then(wait.end, ended_with_a(ended_nz(state), 0x50));
    return wait;
}

void abi_LandThenWait(void)
{
    land_wait_exit_shim(LandThenWait(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0519:7E6 LandThenClear
 * @run 70:A7E6
 * WHAT IT DOES: As LandThenWait, but at zero the object is cleared; until then its frame is 0, and it shows OBJ_3510 on the beats where bit 1
 * of the timer is set.
 * HOW: StopOnLanding, DEC; the clear is a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y the probe's column. Exit: the mover's registers when asked, then the last compare, AND or load.
 */
LandWait LandThenClear(uint8_t slot, uint8_t column, bool carry_in)
{
    LandWait wait;
    uint8_t left, timer, bit, frame;

    wait = land_if_on_row(slot, column, carry_in, 0xA7F0);
    left = DEC8(OBJ->life_timer[slot]);
    wait.end = ended_then(wait.end, ended_nz(left));
    if (left == 0) {
        ClearObject(slot);
        return wait;
    }
    WR8(OBJ->frame[slot], 0);
    timer = RD8(OBJ->life_timer[slot]);
    bit = (uint8_t)(timer & 0x02);
    wait.end = ended_then(wait.end, ended_load(bit));
    if (bit == 0) {
        return wait;
    }
    frame = RD8(OBJ->f3510[slot]);
    WR8(OBJ->frame[slot], frame);
    wait.end = ended_then(wait.end, ended_load(frame));
    return wait;
}

void abi_LandThenClear(void)
{
    land_wait_exit_shim(LandThenClear(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/* Two packed decimal digits and a carry, as the HuC6280 adds them in decimal mode. */
static uint8_t add_decimal(uint8_t a, uint8_t b, bool *carry)
{
    unsigned sum = (a & 0x0Fu) + (b & 0x0Fu) + (*carry ? 1u : 0u);

    if (sum >= 0x0A) {
        sum += 0x06;
    }
    sum += (a & 0xF0u) + (b & 0xF0u);
    if (sum >= 0xA0) {
        sum += 0x60;
    }
    *carry = (sum & 0xFF00u) != 0;
    return (uint8_t)sum;
}

/*
 * @routine 0519:58D CollectHearts
 * @run 70:A58D
 * WHAT IT DOES: A heart item taken: plays the sound a table at $A5B0 names for its kind less $83, adds the hearts a table at $A5B2 gives to the player's hearts (zField94, decimal, at
 * most 99), blends the stage rows (BlendStageRowsOfPlayer) and clears the item.
 * HOW: SoundCommand, a decimal add, BlendStageRowsOfPlayer, a tail jump into ClearObject.
 * Untraced: the carry out of the decimal add (the cap of 99, A5A5).
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as BlendStageRowsOfPlayer.
 */
SoundEnd CollectHearts(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t kind, index, command, hearts, add;
    bool carry;
    Sum diff;
    Ended e;

    kind = RD8(OBJ->kind[slot]);
    diff = sub_bytes(kind, 0x83, true);
    index = diff.value;
    command = rd8((uint16_t)(0xA5B0 + index));
    e = ended_then(ended_of_sum(diff), ended_with_y(ended_load(command), index));
    r.before = e;
    r.sounded = true;
    r.command = command;
    jsr_push(0xA59A);
    r.call = SoundCommand(command, slot, index);
    jsr_pop();
    hearts = RD8(ZP->field94);
    add = rd8((uint16_t)(0xA5B2 + index));
    carry = false;
    hearts = add_decimal(hearts, add, &carry);
    WR8(ZP->field94, hearts);
    e = ended_with_carry(ended_load(hearts), carry);
    if (carry) {
        WR8(ZP->field94, 0x99);
        e = ended_then(e, ended_load(0x99));
    }
    r.end = e;
    jsr_push(0xA5AD);
    r.end = ended_then(e, blend_stage_rows_of_player_exit_shim(BlendStageRowsOfPlayer(slot, index)));
    jsr_pop();
    ClearObject(slot);
    (void)y_entry;
    return r;
}

void abi_CollectHearts(void)
{
    abi_sound_end(CollectHearts(cpu.x, cpu.y));
}

/*
 * @routine 0519:5B4 CollectScoreItem
 * @run 70:A5B4
 * WHAT IT DOES: A score item taken: plays sound $0B, adds the points a table at $A5EB gives (decimal, the two bytes zScoreAddLo and zScoreAddHi, at most 9999) for its kind less $85,
 * blends the stage rows, and the item turns into a kind $9F that waits the number of beats its first byte said ($34B8 timer) in state 0.
 * HOW: SoundCommand, a decimal add through the two bytes, BlendStageRowsOfPlayer with Y kept on the stack.
 * Untraced: the carry out of the add (the cap of 9999, A5D4).
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: A $9F, N set, Z clear; C and V as the blend left them.
 */
SoundEnd CollectScoreItem(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t kind, index, low, high, add, previous;
    bool carry;
    Sum diff;
    Ended e;

    r.before = ended_load(0x0B);
    r.sounded = true;
    r.command = 0x0B;
    jsr_push(0xA5B9);
    r.call = SoundCommand(0x0B, slot, y_entry);
    jsr_pop();
    kind = RD8(OBJ->kind[slot]);
    diff = sub_bytes(kind, 0x85, true);
    index = diff.value;
    e = ended_then(ended_of_sum(diff), ended_with_y(ended_kept(), index));
    low = RD8(ZP->score_add.lo);
    WR8(ZP->temp10, low);
    e = ended_then(e, ended_load(low));
    add = rd8((uint16_t)(0xA5EB + index));
    carry = false;
    low = add_decimal(low, add, &carry);
    WR8(ZP->score_add.lo, low);
    e = ended_then(e, ended_with_carry(ended_load(low), carry));
    high = RD8(ZP->score_add.hi);
    high = add_decimal(high, 0, &carry);
    WR8(ZP->score_add.hi, high);
    e = ended_then(e, ended_with_carry(ended_load(high), carry));
    if (carry) {
        WR8(ZP->score_add.lo, 0x99);
        WR8(ZP->score_add.hi, 0x99);
        e = ended_then(e, ended_load(0x99));
    }
    push8(index);
    jsr_push(0xA5DE);
    e = ended_then(e, blend_stage_rows_of_player_exit_shim(BlendStageRowsOfPlayer(slot, index)));
    jsr_pop();
    previous = pull8();
    e = ended_then(e, ended_load(previous));
    WR8(OBJ->life_timer[slot], previous);
    WR8(OBJ->kind[slot], 0x9F);
    WR8(OBJ->state[slot], 0);
    r.end = ended_then(e, ended_load(0x9F));
    return r;
}

void abi_CollectScoreItem(void)
{
    abi_sound_end(CollectScoreItem(cpu.x, cpu.y));
}

/*
 * @routine 051A:1A8 StartScorePickup
 * @run 70:A9A8
 * WHAT IT DOES: Loads the object's record, clears its facing and adds the score a table at $A9E1 gives for its timer (two bytes of BCD); when that carries over
 * the mark the score tests for it the player gets an extra life (GiveExtraLife). The frame comes from a table at $A9DC by the timer, the angle OBJ_3510 is $18 and the Y speed $FE00.
 * HOW: LoadObjectRecord, AddScoreWord, GiveExtraLife, table reads (bank 70).
 * Untraced: the extra life (A9C2).
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
SoundEnd StartScorePickup(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    SoundEnd life;
    uint8_t nz, last, timer, index, changed, y, frame;
    bool carry;
    Ended e;

    jsr_push(0xA9AB);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->facing_left[slot], 0);
    timer = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(timer << 1);
    e = ended_then(e, ended_with_carry(ended_with_y(ended_load(index), index), (timer & 0x80) != 0));
    WR8(ZP->temp00, rd8((uint16_t)(0xA9E1 + index)));
    WR8(ZP->temp01, rd8((uint16_t)(0xA9E2 + index)));
    jsr_push(0xA9C0);
    changed = AddScoreWord();
    jsr_pop();
    e = ended_then(e, ended_with_carry(ended_load(changed), changed != 0));
    if (changed != 0) {
        jsr_push(0xA9C5);
        life = give_extra_life_exit_shim(GiveExtraLife(slot, y_entry));
        jsr_pop();
        r.before = ended_then(e, life.before);
        r.sounded = life.sounded;
        r.command = life.command;
        r.call = life.call;
        e = life.end;
    }
    y = RD8(OBJ->life_timer[slot]);
    e = ended_then(e, ended_with_y(ended_nz(y), y));
    frame = rd8((uint16_t)(0xA9DC + y));
    WR8(OBJ->frame[slot], frame);
    e = ended_then(e, ended_load(frame));
    WR8(OBJ->f3510[slot], 0x18);
    e = ended_then(e, ended_load(0x18));
    WR8(OBJ->vy_hi[slot], 0xFE);
    e = ended_then(e, ended_load(0xFE));
    WR8(OBJ->vy_lo[slot], 0);
    r.end = e;
    return r;
}

void abi_StartScorePickup(void)
{
    abi_sound_end(StartScorePickup(cpu.x, cpu.y));
}
