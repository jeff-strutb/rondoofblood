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
 * WHAT IT DOES: Sets bit 4 of an object's OBJ_2B18.
 * HOW: ORed in.
 * WAITS: no.
 * Entry: X object. Exit: A the new OBJ_2B18, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterObjectMarkOn(uint8_t object)
{
    uint8_t flags = (uint8_t)(rd8((uint16_t)(OBJ_2B18 + object)) | OBJECT_MARK);

    wr8((uint16_t)(OBJ_2B18 + object), flags);
    return flags;
}

void abi_RichterObjectMarkOn(void)
{
    cpu.a = RichterObjectMarkOn(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 01BD:3EC RichterObjectMarkOff
 * @run 6E:C3EC
 * WHAT IT DOES: Clears bit 4 of an object's OBJ_2B18.
 * HOW: ANDed out.
 * WAITS: no.
 * Entry: X object. Exit: A the new OBJ_2B18, N and Z from it. X, Y, C and V kept.
 */
uint8_t RichterObjectMarkOff(uint8_t object)
{
    uint8_t flags = (uint8_t)(rd8((uint16_t)(OBJ_2B18 + object)) & ~OBJECT_MARK);

    wr8((uint16_t)(OBJ_2B18 + object), flags);
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
 * WHAT IT DOES: Gives an object Richter's OBJ_2FE8, doubled.
 * HOW: Shifted up a place.
 * WAITS: no.
 * Entry: X object. Exit: A the value, N and Z from it, C Richter's bit 7. X, Y and V kept.
 */
uint8_t RichterObjectTake2FE8(uint8_t object)
{
    uint8_t player = rd8(OBJ_2FE8 + PLAYER);

    wr8((uint16_t)(OBJ_2FE8 + object), (uint8_t)(player << 1));
    return player;
}

void abi_RichterObjectTake2FE8(void)
{
    uint8_t player = RichterObjectTake2FE8(cpu.x);

    cpu.a = (uint8_t)(player << 1);
    abi_nz(cpu.a);
    abi_flag(P_C, (player & 0x80) != 0);
}

/*
 * @routine 01BD:3F8 RichterObjectFromPlayer
 * @run 6E:C3F8
 * WHAT IT DOES: Gives an object Richter's facing and his OBJ_2FE8, doubled.
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
    from.facing = rd8(OBJ_2AC0 + PLAYER);
    wr8((uint16_t)(OBJ_2AC0 + object), from.facing);
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
uint8_t RichterObjectSlot(void)
{
    return rd8(zRichterObject);
}

void abi_RichterObjectSlot(void)
{
    cpu.x = RichterObjectSlot();
    abi_nz(cpu.x);
}

/*
 * @routine 01BD:5C5 RichterObjectNextState
 * @run 6E:C5C5
 * WHAT IT DOES: Moves an object on to its next state.
 * HOW: OBJ_27A8 counted up.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state. A, X, Y, C and V kept.
 */
uint8_t RichterObjectNextState(uint8_t object)
{
    uint8_t state = (uint8_t)(rd8((uint16_t)(OBJ_27A8 + object)) + 1);

    wr8((uint16_t)(OBJ_27A8 + object), state);
    return state;
}

void abi_RichterObjectNextState(void)
{
    abi_nz(RichterObjectNextState(cpu.x));
}

/*
 * @routine 01BD:47C RichterObjectQuicken
 * @run 6E:C47C
 * WHAT IT DOES: Once, at its animation's step 6 and while PlayerByte3BE1 is set, makes an object's
 * frame last 4 longer and Richter's one frame shorter than that.
 * HOW: OBJ_35C0 marks it done; OBJ_2F38 the step; OBJ_2F90 the frame's time.
 * WAITS: no.
 * Entry: X object. Exit: X and Y kept. Stopped, A what was read last, N and Z from it, or from
 * comparing the step with 6 and C with it. Done, A Richter's new time, N and Z from the mark, C
 * and V from the add.
 */
Quicken RichterObjectQuicken(uint8_t object)
{
    Quicken quicken = { QUICKEN_DONE_BEFORE, 0, 0, { 0, false, false } };
    uint8_t time;

    quicken.value = rd8((uint16_t)(OBJ_35C0 + object));
    if (quicken.value != 0)
        return quicken;
    quicken.value = rd8((uint16_t)(OBJ_FLAGS_2F38 + object));
    quicken.how = QUICKEN_NOT_STEP;
    if (quicken.value != OBJECT_STATE_QUICK)
        return quicken;
    quicken.value = rd8(PlayerByte3BE1);
    quicken.how = QUICKEN_NOT_ASKED;
    if (quicken.value == 0)
        return quicken;
    quicken.sum = add_bytes(OBJECT_QUICKEN, rd8((uint16_t)(OBJ_2F90 + object)), false);
    wr8((uint16_t)(OBJ_2F90 + object), quicken.sum.value);
    time = (uint8_t)(quicken.sum.value - 1);
    wr8(OBJ_2F90 + PLAYER, time);
    quicken.value = time;
    quicken.mark = (uint8_t)(rd8((uint16_t)(OBJ_35C0 + object)) + 1);
    wr8((uint16_t)(OBJ_35C0 + object), quicken.mark);
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
 * of the four that have one, with its OBJ_2B18 for the flash.
 * HOW: OBJ_3568 is the charge; frames $07, $0E, $15 and $1C become $8E to $91; RichterFlashFlags
 * by OBJ_34B8 less $0E, with bit 7.
 * WAITS: no.
 * Entry: X object. Exit: X kept. Not charged, A 0, Z set, Y $8E. Not one of the four, A the
 * frame, N, Z and C from comparing it with $1C, Y $91. Flashed, A the flags, N and Z from them, Y
 * OBJ_34B8 less $0E, C and V from that subtract. V kept otherwise.
 */
Flash RichterObjectFlash(uint8_t object)
{
    Flash flash = { FLASH_NOT_CHARGED, 0, 0, 0 };

    flash.code = OBJECT_FLASH_FIRST;
    if (rd8((uint16_t)(OBJ_3568 + object)) == 0)
        return flash;
    flash.value = rd8((uint16_t)(OBJ_2A68 + object));
    flash.how = FLASH_OTHER_FRAME;
    for (unsigned i = 0; i < sizeof flash_frames; i++) {
        flash.code = (uint8_t)(OBJECT_FLASH_FIRST + i);
        if (flash.value == flash_frames[i]) {
            wr8((uint16_t)(OBJ_2A68 + object), flash.code);
            flash.base = rd8((uint16_t)(OBJ_34B8 + object));
            flash.value = (uint8_t)(rd8((uint16_t)(RichterFlashFlags + (uint8_t)(flash.base - OBJECT_FLASH_BASE))) |
                                    OBJECT_FLASH_BIT);
            wr8((uint16_t)(OBJ_2B18 + object), flash.value);
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
 * WHAT IT DOES: Charges an object from OBJ_34B8 $0C on, the first time the way it faces is newly
 * pressed.
 * HOW: OBJ_3568 counted up; RichterChargePads by OBJ_2AC0 against zPadPressed.
 * WAITS: no.
 * Entry: X object. Exit: X kept. Before $0C, A OBJ_34B8, N, Z and C from comparing it with $0C.
 * Charged already, A the charge, N and Z from it, C set. Not pressed, A 0, Z set, C set, Y the
 * facing. Charged now, A the press, N and Z from the charge, C set, Y the facing. V kept.
 */
Charge RichterObjectCharge(uint8_t object)
{
    Charge charge = { CHARGE_TOO_SOON, 0, 0, 0 };

    charge.value = rd8((uint16_t)(OBJ_34B8 + object));
    if (charge.value < OBJECT_CHARGE_FROM)
        return charge;
    charge.value = rd8((uint16_t)(OBJ_3568 + object));
    charge.how = CHARGE_ALREADY;
    if (charge.value != 0)
        return charge;
    charge.facing = rd8((uint16_t)(OBJ_2AC0 + object));
    charge.value = (uint8_t)(rd8(zPadPressed) & rd8((uint16_t)(RichterChargePads + charge.facing)));
    charge.how = CHARGE_NOT_PRESSED;
    if (charge.value == 0)
        return charge;
    charge.charge = (uint8_t)(rd8((uint16_t)(OBJ_3568 + object)) + 1);
    wr8((uint16_t)(OBJ_3568 + object), charge.charge);
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
 * @routine 01BD:00C RichterObjectAddXLow
 * @run 6E:C00C
 * WHAT IT DOES: Adds to an object's X position's low byte.
 * HOW: A plain add, the carry left for RichterObjectAddXHigh.
 * WAITS: no.
 * Entry: A the amount, X object. Exit: A the low byte, N, Z, C and V from the add. X and Y kept.
 */
Sum RichterObjectAddXLow(uint8_t object, uint8_t amount)
{
    Sum sum = add_bytes(amount, rd8((uint16_t)(OBJ_X_LO + object)), false);

    wr8((uint16_t)(OBJ_X_LO + object), sum.value);
    return sum;
}

void abi_RichterObjectAddXLow(void)
{
    abi_exit_sum(RichterObjectAddXLow(cpu.x, cpu.a));
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
Sum RichterObjectAddXHigh(uint8_t object, uint8_t amount, bool carry)
{
    Sum sum = add_bytes(amount, rd8((uint16_t)(OBJ_X_HI + object)), carry);

    wr8((uint16_t)(OBJ_X_HI + object), sum.value);
    return sum;
}

void abi_RichterObjectAddXHigh(void)
{
    abi_exit_sum(RichterObjectAddXHigh(cpu.x, cpu.a, (cpu.p & P_C) != 0));
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
 * HOW: OBJ_3250 cleared.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void RichterObjectHitboxOff(uint8_t object)
{
    wr8((uint16_t)(OBJ_3250 + object), 0);
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
    wr8((uint16_t)(OBJ_3930 + object), rd8((uint16_t)(RichterObjectHitboxes + entry)));
    wr8((uint16_t)(OBJ_39E0 + object), rd8((uint16_t)(RichterObjectHitboxes + entry + 1)));
    wr8((uint16_t)(OBJ_3988 + object), rd8((uint16_t)(RichterObjectHitboxes + entry + 2)));
    box.last = rd8((uint16_t)(RichterObjectHitboxes + entry + 3));
    wr8((uint16_t)(OBJ_3A38 + object), box.last);
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
 * HOW: RichterFrameHitboxes by OBJ_2A68; runs on into RichterObjectHitboxFrom.
 * WAITS: no.
 * Entry: X object. Exit: none, A 0, Z set, N clear, Y the frame. Otherwise as
 * RichterObjectHitboxFrom.
 */
Hitbox RichterObjectHitboxForFrame(uint8_t object)
{
    uint8_t frame = rd8((uint16_t)(OBJ_2A68 + object));
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
 * HOW: OBJ_3250 1; runs on into RichterObjectHitboxByFrame.
 * WAITS: no.
 * Entry: X object. Exit: as RichterObjectHitboxByFrame.
 */
Hitbox RichterObjectHitboxOn(uint8_t object)
{
    wr8((uint16_t)(OBJ_3250 + object), 1);
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
 * HOW: RichterObjectAnimations by OBJ_FIELDS_2EE0 into zTemp00 and 01; the step doubled.
 * WAITS: no.
 * Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
 * kept.
 */
ObjectListStep RichterObjectAnimationList(uint8_t object)
{
    ObjectListStep r;
    uint8_t doubled = (uint8_t)(RD8(OBJ->fields_2ee0[object]) << 1);
    uint8_t step;

    WR8(ZP->temp00, rd8((uint16_t)(RichterObjectAnimations + doubled)));
    WR8(ZP->temp01, rd8((uint16_t)(RichterObjectAnimations + 1 + doubled)));
    step = RD8(OBJ->flags[object]);
    r.y = (uint8_t)(step << 1);
    r.carry = (step & 0x80) != 0;
    return r;
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

/* The entry of the object's step in play: how long, then which frame; a mark of $FE starts the list over. */
static AnimationStep object_read_entry(uint8_t object)
{
    AnimationStep result;
    ObjectListStep list;
    uint8_t time, frame, y;

    for (;;) {
        jsr_push(0xDA6B);
        list = RichterObjectAnimationList(object);
        jsr_pop();
        y = list.y;
        time = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->f2f90[object], time);
        if (time == OBJECT_ANIMATION_STOPPED) {
            result = object_animation_exit(OBJECT_ANIMATION_STOPPED, 0, true, true, y);
            result.how = FRAME_END_MARK;
            return result;
        }
        if (time != OBJECT_ANIMATION_LOOPS) {
            break;
        }
        WR8(OBJ->flags[object], 0);
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
 * HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
 * WAITS: no.
 * Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
 */
AnimationStep RichterObjectSetAnimation(uint8_t object, uint8_t animation)
{
    WR8(OBJ->fields_2ee0[object], animation);
    WR8(OBJ->flags[object], 0);
    return object_read_entry(object);
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
 * HOW: OBJ_2F90 the timer, OBJ_FLAGS_2F38 the step, OBJ_2A68 the frame shown.
 * WAITS: no.
 * Entry: X object. Exit: as RichterAnimationTick.
 */
AnimationStep RichterObjectAnimationTick(uint8_t object)
{
    AnimationStep result;
    ObjectListStep list;
    uint8_t time = RD8(OBJ->f2f90[object]);
    uint8_t left, y, frame;

    if (time == OBJECT_ANIMATION_STOPPED) {
        return object_animation_exit(OBJECT_ANIMATION_STOPPED, 0, true, false, 0);
    }
    left = DEC8(OBJ->f2f90[object]);
    if (left == 0) {
        INC8(OBJ->flags[object]);
        return object_read_entry(object);
    }
    jsr_push(0xDA82);
    list = RichterObjectAnimationList(object);
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
AnimationStep RichterObjectAnimationRunning(uint8_t object)
{
    AnimationStep tick;
    uint8_t time;

    jsr_push(0xDA4B);
    tick = RichterObjectAnimationTick(object);
    jsr_pop();
    time = RD8(OBJ->f2f90[object]);
    return object_animation_exit(time, (uint8_t)(time - OBJECT_ANIMATION_STOPPED), time >= OBJECT_ANIMATION_STOPPED, tick.y_set, tick.y);
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

    jsr_push(0xDA43);
    r.step = RichterObjectAnimationTick(object);
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
