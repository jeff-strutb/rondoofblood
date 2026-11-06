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
    ANSWER_TICKS = 0xFE, CD_ERROR_TIMED_OUT = 6, CD_ERROR_ASK_SENSE = 2,
    PHASE_DATA_IN = 0xC8, DROPPED_BLOCK_BYTES = 0x800,
    COMMAND_REQUEST_SENSE = 3, COMMAND_TEST_UNIT = 0, SENSE_BYTES = 10,
    CD_COMMAND_KEPT_BYTES = 10, SENSE_NOT_READY = 4,
    COMMAND_SUBQ = 0xDD, SUBQ_BYTES = 10, COMMAND_DINFO = 0xDE, DINFO_BYTES = 4,
    COMMAND_PAUSE = 0xDA, COMMAND_SEARCH = 0xD8,
    SEARCH_KIND_BITS = 0xC0, SEARCH_KIND_TRACK = 0x80, SEARCH_NO_WAIT = 0x01,
    SEARCH_PLAY_AFTER = 0x02, SEARCH_DONE_IRQ = 0x20, SEARCH_KIND_MASKED = 0xC0,
    COMMAND_PLAY = 0xD9, PLAY_MODE_BITS = 0x07, PLAY_KIND_DISC_END = 0xC0, PLAY_KIND_TIME = 0x40,
    PLAY_MODE_KEEP_LAST = 4, PLAY_MODE_DONE = 1, PLAY_MODE_ON_INTERRUPT = 2,
    CARD_ARG_CX = 4,
    CD_FRAMES_PER_SECOND = 0x75, CD_LAST_FRAME = 0x74,
    CD_SECONDS_PER_MINUTE = 0x60, CD_LAST_SECOND = 0x59
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

/* What a send left in A, which the callers compare with the data-in phase. */
static uint8_t cd_sent_value(CdSent sent)
{
    switch (sent.how) {
    case CD_SENT_PENDING:
        return sent.pending;
    case CD_SENT_NO_WAIT:
        return rd8((uint16_t)(CardCdCommand + 1));
    case CD_SENT_ANSWERED:
        return sent.phase;
    default:
        return 1;
    }
}

/*
 * @card 00:EB5E cd_drop_extra_data
 * WHAT IT DOES: Throws away what the drive still has to send, until it moves to its status.
 * HOW: Reads whole blocks through the auto port and drops them.
 * Entry: nothing. Exit: A $D8, N 1, Z 0, C 1; X 0 and Y 0 if a block was read. V kept. Gives
 * whether a block was read.
 */
bool cd_drop_extra_data(void)
{
    bool dropped = false;

    for (;;) {
        uint8_t phase = read_bus_phase();

        if (phase == PHASE_STATUS)
            return dropped;
        if (phase == PHASE_DATA_IN) {
            for (unsigned i = 0; i < DROPPED_BLOCK_BYTES; i++)
                rd8(CdDataAuto);
            dropped = true;
        }
    }
}

static void cd_dropped_exit_shim(bool dropped)
{
    cpu.a = PHASE_STATUS;
    abi_nz(PHASE_STATUS);
    abi_flag(P_C, true);
    if (dropped) {
        cpu.x = 0;
        cpu.y = 0;
    }
}

void abi_cd_drop_extra_data(void)
{
    cd_dropped_exit_shim(cd_drop_extra_data());
}

/*
 * @card 00:EA3B cd_read_data_in
 * WHAT IT DOES: Takes bytes from the drive to where _BX points, up to the count in _AX.
 * HOW: A byte and an acknowledge for each while the bus is in its data phase; if the drive
 * had more than was asked for, cd_drop_extra_data.
 * Entry: _AX count, _BX pointer. Exit: A the phase's answer bits, N, Z, C from comparing it
 * with $88, or as cd_drop_extra_data. V from the last acknowledge, else kept.
 */
CdDataIn cd_read_data_in(void)
{
    CdDataIn in = { false, 0, 0, false, false };
    uint8_t pointer_low = rd8(CardBl);
    uint8_t count_low = rd8(CardAl);
    uint16_t to = (uint16_t)(pointer_low | (rd8(CardBh) << 8));
    uint16_t count = (uint16_t)(count_low | (rd8((uint16_t)(CardAl + 1)) << 8));

    for (;;) {
        uint8_t phase = read_bus_phase();

        if (phase == PHASE_STATUS)
            break;
        if (phase != PHASE_DATA_IN)
            continue;
        wr8(to, rd8(CdData));
        in.last_control = cd_ack_pulse();
        in.acked = true;
        to++;
        count--;
        if (count == 0)
            break;
    }
    wr8(CardBl, (uint8_t)to);
    wr8(CardBh, (uint8_t)(to >> 8));
    wr8(CardAl, (uint8_t)count);
    wr8((uint16_t)(CardAl + 1), (uint8_t)(count >> 8));
    in.answer = read_bus_phase() & PHASE_ANSWER_BITS;
    if (in.answer == PHASE_STATUS_DONE) {
        in.drained = true;
        in.drained_block = cd_drop_extra_data();
    }
    return in;
}

static void cd_data_in_exit_shim(CdDataIn in)
{
    if (in.acked)
        abi_flag(P_V, (in.last_control & 0x40) != 0);
    if (in.drained) {
        cd_dropped_exit_shim(in.drained_block);
    } else {
        cpu.a = in.answer;
        abi_nz((uint8_t)(in.answer - PHASE_STATUS_DONE));
        abi_flag(P_C, in.answer >= PHASE_STATUS_DONE);
    }
}

void abi_cd_read_data_in(void)
{
    cd_data_in_exit_shim(cd_read_data_in());
}

/* A command built in CardCdCommand, sent, its data if the drive offers it, and its status. */
static CdExchange cd_exchange(bool to_result, uint16_t count)
{
    CdExchange exchange;

    exchange.sent = cd_send_command();
    exchange.read_data = count != 0 && cd_sent_value(exchange.sent) == PHASE_DATA_IN;
    if (exchange.read_data) {
        if (to_result) {
            wr8(CardBl, (uint8_t)CardCdResult);
            wr8(CardBh, (uint8_t)(CardCdResult >> 8));
        }
        wr8(CardAl, (uint8_t)count);
        wr8((uint16_t)(CardAl + 1), (uint8_t)(count >> 8));
        exchange.data = cd_read_data_in();
    }
    exchange.finish = cd_finish_command();
    return exchange;
}

/* X, Y and V as an exchange leaves them: each from the last step that set it. */
static void cd_exchange_exit_shim(CdExchange exchange)
{
    cd_send_exit_shim(exchange.sent);
    if (exchange.read_data)
        cd_data_in_exit_shim(exchange.data);
    if (exchange.finish.timed_out) {
        cd_reset_exit_shim(exchange.finish.reset);
        cpu.a = CD_ERROR_TIMED_OUT;
        abi_nz(CD_ERROR_TIMED_OUT);
    } else {
        cd_status_exit_shim(exchange.finish.read);
    }
}

static uint8_t cd_exchange_value(CdExchange exchange)
{
    return exchange.finish.timed_out ? CD_ERROR_TIMED_OUT : exchange.finish.read.status;
}

/*
 * @card 00:F053 cd_request_sense
 * WHAT IT DOES: Asks the drive why the last command failed, and keeps its answer.
 * HOW: Command 3 for ten bytes into CardCdResult; on success bytes 2 and 9 go to the sense key
 * and sense code.
 * Entry: nothing. Exit: A the command's status, N and Z from it, C 1; V, X and Y as the
 * exchange left them. Clobbers _AX, _BX if the drive sent data.
 */
uint8_t cd_request_sense(CdExchange *exchange)
{
    uint8_t status;

    clear_cd_result();
    clear_cd_command();
    wr8(CardCdCommand, COMMAND_REQUEST_SENSE);
    wr8((uint16_t)(CardCdCommand + 4), SENSE_BYTES);
    *exchange = cd_exchange(true, SENSE_BYTES);
    status = cd_exchange_value(*exchange);
    if (status == 0) {
        wr8(CardCdSenseKey, rd8((uint16_t)(CardCdResult + 2)));
        wr8(CardCdSense, rd8((uint16_t)(CardCdResult + 9)));
    }
    return status;
}

static void cd_sense_exit_shim(CdExchange exchange, uint8_t status)
{
    cd_exchange_exit_shim(exchange);
    cpu.a = status;
    abi_nz(status);
    abi_flag(P_C, true);
}

void abi_cd_request_sense(void)
{
    CdExchange exchange;
    uint8_t status = cd_request_sense(&exchange);

    cd_sense_exit_shim(exchange, status);
}

static bool sense_is_fatal(uint8_t sense)
{
    static const uint8_t fatal[] = {
        0x0B, 0x0D, 0x1C, 0x1D, 0x20, 0x21, 0x22, 0x25, 0x2A, 0x2C
    };

    for (unsigned i = 0; i < sizeof fatal; i++)
        if (sense == fatal[i])
            return true;
    return false;
}

/*
 * @card 00:E20B cd_retry_after_error
 * WHAT IT DOES: Decides whether a failed command is worth trying again.
 * HOW: Only error 2 asks the drive why: keeps the command, requests sense, and gives up on the
 * sense codes that retrying cannot mend. Sense code 4 marks the drive not ready.
 * Entry: A error. Exit: C set to give up, A and N, Z as its last compare left them; for error
 * 2, Y $FF unless the exchange changed it, and X and V as the exchange left them.
 */
CdRetry cd_retry_after_error(uint8_t error)
{
    CdRetry retry;

    retry.give_up = false;
    retry.error = error;
    retry.asked = error == CD_ERROR_ASK_SENSE;
    retry.sense = 0;
    if (!retry.asked)
        return retry;
    for (unsigned i = 0; i < CD_COMMAND_KEPT_BYTES; i++)
        wr8((uint16_t)(CardCdFailedCommand + i), rd8((uint16_t)(CardCdCommand + i)));
    retry.sense_status = cd_request_sense(&retry.sense_exchange);
    retry.sense = rd8(CardCdSense);
    if (retry.sense == SENSE_NOT_READY)
        wr8(CardCdNotReady, retry.sense);
    else
        retry.give_up = sense_is_fatal(retry.sense);
    return retry;
}

/* N and Z from the compare the chain stopped at: the match, or the last in the list. */
static uint8_t sense_compared_value(uint8_t sense)
{
    static const uint8_t passes[] = { 0x04, 0x11, 0x15, 0x16 };

    if (sense == 0 || sense_is_fatal(sense))
        return 0;
    for (unsigned i = 0; i < sizeof passes; i++)
        if (sense == passes[i])
            return 0;
    return (uint8_t)(sense - 0x2C);
}

static void cd_retry_exit_shim(CdRetry retry)
{
    if (retry.asked) {
        cpu.y = 0xFF;
        cd_sense_exit_shim(retry.sense_exchange, retry.sense_status);
        cpu.a = retry.sense;
        abi_nz(sense_compared_value(retry.sense));
    } else {
        uint8_t compared = retry.error == CD_ERROR_TIMED_OUT ? CD_ERROR_TIMED_OUT : CD_ERROR_ASK_SENSE;

        cpu.a = retry.error;
        abi_nz((uint8_t)(retry.error - compared));
    }
    abi_flag(P_C, retry.give_up);
}

void abi_cd_retry_after_error(void)
{
    cd_retry_exit_shim(cd_retry_after_error(cpu.a));
}

/*
 * @card 00:F091 cd_test_unit
 * WHAT IT DOES: Asks whether the drive is ready.
 * HOW: A test command; on failure the retry decision, whose answer does not repeat it.
 * Entry: nothing. Exit, well: A 0, N 0, Z 1, C clear. Exit, failed: C set, A and N, Z as
 * cd_retry_after_error leaves them.
 */
CdTest cd_test_unit(void)
{
    CdTest test;

    clear_cd_command();
    wr8(CardCdCommand, COMMAND_TEST_UNIT);
    test.exchange.sent = cd_send_command();
    test.exchange.read_data = false;
    test.exchange.finish = cd_finish_command();
    test.status = cd_exchange_value(test.exchange);
    test.failed = test.status != 0;
    if (test.failed)
        test.retry = cd_retry_after_error(test.status);
    return test;
}

static void cd_test_exit_shim(CdTest test)
{
    cd_exchange_exit_shim(test.exchange);
    if (test.failed) {
        cd_retry_exit_shim(test.retry);
        abi_flag(P_C, true);
    } else {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);
    }
}

void abi_cd_test_unit(void)
{
    cd_test_exit_shim(cd_test_unit());
}

/*
 * @card 00:F347 cd_stat
 * WHAT IT DOES: Says whether the drive is busy; or, for any other request, tests the unit.
 * HOW: Zero reads the bus's busy bit; anything else is cd_test_unit.
 * Entry: A what. Exit, zero: A the bit, N and Z from it, C 1. Exit: as cd_test_unit.
 */
CdStat cd_stat(uint8_t what)
{
    CdStat stat;

    stat.tested = what != 0;
    if (stat.tested)
        stat.test = cd_test_unit();
    else
        stat.busy = rd8(CdBusStatus) & BUS_BUSY;
    return stat;
}

/* What a failed try leaves for the next: a reset or drop clears, a sense request asks. */
static CdLeftOver exchange_left_over(CdExchange exchange, CdLeftOver before)
{
    if (exchange.finish.timed_out || (exchange.read_data && exchange.data.drained_block))
        return CD_LEFT_CLEARED;
    return before;
}

static CdLeftOver retry_left_over(CdRetry retry, CdLeftOver before)
{
    if (!retry.asked)
        return before;
    return exchange_left_over(retry.sense_exchange, CD_LEFT_ASKED);
}

/* Tries a command the same way until it works or the BIOS gives up. */
static CdAttempt cd_try_command(void (*build)(void), uint16_t count, bool restore_always)
{
    CdAttempt attempt;

    attempt.left_over = CD_LEFT_NOTHING;
    for (;;) {
        clear_cd_command();
        build();
        attempt.exchange = cd_exchange(false, count);
        attempt.status = cd_exchange_value(attempt.exchange);
        if (restore_always)
            restore_card_args();
        attempt.failed = attempt.status != 0;
        if (!attempt.failed)
            return attempt;
        attempt.retry = cd_retry_after_error(attempt.status);
        if (attempt.retry.give_up)
            return attempt;
        attempt.left_over = retry_left_over(attempt.retry,
                                            exchange_left_over(attempt.exchange, attempt.left_over));
        restore_card_args();
    }
}

static void cd_attempt_exit_shim(CdAttempt attempt)
{
    if (attempt.left_over == CD_LEFT_CLEARED)
        cpu.y = 0;
    else if (attempt.left_over == CD_LEFT_ASKED)
        cpu.y = 0xFF;
    cd_exchange_exit_shim(attempt.exchange);
    if (attempt.failed) {
        cd_retry_exit_shim(attempt.retry);
    } else {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

static void build_subq(void)
{
    wr8(CardCdCommand, COMMAND_SUBQ);
    wr8((uint16_t)(CardCdCommand + 1), SUBQ_BYTES);
}

/*
 * @card 00:EFBF cd_subq
 * WHAT IT DOES: Reads the drive's ten bytes of position to where _BX points.
 * HOW: Command $DD, tried until it works or the BIOS gives up.
 * Entry: _BX pointer. Exit, well: A 0, N 0, Z 1, C 1. Exit, gave up: as
 * cd_retry_after_error. X, Y and V as the last exchange left them.
 */
CdAttempt cd_subq(void)
{
    save_card_args();
    return cd_try_command(build_subq, SUBQ_BYTES, false);
}

void abi_cd_subq(void)
{
    cd_attempt_exit_shim(cd_subq());
}

static void build_dinfo(void)
{
    wr8(CardCdCommand, COMMAND_DINFO);
    wr8((uint16_t)(CardCdCommand + 1), rd8(CardAl));
    wr8((uint16_t)(CardCdCommand + 2), rd8((uint16_t)(CardAl + 1)));
}

/*
 * @card 00:EFF1 cd_dinfo
 * WHAT IT DOES: Reads a piece of the disc's table of contents to where _BX points.
 * HOW: Command $DE with _AX as what is asked for; the answer is four bytes.
 * Entry: _AX what, _BX pointer. Exit: as cd_subq. _AX and _BX as they were.
 */
CdAttempt cd_dinfo(void)
{
    save_card_args();
    return cd_try_command(build_dinfo, DINFO_BYTES, true);
}

void abi_cd_dinfo(void)
{
    cd_attempt_exit_shim(cd_dinfo());
}

static void ask_dinfo_to_result(uint8_t what)
{
    wr8(CardAl, what);
    wr8(CardBl, (uint8_t)CardCdResult);
    wr8(CardBh, (uint8_t)(CardCdResult >> 8));
}

/*
 * @card 00:F0A9 cd_contnts
 * WHAT IT DOES: Reads the disc's first and last tracks and its length, and keeps them.
 * HOW: cd_dinfo twice into CardCdResult, with the answers copied to CardDiscFirstTrack and
 * CardDiscLength.
 * Entry: _AH the high byte cd_dinfo asks with. Exit, well: A 0, N 0, Z 1, C 1. Exit, failed: A
 * as cd_dinfo left it, N and Z from it, C 1. X, Y, V as cd_dinfo left them.
 */
CdContents cd_contnts(void)
{
    CdContents contents;

    ask_dinfo_to_result(0);
    contents.attempt = cd_dinfo();
    if (contents.attempt.status != 0)
        return contents;
    wr8(CardDiscFirstTrack, rd8(CardCdResult));
    wr8((uint16_t)(CardDiscFirstTrack + 1), rd8((uint16_t)(CardCdResult + 1)));
    ask_dinfo_to_result(1);
    contents.attempt = cd_dinfo();
    if (contents.attempt.status != 0)
        return contents;
    wr8(CardDiscLength, rd8(CardCdResult));
    wr8((uint16_t)(CardDiscLength + 1), rd8((uint16_t)(CardCdResult + 1)));
    wr8((uint16_t)(CardDiscLength + 2), rd8((uint16_t)(CardCdResult + 2)));
    return contents;
}

void abi_cd_contnts(void)
{
    CdContents contents = cd_contnts();

    cd_attempt_exit_shim(contents.attempt);
    abi_nz(cpu.a);
    abi_flag(P_C, true);
}

/*
 * @card 00:EF94 cd_pause
 * WHAT IT DOES: Pauses the drive if it is playing.
 * HOW: cd_subq; a first byte not zero says nothing plays. Otherwise command $DA, and on
 * failure the retry decision, whose try starts again from cd_subq.
 * Entry: nothing. Exit, not playing: A 0, N and Z from the byte, C 1. Exit, paused: A 0, N 0,
 * Z 1, C 1. Exit, gave up: as cd_retry_after_error. Clobbers _BX.
 */
CdPause cd_pause(void)
{
    CdPause pause;

    for (;;) {
        wr8(CardBl, (uint8_t)CardCdResult);
        wr8(CardBh, (uint8_t)(CardCdResult >> 8));
        pause.subq = cd_subq();
        pause.playing = rd8(CardCdResult);
        pause.told_to_pause = pause.playing == 0;
        pause.failed = false;
        if (!pause.told_to_pause)
            return pause;
        clear_cd_command();
        wr8(CardCdCommand, COMMAND_PAUSE);
        pause.pause.sent = cd_send_command();
        pause.pause.read_data = false;
        pause.pause.finish = cd_finish_command();
        pause.status = cd_exchange_value(pause.pause);
        if (pause.status == 0)
            return pause;
        pause.failed = true;
        pause.retry = cd_retry_after_error(pause.status);
        if (pause.retry.give_up)
            return pause;
    }
}

void abi_cd_pause(void)
{
    CdPause pause = cd_pause();

    cd_attempt_exit_shim(pause.subq);
    if (!pause.told_to_pause) {
        cpu.a = 0;
        abi_nz(pause.playing);
        abi_flag(P_C, true);
        return;
    }
    cd_exchange_exit_shim(pause.pause);
    if (pause.failed) {
        cd_retry_exit_shim(pause.retry);
    } else {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
    }
}

/* The command's address and the flags: the mode bits of _BH say how _AX and _BL are read. */
static uint8_t build_search(void)
{
    uint8_t mode = rd8(CardBh);
    uint8_t kind = mode & SEARCH_KIND_BITS;
    uint8_t copied_to = kind == 0 ? 3 : 2;

    wr8(CardCdCommand, COMMAND_SEARCH);
    wr8((uint16_t)(CardCdCommand + 9), kind);
    if (kind == SEARCH_KIND_TRACK) {
        wr8((uint16_t)(CardAl + 1), 0);
        wr8(CardBl, 0);
    }
    copy_args_to_command(0, copied_to);
    wr8(CardCdNoWait, mode & SEARCH_NO_WAIT);
    wr8((uint16_t)(CardCdCommand + 1), (mode & SEARCH_PLAY_AFTER) >> 1);
    return copied_to;
}

/*
 * @card 00:EF34 cd_search
 * WHAT IT DOES: Seeks to a place on the disc; with bit 0 of _BH, returns at once and lets an
 * interrupt report the end.
 * HOW: Command $D8. Bits 6-7 of _BH say what the address is.
 * Entry: _AX, _BL the place, _BH the mode. Exit, on an interrupt: A 0, Z clear, N and V from
 * the control port, C as the send left it. Exit: as cd_subq, with Y the command offset unless
 * a step changed it.
 */
CdSearch cd_search(void)
{
    CdSearch search;

    search.attempt.left_over = CD_LEFT_NOTHING;
    save_card_args();
    for (;;) {
        clear_cd_command();
        search.copied_to = build_search();
        search.attempt.exchange.sent = cd_send_command();
        search.attempt.exchange.read_data = false;
        search.on_interrupt = rd8(CardCdNoWait) != 0;
        if (search.on_interrupt) {
            search.sent = search.attempt.exchange.sent;
            wr8(CardCdBusy, 0);
            search.irq_control = rd8(CdIrqControl);
            wr8(CdIrqControl, search.irq_control | SEARCH_DONE_IRQ);
            wr8(CardCdNoWait, 0);
            return search;
        }
        search.attempt.exchange.finish = cd_finish_command();
        search.attempt.status = cd_exchange_value(search.attempt.exchange);
        search.attempt.failed = search.attempt.status != 0;
        if (!search.attempt.failed)
            return search;
        search.attempt.retry = cd_retry_after_error(search.attempt.status);
        if (search.attempt.retry.give_up)
            return search;
        restore_card_args();
    }
}

void abi_cd_search(void)
{
    CdSearch search = cd_search();

    if (search.on_interrupt) {
        cd_send_exit_shim(search.sent);
        cpu.y = search.copied_to;
        cpu.a = 0;
        abi_flag(P_Z, false);
        abi_flag(P_N, (search.irq_control & 0x80) != 0);
        abi_flag(P_V, (search.irq_control & 0x40) != 0);
        return;
    }
    cpu.y = search.copied_to;
    cd_attempt_exit_shim(search.attempt);
}

/* One less, in packed decimal: a zero low digit borrows, and zero wraps to $99. */
static uint8_t decimal_minus_one(uint8_t value)
{
    if (value & 0x0F)
        return (uint8_t)(value - 1);
    return value == 0 ? 0x99 : (uint8_t)(value - 0x10 + 0x09);
}

/*
 * @card 00:EF02 disc_end_minus_one_frame
 * WHAT IT DOES: Takes one frame off the time kept in CardDiscEnd.
 * HOW: Minutes, seconds and frames in packed decimal: below 0 frames go to 74 and borrow a
 * second, below 0 seconds go to 59 and borrow a minute.
 * Entry: CardDiscEnd. Exit: A kept, N and Z from it; C from the last digit's step. X, Y, V kept.
 */
DiscEndStep disc_end_minus_one_frame(void)
{
    DiscEndStep step = { false, false, 0 };
    uint8_t frames = decimal_minus_one(rd8((uint16_t)(CardDiscEnd + 2)));

    if (frames < CD_FRAMES_PER_SECOND) {
        wr8((uint16_t)(CardDiscEnd + 2), frames);
        return step;
    }
    wr8((uint16_t)(CardDiscEnd + 2), CD_LAST_FRAME);
    {
        uint8_t seconds = decimal_minus_one(rd8((uint16_t)(CardDiscEnd + 1)));

        if (seconds < CD_SECONDS_PER_MINUTE) {
            wr8((uint16_t)(CardDiscEnd + 1), seconds);
            return step;
        }
        wr8((uint16_t)(CardDiscEnd + 1), CD_LAST_SECOND);
    }
    step.borrowed_minute = true;
    step.minutes_before = rd8(CardDiscEnd);
    wr8(CardDiscEnd, decimal_minus_one(step.minutes_before));
    return step;
}

void abi_disc_end_minus_one_frame(void)
{
    DiscEndStep step = disc_end_minus_one_frame();

    abi_nz(cpu.a);
    abi_flag(P_C, step.borrowed_minute && step.minutes_before != 0);
}

/* The search part of a play: bits 6-7 of _BH say what the address is. */
static void build_play_search(void)
{
    uint8_t mode = rd8(CardBh);
    uint8_t kind = mode & SEARCH_KIND_BITS;

    if (kind == SEARCH_KIND_TRACK) {
        wr8((uint16_t)(CardAl + 1), 0);
        wr8(CardBl, 0);
    }
    wr8((uint16_t)(CardCdCommand + 9), kind);
    wr8(CardCdCommand, COMMAND_SEARCH);
    copy_args_to_command(0, mode != 0 ? 2 : 3);
}

/* The play part: bits 6-7 of _DH say what the end address is, and $C0 the disc's end. */
static void build_play_command(void)
{
    uint8_t mode = rd8(CardDh);
    uint8_t kind = mode & SEARCH_KIND_BITS;

    clear_cd_command();
    wr8(CardCdCommand, COMMAND_PLAY);
    wr8((uint16_t)(CardCdCommand + 1), mode & PLAY_MODE_BITS);
    wr8((uint16_t)(CardCdCommand + 9), kind);
    if (kind == 0) {
        copy_args_to_command(CARD_ARG_CX, 3);
        return;
    }
    if (kind != PLAY_KIND_DISC_END) {
        if (kind == SEARCH_KIND_TRACK) {
            wr8(CardCh, 0);
            wr8(CardDl, 0);
        }
        copy_args_to_command(CARD_ARG_CX, 2);
        return;
    }
    for (unsigned i = 0; i < 3; i++)
        wr8((uint16_t)(CardDiscEnd + i), rd8((uint16_t)(CardDiscLength + i)));
    disc_end_minus_one_frame();
    for (unsigned i = 0; i < 3; i++)
        wr8((uint16_t)(CardCdCommand + 2 + i), rd8((uint16_t)(CardDiscEnd + i)));
    wr8((uint16_t)(CardCdCommand + 9), PLAY_KIND_TIME);
}

/*
 * @card 00:EE10 cd_play
 * WHAT IT DOES: Plays CD audio: seeks to the start unless told not to, then starts the play;
 * bit 0 of _BH stops after the seek.
 * HOW: Commands $D8 and $D9. The play's mode goes in _DH; mode 2 of the drive's previous play
 * hands the end to an interrupt.
 * Entry: _AX, _BL start, _BH its mode, _CX, _DL end, _DH its mode. Exit: A 0, C 1 and the rest
 * as the path left them; or as cd_retry_after_error if it gave up.
 */
CdPlay cd_play(void)
{
    CdPlay play;
    uint8_t mode;

    play.how = CD_PLAY_FINISHED;
    play.left_over = CD_LEFT_NOTHING;
    play.searched = false;
    play.failed = false;
    save_card_args();
    mode = rd8(CardBh);
    play.start_mode = mode;
    if ((mode & SEARCH_KIND_BITS) != SEARCH_KIND_MASKED) {
        play.searched = true;
        play.search = cd_try_command(build_play_search, 0, false);
        if (play.search.failed) {
            play.how = CD_PLAY_GAVE_UP_SEARCHING;
            return play;
        }
    }
    if (mode & SEARCH_NO_WAIT) {
        play.how = CD_PLAY_SEEK_ONLY;
        return play;
    }
    for (;;) {
        uint8_t kept;

        build_play_command();
        play.sent = cd_send_command();
        play.sub = rd8((uint16_t)(CardCdCommand + 1));
        kept = play.sub;
        if (cd_sent_value(play.sent) != PHASE_STATUS) {
            if (play.sub == PLAY_MODE_KEEP_LAST)
                kept = rd8(CardCdPlayMode);
            wr8(CardCdPlayMode, kept);
            if (kept == PLAY_MODE_DONE) {
                play.how = CD_PLAY_RETURNED;
                return play;
            }
            if (kept == PLAY_MODE_ON_INTERRUPT) {
                wr8(CardCdBusy, 0);
                play.irq_control = rd8(CdIrqControl);
                wr8(CdIrqControl, play.irq_control | SEARCH_DONE_IRQ);
                play.how = CD_PLAY_ON_INTERRUPT;
                return play;
            }
        }
        if (play.sub == PLAY_MODE_KEEP_LAST)
            wr8(CardCdPlayMode, play.sub);
        play.finish = cd_finish_command();
        play.status = play.finish.timed_out ? CD_ERROR_TIMED_OUT : play.finish.read.status;
        play.failed = play.status != 0;
        if (!play.failed)
            return play;
        play.retry = cd_retry_after_error(play.status);
        if (play.retry.give_up)
            return play;
        restore_card_args();
        if (play.finish.timed_out)
            play.left_over = CD_LEFT_CLEARED;
        play.left_over = retry_left_over(play.retry, play.left_over);
    }
}

void abi_cd_play(void)
{
    CdPlay play = cd_play();

    if (play.searched) {
        cpu.y = play.start_mode != 0 ? 2 : 3;
        cd_attempt_exit_shim(play.search);
    }
    if (play.how == CD_PLAY_GAVE_UP_SEARCHING)
        return;
    abi_flag(P_C, true);
    if (play.how == CD_PLAY_SEEK_ONLY) {
        cpu.a = 0;
        abi_nz(play.start_mode & SEARCH_NO_WAIT);
        return;
    }
    if (play.left_over == CD_LEFT_CLEARED)
        cpu.y = 0;
    else if (play.left_over == CD_LEFT_ASKED)
        cpu.y = 0xFF;
    cd_send_exit_shim(play.sent);
    switch (play.how) {
    case CD_PLAY_RETURNED:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case CD_PLAY_ON_INTERRUPT:
        cpu.a = 0;
        abi_flag(P_Z, false);
        abi_flag(P_N, (play.irq_control & 0x80) != 0);
        abi_flag(P_V, (play.irq_control & 0x40) != 0);
        abi_flag(P_C, true);
        break;
    default:
        if (play.finish.timed_out) {
            cd_reset_exit_shim(play.finish.reset);
            cpu.a = CD_ERROR_TIMED_OUT;
            abi_nz(CD_ERROR_TIMED_OUT);
        } else {
            cd_status_exit_shim(play.finish.read);
        }
        if (play.failed) {
            cd_retry_exit_shim(play.retry);
        } else {
            cpu.a = 0;
            abi_nz(0);
            abi_flag(P_C, true);
        }
        break;
    }
}

void abi_cd_stat(void)
{
    CdStat stat = cd_stat(cpu.a);

    if (stat.tested) {
        cd_test_exit_shim(stat.test);
    } else {
        cpu.a = stat.busy;
        abi_nz(stat.busy);
        abi_flag(P_C, true);
    }
}
