/*
 * Richter's own objects, from his overlay at $C000 (sector $1BD on): the whip and what he throws,
 * placed by him, marked, and made to flash while charged. Each acts on the object in X; slot 0 is
 * Richter himself.
 *
 * A body returns plain values; its wrapper works out what the assembly leaves in the registers
 * and flags.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "richter.h"

enum {
    OBJECT_MARK = 0x10, OBJECT_STATE_QUICK = 6, OBJECT_QUICKEN = 4, OBJECT_FLASH_FIRST = 0x8E,
    OBJECT_FLASH_BIT = 0x80, OBJECT_CHARGE_FROM = 0x0C, OBJECT_FLASH_BASE = 0x0E, PLAYER = 0,
    OBJECT_FALL = 6, OBJECT_FALL_SLOW = 3, OBJECT_ANIMATION_STOPPED = 0xFF, OBJECT_ANIMATION_LOOPS = 0xFE
};

/* A, N, Z, C and V as an add leaves them. */
static void abi_exit_sum(Sum sum)
{
    cpu.a = sum.value;
    abi_nz(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* A the byte loaded last, N and Z from it. */
static void abi_exit_loaded_object(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

/* The caller's return address dropped, as PLA PLA does: A its high byte, N and Z from it. */
static void object_drop_return_shim(void)
{
    cpu.a = rd8((uint16_t)(0x2100 + (uint8_t)(cpu.s + 2)));
    cpu.s = (uint8_t)(cpu.s + 2);
    abi_nz(cpu.a);
}

/* The frames a charged object flashes in place of: one flash frame for each. */
static const uint8_t flash_frames[] = { 0x07, 0x0E, 0x15, 0x1C };

/*
 * @routine 01BD:238 RichterObjectMarkOn
 * @run 6E:C238
 * WHAT IT DOES: Sets bit 4 of an object's OBJ_SPRITE_ATTR.
 * HOW: ORed in.
 * WAITS: no.
 * Entry: X object. Exit: A the new OBJ_SPRITE_ATTR, N and Z from it. X, Y, C and V kept.
 */
static uint8_t object_mark_on_body(uint8_t object)
{
    uint8_t flags = (uint8_t)(rd8((uint16_t)(OBJ_SPRITE_ATTR + object)) | OBJECT_MARK);

    wr8((uint16_t)(OBJ_SPRITE_ATTR + object), flags);
    return flags;
}

uint8_t RichterObjectMarkOn(uint8_t object)
{
    return object_mark_on_body(object);
}

void abi_RichterObjectMarkOn(void)
{
    cpu.a = RichterObjectMarkOn(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 01E1:37C MariaObjectMarkOn
 * @run 6E:C37C
 * WHAT IT DOES: RichterObjectMarkOn in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterObjectMarkOn's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectMarkOn.
 */
uint8_t MariaObjectMarkOn(uint8_t object)
{
    return object_mark_on_body(object);
}

void abi_MariaObjectMarkOn(void)
{
    cpu.a = MariaObjectMarkOn(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 01BD:3EC RichterObjectMarkOff
 * @run 6E:C3EC
 * WHAT IT DOES: Clears bit 4 of an object's OBJ_SPRITE_ATTR.
 * HOW: ANDed out.
 * WAITS: no.
 * Entry: X object. Exit: A the new OBJ_SPRITE_ATTR, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterObjectMarkOff(uint8_t object)
{
    uint8_t flags = (uint8_t)(rd8((uint16_t)(OBJ_SPRITE_ATTR + object)) & ~OBJECT_MARK);

    wr8((uint16_t)(OBJ_SPRITE_ATTR + object), flags);
    return flags;
}

void abi_RichterObjectMarkOff(void)
{
    cpu.a = RichterObjectMarkOff(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 01BD:401 RichterObjectTake2FE8
 * @run 6E:C401
 * WHAT IT DOES: Gives an object Richter's OBJ_SPRITE_FLIP, doubled.
 * HOW: Shifted up a place.
 * WAITS: no.
 * Entry: X object. Exit: A the value, N and Z from it, C Richter's bit 7. X, Y and V kept.
 */
uint8_t RichterObjectTake2FE8(uint8_t object)
{
    uint8_t player = rd8(OBJ_SPRITE_FLIP + PLAYER);

    wr8((uint16_t)(OBJ_SPRITE_FLIP + object), (uint8_t)(player << 1));
    return player;
}

static void take_2fe8_exit_shim(uint8_t player)
{
    cpu.a = (uint8_t)(player << 1);
    abi_nz(cpu.a);
    abi_flag(P_C, (player & 0x80) != 0);
}

void abi_RichterObjectTake2FE8(void)
{
    take_2fe8_exit_shim(RichterObjectTake2FE8(cpu.x));
}

/*
 * @routine 01E1:397 MariaObjectTake2FE8
 * @run 6E:C397
 * WHAT IT DOES: RichterObjectTake2FE8 in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterObjectTake2FE8's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectTake2FE8.
 */
uint8_t MariaObjectTake2FE8(uint8_t object)
{
    return RichterObjectTake2FE8(object);
}

void abi_MariaObjectTake2FE8(void)
{
    take_2fe8_exit_shim(MariaObjectTake2FE8(cpu.x));
}

/*
 * @routine 01E1:387 MariaObjectFromPlayer
 * @run 6E:C387
 * WHAT IT DOES: Gives an object Maria's OBJ_SPRITE_FLIP, doubled, and her facing; the doubled byte is written twice.
 * HOW: The doubling once inline, then on into the copy of RichterObjectFromPlayer at $C38E, which calls
 * MariaObjectTake2FE8 and copies the facing.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectFromPlayer.
 */
ObjectFromPlayer MariaObjectFromPlayer(uint8_t object)
{
    ObjectFromPlayer from;
    uint8_t player = rd8(OBJ_SPRITE_FLIP + PLAYER);

    wr8((uint16_t)(OBJ_SPRITE_FLIP + object), (uint8_t)(player << 1));
    jsr_push(0xC390);
    from.player_2fe8 = MariaObjectTake2FE8(object);
    jsr_pop();
    from.facing = rd8(OBJ_FACING_LEFT + PLAYER);
    wr8((uint16_t)(OBJ_FACING_LEFT + object), from.facing);
    return from;
}

void abi_MariaObjectFromPlayer(void)
{
    ObjectFromPlayer from = MariaObjectFromPlayer(cpu.x);

    cpu.a = from.facing;
    abi_nz(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
}

/*
 * @routine 01BD:3F8 RichterObjectFromPlayer
 * @run 6E:C3F8
 * WHAT IT DOES: Gives an object Richter's facing and his OBJ_SPRITE_FLIP, doubled.
 * HOW: RichterObjectTake2FE8, then the facing copied.
 * WAITS: no.
 * Entry: X object. Exit: A the facing, N and Z from it, C as RichterObjectTake2FE8. X, Y and V
 * kept.
 */
ObjectFromPlayer RichterObjectFromPlayer(uint8_t object)
{
    ObjectFromPlayer from;

    jsr_push(0xC3FA);
    from.player_2fe8 = RichterObjectTake2FE8(object);
    jsr_pop();
    from.facing = rd8(OBJ_FACING_LEFT + PLAYER);
    wr8((uint16_t)(OBJ_FACING_LEFT + object), from.facing);
    return from;
}

void abi_RichterObjectFromPlayer(void)
{
    ObjectFromPlayer from = RichterObjectFromPlayer(cpu.x);

    cpu.a = from.facing;
    abi_nz(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
}

/*
 * @routine 01BD:409 RichterObjectSlot
 * @run 6E:C409
 * WHAT IT DOES: Gives the slot of the object being run.
 * HOW: zRichterObject.
 * WAITS: no.
 * Entry: nothing. Exit: X the slot, N and Z from it. A, Y, C and V kept.
 */
static uint8_t object_slot_body(void)
{
    return rd8(zRichterObject);
}

uint8_t RichterObjectSlot(void)
{
    return object_slot_body();
}

void abi_RichterObjectSlot(void)
{
    cpu.x = RichterObjectSlot();
    abi_nz(cpu.x);
}

/*
 * @routine 01E1:703 MariaObjectSlot
 * @run 6E:C703
 * WHAT IT DOES: RichterObjectSlot in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterObjectSlot's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectSlot.
 */
uint8_t MariaObjectSlot(void)
{
    return object_slot_body();
}

void abi_MariaObjectSlot(void)
{
    cpu.x = MariaObjectSlot();
    abi_nz(cpu.x);
}

/*
 * @routine 01BD:5C5 RichterObjectNextState
 * @run 6E:C5C5
 * WHAT IT DOES: Moves an object on to its next state.
 * HOW: OBJ_STATE counted up.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state. A, X, Y, C and V kept.
 */
static uint8_t object_next_state_body(uint8_t object)
{
    uint8_t state = (uint8_t)(rd8((uint16_t)(OBJ_STATE + object)) + 1);

    wr8((uint16_t)(OBJ_STATE + object), state);
    return state;
}

uint8_t RichterObjectNextState(uint8_t object)
{
    return object_next_state_body(object);
}

void abi_RichterObjectNextState(void)
{
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01E1:589 MariaObjectNextState
 * @run 6E:C589
 * WHAT IT DOES: RichterObjectNextState in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterObjectNextState's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectNextState.
 */
uint8_t MariaObjectNextState(uint8_t object)
{
    return object_next_state_body(object);
}

void abi_MariaObjectNextState(void)
{
    abi_nz(MariaObjectNextState(cpu.x));
}

/*
 * @routine 01BD:47C RichterObjectQuicken
 * @run 6E:C47C
 * WHAT IT DOES: Once, at its animation's step 6 and while PlayerByte3BE1 is set, makes an object's
 * frame last 4 longer and Richter's one frame shorter than that.
 * HOW: OBJ_WORK_G marks it done; OBJ_2F38 the step; OBJ_HURT_TIMER the frame's time.
 * WAITS: no.
 * Entry: X object. Exit: X and Y kept. Stopped, A what was read last, N and Z from it, or from
 * comparing the step with 6 and C with it. Done, A Richter's new time, N and Z from the mark, C
 * and V from the add.
 */
Quicken RichterObjectQuicken(uint8_t object)
{
    Quicken quicken = { QUICKEN_DONE_BEFORE, 0, 0, { 0, false, false } };
    uint8_t time;

    quicken.value = rd8((uint16_t)(OBJ_WORK_G + object));
    if (quicken.value != 0)
        return quicken;
    quicken.value = rd8((uint16_t)(OBJ_SCROLL_BAND + object));
    quicken.how = QUICKEN_NOT_STEP;
    if (quicken.value != OBJECT_STATE_QUICK)
        return quicken;
    quicken.value = rd8(PlayerByte3BE1);
    quicken.how = QUICKEN_NOT_ASKED;
    if (quicken.value == 0)
        return quicken;
    quicken.sum = add_bytes(OBJECT_QUICKEN, rd8((uint16_t)(OBJ_HURT_TIMER + object)), false);
    wr8((uint16_t)(OBJ_HURT_TIMER + object), quicken.sum.value);
    time = (uint8_t)(quicken.sum.value - 1);
    wr8(OBJ_HURT_TIMER + PLAYER, time);
    quicken.value = time;
    quicken.mark = (uint8_t)(rd8((uint16_t)(OBJ_WORK_G + object)) + 1);
    wr8((uint16_t)(OBJ_WORK_G + object), quicken.mark);
    quicken.how = QUICKEN_DONE;
    return quicken;
}

void abi_RichterObjectQuicken(void)
{
    Quicken quicken = RichterObjectQuicken(cpu.x);

    cpu.a = quicken.value;
    switch (quicken.how) {
    case QUICKEN_NOT_STEP:
        abi_nz((uint8_t)(quicken.value - OBJECT_STATE_QUICK));
        abi_flag(P_C, quicken.value >= OBJECT_STATE_QUICK);
        break;
    case QUICKEN_DONE:
        abi_nz(quicken.mark);
        abi_flag(P_C, quicken.sum.carry);
        abi_flag(P_V, quicken.sum.overflow);
        break;
    default:
        abi_nz(quicken.value);
        break;
    }
}

/*
 * @routine 01BD:49F RichterObjectFlash
 * @run 6E:C49F
 * WHAT IT DOES: While an object is charged, shows the flashing form of its frame, if it is one
 * of the four that have one, with its OBJ_SPRITE_ATTR for the flash.
 * HOW: OBJ_WORK_F is the charge; frames $07, $0E, $15 and $1C become $8E to $91; RichterFlashFlags
 * by OBJ_WORK_D less $0E, with bit 7.
 * WAITS: no.
 * Entry: X object. Exit: X kept. Not charged, A 0, Z set, Y $8E. Not one of the four, A the
 * frame, N, Z and C from comparing it with $1C, Y $91. Flashed, A the flags, N and Z from them, Y
 * OBJ_WORK_D less $0E, C and V from that subtract. V kept otherwise.
 */
Flash RichterObjectFlash(uint8_t object)
{
    Flash flash = { FLASH_NOT_CHARGED, 0, 0, 0 };

    flash.code = OBJECT_FLASH_FIRST;
    if (rd8((uint16_t)(OBJ_WORK_F + object)) == 0)
        return flash;
    flash.value = rd8((uint16_t)(OBJ_FRAME + object));
    flash.how = FLASH_OTHER_FRAME;
    for (unsigned i = 0; i < sizeof flash_frames; i++) {
        flash.code = (uint8_t)(OBJECT_FLASH_FIRST + i);
        if (flash.value == flash_frames[i]) {
            wr8((uint16_t)(OBJ_FRAME + object), flash.code);
            flash.base = rd8((uint16_t)(OBJ_WORK_D + object));
            flash.value = (uint8_t)(rd8((uint16_t)(RichterFlashFlags + (uint8_t)(flash.base - OBJECT_FLASH_BASE))) |
                                    OBJECT_FLASH_BIT);
            wr8((uint16_t)(OBJ_SPRITE_ATTR + object), flash.value);
            flash.how = FLASH_SHOWN;
            return flash;
        }
    }
    return flash;
}

void abi_RichterObjectFlash(void)
{
    Flash flash = RichterObjectFlash(cpu.x);
    uint8_t index = (uint8_t)(flash.base - OBJECT_FLASH_BASE);

    switch (flash.how) {
    case FLASH_NOT_CHARGED:
        cpu.a = 0;
        cpu.y = flash.code;
        abi_nz(0);
        break;
    case FLASH_OTHER_FRAME:
        cpu.a = flash.value;
        cpu.y = flash.code;
        abi_nz((uint8_t)(flash.value - flash_frames[sizeof flash_frames - 1]));
        abi_flag(P_C, flash.value >= flash_frames[sizeof flash_frames - 1]);
        break;
    case FLASH_SHOWN:
        cpu.a = flash.value;
        cpu.y = index;
        abi_nz(flash.value);
        abi_flag(P_C, flash.base >= OBJECT_FLASH_BASE);
        abi_flag(P_V, ((flash.base ^ OBJECT_FLASH_BASE) & (flash.base ^ index) & 0x80) != 0);
        break;
    }
}

/*
 * @routine 01BD:4D1 RichterObjectCharge
 * @run 6E:C4D1
 * WHAT IT DOES: Charges an object from OBJ_WORK_D $0C on, the first time the way it faces is newly
 * pressed.
 * HOW: OBJ_WORK_F counted up; RichterChargePads by OBJ_FACING_LEFT against zPadPressed.
 * WAITS: no.
 * Entry: X object. Exit: X kept. Before $0C, A OBJ_WORK_D, N, Z and C from comparing it with $0C.
 * Charged already, A the charge, N and Z from it, C set. Not pressed, A 0, Z set, C set, Y the
 * facing. Charged now, A the press, N and Z from the charge, C set, Y the facing. V kept.
 */
Charge RichterObjectCharge(uint8_t object)
{
    Charge charge = { CHARGE_TOO_SOON, 0, 0, 0 };

    charge.value = rd8((uint16_t)(OBJ_WORK_D + object));
    if (charge.value < OBJECT_CHARGE_FROM)
        return charge;
    charge.value = rd8((uint16_t)(OBJ_WORK_F + object));
    charge.how = CHARGE_ALREADY;
    if (charge.value != 0)
        return charge;
    charge.facing = rd8((uint16_t)(OBJ_FACING_LEFT + object));
    charge.value = (uint8_t)(rd8(zPadPressed) & rd8((uint16_t)(RichterChargePads + charge.facing)));
    charge.how = CHARGE_NOT_PRESSED;
    if (charge.value == 0)
        return charge;
    charge.charge = (uint8_t)(rd8((uint16_t)(OBJ_WORK_F + object)) + 1);
    wr8((uint16_t)(OBJ_WORK_F + object), charge.charge);
    charge.how = CHARGE_NOW;
    return charge;
}

void abi_RichterObjectCharge(void)
{
    Charge charge = RichterObjectCharge(cpu.x);

    cpu.a = charge.value;
    switch (charge.how) {
    case CHARGE_TOO_SOON:
        abi_nz((uint8_t)(charge.value - OBJECT_CHARGE_FROM));
        abi_flag(P_C, false);
        return;
    case CHARGE_ALREADY:
        abi_nz(charge.value);
        break;
    case CHARGE_NOT_PRESSED:
        cpu.y = charge.facing;
        abi_nz(0);
        break;
    case CHARGE_NOW:
        cpu.y = charge.facing;
        abi_nz(charge.charge);
        break;
    }
    abi_flag(P_C, true);
}

/*
 * @routine 01BE:67B RichterObjectAtPlayer
 * @run 6E:CE7B
 * WHAT IT DOES: Puts an object at Richter's place plus an offset, signed, the X offset turned
 * round when the object faces left.
 * HOW: zTemp04 and zTemp06 the offsets, made 16-bit in zTemp05 and zTemp07.
 * WAITS: no.
 * Entry: X object, zTemp04 and zTemp06. Exit: A the Y position's high byte, N, Z, C and V from its
 * add. X and Y kept.
 */
Sum RichterObjectAtPlayer(uint8_t object)
{
    uint8_t value, base;
    Sum sum;

    WR8(ZP->temp05, 0);
    WR8(ZP->temp07, 0);
    value = RD8(ZP->temp04);
    if (value & 0x80) {
        DEC8(ZP->temp05);
    }
    value = RD8(ZP->temp06);
    if (value & 0x80) {
        DEC8(ZP->temp07);
    }
    value = RD8(OBJ->facing_left[object]);
    if (value != 0) {
        value = RD8(ZP->temp04);
        sum = sub_bytes(0, value, true);
        WR8(ZP->temp04, sum.value);
        value = RD8(ZP->temp05);
        sum = sub_bytes(0, value, sum.carry);
        WR8(ZP->temp05, sum.value);
    }
    base = RD8(OBJ->x_lo[0]);
    value = RD8(ZP->temp04);
    sum = add_bytes(base, value, false);
    WR8(OBJ->x_lo[object], sum.value);
    base = RD8(OBJ->x_hi[0]);
    value = RD8(ZP->temp05);
    sum = add_bytes(base, value, sum.carry);
    WR8(OBJ->x_hi[object], sum.value);
    base = RD8(OBJ->y_lo[0]);
    value = RD8(ZP->temp06);
    sum = add_bytes(base, value, false);
    WR8(OBJ->y_lo[object], sum.value);
    base = RD8(OBJ->y_hi[0]);
    value = RD8(ZP->temp07);
    sum = add_bytes(base, value, sum.carry);
    WR8(OBJ->y_hi[object], sum.value);
    return sum;
}

void abi_RichterObjectAtPlayer(void)
{
    Sum high = RichterObjectAtPlayer(cpu.x);

    cpu.a = high.value;
    abi_nz(high.value);
    abi_flag(P_C, high.carry);
    abi_flag(P_V, high.overflow);
}

/*
 * @routine 01E1:4EC MariaObjectAtPlayer
 * @run 6E:C4EC
 * WHAT IT DOES: RichterObjectAtPlayer in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterObjectAtPlayer's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectAtPlayer.
 */
Sum MariaObjectAtPlayer(uint8_t object)
{
    return RichterObjectAtPlayer(object);
}

void abi_MariaObjectAtPlayer(void)
{
    Sum high = MariaObjectAtPlayer(cpu.x);

    cpu.a = high.value;
    abi_nz(high.value);
    abi_flag(P_C, high.carry);
    abi_flag(P_V, high.overflow);
}

/*
 * @routine 01BD:00C RichterObjectAddXLow
 * @run 6E:C00C
 * WHAT IT DOES: Adds to an object's X position's low byte.
 * HOW: A plain add, the carry left for RichterObjectAddXHigh.
 * WAITS: no.
 * Entry: A the amount, X object. Exit: A the low byte, N, Z, C and V from the add. X and Y kept.
 */
static Sum object_add_x_low_body(uint8_t object, uint8_t amount)
{
    Sum sum = add_bytes(amount, rd8((uint16_t)(OBJ_X_LO + object)), false);

    wr8((uint16_t)(OBJ_X_LO + object), sum.value);
    return sum;
}

Sum RichterObjectAddXLow(uint8_t object, uint8_t amount)
{
    return object_add_x_low_body(object, amount);
}

void abi_RichterObjectAddXLow(void)
{
    abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a));
}

/*
 * @routine 01E1:27C MariaObjectAddXLow
 * @run 6E:C27C
 * WHAT IT DOES: RichterObjectAddXLow in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterObjectAddXLow's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectAddXLow.
 */
Sum MariaObjectAddXLow(uint8_t object, uint8_t amount)
{
    return object_add_x_low_body(object, amount);
}

void abi_MariaObjectAddXLow(void)
{
    abi_exit_sum(MariaObjectAddXLow(cpu.x, cpu.a));
}

/*
 * @routine 01BD:014 RichterObjectAddXHigh
 * @run 6E:C014
 * WHAT IT DOES: Adds to an object's X position's high byte, with the carry from the low byte.
 * HOW: An add with the carry in.
 * WAITS: no.
 * Entry: A the amount, C the carry, X object. Exit: A the high byte, N, Z, C and V from the add.
 * X and Y kept.
 */
static Sum object_add_x_high_body(uint8_t object, uint8_t amount, bool carry)
{
    Sum sum = add_bytes(amount, rd8((uint16_t)(OBJ_X_HI + object)), carry);

    wr8((uint16_t)(OBJ_X_HI + object), sum.value);
    return sum;
}

Sum RichterObjectAddXHigh(uint8_t object, uint8_t amount, bool carry)
{
    return object_add_x_high_body(object, amount, carry);
}

void abi_RichterObjectAddXHigh(void)
{
    abi_exit_sum(RichterObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0));
}

/*
 * @routine 01E1:284 MariaObjectAddXHigh
 * @run 6E:C284
 * WHAT IT DOES: RichterObjectAddXHigh in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterObjectAddXHigh's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectAddXHigh.
 */
Sum MariaObjectAddXHigh(uint8_t object, uint8_t amount, bool carry)
{
    return object_add_x_high_body(object, amount, carry);
}

void abi_MariaObjectAddXHigh(void)
{
    abi_exit_sum(MariaObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0));
}

/*
 * @routine 01BD:1D5 RichterObjectSetFall
 * @run 6E:C1D5
 * WHAT IT DOES: Sets an object's Y speed's low byte, carrying into its high byte.
 * HOW: The carry added to OBJ_VY_HI.
 * WAITS: no.
 * Entry: A the low byte, C the carry, X object. Exit: A the high byte, N, Z, C and V from adding
 * the carry. X and Y kept.
 */
Sum RichterObjectSetFall(uint8_t object, uint8_t low, bool carry)
{
    Sum high;

    wr8((uint16_t)(OBJ_VY_LO + object), low);
    high = add_bytes(0, rd8((uint16_t)(OBJ_VY_HI + object)), carry);
    wr8((uint16_t)(OBJ_VY_HI + object), high.value);
    return high;
}

void abi_RichterObjectSetFall(void)
{
    abi_exit_sum(RichterObjectSetFall(cpu.x, cpu.a, (cpu.p & P_C) != 0));
}

/* An object's Y speed made larger by a pull. */
static Sum fall_by(uint8_t object, uint8_t pull)
{
    Sum low = add_bytes(rd8((uint16_t)(OBJ_VY_LO + object)), pull, false);

    return RichterObjectSetFall(object, low.value, low.carry);
}

/*
 * @routine 01BD:1CF RichterObjectFall
 * @run 6E:C1CF
 * WHAT IT DOES: Pulls an object down: its Y speed grows by 6.
 * HOW: Runs on into RichterObjectSetFall.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectSetFall.
 */
Sum RichterObjectFall(uint8_t object)
{
    return fall_by(object, OBJECT_FALL);
}

void abi_RichterObjectFall(void)
{
    abi_exit_sum(RichterObjectFall(cpu.x));
}

/*
 * @routine 01BD:1E0 RichterObjectFallSlow
 * @run 6E:C1E0
 * WHAT IT DOES: Pulls an object down more gently: its Y speed grows by 3.
 * HOW: RichterObjectSetFall.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectSetFall.
 */
Sum RichterObjectFallSlow(uint8_t object)
{
    return fall_by(object, OBJECT_FALL_SLOW);
}

void abi_RichterObjectFallSlow(void)
{
    abi_exit_sum(RichterObjectFallSlow(cpu.x));
}

/*
 * @routine 01BE:6BE RichterObjectAtPlayerPlace
 * @run 6E:CEBE
 * WHAT IT DOES: Puts an object exactly where Richter is.
 * HOW: His position into zTemp00 to 03; runs on into RichterObjectToPoint.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectToPoint.
 */
uint8_t RichterObjectAtPlayerPlace(uint8_t object)
{
    wr8(zTemp00, rd8(OBJ_X_LO + PLAYER));
    wr8(zTemp01, rd8(OBJ_X_HI + PLAYER));
    wr8(zTemp02, rd8(OBJ_Y_LO + PLAYER));
    wr8(zTemp03, rd8(OBJ_Y_HI + PLAYER));
    return RichterObjectToPoint(object);
}

void abi_RichterObjectAtPlayerPlace(void)
{
    abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x));
}

/*
 * @routine 01BE:6D2 RichterObjectToPoint
 * @run 6E:CED2
 * WHAT IT DOES: Puts an object at the point in zTemp00 to 03.
 * HOW: X low and high, Y low and high.
 * WAITS: no.
 * Entry: X object. Exit: A the Y position's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterObjectToPoint(uint8_t object)
{
    uint8_t high;

    wr8((uint16_t)(OBJ_X_LO + object), rd8(zTemp00));
    wr8((uint16_t)(OBJ_X_HI + object), rd8(zTemp01));
    wr8((uint16_t)(OBJ_Y_LO + object), rd8(zTemp02));
    high = rd8(zTemp03);
    wr8((uint16_t)(OBJ_Y_HI + object), high);
    return high;
}

void abi_RichterObjectToPoint(void)
{
    abi_exit_loaded_object(RichterObjectToPoint(cpu.x));
}

/*
 * @routine 01C0:51B RichterObjectHitboxOff
 * @run 6E:DD1B
 * WHAT IT DOES: Takes an object's hitbox away.
 * HOW: OBJ_HITBOX_ON cleared.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void RichterObjectHitboxOff(uint8_t object)
{
    wr8((uint16_t)(OBJ_HITBOX_ON + object), 0);
}

void abi_RichterObjectHitboxOff(void)
{
    RichterObjectHitboxOff(cpu.x);
}

/*
 * @routine 01C0:52A RichterObjectHitboxFrom
 * @run 6E:DD2A
 * WHAT IT DOES: Gives an object the hitbox picked, or none.
 * HOW: Z from the caller says none; otherwise the hitbox's four bytes from RichterObjectHitboxes,
 * four bytes a hitbox.
 * WAITS: no.
 * Entry: A the hitbox, Z set for none, X object. Exit: none, as RichterObjectHitboxOff. Otherwise
 * A the last byte, N and Z from it, Y four times the hitbox, C its bit 6. X and V kept.
 */
Hitbox RichterObjectHitboxFrom(uint8_t object, uint8_t hitbox, bool none)
{
    Hitbox box = { false, 0, 0, 0 };
    uint8_t entry;

    if (none) {
        RichterObjectHitboxOff(object);
        return box;
    }
    box.given = true;
    box.hitbox = hitbox;
    entry = (uint8_t)(hitbox << 2);
    wr8((uint16_t)(OBJ_HITBOX_DX + object), rd8((uint16_t)(RichterObjectHitboxes + entry)));
    wr8((uint16_t)(OBJ_HITBOX_DY + object), rd8((uint16_t)(RichterObjectHitboxes + entry + 1)));
    wr8((uint16_t)(OBJ_HITBOX_HALF_W + object), rd8((uint16_t)(RichterObjectHitboxes + entry + 2)));
    box.last = rd8((uint16_t)(RichterObjectHitboxes + entry + 3));
    wr8((uint16_t)(OBJ_HITBOX_HALF_H + object), box.last);
    return box;
}

void hitbox_exit_shim(Hitbox box)
{
    if (!box.given)
        return;
    cpu.a = box.last;
    cpu.y = (uint8_t)(box.hitbox << 2);
    abi_nz(box.last);
    abi_flag(P_C, (box.hitbox & 0x40) != 0);
}

void abi_RichterObjectHitboxFrom(void)
{
    hitbox_exit_shim(RichterObjectHitboxFrom(cpu.x, cpu.a, (cpu.p & P_Z) != 0));
}

/*
 * @routine 01C0:524 RichterObjectHitboxByFrame
 * @run 6E:DD24
 * WHAT IT DOES: Gives an object the hitbox of the frame it shows, or none.
 * HOW: RichterFrameHitboxes by OBJ_FRAME; runs on into RichterObjectHitboxFrom.
 * WAITS: no.
 * Entry: X object. Exit: none, A 0, Z set, N clear, Y the frame. Otherwise as
 * RichterObjectHitboxFrom.
 */
Hitbox RichterObjectHitboxForFrame(uint8_t object)
{
    uint8_t frame = rd8((uint16_t)(OBJ_FRAME + object));
    uint8_t hitbox = rd8((uint16_t)(RichterFrameHitboxes + frame));
    Hitbox box = RichterObjectHitboxFrom(object, hitbox, hitbox == 0);

    box.frame = frame;
    return box;
}

void abi_RichterObjectHitboxByFrame(void)
{
    Hitbox box = RichterObjectHitboxForFrame(cpu.x);

    if (!box.given) {
        cpu.a = 0;
        cpu.y = box.frame;
        abi_nz(0);
        return;
    }
    hitbox_exit_shim(box);
}

/*
 * @routine 01C0:51F RichterObjectHitboxOn
 * @run 6E:DD1F
 * WHAT IT DOES: Marks an object as having a hitbox and gives it its frame's.
 * HOW: OBJ_HITBOX_ON 1; runs on into RichterObjectHitboxByFrame.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectHitboxByFrame.
 */
Hitbox RichterObjectHitboxOn(uint8_t object)
{
    wr8((uint16_t)(OBJ_HITBOX_ON + object), 1);
    return RichterObjectHitboxForFrame(object);
}

void abi_RichterObjectHitboxOn(void)
{
    Hitbox box = RichterObjectHitboxOn(cpu.x);

    if (!box.given) {
        cpu.a = 0;
        cpu.y = box.frame;
        abi_nz(0);
        return;
    }
    hitbox_exit_shim(box);
}

/*
 * @routine 01C0:284 RichterObjectAnimationList
 * @run 6E:DA84
 * WHAT IT DOES: Points at an object's animation list and its step's entry.
 * HOW: RichterObjectAnimations by OBJ_WORK_A into zTemp00 and 01; the step doubled.
 * WAITS: no.
 * Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
 * kept.
 */
static ObjectListStep object_animation_list(uint8_t object, uint16_t lists)
{
    ObjectListStep r;
    uint8_t doubled = (uint8_t)(RD8(OBJ->work_a[object]) << 1);
    uint8_t step;

    WR8(ZP->temp00, rd8((uint16_t)(lists + doubled)));
    WR8(ZP->temp01, rd8((uint16_t)(lists + 1 + doubled)));
    step = RD8(OBJ->scroll_band[object]);
    r.y = (uint8_t)(step << 1);
    r.carry = (step & 0x80) != 0;
    return r;
}

ObjectListStep RichterObjectAnimationList(uint8_t object)
{
    return object_animation_list(object, RichterObjectAnimations);
}

/*
 * @routine 01E4:5C8 MariaObjectAnimationList
 * @run 6E:DDC8
 * WHAT IT DOES: RichterObjectAnimationList in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of object animations at $DDDD, $344 above his.
 * WAITS: no.
 * Entry, exit: as RichterObjectAnimationList.
 */
ObjectListStep MariaObjectAnimationList(uint8_t object)
{
    return object_animation_list(object, RichterObjectAnimations + 0x344);
}

void abi_MariaObjectAnimationList(void)
{
    ObjectListStep r = MariaObjectAnimationList(cpu.x);

    cpu.a = r.y;
    cpu.y = r.y;
    abi_nz(r.y);
    abi_flag(P_C, r.carry);
}

void abi_RichterObjectAnimationList(void)
{
    ObjectListStep r = RichterObjectAnimationList(cpu.x);

    cpu.a = r.y;
    cpu.y = r.y;
    abi_nz(r.y);
    abi_flag(P_C, r.carry);
}

/* The exit state of an animation routine as the assembly leaves it. */
static AnimationStep object_animation_exit(uint8_t a, uint8_t nz, bool c, bool y_set, uint8_t y)
{
    AnimationStep r = { FRAME_STOPPED, false, false, 0, 0, true, a, nz, c, y_set, y };

    return r;
}

/*
 * Where one character's copy of the object animation routines lies: the return address each of its
 * calls pushes (the wait's, the running test's, the entry read's and the tick's), and its list
 * pointer. Maria's overlay (01DD) holds the same instructions at $DD84 to $DDC7.
 */
typedef struct {
    uint16_t wait_ret, running_ret, entry_ret, tick_ret;
    ObjectListStep (*list)(uint8_t object);
} ObjectAnimationLayout;

static const ObjectAnimationLayout RICHTER_OBJECT_ANIMATION = {
    0xDA43, 0xDA4B, 0xDA6B, 0xDA82, RichterObjectAnimationList
};
static const ObjectAnimationLayout MARIA_OBJECT_ANIMATION = {
    0xDD87, 0xDD8F, 0xDDAF, 0xDDC6, MariaObjectAnimationList
};

/* The entry of the object's step in play: how long, then which frame; a mark of $FE starts the list over. */
static AnimationStep object_read_entry(const ObjectAnimationLayout *layout, uint8_t object)
{
    AnimationStep result;
    ObjectListStep list;
    uint8_t time, frame, y;

    for (;;) {
        jsr_push(layout->entry_ret);
        list = layout->list(object);
        jsr_pop();
        y = list.y;
        time = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->hurt_timer[object], time);
        if (time == OBJECT_ANIMATION_STOPPED) {
            result = object_animation_exit(OBJECT_ANIMATION_STOPPED, 0, true, true, y);
            result.how = FRAME_END_MARK;
            return result;
        }
        if (time != OBJECT_ANIMATION_LOOPS) {
            break;
        }
        WR8(OBJ->scroll_band[object], 0);
    }
    y++;
    frame = RD8_PTR_Y(ZP->temp00, y);
    WR8(OBJ->frame[object], frame);
    result = object_animation_exit(frame, frame, false, true, y);
    result.how = FRAME_SHOWN;
    result.frame = frame;
    return result;
}

/*
 * @routine 01C0:263 RichterObjectSetAnimation
 * @run 6E:DA63
 * @stop DA84
 * WHAT IT DOES: Starts an object's animation from its first step.
 * HOW: The animation into OBJ_WORK_A, step zero, and its first entry read.
 * WAITS: no.
 * Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
static AnimationStep object_set_animation(const ObjectAnimationLayout *layout, uint8_t object, uint8_t animation)
{
    WR8(OBJ->work_a[object], animation);
    WR8(OBJ->scroll_band[object], 0);
    return object_read_entry(layout, object);
}

AnimationStep RichterObjectSetAnimation(uint8_t object, uint8_t animation)
{
    return object_set_animation(&RICHTER_OBJECT_ANIMATION, object, animation);
}

void abi_RichterObjectSetAnimation(void)
{
    animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a));
}

/*
 * @routine 01C0:251 RichterObjectAnimationTick
 * @run 6E:DA51
 * WHAT IT DOES: Counts down an object's frame on show; at zero moves to the next step of its list.
 * A timer of $FF means its animation has stopped.
 * HOW: OBJ_HURT_TIMER the timer, OBJ_SCROLL_BAND the step, OBJ_FRAME the frame shown.
 * WAITS: no.
 * Entry: X object. Exit: as RichterAnimationTick.
 */
static AnimationStep object_animation_tick(const ObjectAnimationLayout *layout, uint8_t object)
{
    AnimationStep result;
    ObjectListStep list;
    uint8_t time = RD8(OBJ->hurt_timer[object]);
    uint8_t left, y, frame;

    if (time == OBJECT_ANIMATION_STOPPED) {
        return object_animation_exit(OBJECT_ANIMATION_STOPPED, 0, true, false, 0);
    }
    left = DEC8(OBJ->hurt_timer[object]);
    if (left == 0) {
        INC8(OBJ->scroll_band[object]);
        return object_read_entry(layout, object);
    }
    jsr_push(layout->tick_ret);
    list = layout->list(object);
    jsr_pop();
    y = (uint8_t)(list.y + 1);
    frame = RD8_PTR_Y(ZP->temp00, y);
    WR8(OBJ->frame[object], frame);
    result = object_animation_exit(frame, frame, list.carry, true, y);
    result.how = FRAME_SHOWN;
    result.same_frame = true;
    result.frame = frame;
    return result;
}

AnimationStep RichterObjectAnimationTick(uint8_t object)
{
    return object_animation_tick(&RICHTER_OBJECT_ANIMATION, object);
}

void abi_RichterObjectAnimationTick(void)
{
    animation_exit_shim(RichterObjectAnimationTick(cpu.x));
}

/*
 * @routine 01C0:248 RichterObjectAnimationRunning
 * @run 6E:DA48
 * WHAT IT DOES: Steps an object's animation; its caller looks at whether it has ended.
 * HOW: RichterObjectAnimationTick, then the timer against $FF.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and
 * V kept. Clobbers zTemp00-01.
 */
static AnimationStep object_animation_running(const ObjectAnimationLayout *layout, uint8_t object)
{
    AnimationStep tick;
    uint8_t time;

    jsr_push(layout->running_ret);
    tick = object_animation_tick(layout, object);
    jsr_pop();
    time = RD8(OBJ->hurt_timer[object]);
    return object_animation_exit(time, (uint8_t)(time - OBJECT_ANIMATION_STOPPED), time >= OBJECT_ANIMATION_STOPPED, tick.y_set, tick.y);
}

AnimationStep RichterObjectAnimationRunning(uint8_t object)
{
    return object_animation_running(&RICHTER_OBJECT_ANIMATION, object);
}

void abi_RichterObjectAnimationRunning(void)
{
    animation_exit_shim(RichterObjectAnimationRunning(cpu.x));
}

/*
 * @routine 01C0:240 RichterObjectAnimationWait
 * @run 6E:DA40
 * WHAT IT DOES: Steps an object's animation and, unless it has ended or shows frame 0, returns to
 * its caller's caller, so the caller's own work waits for the end.
 * HOW: RichterObjectAnimationTick; Z from it; the assembly drops the return address with two pulls.
 * WAITS: no.
 * Entry: X object. Exit, Z set: as the tick. Otherwise the caller's return address is gone, A its
 * high byte, N and Z from it; its caller then ends at once.
 */
AnimationWaitEnd RichterObjectAnimationWait(uint8_t object)
{
    AnimationWaitEnd r = { { FRAME_STOPPED }, false, 0 };

    jsr_push(RICHTER_OBJECT_ANIMATION.wait_ret);
    r.step = object_animation_tick(&RICHTER_OBJECT_ANIMATION, object);
    jsr_pop();
    if (r.step.nz == 0) {
        return r;
    }
    pull8();
    r.high = pull8();
    r.dropped = true;
    r.step.a = r.high;
    r.step.nz = r.high;
    return r;
}

void abi_RichterObjectAnimationWait(void)
{
    animation_exit_shim(RichterObjectAnimationWait(cpu.x).step);
}

/*
 * @routine 01E4:58C MariaObjectAnimationRunning
 * @run 6E:DD8C
 * WHAT IT DOES: RichterObjectAnimationRunning in Maria's player overlay (01DD).
 * HOW: The same instructions with her tick, MariaObjectAnimationTick at $DD95.
 * WAITS: no.
 * Entry, exit: as RichterObjectAnimationRunning.
 */
AnimationStep MariaObjectAnimationRunning(uint8_t object)
{
    return object_animation_running(&MARIA_OBJECT_ANIMATION, object);
}

void abi_MariaObjectAnimationRunning(void)
{
    animation_exit_shim(MariaObjectAnimationRunning(cpu.x));
}

/*
 * @routine 01E4:595 MariaObjectAnimationTick
 * @run 6E:DD95
 * WHAT IT DOES: RichterObjectAnimationTick in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterObjectAnimationTick's body with her layout: its calls go to MariaObjectAnimationList
 * at $DDC8, where his C would read his list.
 * WAITS: no.
 * Entry, exit: as RichterObjectAnimationTick.
 */
AnimationStep MariaObjectAnimationTick(uint8_t object)
{
    return object_animation_tick(&MARIA_OBJECT_ANIMATION, object);
}

void abi_MariaObjectAnimationTick(void)
{
    animation_exit_shim(MariaObjectAnimationTick(cpu.x));
}

/*
 * @routine 01E4:5A7 MariaObjectSetAnimation
 * @run 6E:DDA7
 * @stop DDC8
 * WHAT IT DOES: RichterObjectSetAnimation in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterObjectSetAnimation's body with her layout: its calls go to MariaObjectAnimationList.
 * WAITS: no.
 * Entry, exit: as RichterObjectSetAnimation.
 */
AnimationStep MariaObjectSetAnimation(uint8_t object, uint8_t animation)
{
    return object_set_animation(&MARIA_OBJECT_ANIMATION, object, animation);
}

void abi_MariaObjectSetAnimation(void)
{
    animation_exit_shim(MariaObjectSetAnimation(cpu.x, cpu.a));
}


/*
 * Maria's own object code (her overlay 01DD, from $C000): routines with no copy in Richter's overlay. Tables of her
 * overlay are read by address: they have no row in config/rondo/tables.csv yet.
 */
enum {
    MARIA_OBJECT_FALL_LIMIT = 7, MARIA_ACCEL_TABLE = 0xC8A2, CAMERA_MODE_STILL = 0x17,
    NEAR_TOP_SCREEN = 1, NEAR_ACROSS_HALF = 4, NEAR_ACROSS = 8, NEAR_DOWN_HALF = 0x12, NEAR_DOWN = 0x24
};

/*
 * @routine 01E1:2CB MariaObjectFallCapped
 * @run 6E:C2CB
 * WHAT IT DOES: Adds an object's own pull (OBJ_WORK_H and OBJ_WORK_I) to its Y speed, held at $0700.
 * HOW: A word add into OBJ_VY_LO and OBJ_VY_HI; at 7 and up the speed is set to $0700.
 * Untraced: the hold at $0700 ($C2E4); no script let one of her objects fall that fast.
 * WAITS: no.
 * Entry: X object. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add; falling slower
 * than 7, N, Z and C from comparing it with 7; held, A 7, N and Z from it, C set. X and Y kept.
 */
Ended MariaObjectFallCapped(uint8_t object)
{
    Sum low, high;
    uint8_t pull;
    Ended e;

    pull = RD8(OBJ->work_h[object]);
    low = add_bytes(pull, RD8(OBJ->vy_lo[object]), false);
    WR8(OBJ->vy_lo[object], low.value);
    pull = RD8(OBJ->work_i[object]);
    high = add_bytes(pull, RD8(OBJ->vy_hi[object]), low.carry);
    WR8(OBJ->vy_hi[object], high.value);
    e = ended_of_sum(high);
    if (high.value & 0x80) {
        return e;
    }
    e = ended_then(e, ended_compare(high.value, MARIA_OBJECT_FALL_LIMIT));
    if (high.value < MARIA_OBJECT_FALL_LIMIT) {
        return e;
    }
    WR8(OBJ->vy_hi[object], MARIA_OBJECT_FALL_LIMIT);
    WR8(OBJ->vy_lo[object], 0);
    return ended_then(e, ended_load(MARIA_OBJECT_FALL_LIMIT));
}

void abi_MariaObjectFallCapped(void)
{
    abi_ended(MariaObjectFallCapped(cpu.x));
}

/* The negative of a scroll speed, and zTemp10 its sign byte ($FF for a negative). */
static uint8_t against_scroll(uint16_t speed_field)
{
    uint8_t value;

    WR8(ZP->temp10, 0);
    value = (uint8_t)((rd8(speed_field) ^ 0xFF) + 1);
    if (value & 0x80) {
        DEC8(ZP->temp10);
    }
    return value;
}

/*
 * @routine 01E1:328 MariaObjectAgainstScroll
 * @run 6E:C328
 * WHAT IT DOES: Moves an object by the negative of the frame's scroll on both axes, so it stays put on the map
 * while the screen moves.
 * HOW: ScrollSpeedX and ScrollSpeedY negated, signed into zTemp10; X through MariaObjectAddXLow and
 * MariaObjectAddXHigh, Y by a word add of its own.
 * Untraced: the sign byte of a negative scroll, $C332 and $C346 (the scroll's negative was never below zero).
 * WAITS: no.
 * Entry: X object. Exit: A the Y position's high byte, N, Z, C and V from its add. X and Y kept. Clobbers zTemp10.
 */
Sum MariaObjectAgainstScroll(uint8_t object)
{
    uint8_t amount, sign;
    Sum sum;

    amount = against_scroll(ScrollSpeedX);
    jsr_push(0xC337);
    sum = MariaObjectAddXLow(object, amount);
    jsr_pop();
    sign = RD8(ZP->temp10);
    jsr_push(0xC33C);
    MariaObjectAddXHigh(object, sign, sum.carry);
    jsr_pop();
    amount = against_scroll(ScrollSpeedY);
    sum = add_bytes(amount, RD8(OBJ->y_lo[object]), false);
    WR8(OBJ->y_lo[object], sum.value);
    sign = RD8(ZP->temp10);
    sum = add_bytes(sign, RD8(OBJ->y_hi[object]), sum.carry);
    WR8(OBJ->y_hi[object], sum.value);
    return sum;
}

void abi_MariaObjectAgainstScroll(void)
{
    abi_exit_sum(MariaObjectAgainstScroll(cpu.x));
}

/*
 * @routine 01E1:361 MariaObjectApplyVelocity
 * @run 6E:C361
 * WHAT IT DOES: Moves an object by its velocity, Y then X.
 * HOW: ApplyVelocityY, then a tail jump into ApplyVelocityX (through their kernel vectors).
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
 */
AxisMove MariaObjectApplyVelocity(uint8_t object)
{
    jsr_push(0xC364);
    ApplyVelocityY(object);
    jsr_pop();
    return ApplyVelocityX(object);
}

void abi_MariaObjectApplyVelocity(void)
{
    abi_ended(ended_of_axis_move(MariaObjectApplyVelocity(cpu.x)));
}

/*
 * @routine 01E1:358 MariaObjectMove
 * @run 6E:C358
 * WHAT IT DOES: Moves an object by its velocity, and against the scroll unless the camera holds still (mode $17).
 * HOW: CameraMode, MariaObjectAgainstScroll, then on into MariaObjectApplyVelocity.
 * Untraced: the camera's mode $17, which skips the scroll ($C35D taken).
 * WAITS: no.
 * Entry: X object. Exit: as MariaObjectApplyVelocity. Clobbers zTemp10.
 */
AxisMove MariaObjectMove(uint8_t object)
{
    if (rd8(CameraMode) != CAMERA_MODE_STILL) {
        jsr_push(0xC361);
        MariaObjectAgainstScroll(object);
        jsr_pop();
    }
    return MariaObjectApplyVelocity(object);
}

void abi_MariaObjectMove(void)
{
    abi_ended(ended_of_axis_move(MariaObjectMove(cpu.x)));
}

/*
 * @routine 01E1:369 MariaObjectFacingVelocity
 * @run 6E:C369
 * WHAT IT DOES: Turns an object's X velocity round when it faces left.
 * HOW: OBJ_FACING_LEFT, then NegateVelocityX (through its kernel vector).
 * WAITS: no.
 * Entry: X object. Exit: facing right, A 0, Z set; else as NegateVelocityX. X and Y kept.
 */
Ended MariaObjectFacingVelocity(uint8_t object)
{
    uint8_t facing = RD8(OBJ->facing_left[object]);
    Sum negated;

    if (facing == 0) {
        return ended_load(facing);
    }
    jsr_push(0xC371);
    negated = NegateVelocityX(object);
    jsr_pop();
    return ended_of_sum(negated);
}

void abi_MariaObjectFacingVelocity(void)
{
    abi_ended(MariaObjectFacingVelocity(cpu.x));
}

/*
 * @routine 01E1:677 MariaObjectClear
 * @run 6E:C677
 * WHAT IT DOES: Clears an object: the end her object handlers branch to.
 * HOW: A tail jump into ClearObject (through its kernel vector).
 * WAITS: no.
 * Entry: X object. Exit: every register and flag kept.
 */
void MariaObjectClear(uint8_t object)
{
    ClearObject(object);
}

void abi_MariaObjectClear(void)
{
    MariaObjectClear(cpu.x);
}

/*
 * @routine 01E2:080 MariaObjectAccelerate
 * @run 6E:C880
 * WHAT IT DOES: Adds to an object's X speed the step for the way it faces, and says whether the speed's sign
 * changed.
 * HOW: The word from her table at $C8A2 by OBJ_FACING_LEFT, added into OBJ_VX_LO and OBJ_VX_HI; the old high byte kept in
 * zTemp00 and its bit 7 compared.
 * WAITS: no.
 * Entry: X object. Exit: A $80 if the sign changed, else 0, N and Z from it; C and V from the high byte's add; Y
 * twice the facing. X kept.
 */
Ended MariaObjectAccelerate(uint8_t object)
{
    uint8_t y = (uint8_t)(RD8(OBJ->facing_left[object]) << 1);
    uint8_t step, before, changed;
    Sum low, high;

    step = rd8((uint16_t)(MARIA_ACCEL_TABLE + y));
    low = add_bytes(step, RD8(OBJ->vx_lo[object]), false);
    WR8(OBJ->vx_lo[object], low.value);
    before = RD8(OBJ->vx_hi[object]);
    WR8(ZP->temp00, before);
    step = rd8((uint16_t)(MARIA_ACCEL_TABLE + 1 + y));
    high = add_bytes(step, RD8(OBJ->vx_hi[object]), low.carry);
    WR8(OBJ->vx_hi[object], high.value);
    changed = (uint8_t)((high.value ^ RD8(ZP->temp00)) & 0x80);
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(changed), high.carry), high.overflow), y);
}

void abi_MariaObjectAccelerate(void)
{
    abi_ended(MariaObjectAccelerate(cpu.x));
}

/*
 * @routine 01E2:0A6 MariaObjectNearPlayer
 * @run 6E:C8A6
 * WHAT IT DOES: Says whether an object is at Maria: within 4 across and $12 down of her point (her Y plus her
 * OBJ_HITBOX_DY), on the first screen; on the second screen down it counts as near at once.
 * HOW: OBJ_Y_HI against 1, OBJ_X_HI, the X difference plus 4 against 8, the Y difference plus $12 against $24.
 * Untraced: the second screen ($C8AB taken) and an object below the first screen ($C8CC taken).
 * WAITS: no.
 * Entry: X object. Exit: C set if near; A the last value tested, N and Z from its test; V from the last add. X and
 * Y kept. Clobbers zTemp00.
 */
Ended MariaObjectNearPlayer(uint8_t object)
{
    uint8_t value, point;
    Sum sum;
    Ended e;

    value = RD8(OBJ->y_hi[object]);
    if (value == NEAR_TOP_SCREEN) {
        return ended_with_carry(ended_compare(value, NEAR_TOP_SCREEN), true);
    }
    value = RD8(OBJ->y_lo[0]);
    sum = add_bytes(value, RD8(OBJ->hitbox_dy[0]), false);
    WR8(ZP->temp00, sum.value);
    e = ended_with_overflow(ended_kept(), sum.overflow);
    value = RD8(OBJ->x_hi[object]);
    if (value != 0) {
        return ended_with_carry(ended_then(e, ended_load(value)), false);
    }
    value = RD8(OBJ->x_lo[object]);
    sum = sub_bytes(value, RD8(OBJ->x_lo[0]), true);
    sum = add_bytes(sum.value, NEAR_ACROSS_HALF, false);
    e = ended_then(ended_with_overflow(e, sum.overflow), ended_compare(sum.value, NEAR_ACROSS));
    if (sum.value >= NEAR_ACROSS) {
        return ended_with_carry(e, false);
    }
    value = RD8(OBJ->y_hi[object]);
    if (value != 0) {
        return ended_with_carry(ended_then(e, ended_load(value)), false);
    }
    value = RD8(OBJ->y_lo[object]);
    point = RD8(ZP->temp00);
    sum = sub_bytes(value, point, true);
    sum = add_bytes(sum.value, NEAR_DOWN_HALF, false);
    e = ended_then(ended_with_overflow(e, sum.overflow), ended_compare(sum.value, NEAR_DOWN));
    return ended_with_carry(e, sum.value < NEAR_DOWN);
}

void abi_MariaObjectNearPlayer(void)
{
    abi_ended(MariaObjectNearPlayer(cpu.x));
}

enum { MARIA_SLOTS_B_FIRST = 0x0B, MARIA_SLOTS_B_END = 0x0F, MARIA_KIND_18 = 0x18, MARIA_KIND_4 = 4 };

/*
 * @routine 01DE:48B MariaClaimSlotsB
 * @run 6D:AC8B
 * WHAT IT DOES: Takes every free one of object slots $0B to $0E for an object of kind $18.
 * HOW: OBJ_KIND zero is free: ClearObject (through its kernel vector), then the kind.
 * Untraced: a slot already in use ($AC90 taken); no script had one taken when she jumped.
 * WAITS: no.
 * Entry: nothing. Exit: X $0F; A the last kind read or $18; N clear, Z and C set from the CPX. Y and V kept.
 */
uint8_t MariaClaimSlotsB(void)
{
    uint8_t slot, kind = 0;

    for (slot = MARIA_SLOTS_B_FIRST; slot != MARIA_SLOTS_B_END; slot++) {
        kind = RD8(OBJ->kind[slot]);
        if (kind != 0) {
            continue;
        }
        jsr_push(0xAC95);
        ClearObject(slot);
        jsr_pop();
        kind = MARIA_KIND_18;
        WR8(OBJ->kind[slot], kind);
    }
    return kind;
}

void abi_MariaClaimSlotsB(void)
{
    cpu.a = MariaClaimSlotsB();
    cpu.x = MARIA_SLOTS_B_END;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 01DE:7D7 MariaClearSlotAsKind
 * @run 6D:AFD7
 * WHAT IT DOES: Clears an object slot and gives it a kind.
 * HOW: ClearObject (through its kernel vector), then Y into OBJ_KIND.
 * WAITS: no.
 * Entry: X object, Y the kind. Exit: A the kind, N and Z from it. X, Y, C and V kept.
 */
uint8_t MariaClearSlotAsKind(uint8_t object, uint8_t kind)
{
    jsr_push(0xAFDA);
    ClearObject(object);
    jsr_pop();
    WR8(OBJ->kind[object], kind);
    return kind;
}

void abi_MariaClearSlotAsKind(void)
{
    cpu.a = MariaClearSlotAsKind(cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 01DE:7BA MariaReplaceSlotsOneTwo
 * @run 6D:AFBA
 * WHAT IT DOES: Clears object slots 1 and 2 as kind 4, each unless it holds kind 1 or 2.
 * HOW: OBJ_KIND compared with 1 and 2; MariaClearSlotAsKind for slot 1, then a run on into it for slot 2.
 * Untraced: a slot of kind 1 or 2 ($AFC3, $AFC7, $AFD1 and $AFD5 taken); no script had one there.
 * WAITS: no.
 * Entry: nothing. Exit: Y 4, X 2; slot 2 kept, A its kind, N, Z and C from comparing it with 1 or 2; else A 4, N
 * and Z from it, C from comparing the kind with 2. V kept.
 */
Ended MariaReplaceSlotsOneTwo(void)
{
    uint8_t slot, kind;
    Ended e = ended_with_y(ended_kept(), MARIA_KIND_4);

    for (slot = 1; slot <= 2; slot++) {
        kind = RD8(OBJ->kind[slot]);
        e = ended_then(e, ended_compare(kind, 1));
        if (kind == 1) {
            continue;
        }
        e = ended_then(e, ended_compare(kind, 2));
        if (kind == 2) {
            continue;
        }
        if (slot == 1) {
            jsr_push(0xAFCB);
            MariaClearSlotAsKind(slot, MARIA_KIND_4);
            jsr_pop();
        } else {
            MariaClearSlotAsKind(slot, MARIA_KIND_4);
        }
        e = ended_then(e, ended_load(MARIA_KIND_4));
    }
    return e;
}

void abi_MariaReplaceSlotsOneTwo(void)
{
    abi_ended(MariaReplaceSlotsOneTwo());
    cpu.x = 2;
}

/*
 * @routine 01BC:7FC RichterObjectStopX
 * @run 6D:BFFC
 * WHAT IT DOES: Stops an object along X.
 * HOW: Zeroes OBJ_VX_LO and OBJ_VX_HI.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void RichterObjectStopX(uint8_t slot)
{
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
}

void abi_RichterObjectStopX(void)
{
    RichterObjectStopX(cpu.x);
}

/*
 * @routine 01BD:003 RichterObjectStop
 * @run 6E:C003
 * WHAT IT DOES: Stops an object along X and Y.
 * HOW: RichterObjectStopX, then runs on into RichterObjectStopY.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void RichterObjectStop(uint8_t slot)
{
    jsr_push(0xC005);
    RichterObjectStopX(slot);
    jsr_pop();
    RichterObjectStopY(slot);
}

void abi_RichterObjectStop(void)
{
    RichterObjectStop(cpu.x);
}

/*
 * @routine 01BD:005 RichterObjectStopY
 * @run 6E:C005
 * WHAT IT DOES: Stops an object along Y.
 * HOW: Zeroes OBJ_VY_LO and OBJ_VY_HI.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void RichterObjectStopY(uint8_t slot)
{
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_RichterObjectStopY(void)
{
    RichterObjectStopY(cpu.x);
}
