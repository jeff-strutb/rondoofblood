/*
 * The card's start of day, from bank 1: the RAM cleared, the vectors and the drive set, the hardware put in its first state.
 *
 * No records yet (row request to the typed access session): the PSG's two ports at $0800, the timer at $0C00, the
 * interrupt controller at $1402, the pad port at $1000, and the card's bytes at $221F, $2226 and zero page $F3 and $F4.
 * They are written by their addresses.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    ZERO_PAGE = 0x2000, ZERO_PAGE_LEFT = 0x00FF, CARD_RAM = 0x2200, CARD_RAM_LEFT = 0x1DFF,
    PSG_SELECT = 0x0800, PSG_BALANCE = 0x0801, TIMER_COUNT = 0x0C00, TIMER_CONTROL = 0x0C01, IRQ_ACK = 0x1403,
    PAD_PORT = 0x1000, IRQ_MASK = 0x1402, CARD_BYTE_221F = 0x221F, CARD_BYTE_2226 = 0x2226, CARD_BYTE_2214_NEXT = 0x2215,
    ZP_F3 = 0xF3 + 0x2000, ZP_F4 = 0xF4 + 0x2000, MEMORY_WIDTH_START = 0x10, SAT_ADDRESS_LOW = 0x00, SAT_ADDRESS_HIGH = 0x10,
    SCREEN_COLUMNS = 0x20, SCREEN_ROWS = 0x1E, PADS_ASKED_ALL = 0x1F
};

/* TII: Y, A and X pushed, the bytes moved up one at a time, X, A and Y pulled. */
static void block_move_tii(uint16_t src, uint16_t dst, uint16_t length, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = rd8((uint16_t)(src + i));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/*
 * @card 01:CB64 init_card_hardware
 * WHAT IT DOES: Puts the machine in its first state: the PSG, timer and pad port written to zero, the interrupts off, the
 * display set to 32 columns by 30 rows, the sprite table's address chosen, the pads read twice and the sprite table cleared.
 * HOW: Zeros to the ports, a few of the card's bytes set, then the card's own entries called in turn: ex_hmode,
 * ex_irqoff, ex_scrmod, ex_scrsiz, ex_dmamod, ex_joysns twice with every pad asked for, ex_satclr and ex_sprdma.
 * WAITS: no.
 * Entry: nothing. Exit: as ex_sprdma.
 */
CardExit init_card_hardware(void)
{
    CardExit e = { 0 };

    WR8(CZP->vdc_reg, 0);
    wr8(PSG_SELECT, 0);
    wr8(PSG_BALANCE, 0);
    wr8(TIMER_COUNT, 0);
    wr8(TIMER_CONTROL, 0);
    wr8(IRQ_ACK, 0);
    wr8(PAD_PORT, 0);
    wr8(IRQ_MASK, 0);
    WR8(CZP->irq_flags, 0);
    wr8(CARD_BYTE_221F, 0);
    WR8(CRAM->vdc_mwr, MEMORY_WIDTH_START);
    wr8(ZP_F3, 0);
    wr8(ZP_F4, 0);
    jsr_push(0xCB8C);
    e = card_exit_over(e, ex_hmode());
    jsr_pop();
    jsr_push(0xCB8F);
    e = card_exit_over(e, ex_irqoff());
    jsr_pop();
    jsr_push(0xCB97);
    e = card_exit_over(e, ex_scrmod(0, SCREEN_COLUMNS, SCREEN_ROWS));
    jsr_pop();
    jsr_push(0xCB9C);
    e = card_exit_over(e, ex_scrsiz(0));
    jsr_pop();
    jsr_push(0xCBA1);
    e = card_exit_over(e, ex_dmamod(0));
    jsr_pop();
    WR8(CRAM->sat_address, SAT_ADDRESS_LOW);
    wr8(CARD_BYTE_2214_NEXT, SAT_ADDRESS_HIGH);
    wr8(CARD_BYTE_2226, 0);
    WR8(CRAM->pad_asked, 0);
    jsr_push(0xCBB4);
    e = card_exit_over(e, ex_joysns());
    jsr_pop();
    jsr_push(0xCBB7);
    e = card_exit_over(e, ex_joysns());
    jsr_pop();
    WR8(CRAM->pad_asked, PADS_ASKED_ALL);
    jsr_push(0xCBBF);
    e = card_exit_over(e, ex_satclr());
    jsr_pop();
    jsr_push(0xCBC2);
    e = card_exit_over(e, ex_sprdma());
    jsr_pop();
    return e;
}

void abi_init_card_hardware(void)
{
    card_exit_apply(init_card_hardware());
}

/*
 * @card 01:CB0F card_boot_init
 * WHAT IT DOES: Clears the card's RAM and starts it: the zero page and $2200 to $3FFF zeroed, the vectors set, the drive
 * reset, interrupts allowed, the hardware put in its first state, the display set up.
 * HOW: Two block moves that copy a zero upward, then init_card_vectors, reset_cd_unit, CLI, init_card_hardware, ex_irqon
 * and boot_display.
 * WAITS: through boot_display's frame.
 * Entry: A, X, Y (pushed by the block moves). Exit: as boot_display.
 */
CardExit card_boot_init(uint8_t a, uint8_t x, uint8_t y)
{
    CardExit e;
    CdSubcodeRead read;

    wr8(ZERO_PAGE, 0);
    block_move_tii(ZERO_PAGE, (uint16_t)(ZERO_PAGE + 1), ZERO_PAGE_LEFT, a, x, y);
    wr8(CARD_RAM, 0);
    block_move_tii(CARD_RAM, (uint16_t)(CARD_RAM + 1), CARD_RAM_LEFT, a, x, y);
    jsr_push(0xCB24);
    (void)init_card_vectors(x, y);
    jsr_pop();
    jsr_push(0xCB26);
    read = reset_cd_unit();
    jsr_pop();
    (void)read;
    cli();
    jsr_push(0xCB29);
    (void)init_card_hardware();
    jsr_pop();
    jsr_push(0xCB2C);
    (void)ex_irqon();
    jsr_pop();
    jsr_push(0xCB2F);
    e = boot_display();
    jsr_pop();
    return e;
}

void abi_card_boot_init(void)
{
    card_exit_apply(card_boot_init(cpu.a, cpu.x, cpu.y));
}
