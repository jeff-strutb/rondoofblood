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

enum {
    OBJECT_MARK = 0x10, OBJECT_STATE_QUICK = 6, OBJECT_QUICKEN = 4, OBJECT_FLASH_FIRST = 0x8E,
    OBJECT_FLASH_BIT = 0x80, OBJECT_CHARGE_FROM = 0x0C, OBJECT_FLASH_BASE = 0x0E, PLAYER = 0
};

/* The frames a charged object flashes in place of: one flash frame for each. */
static const uint8_t flash_frames[] = { 0x07, 0x0E, 0x15, 0x1C };

/*
 * @routine 01BD:238 RichterObjectMarkOn
 * @run 6E:C238
 * WHAT IT DOES: Sets bit 4 of an object's OBJ_2B18.
 * HOW: ORed in.
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
 * Entry: X object. Exit: A the facing, N and Z from it, C as RichterObjectTake2FE8. X, Y and V
 * kept.
 */
ObjectFromPlayer RichterObjectFromPlayer(uint8_t object)
{
    ObjectFromPlayer from;

    from.player_2fe8 = RichterObjectTake2FE8(object);
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
 * Entry: X object, zTemp04 and zTemp06. Exit: A the Y position's high byte, N, Z, C and V from its
 * add. X and Y kept.
 */
Sum RichterObjectAtPlayer(uint8_t object)
{
    int16_t across = (int16_t)(int8_t)rd8(zTemp04);
    int16_t down = (int16_t)(int8_t)rd8(zTemp06);
    Sum low;

    if (rd8((uint16_t)(OBJ_2AC0 + object)) != 0)
        across = (int16_t)-across;
    wr8(zTemp04, (uint8_t)across);
    wr8(zTemp05, (uint8_t)((uint16_t)across >> 8));
    wr8(zTemp07, (uint8_t)((uint16_t)down >> 8));
    low = add_bytes(rd8(OBJ_X_LO + PLAYER), (uint8_t)across, false);
    wr8((uint16_t)(OBJ_X_LO + object), low.value);
    wr8((uint16_t)(OBJ_X_HI + object), add_bytes(rd8(OBJ_X_HI + PLAYER), (uint8_t)((uint16_t)across >> 8), low.carry).value);
    low = add_bytes(rd8(OBJ_Y_LO + PLAYER), (uint8_t)down, false);
    wr8((uint16_t)(OBJ_Y_LO + object), low.value);
    low = add_bytes(rd8(OBJ_Y_HI + PLAYER), (uint8_t)((uint16_t)down >> 8), low.carry);
    wr8((uint16_t)(OBJ_Y_HI + object), low.value);
    return low;
}

void abi_RichterObjectAtPlayer(void)
{
    Sum high = RichterObjectAtPlayer(cpu.x);

    cpu.a = high.value;
    abi_nz(high.value);
    abi_flag(P_C, high.carry);
    abi_flag(P_V, high.overflow);
}
