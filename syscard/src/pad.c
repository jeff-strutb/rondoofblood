/*
 * The BIOS's pad read.
 *
 * No record yet (row request to the typed access session): the joypad port at $1000, the five pads' current buttons at
 * $2228, their previous ones at $2232, the mask of the pads asked for at $2227, and the table of one bit a pad at
 * $E504. They are read by their addresses.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    PAD_PORT = 0x1000, PAD_NOW = 0x2228, PAD_BEFORE = 0x2232, PAD_ASKED = 0x2227, PAD_BIT_TABLE = 0xE504, PADS = 5,
    PAD_SELECT_LINE = 0x01, PAD_READ_BOTH = 0x03, PAD_LOW_NIBBLE = 0x0F, PAD_RUN_SELECT_NOW = 0x0C,
    PAD_RUN_SELECT_NEW = 0x04, RESET_MARK = 0x80, SOFT_RESET_JUMP_AT = 0xE4FB
};

/* The jump through the soft reset's vector: original code that does not come back, with the registers the BIOS holds there. */
static void soft_reset_shim(uint8_t y)
{
    uint8_t low = rd8(CardSoftResetLo);
    uint8_t high = rd8((uint16_t)(CardSoftResetLo + 1));
    uint16_t target = (uint16_t)(low | (high << 8));

    cpu.a = RESET_MARK;
    cpu.y = y;
    abi_nz(RESET_MARK);
    call_original_at(SOFT_RESET_JUMP_AT, target);
}

/*
 * @card 00:E49A ex_joysns
 * WHAT IT DOES: Reads the five pads, and works out which buttons each has newly pressed. Run and select
 * together on pad 1 (or any pad the mask asks for) is a soft reset.
 * HOW: Per pad, the port is written to select the high nibble, read (shifted up, with the pad's previous buttons kept),
 * written to select the low nibble, read, and the buttons made active high; what is new is what was not held before. Then the
 * pads in the mask are tested for run and select newly pressed together.
 * WAITS: no.
 * Entry: the mask of the pads asked for, at $2227. Exit: A the last value it loaded, Y 5, N 0, Z 1, C 1. X, V kept. Soft reset: the
 * reset mark set to $80 and a jump through CardSoftResetLo.
 * The pad port reads are by their address, with the stack's push and pull the original makes to wait on the port.
 */
CardExit ex_joysns(void)
{
    CardExit e = { 0 };
    uint8_t a;
    uint8_t y;
    uint8_t value;

    wr8(PAD_PORT, PAD_SELECT_LINE);
    wr8(PAD_PORT, PAD_READ_BOTH);
    for (y = 0; y < PADS; y++) {
        uint8_t kept;
        uint8_t high;
        uint8_t now;

        wr8(PAD_PORT, PAD_SELECT_LINE);
        push8(PAD_SELECT_LINE);
        (void)pull8();
        value = rd8((uint16_t)(PAD_NOW + y));
        wr8((uint16_t)(PAD_BEFORE + y), value);
        high = (uint8_t)(rd8(PAD_PORT) << 4);
        wr8((uint16_t)(PAD_NOW + y), high);
        wr8(PAD_PORT, 0);
        push8(high);
        (void)pull8();
        now = (uint8_t)(rd8(PAD_PORT) & PAD_LOW_NIBBLE);
        kept = rd8((uint16_t)(PAD_NOW + y));
        now = (uint8_t)((now | kept) ^ 0xFF);
        wr8((uint16_t)(PAD_NOW + y), now);
        value = rd8((uint16_t)(PAD_BEFORE + y));
        a = (uint8_t)(now ^ value);
        value = rd8((uint16_t)(PAD_NOW + y));
        a = (uint8_t)(a & value);
        wr8((uint16_t)(CardPadPressed + y), a);
    }
    for (y = 0; y < PADS; y++) {
        a = rd8(PAD_ASKED);
        a = (uint8_t)(a & rd8((uint16_t)(PAD_BIT_TABLE + y)));
        if (a == 0)
            continue;
        a = rd8((uint16_t)(CardPadPressed + y));
        if (a != PAD_RUN_SELECT_NEW)
            continue;
        a = rd8((uint16_t)(PAD_NOW + y));
        if (a != PAD_RUN_SELECT_NOW)
            continue;
        WR8(CZP->reset_mark, RESET_MARK);
        soft_reset_shim(y);
        break;
    }
    e.a = a;
    e.y = PADS;
    e.n = false;
    e.z = true;
    e.c = true;
    e.set_a = e.set_y = e.set_n = e.set_z = e.set_c = true;
    return e;
}

void abi_ex_joysns(void)
{
    card_exit_apply(ex_joysns());
}

/* ex_joysns's exit, for a caller that goes on with the registers and flags it leaves. */
void ex_joysns_exit_shim(CardExit e)
{
    card_exit_apply(e);
}
