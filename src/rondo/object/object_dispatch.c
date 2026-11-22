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

void abi_EnterHurtStateOnHit(void)
{
    HurtCheck result = EnterHurtStateOnHit(cpu.x);

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

void abi_SetBlinkFlash(void)
{
    BlinkCheck result = SetBlinkFlash(cpu.x);

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
    uint8_t deadly, angle, product_hi, product_mid, angle_low, angle_high;
    Sum sum;

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
    result.bounced = true;
    result.angle = sum;
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
 * Entry: X object. Exit: see the wrapper (cleared: A 0, Y the old zField93, C and V from the kind
 * less $80; changed: A the sideways speed's low byte, Y the facing's table index, C the facing's top
 * bit, V from the borrow into the Y position's high byte).
 */
KindChange ChangeKindFromTable(uint8_t slot)
{
    KindChange result;
    uint8_t old, kind, index, next, low, high, facing, byte;
    Sum sum, borrow;

    jsr_push(0xA5F4);
    SoundCommand(0x0B, slot, 0);   /* X the object; Y: not worked out yet */
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
    KindChange result = ChangeKindFromTable(cpu.x);

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
