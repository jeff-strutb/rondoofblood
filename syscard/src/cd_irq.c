/*
 * The card's interrupt handlers: the CD unit's (IRQ2) and the VDC's (IRQ1), and the jumps through the vectors a program sets.
 *
 * No records yet (row request to the typed access session): the interrupt controller's mask at $1402 is CCDU's own
 * irq_control; the zero page bytes $F2 (the interrupts owed to the CD unit) and $F6 (the VDC's status) and the card's
 * bytes $224C and $2250 are read by their addresses; the user vectors at $2200 to $220B are by address too.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"
#include "card_exit.h"

enum {
    ZP_CD_OWED = 0x20F2, ZP_VDC_STATUS = 0x20F6, VECTOR_IRQ2 = 0x2200, VECTOR_IRQ1 = 0x2202, VECTOR_VSYNC = 0x2208,
    VECTOR_HSYNC = 0x220A, RASTER_LIST_A = 0x220C, RASTER_LIST_B = 0x2210, RASTER_LIST_C = 0x220E, RASTER_LIST_D = 0x2212,
    VDC_DATA = 0x0002, CD_ACK = 0x80, CD_REQUEST = 0x40, CD_BUSY = 0x80, PHASE_BITS = 0xF8, PHASE_MESSAGE = 0xF8,
    IRQ_SECTOR_DONE = 0x04, IRQ_DATA_READY = 0x20, IRQ_PCM = 0x10, IRQ_ADPCM = 0x08, IRQ_HALF_AND_END = 0x0C,
    IRQ_PLAY_BITS = 0x60, IRQ_DMA_STARTED = 0x40, IRQ_ADPCM_ENDED = 0x08,
    CD_COMMAND_READ = 0x08, SECTORS_AT_ONCE = 0x10, ADPCM_DMA_START = 0x02, STATUS_READ = 0x80,
    OWED_DONE = 0x04, OWED_DMA = 0x40, OWED_DATA = 0x20, OWED_SUBCODE = 0x10, OWED_ADPCM = 0x08,
    ADPCM_BUSY = 0x08, BOOT_RESET_MARK_ONE = 0x01, VDC_RASTER = 0x20, VDC_REG_CR = 0x05, VDC_REG_RCR = 0x07, VDC_REG_BXR = 0x08,
    F5_USER_IRQ2 = 0x01, F5_USER_IRQ1 = 0x02, F5_NO_LISTS = 0x20, F5_VSYNC_USER = 0x10, F5_NO_RASTER_LISTS = 0x80,
    F5_HSYNC_USER = 0x40, F6_RASTER = 0x20, F6_VBLANK = 0x04,
    MPR4_BIT = 0x10, MPR5_BIT = 0x20, MPR6_BIT = 0x40, PLAYER_BANK_4 = 0x22EB, PLAYER_BANK_5 = 0x22EC, PLAYER_MODE = 0x22DF,
    PLAYER_DONE = 0x22DE, PLAYER_TIMER = 0x22D4, PLAYER_PSG_SELECT = 0x22ED, TIMER_COUNT_PORT = 0x0C00, PSG_SELECT_PORT = 0x0800
};

/* JMP [vector]: the two bytes read, low first, and a jump into the original code they name, with the registers given. */
static void user_jump_shim(uint16_t at, uint16_t vector, uint8_t a, uint8_t x, uint8_t y)
{
    uint8_t low = rd8(vector);
    uint8_t high = rd8((uint16_t)(vector + 1));

    cpu.a = a;
    cpu.x = x;
    cpu.y = y;
    call_original_at(at, (uint16_t)(low | (high << 8)));
}

/*
 * @card 00:E733 jump_user_irq2
 * WHAT IT DOES: Goes to the program's IRQ2 handler.
 * HOW: JMP through $2200.
 * WAITS: no.
 * Entry, exit: the program's.
 */
void jump_user_irq2(uint8_t a, uint8_t x, uint8_t y)
{
    user_jump_shim(0xE733, VECTOR_IRQ2, a, x, y);
}

void abi_jump_user_irq2(void)
{
    jump_user_irq2(cpu.a, cpu.x, cpu.y);
}

/*
 * @card 00:E867 jump_user_hsync
 * WHAT IT DOES: Goes to the program's horizontal sync handler.
 * HOW: JMP through $220A.
 * WAITS: no.
 * Entry, exit: the program's.
 */
void jump_user_hsync(uint8_t a, uint8_t x, uint8_t y)
{
    user_jump_shim(0xE867, VECTOR_HSYNC, a, x, y);
}

void abi_jump_user_hsync(void)
{
    jump_user_hsync(cpu.a, cpu.x, cpu.y);
}

/*
 * @card 00:E86A jump_user_vsync
 * WHAT IT DOES: Goes to the program's vertical sync handler.
 * HOW: JMP through $2208.
 * WAITS: no.
 * Entry, exit: the program's.
 */
void jump_user_vsync(uint8_t a, uint8_t x, uint8_t y)
{
    user_jump_shim(0xE86A, VECTOR_VSYNC, a, x, y);
}

void abi_jump_user_vsync(void)
{
    jump_user_vsync(cpu.a, cpu.x, cpu.y);
}

/*
 * @card 00:E86D jump_user_irq1
 * WHAT IT DOES: Goes to the program's IRQ1 handler.
 * HOW: JMP through $2202.
 * WAITS: no.
 * Entry, exit: the program's.
 */
void jump_user_irq1(uint8_t a, uint8_t x, uint8_t y)
{
    user_jump_shim(0xE86D, VECTOR_IRQ1, a, x, y);
}

void abi_jump_user_irq1(void)
{
    jump_user_irq1(cpu.a, cpu.x, cpu.y);
}

/* The same with the flags the instruction before the JSR left: the N, Z and C (or V) bits in `flags` under `mask` are set, the rest kept. */
static void call_original_flags_shim(uint16_t ret, uint16_t target, uint8_t *a, uint8_t *x, uint8_t *y, uint8_t *p, uint8_t flags,
                                     uint8_t mask)
{
    cpu.a = *a;
    cpu.x = *x;
    cpu.y = *y;
    cpu.p = (uint8_t)((cpu.p & ~mask) | (flags & mask));
    call_orig(ret, target);
    *a = cpu.a;
    *x = cpu.x;
    *y = cpu.y;
    *p = cpu.p;
}

/* What a routine called leaves in the registers the handler goes on with. */
static void take(CardExit e, uint8_t *a, uint8_t *x, uint8_t *y)
{
    if (e.set_a)
        *a = e.a;
    if (e.set_x)
        *x = e.x;
    if (e.set_y)
        *y = e.y;
}

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

/* LDA #mask; TSB or TRB: A the mask, N and V from the byte before, Z when the byte written is zero. */
static CardExit exit_mask(uint8_t mask, uint8_t old, bool set)
{
    CardExit e = { 0 };
    uint8_t written = set ? (uint8_t)(old | mask) : (uint8_t)(old & ~mask);

    e.a = mask;
    e.n = (old & 0x80) != 0;
    e.v = (old & 0x40) != 0;
    e.z = written == 0;
    e.set_a = e.set_n = e.set_v = e.set_z = true;
    return e;
}

/* LDA #mask; TRB $F2: the bit the interrupt answered is no longer owed. */
static void clear_owed(uint8_t bit)
{
    uint8_t value = rd8(ZP_CD_OWED);

    wr8(ZP_CD_OWED, (uint8_t)(value & ~bit));
}

/* The answer to a sector of an ADPCM play read: the next ones asked for. What it leaves. */
static CardExit irq_sector_done(uint8_t x, uint8_t y)
{
    CardExit e;
    CardExit sent;
    uint8_t held;
    uint8_t value;
    uint8_t amount;

    held = TRB8(CCDU->irq_control, OWED_DONE);
    (void)held;
    clear_owed(OWED_DONE);
    cli();
    value = rd8(CardAdpcmSectorsLeft);
    value = (uint8_t)(value | rd8((uint16_t)(CardAdpcmSectorsLeft + 1)));
    value = (uint8_t)(value | rd8((uint16_t)(CardAdpcmSectorsLeft + 2)));
    e = exit_a(value);
    if (value == 0)
        return e;
    value = (uint8_t)(RD8(CCDU->bus_status) & CD_BUSY);
    e = exit_a(value);
    if (value != 0) {
        WR8(CRAM->cd_busy, 0);
        jsr_push(0xE771);
        ad_stop_clear();
        jsr_pop();
        return e;
    }
    jsr_push(0xE776);
    clear_cd_command(x);
    jsr_pop();
    WR8_IDX(COL(CRAM->cd_command), 0, CD_COMMAND_READ);
    value = RD8(CRAM->adpcm_next_sector.hi);
    WR8_IDX(COL(CRAM->cd_command), 1, value);
    value = RD8(CRAM->adpcm_next_sector.mid);
    WR8_IDX(COL(CRAM->cd_command), 2, value);
    value = RD8(CRAM->adpcm_next_sector.lo);
    WR8_IDX(COL(CRAM->cd_command), 3, value);
    amount = SECTORS_AT_ONCE;
    value = rd8((uint16_t)(CardAdpcmSectorsLeft + 1));
    value = (uint8_t)(value | rd8((uint16_t)(CardAdpcmSectorsLeft + 2)));
    if (value == 0) {
        value = rd8(CardAdpcmSectorsLeft);
        if (value < SECTORS_AT_ONCE)
            amount = value;
    }
    WR8(CRAM->adpcm_step, amount);
    WR8_IDX(COL(CRAM->cd_command), 4, amount);
    WR8(CRAM->cd_busy, 1);
    jsr_push(0xE7AE);
    sent = cd_send_command_start();
    jsr_pop();
    held = TSB8(CCDU->irq_control, IRQ_PLAY_BITS);
    e = card_exit_over(with_x(sent, sent.set_x ? sent.x : amount), exit_mask(IRQ_PLAY_BITS, held, true));
    (void)y;
    return e;
}

/* The answer to the data phase: the handshake of the data and the message bytes, and a play's next step. What it leaves. */
static CardExit irq_data(void)
{
    CardExit e;
    uint8_t held;
    uint8_t value;
    uint8_t bus;

    held = TRB8(CCDU->irq_control, OWED_DATA);
    (void)held;
    clear_owed(OWED_DATA);
    cli();
    value = RD8(CRAM->cd_busy);
    if (value != 0)
        WR8(CCDU->adpcm_dma, 0);
    value = RD8(CCDU->data);
    WR8(CRAM->cd_status, value);
    if (value != 0) {
        WR8(CRAM->cd_busy, 0);
        jsr_push(0xE7E8);
        ad_stop_clear();
        jsr_pop();
    }
    (void)TSB8(CCDU->irq_control, CD_ACK);
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & CD_REQUEST) != 0);
    (void)TRB8(CCDU->irq_control, CD_ACK);
    do {
        value = (uint8_t)(RD8(CCDU->bus_status) & PHASE_BITS);
    } while (value != PHASE_MESSAGE);
    (void)RD8(CCDU->data);
    (void)TSB8(CCDU->irq_control, CD_ACK);
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & CD_REQUEST) != 0);
    (void)TRB8(CCDU->irq_control, CD_ACK);
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & CD_BUSY) != 0);
    value = RD8(CRAM->cd_busy);
    e = with_c(with_v(exit_a(value), (bus & 0x40) != 0), true);     /* the C of the last compare of the message phase */
    if (value == 0)
        return e;
    WR8(CRAM->cd_busy, 0);
    jsr_push(0xE825);
    e = cplay_advance();
    jsr_pop();
    WR8(CRAM->adpcm_step, 0);
    held = TSB8(CCDU->irq_control, OWED_DONE);
    return card_exit_over(e, exit_mask(OWED_DONE, held, true));
}

/* The answer to a subcode byte ready. What it leaves. */
static CardExit irq_subcode(void)
{
    uint8_t held;

    held = TRB8(CCDU->irq_control, OWED_SUBCODE);
    (void)held;
    clear_owed(OWED_SUBCODE);
    cli();
    jsr_push(0xE83D);
    (void)cd_subrd_read();
    jsr_pop();
    held = TSB8(CCDU->irq_control, OWED_SUBCODE);
    return with_c(exit_mask(OWED_SUBCODE, held, true), false);
}

/* The answer to the ADPCM play half way or ended. What it leaves. */
static CardExit irq_adpcm(void)
{
    CardExit e;
    uint8_t held;
    uint8_t value;

    held = TRB8(CCDU->irq_control, IRQ_HALF_AND_END);
    (void)held;
    clear_owed(OWED_ADPCM);
    held = TRB8(CCDU->adpcm_control, IRQ_PLAY_BITS);
    jsr_push(0xE856);
    ad_stop_clear();
    jsr_pop();
    cli();
    value = RD8(CRAM->adpcm_active);
    e = with_v(exit_a(value), (held & 0x40) != 0);
    if (value == 0)
        return e;
    do {
        value = RD8(CCDU->adpcm_status);
    } while ((value & ADPCM_BUSY) != 0);
    jsr_push(0xE866);
    e = ad_play_latch();
    jsr_pop();
    return e;
}

/*
 * @card 00:E742 cd_interrupt
 * WHAT IT DOES: Answers what the CD unit's interrupt says: a sector of an ADPCM play read (the next asked for), the transfer to
 * ADPCM started, data ready (its handshake taken), a subcode byte ready, the ADPCM play half way or ended.
 * HOW: The enabled and raised bits of the unit are ORed into the owed bits at $F2, and each bit owed is answered in turn:
 * bit 2 the next block of an ADPCM play from the disc, bit 6 the transfer started, bit 5 the data phase and its handshake, bit 4
 * the subcode, bit 3 the ADPCM end; each clears its enable and its owed bit and allows interrupts again before it works.
 * WAITS: on the bus's request line in the handshake, and for the ADPCM unit to stop, with interrupts on.
 * Entry: X, Y (the interrupted code's). Exit: RTS to the handler, A, X, Y and the flags as the last step left them.
 */
CardExit cd_interrupt(uint8_t x, uint8_t y)
{
    uint8_t value;
    CardExit e;

    value = RD8(CCDU->irq_control);
    value = (uint8_t)(value & RD8(CCDU->irq_status));
    value = (uint8_t)(value | rd8(ZP_CD_OWED));
    wr8(ZP_CD_OWED, value);
    e = exit_a(value);
    if ((rd8(ZP_CD_OWED) & OWED_DONE) != 0)
        e = card_exit_over(e, irq_sector_done(x, y));
    if ((rd8(ZP_CD_OWED) & OWED_DMA) != 0) {
        (void)TRB8(CCDU->irq_control, OWED_DMA);
        clear_owed(OWED_DMA);
        cli();
        WR8(CCDU->adpcm_dma, ADPCM_DMA_START);
        e = card_exit_over(e, exit_a(ADPCM_DMA_START));
    }
    if ((rd8(ZP_CD_OWED) & OWED_DATA) != 0)
        e = card_exit_over(e, irq_data());
    if ((rd8(ZP_CD_OWED) & OWED_SUBCODE) != 0)
        e = card_exit_over(e, irq_subcode());
    if ((rd8(ZP_CD_OWED) & OWED_ADPCM) != 0)
        e = card_exit_over(e, irq_adpcm());
    return e;
}

void abi_cd_interrupt(void)
{
    card_exit_apply(cd_interrupt(cpu.x, cpu.y));
}

/*
 * @card 00:E736 card_irq2
 * WHAT IT DOES: The IRQ2 interrupt: the CD unit's. A program that has set its own handler (bit 0 of CardIrqFlags) has it;
 * otherwise cd_interrupt answers.
 * HOW: A, X and Y pushed, cd_interrupt called, Y, X and A pulled, RTI.
 * WAITS: through cd_interrupt.
 * Entry: interrupt; A, X, Y the interrupted code's. Exit: RTI, every register as it was.
 */
void card_irq2(uint8_t a, uint8_t x, uint8_t y)
{
    uint8_t flags = RD8(CZP->irq_flags);

    if ((flags & F5_USER_IRQ2) != 0) {
        user_jump_shim(0xE733, VECTOR_IRQ2, a, x, y);   /* the branch into jump_user_irq2: not an entry, the same call */
        return;
    }
    push8(a);
    push8(x);
    push8(y);
    jsr_push(0xE73E);
    (void)cd_interrupt(x, y);
    jsr_pop();
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

void abi_card_irq2(void)
{
    Cpu saved = cpu;

    card_irq2(cpu.a, cpu.x, cpu.y);
    cpu = saved;
}

/*
 * @card 00:E6CF run_bank2_tasks
 * WHAT IT DOES: Runs the card's CD player tasks in its bank 2: the first (at $CD12) when CardPlayerMode is not zero, the second (at $C2EF)
 * unless it is 1; each that answers 0 marks itself done in CardPlayerDone ($22DE: bit 6 for the first, bit 7 for the second). Then
 * the PSG's timer is loaded from $22D4 once, unless the reset mark's bit 0 is set, and the PSG's channel select put back.
 * HOW: MPR4, MPR5 and MPR6 kept on the stack; MPR4 and MPR5 set from $22EB and $22EC, MPR6 to bank 2; the tasks called; the MPRs
 * pulled back.
 * WAITS: no.
 * Entry: X, Y (the interrupted code's). Exit: A $22ED's byte, N and Z from it, C and V as the last compare left them. X, Y as the tasks left them.
 * The tasks themselves, in bank 2, are original code: no script runs them.
 */
CardExit run_bank2_tasks(uint8_t x, uint8_t y)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t a;
    uint8_t p = 0;
    uint8_t mode;

    value = tma(MPR4_BIT);
    push8(value);
    value = tma(MPR5_BIT);
    push8(value);
    value = tma(MPR6_BIT);
    push8(value);
    value = rd8(PLAYER_BANK_4);
    tam(MPR4_BIT, value);
    value = rd8(PLAYER_BANK_5);
    tam(MPR5_BIT, value);
    tam(MPR6_BIT, 2);
    mode = rd8(PLAYER_MODE);
    if (mode != 0) {
        a = mode;
        call_original_flags_shim(0xE6EE, 0xCD12, &a, &x, &y, &p, (uint8_t)(mode & 0x80 ? P_N : 0) | (mode == 0 ? P_Z : 0), P_N | P_Z);
        if (a == 0) {
            value = rd8(PLAYER_DONE);
            wr8(PLAYER_DONE, (uint8_t)(value | 0x40));
        }
    }
    mode = rd8(PLAYER_MODE);
    if (mode != 1) {
        uint8_t compare = (uint8_t)(mode - 1);
        uint8_t flags = (uint8_t)((compare & 0x80 ? P_N : 0) | (compare == 0 ? P_Z : 0) | (mode >= 1 ? P_C : 0));

        a = mode;
        call_original_flags_shim(0xE704, 0xC2EF, &a, &x, &y, &p, flags, P_N | P_Z | P_C);
        if (a == 0) {
            value = rd8(PLAYER_DONE);
            wr8(PLAYER_DONE, (uint8_t)(value | 0x80));
        }
    }
    value = RD8(CZP->reset_mark);
    if ((value & 1) == 0) {
        value = rd8(PLAYER_TIMER);
        if ((value & 0x80) == 0) {
            wr8(TIMER_COUNT_PORT, value);
            wr8(PLAYER_TIMER, (uint8_t)(value | 0x80));
        }
    }
    value = pull8();
    tam(MPR6_BIT, value);
    value = pull8();
    tam(MPR5_BIT, value);
    value = pull8();
    tam(MPR4_BIT, value);
    value = rd8(PLAYER_PSG_SELECT);
    wr8(PSG_SELECT_PORT, value);
    e.a = e.nz = value;
    e.x = x;
    e.y = y;
    e.c = true;
    e.v = (p & P_V) != 0;
    e.set_a = e.set_nz = e.set_x = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_run_bank2_tasks(void)
{
    card_exit_apply(run_bank2_tasks(cpu.x, cpu.y));
}

/*
 * @card 00:E870 card_irq1
 * WHAT IT DOES: The IRQ1 interrupt: the VDC's. A program that has set its own handler (bit 1 of CardIrqFlags) has it. Otherwise: on a
 * raster interrupt the VDC's control register copy is written, the frame counted, the colours and the pads taken unless the program
 * asked not to (bit 5), the program's vertical sync handler called (bit 4); on a vertical blank the raster lists written unless the
 * program asked not to (bit 7), the horizontal sync handler called (bit 6).
 * HOW: A, X and Y pushed, the VDC's status kept at $F6 and tested, TII block moves of the raster lists, ex_colorcmd, ex_joysns,
 * the register the VDC had selected put back, the registers pulled, RTI.
 * WAITS: no.
 * Entry: interrupt; A, X, Y the interrupted code's. Exit: RTI, every register as it was.
 */
void card_irq1(uint8_t a, uint8_t x, uint8_t y)
{
    uint8_t flags = RD8(CZP->irq_flags);
    uint8_t status;

    if ((flags & F5_USER_IRQ1) != 0) {
        user_jump_shim(0xE86D, VECTOR_IRQ1, a, x, y);   /* the branch into jump_user_irq1: not an entry, the same call */
        return;
    }
    push8(a);
    push8(x);
    push8(y);
    a = RD8(CVDC->addr);
    wr8(ZP_VDC_STATUS, a);
    status = rd8(ZP_VDC_STATUS);
    if ((status & F6_RASTER) != 0) {
        CardExit step;

        st0(VDC_REG_CR);
        a = RD8(CZP->vdc_cr.lo);
        WR8(CVDC->data_lo, a);
        a = RD8(CZP->vdc_cr.hi);
        WR8(CVDC->data_hi, a);
        (void)INC8(CRAM->frame_tick);
        flags = RD8(CZP->irq_flags);
        if ((flags & F5_NO_LISTS) == 0) {
            st0(VDC_REG_RCR);
            block_move_tii(RASTER_LIST_A, VDC_DATA, 2, a, x, y);
            st0(VDC_REG_BXR);
            block_move_tii(RASTER_LIST_B, VDC_DATA, 2, a, x, y);
            jsr_push(0xE8A5);
            step = ex_colorcmd(x, y);
            jsr_pop();
            take(step, &a, &x, &y);
            (void)INC8(CRAM->rnd_seed);
            a = RD8(CZP->reset_mark);
            if (a == BOOT_RESET_MARK_ONE) {
                jsr_push(0xE8B1);
                step = run_bank2_tasks(x, y);
                jsr_pop();
                take(step, &a, &x, &y);
            }
            jsr_push(0xE8B4);
            step = ex_joysns();
            jsr_pop();
            take(step, &a, &x, &y);
        }
        flags = RD8(CZP->irq_flags);
        if ((flags & F5_VSYNC_USER) != 0) {
            jsr_push(0xE8B9);
            jump_user_vsync(a, x, y);
            jsr_pop();
        }
    }
    status = rd8(ZP_VDC_STATUS);
    if ((status & F6_VBLANK) != 0) {
        flags = RD8(CZP->irq_flags);
        if ((flags & F5_NO_RASTER_LISTS) == 0) {
            jsr_push(0xE8C1);
            jsr_pop();
            jsr_push(0xE8C3);
            jsr_pop();
            st0(VDC_REG_RCR);
            block_move_tii(RASTER_LIST_C, VDC_DATA, 2, a, x, y);
            st0(VDC_REG_BXR);
            block_move_tii(RASTER_LIST_D, VDC_DATA, 2, a, x, y);
        }
        flags = RD8(CZP->irq_flags);
        if ((flags & F5_HSYNC_USER) != 0) {
            jsr_push(0xE8DA);
            jump_user_hsync(a, x, y);
            jsr_pop();
        }
    }
    a = RD8(CZP->vdc_reg);
    WR8(CVDC->addr, a);
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

void abi_card_irq1(void)
{
    Cpu saved = cpu;

    card_irq1(cpu.a, cpu.x, cpu.y);
    cpu = saved;
}
