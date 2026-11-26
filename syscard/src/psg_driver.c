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
    STATE_STOP_ASKED = 2, PSG_SELECT = 0x0800, PSG_CONTROL = 0x0804, PSG_WAVE = 0x0806, PSG_NOISE = 0x0807, PSG_FREQ_LOW = 0x0802,
    PSG_FREQ_HIGH = 0x0803, PSG_BALANCE = 0x0805, CHANNEL_FLAGS = 0x25FF, NOISE_FLAGS = 0x2601, FREQ_LOW = 0x25E7, FREQ_HIGH = 0x25ED,
    BALANCE = 0x25F9, VOLUME = 0x25F3, PLAYER_MODE_AT = 0x22DF, CHANNEL_USED = 0x247C, CHANNEL_BUSY = 0x2474, WAVE_TABLE_USER = 0x22D2,
    WAVE_TABLE_BUILT_IN = 0xD9A6, WAVE_BUILT_IN_COUNT = 0x2D, WAVE_BYTES = 0x20, ZP_E8 = 0x20E8, ZP_E9 = 0x20E9, ZP_EA = 0x20EA, ZP_EB = 0x20EB,
    CHANNEL_NOW = 0x22E0, PITCH_STEP = 0x255D, PITCH_FRACTION = 0x2563, PITCH_HIGH = 0x24AF, LEFT_STEP = 0x2569, LEFT_FRACTION = 0x256F,
    RIGHT_STEP = 0x2575, RIGHT_FRACTION = 0x257B, SEQ_KIND = 0x25E1, SEQ_STATE = 0x260D, SEQ_POINTER_LOW = 0x259F, SEQ_POINTER_HIGH = 0x25A5,
    NOISE_AT = 0x2601, FREQ_HIGH_AT = 0x25ED, FREQ_LOW_AT = 0x25E7, SHAPE_LOW = 0x24D9, SHAPE_HIGH = 0x24DF, BALANCE_AT = 0x25F9, FLAGS_AT = 0x25FF,
    SHAPES_BUILT_IN = 0xD8A1, SHAPES_USER = 0x22D5
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

/*
 * @card 02:D706 psg_wave_offset
 * WHAT IT DOES: Makes a waveform's number into its offset in the table: the number times 32, in _EX and _EY zero page.
 * HOW: Three shifts through zero page $E8.
 * WAITS: no.
 * Entry: A the number. Exit: $E8 the low byte and $E9 the high; A the high byte, C the last bit shifted out. N, Z from the last ROR; X, Y kept.
 */
CardExit psg_wave_offset(uint8_t number)
{
    CardExit e = { 0 };
    uint8_t a = number;
    uint8_t low = 0;
    uint8_t carry;
    uint8_t value;

    wr8(ZP_E8, 0);
    for (int i = 0; i < 3; i++) {
        carry = (uint8_t)(a & 1);
        a = (uint8_t)(a >> 1);
        value = rd8(ZP_E8);
        low = (uint8_t)((carry << 7) | (value >> 1));
        wr8(ZP_E8, low);
        e.c = (value & 1) != 0;
    }
    wr8(ZP_E9, a);
    e.a = a;
    e.n = (low & 0x80) != 0;
    e.z = low == 0;
    e.set_a = e.set_n = e.set_z = e.set_c = true;
    return e;
}

void abi_psg_wave_offset(void)
{
    card_exit_apply(psg_wave_offset(cpu.a));
}

/* The built-in table's address plus the offset psg_wave_offset left in $E8 and $E9, as the two adds make it. */
static void add_wave_base(Trk *t, uint8_t low_base, uint8_t high_base, bool low_from_memory, uint16_t memory)
{
    uint8_t value;
    Sum sum;
    uint8_t base = low_base;

    if (low_from_memory) {
        base = rd8(memory);
        t_a(t, base);
    } else {
        t_a(t, base);
    }
    value = rd8(ZP_E8);
    sum = add8(base, value, false);
    t_a(t, sum.value);
    t->c = sum.carry;
    t->v = sum.overflow;
    t->c_set = t->v_set = true;
    wr8(ZP_E8, sum.value);
    base = high_base;
    if (low_from_memory) {
        base = rd8((uint16_t)(memory + 1));
        t_a(t, base);
    } else {
        t_a(t, base);
    }
    value = rd8(ZP_E9);
    sum = add8(base, value, sum.carry);
    t_a(t, sum.value);
    t->c = sum.carry;
    t->v = sum.overflow;
    wr8(ZP_E9, sum.value);
}

/* One channel of psg_write_channels, X the channel. */
static void write_channel(Trk *t, uint8_t channel)
{
    uint8_t value;
    uint8_t number;
    Sum sum;
    CardExit step;

    value = rd8((uint16_t)(CHANNEL_STATE + channel));
    t_a(t, value);
    t_cmp(t, value, 1);
    if (t->n)
        return;
    wr8(PSG_SELECT, channel);
    if (!t->z) {
        wr8(PSG_CONTROL, 0);
        wr8((uint16_t)(CHANNEL_STATE + channel), 0);
        value = rd8(PLAYER_MODE_AT);
        t_a(t, value);
        t_cmp(t, value, 1);
        if (value == 1)
            return;
        value = rd8((uint16_t)(CHANNEL_USED + channel));
        t_a(t, value);
        if (value == 0)
            return;
        value = rd8((uint16_t)(CHANNEL_BUSY + channel));
        t_a(t, (uint8_t)(value & 0x7F));
        wr8((uint16_t)(CHANNEL_BUSY + channel), t->a);
        return;
    }
    value = rd8((uint16_t)(CHANNEL_FLAGS + channel));
    t_a(t, value);
    if (!t->n) {
        t_a(t, (uint8_t)(value | 0x80));
        wr8((uint16_t)(CHANNEL_FLAGS + channel), t->a);
        t_a(t, (uint8_t)(t->a & 0x7F));
        number = t->a;
        t_cmp(t, number, WAVE_BUILT_IN_COUNT);
        if (number < WAVE_BUILT_IN_COUNT) {
            jsr_push(0xD68E);
            step = psg_wave_offset(number);
            jsr_pop();
            t_take(t, step);
            add_wave_base(t, 0xA6, 0xD9, false, 0);
        } else {
            t_a(t, (uint8_t)(number - WAVE_BUILT_IN_COUNT));
            t->c = true;
            t->c_set = true;
            jsr_push(0xD6A3);
            step = psg_wave_offset((uint8_t)(number - WAVE_BUILT_IN_COUNT));
            jsr_pop();
            t_take(t, step);
            add_wave_base(t, 0, 0, true, WAVE_TABLE_USER);
        }
        t_a(t, 0x40);
        wr8(PSG_CONTROL, 0x40);
        wr8(PSG_CONTROL, 0);
        for (uint8_t y = 0; y < WAVE_BYTES; y++) {
            value = rd8_zp_y(ZP_E8, y);
            wr8(PSG_WAVE, value);
            t_a(t, value);
            t->y = (uint8_t)(y + 1);
            t->y_set = true;
        }
        t_cmp(t, WAVE_BYTES, WAVE_BYTES);
        t->y = WAVE_BYTES;
    }
    {
        bool noise = false;

        t_cmp(t, channel, 4);
        if (channel >= 4) {
            value = rd8((uint16_t)(NOISE_FLAGS + channel));
            t_a(t, value);
            if ((value & 0x80) == 0) {
                t_a(t, (uint8_t)(value | 0x80));
                wr8(PSG_NOISE, t->a);
                noise = true;
            }
        }
        if (!noise) {
            wr8(PSG_NOISE, 0);
            value = rd8((uint16_t)(FREQ_LOW + channel));
            t_a(t, value);
            wr8(PSG_FREQ_LOW, value);
            value = rd8((uint16_t)(FREQ_HIGH + channel));
            t_a(t, value);
            wr8(PSG_FREQ_HIGH, value);
        }
    }
    value = rd8((uint16_t)(BALANCE + channel));
    t_a(t, value);
    wr8(PSG_BALANCE, value);
    value = rd8((uint16_t)(VOLUME + channel));
    t_a(t, value);
    value = rd8(TEMPO_TICKS);
    sum = sub8(t->a, value, true);
    t_a(t, sum.value);
    t->c = sum.carry;
    t->v = sum.overflow;
    t->c_set = t->v_set = true;
    if (sum.value == 0 || (sum.value & 0x80) != 0) {
        wr8(PSG_CONTROL, 0);
    } else {
        t_a(t, (uint8_t)(sum.value | 0x80));
        wr8(PSG_CONTROL, t->a);
    }
}

/*
 * @card 02:D64D psg_write_channels
 * WHAT IT DOES: Writes the six channels' state to the PSG, from channel 5 down to 0: a channel asked to stop is silenced and forgotten; a
 * new one gets its waveform, noise or frequency, balance and volume.
 * HOW: Each channel's state at $2607: below 1 skipped; 2 and up: key off, state cleared, and the channel's busy bit cleared in $2474
 * unless the player's mode is 1 or the channel is not used; 1: a new note: the waveform ($25FF bit 7 not yet set) copied as 32 bytes
 * from the built-in table at $D9A6 or the user's at $22D2, then noise (channels 4 and 5, if bit 7 of $2601 is clear) or frequency,
 * the balance, and the volume less the fade ($2615), written with the key on, or the key off if none is left.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N from it; A and the other flags as the last channel left them.
 * The PSG's registers at $0800 to $0807 and the arrays are by address; psg_wave_offset's frames are at $D68E and $D6A3.
 */
CardExit psg_write_channels(void)
{
    Trk t = { 0 };

    for (int x = CHANNELS - 1; x >= 0; x--) {
        write_channel(&t, (uint8_t)x);
        t.x = (uint8_t)(x - 1);
        t.x_set = true;
        t_nz(&t, t.x);
    }
    return t_exit(&t);
}

void abi_psg_write_channels(void)
{
    card_exit_apply(psg_write_channels());
}

/* A, as it stands, rolled left twice and masked to the bit that came round: the fraction's carry into the whole part. */
static void roll_carry(Trk *t)
{
    t_rol(t);
    t_rol(t);
    t_a(t, (uint8_t)(t->a & 1));
}

/*
 * @card 02:D257 psg_slide_channel
 * WHAT IT DOES: Moves the current channel's pitch, left volume and right volume one step towards where its slides are taking them.
 * HOW: Each slide has a signed step ($255D, $2569, $2575 plus the channel) and a fraction ($2563, $256F, $257B): the step added to
 * the fraction in seven bits, the carry out of bit 6 into bit 7 added to (or taken from) the whole part ($24AF, the nibbles of $25F9),
 * clipped at its limit, where the slide ends by clearing its step.
 * WAITS: no.
 * Entry: CardPlayChannel ($22E0) the channel. Exit: A, X and the flags as the last step left them.
 */
CardExit psg_slide_channel(void)
{
    Trk t = { 0 };
    uint8_t x = rd8(CHANNEL_NOW);
    uint8_t value;

    t.x = x;
    t.x_set = true;
    t_nz(&t, x);
    value = rd8((uint16_t)(PITCH_STEP + x));
    t_a(&t, value);
    if (value != 0) {
        bool done = false;

        if ((value & 0x80) == 0) {
            t_clc(&t);
            t_adc(&t, rd8((uint16_t)(PITCH_FRACTION + x)));
            wr8(ZP_EA, t.a);
            roll_carry(&t);
            t_clc(&t);
            t_adc(&t, rd8((uint16_t)(PITCH_HIGH + x)));
            if (t.z || t.n)
                done = true;
            if (!done) {
                t.a = 0;
                t.a_set = true;
                wr8((uint16_t)(PITCH_STEP + x), 0);
            }
        } else {
            t_a(&t, (uint8_t)(value ^ 0xFF));
            t_inc(&t);
            t_clc(&t);
            t_adc(&t, rd8((uint16_t)(PITCH_FRACTION + x)));
            wr8(ZP_EA, t.a);
            roll_carry(&t);
            wr8(ZP_EB, t.a);
            value = rd8((uint16_t)(PITCH_HIGH + x));
            t_a(&t, value);
            t_sec(&t);
            t_sbc(&t, rd8(ZP_EB));
            if (!t.z) {
                t_cmp(&t, t.a, 0xE1);
                if (t.a < 0xE1) {
                    t_a(&t, 0xE1);
                    wr8((uint16_t)(PITCH_STEP + x), 0);
                }
            }
        }
        wr8((uint16_t)(PITCH_HIGH + x), t.a);
        value = rd8(ZP_EA);
        t_a(&t, value);
        t_a(&t, (uint8_t)(value & 0x7F));
        wr8((uint16_t)(PITCH_FRACTION + x), t.a);
    }
    value = rd8((uint16_t)(LEFT_STEP + x));
    t_a(&t, value);
    if (value != 0) {
        if ((value & 0x80) == 0) {
            t_clc(&t);
            t_adc(&t, rd8((uint16_t)(LEFT_FRACTION + x)));
            wr8(ZP_EA, t.a);
            roll_carry(&t);
            wr8(ZP_EB, t.a);
            value = rd8((uint16_t)(BALANCE + x));
            t_a(&t, (uint8_t)(value & 0x0F));
            t_clc(&t);
            t_adc(&t, rd8(ZP_EB));
            t_cmp(&t, t.a, 0x10);
            if (t.a >= 0x10) {
                t_a(&t, 0x0F);
                wr8((uint16_t)(LEFT_STEP + x), 0);
            }
        } else {
            t_a(&t, (uint8_t)(value ^ 0xFF));
            t_inc(&t);
            t_clc(&t);
            t_adc(&t, rd8((uint16_t)(LEFT_FRACTION + x)));
            wr8(ZP_EA, t.a);
            roll_carry(&t);
            wr8(ZP_EB, t.a);
            value = rd8((uint16_t)(BALANCE + x));
            t_a(&t, (uint8_t)(value & 0x0F));
            t_sec(&t);
            t_sbc(&t, rd8(ZP_EB));
            if (t.z) {
                t.a = 0;
                t.a_set = true;
                wr8((uint16_t)(LEFT_STEP + x), 0);
            }
        }
        wr8(ZP_EB, t.a);
        value = rd8((uint16_t)(BALANCE + x));
        t_a(&t, (uint8_t)(value & 0xF0));
        t_a(&t, (uint8_t)(t.a | rd8(ZP_EB)));
        wr8((uint16_t)(BALANCE + x), t.a);
        value = rd8(ZP_EA);
        t_a(&t, value);
        t_a(&t, (uint8_t)(value & 0x7F));
        wr8((uint16_t)(LEFT_FRACTION + x), t.a);
    }
    value = rd8((uint16_t)(RIGHT_STEP + x));
    t_a(&t, value);
    if (value == 0)
        return t_exit(&t);
    if ((value & 0x80) == 0) {
        t_clc(&t);
        t_adc(&t, rd8((uint16_t)(RIGHT_FRACTION + x)));
        wr8(ZP_EA, t.a);
        t_a(&t, (uint8_t)(t.a & 0x80));
        t_lsr(&t);
        t_lsr(&t);
        t_lsr(&t);
        wr8(ZP_EB, t.a);
        value = rd8((uint16_t)(BALANCE + x));
        t_a(&t, (uint8_t)(value & 0xF0));
        t_clc(&t);
        t_adc(&t, rd8(ZP_EB));
        if (t.c) {
            t_a(&t, 0xF0);
            wr8((uint16_t)(RIGHT_STEP + x), 0);
        }
    } else {
        t_a(&t, (uint8_t)(value ^ 0xFF));
        t_inc(&t);
        t_clc(&t);
        t_adc(&t, rd8((uint16_t)(RIGHT_FRACTION + x)));
        wr8(ZP_EA, t.a);
        t_a(&t, (uint8_t)(t.a & 0x80));
        t_lsr(&t);
        t_lsr(&t);
        t_lsr(&t);
        wr8(ZP_EB, t.a);
        value = rd8((uint16_t)(BALANCE + x));
        t_a(&t, (uint8_t)(value & 0xF0));
        t_sec(&t);
        t_sbc(&t, rd8(ZP_EB));
        if (t.z) {
            t.a = 0;
            t.a_set = true;
            wr8((uint16_t)(RIGHT_STEP + x), 0);
        }
    }
    wr8(ZP_EB, t.a);
    value = rd8((uint16_t)(BALANCE + x));
    t_a(&t, (uint8_t)(value & 0x0F));
    t_a(&t, (uint8_t)(t.a | rd8(ZP_EB)));
    wr8((uint16_t)(BALANCE + x), t.a);
    value = rd8(ZP_EA);
    t_a(&t, value);
    t_a(&t, (uint8_t)(value & 0x7F));
    wr8((uint16_t)(RIGHT_FRACTION + x), t.a);
    return t_exit(&t);
}

void abi_psg_slide_channel(void)
{
    card_exit_apply(psg_slide_channel());
}

/* INC of a zero page pair: the low byte, and the high if it wrapped; N and Z from the last one. */
static void step_pair(Trk *t, uint16_t low)
{
    uint8_t value = rd8(low);

    value = (uint8_t)(value + 1);
    wr8(low, value);
    t_nz(t, value);
    if (value == 0) {
        value = rd8((uint16_t)(low + 1));
        value = (uint8_t)(value + 1);
        wr8((uint16_t)(low + 1), value);
        t_nz(t, value);
    }
}

/* The pointer in $E8 and $E9 kept in the channel's arrays, and the end of the routine. */
static CardExit keep_sequence_pointer(Trk *t, uint8_t y)
{
    uint8_t value = rd8(ZP_E8);

    t_a(t, value);
    wr8((uint16_t)(SEQ_POINTER_LOW + y), value);
    value = rd8(ZP_E9);
    t_a(t, value);
    wr8((uint16_t)(SEQ_POINTER_HIGH + y), value);
    return t_exit(t);
}

/*
 * @card 02:D350 psg_read_notes
 * WHAT IT DOES: Reads the current channel's note stream when the channel is in the state that wants it: a note's pitch, the sound's
 * shape (its waveform pair), the volume or the channel flags, until a note or the stream's end.
 * HOW: Only for a channel whose kind ($25E1, high nibble) is 1 and whose state ($260D) is not zero. Each byte of the stream at the
 * pointer kept in $259F and $25A5: below $B0 a noise value (channels 4 and 5); $B0 to $BF the high pitch and the next byte the low, and
 * the note is read; $C0 a shape number, built-in or the user's, $D0 the balance, $E0 the flags; anything else ends the stream. A
 * shape, balance or flags byte goes on to the next byte.
 * WAITS: no.
 * Entry: CardPlayChannel ($22E0) the channel. Exit: A, X, Y and the flags as the last step left them.
 */
CardExit psg_read_notes(void)
{
    Trk t = { 0 };
    uint8_t y = rd8(CHANNEL_NOW);
    uint8_t value;
    uint8_t note;

    t.y = y;
    t.y_set = true;
    t_nz(&t, y);
    value = rd8((uint16_t)(SEQ_KIND + y));
    t_a(&t, value);
    t_a(&t, (uint8_t)(value & 0xF0));
    t_cmp(&t, t.a, 0x10);
    if (t.a != 0x10)
        return t_exit(&t);
    value = rd8((uint16_t)(SEQ_STATE + y));
    t_a(&t, value);
    if (value == 0)
        return t_exit(&t);
    value = rd8((uint16_t)(SEQ_POINTER_LOW + y));
    t_a(&t, value);
    wr8(ZP_E8, value);
    value = rd8((uint16_t)(SEQ_POINTER_HIGH + y));
    t_a(&t, value);
    wr8(ZP_E9, value);
    for (;;) {
        note = rd8_zp_y(ZP_E8, 0);
        t_a(&t, note);
        step_pair(&t, ZP_E8);
        t.x = note;
        t.x_set = true;
        t_a(&t, note);
        t_cmp(&t, note, 0xB0);
        if (note < 0xB0) {
            t_cmp(&t, y, 4);
            if (y >= 4) {
                t_a(&t, (uint8_t)(note & 0x1F));
                wr8((uint16_t)(NOISE_AT + y), t.a);
            }
            return keep_sequence_pointer(&t, y);
        }
        t_a(&t, (uint8_t)(note & 0xF0));
        t_cmp(&t, t.a, 0xB0);
        if (t.a == 0xB0) {
            t_a(&t, note);
            t_sec(&t);
            t_sbc(&t, 0xB0);
            wr8((uint16_t)(FREQ_HIGH_AT + y), t.a);
            value = rd8_zp_y(ZP_E8, 0);
            t_a(&t, value);
            step_pair(&t, ZP_E8);
            wr8((uint16_t)(FREQ_LOW_AT + y), value);
            t_cmp(&t, y, 4);
            if (y >= 4) {
                t_a(&t, 0x80);
                wr8((uint16_t)(NOISE_AT + y), 0x80);
            }
            return keep_sequence_pointer(&t, y);
        }
        t_cmp(&t, t.a, 0xC0);
        if (t.a == 0xC0) {
            value = rd8_zp_y(ZP_E8, 0);
            t_a(&t, value);
            t_cmp(&t, value, 0x10);
            if (value < 0x10) {
                t_asl(&t);
                t.x = t.a;
                t_xy_nz(&t, t.a);
                value = rd8((uint16_t)(SHAPES_BUILT_IN + t.x));
                t_a(&t, value);
                wr8((uint16_t)(SHAPE_LOW + y), value);
                value = rd8((uint16_t)(SHAPES_BUILT_IN + 1 + t.x));
                t_a(&t, value);
                wr8((uint16_t)(SHAPE_HIGH + y), value);
            } else {
                t_sec(&t);
                t_sbc(&t, 0x10);
                t_asl(&t);
                t_clc(&t);
                t_adc(&t, rd8(SHAPES_USER));
                wr8(ZP_EA, t.a);
                t.a = 0;
                t.a_set = true;
                t_adc(&t, rd8((uint16_t)(SHAPES_USER + 1)));
                wr8(ZP_EB, t.a);
                value = rd8_zp_y(ZP_EA, 0);
                t_a(&t, value);
                wr8((uint16_t)(SHAPE_LOW + y), value);
                step_pair(&t, ZP_EA);
                value = rd8_zp_y(ZP_EA, 0);
                t_a(&t, value);
                wr8((uint16_t)(SHAPE_HIGH + y), value);
            }
            step_pair(&t, ZP_E8);
            continue;
        }
        t_cmp(&t, t.a, 0xD0);
        if (t.a == 0xD0) {
            value = rd8_zp_y(ZP_E8, 0);
            t_a(&t, value);
            wr8((uint16_t)(BALANCE_AT + y), value);
            step_pair(&t, ZP_E8);
            continue;
        }
        t_cmp(&t, t.a, 0xE0);
        if (t.a == 0xE0) {
            value = rd8_zp_y(ZP_E8, 0);
            t_a(&t, value);
            wr8((uint16_t)(FLAGS_AT + y), value);
            step_pair(&t, ZP_E8);
            continue;
        }
        t.a = 0;
        t.a_set = true;
        wr8((uint16_t)(SEQ_STATE + y), 0);
        value = rd8(ZP_E8);
        t_a(&t, value);
        t_sec(&t);
        t_sbc(&t, 1);
        wr8((uint16_t)(SEQ_POINTER_LOW + y), t.a);
        value = rd8(ZP_E9);
        t_a(&t, value);
        t_sbc(&t, 0);
        wr8((uint16_t)(SEQ_POINTER_HIGH + y), t.a);
        return t_exit(&t);
    }
}

void abi_psg_read_notes(void)
{
    card_exit_apply(psg_read_notes());
}
