/*
 * The card's sound driver for its CD player screens, in bank 2 ($C000 to $DFFF, mapped at MPR6): six channels of PSG sequences
 * kept as arrays in the card's RAM, stepped once a frame.
 *
 * No records yet (row request to the typed access session): the channels' arrays at $2474 to $2613, the song header pointers
 * at $22D0 to $22D3, the player's flags at $22DE to $22E2, and the PSG's ports at $0800 to $0807. They are read and written
 * by their addresses.
 */
#include "card.h"
#include "card_access.h"
#include "card_trk.h"
#include "syscard.h"

enum {
    CHANNEL_STATE = 0x2607, TEMPO_ON = 0x2613, TEMPO_PHASE = 0x2614, TEMPO_TICKS = 0x2615, TEMPO_LIMIT = 0x1E, CHANNELS = 6,
    STATE_STOP_ASKED = 2
};

/*
 * @card 02:D623 psg_tempo_tick
 * WHAT IT DOES: Counts the sound's fade-out time: while the fade is on, adds the step to a phase and counts its overflows,
 * and when the count reaches $1E asks every channel that is playing to stop.
 * HOW: $2613 the fade's step (zero: off), added to $2614 in seven bits; a carry into bit 7 is one tick of $2615; at $1E ticks every
 * channel whose state ($2607 plus the channel) is not zero is set to 2.
 * WAITS: no.
 * Entry: nothing. Exit: A, X and the flags as the last step left them: off, A 0, Z; below the limit, A the ticks, C 0; else X $FF,
 * N, C 1.
 */
CardExit psg_tempo_tick(void)
{
    Trk t = { 0 };
    Sum sum;
    uint8_t value;
    int x;

    value = rd8(TEMPO_ON);
    t_a(&t, value);
    if (value == 0)
        return t_exit(&t);
    t.c = false;
    t.c_set = true;
    value = rd8(TEMPO_PHASE);
    sum = add8(t.a, value, false);
    t_a(&t, sum.value);
    t.c = sum.carry;
    t.v = sum.overflow;
    t.v_set = true;
    if ((sum.value & 0x80) != 0) {
        value = rd8(TEMPO_TICKS);
        value = (uint8_t)(value + 1);
        wr8(TEMPO_TICKS, value);
        t_nz(&t, value);
        t_a(&t, (uint8_t)(sum.value & 0x7F));
    }
    wr8(TEMPO_PHASE, t.a);
    value = rd8(TEMPO_TICKS);
    t_a(&t, value);
    t_cmp(&t, value, TEMPO_LIMIT);
    if (value < TEMPO_LIMIT)
        return t_exit(&t);
    for (x = CHANNELS - 1; x >= 0; x--) {
        value = rd8((uint16_t)(CHANNEL_STATE + x));
        t_a(&t, value);
        if (value != 0) {
            t_a(&t, STATE_STOP_ASKED);
            wr8((uint16_t)(CHANNEL_STATE + x), STATE_STOP_ASKED);
        }
        t.x = (uint8_t)(x - 1);
        t.x_set = true;
        t_nz(&t, t.x);
    }
    return t_exit(&t);
}

void abi_psg_tempo_tick(void)
{
    card_exit_apply(psg_tempo_tick());
}
