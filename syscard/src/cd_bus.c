/*
 * The CD unit's bus: its handshake, the commands the BIOS sends and what it does with the answers.
 *
 * A routine's registers and flags at its exit are returned as a CardExit (syscard.h): the ones it sets, and the
 * rest left as the caller had them. A routine that pushes a register takes the value the register holds as an
 * argument; where a callee leaves it the exit says so.
 *
 * Bytes named by address, not yet by field: the command and result buffers, the saved arguments, the disc's
 * track and length; their array rows are asked of the typed access session.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    BUS_ACK = 0x80, BUS_REQUEST = 0x40, BUS_BUSY = 0x80, CD_RESET_LINE = 0x02,
    PHASE_BITS = 0xF8, PHASE_COMMAND = 0xD0, PHASE_ANSWER_BITS = 0xB8,
    PHASE_DATA_OUT_DONE = 0x98, PHASE_STATUS_DONE = 0x88, PHASE_DATA_DONE = 0x80,
    PHASE_STATUS = 0xD8, PHASE_MESSAGE = 0xF8, PHASE_DATA_IN = 0xC8,
    BUS_SELECT_ID = 0x81, BUS_RELEASE_ID = 0xFF, BUS_ABORT_IRQS = 0x60,
    COMMAND_NO_WAIT = 0xD9, ANSWER_TICKS = 0xFE, CD_ERROR_TIMED_OUT = 6, CD_ERROR_ASK_SENSE = 2,
    COMMAND_REQUEST_SENSE = 3, COMMAND_TEST_UNIT = 0, SENSE_BYTES = 10, SENSE_NOT_READY = 4,
    COMMAND_SUBQ = 0xDD, COMMAND_DINFO = 0xDE, DINFO_BYTES = 4, COMMAND_PAUSE = 0xDA,
    COMMAND_SEARCH = 0xD8, SEARCH_KIND_BITS = 0xC0, SEARCH_KIND_TRACK = 0x80, SEARCH_SIDE_BITS = 0x02,
    SEARCH_DONE_IRQ = 0x20, COMMAND_PLAY = 0xD9, PLAY_MODE_BITS = 0x07, PLAY_KIND_DISC_END = 0xC0,
    PLAY_KIND_TIME = 0x40, PLAY_MODE_KEEP_LAST = 4, PLAY_MODE_DONE = 1, PLAY_MODE_ON_INTERRUPT = 2,
    AD_DMA_FROM_CD = 0x02, AD_DMA_RUNNING = 0x03, AD_STATUS_DMA_BUSY = 0x04,
    CD_STATUS_TRANSFER_DONE = 0x20, AD_ERROR_BUSY = 4, COMMAND_READ = 8, READ_BYTES = 0,
    READ_SECTORS = 1, READ_TO_VRAM_FAST = 0xFF, READ_TO_VRAM_SLOW = 0xFE, READ_BAD_REQUEST = 0x22,
    SENSE_TRY_OTHER_BASE = 0x10, MPR_STUB_TMA = 0x43, MPR_STUB_TAM = 0x53, MPR_STUB_RTS = 0x60,
    CD_LAST_FRAME = 0x74, CD_LAST_SECOND = 0x59, FRAMES_LIMIT = 0x75, SECONDS_LIMIT = 0x60,
    BLOCK_PAGES = 8, VRAM_BLOCK_PAGES = 4, MPR_LIMIT = 7
};


/* A register set, with N and Z from it, as LDA, LDX and LDY leave it. */
static void ex_a(CardExit *r, uint8_t v)
{
    r->a = v;
    r->nz = v;
    r->set_a = r->set_nz = true;
}

static void ex_x(CardExit *r, uint8_t v)
{
    r->x = v;
    r->nz = v;
    r->set_x = r->set_nz = true;
}

static void ex_y(CardExit *r, uint8_t v)
{
    r->y = v;
    r->nz = v;
    r->set_y = r->set_nz = true;
}

static void ex_c(CardExit *r, bool c)
{
    r->c = c;
    r->set_c = true;
}

static void ex_v(CardExit *r, bool v)
{
    r->v = v;
    r->set_v = true;
}

/* A compare with A: N and Z from the difference, C from no borrow. */
static void compare_a(CardExit *r, uint8_t a, uint8_t operand)
{
    r->nz = (uint8_t)(a - operand);
    r->set_nz = true;
    ex_c(r, a >= operand);
}

/* The registers a routine is entered with, for the ones it pushes or reads on. */
static CardExit entered(uint8_t a, uint8_t x, uint8_t y)
{
    CardExit r = { 0 };

    r.a = a;
    r.x = x;
    r.y = y;
    r.set_a = r.set_x = r.set_y = true;
    return r;
}

/*
 * @card 00:EA35 cd_ack_on
 * WHAT IT DOES: Raises the bus's acknowledge.
 * HOW: Sets its bit in the control port, and gives what the port held.
 * WAITS: no.
 * Entry: nothing. Exit: A $80, Z clear, N and V bits 7 and 6 of what the port held. X, Y, C
 * kept.
 */
uint8_t cd_ack_on(void)
{
    return TSB8(CCDU->irq_control, BUS_ACK);
}

/* The exit of an ack on or off: A $80, N and V from what the port held, Z from what it was left holding. */
static CardExit ack_exit(uint8_t held, uint8_t left)
{
    CardExit e = { 0 };

    e.a = BUS_ACK;
    e.n = (held & 0x80) != 0;
    e.v = (held & 0x40) != 0;
    e.z = left == 0;
    e.set_a = e.set_n = e.set_v = e.set_z = true;
    return e;
}

void abi_cd_ack_on(void)
{
    uint8_t held = cd_ack_on();

    card_exit_apply(ack_exit(held, (uint8_t)(held | BUS_ACK)));
}

/*
 * @card 00:EA2F cd_ack_off
 * WHAT IT DOES: Lowers the bus's acknowledge.
 * HOW: Clears its bit in the control port, and gives what the port held.
 * WAITS: no.
 * Entry: nothing. Exit: A $80, Z if nothing else was set, N and V bits 7 and 6 of what the
 * port held. X, Y, C kept.
 */
uint8_t cd_ack_off(void)
{
    return TRB8(CCDU->irq_control, BUS_ACK);
}

void abi_cd_ack_off(void)
{
    uint8_t held = cd_ack_off();

    card_exit_apply(ack_exit(held, (uint8_t)(held & ~BUS_ACK)));
}

/*
 * @card 00:EA27 cd_ack_pulse
 * WHAT IT DOES: Acknowledges a byte on the bus.
 * HOW: Raises the acknowledge, waits for the drive to drop its request, lowers it.
 * WAITS: on the drive's request bit, with the stack as the caller's and the two bytes of its own frame.
 * Entry: nothing. Exit: as cd_ack_off, which it ends in without a frame. Returns what the port held.
 */
uint8_t cd_ack_pulse(void)
{
    uint8_t status;

    jsr_push(0xEA29);
    (void)cd_ack_on();
    jsr_pop();
    do {
        status = RD8(CCDU->bus_status);
    } while (status & BUS_REQUEST);
    return cd_ack_off();
}

void abi_cd_ack_pulse(void)
{
    uint8_t held = cd_ack_pulse();

    card_exit_apply(ack_exit(held, (uint8_t)(held & ~BUS_ACK)));
}

/*
 * @card 00:E8E3 cd_reset
 * WHAT IT DOES: Resets the CD unit.
 * HOW: Pulses its reset line, with pauses the C does not state.
 * WAITS: no.
 * Entry: nothing. Exit: A what the port was left holding, X 0, Y 0, N 0, Z 1, V bit 6 of what
 * the port first held. C kept.
 */
CardExit cd_reset(void)
{
    CardExit e = { 0 };
    uint8_t before = TSB8(CCDU->reset, CD_RESET_LINE);
    uint8_t after = RD8(CCDU->reset);

    after = (uint8_t)(after & ~CD_RESET_LINE);
    WR8(CCDU->reset, after);
    e.a = after;
    e.x = 0;
    e.y = 0;
    e.nz = 0;
    e.v = (before & 0x40) != 0;
    e.set_a = e.set_x = e.set_y = e.set_nz = e.set_v = true;
    return e;
}

void abi_cd_reset(void)
{
    card_exit_apply(cd_reset());
}

/* The bus phase, kept where the card keeps it: the status port's bits, less the low three. */
static uint8_t read_bus_phase(void)
{
    uint8_t status = RD8(CCDU->bus_status);
    uint8_t phase = (uint8_t)(status & PHASE_BITS);

    WR8(CRAM->bus_phase, phase);
    return phase;
}

/*
 * @card 00:E900 cd_send_command
 * WHAT IT DOES: Sends the command in CardCdCommand to the drive, and unless told not to,
 * waits for it to answer.
 * HOW: Waits for a transfer in progress, selects the drive, hands over bytes while the bus
 * is in its command phase. Then waits up to $FE frames for the request bit.
 * WAITS: yes, on the drive's bus and on the frame count the interrupt keeps, with the stack as the caller's.
 * Entry: the command built. Exit: A, N, Z, C and V as the path left them (the phase answered, the byte
 * that said something else is running, or 1 for a timeout); X the bytes sent, or the first frame count if it
 * waited. Y kept.
 */
CardExit cd_send_command(void)
{
    CardExit e = { 0 };
    uint8_t status;
    uint8_t phase;
    uint8_t x;
    uint8_t vbyte;
    uint8_t pending;
    uint8_t command;
    uint8_t first;
    uint8_t last;
    bool carry;
    bool again = false;
    bool selected = false;

    while (RD8(CRAM->cd_busy) != 0) {
    }
    WR8(CRAM->cd_timed_out, 0);
    while (!selected) {
        WR8(CCDU->data, BUS_SELECT_ID);
        status = RD8(CCDU->bus_status);
        vbyte = status;
        if (status & BUS_BUSY) {
            bool done = false;

            (void)TRB8(CCDU->irq_control, BUS_ABORT_IRQS);
            WR8(CCDU->bus_status, BUS_ABORT_IRQS);
            WR8(CCDU->data, BUS_RELEASE_ID);
            status = RD8(CCDU->bus_status);
            vbyte = status;
            if ((status & BUS_REQUEST) == 0) {
                continue;
            }
            do {
                jsr_push(0xE936);
                vbyte = cd_ack_pulse();
                jsr_pop();
                for (;;) {
                    status = RD8(CCDU->bus_status);
                    vbyte = status;
                    if (status & BUS_REQUEST) {
                        break;
                    }
                    status = RD8(CCDU->bus_status);
                    vbyte = status;
                    if ((status & BUS_BUSY) == 0) {
                        done = true;
                        break;
                    }
                }
            } while (!done);
            continue;
        }
        WR8(CCDU->bus_status, BUS_SELECT_ID);
        x = 0;
        do {
            status = RD8(CCDU->bus_status);
            if (status & BUS_REQUEST) {
                selected = true;
                break;
            }
            x--;
        } while (x != 0);
    }
    WR8(CRAM->bus_phase, 0);
    x = 0;
    for (;;) {
        phase = read_bus_phase();
        if (phase == PHASE_COMMAND) {
            uint8_t byte = RD8_IDX(COL(CRAM->cd_command), x);

            x++;
            WR8(CCDU->data, byte);
            jsr_push(0xE988);
            vbyte = cd_ack_pulse();
            jsr_pop();
            continue;
        }
        phase = (uint8_t)(phase & PHASE_ANSWER_BITS);
        if (phase == PHASE_DATA_OUT_DONE || phase == PHASE_STATUS_DONE || phase == PHASE_DATA_DONE) {
            break;
        }
    }
    e.x = x;
    e.v = (vbyte & 0x40) != 0;
    e.set_x = e.set_v = true;
    pending = RD8(CRAM->cd_busy);
    pending = (uint8_t)(pending | RD8(CRAM->cd_no_wait));
    if (pending != 0) {
        e.a = e.nz = pending;
        e.c = true;
        e.set_a = e.set_nz = e.set_c = true;
        return e;
    }
    command = RD8_IDX(COL(CRAM->cd_command), 0);
    carry = command >= COMMAND_NO_WAIT;
    if (command == COMMAND_NO_WAIT) {
        uint8_t sub = RD8_IDX(COL(CRAM->cd_command), 1);

        if (sub == 1 || sub == 2) {
            e.a = sub;
            e.nz = 0;
            e.c = true;
            e.set_a = e.set_nz = e.set_c = true;
            return e;
        }
        carry = sub >= 2;
    }
    first = RD8(CRAM->frame_tick);
    e.x = first;
    for (;;) {
        Sum difference;
        uint8_t elapsed;

        phase = read_bus_phase();
        if (phase & BUS_REQUEST) {
            e.a = phase;
            e.n = false;
            e.z = false;
            e.v = true;
            e.c = again ? false : carry;
            e.set_a = e.set_n = e.set_z = e.set_v = e.set_c = true;
            return e;
        }
        last = RD8(CRAM->frame_tick);
        difference = sub8(first, last, true);
        elapsed = (uint8_t)(0 - difference.value);
        if (elapsed >= ANSWER_TICKS) {
            WR8(CRAM->cd_timed_out, 1);
            e.a = e.nz = 1;
            e.c = true;
            e.v = difference.overflow;
            e.set_a = e.set_nz = e.set_c = e.set_v = true;
            return e;
        }
        again = true;
    }
}

void abi_cd_send_command(void)
{
    card_exit_apply(cd_send_command());
}

/*
 * @card 00:E9DC cd_read_status
 * WHAT IT DOES: Takes the drive's status byte and its message at the end of a command.
 * HOW: Waits for the status phase, takes the byte; then the message phase, or status again,
 * takes that; waits for the drive to leave the bus.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: nothing. Exit: A the status, N and Z from it, C 1, V bit 6 of the bus's last read. X
 * and Y kept.
 */
CardExit cd_read_status(void)
{
    CardExit e = { 0 };
    uint8_t phase;
    uint8_t status;
    uint8_t value;

    WR8(CRAM->cd_status, 0);
    do {
        do {
            phase = read_bus_phase();
        } while (phase != PHASE_STATUS);
        value = RD8(CCDU->data);
        WR8(CRAM->cd_status, value);
        jsr_push(0xE9F3);
        (void)cd_ack_on();
        jsr_pop();
        do {
            status = RD8(CCDU->bus_status);
        } while (status & BUS_REQUEST);
        jsr_push(0xE9FB);
        (void)cd_ack_off();
        jsr_pop();
        do {
            phase = read_bus_phase();
        } while (phase != PHASE_MESSAGE && phase != PHASE_STATUS);
    } while (phase != PHASE_MESSAGE);
    value = RD8(CCDU->data);
    WR8(CRAM->bus_phase, value);
    jsr_push(0xEA15);
    (void)cd_ack_on();
    jsr_pop();
    do {
        status = RD8(CCDU->bus_status);
    } while (status & BUS_REQUEST);
    jsr_push(0xEA1D);
    (void)cd_ack_off();
    jsr_pop();
    do {
        status = RD8(CCDU->bus_status);
    } while (status & BUS_BUSY);
    value = RD8(CRAM->cd_status);
    e.a = e.nz = value;
    e.c = true;
    e.v = (status & 0x40) != 0;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_cd_read_status(void)
{
    card_exit_apply(cd_read_status());
}

/*
 * @card 00:E9C5 cd_finish_command
 * WHAT IT DOES: Finishes a command: if the drive timed out resets it, else takes its status.
 * HOW: CardCdTimedOut. A zero status clears the sense code.
 * WAITS: only through cd_read_status.
 * Entry: nothing. Exit, timed out: A 6, N and Z from it, X 0, Y 0, V as cd_reset. Exit: as
 * cd_read_status.
 */
CardExit cd_finish_command(void)
{
    CardExit e;
    uint8_t timed_out = RD8(CRAM->cd_timed_out);

    if (timed_out != 0) {
        WR8(CRAM->cd_timed_out, 0);
        jsr_push(0xE9D0);
        e = cd_reset();
        jsr_pop();
        ex_a(&e, CD_ERROR_TIMED_OUT);
        return e;
    }
    jsr_push(0xE9D6);
    e = cd_read_status();
    jsr_pop();
    if (e.a == 0) {
        WR8(CRAM->cd_sense, 0);
    }
    return e;
}

void abi_cd_finish_command(void)
{
    card_exit_apply(cd_finish_command());
}

/*
 * @card 00:EB5E cd_drop_extra_data
 * WHAT IT DOES: Throws away what the drive still has to send, until it moves to its status.
 * HOW: Reads whole blocks through the auto port and drops them.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: nothing. Exit: A $D8, N 1, Z 0, C 1; X 0 and Y 0 if a block was read. V kept.
 */
CardExit cd_drop_extra_data(void)
{
    CardExit e = { 0 };
    uint8_t phase;

    for (;;) {
        phase = read_bus_phase();
        if (phase == PHASE_DATA_IN) {
            unsigned count = 8u * 256u;

            while (count != 0) {
                (void)RD8(CCDU->data_auto);
                count--;
            }
            e.x = 0;
            e.y = 0;
            e.set_x = e.set_y = true;
            continue;
        }
        if (phase == PHASE_STATUS) {
            break;
        }
    }
    phase = RD8(CRAM->bus_phase);
    e.a = e.nz = phase;
    e.c = true;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_cd_drop_extra_data(void)
{
    card_exit_apply(cd_drop_extra_data());
}

/*
 * @card 00:EA3B cd_read_data_in
 * WHAT IT DOES: Takes bytes from the drive to where _BX points, up to the count in _AX.
 * HOW: A byte and an acknowledge for each while the bus is in its data phase; if the drive
 * had more than was asked for, cd_drop_extra_data.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: _AX count, _BX pointer. Exit: A the phase's answer bits, N, Z, C from comparing it
 * with $88, or as cd_drop_extra_data. V from the last acknowledge, else kept.
 */
CardExit cd_read_data_in(void)
{
    CardExit e = { 0 };
    uint8_t phase;
    uint8_t value;
    uint8_t low;
    uint8_t held;

    for (;;) {
        phase = read_bus_phase();
        if (phase == PHASE_DATA_IN) {
            value = RD8(CCDU->data);
            WR8_PTR(CZP->b.lo, value);
            jsr_push(0xEA54);
            held = cd_ack_pulse();
            jsr_pop();
            e.v = (held & 0x40) != 0;
            e.set_v = true;
            if (INC8(CZP->b.lo) == 0) {
                (void)INC8(CZP->b.hi);
            }
            low = RD8(CZP->a.lo);
            if (low == 0) {
                (void)DEC8(CZP->a.hi);
            }
            low = (uint8_t)(low - 1);
            WR8(CZP->a.lo, low);
            value = RD8(CZP->a.hi);
            if ((uint8_t)(low | value) != 0) {
                continue;
            }
            break;
        }
        if (phase == PHASE_STATUS) {
            break;
        }
    }
    value = RD8(CCDU->bus_status);
    phase = (uint8_t)(value & PHASE_BITS);
    WR8(CRAM->bus_phase, phase);
    phase = (uint8_t)(phase & PHASE_ANSWER_BITS);
    if (phase == PHASE_STATUS_DONE) {
        return card_exit_over(e, cd_drop_extra_data());
    }
    e.a = phase;
    e.set_a = true;
    compare_a(&e, phase, PHASE_STATUS_DONE);
    return e;
}

void abi_cd_read_data_in(void)
{
    card_exit_apply(cd_read_data_in());
}

/* The sense codes cd_retry_after_error tells apart, in the order it compares them: the code, and whether to give up. */
static const struct {
    uint8_t code;
    bool give_up;
} SENSE_ANSWERS[] = {
    { 0x0B, true }, { 0x0D, true }, { 0x11, false }, { 0x15, false }, { 0x16, false }, { 0x1C, true },
    { 0x1D, true }, { 0x20, true }, { 0x21, true }, { 0x22, true }, { 0x25, true }, { 0x2A, true },
    { 0x2C, true }
};

/*
 * @card 00:F053 cd_request_sense
 * WHAT IT DOES: Asks the drive why the last command failed, and keeps its answer.
 * HOW: Command 3 for ten bytes into CardCdResult; on success bytes 2 and 9 go to the sense key
 * and sense code.
 * WAITS: only through the exchange.
 * Entry: X (only pushed). Exit: A the command's status, N and Z from it, C 1; V, X and Y as the
 * exchange left them. Clobbers _AX, _BX if the drive sent data.
 */
CardExit cd_request_sense(uint8_t x)
{
    CardExit r = entered(0, x, 0);
    CardExit step;
    uint8_t status;
    uint8_t value;

    r.set_a = r.set_y = false;
    jsr_push(0xF056);
    clear_cd_result(x);
    jsr_pop();
    jsr_push(0xF059);
    clear_cd_command(x);
    jsr_pop();
    WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_REQUEST_SENSE);
    WR8_IDX(COL(CRAM->cd_command), 4, SENSE_BYTES);
    jsr_push(0xF066);
    step = cd_send_command();
    jsr_pop();
    r = card_exit_over(r, step);
    if (r.a == PHASE_DATA_IN) {
        WR8(CZP->b.lo, 0x56);
        WR8(CZP->b.hi, 0x22);
        WR8(CZP->a.lo, SENSE_BYTES);
        WR8(CZP->a.hi, 0);
        jsr_push(0xF07B);
        step = cd_read_data_in();
        jsr_pop();
        r = card_exit_over(r, step);
    }
    jsr_push(0xF07E);
    step = cd_finish_command();
    jsr_pop();
    r = card_exit_over(r, step);
    status = r.a;
    push8(status);
    if (status == 0) {
        value = RD8_IDX(COL(CRAM->cd_result), 2);
        WR8(CRAM->cd_sense_key, value);
        value = RD8_IDX(COL(CRAM->cd_result), 9);
        WR8(CRAM->cd_sense, value);
    }
    r.a = pull8();
    r.nz = r.a;
    r.c = true;
    r.set_a = r.set_nz = r.set_c = true;
    return r;
}

void abi_cd_request_sense(void)
{
    card_exit_apply(cd_request_sense(cpu.x));
}

/*
 * @card 00:E20B cd_retry_after_error
 * WHAT IT DOES: Decides whether a failed command is worth trying again.
 * HOW: Only error 2 asks the drive why: keeps the command, requests sense, and gives up on the
 * sense codes that retrying cannot mend. Sense code 4 marks the drive not ready.
 * WAITS: only through the sense exchange.
 * Entry: A error, X (only pushed, in the sense request). Exit: C set to give up, A and N, Z as its last
 * compare left them; for error 2, Y $FF unless the exchange changed it, and X and V as the exchange left them.
 */
CardExit cd_retry_after_error(uint8_t error, uint8_t x)
{
    CardExit r = entered(error, x, 0);
    CardExit step;
    uint8_t sense;
    int y;
    bool give_up = false;

    r.set_x = r.set_y = false;
    compare_a(&r, error, CD_ERROR_TIMED_OUT);
    if (error != CD_ERROR_TIMED_OUT) {
        compare_a(&r, error, CD_ERROR_ASK_SENSE);
        if (error == CD_ERROR_ASK_SENSE) {
            uint8_t byte;

            y = SENSE_BYTES - 1;
            do {
                byte = RD8_IDX(COL(CRAM->cd_command), (unsigned)y);
                WR8_IDX(COL(CRAM->cd_failed_command), (unsigned)y, byte);
                y--;
            } while (y >= 0);
            r.y = 0xFF;
            r.set_y = true;
            jsr_push(0xE221);
            step = cd_request_sense(x);
            jsr_pop();
            r = card_exit_over(r, step);
            sense = RD8(CRAM->cd_sense);
            ex_a(&r, sense);
            if (sense != 0) {
                compare_a(&r, sense, SENSE_NOT_READY);
                if (sense == SENSE_NOT_READY) {
                    WR8(CRAM->cd_not_ready, sense);
                } else {
                    bool decided = false;

                    for (unsigned i = 0; i < sizeof SENSE_ANSWERS / sizeof SENSE_ANSWERS[0] && !decided; i++) {
                        compare_a(&r, sense, SENSE_ANSWERS[i].code);
                        if (sense == SENSE_ANSWERS[i].code) {
                            give_up = SENSE_ANSWERS[i].give_up;
                            decided = true;
                        }
                    }
                }
            }
        }
    }
    r.c = give_up;
    r.set_c = true;
    return r;
}

void abi_cd_retry_after_error(void)
{
    card_exit_apply(cd_retry_after_error(cpu.a, cpu.x));
}

/*
 * @card 00:F091 cd_test_unit
 * WHAT IT DOES: Asks whether the drive is ready.
 * HOW: A test command; on failure the retry decision, whose answer does not repeat it.
 * WAITS: only through the exchange.
 * Entry: X (only pushed). Exit, well: A 0, N 0, Z 1, C clear. Exit, failed: C set, A and N, Z as
 * cd_retry_after_error leaves them.
 */
CardExit cd_test_unit(uint8_t x)
{
    CardExit r = entered(0, x, 0);
    CardExit step;

    r.set_a = r.set_y = false;
    jsr_push(0xF093);
    clear_cd_command(x);
    jsr_pop();
    WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_TEST_UNIT);
    jsr_push(0xF09B);
    step = cd_send_command();
    jsr_pop();
    r = card_exit_over(r, step);
    jsr_push(0xF09E);
    step = cd_finish_command();
    jsr_pop();
    r = card_exit_over(r, step);
    compare_a(&r, r.a, 0);
    if (r.a == 0) {
        r.c = false;
        return r;
    }
    jsr_push(0xF0A5);
    step = cd_retry_after_error(r.a, r.x);
    jsr_pop();
    r = card_exit_over(r, step);
    r.c = true;
    return r;
}

void abi_cd_test_unit(void)
{
    card_exit_apply(cd_test_unit(cpu.x));
}

/*
 * @card 00:F347 cd_stat
 * WHAT IT DOES: Says whether the drive is busy; or, for any other request, tests the unit.
 * HOW: Zero reads the bus's busy bit; anything else is cd_test_unit.
 * WAITS: only through cd_test_unit.
 * Entry: A what, X (only pushed, in the test). Exit, zero: A the bit, N and Z from it, C 1. Exit: as
 * cd_test_unit, which it ends in without a frame.
 */
CardExit cd_stat(uint8_t what, uint8_t x)
{
    CardExit e = { 0 };
    uint8_t status;

    if (what != 0) {
        return cd_test_unit(x);
    }
    status = RD8(CCDU->bus_status);
    e.a = e.nz = (uint8_t)(status & BUS_BUSY);
    e.c = true;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_cd_stat(void)
{
    card_exit_apply(cd_stat(cpu.a, cpu.x));
}

/*
 * @card 00:EFBF cd_subq
 * WHAT IT DOES: Reads the drive's ten bytes of position to where _BX points.
 * HOW: Command $DD, tried until it works or the BIOS gives up.
 * WAITS: only through the exchange.
 * Entry: _BX pointer; A, X (pushed). Exit, well: A 0, N 0, Z 1, C 1. Exit, gave up: as
 * cd_retry_after_error. X, Y and V as the last exchange left them.
 */
CardExit cd_subq(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;

    r.set_y = false;
    jsr_push(0xEFC2);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        jsr_push(0xEFC5);
        clear_cd_command(r.x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_SUBQ);
        WR8_IDX(COL(CRAM->cd_command), 1, SENSE_BYTES);
        jsr_push(0xEFD2);
        step = cd_send_command();
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.a == PHASE_DATA_IN) {
            WR8(CZP->a.lo, SENSE_BYTES);
            WR8(CZP->a.hi, 0);
            jsr_push(0xEFDF);
            step = cd_read_data_in();
            jsr_pop();
            r = card_exit_over(r, step);
        }
        jsr_push(0xEFE2);
        step = cd_finish_command();
        jsr_pop();
        r = card_exit_over(r, step);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        jsr_push(0xEFE9);
        step = cd_retry_after_error(r.a, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.c) {
            return r;
        }
        jsr_push(0xEFEE);
        restore_card_args(r.a, r.x);
        jsr_pop();
    }
}

void abi_cd_subq(void)
{
    card_exit_apply(cd_subq(cpu.a, cpu.x));
}

/*
 * @card 00:EFF1 cd_dinfo
 * WHAT IT DOES: Reads a piece of the disc's table of contents to where _BX points.
 * HOW: Command $DE with _AX as what is asked for; the answer is four bytes.
 * WAITS: only through the exchange.
 * Entry: _AX what, _BX pointer; A, X (pushed). Exit: as cd_subq. _AX and _BX as they were.
 * Not run by any script.
 */
CardExit cd_dinfo(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;
    uint8_t value;
    uint8_t status;

    r.set_y = false;
    jsr_push(0xEFF4);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        jsr_push(0xEFF7);
        clear_cd_command(r.x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_DINFO);
        value = RD8(CZP->a.lo);
        WR8_IDX(COL(CRAM->cd_command), 1, value);
        value = RD8(CZP->a.hi);
        WR8_IDX(COL(CRAM->cd_command), 2, value);
        jsr_push(0xF009);
        step = cd_send_command();
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.a == PHASE_DATA_IN) {
            WR8(CZP->a.lo, DINFO_BYTES);
            WR8(CZP->a.hi, 0);
            jsr_push(0xF016);
            step = cd_read_data_in();
            jsr_pop();
            r = card_exit_over(r, step);
        }
        jsr_push(0xF019);
        step = cd_finish_command();
        jsr_pop();
        r = card_exit_over(r, step);
        status = r.a;
        push8(status);
        jsr_push(0xF01D);
        restore_card_args(r.a, r.x);
        jsr_pop();
        r.a = pull8();
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        jsr_push(0xF025);
        step = cd_retry_after_error(r.a, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.c) {
            return r;
        }
        jsr_push(0xF02A);
        restore_card_args(r.a, r.x);
        jsr_pop();
    }
}

void abi_cd_dinfo(void)
{
    card_exit_apply(cd_dinfo(cpu.a, cpu.x));
}

/*
 * @card 00:EF94 cd_pause
 * WHAT IT DOES: Pauses the drive if it is playing.
 * HOW: cd_subq; a first byte not zero says nothing plays. Otherwise command $DA, and on
 * failure the retry decision, whose try starts again from cd_subq.
 * WAITS: only through the exchanges.
 * Entry: X (pushed). Exit, not playing: A 0, N and Z from the byte, C 1. Exit, paused: A 0, N 0,
 * Z 1, C 1. Exit, gave up: as cd_retry_after_error. Clobbers _BX.
 */
CardExit cd_pause(uint8_t x)
{
    CardExit r = entered(0, x, 0);
    CardExit step;
    uint8_t first;

    r.set_a = r.set_y = false;
    for (;;) {
        WR8(CZP->b.lo, 0x56);
        WR8(CZP->b.hi, 0x22);
        jsr_push(0xEF9E);
        step = cd_subq(0x22, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        first = RD8_IDX(COL(CRAM->cd_result), 0);
        compare_a(&r, first, 0);
        if (first != 0) {
            r.a = 0;
            r.set_a = true;
            return r;
        }
        jsr_push(0xEFAA);
        clear_cd_command(r.x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_PAUSE);
        jsr_push(0xEFB2);
        step = cd_send_command();
        jsr_pop();
        r = card_exit_over(r, step);
        jsr_push(0xEFB5);
        step = cd_finish_command();
        jsr_pop();
        r = card_exit_over(r, step);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        jsr_push(0xEFBC);
        step = cd_retry_after_error(r.a, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.c) {
            return r;
        }
    }
}

void abi_cd_pause(void)
{
    card_exit_apply(cd_pause(cpu.x));
}

/*
 * @card 00:EF34 cd_search
 * WHAT IT DOES: Seeks to a place on the disc; with bit 0 of _BH, returns at once and lets an
 * interrupt report the end.
 * HOW: Command $D8. Bits 6-7 of _BH say what the address is.
 * WAITS: only through the exchange.
 * Entry: _AX, _BL the place, _BH the mode; A, X (pushed). Exit, on an interrupt: A 0, Z clear, N and V
 * from the control port, C as the send left it. Exit: as cd_subq, with Y the command offset unless a step
 * changed it.
 */
CardExit cd_search(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;
    uint8_t mode;
    uint8_t kind;
    uint8_t held;

    r.set_y = false;
    jsr_push(0xEF37);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        jsr_push(0xEF3A);
        clear_cd_command(r.x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_SEARCH);
        mode = RD8(CZP->b.hi);
        kind = (uint8_t)(mode & SEARCH_KIND_BITS);
        WR8_IDX(COL(CRAM->cd_command), 9, kind);
        r.a = kind;
        r.nz = kind;
        r.set_a = r.set_nz = true;
        if (kind == 0) {
            r.x = 0;
            r.y = 3;
            r.set_x = r.set_y = true;
            jsr_push(0xEF4E);
            step = (CardExit){ 0 };
            step.a = copy_args_to_command(0, 3);
            jsr_pop();
            step.nz = step.a;
            step.set_a = step.set_nz = true;
            r = card_exit_over(r, step);
        } else {
            if (kind == SEARCH_KIND_TRACK) {
                WR8(CZP->a.hi, 0);
                WR8(CZP->b.lo, 0);
            }
            r.x = 0;
            r.y = 2;
            r.set_x = r.set_y = true;
            jsr_push(0xEF5E);
            step = (CardExit){ 0 };
            step.a = copy_args_to_command(0, 2);
            jsr_pop();
            step.nz = step.a;
            step.set_a = step.set_nz = true;
            r = card_exit_over(r, step);
        }
        mode = RD8(CZP->b.hi);
        r.x = mode;
        r.set_x = true;
        WR8(CRAM->cd_no_wait, (uint8_t)(mode & 1));
        WR8_IDX(COL(CRAM->cd_command), 1, (uint8_t)((mode & SEARCH_SIDE_BITS) >> 1));
        jsr_push(0xEF70);
        step = cd_send_command();
        jsr_pop();
        r = card_exit_over(r, step);
        if (RD8(CRAM->cd_no_wait) != 0) {
            WR8(CRAM->cd_busy, 0);
            held = TSB8(CCDU->irq_control, SEARCH_DONE_IRQ);
            WR8(CRAM->cd_no_wait, 0);
            r.a = 0;
            r.set_a = true;
            r.n = (held & 0x80) != 0;
            r.v = (held & 0x40) != 0;
            r.z = false;
            r.set_n = r.set_v = r.set_z = true;
            r.set_nz = false;
            return r;
        }
        jsr_push(0xEF85);
        step = cd_finish_command();
        jsr_pop();
        r = card_exit_over(r, step);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        jsr_push(0xEF8C);
        step = cd_retry_after_error(r.a, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.c) {
            return r;
        }
        jsr_push(0xEF91);
        restore_card_args(r.a, r.x);
        jsr_pop();
    }
}

void abi_cd_search(void)
{
    card_exit_apply(cd_search(cpu.a, cpu.x));
}

/* One byte less another in decimal mode, with its carry in as no borrow: the digits, and the carry out. */
static Sum sub8_decimal(uint8_t a, uint8_t b, bool carry_in)
{
    Sum s;
    int low = (a & 0x0F) - (b & 0x0F) - (carry_in ? 0 : 1);
    int high = (a >> 4) - (b >> 4);

    if (low < 0) {
        low = (low - 6) & 0x0F;
        high -= 1;
    }
    s.carry = high >= 0;
    if (high < 0) {
        high = (high - 6) & 0x0F;
    }
    s.value = (uint8_t)((high << 4) | low);
    s.overflow = false;
    return s;
}

/*
 * @card 00:EF02 disc_end_minus_one_frame
 * WHAT IT DOES: Takes one frame off the time kept in CardDiscEnd.
 * HOW: Minutes, seconds and frames in packed decimal: below 0 frames go to 74 and borrow a
 * second, below 0 seconds go to 59 and borrow a minute.
 * WAITS: no (the decimal flag is set inside, so an interrupt would run with it set).
 * Entry: A (pushed), CardDiscEnd. Exit: A kept, N and Z from it; C from the last digit's step. X, Y, V kept.
 * Returns the C and the A pulled.
 */
CardExit disc_end_minus_one_frame(uint8_t a)
{
    CardExit e = { 0 };
    Sum step;
    uint8_t value;
    bool carry;

    push8(a);
    value = RD8_IDX(COL(CRAM->disc_end), 2);
    step = sub8_decimal(value, 1, true);
    WR8_IDX(COL(CRAM->disc_end), 2, step.value);
    carry = step.value >= FRAMES_LIMIT;
    if (!carry) {
        carry = step.value >= FRAMES_LIMIT;
    }
    if (step.value >= FRAMES_LIMIT) {
        WR8_IDX(COL(CRAM->disc_end), 2, CD_LAST_FRAME);
        value = RD8_IDX(COL(CRAM->disc_end), 1);
        step = sub8_decimal(value, 1, true);
        WR8_IDX(COL(CRAM->disc_end), 1, step.value);
        carry = step.value >= SECONDS_LIMIT;
        if (step.value >= SECONDS_LIMIT) {
            WR8_IDX(COL(CRAM->disc_end), 1, CD_LAST_SECOND);
            value = RD8_IDX(COL(CRAM->disc_end), 0);
            step = sub8_decimal(value, 1, true);
            WR8_IDX(COL(CRAM->disc_end), 0, step.value);
            carry = step.carry;
        }
    }
    e.a = e.nz = pull8();
    e.c = carry;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_disc_end_minus_one_frame(void)
{
    card_exit_apply(disc_end_minus_one_frame(cpu.a));
}

/* The exit of a TSB or TRB by itself: N and V from the byte it held, Z from the byte it left. */
static void ex_held(CardExit *r, uint8_t held, uint8_t left)
{
    r->n = (held & 0x80) != 0;
    r->v = (held & 0x40) != 0;
    r->z = left == 0;
    r->set_n = r->set_v = r->set_z = true;
    r->set_nz = false;
}

/* A three-byte argument copied to the command, as the BIOS calls it: X offset, Y command offset; gives A. */
static void copy_args(CardExit *r, uint8_t x, uint8_t y, uint16_t ret)
{
    r->x = x;
    r->y = y;
    r->set_x = r->set_y = true;
    jsr_push(ret);
    ex_a(r, copy_args_to_command(x, y));
    jsr_pop();
}

/*
 * @card 00:EE10 cd_play
 * WHAT IT DOES: Plays CD audio: seeks to the start unless told not to, then starts the play;
 * bit 0 of _BH stops after the seek.
 * HOW: Commands $D8 and $D9. The play's mode goes in _DH; mode 2 of the drive's previous play
 * hands the end to an interrupt.
 * WAITS: only through the exchanges.
 * Entry: _AX, _BL start, _BH its mode, _CX, _DL end, _DH its mode; A, X (pushed). Exit: A 0, C 1 and the
 * rest as the path left them; or as cd_retry_after_error if it gave up.
 */
CardExit cd_play(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;
    uint8_t mode;
    uint8_t kind;
    uint8_t value;
    bool again;

    r.set_y = false;
    jsr_push(0xEE13);
    save_card_args(a, x);
    jsr_pop();
    do {
        again = false;
        jsr_push(0xEE16);
        clear_cd_command(r.x);
        jsr_pop();
        mode = RD8(CZP->b.hi);
        kind = (uint8_t)(mode & SEARCH_KIND_BITS);
        compare_a(&r, kind, PLAY_KIND_DISC_END);
        if (kind != PLAY_KIND_DISC_END) {
            r.a = kind;
            r.set_a = true;
            compare_a(&r, kind, SEARCH_KIND_TRACK);
            if (kind == SEARCH_KIND_TRACK) {
                WR8(CZP->a.hi, 0);
                WR8(CZP->b.lo, 0);
            }
            WR8_IDX(COL(CRAM->cd_command), 9, kind);
            WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_SEARCH);
            mode = RD8(CZP->b.hi);
            if (mode == 0) {
                copy_args(&r, 0, 3, 0xEE38);
            } else {
                copy_args(&r, 0, 2, 0xEE40);
            }
            jsr_push(0xEE43);
            step = cd_send_command();
            jsr_pop();
            r = card_exit_over(r, step);
            jsr_push(0xEE46);
            step = cd_finish_command();
            jsr_pop();
            r = card_exit_over(r, step);
            compare_a(&r, r.a, 0);
            if (r.a != 0) {
                jsr_push(0xEE4D);
                step = cd_retry_after_error(r.a, r.x);
                jsr_pop();
                r = card_exit_over(r, step);
                if (r.c) {
                    return r;
                }
                jsr_push(0xEE53);
                restore_card_args(r.a, r.x);
                jsr_pop();
                again = true;
            }
        }
    } while (again);
    for (;;) {
        mode = RD8(CZP->b.hi);
        value = (uint8_t)(mode & 1);
        r.a = value;
        r.nz = value;
        r.set_a = r.set_nz = true;
        if (value != 0) {
            r.a = 0;
            return r;
        }
        jsr_push(0xEE60);
        clear_cd_command(r.x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_PLAY);
        value = RD8(CZP->d.hi);
        WR8_IDX(COL(CRAM->cd_command), 1, (uint8_t)(value & PLAY_MODE_BITS));
        value = RD8(CZP->d.hi);
        kind = (uint8_t)(value & SEARCH_KIND_BITS);
        WR8_IDX(COL(CRAM->cd_command), 9, kind);
        ex_a(&r, kind);
        if (kind == 0) {
            copy_args(&r, 4, 3, 0xEE7C);
        } else {
            if (kind == PLAY_KIND_DISC_END) {
                CardExit end;
                uint8_t byte;

                compare_a(&r, kind, PLAY_KIND_DISC_END);
                for (unsigned i = 0; i < 3; i++) {
                    byte = RD8_IDX(COL(CRAM->disc_length), i);
                    WR8_IDX(COL(CRAM->disc_end), i, byte);
                }
                jsr_push(0xEEA7);
                end = disc_end_minus_one_frame(byte);
                jsr_pop();
                r = card_exit_over(r, end);
                for (unsigned i = 0; i < 3; i++) {
                    byte = RD8_IDX(COL(CRAM->disc_end), i);
                    WR8_IDX(COL(CRAM->cd_command), 2 + i, byte);
                }
                ex_a(&r, PLAY_KIND_TIME);
                WR8_IDX(COL(CRAM->cd_command), 9, PLAY_KIND_TIME);
            } else {
                compare_a(&r, kind, PLAY_KIND_DISC_END);
                compare_a(&r, kind, SEARCH_KIND_TRACK);
                if (kind == SEARCH_KIND_TRACK) {
                    WR8(CZP->c.hi, 0);
                    WR8(CZP->d.lo, 0);
                }
                copy_args(&r, 4, 2, 0xEE91);
            }
        }
        jsr_push(0xEEC1);
        step = cd_send_command();
        jsr_pop();
        r = card_exit_over(r, step);
        compare_a(&r, r.a, PHASE_STATUS);
        if (r.a != PHASE_STATUS) {
            value = RD8_IDX(COL(CRAM->cd_command), 1);
            ex_a(&r, value);
            compare_a(&r, value, PLAY_MODE_KEEP_LAST);
            if (value == PLAY_MODE_KEEP_LAST) {
                value = RD8(CRAM->cd_play_mode);
                ex_a(&r, value);
            }
            WR8(CRAM->cd_play_mode, value);
            compare_a(&r, value, PLAY_MODE_DONE);
            if (value == PLAY_MODE_DONE) {
                r.a = 0;
                return r;
            }
            compare_a(&r, value, PLAY_MODE_ON_INTERRUPT);
            if (value == PLAY_MODE_ON_INTERRUPT) {
                uint8_t held;

                WR8(CRAM->cd_busy, 0);
                held = TSB8(CCDU->irq_control, SEARCH_DONE_IRQ);
                ex_a(&r, SEARCH_DONE_IRQ);
                ex_held(&r, held, (uint8_t)(held | SEARCH_DONE_IRQ));
                r.a = 0;
                return r;
            }
        }
        value = RD8_IDX(COL(CRAM->cd_command), 1);
        ex_a(&r, value);
        compare_a(&r, value, PLAY_MODE_KEEP_LAST);
        if (value == PLAY_MODE_KEEP_LAST) {
            WR8(CRAM->cd_play_mode, value);
        }
        jsr_push(0xEEF2);
        step = cd_finish_command();
        jsr_pop();
        r = card_exit_over(r, step);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        jsr_push(0xEEF9);
        step = cd_retry_after_error(r.a, r.x);
        jsr_pop();
        r = card_exit_over(r, step);
        if (r.c) {
            return r;
        }
        jsr_push(0xEEFE);
        restore_card_args(r.a, r.x);
        jsr_pop();
    }
}

void abi_cd_play(void)
{
    card_exit_apply(cd_play(cpu.a, cpu.x));
}

/* The calls every command makes, each between its frame, merging the registers it leaves. */
static void do_send(CardExit *r, uint16_t ret)
{
    jsr_push(ret);
    *r = card_exit_over(*r, cd_send_command());
    jsr_pop();
}

static void do_finish(CardExit *r, uint16_t ret)
{
    jsr_push(ret);
    *r = card_exit_over(*r, cd_finish_command());
    jsr_pop();
}

static void do_retry(CardExit *r, uint16_t ret)
{
    jsr_push(ret);
    *r = card_exit_over(*r, cd_retry_after_error(r->a, r->x));
    jsr_pop();
}

static void do_restore(CardExit *r, uint16_t ret)
{
    jsr_push(ret);
    restore_card_args(r->a, r->x);
    jsr_pop();
}

static void do_clear(CardExit *r, uint16_t ret)
{
    jsr_push(ret);
    clear_cd_command(r->x);
    jsr_pop();
}

/*
 * @card 00:EA79 cd_read_to_memory
 * WHAT IT DOES: Takes whole blocks from the drive to memory, through the auto port, up to the
 * count in _AX.
 * HOW: Bytes to where _BX points, in blocks of eight pages, until the count runs out or the
 * drive moves to its status phase.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: _AX count, _BX pointer. Exit, status phase: A $D8, N 0, Z 1, C 1. Exit, count out: A
 * the phase, N and Z from it, C 1. X and Y as the loop left them, if a byte moved. V kept.
 * Clobbers _AX, _BH.
 */
CardExit cd_read_to_memory(void)
{
    CardExit e = { 0 };
    uint8_t phase;
    uint8_t low;
    uint8_t high;
    uint8_t value;
    bool finished = false;

    while (!finished) {
        phase = read_bus_phase();
        if (phase == PHASE_STATUS) {
            e.a = phase;
            e.nz = 0;
            e.c = true;
            e.set_a = e.set_nz = e.set_c = true;
            return e;
        }
        if (phase != PHASE_DATA_IN) {
            continue;
        }
        low = RD8(CZP->a.lo);
        high = RD8(CZP->a.hi);
        if ((uint8_t)(low | high) == 0) {
            break;
        }
        {
            uint8_t y = 0;
            uint8_t x = BLOCK_PAGES;

            for (;;) {
                value = RD8(CCDU->data_auto);
                WR8_PTR_Y(CZP->b.lo, y, value);
                low = RD8(CZP->a.lo);
                if (low == 0) {
                    (void)DEC8(CZP->a.hi);
                }
                low = (uint8_t)(low - 1);
                WR8(CZP->a.lo, low);
                high = RD8(CZP->a.hi);
                if ((uint8_t)(low | high) == 0) {
                    e.x = x;
                    e.y = y;
                    e.set_x = e.set_y = true;
                    finished = true;
                    break;
                }
                y++;
                if (y == 0) {
                    (void)INC8(CZP->b.hi);
                    x--;
                    if (x == 0) {
                        e.x = 0;
                        e.y = 0;
                        e.set_x = e.set_y = true;
                        break;
                    }
                }
            }
        }
    }
    phase = read_bus_phase();
    e.a = e.nz = phase;
    e.c = true;
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_cd_read_to_memory(void)
{
    card_exit_apply(cd_read_to_memory());
}

/* The end of a read to VRAM: the phase again; a status answer is thrown away to the end, else its compare. */
static CardExit vram_read_end(CardExit e)
{
    uint8_t value = RD8(CCDU->bus_status);
    uint8_t phase = (uint8_t)(value & PHASE_BITS);

    WR8(CRAM->bus_phase, phase);
    phase = (uint8_t)(phase & PHASE_ANSWER_BITS);
    if (phase == PHASE_STATUS_DONE) {
        return card_exit_over(e, cd_drop_extra_data());
    }
    e.a = phase;
    e.set_a = true;
    compare_a(&e, phase, PHASE_STATUS_DONE);
    return e;
}

/*
 * @card 00:EABE cd_read_to_vram_fast
 * WHAT IT DOES: Takes whole blocks from the drive to VRAM, through the auto port, up to the
 * count in _AX.
 * HOW: A word to the VDC's data port for each two bytes, in blocks of four pages of words.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: _AX count, VDC on its data port. Exit, status phase first: as the end of a read. Exit,
 * count out: as cd_read_data_in's end. X, Y as the loop left them. Clobbers _AX.
 * Not run by any script: it is the read to VRAM of a sector count.
 */
CardExit cd_read_to_vram_fast(void)
{
    CardExit e = { 0 };
    uint8_t phase;
    uint8_t value;
    uint8_t low;
    uint8_t high;
    bool finished = false;

    while (!finished) {
        uint8_t y = 0;
        uint8_t x = VRAM_BLOCK_PAGES;

        phase = read_bus_phase();
        if (phase == PHASE_STATUS) {
            break;
        }
        if (phase != PHASE_DATA_IN) {
            continue;
        }
        for (;;) {
            Sum step;

            value = RD8(CCDU->data_auto);
            WR8(CVDC->data_lo, value);
            value = RD8(CCDU->data_auto);
            WR8(CVDC->data_hi, value);
            low = RD8(CZP->a.lo);
            step = sub8(low, 2, true);
            WR8(CZP->a.lo, step.value);
            high = RD8(CZP->a.hi);
            step = sub8(high, 0, step.carry);
            WR8(CZP->a.hi, step.value);
            low = RD8(CZP->a.lo);
            if ((uint8_t)(step.value | low) == 0) {
                e.x = x;
                e.y = y;
                e.set_x = e.set_y = true;
                finished = true;
                break;
            }
            y--;
            if (y == 0) {
                x--;
                if (x == 0) {
                    e.x = 0;
                    e.y = 0;
                    e.set_x = e.set_y = true;
                    break;
                }
            }
        }
    }
    return vram_read_end(e);
}

void abi_cd_read_to_vram_fast(void)
{
    card_exit_apply(cd_read_to_vram_fast());
}

/*
 * @card 00:EB16 cd_read_to_vram_slow
 * WHAT IT DOES: Takes bytes from the drive to VRAM one handshake at a time, up to the count in
 * _AX.
 * HOW: Each byte to the VDC's data port, low then high, through a pointer in _BX.
 * WAITS: on the drive's bus, with the stack as the caller's.
 * Entry: _AX count. Exit, status phase first: A $D8, N 0, Z 1, C 1. Exit, count out: as
 * cd_read_data_in's end. Clobbers _AX, _BX.
 * Not run by any script: it is the read to VRAM of a byte count.
 */
CardExit cd_read_to_vram_slow(void)
{
    CardExit e = { 0 };
    uint8_t phase;
    uint8_t value;
    uint8_t low;
    uint8_t high;
    uint8_t held;
    Sum step;

    WR8(CZP->b.lo, 2);
    WR8(CZP->b.hi, 0);
    for (;;) {
        phase = read_bus_phase();
        if (phase == PHASE_STATUS) {
            e.a = phase;
            e.nz = 0;
            e.c = true;
            e.set_a = e.set_nz = e.set_c = true;
            return e;
        }
        if (phase != PHASE_DATA_IN) {
            continue;
        }
        value = RD8(CCDU->data);
        WR8_PTR(CZP->b.lo, value);
        jsr_push(0xEB38);
        held = cd_ack_pulse();
        jsr_pop();
        e.v = (held & 0x40) != 0;
        e.set_v = true;
        value = RD8(CZP->b.lo);
        WR8(CZP->b.lo, (uint8_t)(value ^ 1));
        low = RD8(CZP->a.lo);
        step = sub8(low, 1, true);
        WR8(CZP->a.lo, step.value);
        high = RD8(CZP->a.hi);
        step = sub8(high, 0, step.carry);
        WR8(CZP->a.hi, step.value);
        low = RD8(CZP->a.lo);
        if ((uint8_t)(step.value | low) == 0) {
            break;
        }
    }
    return vram_read_end(e);
}

void abi_cd_read_to_vram_slow(void)
{
    card_exit_apply(cd_read_to_vram_slow());
}

/*
 * @card 00:ED9F cd_bytes_to_sectors
 * WHAT IT DOES: Works out how many sectors hold the count in _AX.
 * HOW: The high byte over 8, and one more if anything is left over: CardCdWhole says none was.
 * WAITS: no.
 * Entry: _AX bytes. Exit: A CardCdWhole; N and Z from it, or from the sectors counted if one
 * was added; C bit 2 of _AH. X, Y, V kept.
 */
CardExit cd_bytes_to_sectors(void)
{
    CardExit e = { 0 };
    uint8_t high;
    uint8_t whole;
    uint8_t low;

    WR8(CRAM->cd_whole, 1);
    high = RD8(CZP->a.hi);
    if (high & 1) {
        WR8(CRAM->cd_whole, 0);
    }
    high = (uint8_t)(high >> 1);
    if (high & 1) {
        WR8(CRAM->cd_whole, 0);
    }
    high = (uint8_t)(high >> 1);
    e.c = (high & 1) != 0;
    if (e.c) {
        WR8(CRAM->cd_whole, 0);
    }
    high = (uint8_t)(high >> 1);
    WR8(CRAM->cd_sectors, high);
    low = RD8(CZP->a.lo);
    if (low != 0) {
        WR8(CRAM->cd_whole, 0);
    }
    whole = RD8(CRAM->cd_whole);
    e.a = e.nz = whole;
    if (whole == 0) {
        e.nz = INC8(CRAM->cd_sectors);
    }
    e.set_a = e.set_nz = e.set_c = true;
    return e;
}

void abi_cd_bytes_to_sectors(void)
{
    card_exit_apply(cd_bytes_to_sectors());
}

/* The choice of the other base once the drive has said a sense code of $10 or more. */
static void choose_other_base(CardExit *r)
{
    uint8_t sense = RD8(CRAM->cd_sense);

    ex_a(r, sense);
    compare_a(r, sense, SENSE_TRY_OTHER_BASE);
    if (sense >= SENSE_TRY_OTHER_BASE) {
        WR8(CRAM->cd_base_index, 1);
        ex_a(r, 1);
    }
}

/* A call of the MPR stub the card builds in RAM: its two bytes of code, its frame, and what it does. */
static uint8_t stub_tma(uint8_t mask, uint16_t ret)
{
    uint8_t bank;

    jsr_push(ret);
    bank = tma(mask);
    jsr_pop();
    return bank;
}

static void stub_tam(uint8_t mask, uint8_t bank, uint16_t ret)
{
    jsr_push(ret);
    tam(mask, bank);
    jsr_pop();
}

/*
 * @card 00:EC05 cd_read
 * WHAT IT DOES: Reads sectors from the disc to memory or VRAM.
 * HOW: Command 8, with the address plus the chosen base. _DH says where they go: 0 _AX bytes
 * to _BX, 1 _AL sectors to _BX, 2-6 _AL sectors through the MPR it names, $FE _AX bytes to VRAM
 * with a handshake each, $FF _AL sectors to VRAM. After a sense code of $10 or more it uses the
 * other base.
 * WAITS: only through the exchanges.
 * Entry: _CL, _CH, _DL address; the rest by _DH; A, X (pushed). Exit: A 0 if well, $22 for a bad request;
 * else as cd_retry_after_error. C 1 if well. X, Y and V as the last step left them.
 * Not run by any script: a read through an MPR, a read to VRAM, a bad request.
 */
CardExit cd_read(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;
    uint8_t mode;
    uint8_t value;
    uint8_t sectors;

    r.set_y = false;
    WR8(CRAM->cd_base_index, 0);
    jsr_push(0xEC0B);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        bool retry = false;

        do_clear(&r, 0xEC0E);
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_READ);
        jsr_push(0xEC16);
        step = add_cd_base();
        jsr_pop();
        r = card_exit_over(r, step);
        copy_args(&r, 4, 1, 0xEC1D);
        mode = RD8(CZP->d.hi);
        ex_a(&r, mode);
        compare_a(&r, mode, 2);
        if (mode < 2) {
            mode = RD8(CZP->d.hi);
            ex_a(&r, mode);
            if (mode == 0) {
                jsr_push(0xEC46);
                step = cd_bytes_to_sectors();
                jsr_pop();
                r = card_exit_over(r, step);
            } else {
                value = RD8(CZP->a.lo);
                WR8(CRAM->cd_sectors, value);
                WR8(CZP->a.hi, (uint8_t)(value << 3));
                WR8(CZP->a.lo, 0);
            }
            sectors = RD8(CRAM->cd_sectors);
            ex_a(&r, sectors);
            if (sectors == 0) {
                ex_a(&r, READ_BAD_REQUEST);
                return r;
            }
            WR8_IDX(COL(CRAM->cd_command), 4, sectors);
            do_send(&r, 0xEC51);
            compare_a(&r, r.a, PHASE_DATA_IN);
            if (r.a == PHASE_DATA_IN) {
                jsr_push(0xEC58);
                step = cd_read_to_memory();
                jsr_pop();
                r = card_exit_over(r, step);
                compare_a(&r, r.a, PHASE_DATA_IN);
                if (r.a == PHASE_DATA_IN) {
                    jsr_push(0xEC5F);
                    step = cd_drop_extra_data();
                    jsr_pop();
                    r = card_exit_over(r, step);
                }
            }
            do_finish(&r, 0xEC62);
            compare_a(&r, r.a, 0);
            if (r.a == 0) {
                uint8_t low = RD8(CZP->a.lo);
                uint8_t high = RD8(CZP->a.hi);

                ex_a(&r, (uint8_t)(low | high));
                if ((uint8_t)(low | high) == 0) {
                    return r;
                }
            } else {
                do_retry(&r, 0xEC69);
                if (r.c) {
                    return r;
                }
            }
            retry = true;
        } else if (mode == READ_TO_VRAM_FAST) {
            value = RD8(CZP->a.lo);
            ex_a(&r, value);
            if (value == 0) {
                ex_a(&r, READ_BAD_REQUEST);
                return r;
            }
            WR8_IDX(COL(CRAM->cd_command), 4, value);
            do_send(&r, 0xEC83);
            compare_a(&r, r.a, PHASE_DATA_IN);
            if (r.a == PHASE_DATA_IN) {
                value = RD8(CZP->a.lo);
                WR8(CZP->a.hi, (uint8_t)(value << 3));
                WR8(CZP->a.lo, 0);
                WR8(CZP->vdc_reg, 0);
                WR8(CVDC->addr, 0);
                jsr_push(0xEC9A);
                step = (CardExit){ 0 };
                step.a = write_vram_word_pending();
                jsr_pop();
                WR8(CZP->vdc_reg, 2);
                WR8(CVDC->addr, 2);
                jsr_push(0xECA4);
                step = cd_read_to_vram_fast();
                jsr_pop();
                r = card_exit_over(r, step);
                WR8(CRAM->vram_pending, 0);
            }
            do_finish(&r, 0xECAA);
            compare_a(&r, r.a, 0);
            if (r.a == 0) {
                uint8_t low = RD8(CZP->a.lo);
                uint8_t high = RD8(CZP->a.hi);

                ex_a(&r, (uint8_t)(low | high));
                if ((uint8_t)(low | high) == 0) {
                    return r;
                }
            } else {
                do_retry(&r, 0xECB1);
                if (r.c) {
                    return r;
                }
            }
            retry = true;
        } else if (mode == READ_TO_VRAM_SLOW) {
            jsr_push(0xECBE);
            step = cd_bytes_to_sectors();
            jsr_pop();
            r = card_exit_over(r, step);
            sectors = RD8(CRAM->cd_sectors);
            ex_a(&r, sectors);
            if (sectors == 0) {
                ex_a(&r, READ_BAD_REQUEST);
                return r;
            }
            WR8_IDX(COL(CRAM->cd_command), 4, sectors);
            do_send(&r, 0xECC9);
            compare_a(&r, r.a, PHASE_DATA_IN);
            if (r.a == PHASE_DATA_IN) {
                WR8(CZP->vdc_reg, 0);
                WR8(CVDC->addr, 0);
                jsr_push(0xECD7);
                (void)write_vram_word_pending();
                jsr_pop();
                WR8(CZP->vdc_reg, 2);
                WR8(CVDC->addr, 2);
                jsr_push(0xECE1);
                step = cd_read_to_vram_slow();
                jsr_pop();
                r = card_exit_over(r, step);
                WR8(CRAM->vram_pending, 0);
            }
            do_finish(&r, 0xECE7);
            compare_a(&r, r.a, 0);
            if (r.a == 0) {
                uint8_t low = RD8(CZP->a.lo);
                uint8_t high = RD8(CZP->a.hi);

                ex_a(&r, (uint8_t)(low | high));
                if ((uint8_t)(low | high) == 0) {
                    return r;
                }
            } else {
                do_retry(&r, 0xECEE);
                if (r.c) {
                    return r;
                }
            }
            retry = true;
        } else {
            uint8_t mask = 1;
            uint8_t count;
            uint8_t sector_low;

            compare_a(&r, mode, READ_TO_VRAM_FAST);
            compare_a(&r, mode, READ_TO_VRAM_SLOW);
            value = RD8(CZP->a.lo);
            r.x = value;
            r.nz = value;
            r.set_x = r.set_nz = true;
            if (value != 0) {
                compare_a(&r, mode, MPR_LIMIT);
            }
            if (value == 0 || !((uint8_t)(mode - MPR_LIMIT) & 0x80)) {
                ex_a(&r, READ_BAD_REQUEST);
                return r;
            }
            for (count = mode; count != 0; count--) {
                mask = (uint8_t)(mask << 1);
            }
            r.x = 0;
            WR8(CRAM->mpr_stub_mask, mask);
            WR8(CRAM->mpr_stub, MPR_STUB_TMA);
            WR8(CRAM->mpr_stub_return, MPR_STUB_RTS);
            ex_a(&r, stub_tma(mask, 0xED25));
            WR8(CRAM->saved_mpr_bank, r.a);
            value = RD8(CZP->b.lo);
            WR8(CZP->d.lo, value);
            value = RD8(CZP->a.lo);
            WR8_IDX(COL(CRAM->cd_command), 4, value);
            sectors = (uint8_t)(value >> 2);
            WR8(CRAM->cd_sectors, sectors);
            value = RD8(CZP->a.lo);
            WR8(CRAM->cd_whole, (uint8_t)(value & 3));
            do_send(&r, 0xED40);
            r.a = (uint8_t)(r.a | 0x40);
            compare_a(&r, r.a, PHASE_DATA_IN);
            sector_low = r.a;
            while (sector_low == PHASE_DATA_IN) {
                sectors = RD8(CRAM->cd_sectors);
                if (sectors != 0) {
                    ex_a(&r, 0x20);
                    WR8(CZP->a.hi, 0x20);
                } else {
                    value = RD8(CRAM->cd_whole);
                    ex_a(&r, value);
                    if (value == 0) {
                        break;
                    }
                    WR8(CZP->a.hi, (uint8_t)(value << 3));
                }
                WR8(CZP->a.lo, 0);
                value = RD8(CZP->d.hi);
                WR8(CZP->b.hi, (uint8_t)(value << 5));
                WR8(CZP->b.lo, 0);
                WR8(CRAM->mpr_stub, MPR_STUB_TAM);
                value = RD8(CZP->d.lo);
                ex_a(&r, value);
                stub_tam(mask, value, 0xED72);
                jsr_push(0xED75);
                step = cd_read_to_memory();
                jsr_pop();
                r = card_exit_over(r, step);
                r.a = (uint8_t)(r.a | 0x40);
                compare_a(&r, r.a, PHASE_DATA_IN);
                if (r.a != PHASE_DATA_IN) {
                    break;
                }
                (void)INC8(CZP->d.lo);
                sectors = DEC8(CRAM->cd_sectors);
                if (sectors & 0x80) {
                    break;
                }
                sector_low = r.a;
            }
            do_finish(&r, 0xED85);
            compare_a(&r, r.a, 0);
            if (r.a != 0) {
                do_retry(&r, 0xED8C);
                if (!r.c) {
                    retry = true;
                }
            }
            if (!retry) {
                value = r.a;
                push8(value);
                WR8(CRAM->mpr_stub, MPR_STUB_TAM);
                ex_a(&r, RD8(CRAM->saved_mpr_bank));
                stub_tam(mask, r.a, 0xED9D);
                r.a = pull8();
                r.nz = r.a;
                return r;
            }
        }
        if (retry) {
            do_restore(&r, 0xECF4);
            choose_other_base(&r);
        }
    }
}

void abi_cd_read(void)
{
    card_exit_apply(cd_read(cpu.a, cpu.x));
}

/*
 * @card 00:EDCB cd_seek
 * WHAT IT DOES: Moves the drive to a disc address: _CL, _CH, _DL (high first) plus the chosen
 * base.
 * HOW: Command 8 for one sector, as cd_read sends it, and whatever data the drive offers is
 * thrown away. Retries as cd_read does: from the first base, and after a sense code of $10 or
 * more from the other.
 * WAITS: only through the exchanges.
 * Entry: _CL, _CH, _DL address; A, X (pushed). Exit, well: A 0, N 0, Z 1, C 1. Exit, gave up: as
 * cd_retry_after_error. Y 1 unless a step changed it; X and V as the last step left them. _CL,
 * _CH, _DL left as the address plus the base.
 * Not run by any script.
 */
CardExit cd_seek(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;

    r.set_y = false;
    WR8(CRAM->cd_base_index, 0);
    jsr_push(0xEDD1);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        do_clear(&r, 0xEDD4);
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_READ);
        jsr_push(0xEDDC);
        step = add_cd_base();
        jsr_pop();
        r = card_exit_over(r, step);
        copy_args(&r, 4, 1, 0xEDE3);
        WR8_IDX(COL(CRAM->cd_command), 4, 1);
        ex_a(&r, 1);
        do_send(&r, 0xEDEB);
        compare_a(&r, r.a, PHASE_DATA_IN);
        if (r.a == PHASE_DATA_IN) {
            jsr_push(0xEDF2);
            step = cd_drop_extra_data();
            jsr_pop();
            r = card_exit_over(r, step);
        }
        do_finish(&r, 0xEDF5);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        do_retry(&r, 0xEDFC);
        if (r.c) {
            return r;
        }
        do_restore(&r, 0xEE02);
        choose_other_base(&r);
    }
}

void abi_cd_seek(void)
{
    card_exit_apply(cd_seek(cpu.a, cpu.x));
}

/*
 * @card 00:F0A9 cd_contnts
 * WHAT IT DOES: Reads the disc's first and last tracks and its length, and keeps them.
 * HOW: cd_dinfo twice into CardCdResult, with the answers copied to CardDiscFirstTrack and
 * CardDiscLength.
 * WAITS: only through the exchanges.
 * Entry: _AH the high byte cd_dinfo asks with; A, X (pushed by it). Exit, well: A 0, N 0, Z 1, C 1.
 * Exit, failed: A as cd_dinfo left it, N and Z from it, C 1. X, Y, V as cd_dinfo left them.
 * Not run by any script.
 */
CardExit cd_contnts(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    uint8_t value;

    r.set_a = r.set_y = false;
    WR8(CZP->a.lo, 0);
    WR8(CZP->b.lo, 0x56);
    WR8(CZP->b.hi, 0x22);
    jsr_push(0xF0B6);
    r = card_exit_over(r, cd_dinfo(0x22, r.x));
    jsr_pop();
    compare_a(&r, r.a, 0);
    if (r.a != 0) {
        return r;
    }
    value = RD8_IDX(COL(CRAM->cd_result), 0);
    WR8_IDX(COL(CRAM->disc_first_track), 0, value);
    value = RD8_IDX(COL(CRAM->cd_result), 1);
    WR8_IDX(COL(CRAM->disc_first_track), 1, value);
    WR8(CZP->a.lo, 1);
    WR8(CZP->b.lo, 0x56);
    WR8(CZP->b.hi, 0x22);
    jsr_push(0xF0D5);
    r = card_exit_over(r, cd_dinfo(0x22, r.x));
    jsr_pop();
    compare_a(&r, r.a, 0);
    if (r.a != 0) {
        return r;
    }
    push8(r.a);
    for (unsigned i = 0; i < 3; i++) {
        value = RD8_IDX(COL(CRAM->cd_result), i);
        WR8_IDX(COL(CRAM->disc_length), i, value);
    }
    r.a = pull8();
    return r;
}

void abi_cd_contnts(void)
{
    card_exit_apply(cd_contnts(cpu.a, cpu.x));
}

/*
 * @card 00:F393 ad_trans
 * WHAT IT DOES: Reads sectors from the disc into ADPCM memory.
 * HOW: Command 8 with the unit's own transfer running until the drive is done. Refuses while
 * a transfer is already running, and retries like cd_read.
 * WAITS: on the unit's transfer and the drive's end of it, with interrupts in them, the stack as the caller's.
 * Entry: _AL sectors, _BX ADPCM address, _CL, _CH, _DL disc address, _DH 0 to set the address; A, X
 * (pushed). Exit, refused: A 4, N and Z from it, V bit 6 of the transfer port. Exit, well: A 0, N 0, Z 1,
 * C 1. Exit, gave up: as cd_retry_after_error. Y 1 unless a step changed it.
 */
CardExit ad_trans(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    CardExit step;
    uint8_t status;
    uint8_t value;
    uint8_t held;

    r.set_y = false;
    held = RD8(CCDU->adpcm_dma);
    if (held & AD_DMA_RUNNING) {
        ex_a(&r, AD_ERROR_BUSY);
        ex_v(&r, (held & 0x40) != 0);
        return r;
    }
    WR8(CRAM->cd_base_index, 0);
    jsr_push(0xF3A3);
    save_card_args(a, x);
    jsr_pop();
    for (;;) {
        value = RD8(CZP->d.hi);
        ex_a(&r, value);
        if (value == 0) {
            uint8_t low = RD8(CZP->b.lo);
            uint8_t high = RD8(CZP->b.hi);

            r.x = low;
            r.y = high;
            r.set_x = r.set_y = true;
            jsr_push(0xF3AE);
            ad_set_address(low, high);
            jsr_pop();
            jsr_push(0xF3B1);
            held = ad_latch_write_address();
            jsr_pop();
            r.a = BUS_ACK;
            r.set_a = true;
            (void)held;
        }
        do_clear(&r, 0xF3B4);
        WR8_IDX(COL(CRAM->cd_command), 0, COMMAND_READ);
        jsr_push(0xF3BC);
        step = add_cd_base();
        jsr_pop();
        r = card_exit_over(r, step);
        copy_args(&r, 4, 1, 0xF3C3);
        value = RD8(CZP->a.lo);
        WR8_IDX(COL(CRAM->cd_command), 4, value);
        do_send(&r, 0xF3CB);
        WR8(CCDU->adpcm_dma, AD_DMA_FROM_CD);
        for (;;) {
            status = RD8(CCDU->adpcm_status);
            if (status & AD_STATUS_DMA_BUSY) {
                continue;
            }
            status = RD8(CCDU->irq_status);
            if (status & CD_STATUS_TRANSFER_DONE) {
                break;
            }
        }
        WR8(CCDU->adpcm_dma, 0);
        do_finish(&r, 0xF3E3);
        compare_a(&r, r.a, 0);
        if (r.a == 0) {
            return r;
        }
        do_retry(&r, 0xF3EA);
        if (r.c) {
            return r;
        }
        do_restore(&r, 0xF3EF);
        choose_other_base(&r);
    }
}

void abi_ad_trans(void)
{
    card_exit_apply(ad_trans(cpu.a, cpu.x));
}

/*
 * @card 00:EB8F cd_base
 * WHAT IT DOES: Sets the disc address that reads count from: _AL, _AH, _BL as a sector number
 * if _BH is zero, as BCD minutes, seconds and frames if bit 6 of _BH is set, or else the start
 * of the track numbered _AL. Bit 1 of _CL keeps the first base as it was, bit 0 the second.
 * HOW: A track's start is asked of the drive's table of contents into CardCdResult; a position
 * is copied there; either is made into a sector number by msf_to_lba in bank 1, mapped at
 * $C000 for the call. Then the sector number goes to the bases asked for.
 * WAITS: only through cd_dinfo.
 * Entry: _AX, _BX, _CL; A, X (pushed in cd_dinfo). Exit: A _BL, N and Z from it, if a base was set. Else
 * A 0, Z 1, N 0 for a sector number, or A the bank MPR6 held, N and Z from it. C and V from msf_to_lba, kept for
 * a sector number. X and Y as cd_dinfo leaves them for a track; kept otherwise.
 * Not run by any script.
 */
CardExit cd_base(uint8_t a, uint8_t x)
{
    CardExit r = entered(a, x, 0);
    uint8_t mode;
    uint8_t value;
    uint8_t held;
    uint8_t flags;

    r.set_a = r.set_x = r.set_y = false;
    mode = RD8(CZP->b.hi);
    ex_a(&r, mode);
    if (mode != 0) {
        mode = RD8(CZP->b.hi);
        if (mode & 0x40) {
            value = RD8(CZP->a.lo);
            WR8_IDX(COL(CRAM->cd_result), 0, value);
            value = RD8(CZP->a.hi);
            WR8_IDX(COL(CRAM->cd_result), 1, value);
            value = RD8(CZP->b.lo);
            WR8_IDX(COL(CRAM->cd_result), 2, value);
        } else {
            value = RD8(CZP->a.lo);
            WR8(CZP->a.hi, value);
            WR8(CZP->a.lo, 2);
            WR8(CZP->b.lo, (uint8_t)ADDR_OF(CRAM->cd_result[0]));
            WR8(CZP->b.hi, (uint8_t)(ADDR_OF(CRAM->cd_result[0]) >> 8));
            jsr_push(0xEBA9);
            r = card_exit_over(r, cd_dinfo((uint8_t)(ADDR_OF(CRAM->cd_result[0]) >> 8), r.x));
            jsr_pop();
        }
        held = tma(MPR6);
        push8(held);
        tam(MPR6, 1);
        jsr_push(0xEBC4);
        r = card_exit_over(r, msf_to_lba(r.x));
        jsr_pop();
        held = pull8();
        tam(MPR6, held);
        ex_a(&r, held);
    }
    flags = RD8(CZP->c.lo);
    if (!(flags & 0x02)) {
        value = RD8(CZP->a.lo);
        WR8_IDX(COL(CRAM->cd_bases), 0, value);
        value = RD8(CZP->a.hi);
        WR8_IDX(COL(CRAM->cd_bases), 1, value);
        value = RD8(CZP->b.lo);
        WR8_IDX(COL(CRAM->cd_bases), 2, value);
        ex_a(&r, value);
    }
    flags = RD8(CZP->c.lo);
    if (!(flags & 0x01)) {
        value = RD8(CZP->a.lo);
        WR8_IDX(COL(CRAM->cd_bases), 3, value);
        value = RD8(CZP->a.hi);
        WR8_IDX(COL(CRAM->cd_bases), 4, value);
        value = RD8(CZP->b.lo);
        WR8_IDX(COL(CRAM->cd_bases), 5, value);
        ex_a(&r, value);
    }
    return r;
}

void abi_cd_base(void)
{
    card_exit_apply(cd_base(cpu.a, cpu.x));
}

/* The A each of the game's CD calls leaves, and its exit: the call's CardExit. */
uint8_t cd_play_result(CdPlay play) { return play.a; }
uint8_t cd_read_result(CdRead read) { return read.a; }
uint8_t cd_search_result(CdSearch search) { return search.a; }
uint8_t cd_stat_result(CdStat stat) { return stat.a; }
uint8_t cd_pause_result(CdPause pause) { return pause.a; }
uint8_t cd_seek_result(CdSeek seek) { return seek.a; }
uint8_t ad_trans_result(AdTrans trans) { return trans.a; }
void cd_play_exit_shim(CdPlay play) { card_exit_apply(play); }
void cd_read_exit_shim(CdRead read) { card_exit_apply(read); }
void cd_search_exit_shim(CdSearch search) { card_exit_apply(search); }
void cd_stat_exit_shim(CdStat stat) { card_exit_apply(stat); }
void cd_pause_exit_shim(CdPause pause) { card_exit_apply(pause); }
void cd_seek_exit_shim(CdSeek seek) { card_exit_apply(seek); }
void ad_trans_exit_shim(AdTrans trans) { card_exit_apply(trans); }
