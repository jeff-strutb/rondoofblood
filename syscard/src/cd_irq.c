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
    F5_HSYNC_USER = 0x40, F6_RASTER = 0x20, F6_VBLANK = 0x04
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

/* JSR into original code with no C (the unwritten reads of the disc): the registers are the ones given, and those it leaves come back. */
static void call_original_shim(uint16_t ret, uint16_t target, uint8_t *a, uint8_t *x, uint8_t *y, uint8_t *p)
{
    cpu.a = *a;
    cpu.x = *x;
    cpu.y = *y;
    abi_nz(*a);
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

/* The registers and flags a handler leaves, as it works them out instruction by instruction. */
typedef struct {
    uint8_t a, x, y;
    bool n, z, v, c;
    bool a_set, x_set, y_set, v_set, c_set;
} Trk;

static void t_nz(Trk *t, uint8_t value)
{
    t->n = (value & 0x80) != 0;
    t->z = value == 0;
}

/* LDA, AND or ORA: A the value, N and Z from it. */
static void t_a(Trk *t, uint8_t value)
{
    t->a = value;
    t->a_set = true;
    t_nz(t, value);
}

/* TST #mask, memory: N and V from the byte, Z from the mask AND it. */
static void t_tst(Trk *t, uint8_t mask, uint8_t memory)
{
    t->n = (memory & 0x80) != 0;
    t->v = (memory & 0x40) != 0;
    t->v_set = true;
    t->z = (mask & memory) == 0;
}

/* TSB or TRB with A the mask just loaded: N and V from the old byte, Z from A AND it. */
static void t_tb(Trk *t, uint8_t old)
{
    t_tst(t, t->a, old);
}

static void t_cmp(Trk *t, uint8_t value, uint8_t operand)
{
    t_nz(t, (uint8_t)(value - operand));
    t->c = value >= operand;
    t->c_set = true;
}

/* What a routine called leaves. */
static void t_take(Trk *t, CardExit e)
{
    if (e.set_a) {
        t->a = e.a;
        t->a_set = true;
    }
    if (e.set_x) {
        t->x = e.x;
        t->x_set = true;
    }
    if (e.set_y) {
        t->y = e.y;
        t->y_set = true;
    }
    if (e.set_nz)
        t_nz(t, e.nz);
    if (e.set_n)
        t->n = e.n;
    if (e.set_z)
        t->z = e.z;
    if (e.set_c) {
        t->c = e.c;
        t->c_set = true;
    }
    if (e.set_v) {
        t->v = e.v;
        t->v_set = true;
    }
}

static CardExit t_exit(const Trk *t)
{
    CardExit e = { 0 };

    e.a = t->a;
    e.x = t->x;
    e.y = t->y;
    e.n = t->n;
    e.z = t->z;
    e.v = t->v;
    e.c = t->c;
    e.set_a = t->a_set;
    e.set_x = t->x_set;
    e.set_y = t->y_set;
    e.set_n = e.set_z = true;
    e.set_v = t->v_set;
    e.set_c = t->c_set;
    return e;
}

/* LDA #mask; TRB or TSB $1802 (or the port named): the old byte is what the flags come from. */
static void t_mask(Trk *t, uint8_t mask, uint8_t old)
{
    t_a(t, mask);
    t_tb(t, old);
}

/* LDA #mask; TRB $F2: the bit the interrupt answered is no longer owed. */
static void clear_owed(Trk *t, uint8_t bit)
{
    uint8_t value = rd8(ZP_CD_OWED);

    wr8(ZP_CD_OWED, (uint8_t)(value & ~bit));
    t_mask(t, bit, value);
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
    Trk t = { 0 };
    uint8_t value;
    uint8_t held;
    uint8_t amount;

    t.x = x;
    t.y = y;
    held = RD8(CCDU->irq_control);
    t_a(&t, held);
    value = RD8(CCDU->irq_status);
    t_a(&t, (uint8_t)(t.a & value));
    value = rd8(ZP_CD_OWED);
    t_a(&t, (uint8_t)(t.a | value));
    wr8(ZP_CD_OWED, t.a);
    if ((rd8(ZP_CD_OWED) & OWED_DONE) != 0) {
        held = TRB8(CCDU->irq_control, OWED_DONE);
        t_mask(&t, OWED_DONE, held);
        clear_owed(&t, OWED_DONE);
        cli();
        value = rd8(CardAdpcmSectorsLeft);
        t_a(&t, value);
        value = rd8((uint16_t)(CardAdpcmSectorsLeft + 1));
        t_a(&t, (uint8_t)(t.a | value));
        value = rd8((uint16_t)(CardAdpcmSectorsLeft + 2));
        t_a(&t, (uint8_t)(t.a | value));
        if (t.a != 0) {
            value = RD8(CCDU->bus_status);
            t_a(&t, (uint8_t)(value & CD_BUSY));
            if (t.a != 0) {
                WR8(CRAM->cd_busy, 0);
                jsr_push(0xE771);
                ad_stop_clear();
                jsr_pop();
            } else {
                jsr_push(0xE776);
                clear_cd_command(t.x);
                jsr_pop();
                t_nz(&t, t.x);
                t_a(&t, CD_COMMAND_READ);
                WR8_IDX(COL(CRAM->cd_command), 0, CD_COMMAND_READ);
                value = RD8(CRAM->adpcm_next_sector.hi);
                t_a(&t, value);
                WR8_IDX(COL(CRAM->cd_command), 1, value);
                value = RD8(CRAM->adpcm_next_sector.mid);
                t_a(&t, value);
                WR8_IDX(COL(CRAM->cd_command), 2, value);
                value = RD8(CRAM->adpcm_next_sector.lo);
                t_a(&t, value);
                WR8_IDX(COL(CRAM->cd_command), 3, value);
                t.x = SECTORS_AT_ONCE;
                t.x_set = true;
                value = rd8((uint16_t)(CardAdpcmSectorsLeft + 1));
                t_a(&t, value);
                value = rd8((uint16_t)(CardAdpcmSectorsLeft + 2));
                t_a(&t, (uint8_t)(t.a | value));
                if (t.a == 0) {
                    value = rd8(CardAdpcmSectorsLeft);
                    t_a(&t, value);
                    t_cmp(&t, value, SECTORS_AT_ONCE);
                    if (value < SECTORS_AT_ONCE) {
                        t.x = value;
                        t_nz(&t, value);
                    }
                }
                amount = t.x;
                t_a(&t, amount);
                WR8(CRAM->adpcm_step, amount);
                WR8_IDX(COL(CRAM->cd_command), 4, amount);
                t_a(&t, 1);
                WR8(CRAM->cd_busy, 1);
                jsr_push(0xE7AE);
                t_take(&t, cd_send_command_start());
                jsr_pop();
                held = TSB8(CCDU->irq_control, IRQ_PLAY_BITS);
                t_mask(&t, IRQ_PLAY_BITS, held);
            }
        }
    }
    if ((rd8(ZP_CD_OWED) & OWED_DMA) != 0) {
        held = TRB8(CCDU->irq_control, OWED_DMA);
        t_mask(&t, OWED_DMA, held);
        clear_owed(&t, OWED_DMA);
        cli();
        t_a(&t, ADPCM_DMA_START);
        WR8(CCDU->adpcm_dma, ADPCM_DMA_START);
    }
    if ((rd8(ZP_CD_OWED) & OWED_DATA) != 0) {
        held = TRB8(CCDU->irq_control, OWED_DATA);
        t_mask(&t, OWED_DATA, held);
        clear_owed(&t, OWED_DATA);
        cli();
        value = RD8(CRAM->cd_busy);
        t_a(&t, value);
        if (value != 0)
            WR8(CCDU->adpcm_dma, 0);
        value = RD8(CCDU->data);
        t_a(&t, value);
        WR8(CRAM->cd_status, value);
        if (value != 0) {
            WR8(CRAM->cd_busy, 0);
            jsr_push(0xE7E8);
            ad_stop_clear();
            jsr_pop();
        }
        held = TSB8(CCDU->irq_control, CD_ACK);
        t_mask(&t, CD_ACK, held);
        do {
            value = RD8(CCDU->bus_status);
            t_tst(&t, CD_REQUEST, value);
        } while ((value & CD_REQUEST) != 0);
        held = TRB8(CCDU->irq_control, CD_ACK);
        t_mask(&t, CD_ACK, held);
        do {
            value = RD8(CCDU->bus_status);
            t_a(&t, (uint8_t)(value & PHASE_BITS));
            t_cmp(&t, t.a, PHASE_MESSAGE);
        } while (t.a != PHASE_MESSAGE);
        value = RD8(CCDU->data);
        t_a(&t, value);
        held = TSB8(CCDU->irq_control, CD_ACK);
        t_mask(&t, CD_ACK, held);
        do {
            value = RD8(CCDU->bus_status);
            t_tst(&t, CD_REQUEST, value);
        } while ((value & CD_REQUEST) != 0);
        held = TRB8(CCDU->irq_control, CD_ACK);
        t_mask(&t, CD_ACK, held);
        do {
            value = RD8(CCDU->bus_status);
            t_tst(&t, CD_BUSY, value);
        } while ((value & CD_BUSY) != 0);
        value = RD8(CRAM->cd_busy);
        t_a(&t, value);
        if (value != 0) {
            WR8(CRAM->cd_busy, 0);
            jsr_push(0xE825);
            t_take(&t, cplay_advance());
            jsr_pop();
            WR8(CRAM->adpcm_step, 0);
            held = TSB8(CCDU->irq_control, OWED_DONE);
            t_mask(&t, OWED_DONE, held);
        }
    }
    if ((rd8(ZP_CD_OWED) & OWED_SUBCODE) != 0) {
        held = TRB8(CCDU->irq_control, OWED_SUBCODE);
        t_mask(&t, OWED_SUBCODE, held);
        clear_owed(&t, OWED_SUBCODE);
        cli();
        jsr_push(0xE83D);
        value = cd_subrd_read();
        jsr_pop();
        t_a(&t, value);
        t.c = false;
        t.c_set = true;
        held = TSB8(CCDU->irq_control, OWED_SUBCODE);
        t_mask(&t, OWED_SUBCODE, held);
    }
    if ((rd8(ZP_CD_OWED) & OWED_ADPCM) != 0) {
        held = TRB8(CCDU->irq_control, IRQ_HALF_AND_END);
        t_mask(&t, IRQ_HALF_AND_END, held);
        clear_owed(&t, OWED_ADPCM);
        held = TRB8(CCDU->adpcm_control, IRQ_PLAY_BITS);
        t_mask(&t, IRQ_PLAY_BITS, held);
        jsr_push(0xE856);
        ad_stop_clear();
        jsr_pop();
        cli();
        value = RD8(CRAM->adpcm_active);
        t_a(&t, value);
        if (value != 0) {
            do {
                value = RD8(CCDU->adpcm_status);
                t_a(&t, (uint8_t)(value & ADPCM_BUSY));
            } while (t.a != 0);
            jsr_push(0xE866);
            t_take(&t, ad_play_latch());
            jsr_pop();
        }
    }
    return t_exit(&t);
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
            if (a == BOOT_RESET_MARK_ONE)
                {
                    uint8_t rp = 0;

                    call_original_shim(0xE8B1, 0xE6CF, &a, &x, &y, &rp);
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
