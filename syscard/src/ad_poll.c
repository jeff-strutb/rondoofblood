/* A read from the disc into ADPCM memory run a step at a time: the caller polls it. */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    BUS_BUSY = 0x80, BUS_REQUEST = 0x40, BUS_SELECT_ID = 0x81, SELECT_TRIES = 256,
    PHASE_BITS = 0xF8, PHASE_COMMAND = 0xD0, PHASE_STATUS = 0xD8, PHASE_MESSAGE = 0xF8,
    ANSWER_BITS = 0xB8, ANSWER_STATUS = 0x98, ANSWER_DATA_IN = 0x88,
    AD_DMA_FROM_CD = 0x02, BUS_ACK_A = 0x80, STEP_STATUS = 4, STEP_DATA = 3, STEP_COMMAND = 2, STEP_SELECT = 1
};

/* What cd_ack_on leaves: A $80, Z clear, N and V from what the control port held. */
static CardExit ack_on_exit(uint8_t held)
{
    CardExit e = { 0 };

    e.a = BUS_ACK_A;
    e.n = (held & 0x80) != 0;
    e.v = (held & 0x40) != 0;
    e.z = false;
    e.set_a = e.set_n = e.set_v = e.set_z = true;
    return e;
}

/* What cd_ack_off leaves: A $80, N and V from what the port held, Z if nothing else was set in it. */
static CardExit ack_off_exit(uint8_t held)
{
    CardExit e = ack_on_exit(held);

    e.z = (held & ~BUS_ACK_A) == 0;
    return e;
}

/*
 * @card 00:F793 ad_trans_select
 * WHAT IT DOES: Selects the drive for ad_trans_step's read, once.
 * HOW: Puts the drive's ID on the bus; if the bus is free, raises select and looks 256 times,
 * with a pause the C does not state, for the drive's request. Seeing it moves the state to 2.
 * WAITS: no: 256 looks at most.
 * Entry: nothing. Exit: A 2 selected, 1 not; N 0, Z 0; V bit 6 of the bus as the busy test
 * read it; X the tries left when selected, 0 when not, kept when the bus was busy. Y, C kept.
 */
CardExit ad_trans_select(void)
{
    CardExit e = { 0 };
    uint8_t bus;
    unsigned x = 0;

    WR8(CCDU->data, BUS_SELECT_ID);
    bus = RD8(CCDU->bus_status);
    e.v = (bus & 0x40) != 0;
    e.set_v = true;
    e.a = 1;
    e.nz = 1;
    e.set_a = e.set_nz = true;
    if ((bus & BUS_BUSY) != 0)
        return e;
    WR8(CCDU->bus_status, BUS_SELECT_ID);
    do {
        bus = RD8(CCDU->bus_status);
        if ((bus & BUS_REQUEST) != 0) {
            WR8(CRAM->ad_trans_step, STEP_COMMAND);
            e.a = e.nz = STEP_COMMAND;
            e.x = (uint8_t)x;
            e.set_x = true;
            return e;
        }
        x = (x - 1) & 0xFF;
    } while (x != 0);
    e.x = 0;
    e.set_x = true;
    return e;
}

/*
 * @card 00:F7BB ad_trans_command
 * WHAT IT DOES: Moves ad_trans_step's command on by one bus phase.
 * HOW: In the command phase, sends the next byte of CardCdCommand and acknowledges it. A data
 * phase moves the state to 3, a status phase to 4; any other phase does nothing.
 * WAITS: through cd_ack_pulse, on the drive's request bit, in the command phase.
 * Entry: nothing. Exit: a byte sent, A $80, X its index, C set, Z, N and V as cd_ack_off leaves
 * them. A data or status phase: A the new state, N 0, Z 0, C set. Another: A the phase and $B8,
 * N, Z and C from comparing it with $88. V and X kept on both. Y kept.
 */
CardExit ad_trans_command(void)
{
    CardExit e = { 0 };
    uint8_t phase = (uint8_t)(RD8(CCDU->bus_status) & PHASE_BITS);
    uint8_t answer;
    uint8_t index;
    uint8_t byte;

    WR8(CRAM->bus_phase, phase);
    if (phase == PHASE_COMMAND) {
        index = RD8(CRAM->ad_trans_sent);
        (void)INC8(CRAM->ad_trans_sent);
        byte = RD8_IDX(COL(CRAM->cd_command), index);
        WR8(CCDU->data, byte);
        e = ack_off_exit(cd_ack_pulse());
        e.x = index;
        e.c = true;
        e.set_x = e.set_c = true;
        return e;
    }
    answer = (uint8_t)(phase & ANSWER_BITS);
    e.a = answer;
    e.nz = (uint8_t)(answer - ANSWER_DATA_IN);
    e.c = answer >= ANSWER_DATA_IN;
    e.set_a = e.set_nz = e.set_c = true;
    if (answer == ANSWER_STATUS) {
        e.a = e.nz = STEP_STATUS;
        WR8(CRAM->ad_trans_step, STEP_STATUS);
    } else if (answer == ANSWER_DATA_IN) {
        e.a = e.nz = STEP_DATA;
        WR8(CRAM->ad_trans_step, STEP_DATA);
    }
    return e;
}

/*
 * @card 00:F7EF ad_trans_start_dma
 * WHAT IT DOES: Starts the drive's data flowing into ADPCM memory, at the address given.
 * HOW: CardAdTransAddress to the ADPCM unit as its write address, the transfer from the CD
 * on, and the state to 4.
 * WAITS: no.
 * Entry: nothing. Exit: A 4, N 0, Z 0, V bit 6 of the ADPCM control port as the strobe found
 * it. X, Y, C kept.
 */
CardExit ad_trans_start_dma(void)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t held;

    value = RD8(CRAM->ad_trans_address);
    wr8(AdpcmAddressLo, value);
    value = rd8((uint16_t)(CardAdTransAddress + 1));
    WR8(CCDU->adpcm_address_hi, value);
    jsr_push(0xF7FE);
    held = ad_latch_write_address();
    jsr_pop();
    WR8(CCDU->adpcm_dma, AD_DMA_FROM_CD);
    WR8(CRAM->ad_trans_step, STEP_STATUS);
    e.a = e.nz = STEP_STATUS;
    e.v = (held & 0x40) != 0;
    e.set_a = e.set_nz = e.set_v = true;
    return e;
}

/*
 * @card 00:F809 ad_trans_finish
 * WHAT IT DOES: Once the drive is in its status phase, ends ad_trans_step's read.
 * HOW: Stops the transfer, takes the status byte into CardCdStatus and the message byte into
 * CardBusPhase, each with its handshake, waits for the bus to go free and sets the state to 0.
 * WAITS: on the bus, with the stack as the caller's and the two bytes of its own frame.
 * Entry: nothing. Exit: done, A the status, N and Z from it, C set, V bit 6 of the bus's last
 * read. Not yet, A the phase, N from it less $D8, Z 0, C clear, V kept. X, Y kept.
 */
CardExit ad_trans_finish(void)
{
    CardExit e = { 0 };
    uint8_t phase = (uint8_t)(RD8(CCDU->bus_status) & PHASE_BITS);
    uint8_t value;
    uint8_t bus;

    WR8(CRAM->bus_phase, phase);
    if (phase != PHASE_STATUS) {
        e.a = phase;
        e.n = ((uint8_t)(phase - PHASE_STATUS) & 0x80) != 0;
        e.z = false;
        e.c = false;
        e.set_a = e.set_n = e.set_z = e.set_c = true;
        return e;
    }
    WR8(CCDU->adpcm_dma, 0);
    value = RD8(CCDU->data);
    WR8(CRAM->cd_status, value);
    jsr_push(0xF821);
    (void)cd_ack_on();
    jsr_pop();
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & BUS_REQUEST) != 0);
    jsr_push(0xF82A);
    (void)cd_ack_off();
    jsr_pop();
    do {
        phase = (uint8_t)(RD8(CCDU->bus_status) & PHASE_BITS);
        WR8(CRAM->bus_phase, phase);
    } while (phase != PHASE_MESSAGE);
    value = RD8(CCDU->data);
    WR8(CRAM->bus_phase, value);
    jsr_push(0xF83F);
    (void)cd_ack_on();
    jsr_pop();
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & BUS_REQUEST) != 0);
    jsr_push(0xF848);
    (void)cd_ack_off();
    jsr_pop();
    do {
        bus = RD8(CCDU->bus_status);
    } while ((bus & BUS_BUSY) != 0);
    WR8(CRAM->ad_trans_step, 0);
    value = RD8(CRAM->cd_status);
    e.a = e.nz = value;
    e.c = true;
    e.v = (bus & 0x40) != 0;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

/*
 * @card 00:F76C ad_trans_step
 * WHAT IT DOES: Takes one step of the read from the disc into ADPCM memory that the entry at
 * $F730 sets up (command $08, sectors and count from _CL-_DL and _AL, ADPCM address _BX),
 * without waiting on the drive: the caller calls it until it returns with C set.
 * HOW: Dispatches on CardAdTransStep: 1 ad_trans_select, 2 ad_trans_command, 3
 * ad_trans_start_dma, 4 ad_trans_finish. 0 and anything above 4 do nothing.
 * WAITS: through ad_trans_command and ad_trans_finish.
 * Entry: nothing. Exit: state 4 as ad_trans_finish. Otherwise C clear, and A, X, N, Z and V as
 * the step left them; state 0, A 0, N 0, Z 1; above 4, A the state, N and Z from it less 4.
 * Y kept.
 */
CardExit ad_trans_step(void)
{
    CardExit e = { 0 };
    CardExit step = { 0 };
    uint8_t state = RD8(CRAM->ad_trans_step);

    e.a = state;
    e.nz = state;
    e.set_a = e.set_nz = true;
    if (state == STEP_SELECT) {
        jsr_push(0xF777);
        step = ad_trans_select();
        jsr_pop();
        e = card_exit_over(e, step);
    } else if (state == STEP_COMMAND) {
        jsr_push(0xF77F);
        step = ad_trans_command();
        jsr_pop();
        e = card_exit_over(e, step);
    } else if (state == STEP_DATA) {
        jsr_push(0xF787);
        step = ad_trans_start_dma();
        jsr_pop();
        e = card_exit_over(e, step);
    } else if (state == STEP_STATUS) {
        jsr_push(0xF790);
        step = ad_trans_finish();
        jsr_pop();
        return card_exit_over(e, step);
    } else if (state != 0) {
        e.nz = (uint8_t)(state - STEP_STATUS);
    }
    e.c = false;
    e.set_c = true;
    return e;
}

void abi_ad_trans_select(void)
{
    card_exit_apply(ad_trans_select());
}

void abi_ad_trans_command(void)
{
    card_exit_apply(ad_trans_command());
}

void abi_ad_trans_start_dma(void)
{
    card_exit_apply(ad_trans_start_dma());
}

void abi_ad_trans_finish(void)
{
    card_exit_apply(ad_trans_finish());
}

void abi_ad_trans_step(void)
{
    card_exit_apply(ad_trans_step());
}
