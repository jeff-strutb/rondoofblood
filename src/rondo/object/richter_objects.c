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
#include "ops6280.h"
#include "maria.h"

Regs claim_kind3_slot(uint8_t object, uint16_t clear_ret);   /* object/maria_objects.c */

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
 * WHAT IT DOES: Once, at its animation's step 6 and while PlayerWhipStruck is set, makes an object's
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
    quicken.value = rd8((uint16_t)(OBJ_ANIMATION_AT + object));
    quicken.how = QUICKEN_NOT_STEP;
    if (quicken.value != OBJECT_STATE_QUICK)
        return quicken;
    quicken.value = rd8(PlayerWhipStruck);
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

static void quicken_exit(Quicken quicken)
{

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

void abi_RichterObjectQuicken(void)
{
    quicken_exit(RichterObjectQuicken(cpu.x));
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

static void flash_exit(Flash flash)
{
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

void abi_RichterObjectFlash(void)
{
    flash_exit(RichterObjectFlash(cpu.x));
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

static void charge_exit(Charge charge)
{

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

void abi_RichterObjectCharge(void)
{
    charge_exit(RichterObjectCharge(cpu.x));
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

static void hitbox_frame_exit(Hitbox box)
{

    if (!box.given) {
        cpu.a = 0;
        cpu.y = box.frame;
        abi_nz(0);
        return;
    }
    hitbox_exit_shim(box);
}

void abi_RichterObjectHitboxByFrame(void)
{
    hitbox_frame_exit(RichterObjectHitboxForFrame(cpu.x));
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

static void hitbox_on_exit(Hitbox box)
{

    if (!box.given) {
        cpu.a = 0;
        cpu.y = box.frame;
        abi_nz(0);
        return;
    }
    hitbox_exit_shim(box);
}

void abi_RichterObjectHitboxOn(void)
{
    hitbox_on_exit(RichterObjectHitboxOn(cpu.x));
}

/*
 * @routine 01C0:284 RichterObjectAnimationList
 * @run 6E:DA84
 * WHAT IT DOES: Points at an object's animation list and its step's entry.
 * HOW: RichterObjectAnimations by OBJ_ANIMATION into zTemp00 and 01; the step doubled.
 * WAITS: no.
 * Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
 * kept.
 */
static ObjectListStep object_animation_list(uint8_t object, uint16_t lists)
{
    ObjectListStep r;
    uint8_t doubled = (uint8_t)(RD8(OBJ->animation[object]) << 1);
    uint8_t step;

    WR8(ZP->temp00, rd8((uint16_t)(lists + doubled)));
    WR8(ZP->temp01, rd8((uint16_t)(lists + 1 + doubled)));
    step = RD8(OBJ->animation_at[object]);
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
 * HOW: The animation into OBJ_ANIMATION, step zero, and its first entry read.
 * WAITS: no.
 * Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
static AnimationStep object_set_animation(const ObjectAnimationLayout *layout, uint8_t object, uint8_t animation)
{
    WR8(OBJ->animation[object], animation);
    WR8(OBJ->animation_at[object], 0);
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
 * HOW: OBJ_HURT_TIMER the timer, OBJ_ANIMATION_AT the step, OBJ_FRAME the frame shown.
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
        INC8(OBJ->animation_at[object]);
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

/*
 * Richter's objects in bank $6E, from $C076 on: the pieces of his death, the dispatch of slots 1 to
 * 15 by kind (the 23 words at $C3BE), and each kind's handler. These bodies run on the register
 * file as their assembly does: the registers and flags each instruction leaves are in cpu as it
 * leaves them, and a call to typed C is followed by that callee's exit. Kinds are named by number
 * where the code does not settle what they are.
 */

/* A JSR to typed C: the frame around the call. */
#define JSR(ret, stmt) do { jsr_push(ret); stmt; jsr_pop(); } while (0)

/* What the object routines' typed C leaves, as their wrappers give it. */
static void from_player_exit(ObjectFromPlayer from)
{
    cpu.a = from.facing;
    abi_nz(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
}

static void sound_exit(uint8_t command)
{
    sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
}

/* RichterObjectAnimationWait at a JSR: true when it took the caller's return address (the caller ends). */
static bool object_wait(uint16_t ret)
{
    AnimationWaitEnd wait;

    jsr_push(ret);
    wait = RichterObjectAnimationWait(cpu.x);
    animation_exit_shim(wait.step);
    if (wait.dropped) {
        return true;
    }
    jsr_pop();
    return false;
}

enum {
    SLOT_END = 0x10, DEATH_PIECE_KIND = 1, DEATH_PIECE_TIMER_STEP = 2, DEATH_ROW_COUNT = 0x60,
    DEATH_ROW_TOP = 0x41, DEATH_POINTS = 6, DEATH_X_MIN = 0x10, DEATH_X_MAX = 0xF0, SCRIPT_DEATH_PIECE = 8,
    PLAYER_STATE_HIT = 9
};

/*
 * @routine 01BD:11C RichterDeathPointStep
 * @run 6E:C11C
 * WHAT IT DOES: Gives the stage overlay's entry at $8009 the point in zTemp08-09, then moves the point $40
 * on and counts X down.
 * HOW: The JSR to $8009 (the C of the routine resident there), a 16-bit add of $0040, DEX.
 * WAITS: no.
 * Entry: X the count, the rest as the stage's code takes them. Exit: A the point's high byte, C and V from
 * its add, X one less, N and Z from it; Y as the stage's code left it.
 */
void RichterDeathPointStep(void)
{
    call_routine(0xC11F, StageVecClearBitRows);
    op_lda(RD8(ZP->temp08));
    op_clc();
    op_adc(0x40);
    WR8(ZP->temp08, cpu.a);
    op_cla();
    op_adc(RD8(ZP->temp09));
    WR8(ZP->temp09, cpu.a);
    op_dex();
}

/*
 * @routine 01BD:100 RichterDeathPointRow
 * @run 6E:C100
 * WHAT IT DOES: Gives the stage overlay's entry at $8009 X points $40 apart from zTemp08-09, each with his
 * OBJ_WORK_K in A as zTemp0F; what the stage's code leaves in zTemp0F is his OBJ_WORK_K after; his OBJ_WORK_J
 * counts down, and at 0 his frame and kind go.
 * HOW: RichterDeathPointStep X times.
 * WAITS: no.
 * Entry: X the count, zTemp08-09 the first point. Exit: counting, N and Z from OBJ_WORK_J, A zTemp0F; ended,
 * A zTemp0F, Z set. X 0; Y as the stage's code left it; C and V from the last point's add.
 */
void RichterDeathPointRow(void)
{
    do {
        op_lda(RD8(OBJ->work_k[0]));
        WR8(ZP->temp0f, cpu.a);
        JSR(0xC108, RichterDeathPointStep());
    } while (!(cpu.p & P_Z));
    op_lda(RD8(ZP->temp0f));
    WR8(OBJ->work_k[0], cpu.a);
    WR8(OBJ->work_j[0], f_dec(RD8(OBJ->work_j[0])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    WR8(OBJ->frame[0], 0);
    WR8(OBJ->kind[0], 0);
}

/*
 * @routine 01BD:090 RichterDeathFrame
 * @run 6E:C090
 * WHAT IT DOES: The player's object in his death (state 0 on the floor, then state 1): moved with the
 * scroll, held between $10 and $F0 across, falling; when the animation ends, slots 1 to 15 become pieces
 * (kind 1, a timer of four times the slot), the death's row of points starts ($60, OBJ_WORK_K 0), the
 * death's sound, state 1. In state 1, moved and falling on even frames; the row of points from $6080 once
 * OBJ_WORK_J is down to $40.
 * HOW: RichterObjectScrollAndMove, RichterObjectFall, RichterAnimationRunning, RichterDeathSound;
 * RichterDeathPointRow.
 * WAITS: no.
 * Entry: X 0. Exit: as its last step leaves the registers.
 */
void RichterDeathFrame(void)
{
    AnimationStep running;

    op_lda(RD8(OBJ->kind[cpu.x]));
    if (cpu.p & P_Z) {
        return;
    }
    op_lda(RD8(OBJ->state[0]));
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(ZP->frame_tick));
        op_and(0x01);
        if (cpu.p & P_Z) {
            JSR(0xC0E8, RichterObjectScrollAndMove());
            JSR(0xC0EB, abi_exit_sum(RichterObjectFall(cpu.x)));
        }
        op_lda(RD8(OBJ->work_j[0]));
        op_cmp(DEATH_ROW_TOP);
        if (cpu.p & P_C) {
            WR8(OBJ->work_j[0], f_dec(RD8(OBJ->work_j[0])));
            return;
        }
        op_lda(0x60);
        WR8(ZP->temp09, cpu.a);
        op_lda(0x80);
        WR8(ZP->temp08, cpu.a);
        op_ldx(DEATH_POINTS);
        RichterDeathPointRow();
        return;
    }
    JSR(0xC09E, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->x_lo[0]));
    op_cmp(DEATH_X_MIN);
    if (!(cpu.p & P_C)) {
        op_lda(DEATH_X_MIN);
        WR8(OBJ->x_lo[0], cpu.a);
    } else {
        op_cmp(DEATH_X_MAX);
        if (cpu.p & P_C) {
            op_lda(DEATH_X_MAX);
            WR8(OBJ->x_lo[0], cpu.a);
        }
    }
    JSR(0xC0B8, abi_exit_sum(RichterObjectFall(cpu.x)));
    jsr_push(0xC0BB);
    running = RichterAnimationRunning();
    jsr_pop();
    animation_exit_shim(running);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_ldy(1);
    do {
        op_lda(DEATH_PIECE_KIND);
        WR8(OBJ->kind[cpu.y], cpu.a);
        op_tya();
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        WR8(OBJ->anim_timer[cpu.y], cpu.a);
        op_iny();
        op_cpy(SLOT_END);
    } while (!(cpu.p & P_Z));
    op_lda(DEATH_ROW_COUNT);
    WR8(OBJ->work_j[0], cpu.a);
    WR8(OBJ->work_k[0], 0);
    JSR(0xC0DB, death_exit_shim(RichterDeathSound(cpu.x, cpu.y)));
    WR8(OBJ->state[0], f_inc(RD8(OBJ->state[0])));
}

/*
 * @routine 01BD:12D RichterDeathPieceFrame
 * @run 6E:C12D
 * WHAT IT DOES: One of the death's pieces, slots 1 to 15: its state's handler, if the slot holds an object.
 * HOW: zRichterObject keeps the slot; the four words at $C13D by OBJ_STATE.
 * WAITS: no.
 * Entry: X the slot. Exit: empty, A 0, Z set; else as the handler.
 */
void RichterDeathPieceFrame(void)
{
    uint16_t entry, handler;

    op_lda(RD8(OBJ->kind[cpu.x]));
    if (cpu.p & P_Z) {
        return;
    }
    WR8(ZP->richter_object, cpu.x);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tax();
    entry = (uint16_t)(RichterDeathPieceStates + cpu.x);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xC145:
        RichterDeathPieceStart();
        break;
    case 0xC187:
        RichterDeathPieceFly();
        break;
    case 0xC190:
        RichterDeathPieceFall();
        break;
    case 0xC1A5:
        RichterDeathPieceEnd();
        break;
    default:
        run_routine(handler);
        break;
    }
}

/*
 * @routine 01BD:076 RichterDeathObjects
 * @run 6E:C076
 * WHAT IT DOES: The frame of his death's objects: the player's object, the scroll's steps stopped, then
 * each of slots 1 to 15 as a piece.
 * HOW: RichterDeathFrame for slot 0; ScrollStep cleared; RichterDeathPieceFrame for X 1 to $0F.
 * WAITS: no.
 * Entry: nothing. Exit: X $10, N clear, Z and C set; A and Y as the last piece left them.
 */
void RichterDeathObjects(void)
{
    op_clx();
    JSR(0xC07A, RichterDeathFrame());
    WR8(SCROLLE->step_x_hi[0], 0);
    WR8(SCROLLE->step_x_lo[0], 0);
    WR8(SCROLLE->step_y_lo[0], 0);
    WR8(SCROLLE->step_y_hi[0], 0);
    op_inx();
    do {
        JSR(0xC08A, RichterDeathPieceFrame());
        op_inx();
        op_cpx(SLOT_END);
    } while (!(cpu.p & P_Z));
}

/*
 * @routine 01BD:145 RichterDeathPieceStart
 * @run 6E:C145
 * WHAT IT DOES: A death piece's state 0: when its timer runs out, put at the player by the slot's offsets,
 * flung up and across by the slot's speeds, marked, animation $14.
 * HOW: The slot's bytes at $C218 and $C228 as RichterObjectAtPlayer's offsets; Y speed $FF and the byte at
 * $C1E8; X speed the word at $C1F8; RichterObjectMarkOn; a jump to RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: counting, N and Z from the timer; else as
 * RichterObjectStartAnimation.
 */
void RichterDeathPieceStart(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(rd8((uint16_t)(RichterDeathPieceOffsetsX + cpu.x)));
    WR8(ZP->temp04, cpu.a);
    op_lda(rd8((uint16_t)(RichterDeathPieceOffsetsY + cpu.x)));
    WR8(ZP->temp06, cpu.a);
    JSR(0xC15A, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(rd8((uint16_t)(RichterDeathPieceSpeedsY + cpu.x)));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    op_lda(0xFF);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    op_txa();
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterDeathPieceSpeedsX + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterDeathPieceSpeedsX + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    JSR(0xC177, op_lda(RichterObjectMarkOn(cpu.x)));
    op_lda(0x14);
    RichterObjectStartAnimation();
}

/*
 * @routine 01BD:17C RichterDeathPieceMove
 * @run 6E:C17C
 * WHAT IT DOES: A death piece moves with the scroll and falls slowly; then X against $0F.
 * HOW: RichterObjectScrollAndMove, RichterObjectFallSlow, CPX.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: X the slot, N, Z and C from comparing it with $0F; A and V as the
 * fall left them.
 */
void RichterDeathPieceMove(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xC181, RichterObjectScrollAndMove());
    JSR(0xC184, abi_exit_sum(RichterObjectFallSlow(cpu.x)));
    op_cpx(0x0F);
}

/*
 * @routine 01BD:187 RichterDeathPieceFly
 * @run 6E:C187
 * WHAT IT DOES: A death piece's state 1: moves until its animation ends, then the next state.
 * HOW: RichterDeathPieceMove, RichterObjectAnimationWait, a jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, as RichterObjectAnimationWait; else as
 * RichterObjectNextState.
 */
void RichterDeathPieceFly(void)
{
    JSR(0xC18A, RichterDeathPieceMove());
    if (object_wait(0xC18D)) {
        return;
    }
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:190 RichterDeathPieceFall
 * @run 6E:C190
 * WHAT IT DOES: A death piece's state 2: moves; the last piece (slot $0F), once the player's OBJ_WORK_J is
 * 0, starts the end's row ($40, OBJ_WORK_K 0) and goes on.
 * HOW: RichterDeathPieceMove; its RTS where it is not the last ($C186); RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterDeathPieceMove, or N and Z from OBJ_WORK_J, or as
 * RichterObjectNextState.
 */
void RichterDeathPieceFall(void)
{
    JSR(0xC193, RichterDeathPieceMove());
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->work_j[0]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(0x40);
    WR8(OBJ->work_j[0], cpu.a);
    WR8(OBJ->work_k[0], 0);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:1A5 RichterDeathPieceEnd
 * @run 6E:C1A5
 * WHAT IT DOES: A death piece's state 3: moves; the last piece gives the end's row of six points from $6C00,
 * and when the player's OBJ_WORK_J runs out clears slots 1 to 15 and sets zField89 to 1.
 * HOW: RichterDeathPieceMove; RichterDeathPointRow; ClearObject (through its kernel vector).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step; ended, A 1, X the slot, N and Z from it.
 */
void RichterDeathPieceEnd(void)
{
    JSR(0xC1A8, RichterDeathPieceMove());
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(0x6C);
    WR8(ZP->temp09, cpu.a);
    op_lda(0x00);
    WR8(ZP->temp08, cpu.a);
    op_ldx(DEATH_POINTS);
    JSR(0xC1B7, RichterDeathPointRow());
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->work_j[0]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_ldx(1);
    do {
        JSR(0xC1C3, ClearObject(cpu.x));
        op_inx();
        op_cpx(SLOT_END);
    } while (!(cpu.p & P_Z));
    op_lda(1);
    WR8(ZP->field89, cpu.a);
    op_ldx(RD8(ZP->richter_object));
}

enum { COMPANION_FIRST = 1, COMPANION_END = 3, SPRITE_SET_COMPANION = 3, COMPANION_DROP = 0x10,
       COMPANION_MARK = 0x08, COMPANION_HOLD = 0x08, SUB_MODE_ENDING = 3 };

/*
 * @routine 01BD:241 RichterCompanionKind1
 * @run 6E:C241
 * WHAT IT DOES: Slot 1 or 2 holding kind 1 or 2. State 0: when its timer runs out (or was 0), animation
 * $11, sprite set 3, state 1, its place from the player's by the offsets at $C2EF and $C2F3 (by the slot
 * less 1 and bit 1 of zFrameTick, kept in OBJ_WORK_E), and the timer $10 when the stage's terrain $10 below
 * him has bit 3. State 1: when the timer runs out, script 8 and the slot emptied; otherwise it rises with
 * OBJ_WORK_D, and once the animation ends goes back to state 0 unless the player's OBJ_ANIM_TIMER is below
 * 8, his invincibility runs, or the game is in sub-mode 3 (then emptied).
 * HOW: RichterObjectSetAnimation; StageTileRead ($8006); RichterStartHitScript with X pushed;
 * RichterObjectAnimationWait; ClearObject (through its kernel vector).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterCompanionKind1(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(OBJ->anim_timer[cpu.x]));
        if (!(cpu.p & P_Z)) {
            WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
            if (cpu.p & P_Z) {
                op_phx();
                op_lda(SCRIPT_DEATH_PIECE);
                JSR(0xC2B8, (RichterStartHitScript(cpu.a), hit_script_exit_shim(SCRIPT_DEATH_PIECE)));
                op_plx();
                ClearObject(cpu.x);
                return;
            }
        }
        op_ldy(RD8(OBJ->work_e[cpu.x]));
        op_lda(RD8(OBJ->work_d[cpu.x]));
        op_eor(0xFF);
        cpu.a = f_inc(cpu.a);
        op_clc();
        op_adc(rd8((uint16_t)(RichterCompanionOffsetsY + cpu.y)));
        op_clc();
        op_adc(RD8(OBJ->y_lo[0]));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        WR8(OBJ->work_d[cpu.x], f_inc(RD8(OBJ->work_d[cpu.x])));
        if (object_wait(0xC2D6)) {
            return;
        }
        op_lda(RD8(OBJ->anim_timer[0]));
        op_cmp(COMPANION_HOLD);
        if (!(cpu.p & P_C)) {
            ClearObject(cpu.x);
            return;
        }
        op_lda(RD8(PVARS->invincible_timer));
        if (!(cpu.p & P_Z)) {
            ClearObject(cpu.x);
            return;
        }
        op_lda(RD8(ZP->game_sub_mode));
        op_cmp(SUB_MODE_ENDING);
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
            return;
        }
        WR8(OBJ->work_d[cpu.x], 0);
        WR8(OBJ->state[cpu.x], 0);
        return;
    }
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (!(cpu.p & P_Z)) {
        WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
        if (!(cpu.p & P_Z)) {
            return;
        }
    }
    op_lda(0x11);
    JSR(0xC257, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_lda(SPRITE_SET_COMPANION);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
    op_txa();
    cpu.a = f_dec(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(ZP->frame_tick));
    op_and(0x02);
    op_ora(RD8(ZP->temp00));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_tay();
    op_lda(RD8(OBJ->y_lo[0]));
    op_clc();
    op_adc(rd8((uint16_t)(RichterCompanionOffsetsY + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_cla();
    op_adc(RD8(OBJ->y_hi[0]));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_lo[0]));
    op_clc();
    op_adc(rd8((uint16_t)(RichterCompanionOffsetsX + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_lo[0]));
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    op_lda(RD8(OBJ->y_lo[0]));
    op_clc();
    op_adc(COMPANION_DROP);
    WR8(ZP->temp02, cpu.a);
    if (cpu.p & P_C) {
        return;
    }
    WR8(ZP->temp03, 0);
    JSR(0xC29E, terrain_read_exit_shim(StageTileRead()));
    op_and(COMPANION_MARK);
    if (cpu.p & P_Z) {
        return;
    }
    op_lda(0x10);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
}

/*
 * @routine 01BD:2F7 RichterCompanionKind3
 * @run 6E:C2F7
 * WHAT IT DOES: Slot 1 or 2 holding kind 3: emptied while the player is in state 9; state 0 takes his
 * sprite flip, goes to his place, sprite set 3, frame $14, timer $80; state 1 stays at his place, marked,
 * and is emptied when the timer runs out.
 * HOW: RichterObjectTake2FE8, RichterObjectAtPlayerPlace, RichterObjectNextState, RichterObjectMarkOn;
 * ClearObject (through its kernel vector).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterCompanionKind3(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HIT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        JSR(0xC320, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
        JSR(0xC323, op_lda(RichterObjectMarkOn(cpu.x)));
        WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
        }
        return;
    }
    JSR(0xC308, take_2fe8_exit_shim(RichterObjectTake2FE8(cpu.x)));
    JSR(0xC30B, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
    op_lda(SPRITE_SET_COMPANION);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x14);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(0x80);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:36E RichterCompanionAtPlayer
 * @run 6E:C36E
 * WHAT IT DOES: Puts a kind 4 companion at the player by the slot and the player's state: the offsets at
 * $C386, two bytes at twice the slot less 1 plus the byte at $C396 by his state.
 * HOW: A jump to RichterObjectAtPlayer with zTemp04 and zTemp06.
 * WAITS: no.
 * Entry: X the slot. Exit: as RichterObjectAtPlayer, Y the offset.
 */
void RichterCompanionAtPlayer(void)
{
    op_txa();
    cpu.a = f_dec(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_ldy(RD8(OBJ->state[0]));
    op_clc();
    op_adc(rd8((uint16_t)(RichterCompanionPlaceByState + cpu.y)));
    op_tay();
    op_lda(rd8((uint16_t)(RichterCompanionPlaces + cpu.y)));
    WR8(ZP->temp04, cpu.a);
    op_lda(rd8((uint16_t)(RichterCompanionPlaces + 1 + cpu.y)));
    WR8(ZP->temp06, cpu.a);
    abi_exit_sum(RichterObjectAtPlayer(cpu.x));
}

/*
 * @routine 01BD:34A RichterCompanionKind4
 * @run 6E:C34A
 * WHAT IT DOES: Slot 1 or 2 holding kind 4: state 0 takes the player's facing, marked, put at him,
 * sprite set 3, animation $1B, state 1; state 1 moves with the scroll and stays at him while the
 * animation runs, emptied at its end.
 * HOW: RichterObjectFromPlayer, RichterObjectMarkOn, RichterCompanionAtPlayer, RichterObjectSetAnimation,
 * RichterObjectNextState; RichterObjectScrollAndMove, RichterObjectAnimationRunning; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last call left them.
 */
void RichterCompanionKind4(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        JSR(0xC369, RichterObjectScrollAndMove());
        jsr_push(0xC36C);
        running = RichterObjectAnimationRunning(cpu.x);
        jsr_pop();
        animation_exit_shim(running);
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
            return;
        }
        RichterCompanionAtPlayer();
        return;
    }
    JSR(0xC354, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    JSR(0xC357, op_lda(RichterObjectMarkOn(cpu.x)));
    JSR(0xC359, RichterCompanionAtPlayer());
    op_lda(SPRITE_SET_COMPANION);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x1B);
    JSR(0xC363, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:338 RichterCompanionByKind
 * @run 6E:C338
 * WHAT IT DOES: Runs slot 1's or 2's handler by its kind: none for 0, RichterCompanionKind1 for 1 and 2,
 * RichterCompanionKind3, RichterCompanionKind4.
 * HOW: The five words at $C340 by OBJ_KIND.
 * WAITS: no.
 * Entry: X the slot. Exit: as the handler leaves them.
 */
void RichterCompanionByKind(void)
{
    uint16_t entry, handler;

    op_lda(RD8(OBJ->kind[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tax();
    entry = (uint16_t)(RichterCompanionKinds + cpu.x);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xC409:
        op_ldx(RichterObjectSlot());
        break;
    case 0xC241:
        RichterCompanionKind1();
        break;
    case 0xC2F7:
        RichterCompanionKind3();
        break;
    case 0xC34A:
        RichterCompanionKind4();
        break;
    default:
        run_routine(handler);
        break;
    }
}

/*
 * @routine 01BD:32C RichterCompanions
 * @run 6E:C32C
 * WHAT IT DOES: Runs slots 1 and 2.
 * HOW: zRichterObject each slot; RichterCompanionByKind.
 * WAITS: no.
 * Entry: nothing. Exit: X 3, N clear, Z and C set; A and Y as the last handler left them.
 */
void RichterCompanions(void)
{
    op_ldx(COMPANION_FIRST);
    do {
        WR8(ZP->richter_object, cpu.x);
        JSR(0xC332, RichterCompanionByKind());
        op_inx();
        op_cpx(COMPANION_END);
    } while (!(cpu.p & P_Z));
}

/*
 * @routine 01BD:3B4 RichterObjectByKind
 * @run 6E:C3B4
 * WHAT IT DOES: Runs an object of slots 3 to 15 by its kind: the 23 handlers at $C3BE.
 * HOW: zRichterObject the slot; the jump by twice OBJ_KIND.
 * WAITS: no.
 * Entry: X the slot. Exit: as the handler leaves them.
 */
void RichterObjectByKind(void)
{
    uint16_t entry, handler;

    WR8(ZP->richter_object, cpu.x);
    op_lda(RD8(OBJ->kind[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tax();
    entry = (uint16_t)(RichterObjectKinds + cpu.x);
    handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    switch (handler) {
    case 0xC409: op_ldx(RichterObjectSlot()); break;
    case 0xC40C: RichterWhipFrame(); break;
    case 0xC57C: RichterKind2Frame(); break;
    case 0xC64D: RichterKind3Frame(); break;
    case 0xCB6C: RichterKind4Frame(); break;
    case 0xC858: RichterKind5Frame(); break;
    case 0xC991: RichterKind6Frame(); break;
    case 0xCB01: RichterKind7Frame(); break;
    case 0xD0BA: RichterKind8Frame(); break;
    case 0xC83B: RichterKind9Frame(); break;
    case 0xCE13: RichterKind10Frame(); break;
    case 0xCF0C: RichterKind11Frame(); break;
    case 0xD05F: RichterKind12Frame(); break;
    case 0xD2DE: RichterKind13Frame(); break;
    case 0xD41A: RichterKind14Frame(); break;
    case 0xD4DA: RichterKind15Frame(); break;
    case 0xD5AD: RichterKind16Frame(); break;
    case 0xD6A5: RichterKind17Frame(); break;
    case 0xD6EC: RichterKind18Frame(); break;
    case 0xD86F: RichterKind19Frame(); break;
    case 0xD945: RichterKind20Frame(); break;
    case 0xDA3D: op_ldx(RichterObjectSlotLast()); break;
    case 0xC56E: RichterKind22Frame(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BD:3A7 RichterObjects
 * @run 6E:C3A7
 * WHAT IT DOES: Richter's objects' frame: slots 1 and 2, then each of slots 3 to 15 by its kind.
 * HOW: RichterCompanions; RichterObjectByKind for X 3 to $0F.
 * WAITS: no.
 * Entry: nothing. Exit: X $10, N clear, Z and C set; A and Y as the last handler left them.
 */
void RichterObjects(void)
{
    JSR(0xC3AA, RichterCompanions());
    op_ldx(3);
    do {
        JSR(0xC3AE, RichterObjectByKind());
        op_inx();
        op_cpx(SLOT_END);
    } while (!(cpu.p & P_Z));
}

/*
 * @routine 01BD:3F5 RichterObjectMarkThenFromPlayer
 * @run 6E:C3F5
 * WHAT IT DOES: Marks the object on, then as RichterObjectFromPlayer.
 * HOW: RichterObjectMarkOn; runs on into RichterObjectFromPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectFromPlayer.
 */
void RichterObjectMarkThenFromPlayer(void)
{
    JSR(0xC3F8, op_lda(RichterObjectMarkOn(cpu.x)));
    from_player_exit(RichterObjectFromPlayer(cpu.x));
}

enum { WHIP_HIT_TIMER = 5, WHIP_ANIMATION_DONE = 0xFF, WHIP_STEP_ATTACK = 1, WHIP_CHARGED = 0x0E,
       WHIP_TOP = 0x28, ENEMY_SLOTS = 0x10, HITBOX_STRUCK = 2, KIND_SPARK = 0x16, FRAME_SPARK = 0x7D,
       ANIM_SPARK = 0x1A, SLOT_SPARK = 4 };

/*
 * @routine 01BD:52B RichterWhipSpark
 * @run 6E:C52B
 * WHAT IT DOES: A spark where the whip struck, unless slot 4 is in use: the middle of the stage's report of
 * the hit (StageHitReport's boxes), kind $16 in the next slot, frame $7D, sprite set 1, animation $1A.
 * HOW: Averages by ADC and ROR; ClearObject (through its kernel vector), RichterObjectSetAnimation; X back.
 * WAITS: no.
 * Entry: X the whip's slot. Exit: slot 4 in use, A its kind, N and Z from it; else X the whip's slot, N and
 * Z from it, A, Y, C and V as RichterObjectSetAnimation left them.
 */
void RichterWhipSpark(void)
{
    op_lda(RD8(STAGEHIT->bytes[2]));
    op_clc();
    op_adc(RD8(STAGEHIT->bytes[0]));
    cpu.a = f_ror(cpu.a);
    op_clc();
    op_adc(RD8(STAGEHIT->bytes[0]));
    cpu.a = f_ror(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(STAGEHIT->bytes[3]));
    op_clc();
    op_adc(RD8(STAGEHIT->bytes[1]));
    cpu.a = f_ror(cpu.a);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->kind[SLOT_SPARK]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_inx();
    JSR(0xC54E, ClearObject(cpu.x));
    op_lda(KIND_SPARK);
    WR8(OBJ->kind[cpu.x], cpu.a);
    op_lda(RD8(ZP->temp00));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(ZP->temp01));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(FRAME_SPARK);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(ANIM_SPARK);
    JSR(0xC56C, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_dex();
}

/*
 * @routine 01BD:512 RichterWhipStrike
 * @run 6E:C512
 * WHAT IT DOES: When the object that tested the whip is an enemy (slot $10 up) with OBJ_HITBOX_ON 2 or more:
 * the spark and PlayerWhipStruck; then OBJ_HIT_BY_SLOT is cleared.
 * HOW: RichterWhipSpark.
 * WAITS: no.
 * Entry: X the whip's slot. Exit: Y the hitter's slot; as the compares or RichterWhipSpark left the rest, A 1
 * after a strike.
 */
void RichterWhipStrike(void)
{
    op_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
    op_cpy(ENEMY_SLOTS);
    if (cpu.p & P_C) {
        op_lda(RD8(OBJ->hitbox_on[cpu.y]));
        op_cmp(HITBOX_STRUCK);
        if (cpu.p & P_C) {
            JSR(0xC522, RichterWhipSpark());
            op_lda(1);
            WR8(PVARS->whip_struck, cpu.a);
        }
    }
    WR8(OBJ->hit_by_slot[cpu.x], 0);
}

/*
 * @routine 01BD:40C RichterWhipFrame
 * @run 6E:C40C
 * WHAT IT DOES: The whip, kind 1: in state 0, hit timer 5, his facing and place, and the animation for his
 * state (the bytes at $C505) started. After: unmarked, his facing, its charge counted and checked, his place,
 * the animation by his state (none for $FF, emptied then); emptied unless he is at step 1 attacking and the
 * animation runs; quickened, its sprite flipped from the charge's 14th frame on, flashed, struck, hitbox
 * on; marked while it is above row $28.
 * HOW: RichterObjectFromPlayer, RichterObjectAtPlayerPlace, RichterObjectStartAnimation; RichterObjectMarkOff,
 * RichterObjectCharge, RichterObjectQuicken, RichterObjectAnimationRunning, RichterChargePads,
 * RichterObjectFlash, RichterWhipStrike, RichterObjectHitboxOn, RichterObjectMarkOn; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterWhipFrame(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (cpu.p & P_Z) {
        op_lda(WHIP_HIT_TIMER);
        WR8(OBJ->work_n[cpu.x], cpu.a);
        JSR(0xC41B, from_player_exit(RichterObjectFromPlayer(cpu.x)));
        JSR(0xC41E, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
        op_ldy(RD8(OBJ->state[0]));
        op_lda(rd8((uint16_t)(RichterWhipAnimations + cpu.y)));
        RichterObjectStartAnimation();
        return;
    }
    JSR(0xC42A, op_lda(RichterObjectMarkOff(cpu.x)));
    JSR(0xC42D, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    WR8(OBJ->work_d[cpu.x], f_inc(RD8(OBJ->work_d[cpu.x])));
    JSR(0xC433, charge_exit(RichterObjectCharge(cpu.x)));
    JSR(0xC436, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
    op_ldy(RD8(OBJ->state[0]));
    op_lda(rd8((uint16_t)(RichterWhipAnimations + cpu.y)));
    op_cmp(WHIP_ANIMATION_DONE);
    WR8(OBJ->animation[cpu.x], cpu.a);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->substate[0]));
    op_cmp(WHIP_STEP_ATTACK);
    if (!(cpu.p & P_Z)) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xC44D, quicken_exit(RichterObjectQuicken(cpu.x)));
    jsr_push(0xC450);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_cmp(WHIP_CHARGED);
    if (cpu.p & P_C) {
        op_sec();
        op_sbc(WHIP_CHARGED);
        op_tay();
        op_lda(rd8((uint16_t)(RichterChargePads + 2 + cpu.y)));
        op_ora(0x80);
        WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    }
    JSR(0xC468, flash_exit(RichterObjectFlash(cpu.x)));
    JSR(0xC46B, RichterWhipStrike());
    JSR(0xC46E, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_cmp(WHIP_TOP);
    if (cpu.p & P_C) {
        return;
    }
    op_lda(RichterObjectMarkOn(cpu.x));
}

/*
 * @routine 01BD:56E RichterKind22Frame
 * @run 6E:C56E
 * WHAT IT DOES: Kind $16, the whip's spark: moves with the scroll until its animation ends, then is emptied.
 * HOW: RichterObjectScrollAndMove, RichterObjectAnimationRunning; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAnimationRunning.
 */
void RichterKind22Frame(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    JSR(0xC573, RichterObjectScrollAndMove());
    jsr_push(0xC576);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
    }
}

/*
 * @routine 01BD:5BD RichterObjectStartAnimation
 * @run 6E:C5BD
 * WHAT IT DOES: Starts an object's animation, sprite set 1, and its next state.
 * HOW: RichterObjectSetAnimation; runs on into RichterObjectNextState.
 * WAITS: no.
 * Entry: A the animation, X object. Exit: as RichterObjectNextState.
 */
void RichterObjectStartAnimation(void)
{
    JSR(0xC5C0, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:637 RichterObjectFallCapped
 * @run 6E:C637
 * WHAT IT DOES: Adds A to an object's Y speed as a fall, held at $0700.
 * HOW: RichterObjectSetFall; at 7 and up the speed is $0700.
 * WAITS: no.
 * Entry: A the pull, X object. Exit: as RichterObjectSetFall, or A 7, N and Z from it, C set.
 */
void RichterObjectFallCapped(void)
{
    op_clc();
    op_adc(RD8(OBJ->vy_lo[cpu.x]));
    JSR(0xC63E, abi_exit_sum(RichterObjectSetFall(cpu.x, cpu.a, (cpu.p & P_C) != 0)));
    if (cpu.p & P_N) {
        return;
    }
    op_cmp(7);
    if (!(cpu.p & P_C)) {
        return;
    }
    op_lda(7);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    WR8(OBJ->vy_lo[cpu.x], 0);
}

/* The leaves of player_leaves.c with the exits their wrappers give. */
static void velocity_x_from_table(void)
{
    abi_ended(set_velocity_from_table_exit_shim(SetVelocityXFromTable(cpu.x, cpu.a)));
}

static void velocity_y_from_table(void)
{
    abi_ended(set_velocity_from_table_exit_shim(SetVelocityYFromTable(cpu.x, cpu.a)));
}

static void negate_if_facing_left(void)
{
    abi_ended(negate_velocity_exit_shim(NegateVelocityXIfFacingLeft(cpu.x)));
}

static void flags_against_four(void)
{
    EndedX r = richter_flags_against_four_exit_shim(RichterFlagsAgainstFour());

    abi_ended(r.end);
    cpu.x = r.x;
}

static void state_doubled(void)
{
    EndedX r = richter_state_doubled_exit_shim(RichterStateDoubled());

    abi_ended(r.end);
    cpu.x = r.x;
}

/* A jump through a kind's table of state handlers: the two bytes of the word at the table plus X. */
static uint16_t state_handler(uint16_t table)
{
    uint16_t entry = (uint16_t)(table + cpu.x);

    return (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
}

enum { THROW_SOUND_EVERY = 0x07, SOUND_THROWN_ITEM = 0x06, SOUND_ITEM_DEFLECTED = 0x11,
       PLAYER_STATE_HURT = 9 };

/*
 * @routine 01BD:5C9 RichterKind2Fly
 * @run 6E:C5C9
 * WHAT IT DOES: Kind 2 in flight: with its hitbox on, sound 6 every eighth wave count; deflected (OBJ_WORK_P
 * 2), hitbox and mark off, the bounce's speeds (X entry 7 by facing, Y entry 2), animation $1C and sound $11;
 * animation 8 once its own ends; hitbox by frame; emptied below the map or off the screen's sides; else moved
 * with the scroll and falling at $44 a frame to $0700.
 * HOW: zWaveCounter; SoundCommand; SetVelocityXFromTable, NegateVelocityXIfFacingLeft, SetVelocityYFromTable;
 * RichterObjectSetAnimation, RichterObjectAnimationRunning, RichterObjectHitboxByFrame; ClearObject;
 * RichterObjectScrollAndMove; runs on into RichterObjectFallCapped.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last instruction or call left them.
 */
void RichterKind2Fly(void)
{
    AnimationStep running;

    op_lda(RD8(OBJ->hitbox_on[cpu.x]));
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(ZP->wave_counter));
        op_and(THROW_SOUND_EVERY);
        if (cpu.p & P_Z) {
            op_lda(SOUND_THROWN_ITEM);
            JSR(0xC5D9, sound_exit(cpu.a));
        }
    }
    op_lda(RD8(OBJ->work_p[cpu.x]));
    op_cmp(HITBOX_STRUCK);
    if (cpu.p & P_Z) {
        WR8(OBJ->hitbox_on[cpu.x], 0);
        WR8(OBJ->work_p[cpu.x], 0);
        op_lda(7);
        JSR(0xC5EB, velocity_x_from_table());
        JSR(0xC5EE, negate_if_facing_left());
        op_lda(2);
        JSR(0xC5F3, velocity_y_from_table());
        op_lda(0x1C);
        JSR(0xC5F8, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
        op_lda(SOUND_ITEM_DEFLECTED);
        JSR(0xC5FD, sound_exit(cpu.a));
    }
    jsr_push(0xC600);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        op_lda(8);
        JSR(0xC607, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    }
    JSR(0xC60A, hitbox_frame_exit(RichterObjectHitboxForFrame(cpu.x)));
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_cmp(1);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_cmp(0xFF);
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_cmp(0xF0);
        if (cpu.p & P_C) {
            ClearObject(cpu.x);
            return;
        }
    }
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(OBJ->x_lo[cpu.x]));
        op_cmp(0x10);
        if (cpu.p & P_C) {
            op_cmp(0xF0);
            if (!(cpu.p & P_C)) {
                ClearObject(cpu.x);
                return;
            }
        }
    }
    JSR(0xC635, RichterObjectScrollAndMove());
    op_lda(0x44);
    RichterObjectFallCapped();
}

/*
 * @routine 01BD:57C RichterKind2Frame
 * @run 6E:C57C
 * WHAT IT DOES: Kind 2, an item thrown in an arc: in flight, as RichterKind2Fly; at state 0, emptied while
 * the player is hurt; else, once RichterFlagsAgainstFour lets it, his facing, his place offset ($14, $E8),
 * the throw's speeds (X entry 0 by facing, Y entry 1), hit timer 8, hitbox on, animation 7 started.
 * HOW: RichterFlagsAgainstFour; RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft, SetVelocityYFromTable; runs on into RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind2Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        RichterKind2Fly();
        return;
    }
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xC595, flags_against_four());
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xC59B, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(0x14);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xE8);
    WR8(ZP->temp06, cpu.a);
    JSR(0xC5A6, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(0);
    JSR(0xC5AB, velocity_x_from_table());
    JSR(0xC5AE, negate_if_facing_left());
    op_lda(1);
    JSR(0xC5B3, velocity_y_from_table());
    op_lda(8);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    WR8(OBJ->hitbox_on[cpu.x], f_inc(RD8(OBJ->hitbox_on[cpu.x])));
    op_lda(7);
    RichterObjectStartAnimation();
}

/*
 * @routine 01BD:815 RichterKind3Trail
 * @run 6E:C815
 * WHAT IT DOES: Every eighth frame by OBJ_WORK_F, a kind 9 object at this one's place in the first free of
 * the next two slots.
 * HOW: ClearObjectInYAndCopyPlace.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last instruction or call left them; Y the slot looked at.
 */
void RichterKind3Trail(void)
{
    WR8(OBJ->work_f[cpu.x], f_inc(RD8(OBJ->work_f[cpu.x])));
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_cmp(8);
    if (!(cpu.p & P_Z)) {
        return;
    }
    WR8(OBJ->work_f[cpu.x], 0);
    op_txa();
    op_tay();
    op_iny();
    op_lda(RD8(OBJ->kind[cpu.y]));
    if (!(cpu.p & P_Z)) {
        op_iny();
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (!(cpu.p & P_Z)) {
            return;
        }
    }
    JSR(0xC835, abi_ended(clear_in_y_exit_shim(ClearObjectInYAndCopyPlace(cpu.x, cpu.y))));
    op_lda(9);
    WR8(OBJ->kind[cpu.y], cpu.a);
}

/*
 * @routine 01BD:794 RichterKind3Count
 * @run 6E:C794
 * WHAT IT DOES: Counts kind 3's OBJ_WORK_E up (emptied at $FF), its animation the byte at $C7FF by the count's
 * high nibble; sound 7 every sixteenth count, or with animation $0B sound 8 every fourth.
 * HOW: Tail jumps to SoundCommand; ClearObject at $FF.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last instruction or call left them.
 */
void RichterKind3Count(void)
{
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_cmp(0xFF);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    WR8(OBJ->work_e[cpu.x], f_inc(RD8(OBJ->work_e[cpu.x])));
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind3Animations + cpu.y)));
    WR8(OBJ->animation[cpu.x], cpu.a);
    op_cmp(0x0B);
    if (cpu.p & P_Z) {
        op_lda(RD8(OBJ->work_e[cpu.x]));
        op_and(0x03);
        if (!(cpu.p & P_Z)) {
            return;
        }
        op_lda(8);
        sound_exit(cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_and(0x0F);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(7);
    sound_exit(cpu.a);
}

/*
 * @routine 01BD:6EA RichterKind3Move
 * @run 6E:C6EA
 * WHAT IT DOES: Kind 3 on its way: the trail, the animation, the hitbox, moved with the scroll; its X speed
 * eased by the word at $C80F by facing until it is the byte at $C813, and the next state when the speed's
 * sign turns.
 * HOW: RichterKind3Trail, RichterObjectAnimationTick, RichterObjectHitboxOn, RichterObjectScrollAndMove.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last instruction left them.
 */
void RichterKind3Move(void)
{
    JSR(0xC6ED, RichterKind3Trail());
    JSR(0xC6F0, animation_exit_shim(RichterObjectAnimationTick(cpu.x)));
    JSR(0xC6F3, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    JSR(0xC6F6, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_tay();
    op_lda(RD8(OBJ->vx_hi[cpu.x]));
    op_cmp(rd8((uint16_t)(RichterKind3TopSpeeds + cpu.y)));
    if (cpu.p & P_Z) {
        return;
    }
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind3Eases + cpu.y)));
    op_clc();
    op_adc(RD8(OBJ->vx_lo[cpu.x]));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->vx_hi[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind3Eases + 1 + cpu.y)));
    op_adc(RD8(OBJ->vx_hi[cpu.x]));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_eor(RD8(ZP->temp00));
    op_and(0x80);
    if (cpu.p & P_Z) {
        return;
    }
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
}

/*
 * @routine 01BD:6C0 RichterKind3Out
 * @run 6E:C6C0
 * WHAT IT DOES: Kind 3's first leg: deflected (OBJ_WORK_P 2), its count to $30, timer $40, X speed the byte at
 * $C7C7 by facing (both bytes), sound $11; then the count, and as RichterKind3Move.
 * HOW: SoundCommand; RichterKind3Count; runs on into RichterKind3Move.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterKind3Move.
 */
void RichterKind3Out(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->work_p[cpu.x]));
    op_cmp(HITBOX_STRUCK);
    if (cpu.p & P_Z) {
        op_lda(0x30);
        WR8(OBJ->work_e[cpu.x], cpu.a);
        op_lda(0x40);
        WR8(OBJ->anim_timer[cpu.x], cpu.a);
        op_ldy(RD8(OBJ->facing_left[cpu.x]));
        op_lda(rd8((uint16_t)(RichterKind3DeflectSpeeds + cpu.y)));
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        WR8(OBJ->work_p[cpu.x], 0);
        op_lda(SOUND_ITEM_DEFLECTED);
        JSR(0xC6E7, sound_exit(cpu.a));
    }
    JSR(0xC6EA, RichterKind3Count());
    RichterKind3Move();
}

/*
 * @routine 01BD:729 RichterKind3Back
 * @run 6E:C729
 * WHAT IT DOES: Kind 3 coming back: deflected when its timer is out, the bounce (X speed by the facing turned,
 * Y entry 2), timer $60, animation $0A, hitbox off, sound $11, the next state; otherwise the count, and
 * emptied when it meets the player or reaches the screen's edge it was thrown from; else as
 * RichterKind3Move.
 * HOW: The words at $C98D; SetVelocityYFromTable; SoundCommand; RichterObjectNextState; RichterKind3Count;
 * ObjectTouchesPlayerBox; the X high bytes at $C7FD; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind3Back(void)
{
    bool deflect = false;

    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (cpu.p & P_Z) {
        deflect = true;
    } else {
        WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
        deflect = (cpu.p & P_Z) != 0;
    }
    if (deflect) {
        op_lda(RD8(OBJ->work_p[cpu.x]));
        op_cmp(HITBOX_STRUCK);
        if (cpu.p & P_Z) {
            op_lda(RD8(OBJ->facing_left[cpu.x]));
            cpu.a = f_inc(cpu.a);
            op_and(0x01);
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(RichterBounceSpeedsX + cpu.y)));
            WR8(OBJ->vx_lo[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(RichterBounceSpeedsX + 1 + cpu.y)));
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
            op_lda(2);
            JSR(0xC755, velocity_y_from_table());
            op_lda(0x60);
            WR8(OBJ->anim_timer[cpu.x], cpu.a);
            op_lda(0x0A);
            WR8(OBJ->animation[cpu.x], cpu.a);
            WR8(OBJ->hitbox_on[cpu.x], 0);
            op_lda(SOUND_ITEM_DEFLECTED);
            JSR(0xC767, sound_exit(cpu.a));
            abi_nz(RichterObjectNextState(cpu.x));
            return;
        }
    }
    WR8(OBJ->work_p[cpu.x], 0);
    JSR(0xC770, RichterKind3Count());
    JSR(0xC773, abi_ended(object_touches_player_box_exit_shim(ObjectTouchesPlayerBox(cpu.x))));
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_ldy(RD8(OBJ->facing_left[cpu.x]));
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    op_cmp(rd8((uint16_t)(RichterKind3EdgeScreens + cpu.y)));
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    RichterKind3Move();
}

/*
 * @routine 01BD:786 RichterKind3Drop
 * @run 6E:C786
 * WHAT IT DOES: Kind 3 after a deflection: falls as RichterObjectFall38, emptied when its timer runs out.
 * HOW: RichterObjectFall38; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: counting, N and Z from the timer.
 */
void RichterKind3Drop(void)
{
    JSR(0xC789, RichterObjectFall38());
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
    }
}

/*
 * @routine 01BD:68F RichterKind3Rise
 * @run 6E:C68F
 * WHAT IT DOES: Kind 3's second state: its count up; emptied while the player is hurt; animated, moved with
 * the scroll, held $11 above his Y; the next state when the count reaches 2.
 * HOW: RichterObjectAnimationTick, RichterObjectScrollAndMove, RichterObjectNextState; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind3Rise(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->work_e[cpu.x], f_inc(RD8(OBJ->work_e[cpu.x])));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xC6A1, animation_exit_shim(RichterObjectAnimationTick(cpu.x)));
    JSR(0xC6A4, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->y_lo[0]));
    op_clc();
    op_adc(0xEF);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_hi[0]));
    op_adc(0xFF);
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_cmp(2);
    if (!(cpu.p & P_Z)) {
        return;
    }
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:65D RichterKind3Start
 * @run 6E:C65D
 * WHAT IT DOES: Kind 3's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, his
 * facing, his place offset ($0A, $EF), X speed entry 1 by facing, hit timer 9, animation 9 started.
 * HOW: RichterFlagsAgainstFour, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft; a jump to RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind3Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xC66C, flags_against_four());
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xC672, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(0x0A);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xEF);
    WR8(ZP->temp06, cpu.a);
    JSR(0xC67D, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(1);
    JSR(0xC682, velocity_x_from_table());
    JSR(0xC685, negate_if_facing_left());
    op_lda(9);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    op_lda(9);
    RichterObjectStartAnimation();
}

/*
 * @routine 01BD:64D RichterKind3Frame
 * @run 6E:C64D
 * WHAT IT DOES: Kind 3: runs its state's handler.
 * HOW: RichterStateDoubled; the five words at $C653.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind3Frame(void)
{
    uint16_t handler;

    JSR(0xC650, state_doubled());
    handler = state_handler(RichterKind3States);
    switch (handler) {
    case 0xC65D: RichterKind3Start(); break;
    case 0xC68F: RichterKind3Rise(); break;
    case 0xC6C0: RichterKind3Out(); break;
    case 0xC729: RichterKind3Back(); break;
    case 0xC786: RichterKind3Drop(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BE:03B RichterKind9Frame
 * @run 6E:C83B
 * WHAT IT DOES: Kind 9, kind 3's trail: state 0 takes the player's facing, Y speed entry 3, animation $0C
 * started; then moved with the scroll until its animation ends, and emptied.
 * HOW: RichterObjectFromPlayer, SetVelocityYFromTable, RichterObjectStartAnimation;
 * RichterObjectScrollAndMove, RichterObjectAnimationWait; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind9Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (cpu.p & P_Z) {
        JSR(0xC845, from_player_exit(RichterObjectFromPlayer(cpu.x)));
        op_lda(3);
        JSR(0xC84A, velocity_y_from_table());
        op_lda(0x0C);
        RichterObjectStartAnimation();
        return;
    }
    JSR(0xC852, RichterObjectScrollAndMove());
    if (object_wait(0xC855)) {
        return;
    }
    ClearObject(cpu.x);
}

enum { KIND_THREE_WAY = 5, KIND5_FIRST_SLOTS_A = 5, KIND5_FIRST_SLOTS_B = 8, KIND5_FIRST_SLOTS_C = 0x0B,
       SOUND_KIND5 = 0x07, FRAME_KIND5 = 0x49, FRAME_KIND5_STOPPED = 0x3D, KIND5_STOP_FRAMES = 6,
       KIND5_STUCK_FRAMES = 4, KIND5_BOUNCE_STATE = 3, FALL_PULL_38 = 0x38 };

/*
 * @routine 01BE:0B6 RichterKind5Throw
 * @run 6E:C8B6
 * WHAT IT DOES: Throws a kind 5: sound 7, the player's facing, his place offset ($0A, $F5), X speed entry 2
 * by facing, hit timer 6, animation 6 started, frame $49.
 * HOW: SoundCommand, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft, RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: X the slot. Exit: A $49, N and Z from the next state; the rest as RichterObjectStartAnimation.
 */
void RichterKind5Throw(void)
{
    op_lda(SOUND_KIND5);
    JSR(0xC8BB, sound_exit(cpu.a));
    JSR(0xC8BE, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(0x0A);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xF5);
    WR8(ZP->temp06, cpu.a);
    JSR(0xC8C9, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(2);
    JSR(0xC8CE, velocity_x_from_table());
    JSR(0xC8D1, negate_if_facing_left());
    op_lda(6);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    JSR(0xC8D9, RichterObjectStartAnimation());
    op_lda(FRAME_KIND5);
    WR8(OBJ->frame[cpu.x], cpu.a);
}

/* $C8A6 on: kind 5's Y speed by its place among the three (the words at $C866), then the throw. */
static void kind5_aim(void)
{
    op_txa();
    op_sec();
    op_sbc(KIND5_FIRST_SLOTS_A);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind5SpeedsY + cpu.y)));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind5SpeedsY + 1 + cpu.y)));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    RichterKind5Throw();
}

/*
 * @routine 01BE:078 RichterKind5Spread
 * @run 6E:C878
 * WHAT IT DOES: The first of a kind 5 throw (slot 5, 8 or $0B) makes the next two slots kind 5 too, then aims
 * and throws as RichterKind5Start.
 * HOW: OBJ_KIND; RichterKind5Start's aim from $C8A6.
 * WAITS: no.
 * Entry: X the slot. Exit: as RichterKind5Throw.
 */
void RichterKind5Spread(void)
{
    op_inx();
    op_lda(KIND_THREE_WAY);
    WR8(OBJ->kind[cpu.x], cpu.a);
    op_inx();
    WR8(OBJ->kind[cpu.x], cpu.a);
    op_dex();
    op_dex();
    kind5_aim();
}

/*
 * @routine 01BE:086 RichterKind5Start
 * @run 6E:C886
 * WHAT IT DOES: Kind 5's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, the
 * first of three (slots 5, 8, $0B) spreads to the next two; each is aimed by its place and thrown.
 * HOW: RichterFlagsAgainstFour; RichterKind5Spread; the words at $C866; runs on into RichterKind5Throw.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind5Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xC895, flags_against_four());
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_cpx(KIND5_FIRST_SLOTS_A);
    if (!(cpu.p & P_Z)) {
        op_cpx(KIND5_FIRST_SLOTS_B);
    }
    if (!(cpu.p & P_Z)) {
        op_cpx(KIND5_FIRST_SLOTS_C);
    }
    if (cpu.p & P_Z) {
        RichterKind5Spread();
        return;
    }
    kind5_aim();
}

/*
 * @routine 01BE:0DF RichterKind5Fly
 * @run 6E:C8DF
 * WHAT IT DOES: Kind 5 (and kind 12) in flight: emptied off range; while the object that tested it is not
 * striking (OBJ_HITBOX_ON 2) or nothing hit it, moved with the scroll, hitbox on, OBJ_WORK_P cleared. Hit
 * (OBJ_WORK_P 1): stopped, frame $3D for 6 frames, the next state, hitbox off, or for 4 frames with it on
 * when the hitter's is. Deflected (2): hitbox off, the bounce's X speed by facing, animation $0D, Y entry 2,
 * sound $11, state 3.
 * HOW: ClearObjectIfOffRange; OBJ_HIT_BY_SLOT; RichterObjectScrollAndMove, RichterObjectHitboxOn,
 * RichterObjectStopX; the words at $C98D; RichterObjectSetAnimation, SetVelocityYFromTable, SoundCommand.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind5Fly(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xC8E4, abi_ended(clear_object_exit_shim(ClearObjectIfOffRange(cpu.x))));
    op_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
    op_lda(RD8(OBJ->hitbox_on[cpu.y]));
    op_cmp(HITBOX_STRUCK);
    if (cpu.p & P_Z) {
        op_lda(RD8(OBJ->work_p[cpu.x]));
        op_cmp(1);
        if (cpu.p & P_Z) {
            JSR(0xC906, RichterObjectStopX(cpu.x));
            op_lda(FRAME_KIND5_STOPPED);
            WR8(OBJ->frame[cpu.x], cpu.a);
            op_lda(KIND5_STOP_FRAMES);
            WR8(OBJ->hurt_timer[cpu.x], cpu.a);
            WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
            WR8(OBJ->hitbox_on[cpu.x], 0);
            op_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
            op_lda(RD8(OBJ->hitbox_on[cpu.y]));
            if (cpu.p & P_Z) {
                return;
            }
            op_lda(1);
            WR8(OBJ->hitbox_on[cpu.x], cpu.a);
            op_lda(KIND5_STUCK_FRAMES);
            WR8(OBJ->hurt_timer[cpu.x], cpu.a);
            return;
        }
        op_cmp(2);
        if (cpu.p & P_Z) {
            WR8(OBJ->hitbox_on[cpu.x], 0);
            op_lda(RD8(OBJ->facing_left[cpu.x]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(RichterBounceSpeedsX + cpu.y)));
            WR8(OBJ->vx_lo[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(RichterBounceSpeedsX + 1 + cpu.y)));
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
            op_lda(0x0D);
            JSR(0xC942, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
            op_lda(2);
            JSR(0xC947, velocity_y_from_table());
            op_lda(SOUND_ITEM_DEFLECTED);
            JSR(0xC94C, sound_exit(cpu.a));
            op_lda(KIND5_BOUNCE_STATE);
            WR8(OBJ->state[cpu.x], cpu.a);
            return;
        }
    }
    JSR(0xC8FC, RichterObjectScrollAndMove());
    JSR(0xC8FF, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    WR8(OBJ->work_p[cpu.x], 0);
}

/*
 * @routine 01BE:152 RichterKind5Stuck
 * @run 6E:C952
 * WHAT IT DOES: Kind 5 after a hit: moved with the scroll, emptied when its timer runs out.
 * HOW: RichterObjectScrollAndMove; ClearObjectIfOffRange's jump to ClearObject ($C98A).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: counting, N and Z from the timer.
 */
void RichterKind5Stuck(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xC957, RichterObjectScrollAndMove());
    WR8(OBJ->hurt_timer[cpu.x], f_dec(RD8(OBJ->hurt_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    ClearObject(cpu.x);
}

/*
 * @routine 01BE:160 RichterObjectFall38
 * @run 6E:C960
 * WHAT IT DOES: An object falls: emptied off range, moved with the scroll, $38 more Y speed, animated.
 * HOW: ClearObjectIfOffRange, RichterObjectScrollAndMove, RichterObjectSetFall, a tail jump to
 * RichterObjectAnimationTick.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAnimationTick.
 */
void RichterObjectFall38(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xC965, abi_ended(clear_object_exit_shim(ClearObjectIfOffRange(cpu.x))));
    JSR(0xC968, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->vy_lo[cpu.x]));
    op_clc();
    op_adc(FALL_PULL_38);
    JSR(0xC971, abi_exit_sum(RichterObjectSetFall(cpu.x, cpu.a, (cpu.p & P_C) != 0)));
    animation_exit_shim(RichterObjectAnimationTick(cpu.x));
}

/*
 * @routine 01BE:058 RichterKind5Frame
 * @run 6E:C858
 * WHAT IT DOES: Kind 5: runs its state's handler.
 * HOW: RichterStateDoubled; the four words at $C85E.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind5Frame(void)
{
    uint16_t handler;

    JSR(0xC85B, state_doubled());
    handler = state_handler(RichterKind5States);
    switch (handler) {
    case 0xC886: RichterKind5Start(); break;
    case 0xC8DF: RichterKind5Fly(); break;
    case 0xC952: RichterKind5Stuck(); break;
    case 0xC960: RichterObjectFall38(); break;
    default: run_routine(handler); break;
    }
}

enum { KIND_STOPWATCH_FIRST = 4, SOUND_KIND6 = 0x5F, KIND6 = 6, KIND6_PLACE_FIRST = 4, ANIM_KIND6 = 0x16,
       KIND6_SPIN_END = 0x73, ANIM_KIND6_SPUN = 0x0E, ANIM_KIND6_END = 0x0F, KIND6_LAST_SLOT = 0x0F,
       KIND6_WAIT_STATE = 5, KIND6_END_STATE = 6, ANIM_KIND6_FADE = 0x10, FADE_OUT = 2 };

/*
 * @routine 01BE:1A5 RichterKind6Start
 * @run 6E:C9A5
 * WHAT IT DOES: Kind 6's state 0: the first (slot 4) sounds $5F, sets TimeSlowMode, zFadeCount and
 * zFadeActive, and makes slots 4 to 15 kind 6; each takes its place from the four bytes at $CA87 by four
 * times its slot less 4 (the first from its own), animation $16, sprite set 1, its circling start, the next
 * state.
 * HOW: SoundCommand; RichterObjectSetAnimation; RichterKind6CircleStart; a jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectNextState.
 */
void RichterKind6Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_txa();
    op_sec();
    op_sbc(KIND_STOPWATCH_FIRST);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_cmp(4);
    if (cpu.p & P_Z) {
        op_lda(SOUND_KIND6);
        JSR(0xC9B6, sound_exit(cpu.a));
        op_lda(1);
        WR8(PVARS->time_slow_mode, cpu.a);
        WR8(ZP->fade_count, cpu.a);
        WR8(ZP->fade_active, cpu.a);
        op_ldy(KIND6_PLACE_FIRST);
        op_lda(KIND6);
        do {
            WR8(OBJ->kind[cpu.y], cpu.a);
            op_iny();
            op_cpy(SLOT_END);
        } while (!(cpu.p & P_Z));
        op_lda(4);
    }
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind6Places + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 1 + cpu.y)));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 2 + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 3 + cpu.y)));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(ANIM_KIND6);
    JSR(0xC9EB, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    JSR(0xC9F3, RichterKind6CircleStart());
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:1F6 RichterKind6Circle
 * @run 6E:C9F6
 * WHAT IT DOES: Kind 6 circling: a step of the circle, the animation; when the step count reaches $73,
 * animation $0E and the next state.
 * HOW: RichterKind6CircleStep, RichterObjectAnimationTick; RichterKind6Count's RTS ($CA36) before then.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind6Circle(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xC9FB, RichterKind6CircleStep());
    JSR(0xC9FE, animation_exit_shim(RichterObjectAnimationTick(cpu.x)));
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    op_cmp(KIND6_SPIN_END);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(ANIM_KIND6_SPUN);
    WR8(OBJ->animation[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:20D RichterKind6Hold
 * @run 6E:CA0D
 * WHAT IT DOES: Kind 6, once its animation ends: the two timers by its slot (the words at $CAB7), the next
 * state.
 * HOW: RichterObjectAnimationWait; a jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: waiting, as RichterObjectAnimationWait; else as
 * RichterObjectNextState.
 */
void RichterKind6Hold(void)
{
    op_ldx(RD8(ZP->richter_object));
    if (object_wait(0xCA12)) {
        return;
    }
    op_txa();
    op_sec();
    op_sbc(KIND_STOPWATCH_FIRST);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind6Timers + cpu.y)));
    WR8(OBJ->hurt_timer[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Timers + 1 + cpu.y)));
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:227 RichterKind6Count
 * @run 6E:CA27
 * WHAT IT DOES: Kind 6 waiting out its two timers, the second counted down each time the first runs out;
 * then animation $0F and the next state.
 * HOW: OBJ_HURT_TIMER and OBJ_ANIM_TIMER; RichterObjectSetAnimation, RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: counting, N and Z from the count; else as RichterObjectNextState.
 */
void RichterKind6Count(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->hurt_timer[cpu.x], f_dec(RD8(OBJ->hurt_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (!(cpu.p & P_Z)) {
        WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
        return;
    }
    op_lda(ANIM_KIND6_END);
    JSR(0xCA3C, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:23F RichterKind6Wait
 * @run 6E:CA3F
 * WHAT IT DOES: Kind 6: the next state once its animation ends.
 * HOW: RichterObjectAnimationWait, a jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait or RichterObjectNextState.
 */
void RichterKind6Wait(void)
{
    op_ldx(RD8(ZP->richter_object));
    if (object_wait(0xCA44)) {
        return;
    }
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:247 RichterKind6Gather
 * @run 6E:CA47
 * WHAT IT DOES: Kind 6's state 5: the last (slot $0F), once slots 4 to 15 have all reached state 5, puts them
 * all in state 6 with animation $10 and sprite set 3.
 * HOW: OBJ_STATE of each; RichterObjectSetAnimation with X pushed.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind6Gather(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_cpx(KIND6_LAST_SLOT);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_ldy(3);
    for (;;) {
        op_iny();
        op_cpy(SLOT_END);
        if (cpu.p & P_Z) {
            break;
        }
        op_lda(RD8(OBJ->state[cpu.y]));
        op_cmp(KIND6_WAIT_STATE);
        if (!(cpu.p & P_Z)) {
            return;
        }
    }
    op_phx();
    op_ldx(KIND6_PLACE_FIRST);
    do {
        op_lda(KIND6_END_STATE);
        WR8(OBJ->state[cpu.x], cpu.a);
        op_lda(ANIM_KIND6_FADE);
        JSR(0xCA69, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
        op_lda(SPRITE_SET_COMPANION);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_inx();
        op_cpx(SLOT_END);
    } while (!(cpu.p & P_Z));
    op_plx();
}

/*
 * @routine 01BE:275 RichterKind6Fade
 * @run 6E:CA75
 * WHAT IT DOES: Kind 6's last state: rises a pixel a frame until its animation ends; then zFadeCount 2,
 * TimeSlowMode off, emptied.
 * HOW: RichterObjectAnimationWait; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait, or A 2 and the rest as ClearObject
 * keeps them.
 */
void RichterKind6Fade(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->y_lo[cpu.x], f_dec(RD8(OBJ->y_lo[cpu.x])));
    if (object_wait(0xCA7D)) {
        return;
    }
    op_lda(FADE_OUT);
    WR8(ZP->fade_count, cpu.a);
    WR8(PVARS->time_slow_mode, 0);
    ClearObject(cpu.x);
}

/*
 * @routine 01BE:191 RichterKind6Frame
 * @run 6E:C991
 * WHAT IT DOES: Kind 6: runs its state's handler.
 * HOW: RichterStateDoubled; the seven words at $C997.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind6Frame(void)
{
    uint16_t handler;

    JSR(0xC994, state_doubled());
    handler = state_handler(RichterKind6States);
    switch (handler) {
    case 0xC9A5: RichterKind6Start(); break;
    case 0xC9F6: RichterKind6Circle(); break;
    case 0xCA0D: RichterKind6Hold(); break;
    case 0xCA27: RichterKind6Count(); break;
    case 0xCA3F: RichterKind6Wait(); break;
    case 0xCA47: RichterKind6Gather(); break;
    case 0xCA75: RichterKind6Fade(); break;
    default: run_routine(handler); break;
    }
}

enum { KIND7_HIT_FRAMES = 0x0D, KIND7_STEP = 5, PLAYER_STEP_ATTACK = 1, KIND_COMPANION3 = 3 };

/*
 * @routine 01BE:2CF RichterKind7End
 * @run 6E:CACF
 * WHAT IT DOES: Kind 7 ends: if a stage object caught it (PlayerKind7Caught), that is cleared with the item in
 * hand; else, unless it struck (OBJ_WORK_D), a kind 3 companion is made sure of; if it struck, the flag is
 * cleared and a kind 3 companion in slot 1 or 2 is emptied. Then it is emptied.
 * HOW: RichterClaimKind3Slot; ClearObjectInY (through its kernel vector); ClearObject.
 * WAITS: no.
 * Entry: X the slot. Exit: as the last call left them.
 */
void RichterKind7End(void)
{
    op_lda(RD8(PVARS2->kind7_caught));
    if (!(cpu.p & P_Z)) {
        WR8(PVARS2->kind7_caught, 0);
        WR8(ZP->field93, 0);
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->work_d[cpu.x]));
    if (cpu.p & P_Z) {
        JSR(0xCADC, RichterClaimKind3Slot());
        ClearObject(cpu.x);
        return;
    }
    WR8(PVARS2->kind7_caught, 0);
    op_ldy(1);
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_cmp(KIND_COMPANION3);
    if (!(cpu.p & P_Z)) {
        op_iny();
        op_lda(RD8(OBJ->kind[cpu.y]));
        op_cmp(KIND_COMPANION3);
    }
    if (cpu.p & P_Z) {
        JSR(0xCAF7, ClearObjectInY(cpu.y));
    }
    ClearObject(cpu.x);
}

/*
 * @routine 01BE:301 RichterKind7Frame
 * @run 6E:CB01
 * WHAT IT DOES: Kind 7, held by the player while he attacks: emptied while he is hurt; ended when his step
 * is not 1; at state 0, once his animation reaches step 5, the frame for his state (the bytes at $CB5F),
 * sprite set 1, his facing, hitbox on, hit timer $0D, his place, the next state; then the frame and his place
 * each frame, and OBJ_WORK_D set the first time an enemy (slot $10 up) with OBJ_HITBOX_ON 2 or more tested it.
 * HOW: RichterKind7End; RichterObjectFromPlayer, RichterObjectHitboxOn, RichterObjectAtPlayerPlace,
 * RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind7Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->substate[0]));
    op_cmp(PLAYER_STEP_ATTACK);
    if (!(cpu.p & P_Z)) {
        RichterKind7End();
        return;
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    if (cpu.p & P_Z) {
        op_lda(RD8(OBJ->animation_at[0]));
        op_cmp(KIND7_STEP);
        if (!(cpu.p & P_Z)) {
            return;
        }
        op_ldy(RD8(OBJ->state[0]));
        op_lda(rd8((uint16_t)(RichterKind7Frames + cpu.y)));
        WR8(OBJ->frame[cpu.x], cpu.a);
        op_lda(1);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        JSR(0xCB2F, from_player_exit(RichterObjectFromPlayer(cpu.x)));
        JSR(0xCB32, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
        op_lda(KIND7_HIT_FRAMES);
        WR8(OBJ->work_n[cpu.x], cpu.a);
        JSR(0xCB3A, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    op_ldy(RD8(OBJ->state[0]));
    op_lda(rd8((uint16_t)(RichterKind7Frames + cpu.y)));
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    if (cpu.p & P_Z) {
        op_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
        op_cpy(ENEMY_SLOTS);
        if (cpu.p & P_C) {
            op_lda(RD8(OBJ->hitbox_on[cpu.y]));
            op_cmp(HITBOX_STRUCK);
            if (cpu.p & P_C) {
                WR8(OBJ->work_d[cpu.x], f_inc(RD8(OBJ->work_d[cpu.x])));
            }
        }
    }
    abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x));
}

/*
 * @routine 01C0:045 RichterClaimKind3Slot
 * @run 6E:D845
 * WHAT IT DOES: MariaClaimKind3Slot in Richter's overlay, the same bytes at his address: a companion of kind
 * 3 in slot 1 when it is empty; nothing when slot 1 holds kind 3; else slot 2 when it is empty.
 * HOW: MariaClaimKind3Slot's body with his JSR's return.
 * WAITS: no.
 * Entry, exit: as MariaClaimKind3Slot.
 */
void RichterClaimKind3Slot(void)
{
    regs_exit(claim_kind3_slot(cpu.x, 0xD85C));
}

enum { KIND4_FLOOR_MARK = 0x20, KIND4_LANDED_BITS = 0x31, KIND4_WALL_BITS = 0x22, KIND4_STEP_BIT = 0x10,
       SOUND_KIND4_LAND = 0x05, FRAME_KIND4_LAND = 0x56, KIND4_LAND_FRAMES = 3, KIND4_BURN_STATE = 4,
       ANIM_KIND4_RUN = 0x12, ANIM_KIND4_SPLASH = 0x13, KIND4_FLAME_STEP = 0x0B, KIND_FLAME = 0x0A,
       KIND4_FLAMES = 2, KIND4_CLIMB_PULL = 0x28 };

/*
 * @routine 01BE:3CC RichterObjectProbe
 * @run 6E:CBCC
 * WHAT IT DOES: Reads the stage's terrain at a point offset from an object (zTemp04-05 across, zTemp06-07
 * down); a point past the bottom of the first screen down reads as nothing.
 * HOW: The point to zTemp00-03 (Y first; a carry out of its low byte is past the bottom); a tail jump to the
 * stage overlay's read at $8006 (StageTileRead).
 * WAITS: no.
 * Entry: X the object, zTemp04-07 the offset. Exit: past the bottom, A 0, N and Z from the add, C set, V from
 * it. Else as the stage's read.
 */
void RichterObjectProbe(void)
{
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(RD8(ZP->temp06));
    WR8(ZP->temp02, cpu.a);
    if (cpu.p & P_C) {
        op_cla();
        return;
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(RD8(ZP->temp07));
    WR8(ZP->temp03, cpu.a);
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_clc();
    op_adc(RD8(ZP->temp04));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    op_adc(RD8(ZP->temp05));
    WR8(ZP->temp01, cpu.a);
    terrain_read_exit_shim(StageTileRead());
}

/*
 * @routine 01BE:3BE RichterObjectProbeBelow
 * @run 6E:CBBE
 * WHAT IT DOES: RichterObjectProbe straight below an object by A; its answer's bits 0, 4 and 5.
 * HOW: zTemp06 A, zTemp04, 05 and 07 zero; AND #$31.
 * WAITS: no.
 * Entry: A how far below, X the object. Exit: A the bits, N and Z from them; the rest as the read left them.
 */
void RichterObjectProbeBelow(void)
{
    WR8(ZP->temp06, cpu.a);
    WR8(ZP->temp07, 0);
    WR8(ZP->temp04, 0);
    WR8(ZP->temp05, 0);
    JSR(0xCBC9, RichterObjectProbe());
    op_and(KIND4_LANDED_BITS);
}

/*
 * @routine 01BE:428 RichterObjectHitAndMove
 * @run 6E:CC28
 * WHAT IT DOES: An object's hitbox on, then moved with the scroll.
 * HOW: RichterObjectHitboxOn, RichterObjectScrollAndMove.
 * WAITS: no.
 * Entry: X the object. Exit: as RichterObjectScrollAndMove.
 */
void RichterObjectHitAndMove(void)
{
    JSR(0xCC2B, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    JSR(0xCC2E, RichterObjectScrollAndMove());
}

/* $CC33 on: kind 4 lands: sound 5, set on the floor by the read's row, stopped, frame $56 for 3 frames. */
static void kind4_land(void)
{
    op_lda(SOUND_KIND4_LAND);
    JSR(0xCC39, sound_exit(cpu.a));
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_sec();
    op_sbc(RD8(ZP->temp02));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    JSR(0xCC45, RichterObjectStop(cpu.x));
    WR8(OBJ->y_frac[cpu.x], 0);
    op_lda(FRAME_KIND4_LAND);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(KIND4_LAND_FRAMES);
    WR8(OBJ->hurt_timer[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BE:42F RichterKind4LandMarked
 * @run 6E:CC2F
 * WHAT IT DOES: Kind 4 lands on a marked floor: OBJ_WORK_D 1, then the landing.
 * HOW: The landing's stores from $CC33.
 * WAITS: no.
 * Entry: X the slot, zTemp02 the read's row. Exit: as RichterObjectNextState.
 */
void RichterKind4LandMarked(void)
{
    op_lda(1);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    kind4_land();
}

/*
 * @routine 01BE:3F1 RichterKind4Fall
 * @run 6E:CBF1
 * WHAT IT DOES: Kind 4 in the air: emptied below the map; falling on the first screen, a floor 8 below lands
 * it (marked when the read has bit 5); else a floor at its feet or $10 below lands it; rising or above, a
 * marked floor at its feet lands it; otherwise hitbox on and moved.
 * HOW: RichterObjectProbeBelow; RichterKind4LandMarked and the landing; RichterObjectHitAndMove.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind4Fall(void)
{
    bool feet_only = false;

    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_cmp(1);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_cmp(0xFF);
    if (cpu.p & P_Z) {
        feet_only = true;
    } else {
        op_lda(RD8(OBJ->vy_hi[cpu.x]));
        if (cpu.p & P_N) {
            feet_only = true;
        }
    }
    if (!feet_only) {
        op_lda(8);
        JSR(0xCC0B, RichterObjectProbeBelow());
        if (cpu.p & P_Z) {
            feet_only = true;
        } else {
            op_and(KIND4_FLOOR_MARK);
            if (!(cpu.p & P_Z)) {
                RichterKind4LandMarked();
                return;
            }
            op_lda(0);
            JSR(0xCC16, RichterObjectProbeBelow());
            if (cpu.p & P_Z) {
                kind4_land();
                return;
            }
            op_lda(0x10);
            JSR(0xCC1D, RichterObjectProbeBelow());
            if (!(cpu.p & P_Z)) {
                RichterKind4LandMarked();
                return;
            }
            feet_only = true;
        }
    }
    op_lda(0);
    JSR(0xCC24, RichterObjectProbeBelow());
    op_and(KIND4_FLOOR_MARK);
    if (!(cpu.p & P_Z)) {
        RichterKind4LandMarked();
        return;
    }
    RichterObjectHitAndMove();
}

/*
 * @routine 01BE:37E RichterKind4Start
 * @run 6E:CB7E
 * WHAT IT DOES: Kind 4's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, his
 * facing, his place offset ($0C, $EE), the throw's speeds (X entry 3 by facing, Y entry 4), hit timer 7,
 * sprite set 1, frame $55, the next state.
 * HOW: RichterFlagsAgainstFour, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft, SetVelocityYFromTable.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind4Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xCB8D, flags_against_four());
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xCB93, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(0x0C);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xEE);
    WR8(ZP->temp06, cpu.a);
    JSR(0xCB9E, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(3);
    JSR(0xCBA3, velocity_x_from_table());
    JSR(0xCBA6, negate_if_facing_left());
    op_lda(4);
    JSR(0xCBAB, velocity_y_from_table());
    op_lda(7);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x55);
    WR8(OBJ->frame[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
}

/*
 * @routine 01BE:455 RichterKind4Landed
 * @run 6E:CC55
 * WHAT IT DOES: Kind 4 on the floor: hitbox and move; when its timer runs out, landed plain: animation $12, X
 * speed entry 6 by facing, its place kept in OBJ_WORK_F to H, state 4; landed marked: animation $13, the
 * next state.
 * HOW: RichterObjectHitAndMove; RichterObjectSetAnimation, SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft, RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind4Landed(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xCC5A, RichterObjectHitAndMove());
    WR8(OBJ->hurt_timer[cpu.x], f_dec(RD8(OBJ->hurt_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->work_d[cpu.x]));
    if (!(cpu.p & P_Z)) {
        op_lda(ANIM_KIND4_SPLASH);
        JSR(0xCC8F, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    op_lda(ANIM_KIND4_RUN);
    JSR(0xCC6A, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_lda(6);
    JSR(0xCC6F, velocity_x_from_table());
    JSR(0xCC72, negate_if_facing_left());
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    WR8(OBJ->work_g[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_frac[cpu.x]));
    WR8(OBJ->work_h[cpu.x], cpu.a);
    op_lda(KIND4_BURN_STATE);
    WR8(OBJ->state[cpu.x], cpu.a);
}

/*
 * @routine 01BE:492 RichterKind4Splash
 * @run 6E:CC92
 * WHAT IT DOES: Kind 4 after a marked landing: hitbox and move until the animation ends, then emptied.
 * HOW: RichterObjectHitAndMove, RichterObjectAnimationWait, ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait.
 */
void RichterKind4Splash(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xCC97, RichterObjectHitAndMove());
    if (object_wait(0xCC9A)) {
        return;
    }
    ClearObject(cpu.x);
}

/*
 * @routine 01BE:58F RichterKind4Spawn
 * @run 6E:CD8F
 * WHAT IT DOES: At its animation's step $0B, the next slot copies this one as a flame (kind $0A, state 0,
 * animation $12, its count one more), up to two after the first.
 * HOW: The copy of the fields; RichterObjectSetAnimation on the new slot with X pushed.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last instruction or call left them; Y the next slot.
 */
void RichterKind4Spawn(void)
{
    op_txa();
    op_tay();
    op_iny();
    op_lda(RD8(OBJ->animation_at[cpu.x]));
    op_cmp(KIND4_FLAME_STEP);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_cmp(KIND4_FLAMES);
    if (cpu.p & P_Z) {
        return;
    }
    cpu.a = f_inc(cpu.a);
    WR8(OBJ->work_e[cpu.y], cpu.a);
    op_lda(RD8(OBJ->sprite_flip[cpu.x]));
    WR8(OBJ->sprite_flip[cpu.y], cpu.a);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    WR8(OBJ->work_f[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(OBJ->x_hi[cpu.y], cpu.a);
    WR8(OBJ->work_g[cpu.y], cpu.a);
    op_lda(RD8(OBJ->x_frac[cpu.x]));
    WR8(OBJ->x_frac[cpu.y], cpu.a);
    WR8(OBJ->work_h[cpu.y], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    WR8(OBJ->y_hi[cpu.y], cpu.a);
    op_lda(RD8(OBJ->vx_hi[cpu.x]));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    op_lda(RD8(OBJ->vx_lo[cpu.x]));
    WR8(OBJ->vx_lo[cpu.y], cpu.a);
    op_lda(RD8(OBJ->vy_hi[cpu.x]));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    op_lda(RD8(OBJ->vy_lo[cpu.x]));
    WR8(OBJ->vy_lo[cpu.y], cpu.a);
    op_lda(RD8(OBJ->sprite_set[cpu.x]));
    WR8(OBJ->sprite_set[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_n[cpu.x]));
    WR8(OBJ->work_n[cpu.y], cpu.a);
    op_phx();
    op_tya();
    op_tax();
    op_lda(ANIM_KIND4_RUN);
    JSR(0xCE05, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    op_lda(KIND_FLAME);
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
    op_plx();
}

/*
 * @routine 01BE:49D RichterKind4Burn
 * @run 6E:CC9D
 * WHAT IT DOES: Kind 4 (and the flames, kind $0A) burning along the floor: emptied when its animation ends;
 * the flames spread; hitbox and move; its angle nudged; a floor 8 below with bit 4 sets Y speed entry 8,
 * otherwise it is raised; a wall at the side by facing (the words at $CE0F, bits 1 and 5) stops it; no floor
 * 8 below, the next state.
 * HOW: RichterObjectAnimationRunning; RichterKind4Spawn; RichterObjectHitAndMove;
 * NudgeAngleByVelocityAndScroll; RichterObjectProbeBelow, SetVelocityYFromTable, MoveObjectUpBySigned;
 * RichterObjectProbe; tail jumps to RichterObjectStopX and RichterObjectNextState; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind4Burn(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    jsr_push(0xCCA2);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xCCAA, RichterKind4Spawn());
    JSR(0xCCAD, RichterObjectHitAndMove());
    JSR(0xCCB0, abi_ended(nudge_angle_by_velocity_and_scroll_exit_shim(NudgeAngleByVelocityAndScroll(cpu.x))));
    op_lda(8);
    JSR(0xCCB5, RichterObjectProbeBelow());
    if (cpu.p & P_Z) {
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    op_and(KIND4_STEP_BIT);
    if (!(cpu.p & P_Z)) {
        op_lda(8);
        JSR(0xCCC0, velocity_y_from_table());
    } else {
        JSR(0xCCC5, abi_ended(move_object_up_exit_shim(MoveObjectUpBySigned(cpu.x))));
    }
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind4WallReach + cpu.y)));
    WR8(ZP->temp04, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind4WallReach + 1 + cpu.y)));
    WR8(ZP->temp05, cpu.a);
    WR8(ZP->temp06, 0);
    WR8(ZP->temp07, 0);
    JSR(0xCCDB, RichterObjectProbe());
    op_and(KIND4_WALL_BITS);
    if (cpu.p & P_Z) {
        return;
    }
    RichterObjectStopX(cpu.x);
}

/*
 * @routine 01BE:531 RichterKind4Drop
 * @run 6E:CD31
 * WHAT IT DOES: A burning kind 4 or flame off the floor's edge: emptied when its animation ends; spread,
 * hitbox and move, angle nudged; a floor 8 below raises it, stops it falling and goes back a state; else it
 * falls at $28 a frame to $0700.
 * HOW: RichterObjectAnimationRunning, RichterKind4Spawn, RichterObjectHitAndMove,
 * NudgeAngleByVelocityAndScroll, RichterObjectProbe, MoveObjectUpBySigned, RichterObjectStopY; a jump to
 * RichterObjectFallCapped; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind4Drop(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    jsr_push(0xCD36);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xCD3E, RichterKind4Spawn());
    JSR(0xCD41, RichterObjectHitAndMove());
    JSR(0xCD44, abi_ended(nudge_angle_by_velocity_and_scroll_exit_shim(NudgeAngleByVelocityAndScroll(cpu.x))));
    op_lda(8);
    WR8(ZP->temp06, cpu.a);
    WR8(ZP->temp07, 0);
    WR8(ZP->temp04, 0);
    WR8(ZP->temp05, 0);
    JSR(0xCD51, RichterObjectProbe());
    op_and(KIND4_LANDED_BITS);
    if (cpu.p & P_Z) {
        op_lda(KIND4_CLIMB_PULL);
        RichterObjectFallCapped();
        return;
    }
    JSR(0xCD58, abi_ended(move_object_up_exit_shim(MoveObjectUpBySigned(cpu.x))));
    WR8(OBJ->y_frac[cpu.x], 0);
    JSR(0xCD5E, RichterObjectStopY(cpu.x));
    WR8(OBJ->state[cpu.x], f_dec(RD8(OBJ->state[cpu.x])));
}

/*
 * @routine 01BE:36C RichterKind4Frame
 * @run 6E:CB6C
 * WHAT IT DOES: Kind 4: runs its state's handler.
 * HOW: RichterStateDoubled; the six words at $CB72.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind4Frame(void)
{
    uint16_t handler;

    JSR(0xCB6F, state_doubled());
    handler = state_handler(RichterKind4States);
    switch (handler) {
    case 0xCB7E: RichterKind4Start(); break;
    case 0xCBF1: RichterKind4Fall(); break;
    case 0xCC55: RichterKind4Landed(); break;
    case 0xCC92: RichterKind4Splash(); break;
    case 0xCC9D: RichterKind4Burn(); break;
    case 0xCD31: RichterKind4Drop(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BE:613 RichterKind10Frame
 * @run 6E:CE13
 * WHAT IT DOES: Kind $0A, a flame of kind 4: runs its state's handler, kind 4's burning and dropping.
 * HOW: RichterStateDoubled; the two words at $CE19.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind10Frame(void)
{
    uint16_t handler;

    JSR(0xCE16, state_doubled());
    handler = state_handler(RichterKind10States);
    switch (handler) {
    case 0xCC9D: RichterKind4Burn(); break;
    case 0xCD31: RichterKind4Drop(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BE:673 RichterObjectMove
 * @run 6E:CE73
 * WHAT IT DOES: Moves an object by its velocity, Y then X.
 * HOW: ApplyVelocityY, then a tail jump to ApplyVelocityX (through their kernel vectors).
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocityX.
 */
void RichterObjectMove(void)
{
    JSR(0xCE76, axis_move_exit_shim(ApplyVelocityY(cpu.x)));
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
}

/*
 * @routine 01BE:63D RichterObjectScrollAndMove
 * @run 6E:CE3D
 * WHAT IT DOES: Moves an object against the scroll (unless the camera is in mode $17), then by its velocity.
 * HOW: ScrollSpeedX and ScrollSpeedY negated and added with their signs (zTemp10 the high byte);
 * RichterObjectAddXLow and RichterObjectAddXHigh; runs on into RichterObjectMove.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectMove.
 */
void RichterObjectScrollAndMove(void)
{
    op_lda(RD8(SCROLLLIM->camera_mode));
    op_cmp(0x17);
    if (!(cpu.p & P_Z)) {
        WR8(ZP->temp10, 0);
        op_lda(rd8(ScrollSpeedX));
        op_eor(0xFF);
        cpu.a = f_inc(cpu.a);
        if (cpu.p & P_N) {
            WR8(ZP->temp10, f_dec(RD8(ZP->temp10)));
        }
        JSR(0xCE53, abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a)));
        op_lda(RD8(ZP->temp10));
        JSR(0xCE58, abi_exit_sum(RichterObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0)));
        WR8(ZP->temp10, 0);
        op_lda(rd8(ScrollSpeedY));
        op_eor(0xFF);
        cpu.a = f_inc(cpu.a);
        if (cpu.p & P_N) {
            WR8(ZP->temp10, f_dec(RD8(ZP->temp10)));
        }
        op_clc();
        op_adc(RD8(OBJ->y_lo[cpu.x]));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        op_lda(RD8(ZP->temp10));
        op_adc(RD8(OBJ->y_hi[cpu.x]));
        WR8(OBJ->y_hi[cpu.x], cpu.a);
    }
    RichterObjectMove();
}

/*
 * @routine 01BE:6E7 RichterAngleToObject
 * @run 6E:CEE7
 * WHAT IT DOES: The angle from object X to object Y: the deltas of their positions into zTemp00-03.
 * HOW: Two 16-bit subtracts; a tail jump to AngleFromDelta (through its kernel vector).
 * WAITS: no.
 * Entry: X and Y the objects. Exit: as AngleFromDelta.
 */
void RichterAngleToObject(void)
{
    op_lda(RD8(OBJ->x_lo[cpu.y]));
    op_sec();
    op_sbc(RD8(OBJ->x_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.y]));
    op_sbc(RD8(OBJ->x_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.y]));
    op_sec();
    op_sbc(RD8(OBJ->y_lo[cpu.x]));
    WR8(ZP->temp02, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.y]));
    op_sbc(RD8(OBJ->y_hi[cpu.x]));
    WR8(ZP->temp03, cpu.a);
    angle_exit_shim(AngleFromDelta());
}

enum { KIND11_STEP = 2, KIND11_SOUND_OTHER = 0x3D, KIND11_SOUND = 0x3E, KIND_ELEVEN_B = 0x0D,
       KIND11_WOBBLE_FRAMES = 0x40, KIND11_ANGLE_STEP = 0x70, KIND11_TURN = 0x0F, KIND11_SCRIPT_KIND = 0x0C };

/*
 * @routine 01BE:716 RichterKind11Start
 * @run 6E:CF16
 * WHAT IT DOES: Kind $0B's state 0: once the player's animation reaches step 2, marked, his facing, his place
 * offset (0, $F8), its angle count up, its angle the byte at $CFFF by slot, hit timer 8, the wobble $40,
 * animation 8 started; sound $3D unless it is kind $0D, then sound $3E.
 * HOW: RichterObjectMarkThenFromPlayer, RichterObjectAtPlayer, RichterObjectStartAnimation, SoundCommand.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as SoundCommand with $3E.
 */
void RichterKind11Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->animation_at[0]));
    op_cmp(KIND11_STEP);
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xCF23, RichterObjectMarkThenFromPlayer());
    op_lda(0);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xF8);
    WR8(ZP->temp06, cpu.a);
    JSR(0xCF2E, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    WR8(OBJ->work_g[cpu.x], f_inc(RD8(OBJ->work_g[cpu.x])));
    op_lda(rd8((uint16_t)(RichterKind11Angles + cpu.x)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(8);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    op_lda(KIND11_WOBBLE_FRAMES);
    WR8(OBJ->work_i[cpu.x], cpu.a);
    op_lda(8);
    JSR(0xCF46, RichterObjectStartAnimation());
    op_lda(RD8(OBJ->kind[cpu.x]));
    op_cmp(KIND_ELEVEN_B);
    if (!(cpu.p & P_Z)) {
        op_lda(KIND11_SOUND_OTHER);
        JSR(0xCF52, sound_exit(cpu.a));
    }
    op_lda(KIND11_SOUND);
    sound_exit(cpu.a);
}

/*
 * @routine 01BE:7E6 RichterKind11Move
 * @run 6E:CFE6
 * WHAT IT DOES: Kind $0B on its way: animated, hitbox on, moved with the scroll, then its velocity from its
 * angle (OBJ_WORK_D) at the speed in OBJ_WORK_F and G.
 * HOW: RichterObjectAnimationTick, RichterObjectHitboxOn, RichterObjectScrollAndMove; zTemp00-01; a tail jump
 * to SetVelocityFromAngle (through its kernel vector).
 * WAITS: no.
 * Entry: X the slot. Exit: as SetVelocityFromAngle.
 */
void RichterKind11Move(void)
{
    JSR(0xCFE9, animation_exit_shim(RichterObjectAnimationTick(cpu.x)));
    JSR(0xCFEC, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    JSR(0xCFEF, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
}

/*
 * @routine 01BE:76F RichterKind11Fly
 * @run 6E:CF6F
 * WHAT IT DOES: Kind $0B in flight: its angle wobbles by the byte at $CF57 (facing plus OBJ_WORK_J); while the
 * wobble count runs it is unmarked, and every sixteenth frame of it script 1 (3 for kind $0C) on slot 3; at
 * its end the speed grows by $70 (held at $0F00), OBJ_WORK_J the byte at $CF5F by it; emptied below the
 * map's second screen or off the screen's sides; else as RichterKind11Move.
 * HOW: RichterObjectMarkOff; StartSoleScriptSlot with X pushed; ClearObject; runs on into RichterKind11Move.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind11Fly(void)
{
    bool grow = false;

    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_clc();
    op_adc(RD8(OBJ->work_j[cpu.x]));
    op_tay();
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(RichterKind11Wobble + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_i[cpu.x]));
    if (cpu.p & P_Z) {
        grow = true;
    } else {
        WR8(OBJ->work_i[cpu.x], f_dec(RD8(OBJ->work_i[cpu.x])));
        if (cpu.p & P_Z) {
            JSR(0xCF90, op_lda(RichterObjectMarkOff(cpu.x)));
            op_lda(RD8(OBJ->work_i[cpu.x]));
            op_and(0x0F);
            if (cpu.p & P_Z) {
                op_phx();
                op_ldy(1);
                op_lda(RD8(OBJ->kind[cpu.x]));
                op_cmp(KIND11_SCRIPT_KIND);
                if (cpu.p & P_Z) {
                    op_ldy(3);
                }
                op_tya();
                op_ldx(3);
                JSR(0xCFA9, sole_slot_exit_shim(StartSoleScriptSlot(cpu.a, cpu.x)));
                op_plx();
            }
            grow = true;
        }
    }
    if (grow) {
        op_lda(KIND11_ANGLE_STEP);
        op_clc();
        op_adc(RD8(OBJ->work_f[cpu.x]));
        WR8(OBJ->work_f[cpu.x], cpu.a);
        op_cla();
        op_adc(RD8(OBJ->work_g[cpu.x]));
        WR8(OBJ->work_g[cpu.x], cpu.a);
        op_cmp(KIND11_TURN);
        if (cpu.p & P_Z) {
            WR8(OBJ->work_f[cpu.x], 0);
        }
        op_tay();
        op_lda(rd8((uint16_t)(RichterKind11WobbleSteps + cpu.y)));
        WR8(OBJ->work_j[cpu.x], cpu.a);
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    if (!(cpu.p & P_Z) && !(cpu.p & P_N)) {
        op_cmp(2);
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
            return;
        }
    }
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(OBJ->x_lo[cpu.x]));
        op_cmp(0x20);
        if (cpu.p & P_C) {
            op_cmp(0xE0);
            if (!(cpu.p & P_C)) {
                ClearObject(cpu.x);
                return;
            }
        }
    }
    RichterKind11Move();
}

/*
 * @routine 01BE:70C RichterKind11Frame
 * @run 6E:CF0C
 * WHAT IT DOES: Kind $0B: runs its state's handler.
 * HOW: RichterStateDoubled; the two words at $CF12.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind11Frame(void)
{
    uint16_t handler;

    JSR(0xCF0F, state_doubled());
    handler = state_handler(RichterKind11States);
    switch (handler) {
    case 0xCF16: RichterKind11Start(); break;
    case 0xCF6F: RichterKind11Fly(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BF:00F RichterKind6CircleStart
 * @run 6E:D00F
 * WHAT IT DOES: Starts kind 6's circle: speed $04E0, its angle the byte at $D025 by slot, the count $46.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the slot. Exit: A $46, N and Z from it.
 */
void RichterKind6CircleStart(void)
{
    op_lda(0xE0);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(4);
    WR8(OBJ->work_g[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Angles + cpu.x)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(0x46);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
}

/*
 * @routine 01BF:035 RichterKind6CircleStep
 * @run 6E:D035
 * WHAT IT DOES: A step of kind 6's circle until the count reaches $73: moved, its angle 2 on, its velocity from
 * the angle at its speed, the count up.
 * HOW: RichterObjectMove; zWaveCounter read; SetVelocityFromAngle (through its kernel vector).
 * WAITS: no.
 * Entry: X the slot. Exit: at $73, A $73, Z and C set; else N and Z from the count, the rest as
 * SetVelocityFromAngle.
 */
void RichterKind6CircleStep(void)
{
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    op_cmp(KIND6_SPIN_END);
    if (cpu.p & P_Z) {
        return;
    }
    JSR(0xD040, RichterObjectMove());
    op_lda(RD8(ZP->wave_counter));
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(2);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xD05B, scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x)));
    WR8(OBJ->anim_timer[cpu.x], f_inc(RD8(OBJ->anim_timer[cpu.x])));
}

/*
 * @routine 01BF:06D RichterKind12Start
 * @run 6E:D06D
 * WHAT IT DOES: Kind $0C's state 0: thrown as kind 5, then moved down by the word at $D0A4 by its place among
 * three, his facing, and across by the word at $D0A0 by his facing.
 * HOW: RichterKind5Throw; RichterObjectFromPlayer, RichterObjectAddXLow and RichterObjectAddXHigh.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectAddXHigh, Y the offset.
 */
void RichterKind12Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xD072, RichterKind5Throw());
    op_txa();
    op_sec();
    op_sbc(KIND5_FIRST_SLOTS_A);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind12Drops + cpu.y)));
    op_clc();
    op_adc(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind12Drops + 1 + cpu.y)));
    op_adc(RD8(OBJ->y_hi[cpu.x]));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    JSR(0xD08E, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(RD8(OBJ->facing_left[0]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind12Across + cpu.y)));
    JSR(0xD099, abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a)));
    op_lda(rd8((uint16_t)(RichterKind12Across + 1 + cpu.y)));
    JSR(0xD09F, abi_exit_sum(RichterObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0)));
}

/*
 * @routine 01BF:05F RichterKind12Frame
 * @run 6E:D05F
 * WHAT IT DOES: Kind $0C: runs its state's handler: its start, then kind 5's flight, stuck and fall.
 * HOW: RichterStateDoubled; the four words at $D065.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind12Frame(void)
{
    uint16_t handler;

    JSR(0xD062, state_doubled());
    handler = state_handler(RichterKind12States);
    switch (handler) {
    case 0xD06D: RichterKind12Start(); break;
    case 0xC8DF: RichterKind5Fly(); break;
    case 0xC952: RichterKind5Stuck(); break;
    case 0xC960: RichterObjectFall38(); break;
    default: run_routine(handler); break;
    }
}

enum { KIND8_LEAD = 5, KIND8_FIRST_TAIL = 6, KIND_CHAIN = 8, KIND8_ANGLE = 0xA4, KIND8_HIT = 0x0A,
       ANIM_KIND8_TAIL = 6, SOUND_KIND8 = 0x09, ANIM_KIND8_LEAD = 0x19, KIND8_GROW = 0x18 };

/*
 * @routine 01BF:143 RichterKind8Fly
 * @run 6E:D143
 * WHAT IT DOES: Kind 8, a chain led by slot 5: each waits out its delay moving with the scroll; the lead
 * wobbles its angle (the byte at $CF57 by facing), grows its speed by $18, moves as kind $0B, and records its
 * velocity in the path at $D1DE by its count; emptied below the map's second screen or off $60-$9F of a
 * side screen. A tail link, while the one before it is out, takes the lead's velocity from the path its own
 * delay back and moves with the scroll; else it is emptied.
 * HOW: OBJ_WORK_I the delay, OBJ_WORK_J the count; RichterKind11Move; RichterObjectScrollAndMove; ClearObject.
 * WAITS: no.
 * Entry: X the slot. Exit: as its last step left them.
 */
void RichterKind8Fly(void)
{
    op_lda(RD8(OBJ->work_i[cpu.x]));
    if (!(cpu.p & P_Z)) {
        WR8(OBJ->work_i[cpu.x], f_dec(RD8(OBJ->work_i[cpu.x])));
        RichterObjectScrollAndMove();
        return;
    }
    op_cpx(KIND8_LEAD);
    if (cpu.p & P_Z) {
        op_ldy(RD8(OBJ->facing_left[cpu.x]));
        op_lda(RD8(OBJ->work_d[cpu.x]));
        op_clc();
        op_adc(rd8((uint16_t)(RichterKind11Wobble + cpu.y)));
        WR8(OBJ->work_d[cpu.x], cpu.a);
        op_lda(KIND8_GROW);
        op_clc();
        op_adc(RD8(OBJ->work_f[cpu.x]));
        WR8(OBJ->work_f[cpu.x], cpu.a);
        op_cla();
        op_adc(RD8(OBJ->work_g[cpu.x]));
        WR8(OBJ->work_g[cpu.x], cpu.a);
        JSR(0xD172, RichterKind11Move());
        op_lda(RD8(OBJ->work_j[cpu.x]));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(RD8(OBJ->vx_lo[cpu.x]));
        wr8((uint16_t)(RichterKind8Path + cpu.y), cpu.a);
        op_lda(RD8(OBJ->vx_hi[cpu.x]));
        wr8((uint16_t)(RichterKind8Path + 1 + cpu.y), cpu.a);
        op_lda(RD8(OBJ->vy_lo[cpu.x]));
        wr8((uint16_t)(RichterKind8Path + 2 + cpu.y), cpu.a);
        op_lda(RD8(OBJ->vy_hi[cpu.x]));
        wr8((uint16_t)(RichterKind8Path + 3 + cpu.y), cpu.a);
        WR8(OBJ->work_j[cpu.x], f_inc(RD8(OBJ->work_j[cpu.x])));
        op_lda(RD8(OBJ->y_hi[cpu.x]));
        op_cmp(2);
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
            return;
        }
        op_lda(RD8(OBJ->x_hi[cpu.x]));
        if (cpu.p & P_Z) {
            return;
        }
        op_lda(RD8(OBJ->x_lo[cpu.x]));
        op_cmp(0x60);
        if (!(cpu.p & P_C)) {
            return;
        }
        op_cmp(0xA0);
        if (cpu.p & P_C) {
            return;
        }
        if (cpu.p & P_Z) {
            ClearObject(cpu.x);
            return;
        }
        ClearObject(cpu.x);
        return;
    }
    {
        uint8_t before = (uint8_t)(cpu.x - 1);

        op_lda(RD8(OBJ->kind[before]));
    }
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->work_j[KIND8_LEAD]));
    op_sec();
    op_sbc(RD8(OBJ->work_j[cpu.x]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind8Path + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind8Path + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind8Path + 2 + cpu.y)));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind8Path + 3 + cpu.y)));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    JSR(0xD1DD, RichterObjectScrollAndMove());
}

/*
 * @routine 01BF:0BA RichterKind8Frame
 * @run 6E:D0BA
 * WHAT IT DOES: Kind 8: in flight, as RichterKind8Fly. At state 0: emptied while the player is hurt; once
 * RichterFlagsAgainstFour lets it, the lead (slot 5) makes slots 6 to 15 kind 8; each takes his facing, his
 * place offset (0, $E8), speed $0100 at angle $A4, hit timer $0A, its delay and count the byte at $D133 by
 * slot, marked, animation 6 started, hitbox on; the lead then unmarked, sound 9, sprite set 3, animation $19.
 * HOW: RichterFlagsAgainstFour; RichterObjectFromPlayer, RichterObjectAtPlayer, RichterObjectMarkOn,
 * RichterObjectStartAnimation, RichterObjectHitboxOn; RichterObjectMarkOff, SoundCommand, a tail jump to
 * RichterObjectSetAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind8Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        RichterKind8Fly();
        return;
    }
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_HURT);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    JSR(0xD0D3, flags_against_four());
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_cpx(KIND8_LEAD);
    if (cpu.p & P_Z) {
        op_ldy(KIND8_FIRST_TAIL);
        op_lda(KIND_CHAIN);
        do {
            WR8(OBJ->kind[cpu.y], cpu.a);
            op_iny();
            op_cpy(SLOT_END);
        } while (!(cpu.p & P_Z));
    }
    JSR(0xD0E9, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_lda(0);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xE8);
    WR8(ZP->temp06, cpu.a);
    JSR(0xD0F4, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
    op_lda(0);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->work_g[cpu.x], cpu.a);
    op_lda(KIND8_ANGLE);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(KIND8_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind8Delays + cpu.x)));
    WR8(OBJ->work_i[cpu.x], cpu.a);
    WR8(OBJ->work_j[cpu.x], cpu.a);
    JSR(0xD114, op_lda(RichterObjectMarkOn(cpu.x)));
    op_lda(ANIM_KIND8_TAIL);
    JSR(0xD119, RichterObjectStartAnimation());
    JSR(0xD11C, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    op_cpx(KIND8_LEAD);
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xD124, op_lda(RichterObjectMarkOff(cpu.x)));
    op_lda(SOUND_KIND8);
    JSR(0xD129, sound_exit(cpu.a));
    op_lda(SPRITE_SET_COMPANION);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(ANIM_KIND8_LEAD);
    animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a));
}

enum { KIND13_LEAD = 5, KIND13_PIECES = 6, KIND13_FIRST_PIECE = 6, KIND13_END_SLOT = 0x0F, KIND13_HIT = 0x0D,
       ANIM_KIND13_GROW = 0x17, ANIM_KIND13 = 0x15, KIND13_FIRST_FRAME = 0x6E, PLAYER_STATE_REACH_A = 0x0B,
       PLAYER_STATE_REACH_B = 0x0C };

/*
 * @routine 01BF:3B2 RichterKind13Pieces
 * @run 6E:D3B2
 * WHAT IT DOES: Places kind $0D's six pieces (slots 6 to 11) round slot 5 by the list at zTemp00-01: a count,
 * then for each its Y offset, its X offset (taken from the lead's X when he faces left), and its hitbox;
 * each with hit timer $0D and its hitbox on.
 * HOW: Reads through the zero page pointer; RichterObjectHitboxFrom with Y pushed.
 * WAITS: no.
 * Entry: zTemp00-01 the list. Exit: X one past the last piece, N and Z from the count's DEC; the rest as the
 * last hitbox left them.
 */
void RichterKind13Pieces(void)
{
    op_ldx(KIND13_PIECES);
    op_cly();
    op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    WR8(ZP->temp03, cpu.a);
    op_iny();
    do {
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        op_iny();
        op_clc();
        op_adc(RD8(OBJ->y_lo[KIND13_LEAD]));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        op_adc(RD8(OBJ->y_hi[KIND13_LEAD]));
        WR8(OBJ->y_hi[cpu.x], cpu.a);
        op_iny();
        op_lda(RD8(OBJ->facing_left[KIND13_LEAD]));
        if (cpu.p & P_Z) {
            op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
            op_iny();
            op_clc();
            op_adc(RD8(OBJ->x_lo[KIND13_LEAD]));
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
            op_adc(RD8(OBJ->x_hi[KIND13_LEAD]));
            WR8(OBJ->x_hi[cpu.x], cpu.a);
            op_iny();
        } else {
            op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
            WR8(ZP->temp04, cpu.a);
            op_iny();
            op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
            WR8(ZP->temp05, cpu.a);
            op_iny();
            op_lda(RD8(OBJ->x_lo[KIND13_LEAD]));
            op_sec();
            op_sbc(RD8(ZP->temp04));
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            op_lda(RD8(OBJ->x_hi[KIND13_LEAD]));
            op_sbc(RD8(ZP->temp05));
            WR8(OBJ->x_hi[cpu.x], cpu.a);
        }
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        op_iny();
        op_phy();
        JSR(0xD409, hitbox_exit_shim(RichterObjectHitboxFrom(cpu.x, cpu.a, (cpu.p & P_Z) != 0)));
        op_ply();
        op_lda(KIND13_HIT);
        WR8(OBJ->work_n[cpu.x], cpu.a);
        op_lda(1);
        WR8(OBJ->hitbox_on[cpu.x], cpu.a);
        op_inx();
        WR8(ZP->temp03, f_dec(RD8(ZP->temp03)));
    } while (!(cpu.p & P_Z));
}

/*
 * @routine 01BF:39A RichterKind13Place
 * @run 6E:D39A
 * WHAT IT DOES: Places kind $0D's pieces by its frame: the list at $DD48 by twice the frame less $6E.
 * HOW: zTemp00-01; RichterKind13Pieces with X pushed.
 * WAITS: no.
 * Entry: X the slot. Exit: X the slot, N and Z from it; the rest as RichterKind13Pieces left them.
 */
void RichterKind13Place(void)
{
    op_lda(RD8(OBJ->frame[cpu.x]));
    op_sec();
    op_sbc(KIND13_FIRST_FRAME);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind13Lists + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind13Lists + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_phx();
    JSR(0xD3B0, RichterKind13Pieces());
    op_plx();
}

/*
 * @routine 01BF:344 RichterKind13AtPlayer
 * @run 6E:D344
 * WHAT IT DOES: Kind $0D at the player's place, moved further across by facing in his states $0B and $0C
 * (the words at $D392 and $D396).
 * HOW: RichterObjectAtPlayerPlace; RichterObjectAddXLow and RichterObjectAddXHigh.
 * WAITS: no.
 * Entry: X the slot. Exit: as the last step left them.
 */
void RichterKind13AtPlayer(void)
{
    uint16_t table;

    JSR(0xD347, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
    op_lda(RD8(OBJ->state[0]));
    op_cmp(PLAYER_STATE_REACH_A);
    if (cpu.p & P_Z) {
        table = RichterKind13ReachA;
    } else {
        op_cmp(PLAYER_STATE_REACH_B);
        if (!(cpu.p & P_Z)) {
            return;
        }
        table = RichterKind13ReachB;
    }
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(table + cpu.y)));
    JSR(table == RichterKind13ReachA ? 0xD35E : 0xD370, abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a)));
    op_lda(rd8((uint16_t)(table + 1 + cpu.y)));
    JSR(table == RichterKind13ReachA ? 0xD364 : 0xD376, abi_exit_sum(RichterObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0)));
}

/*
 * @routine 01BF:2EC RichterKind13Start
 * @run 6E:D2EC
 * WHAT IT DOES: Kind $0D's state 0 as kind $0B's; once started, speed $0160, the wobble $3C, animation $17.
 * HOW: RichterKind11Start; a tail jump to RichterObjectSetAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step left them.
 */
void RichterKind13Start(void)
{
    JSR(0xD2EF, RichterKind11Start());
    op_lda(RD8(OBJ->state[cpu.x]));
    if (cpu.p & P_Z) {
        return;
    }
    op_lda(0x60);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->work_g[cpu.x], cpu.a);
    op_lda(0x3C);
    WR8(OBJ->work_i[cpu.x], cpu.a);
    op_lda(ANIM_KIND13_GROW);
    animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a));
}

/*
 * @routine 01BF:309 RichterKind13Fly
 * @run 6E:D309
 * WHAT IT DOES: Kind $0D in flight as kind $0B; when its wobble runs out, the next state, and all but the lead
 * (slot 5) are emptied.
 * HOW: RichterKind11Fly; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step left them.
 */
void RichterKind13Fly(void)
{
    JSR(0xD30C, RichterKind11Fly());
    op_lda(RD8(OBJ->work_i[cpu.x]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
    op_cpx(KIND13_LEAD);
    if (cpu.p & P_Z) {
        return;
    }
    ClearObject(cpu.x);
}

/*
 * @routine 01BF:31C RichterKind13Hold
 * @run 6E:D31C
 * WHAT IT DOES: Kind $0D held by the player while he attacks (step 1): hit timer $0D, sprite set 1, his facing,
 * animation $15, the next state, at his place, its pieces placed.
 * HOW: RichterObjectFromPlayer, RichterObjectSetAnimation, RichterKind13AtPlayer; a jump to RichterKind13Place.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step left them.
 */
void RichterKind13Hold(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->substate[0]));
    op_cmp(PLAYER_STEP_ATTACK);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(KIND13_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    JSR(0xD333, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    op_ldy(RD8(OBJ->state[0]));
    op_lda(ANIM_KIND13);
    JSR(0xD33B, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
    JSR(0xD341, RichterKind13AtPlayer());
    RichterKind13Place();
}

/*
 * @routine 01BF:377 RichterKind13Swing
 * @run 6E:D377
 * WHAT IT DOES: Kind $0D swinging: his facing, at his place; its pieces placed while the animation runs; at its
 * end slots from this one to $0E are emptied.
 * HOW: RichterObjectFromPlayer, RichterKind13AtPlayer, RichterObjectAnimationRunning, RichterKind13Place;
 * ClearObject with X pushed.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step left them.
 */
void RichterKind13Swing(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    JSR(0xD37C, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    JSR(0xD37F, RichterKind13AtPlayer());
    jsr_push(0xD382);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (!(cpu.p & P_Z)) {
        RichterKind13Place();
        return;
    }
    op_phx();
    do {
        JSR(0xD38B, ClearObject(cpu.x));
        op_inx();
        op_cpx(KIND13_END_SLOT);
    } while (!(cpu.p & P_Z));
    op_plx();
}

/*
 * @routine 01BF:2DE RichterKind13Frame
 * @run 6E:D2DE
 * WHAT IT DOES: Kind $0D: runs its state's handler.
 * HOW: RichterStateDoubled; the four words at $D2E4.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind13Frame(void)
{
    uint16_t handler;

    JSR(0xD2E1, state_doubled());
    handler = state_handler(RichterKind13States);
    switch (handler) {
    case 0xD2EC: RichterKind13Start(); break;
    case 0xD309: RichterKind13Fly(); break;
    case 0xD31C: RichterKind13Hold(); break;
    case 0xD377: RichterKind13Swing(); break;
    default: run_routine(handler); break;
    }
}

enum { KIND14_STEP = 2, KIND14_SCRIPT = 2, KIND14_SCRIPT_SLOT = 3, KIND14_FRAMES = 0xA0, SOUND_KIND14 = 0x33,
       KIND14_RASTER = 2, KIND14_FIRST_DROP = 6, KIND_DROP = 0x0F, FRAME_DROP = 0x61, KIND15_HIT = 0x0B,
       KIND15_SHOT = 0x9C, SHOT_FIRST = 0x30, SHOT_END = 0x40 };

/*
 * @routine 01BF:41A RichterKind14Frame
 * @run 6E:D41A
 * WHAT IT DOES: Kind $0E: at state 0, script 2 on slot 3 until the player's animation reaches step 2; then
 * $A0 frames, sound $33, the raster stage kept (PlayerKeptRasterStage) and set to 2, the next state. Then,
 * when its frames run out, the raster stage given back and it is emptied; on even wave counts, a drop (kind
 * $0F) in the slot 6 plus its count when free: its place from the four bytes at $D56D by its other count
 * (X wobbling by the wave counter's low bits), sprite set 1, frame $61, the speeds (X entry 4 by facing, Y
 * entry 5), hit timer $0B, hitbox on; the counts move on (the second in BCD, both kept below $10).
 * HOW: StartSoleScriptSlot with X pushed; SoundCommand; zRasterStage; SetVelocityXFromTable,
 * NegateVelocityXIfFacingLeft, SetVelocityYFromTable, RichterObjectHitboxOn with X pushed and swapped;
 * ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind14Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->state[cpu.x]));
    if (cpu.p & P_Z) {
        op_lda(RD8(OBJ->animation_at[0]));
        op_cmp(KIND14_STEP);
        if (!(cpu.p & P_Z)) {
            op_phx();
            op_ldx(KIND14_SCRIPT_SLOT);
            op_lda(KIND14_SCRIPT);
            JSR(0xD430, sole_slot_exit_shim(StartSoleScriptSlot(cpu.a, cpu.x)));
            op_plx();
            return;
        }
        op_lda(KIND14_FRAMES);
        WR8(OBJ->hurt_timer[cpu.x], cpu.a);
        op_lda(SOUND_KIND14);
        JSR(0xD43C, sound_exit(cpu.a));
        op_lda(RD8(ZP->raster_stage));
        WR8(PVARS->kept_raster_stage, cpu.a);
        op_lda(KIND14_RASTER);
        WR8(ZP->raster_stage, cpu.a);
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    WR8(OBJ->hurt_timer[cpu.x], f_dec(RD8(OBJ->hurt_timer[cpu.x])));
    if (cpu.p & P_Z) {
        op_lda(RD8(PVARS->kept_raster_stage));
        WR8(ZP->raster_stage, cpu.a);
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(ZP->wave_counter));
    op_and(0x01);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(KIND14_FIRST_DROP);
    op_tay();
    op_lda(RD8(OBJ->kind[cpu.y]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(KIND_DROP);
    WR8(OBJ->kind[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_phx();
    op_sxy();
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind14Drops + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind14Drops + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind14Drops + 2 + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    op_lda(rd8((uint16_t)(RichterKind14Drops + 3 + cpu.y)));
    WR8(ZP->temp03, cpu.a);
    op_lda(RD8(ZP->temp00));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(RD8(ZP->temp01));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(RD8(ZP->wave_counter));
    op_and(0x07);
    op_clc();
    op_adc(RD8(ZP->temp02));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(ZP->temp03));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(FRAME_DROP);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(4);
    JSR(0xD4B2, velocity_x_from_table());
    JSR(0xD4B5, negate_if_facing_left());
    op_lda(5);
    JSR(0xD4BA, velocity_y_from_table());
    op_lda(KIND15_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    JSR(0xD4C2, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    op_plx();
    op_lda(RD8(OBJ->work_e[cpu.x]));
    cpu.a = f_inc(cpu.a);
    op_and(0x0F);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_sed();
    op_clc();
    op_adc(1);
    op_cld();
    op_and(0x0F);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

/*
 * @routine 01BF:511 RichterKind15Shot
 * @run 6E:D511
 * WHAT IT DOES: When a drop is struck by an object whose OBJ_HITBOX_ON is 2 or more, a kind $9C object in the
 * first free slot from $30 to $3F at that object's place, $10 up and wobbling across by the wave counter.
 * HOW: The point in zTemp00-03; ClearObject, RichterObjectToPoint; X pushed.
 * WAITS: no.
 * Entry: A the striking object's slot, X the drop. Exit: X the drop, N and Z from it; the rest as the last step
 * left them.
 */
void RichterKind15Shot(void)
{
    op_phx();
    op_tax();
    op_lda(RD8(OBJ->hitbox_on[cpu.x]));
    op_cmp(HITBOX_STRUCK);
    if (!(cpu.p & P_C)) {
        op_plx();
        return;
    }
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp04, 0);
    op_lda(RD8(ZP->wave_counter));
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_and(0x1F);
    op_sec();
    op_sbc(0x10);
    if (cpu.p & P_N) {
        WR8(ZP->temp04, f_dec(RD8(ZP->temp04)));
    }
    op_clc();
    op_adc(RD8(ZP->temp00));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(ZP->temp01));
    op_adc(RD8(ZP->temp04));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(0xF0);
    WR8(ZP->temp02, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(0xFF);
    WR8(ZP->temp03, cpu.a);
    op_ldx(SHOT_FIRST);
    for (;;) {
        op_lda(RD8(OBJ->kind[cpu.x]));
        if (cpu.p & P_Z) {
            break;
        }
        op_inx();
        op_cpx(SHOT_END);
        if (cpu.p & P_Z) {
            op_plx();
            return;
        }
    }
    JSR(0xD55F, ClearObject(cpu.x));
    op_lda(KIND15_SHOT);
    WR8(OBJ->kind[cpu.x], cpu.a);
    JSR(0xD567, abi_exit_loaded_object(RichterObjectToPoint(cpu.x)));
    op_plx();
}

/*
 * @routine 01BF:4DA RichterKind15Frame
 * @run 6E:D4DA
 * WHAT IT DOES: Kind $0F, a drop of kind $0E: its frame by its slot's bit 0 and the wave counter (the four
 * bytes at $D569), marked, moved with the scroll; struck, a shot; emptied below the map; its hitbox on only
 * on every eighth wave count.
 * HOW: RichterObjectMarkOn, RichterObjectScrollAndMove, RichterKind15Shot; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
 */
void RichterKind15Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_txa();
    op_and(0x01);
    op_eor(RD8(ZP->wave_counter));
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind15Frames + cpu.y)));
    WR8(OBJ->frame[cpu.x], cpu.a);
    JSR(0xD4ED, op_lda(RichterObjectMarkOn(cpu.x)));
    JSR(0xD4F0, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->hit_by_slot[cpu.x]));
    if (!(cpu.p & P_Z)) {
        JSR(0xD4F8, RichterKind15Shot());
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_cmp(1);
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    WR8(OBJ->hit_by_slot[cpu.x], 0);
    WR8(OBJ->hitbox_on[cpu.x], 0);
    {
        uint8_t wave = RD8(ZP->wave_counter);

        op_tst(0x07, wave);
    }
    if (!(cpu.p & P_Z)) {
        return;
    }
    WR8(OBJ->hitbox_on[cpu.x], f_inc(RD8(OBJ->hitbox_on[cpu.x])));
}

enum { KIND16 = 0x10, TIME_SLOW_TWO = 2, KIND16_ATTR = 0x85, KIND16_HIT = 0x0C, KIND16_AIM_FRAMES = 0x70,
       KIND16_TURN_AT = 0x38, ANIM_KIND16_STOP = 0x0F, TARGET_FIRST = 0x10, TARGET_END = 0x58,
       FRAME_KIND17 = 0x7A, PLAYER_STEP_KIND17_A = 2, PLAYER_STEP_KIND17_B = 3 };

/*
 * @routine 01BF:686 RichterFindTarget
 * @run 6E:D686
 * WHAT IT DOES: Finds the first enemy (slots $10 to $57) to aim at: OBJ_HITBOX_ON 2, its health's bit 7
 * clear, on the first screen across and down.
 * HOW: A scan by Y.
 * WAITS: no.
 * Entry: nothing. Exit: found, C set, Y the slot; else C clear, Y $58; A and N, Z as the last test left them.
 */
void RichterFindTarget(void)
{
    op_ldy(TARGET_FIRST);
    do {
        op_lda(RD8(OBJ->hitbox_on[cpu.y]));
        op_cmp(HITBOX_STRUCK);
        if (cpu.p & P_Z) {
            op_lda(RD8(OBJ->health[cpu.y]));
            if (!(cpu.p & P_N)) {
                op_lda(RD8(OBJ->x_hi[cpu.y]));
                op_ora(RD8(OBJ->y_hi[cpu.y]));
                if (cpu.p & P_Z) {
                    op_sec();
                    return;
                }
            }
        }
        op_iny();
        op_cpy(TARGET_END);
    } while (!(cpu.p & P_Z));
    op_clc();
}

/*
 * @routine 01BF:5C1 RichterKind16Start
 * @run 6E:D5C1
 * WHAT IT DOES: Kind $10's state 0: the first (slot 4) sounds $5F, sets TimeSlowMode 2 and zFadeCount 1, and
 * makes slots 4 to 15 kind $10; each takes its place as kind 6's does, sprite attribute $85, its circling
 * start, animation $16 started.
 * HOW: SoundCommand; RichterKind6Places; RichterKind6CircleStart; a jump to RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectStartAnimation.
 */
void RichterKind16Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_txa();
    op_sec();
    op_sbc(KIND_STOPWATCH_FIRST);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_cmp(4);
    if (cpu.p & P_Z) {
        op_lda(SOUND_KIND6);
        JSR(0xD5D2, sound_exit(cpu.a));
        op_lda(TIME_SLOW_TWO);
        WR8(PVARS->time_slow_mode, cpu.a);
        op_lda(1);
        WR8(ZP->fade_count, cpu.a);
        op_ldy(KIND6_PLACE_FIRST);
        op_lda(KIND16);
        do {
            WR8(OBJ->kind[cpu.y], cpu.a);
            op_iny();
            op_cpy(SLOT_END);
        } while (!(cpu.p & P_Z));
        op_lda(4);
    }
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind6Places + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 1 + cpu.y)));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 2 + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind6Places + 3 + cpu.y)));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(KIND16_ATTR);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    JSR(0xD60A, RichterKind6CircleStart());
    op_lda(ANIM_KIND6);
    RichterObjectStartAnimation();
}

/*
 * @routine 01BF:60F RichterKind16Aim
 * @run 6E:D60F
 * WHAT IT DOES: Kind $10 waiting out its two timers as kind 6; then animation $16 and, with a target, flies at
 * it (speed $0380 at the angle to it), $70 frames, hit timer $0C, hitbox on; with none, animation $0F and the
 * timer cleared; the next state.
 * HOW: RichterObjectSetAnimation, RichterFindTarget, RichterAngleToObject, SetVelocityFromAngle with A pushed,
 * RichterObjectHitboxOn, RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind16Aim(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->hurt_timer[cpu.x], f_dec(RD8(OBJ->hurt_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (!(cpu.p & P_Z)) {
        WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
        return;
    }
    op_lda(ANIM_KIND6);
    JSR(0xD625, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
    JSR(0xD628, RichterFindTarget());
    if (!(cpu.p & P_C)) {
        op_lda(ANIM_KIND16_STOP);
        JSR(0xD64F, animation_exit_shim(RichterObjectSetAnimation(cpu.x, cpu.a)));
        WR8(OBJ->anim_timer[cpu.x], 0);
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    JSR(0xD62D, RichterAngleToObject());
    op_pha();
    op_lda(0x80);
    WR8(ZP->temp00, cpu.a);
    op_lda(3);
    WR8(ZP->temp01, cpu.a);
    op_pla();
    JSR(0xD63A, scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x)));
    op_lda(KIND16_AIM_FRAMES);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    op_lda(KIND16_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    JSR(0xD647, hitbox_on_exit(RichterObjectHitboxOn(cpu.x)));
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BF:655 RichterKind16Fly
 * @run 6E:D655
 * WHAT IT DOES: Kind $10 flying at its target: when its animation ends, hitbox and attribute off, the next
 * state; while its timer runs, moved, turned back both ways at $38; at its end stopped, hitbox off, animation
 * $0F.
 * HOW: RichterObjectAnimationRunning; NegateVelocityX ($D869) and RichterObjectNegateVelocityY; a jump to
 * RichterObjectMove; RichterObjectStop.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind16Fly(void)
{
    AnimationStep running;

    op_ldx(RD8(ZP->richter_object));
    jsr_push(0xD65A);
    running = RichterObjectAnimationRunning(cpu.x);
    jsr_pop();
    animation_exit_shim(running);
    if (cpu.p & P_Z) {
        WR8(OBJ->hitbox_on[cpu.x], 0);
        WR8(OBJ->sprite_attr[cpu.x], 0);
        abi_nz(RichterObjectNextState(cpu.x));
        return;
    }
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    if (cpu.p & P_Z) {
        JSR(0xD674, RichterObjectStop(cpu.x));
        WR8(OBJ->hitbox_on[cpu.x], 0);
        op_lda(ANIM_KIND16_STOP);
        WR8(OBJ->animation[cpu.x], cpu.a);
        return;
    }
    op_cmp(KIND16_TURN_AT);
    if (cpu.p & P_Z) {
        JSR(0xD668, negate_exit_shim(NegateVelocityX(cpu.x)));
        JSR(0xD66B, RichterObjectNegateVelocityY());
    }
    WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
    RichterObjectMove();
}

/*
 * @routine 01BF:5AD RichterKind16Frame
 * @run 6E:D5AD
 * WHAT IT DOES: Kind $10: runs its state's handler, some of them kind 6's.
 * HOW: RichterStateDoubled; the seven words at $D5B3.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind16Frame(void)
{
    uint16_t handler;

    JSR(0xD5B0, state_doubled());
    handler = state_handler(RichterKind16States);
    switch (handler) {
    case 0xD5C1: RichterKind16Start(); break;
    case 0xC9F6: RichterKind6Circle(); break;
    case 0xCA0D: RichterKind6Hold(); break;
    case 0xD60F: RichterKind16Aim(); break;
    case 0xD655: RichterKind16Fly(); break;
    case 0xCA47: RichterKind6Gather(); break;
    case 0xCA75: RichterKind6Fade(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01BF:6A5 RichterKind17Frame
 * @run 6E:D6A5
 * WHAT IT DOES: Kind $11, held by the player at his steps 2 and 3: otherwise a kind 3 companion is made sure
 * of and it is emptied. At state 0, once his animation reaches step 2, frame $7A, sprite set 1, his facing and
 * place, the next state; then at his place, its sprite attribute by bit 0 of the wave counter (the two bytes
 * at $D6EA).
 * HOW: RichterClaimKind3Slot, ClearObject; RichterObjectFromPlayer, RichterObjectAtPlayerPlace,
 * RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind17Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->substate[0]));
    op_cmp(PLAYER_STEP_KIND17_A);
    if (!(cpu.p & P_Z)) {
        op_lda(RD8(OBJ->substate[0]));
        op_cmp(PLAYER_STEP_KIND17_B);
        if (!(cpu.p & P_Z)) {
            JSR(0xD6B8, RichterClaimKind3Slot());
            ClearObject(cpu.x);
            return;
        }
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        JSR(0xD6DE, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
        op_lda(RD8(ZP->wave_counter));
        op_and(0x01);
        op_tay();
        op_lda(rd8((uint16_t)(RichterKind17Attributes + cpu.y)));
        WR8(OBJ->sprite_attr[cpu.x], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->animation_at[0]));
    op_cmp(2);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(FRAME_KIND17);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    JSR(0xD6D5, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    JSR(0xD6D8, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
    abi_nz(RichterObjectNextState(cpu.x));
}

enum { KIND18_STEP = 2, KIND18_AIM_X = 0x80, KIND18_AIM_Y = 0x50, ANIM_KIND18 = 0x18, KIND18_BOX_LONG = 0x38,
       KIND18_BOX_SHORT = 0x18, KIND18_HIT = 0x0E, KIND18_HOLD = 0x10, KIND18_RISE = 0xFF, KIND18_FALL_AT = 0x80,
       KIND18_END_FRAMES = 0x24, KIND18_ROWS = 0x40, KIND18_POINTS = 0x0B };

/*
 * @routine 01BF:793 RichterKind18Pair
 * @run 6E:D793
 * WHAT IT DOES: The second half of kind $12 (the next slot) at its place.
 * HOW: OBJ_X_LO and OBJ_Y_LO copied.
 * WAITS: no.
 * Entry: X the slot. Exit: A its Y low byte, N and Z from it.
 */
void RichterKind18Pair(void)
{
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    {
        uint8_t next = (uint8_t)(cpu.x + 1);

        WR8(OBJ->x_lo[next], cpu.a);
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        WR8(OBJ->y_lo[next], cpu.a);
    }
}

/*
 * @routine 01BF:7F8 RichterKind18Mark
 * @run 6E:D7F8
 * WHAT IT DOES: Marks kind $12 on.
 * HOW: RichterObjectMarkOn.
 * WAITS: no.
 * Entry: X the slot. Exit: as RichterObjectMarkOn.
 */
void RichterKind18Mark(void)
{
    JSR(0xD7FB, op_lda(RichterObjectMarkOn(cpu.x)));
}

/*
 * @routine 01BF:7FC RichterKind18End
 * @run 6E:D7FC
 * WHAT IT DOES: Kind $12's rise ends: $24 frames, the next state.
 * HOW: A jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: X the slot. Exit: as RichterObjectNextState.
 */
void RichterKind18End(void)
{
    op_lda(KIND18_END_FRAMES);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BF:700 RichterKind18Start
 * @run 6E:D700
 * WHAT IT DOES: Kind $12's state 0: once the player's animation reaches step 2, his facing and place, then
 * flies at the screen's point ($80, $50) at speed $0400, animation $18 started.
 * HOW: RichterObjectFromPlayer, RichterObjectAtPlayerPlace; the delta to zTemp00-03; AngleFromDelta,
 * SetVelocityFromAngle with A pushed; a jump to RichterObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectStartAnimation.
 */
void RichterKind18Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->animation_at[0]));
    op_cmp(KIND18_STEP);
    if (!(cpu.p & P_Z)) {
        return;
    }
    JSR(0xD70D, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    JSR(0xD710, abi_exit_loaded_object(RichterObjectAtPlayerPlace(cpu.x)));
    op_lda(KIND18_AIM_X);
    op_sec();
    op_sbc(RD8(OBJ->x_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_cla();
    op_sbc(RD8(OBJ->x_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(KIND18_AIM_Y);
    op_sec();
    op_sbc(RD8(OBJ->y_lo[cpu.x]));
    WR8(ZP->temp02, cpu.a);
    op_cla();
    op_sbc(RD8(OBJ->y_hi[cpu.x]));
    WR8(ZP->temp03, cpu.a);
    JSR(0xD72F, angle_exit_shim(AngleFromDelta()));
    op_pha();
    op_lda(0);
    WR8(ZP->temp00, cpu.a);
    op_lda(4);
    WR8(ZP->temp01, cpu.a);
    op_pla();
    JSR(0xD73C, scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x)));
    op_lda(ANIM_KIND18);
    RichterObjectStartAnimation();
}

/*
 * @routine 01BF:741 RichterKind18Fly
 * @run 6E:D741
 * WHAT IT DOES: Kind $12 flying to the screen's middle: moved; there ($7C-$83 across, $48-$57 down), stopped;
 * once its animation ends, script 2 on slot 3, both halves get a hitbox $38 by $18 (crossed), on, hit timer
 * $0E, $10 frames, the next state; the second half at its place.
 * HOW: RichterObjectMove; RichterObjectStop, RichterObjectAnimationWait; StartSoleScriptSlot with X pushed;
 * runs on into RichterKind18Pair.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterKind18Pair, or the last compare.
 */
void RichterKind18Fly(void)
{
    uint8_t next;

    op_ldx(RD8(ZP->richter_object));
    JSR(0xD746, RichterObjectMove());
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_cmp(0x7C);
    if (!(cpu.p & P_C)) {
        RichterKind18Pair();
        return;
    }
    op_cmp(0x84);
    if (cpu.p & P_C) {
        RichterKind18Pair();
        return;
    }
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_cmp(0x48);
    if (!(cpu.p & P_C)) {
        RichterKind18Pair();
        return;
    }
    op_cmp(0x58);
    if (cpu.p & P_C) {
        RichterKind18Pair();
        return;
    }
    JSR(0xD75F, RichterObjectStop(cpu.x));
    if (object_wait(0xD762)) {
        return;
    }
    op_phx();
    op_ldx(KIND14_SCRIPT_SLOT);
    op_lda(KIND14_SCRIPT);
    JSR(0xD76A, sole_slot_exit_shim(StartSoleScriptSlot(cpu.a, cpu.x)));
    op_plx();
    next = (uint8_t)(cpu.x + 1);
    op_lda(KIND18_BOX_LONG);
    WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
    WR8(OBJ->hitbox_half_h[next], cpu.a);
    op_lda(KIND18_BOX_SHORT);
    WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
    WR8(OBJ->hitbox_half_w[next], cpu.a);
    op_lda(1);
    WR8(OBJ->hitbox_on[cpu.x], cpu.a);
    WR8(OBJ->hitbox_on[next], cpu.a);
    op_lda(KIND18_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    WR8(OBJ->work_n[next], cpu.a);
    op_lda(KIND18_HOLD);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], f_inc(RD8(OBJ->state[cpu.x])));
    RichterKind18Pair();
}

/*
 * @routine 01BF:7A0 RichterKind18Hold
 * @run 6E:D7A0
 * WHAT IT DOES: Kind $12 holding in the middle, marked; when its frames run out, Y speed entry 6, $FF frames,
 * the next state.
 * HOW: RichterKind18Mark, SetVelocityYFromTable, RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind18Hold(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xD7A5, RichterKind18Mark());
    WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(6);
    JSR(0xD7B0, velocity_y_from_table());
    op_lda(KIND18_RISE);
    WR8(OBJ->anim_timer[cpu.x], cpu.a);
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BF:7B8 RichterKind18Rise
 * @run 6E:D7B8
 * WHAT IT DOES: Kind $12 rising: when its frames run out, as RichterKind18End; moved, its half paired; past $80
 * frames left, its Y speed $10 less a frame; off the first screen, back on it with a step of $40 across and
 * sound $35 or $36 by bit 7 of the step's sum, paired again; marked.
 * HOW: RichterObjectMove, RichterKind18Pair, RichterObjectAddXLow, SoundCommand; runs on into
 * RichterKind18Mark.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterKind18Mark or RichterKind18End.
 */
void RichterKind18Rise(void)
{
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->anim_timer[cpu.x], f_dec(RD8(OBJ->anim_timer[cpu.x])));
    if (cpu.p & P_Z) {
        RichterKind18End();
        return;
    }
    JSR(0xD7C2, RichterObjectMove());
    JSR(0xD7C5, RichterKind18Pair());
    op_lda(RD8(OBJ->anim_timer[cpu.x]));
    op_cmp(KIND18_FALL_AT);
    if (cpu.p & P_C) {
        op_lda(0xF0);
        op_clc();
        op_adc(RD8(OBJ->vy_lo[cpu.x]));
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        op_lda(0xFF);
        op_adc(RD8(OBJ->vy_hi[cpu.x]));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    if (!(cpu.p & P_Z)) {
        WR8(OBJ->y_hi[cpu.x], 0);
        op_lda(0x40);
        JSR(0xD7EA, abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a)));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        op_tay();
        op_lda(rd8((uint16_t)(RichterKind18Sounds + cpu.y)));
        JSR(0xD7F5, sound_exit(cpu.a));
        JSR(0xD7F8, RichterKind18Pair());
    }
    RichterKind18Mark();
}

/*
 * @routine 01BF:804 RichterKind18Fade
 * @run 6E:D804
 * WHAT IT DOES: Kind $12's end begins: $40 rows of points, the count cleared, unmarked, the next state.
 * HOW: RichterObjectMarkOff; a jump to RichterObjectNextState.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterObjectNextState.
 */
void RichterKind18Fade(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(KIND18_ROWS);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    WR8(OBJ->work_e[cpu.x], 0);
    JSR(0xD811, op_lda(RichterObjectMarkOff(cpu.x)));
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BF:814 RichterKind18Points
 * @run 6E:D814
 * WHAT IT DOES: Kind $12's end: eleven points from $6C00 to the stage overlay's entry at $8009 each frame, its
 * OBJ_WORK_E in zTemp0F for each and what the stage's code leaves there kept; when its rows run out, both
 * halves emptied.
 * HOW: RichterDeathPointStep; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind18Points(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->work_e[cpu.x]));
    WR8(ZP->temp10, cpu.a);
    op_lda(0x6C);
    WR8(ZP->temp09, cpu.a);
    op_lda(0);
    WR8(ZP->temp08, cpu.a);
    op_ldx(KIND18_POINTS);
    do {
        op_lda(RD8(ZP->temp10));
        WR8(ZP->temp0f, cpu.a);
        JSR(0xD82C, RichterDeathPointStep());
    } while (!(cpu.p & P_Z));
    op_lda(RD8(ZP->temp0f));
    op_ldx(RD8(ZP->richter_object));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    WR8(OBJ->work_d[cpu.x], f_dec(RD8(OBJ->work_d[cpu.x])));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_inx();
    JSR(0xD83F, ClearObject(cpu.x));
    op_dex();
    ClearObject(cpu.x);
}

/*
 * @routine 01BF:6EC RichterKind18Frame
 * @run 6E:D6EC
 * WHAT IT DOES: Kind $12: runs its state's handler.
 * HOW: RichterStateDoubled; the seven words at $D6F2.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind18Frame(void)
{
    uint16_t handler;

    JSR(0xD6EF, state_doubled());
    handler = state_handler(RichterKind18States);
    switch (handler) {
    case 0xD700: RichterKind18Start(); break;
    case 0xD741: RichterKind18Fly(); break;
    case 0xD7A0: RichterKind18Hold(); break;
    case 0xD7B8: RichterKind18Rise(); break;
    case 0xD804: RichterKind18Fade(); break;
    case 0xD814: RichterKind18Points(); break;
    default: run_routine(handler); break;
    }
}

/*
 * @routine 01C0:06C RichterObjectNegateVelocityY
 * @run 6E:D86C
 * WHAT IT DOES: Turns an object's vertical velocity round.
 * HOW: A tail jump to NegateVelocityY (through its kernel vector).
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityY.
 */
void RichterObjectNegateVelocityY(void)
{
    negate_exit_shim(NegateVelocityY(cpu.x));
}

enum { KIND19_STEP = 2, FRAME_KIND19 = 0x12, KIND19_FIRST_LINK = 6, KIND19_LINK_END = 0x0E, KIND_LINK = 0x14,
       KIND19_STOPS = 8 };

/*
 * @routine 01C0:094 RichterKind19AtPlayer
 * @run 6E:D894
 * WHAT IT DOES: Kind $13 at the player, offset ($F2, $FB).
 * HOW: RichterObjectAtPlayer.
 * WAITS: no.
 * Entry: X the slot. Exit: as RichterObjectAtPlayer.
 */
void RichterKind19AtPlayer(void)
{
    op_lda(0xF2);
    WR8(ZP->temp04, cpu.a);
    op_lda(0xFB);
    WR8(ZP->temp06, cpu.a);
    JSR(0xD89F, abi_exit_sum(RichterObjectAtPlayer(cpu.x)));
}

/*
 * @routine 01C0:07B RichterKind19Start
 * @run 6E:D87B
 * WHAT IT DOES: Kind $13's state 0: once the player's animation reaches step 2, frame $12, sprite set 3, his
 * facing, the next state, at him.
 * HOW: RichterObjectFromPlayer, RichterObjectNextState; runs on into RichterKind19AtPlayer.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as RichterKind19AtPlayer, or the compare.
 */
void RichterKind19Start(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->animation_at[0]));
    op_cmp(KIND19_STEP);
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(FRAME_KIND19);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(SPRITE_SET_COMPANION);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    JSR(0xD891, from_player_exit(RichterObjectFromPlayer(cpu.x)));
    JSR(0xD894, abi_nz(RichterObjectNextState(cpu.x)));
    RichterKind19AtPlayer();
}

/*
 * @routine 01C0:0A0 RichterKind19Hold
 * @run 6E:D8A0
 * WHAT IT DOES: Kind $13 at the player; once his step is 0, X speed entry 5 by facing, frame 0, the next
 * state, and slots 6 to 13 copy its place as links (kind $14) with its facing and sprite flip.
 * HOW: RichterKind19AtPlayer, SetVelocityXFromTable, NegateVelocityXIfFacingLeft, RichterObjectNextState,
 * ClearObjectInYAndCopyPlace.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind19Hold(void)
{
    op_ldx(RD8(ZP->richter_object));
    JSR(0xD8A5, RichterKind19AtPlayer());
    op_lda(RD8(OBJ->substate[0]));
    if (!(cpu.p & P_Z)) {
        return;
    }
    op_lda(5);
    JSR(0xD8AF, velocity_x_from_table());
    JSR(0xD8B2, negate_if_facing_left());
    WR8(OBJ->frame[cpu.x], 0);
    JSR(0xD8B8, abi_nz(RichterObjectNextState(cpu.x)));
    op_ldy(KIND19_FIRST_LINK);
    do {
        JSR(0xD8BD, abi_ended(clear_in_y_exit_shim(ClearObjectInYAndCopyPlace(cpu.x, cpu.y))));
        op_lda(KIND_LINK);
        WR8(OBJ->kind[cpu.y], cpu.a);
        op_lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        op_lda(RD8(OBJ->sprite_flip[cpu.x]));
        WR8(OBJ->sprite_flip[cpu.y], cpu.a);
        op_iny();
        op_cpy(KIND19_LINK_END);
    } while (!(cpu.p & P_Z));
}

/*
 * @routine 01C0:0D8 RichterKind19Fly
 * @run 6E:D8D8
 * WHAT IT DOES: Kind $13 flying: up to 8 times, a stop (OBJ_WORK_L) takes its X speed away; moved with the
 * scroll; its X speed grows by the word at $D8D4 by facing; emptied once it is $20 into a screen beside.
 * HOW: OBJ_WORK_M the stops; RichterObjectScrollAndMove; ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last instruction left them.
 */
void RichterKind19Fly(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->work_m[cpu.x]));
    op_cmp(KIND19_STOPS);
    if (!(cpu.p & P_C)) {
        op_lda(RD8(OBJ->work_l[cpu.x]));
        if (!(cpu.p & P_Z)) {
            WR8(OBJ->work_l[cpu.x], 0);
            WR8(OBJ->vx_lo[cpu.x], 0);
            WR8(OBJ->vx_hi[cpu.x], 0);
            WR8(OBJ->work_m[cpu.x], f_inc(RD8(OBJ->work_m[cpu.x])));
        }
    }
    JSR(0xD8F5, RichterObjectScrollAndMove());
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RichterKind19Pulls + cpu.y)));
    op_clc();
    op_adc(RD8(OBJ->vx_lo[cpu.x]));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(RichterKind19Pulls + 1 + cpu.y)));
    op_adc(RD8(OBJ->vx_hi[cpu.x]));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (cpu.p & P_Z) {
        return;
    }
    if (cpu.p & P_N) {
        op_lda(RD8(OBJ->x_lo[cpu.x]));
        op_cmp(0xE0);
        if (cpu.p & P_C) {
            return;
        }
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_cmp(0x20);
    if (!(cpu.p & P_C)) {
        return;
    }
    ClearObject(cpu.x);
}

/*
 * @routine 01C0:06F RichterKind19Frame
 * @run 6E:D86F
 * WHAT IT DOES: Kind $13: runs its state's handler.
 * HOW: RichterStateDoubled; the three words at $D875.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the handler leaves them.
 */
void RichterKind19Frame(void)
{
    uint16_t handler;

    JSR(0xD872, state_doubled());
    handler = state_handler(RichterKind19States);
    switch (handler) {
    case 0xD87B: RichterKind19Start(); break;
    case 0xD8A0: RichterKind19Hold(); break;
    case 0xD8D8: RichterKind19Fly(); break;
    default: run_routine(handler); break;
    }
}

enum { KIND20_LEAD = 5, KIND20_HEAD = 6, FRAME_KIND20 = 0x2C, FRAME_KIND20_HEAD = 0x12, SOUND_KIND20 = 0x09,
       KIND20_HIT = 0x0A, KIND20_REACH_MAX = 0x60, KIND20_WIDTH_MAX = 0x18 };

/*
 * @routine 01C0:182 RichterKind20Struck
 * @run 6E:D982
 * WHAT IT DOES: Notes on slot 5's OBJ_WORK_L whether the object that tested this link is striking
 * (OBJ_HITBOX_ON 2 or more), clearing OBJ_HIT_BY_SLOT then.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the link. Exit: Y the tester's slot; A and the flags as the compare or the stores left them.
 */
void RichterKind20Struck(void)
{
    WR8(OBJ->work_l[KIND20_LEAD], 0);
    op_ldy(RD8(OBJ->hit_by_slot[cpu.x]));
    op_lda(RD8(OBJ->hitbox_on[cpu.y]));
    op_cmp(HITBOX_STRUCK);
    if (!(cpu.p & P_C)) {
        return;
    }
    op_lda(1);
    WR8(OBJ->work_l[KIND20_LEAD], cpu.a);
    WR8(OBJ->hit_by_slot[cpu.x], 0);
}

/*
 * @routine 01C0:198 RichterKind20Swing
 * @run 6E:D998
 * WHAT IT DOES: A link of kind $14 swinging round slot 5: the strike noted; its reach (OBJ_WORK_F-G) grows by
 * $0100 to $6000, its width (OBJ_WORK_H-I) by $40 to $1800; its angle (OBJ_WORK_E) 4 on; placed at slot 5
 * plus the reach by the angle's cosine across and the width by its sine down.
 * HOW: RichterKind20Struck; ScaleByCosine and ScaleBySine (through their kernel vectors), the product's high
 * byte with its sign.
 * WAITS: no.
 * Entry: X the link. Exit: A the Y high byte, N and Z from it, C and V from the add; Y as ScaleBySine left it.
 */
void RichterKind20Swing(void)
{
    JSR(0xD99A, RichterKind20Struck());
    op_lda(RD8(OBJ->work_g[cpu.x]));
    op_cmp(KIND20_REACH_MAX);
    if (!(cpu.p & P_C)) {
        op_lda(0);
        op_clc();
        op_adc(RD8(OBJ->work_f[cpu.x]));
        WR8(OBJ->work_f[cpu.x], cpu.a);
        op_lda(1);
        op_adc(RD8(OBJ->work_g[cpu.x]));
        WR8(OBJ->work_g[cpu.x], cpu.a);
    }
    op_lda(RD8(OBJ->work_i[cpu.x]));
    op_cmp(KIND20_WIDTH_MAX);
    if (!(cpu.p & P_C)) {
        op_lda(0x40);
        op_clc();
        op_adc(RD8(OBJ->work_h[cpu.x]));
        WR8(OBJ->work_h[cpu.x], cpu.a);
        op_lda(0);
        op_adc(RD8(OBJ->work_i[cpu.x]));
        WR8(OBJ->work_i[cpu.x], cpu.a);
    }
    op_lda(0);
    op_clc();
    op_adc(RD8(OBJ->work_d[cpu.x]));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(4);
    op_clc();
    op_adc(RD8(OBJ->work_e[cpu.x]));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_h[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_i[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    JSR(0xD9EC, scale_exit_shim(ScaleByCosine(cpu.a)));
    WR8(ZP->temp00, 0);
    op_lda(RD8(ZP->product.hi));
    if (cpu.p & P_N) {
        WR8(ZP->temp00, f_dec(RD8(ZP->temp00)));
    }
    op_lda(RD8(OBJ->x_lo[KIND20_LEAD]));
    op_clc();
    op_adc(RD8(ZP->product.hi));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_hi[KIND20_LEAD]));
    op_adc(RD8(ZP->temp00));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    JSR(0xDA15, scale_exit_shim(ScaleBySine(cpu.a)));
    WR8(ZP->temp00, 0);
    op_lda(RD8(ZP->product.hi));
    if (cpu.p & P_N) {
        WR8(ZP->temp00, f_dec(RD8(ZP->temp00)));
    }
    op_lda(RD8(OBJ->y_lo[KIND20_LEAD]));
    op_clc();
    op_adc(RD8(ZP->product.hi));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_hi[KIND20_LEAD]));
    op_adc(RD8(ZP->temp00));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
}

/*
 * @routine 01C0:145 RichterKind20Frame
 * @run 6E:D945
 * WHAT IT DOES: Kind $14, a link swinging round slot 5: emptied once slot 5 is empty; at state 0 its angle the
 * byte at $DA2F by slot, frame $2C, sprite set 1, the next state, the head (slot 6) frame $12, sprite set 3
 * and sound 9; hit timer $0A, hitbox on. Then as RichterKind20Swing.
 * HOW: RichterObjectNextState, SoundCommand, a tail jump to RichterObjectHitboxOn; RichterKind20Swing.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as its last step left them.
 */
void RichterKind20Frame(void)
{
    op_ldx(RD8(ZP->richter_object));
    op_lda(RD8(OBJ->kind[KIND20_LEAD]));
    if (cpu.p & P_Z) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!(cpu.p & P_Z)) {
        RichterKind20Swing();
        return;
    }
    op_lda(rd8((uint16_t)(RichterKind20Angles + cpu.x)));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(FRAME_KIND20);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(1);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    JSR(0xD967, abi_nz(RichterObjectNextState(cpu.x)));
    op_cpx(KIND20_HEAD);
    if (cpu.p & P_Z) {
        op_lda(FRAME_KIND20_HEAD);
        WR8(OBJ->frame[cpu.x], cpu.a);
        op_lda(SPRITE_SET_COMPANION);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_lda(SOUND_KIND20);
        JSR(0xD97A, sound_exit(cpu.a));
    }
    op_lda(KIND20_HIT);
    WR8(OBJ->work_n[cpu.x], cpu.a);
    hitbox_on_exit(RichterObjectHitboxOn(cpu.x));
}

/*
 * @routine 01C0:23D RichterObjectSlotLast
 * @run 6E:DA3D
 * WHAT IT DOES: RichterObjectSlot again, kind $15's handler: the same bytes at this address.
 * HOW: RichterObjectSlot's body.
 * WAITS: no.
 * Entry, exit: as RichterObjectSlot.
 */
uint8_t RichterObjectSlotLast(void)
{
    return object_slot_body();
}

void abi_RichterObjectSlotLast(void)
{
    op_ldx(RichterObjectSlotLast());
}

/* The wrappers: each body runs on the register file. */
void abi_RichterDeathPointStep(void) { RichterDeathPointStep(); }
void abi_RichterDeathPointRow(void) { RichterDeathPointRow(); }
void abi_RichterDeathFrame(void) { RichterDeathFrame(); }
void abi_RichterDeathPieceFrame(void) { RichterDeathPieceFrame(); }
void abi_RichterDeathObjects(void) { RichterDeathObjects(); }
void abi_RichterDeathPieceStart(void) { RichterDeathPieceStart(); }
void abi_RichterDeathPieceMove(void) { RichterDeathPieceMove(); }
void abi_RichterDeathPieceFly(void) { RichterDeathPieceFly(); }
void abi_RichterDeathPieceFall(void) { RichterDeathPieceFall(); }
void abi_RichterDeathPieceEnd(void) { RichterDeathPieceEnd(); }
void abi_RichterCompanionKind1(void) { RichterCompanionKind1(); }
void abi_RichterCompanionKind3(void) { RichterCompanionKind3(); }
void abi_RichterCompanionAtPlayer(void) { RichterCompanionAtPlayer(); }
void abi_RichterCompanionKind4(void) { RichterCompanionKind4(); }
void abi_RichterCompanionByKind(void) { RichterCompanionByKind(); }
void abi_RichterCompanions(void) { RichterCompanions(); }
void abi_RichterObjectByKind(void) { RichterObjectByKind(); }
void abi_RichterObjects(void) { RichterObjects(); }
void abi_RichterObjectMarkThenFromPlayer(void) { RichterObjectMarkThenFromPlayer(); }
void abi_RichterWhipSpark(void) { RichterWhipSpark(); }
void abi_RichterWhipStrike(void) { RichterWhipStrike(); }
void abi_RichterWhipFrame(void) { RichterWhipFrame(); }
void abi_RichterKind22Frame(void) { RichterKind22Frame(); }
void abi_RichterObjectStartAnimation(void) { RichterObjectStartAnimation(); }
void abi_RichterObjectFallCapped(void) { RichterObjectFallCapped(); }
void abi_RichterKind2Fly(void) { RichterKind2Fly(); }
void abi_RichterKind2Frame(void) { RichterKind2Frame(); }
void abi_RichterKind3Trail(void) { RichterKind3Trail(); }
void abi_RichterKind3Count(void) { RichterKind3Count(); }
void abi_RichterKind3Move(void) { RichterKind3Move(); }
void abi_RichterKind3Out(void) { RichterKind3Out(); }
void abi_RichterKind3Back(void) { RichterKind3Back(); }
void abi_RichterKind3Drop(void) { RichterKind3Drop(); }
void abi_RichterKind3Rise(void) { RichterKind3Rise(); }
void abi_RichterKind3Start(void) { RichterKind3Start(); }
void abi_RichterKind3Frame(void) { RichterKind3Frame(); }
void abi_RichterKind9Frame(void) { RichterKind9Frame(); }
void abi_RichterKind5Throw(void) { RichterKind5Throw(); }
void abi_RichterKind5Spread(void) { RichterKind5Spread(); }
void abi_RichterKind5Start(void) { RichterKind5Start(); }
void abi_RichterKind5Fly(void) { RichterKind5Fly(); }
void abi_RichterKind5Stuck(void) { RichterKind5Stuck(); }
void abi_RichterObjectFall38(void) { RichterObjectFall38(); }
void abi_RichterKind5Frame(void) { RichterKind5Frame(); }
void abi_RichterKind6Start(void) { RichterKind6Start(); }
void abi_RichterKind6Circle(void) { RichterKind6Circle(); }
void abi_RichterKind6Hold(void) { RichterKind6Hold(); }
void abi_RichterKind6Count(void) { RichterKind6Count(); }
void abi_RichterKind6Wait(void) { RichterKind6Wait(); }
void abi_RichterKind6Gather(void) { RichterKind6Gather(); }
void abi_RichterKind6Fade(void) { RichterKind6Fade(); }
void abi_RichterKind6Frame(void) { RichterKind6Frame(); }
void abi_RichterKind7End(void) { RichterKind7End(); }
void abi_RichterKind7Frame(void) { RichterKind7Frame(); }
void abi_RichterClaimKind3Slot(void) { RichterClaimKind3Slot(); }
void abi_RichterObjectProbe(void) { RichterObjectProbe(); }
void abi_RichterObjectProbeBelow(void) { RichterObjectProbeBelow(); }
void abi_RichterObjectHitAndMove(void) { RichterObjectHitAndMove(); }
void abi_RichterKind4LandMarked(void) { RichterKind4LandMarked(); }
void abi_RichterKind4Fall(void) { RichterKind4Fall(); }
void abi_RichterKind4Start(void) { RichterKind4Start(); }
void abi_RichterKind4Landed(void) { RichterKind4Landed(); }
void abi_RichterKind4Splash(void) { RichterKind4Splash(); }
void abi_RichterKind4Spawn(void) { RichterKind4Spawn(); }
void abi_RichterKind4Burn(void) { RichterKind4Burn(); }
void abi_RichterKind4Drop(void) { RichterKind4Drop(); }
void abi_RichterKind4Frame(void) { RichterKind4Frame(); }
void abi_RichterKind10Frame(void) { RichterKind10Frame(); }
void abi_RichterObjectMove(void) { RichterObjectMove(); }
void abi_RichterObjectScrollAndMove(void) { RichterObjectScrollAndMove(); }
void abi_RichterAngleToObject(void) { RichterAngleToObject(); }
void abi_RichterKind11Start(void) { RichterKind11Start(); }
void abi_RichterKind11Move(void) { RichterKind11Move(); }
void abi_RichterKind11Fly(void) { RichterKind11Fly(); }
void abi_RichterKind11Frame(void) { RichterKind11Frame(); }
void abi_RichterKind6CircleStart(void) { RichterKind6CircleStart(); }
void abi_RichterKind6CircleStep(void) { RichterKind6CircleStep(); }
void abi_RichterKind12Start(void) { RichterKind12Start(); }
void abi_RichterKind12Frame(void) { RichterKind12Frame(); }
void abi_RichterKind8Fly(void) { RichterKind8Fly(); }
void abi_RichterKind8Frame(void) { RichterKind8Frame(); }
void abi_RichterKind13Pieces(void) { RichterKind13Pieces(); }
void abi_RichterKind13Place(void) { RichterKind13Place(); }
void abi_RichterKind13AtPlayer(void) { RichterKind13AtPlayer(); }
void abi_RichterKind13Start(void) { RichterKind13Start(); }
void abi_RichterKind13Fly(void) { RichterKind13Fly(); }
void abi_RichterKind13Hold(void) { RichterKind13Hold(); }
void abi_RichterKind13Swing(void) { RichterKind13Swing(); }
void abi_RichterKind13Frame(void) { RichterKind13Frame(); }
void abi_RichterKind14Frame(void) { RichterKind14Frame(); }
void abi_RichterKind15Shot(void) { RichterKind15Shot(); }
void abi_RichterKind15Frame(void) { RichterKind15Frame(); }
void abi_RichterFindTarget(void) { RichterFindTarget(); }
void abi_RichterKind16Start(void) { RichterKind16Start(); }
void abi_RichterKind16Aim(void) { RichterKind16Aim(); }
void abi_RichterKind16Fly(void) { RichterKind16Fly(); }
void abi_RichterKind16Frame(void) { RichterKind16Frame(); }
void abi_RichterKind17Frame(void) { RichterKind17Frame(); }
void abi_RichterKind18Pair(void) { RichterKind18Pair(); }
void abi_RichterKind18Mark(void) { RichterKind18Mark(); }
void abi_RichterKind18End(void) { RichterKind18End(); }
void abi_RichterKind18Start(void) { RichterKind18Start(); }
void abi_RichterKind18Fly(void) { RichterKind18Fly(); }
void abi_RichterKind18Hold(void) { RichterKind18Hold(); }
void abi_RichterKind18Rise(void) { RichterKind18Rise(); }
void abi_RichterKind18Fade(void) { RichterKind18Fade(); }
void abi_RichterKind18Points(void) { RichterKind18Points(); }
void abi_RichterKind18Frame(void) { RichterKind18Frame(); }
void abi_RichterObjectNegateVelocityY(void) { RichterObjectNegateVelocityY(); }
void abi_RichterKind19AtPlayer(void) { RichterKind19AtPlayer(); }
void abi_RichterKind19Start(void) { RichterKind19Start(); }
void abi_RichterKind19Hold(void) { RichterKind19Hold(); }
void abi_RichterKind19Fly(void) { RichterKind19Fly(); }
void abi_RichterKind19Frame(void) { RichterKind19Frame(); }
void abi_RichterKind20Struck(void) { RichterKind20Struck(); }
void abi_RichterKind20Swing(void) { RichterKind20Swing(); }
void abi_RichterKind20Frame(void) { RichterKind20Frame(); }
