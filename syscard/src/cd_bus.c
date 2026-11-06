/* The handshake on the CD unit's bus, and its reset line. */
#include "card.h"
#include "syscard.h"

enum {
    BUS_ACK = 0x80, BUS_REQUEST = 0x40, BUS_BUSY = 0x80, CD_RESET_LINE = 0x02,
    PHASE_BITS = 0xF8, PHASE_COMMAND = 0xD0, PHASE_ANSWER_BITS = 0xB8,
    PHASE_DATA_OUT_DONE = 0x98, PHASE_STATUS_DONE = 0x88, PHASE_DATA_DONE = 0x80,
    PHASE_STATUS = 0xD8, PHASE_MESSAGE = 0xF8,
    BUS_SELECT_ID = 0x81, BUS_RELEASE_ID = 0xFF, BUS_ABORT_IRQS = 0x60,
    SELECT_TRIES = 256, COMMAND_NO_WAIT = 0xD9,
    ANSWER_TICKS = 0xFE, CD_ERROR_TIMED_OUT = 6
};

/*
 * @card 00:EA35 cd_ack_on
 * WHAT IT DOES: Raises the bus's acknowledge.
 * HOW: Sets its bit in the control port, and gives what the port held.
 * Entry: nothing. Exit: A $80, Z clear, N and V bits 7 and 6 of what the port held. X, Y, C
 * kept.
 */
uint8_t cd_ack_on(void)
{
    uint8_t control = rd8(CdIrqControl);

    wr8(CdIrqControl, control | BUS_ACK);
    return control;
}

void abi_cd_ack_on(void)
{
    uint8_t control = cd_ack_on();

    cpu.a = BUS_ACK;
    abi_flag(P_Z, false);
    abi_flag(P_N, (control & 0x80) != 0);
    abi_flag(P_V, (control & 0x40) != 0);
}

/*
 * @card 00:EA2F cd_ack_off
 * WHAT IT DOES: Lowers the bus's acknowledge.
 * HOW: Clears its bit in the control port, and gives what the port held.
 * Entry: nothing. Exit: A $80, Z if nothing else was set, N and V bits 7 and 6 of what the
 * port held. X, Y, C kept.
 */
uint8_t cd_ack_off(void)
{
    uint8_t control = rd8(CdIrqControl);

    wr8(CdIrqControl, (uint8_t)(control & ~BUS_ACK));
    return control;
}

static void ack_off_exit_shim(uint8_t control)
{
    cpu.a = BUS_ACK;
    abi_flag(P_Z, (control & ~BUS_ACK) == 0);
    abi_flag(P_N, (control & 0x80) != 0);
    abi_flag(P_V, (control & 0x40) != 0);
}

void abi_cd_ack_off(void)
{
    ack_off_exit_shim(cd_ack_off());
}

/*
 * @card 00:EA27 cd_ack_pulse
 * WHAT IT DOES: Acknowledges a byte on the bus.
 * HOW: Raises the acknowledge, waits for the drive to drop its request, lowers it.
 * Entry: nothing. Exit: as cd_ack_off.
 */
uint8_t cd_ack_pulse(void)
{
    cd_ack_on();
    while (rd8(CdBusStatus) & BUS_REQUEST)
        ;
    return cd_ack_off();
}

void abi_cd_ack_pulse(void)
{
    ack_off_exit_shim(cd_ack_pulse());
}

/*
 * @card 00:E8E3 cd_reset
 * WHAT IT DOES: Resets the CD unit.
 * HOW: Pulses its reset line, with pauses the C does not state.
 * Entry: nothing. Exit: A what the port was left holding, X 0, Y 0, N 0, Z 1, V bit 6 of what
 * the port first held. C kept.
 */
CdReset cd_reset(void)
{
    CdReset reset;

    reset.before = rd8(CdResetPort);
    wr8(CdResetPort, reset.before | CD_RESET_LINE);
    reset.after = (uint8_t)(rd8(CdResetPort) & ~CD_RESET_LINE);
    wr8(CdResetPort, reset.after);
    return reset;
}

static void cd_reset_exit_shim(CdReset reset)
{
    cpu.x = 0;
    cpu.y = 0;
    abi_nz(0);
    abi_flag(P_V, (reset.before & 0x40) != 0);
}

void abi_cd_reset(void)
{
    CdReset reset = cd_reset();

    cpu.a = reset.after;
    cd_reset_exit_shim(reset);
}

/* The bus phase, kept where the card keeps it. */
static uint8_t read_bus_phase(void)
{
    uint8_t phase = rd8(CdBusStatus) & PHASE_BITS;

    wr8(CardBusPhase, phase);
    return phase;
}

/* The drive is busy with something else: drops the select, and takes bytes until it is free. */
static void clear_busy_bus(void)
{
    wr8(CdIrqControl, (uint8_t)(rd8(CdIrqControl) & ~BUS_ABORT_IRQS));
    wr8(CdBusStatus, BUS_ABORT_IRQS);
    wr8(CdData, BUS_RELEASE_ID);
    if (!(rd8(CdBusStatus) & BUS_REQUEST))
        return;
    do {
        cd_ack_pulse();
        while (rd8(CdBusStatus) & BUS_REQUEST)
            cd_ack_pulse();
    } while (rd8(CdBusStatus) & BUS_BUSY);
}

/* Selects the drive: true once it asks for the command, within its tries. */
static bool select_drive(uint8_t *flag_byte)
{
    for (;;) {
        wr8(CdData, BUS_SELECT_ID);
        *flag_byte = rd8(CdBusStatus);
        if (*flag_byte & BUS_BUSY) {
            clear_busy_bus();
            continue;
        }
        wr8(CdBusStatus, BUS_SELECT_ID);
        for (int i = 0; i < SELECT_TRIES; i++)
            if (rd8(CdBusStatus) & BUS_REQUEST)
                return true;
    }
}

/*
 * @card 00:E900 cd_send_command
 * WHAT IT DOES: Sends the command in CardCdCommand to the drive, and unless told not to,
 * waits for it to answer.
 * HOW: Waits for a transfer in progress, selects the drive, hands over bytes while the bus
 * is in its command phase. Then waits up to $FE frames for the request bit.
 * Entry: the command built. Exit, see cd_send_exit_shim: A, N, Z, C and V as the path left
 * them; X the bytes sent, or the first frame count if it waited. Y kept.
 */
CdSent cd_send_command(void)
{
    CdSent sent = { CD_SENT_TIMED_OUT, 0, 0, 0, 0, 0, 0, false };
    uint8_t answer;

    while (rd8(CardCdBusy) != 0)
        ;
    wr8(CardCdTimedOut, 0);
    select_drive(&sent.flag_byte);
    wr8(CardBusPhase, 0);
    for (;;) {
        uint8_t phase = read_bus_phase();

        if (phase == PHASE_COMMAND) {
            wr8(CdData, rd8((uint16_t)(CardCdCommand + sent.sent)));
            sent.sent++;
            sent.flag_byte = cd_ack_pulse();
            continue;
        }
        answer = phase & PHASE_ANSWER_BITS;
        if (answer == PHASE_DATA_OUT_DONE || answer == PHASE_STATUS_DONE || answer == PHASE_DATA_DONE)
            break;
    }
    sent.pending = rd8(CardCdBusy) | rd8(CardCdNoWait);
    if (sent.pending != 0) {
        sent.how = CD_SENT_PENDING;
        return sent;
    }
    if (rd8(CardCdCommand) == COMMAND_NO_WAIT) {
        uint8_t sub = rd8((uint16_t)(CardCdCommand + 1));

        if (sub == 1 || sub == 2) {
            sent.how = CD_SENT_NO_WAIT;
            return sent;
        }
    }
    sent.first_tick = rd8(CardFrameTick);
    for (;;) {
        sent.phase = read_bus_phase();
        if (sent.phase & BUS_REQUEST) {
            sent.how = CD_SENT_ANSWERED;
            return sent;
        }
        sent.last_tick = rd8(CardFrameTick);
        if ((uint8_t)(sent.last_tick - sent.first_tick) >= ANSWER_TICKS) {
            wr8(CardCdTimedOut, 1);
            sent.how = CD_SENT_TIMED_OUT;
            return sent;
        }
        sent.waited_again = true;
    }
}

/* C as the compares before the wait left it: with command $D9 its second byte against 2. */
static bool carry_before_answer_wait(void)
{
    uint8_t command = rd8(CardCdCommand);

    if (command == COMMAND_NO_WAIT)
        return rd8((uint16_t)(CardCdCommand + 1)) >= 2;
    return command >= COMMAND_NO_WAIT;
}

static void cd_send_exit_shim(CdSent sent)
{
    abi_flag(P_V, (sent.flag_byte & 0x40) != 0);
    cpu.x = sent.sent;
    switch (sent.how) {
    case CD_SENT_PENDING:
        cpu.a = sent.pending;
        abi_nz(sent.pending);
        abi_flag(P_C, true);
        break;
    case CD_SENT_NO_WAIT:
        cpu.a = rd8((uint16_t)(CardCdCommand + 1));
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case CD_SENT_ANSWERED:
        /* BIT #$40: N and V from the operand, Z from A and it. */
        cpu.a = sent.phase;
        cpu.x = sent.first_tick;
        abi_flag(P_N, false);
        abi_flag(P_V, true);
        abi_flag(P_Z, false);
        abi_flag(P_C, sent.waited_again ? false : carry_before_answer_wait());
        break;
    case CD_SENT_TIMED_OUT: {
        /* The last SBC was the first count less the last, whose negation reached $FE. */
        unsigned diff = (unsigned)sent.first_tick - sent.last_tick;

        cpu.a = 1;
        cpu.x = sent.first_tick;
        abi_nz(1);
        abi_flag(P_C, true);
        abi_flag(P_V, ((sent.first_tick ^ sent.last_tick) & (sent.first_tick ^ diff) & 0x80) != 0);
        break;
    }
    }
}

void abi_cd_send_command(void)
{
    cd_send_exit_shim(cd_send_command());
}

/*
 * @card 00:E9DC cd_read_status
 * WHAT IT DOES: Takes the drive's status byte and its message at the end of a command.
 * HOW: Waits for the status phase, takes the byte; then the message phase, or status again,
 * takes that; waits for the drive to leave the bus.
 * Entry: nothing. Exit: A the status, N and Z from it, C 1, V bit 6 of the bus's last read. X
 * and Y kept.
 */
CdStatusRead cd_read_status(void)
{
    CdStatusRead read;
    uint8_t message;
    bool at_message = false;

    wr8(CardCdStatus, 0);
    while (!at_message) {
        uint8_t phase;

        while (read_bus_phase() != PHASE_STATUS)
            ;
        wr8(CardCdStatus, rd8(CdData));
        cd_ack_on();
        while (rd8(CdBusStatus) & BUS_REQUEST)
            ;
        cd_ack_off();
        do {
            phase = read_bus_phase();
            at_message = phase == PHASE_MESSAGE;
        } while (!at_message && phase != PHASE_STATUS);
    }
    message = rd8(CdData);
    wr8(CardBusPhase, message);
    cd_ack_on();
    while (rd8(CdBusStatus) & BUS_REQUEST)
        ;
    cd_ack_off();
    do {
        read.last_bus = rd8(CdBusStatus);
    } while (read.last_bus & BUS_BUSY);
    read.status = rd8(CardCdStatus);
    return read;
}

static void cd_status_exit_shim(CdStatusRead read)
{
    cpu.a = read.status;
    abi_nz(read.status);
    abi_flag(P_C, true);
    abi_flag(P_V, (read.last_bus & 0x40) != 0);
}

void abi_cd_read_status(void)
{
    cd_status_exit_shim(cd_read_status());
}

/*
 * @card 00:E9C5 cd_finish_command
 * WHAT IT DOES: Finishes a command: if the drive timed out resets it, else takes its status.
 * HOW: CardCdTimedOut. A zero status clears the sense code.
 * Entry: nothing. Exit, timed out: A 6, N and Z from it, X 0, Y 0, V as cd_reset. Exit: as
 * cd_read_status.
 */
CdFinish cd_finish_command(void)
{
    CdFinish finish = { false, { 0, 0 }, { 0, 0 } };

    if (rd8(CardCdTimedOut) != 0) {
        wr8(CardCdTimedOut, 0);
        finish.timed_out = true;
        finish.reset = cd_reset();
        return finish;
    }
    finish.read = cd_read_status();
    if (finish.read.status == 0)
        wr8(CardCdSense, 0);
    return finish;
}

void abi_cd_finish_command(void)
{
    CdFinish finish = cd_finish_command();

    if (finish.timed_out) {
        cd_reset_exit_shim(finish.reset);
        cpu.a = CD_ERROR_TIMED_OUT;
        abi_nz(CD_ERROR_TIMED_OUT);
    } else {
        cd_status_exit_shim(finish.read);
    }
}
