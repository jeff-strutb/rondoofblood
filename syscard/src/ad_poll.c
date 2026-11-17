/* A read from the disc into ADPCM memory run a step at a time: the caller polls it. */
#include "card.h"
#include "syscard.h"

enum {
    BUS_BUSY = 0x80, BUS_REQUEST = 0x40, BUS_SELECT_ID = 0x81, SELECT_TRIES = 256,
    PHASE_BITS = 0xF8, PHASE_COMMAND = 0xD0, PHASE_STATUS = 0xD8, PHASE_MESSAGE = 0xF8,
    ANSWER_BITS = 0xB8, ANSWER_STATUS = 0x98, ANSWER_DATA_IN = 0x88,
    AD_DMA_FROM_CD = 0x02, AD_STROBE_WRITE_TWO = 0x02
};

/* The bus phase, kept where the card keeps it. */
static uint8_t read_bus_phase(void)
{
    uint8_t phase = rd8(CdBusStatus) & PHASE_BITS;

    wr8(CardBusPhase, phase);
    return phase;
}

/*
 * @card 00:F793 ad_trans_select
 * WHAT IT DOES: Selects the drive for ad_trans_step's read, once.
 * HOW: Puts the drive's ID on the bus; if the bus is free, raises select and looks 256 times,
 * with a pause the C does not state, for the drive's request. Seeing it moves the state to 2.
 * Entry: nothing. Exit: A 2 selected, 1 not; N 0, Z 0; V bit 6 of the bus as the busy test
 * read it; X the tries left when selected, 0 when not, kept when the bus was busy. Y, C kept.
 */
AdTransSelect ad_trans_select(void)
{
    AdTransSelect select = { AD_SELECT_TIMED_OUT, 0, 0 };

    wr8(CdData, BUS_SELECT_ID);
    select.bus = rd8(CdBusStatus);
    if (select.bus & BUS_BUSY) {
        select.how = AD_SELECT_BUSY;
        return select;
    }
    wr8(CdBusStatus, BUS_SELECT_ID);
    for (int tried = 0; tried < SELECT_TRIES; tried++) {
        if (rd8(CdBusStatus) & BUS_REQUEST) {
            select.how = AD_SELECTED;
            select.tries_left = (uint8_t)(SELECT_TRIES - tried);
            wr8(CardAdTransStep, AD_TRANS_COMMAND);
            return select;
        }
    }
    return select;
}

static void select_exit_shim(AdTransSelect select)
{
    cpu.a = select.how == AD_SELECTED ? AD_TRANS_COMMAND : AD_TRANS_SELECT;
    abi_nz(cpu.a);
    abi_flag(P_V, (select.bus & 0x40) != 0);
    if (select.how != AD_SELECT_BUSY)
        cpu.x = select.tries_left;
}

void abi_ad_trans_select(void)
{
    select_exit_shim(ad_trans_select());
}

/*
 * @card 00:F7BB ad_trans_command
 * WHAT IT DOES: Moves ad_trans_step's command on by one bus phase.
 * HOW: In the command phase, sends the next byte of CardCdCommand and acknowledges it. A data
 * phase moves the state to 3, a status phase to 4; any other phase does nothing.
 * Entry: nothing. Exit: a byte sent, A $80, X its index, C set, Z, N and V as cd_ack_off leaves
 * them. A data or status phase: A the new state, N 0, Z 0, C set. Another: A the phase and $B8,
 * N, Z and C from comparing it with $88. V and X kept on both. Y kept.
 */
AdTransCommand ad_trans_command(void)
{
    AdTransCommand command = { AD_COMMAND_WAITING, 0, 0, 0 };
    uint8_t phase = read_bus_phase();

    if (phase == PHASE_COMMAND) {
        command.how = AD_COMMAND_SENT;
        command.index = rd8(CardAdTransSent);
        wr8(CardAdTransSent, (uint8_t)(command.index + 1));
        wr8(CdData, rd8((uint16_t)(CardCdCommand + command.index)));
        command.control = cd_ack_pulse();
        return command;
    }
    command.answer = phase & ANSWER_BITS;
    if (command.answer == ANSWER_STATUS) {
        command.how = AD_COMMAND_DONE;
        wr8(CardAdTransStep, AD_TRANS_STATUS);
    } else if (command.answer == ANSWER_DATA_IN) {
        command.how = AD_COMMAND_DONE;
        wr8(CardAdTransStep, AD_TRANS_DATA);
    }
    return command;
}

/* Every path of ad_trans_command's exit but C, which ad_trans_step clears. */
static void command_exit_shim(AdTransCommand command)
{
    switch (command.how) {
    case AD_COMMAND_SENT:
        cpu.a = 0x80;
        cpu.x = command.index;
        abi_flag(P_Z, (command.control & 0x7F) == 0);
        abi_flag(P_N, (command.control & 0x80) != 0);
        abi_flag(P_V, (command.control & 0x40) != 0);
        break;
    case AD_COMMAND_DONE:
        cpu.a = command.answer == ANSWER_STATUS ? AD_TRANS_STATUS : AD_TRANS_DATA;
        abi_nz(cpu.a);
        break;
    default:
        cpu.a = command.answer;
        abi_nz((uint8_t)(command.answer - ANSWER_DATA_IN));
        break;
    }
}

void abi_ad_trans_command(void)
{
    AdTransCommand command = ad_trans_command();

    command_exit_shim(command);
    abi_flag(P_C, command.how != AD_COMMAND_WAITING || command.answer >= ANSWER_DATA_IN);
}

/*
 * @card 00:F7EF ad_trans_start_dma
 * WHAT IT DOES: Starts the drive's data flowing into ADPCM memory, at the address given.
 * HOW: CardAdTransAddress to the ADPCM unit as its write address, the transfer from the CD
 * on, and the state to 4.
 * Entry: nothing. Exit: A 4, N 0, Z 0, V bit 6 of the ADPCM control port as the strobe found
 * it. X, Y, C kept.
 */
uint8_t ad_trans_start_dma(void)
{
    uint8_t control;

    ad_set_address(rd8(CardAdTransAddress), rd8((uint16_t)(CardAdTransAddress + 1)));
    control = ad_latch_write_address();
    wr8(AdpcmDma, AD_DMA_FROM_CD);
    wr8(CardAdTransStep, AD_TRANS_STATUS);
    return control;
}

static void start_dma_exit_shim(uint8_t control)
{
    cpu.a = AD_TRANS_STATUS;
    abi_nz(cpu.a);
    abi_flag(P_V, (control & 0x40) != 0);
}

void abi_ad_trans_start_dma(void)
{
    start_dma_exit_shim(ad_trans_start_dma());
}

/*
 * @card 00:F809 ad_trans_finish
 * WHAT IT DOES: Once the drive is in its status phase, ends ad_trans_step's read.
 * HOW: Stops the transfer, takes the status byte into CardCdStatus and the message byte into
 * CardBusPhase, each with its handshake, waits for the bus to go free and sets the state to 0.
 * Entry: nothing. Exit: done, A the status, N and Z from it, C set, V bit 6 of the bus's last
 * read. Not yet, A the phase, N from it less $D8, Z 0, C clear, V kept. X, Y kept.
 */
AdTransFinish ad_trans_finish(void)
{
    AdTransFinish finish = { false, 0, 0, 0 };

    finish.phase = read_bus_phase();
    if (finish.phase != PHASE_STATUS)
        return finish;
    wr8(AdpcmDma, 0);
    finish.status = rd8(CdData);
    wr8(CardCdStatus, finish.status);
    cd_ack_on();
    while (rd8(CdBusStatus) & BUS_REQUEST)
        ;
    cd_ack_off();
    while (read_bus_phase() != PHASE_MESSAGE)
        ;
    wr8(CardBusPhase, rd8(CdData));
    cd_ack_on();
    while (rd8(CdBusStatus) & BUS_REQUEST)
        ;
    cd_ack_off();
    do
        finish.last_bus = rd8(CdBusStatus);
    while (finish.last_bus & BUS_BUSY);
    wr8(CardAdTransStep, AD_TRANS_IDLE);
    finish.done = true;
    finish.status = rd8(CardCdStatus);
    return finish;
}

static void finish_exit_shim(AdTransFinish finish)
{
    abi_flag(P_C, finish.done);
    if (finish.done) {
        cpu.a = finish.status;
        abi_nz(cpu.a);
        abi_flag(P_V, (finish.last_bus & 0x40) != 0);
    } else {
        cpu.a = finish.phase;
        abi_nz((uint8_t)(finish.phase - PHASE_STATUS));
    }
}

void abi_ad_trans_finish(void)
{
    finish_exit_shim(ad_trans_finish());
}

/*
 * @card 00:F76C ad_trans_step
 * WHAT IT DOES: Takes one step of the read from the disc into ADPCM memory that the entry at
 * $F730 sets up (command $08, sectors and count from _CL-_DL and _AL, ADPCM address _BX),
 * without waiting on the drive: the caller calls it until it returns with C set.
 * HOW: Dispatches on CardAdTransStep: 1 ad_trans_select, 2 ad_trans_command, 3
 * ad_trans_start_dma, 4 ad_trans_finish. 0 and anything above 4 do nothing.
 * Entry: nothing. Exit: state 4 as ad_trans_finish. Otherwise C clear, and A, X, N, Z and V as
 * the step left them; state 0, A 0, N 0, Z 1; above 4, A the state, N and Z from it less 4.
 * Y kept.
 */
AdTransStep ad_trans_step(void)
{
    AdTransStep step = { 0 };

    step.state = rd8(CardAdTransStep);
    switch (step.state) {
    case AD_TRANS_SELECT:
        step.select = ad_trans_select();
        break;
    case AD_TRANS_COMMAND:
        step.command = ad_trans_command();
        break;
    case AD_TRANS_DATA:
        step.control = ad_trans_start_dma();
        break;
    case AD_TRANS_STATUS:
        step.finish = ad_trans_finish();
        break;
    default:
        break;
    }
    return step;
}

void abi_ad_trans_step(void)
{
    AdTransStep step = ad_trans_step();

    switch (step.state) {
    case AD_TRANS_SELECT:
        select_exit_shim(step.select);
        break;
    case AD_TRANS_COMMAND:
        command_exit_shim(step.command);
        break;
    case AD_TRANS_DATA:
        start_dma_exit_shim(step.control);
        break;
    case AD_TRANS_STATUS:
        finish_exit_shim(step.finish);
        return;
    case AD_TRANS_IDLE:
        cpu.a = 0;
        abi_nz(0);
        break;
    default:
        cpu.a = step.state;
        abi_nz((uint8_t)(step.state - AD_TRANS_STATUS));
        break;
    }
    abi_flag(P_C, false);
}
