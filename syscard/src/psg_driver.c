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
#include "card_exit.h"
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
    SHAPES_BUILT_IN = 0xD8A1, SHAPES_USER = 0x22D5, ENV_KIND = 0x25E1, FREQ_NOW_LOW = 0x25E7, FREQ_NOW_HIGH = 0x25ED, FREQ_BASE_LOW = 0x249D,
    FREQ_BASE_HIGH = 0x24A3, VIB_STEP = 0x2581, VIB_ON = 0x2593, VIB_COUNT = 0x2599, VIB_LOW = 0x2587, VIB_HIGH = 0x258D, DETUNE = 0x2557,
    SLIDE_ON = 0x254B, SLIDE_COUNT = 0x2551, SLIDE_LOW = 0x2539, SLIDE_HIGH = 0x253F, SLIDE_AT = 0x2545, PORT_ON = 0x2527, PORT_COUNT = 0x252D,
    PORT_LOW = 0x2515, PORT_HIGH = 0x251B, PORT_AT = 0x2521, SCALE = 0x2533, SCALE_BASE = 0x2497, ENV_STATE = 0x260D, ENV_TIMER = 0x24BB,
    ENV_LEFT = 0x250F, ENV_LEVEL_LOW = 0x24EB, ENV_LEVEL_HIGH = 0x24F1, ENV_RATE_LOW = 0x24F7, ENV_RATE_HIGH = 0x24FD, ENV_RELEASE_LOW = 0x2503,
    ENV_RELEASE_HIGH = 0x2509, ENV_AT = 0x24E5, VOLUME_BASE = 0x24AF, VOLUME_OUT = 0x25F3, ENV_LEVEL_MAX = 0x7C, SONG_NEXT = 0x22E1, SONG_LIST = 0x22D0,
    SONG_FLAGS = 0x22DE, SONG_CHANNEL_FLAGS = 0x22E2, NOTE_COUNT = 0x24A9, CHANNEL_DEFAULT_SHAPE = 0xD8A1, CHANNEL_START_LOW = 0x248B,
    CHANNEL_START_HIGH = 0x2491, CHANNEL_LOOP_LOW = 0x24C1, CHANNEL_LOOP_HIGH = 0x24C7, SCALE_BASE_AT = 0x2497, ENV_RELEASE_HIGH_AT = 0x2509
};

/* An array of the second player (mode 0) lies lower than the first's: those from $2480 to $25A5 by $19D, those from $25E1 to $2615 by $18B. */
static inline uint16_t ar(uint8_t set, uint16_t address)
{
    if (set == 0)
        return address;
    return address >= 0x25E1 ? (uint16_t)(address - 0x18B) : (uint16_t)(address - 0x19D);
}

static CardExit tempo_tick_body(uint8_t set)
{
    uint8_t step = rd8(ar(set, TEMPO_ON));
    uint8_t phase;
    uint8_t ticks;
    uint8_t state = 0;
    uint8_t a;
    Sum sum;
    CardExit e = { 0 };

    if (step == 0)
        return exit_a(0);
    phase = rd8(ar(set, TEMPO_PHASE));
    sum = add8(step, phase, false);
    a = sum.value;
    if ((a & 0x80) != 0) {
        ticks = rd8(ar(set, TEMPO_TICKS));
        wr8(ar(set, TEMPO_TICKS), (uint8_t)(ticks + 1));
        a = (uint8_t)(a & 0x7F);
    }
    wr8(ar(set, TEMPO_PHASE), a);
    ticks = rd8(ar(set, TEMPO_TICKS));
    if (ticks < TEMPO_LIMIT)
        return with_v(exit_cmp(ticks, TEMPO_LIMIT), sum.overflow);
    for (int x = CHANNELS - 1; x >= 0; x--) {
        state = rd8((uint16_t)(ar(set, CHANNEL_STATE) + x));
        if (state != 0) {
            wr8((uint16_t)(ar(set, CHANNEL_STATE) + x), STATE_STOP_ASKED);
            state = STATE_STOP_ASKED;
        }
    }
    e.a = state;
    e.x = 0xFF;
    e.nz = 0xFF;
    e.c = true;
    e.v = sum.overflow;
    e.set_a = e.set_x = e.set_nz = e.set_c = e.set_v = true;
    return e;
}


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
    return tempo_tick_body(0);
}

void abi_psg_tempo_tick(void)
{
    card_exit_apply(psg_tempo_tick());
}

/*
 * @card 02:CC26 psg_tempo_tick_alt
 * WHAT IT DOES: The second player's psg_tempo_tick (mode 0): counts the sound's fade-out time: while the fade is on, adds the step to a phase and counts its overflows,
 * and when the count reaches $1E asks every channel that is playing to stop.
 * HOW: $2488 the fade's step (zero: off), added to $2489 in seven bits; a carry into bit 7 is one tick of $248A; at $1E ticks every
 * channel whose state ($247C plus the channel) is not zero is set to 2.
 * WAITS: no.
 * Entry: nothing. Exit: A, X and the flags as the last step left them: off, A 0, Z; below the limit, A the ticks, C 0; else X $FF,
 * N, C 1.
 */
CardExit psg_tempo_tick_alt(void)
{
    return tempo_tick_body(1);
}

void abi_psg_tempo_tick_alt(void)
{
    card_exit_apply(psg_tempo_tick_alt());
}

static CardExit wave_offset_body(uint8_t number)
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


/*
 * @card 02:D706 psg_wave_offset
 * WHAT IT DOES: Makes a waveform's number into its offset in the table: the number times 32, in _EX and _EY zero page.
 * HOW: Three shifts through zero page $E8.
 * WAITS: no.
 * Entry: A the number. Exit: $E8 the low byte and $E9 the high; A the high byte, C the last bit shifted out. N, Z from the last ROR; X, Y kept.
 */
CardExit psg_wave_offset(uint8_t number)
{
    return wave_offset_body(number);
}

void abi_psg_wave_offset(void)
{
    card_exit_apply(psg_wave_offset(cpu.a));
}

/*
 * @card 02:CD04 psg_wave_offset_alt
 * WHAT IT DOES: The second player's psg_wave_offset: Makes a waveform's number into its offset in the table: the number times 32, in _EX and _EY zero page.
 * HOW: Three shifts through zero page $E8.
 * WAITS: no.
 * Entry: A the number. Exit: $E8 the low byte and $E9 the high; A the high byte, C the last bit shifted out. N, Z from the last ROR; X, Y kept.
 */
CardExit psg_wave_offset_alt(uint8_t number)
{
    return wave_offset_body(number);
}

void abi_psg_wave_offset_alt(void)
{
    card_exit_apply(psg_wave_offset_alt(cpu.a));
}

/* The table's address plus the offset psg_wave_offset left in $E8 and $E9, as the two adds make it: the second add's sum. */
static Sum wave_address(uint8_t low_base, uint8_t high_base, bool from_memory, uint16_t memory)
{
    Sum low;
    Sum high;
    uint8_t value;

    if (from_memory)
        low_base = rd8(memory);
    value = rd8(ZP_E8);
    low = add8(low_base, value, false);
    wr8(ZP_E8, low.value);
    if (from_memory)
        high_base = rd8((uint16_t)(memory + 1));
    value = rd8(ZP_E9);
    high = add8(high_base, value, low.carry);
    wr8(ZP_E9, high.value);
    return high;
}

/* One channel of psg_write_channels: what its path leaves in A, C, V and Y (those it touches). */
static CardExit write_channel(uint8_t set, uint8_t channel)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t state;
    uint8_t a;
    bool noise = false;
    Sum sum;

    state = rd8((uint16_t)(ar(set, CHANNEL_STATE) + channel));
    e = exit_cmp(state, 1);
    if (((uint8_t)(state - 1) & 0x80) != 0)
        return e;
    wr8(PSG_SELECT, channel);
    if (state != 1) {
        wr8(PSG_CONTROL, 0);
        wr8((uint16_t)(ar(set, CHANNEL_STATE) + channel), 0);
        if (set != 0)
            return e;
        value = rd8(PLAYER_MODE_AT);
        e = exit_cmp(value, 1);
        if (value == 1)
            return e;
        value = rd8((uint16_t)(ar(set, CHANNEL_USED) + channel));
        e.a = value;
        if (value == 0)
            return e;
        value = rd8((uint16_t)(ar(set, CHANNEL_BUSY) + channel));
        e.a = (uint8_t)(value & 0x7F);
        wr8((uint16_t)(ar(set, CHANNEL_BUSY) + channel), e.a);
        return e;
    }
    if (set != 0 && rd8(PLAYER_MODE_AT) != 0) {
        value = rd8((uint16_t)(CHANNEL_STATE + channel));
        e = exit_cmp(value, 1);
        if (value == 1)
            return e;
    }
    value = rd8((uint16_t)(ar(set, CHANNEL_FLAGS) + channel));
    a = value;
    if ((value & 0x80) == 0) {
        uint8_t number;
        CardExit step;

        a = (uint8_t)(value | 0x80);
        wr8((uint16_t)(ar(set, CHANNEL_FLAGS) + channel), a);
        number = (uint8_t)(a & 0x7F);
        if (number < WAVE_BUILT_IN_COUNT) {
            jsr_push(set ? 0xCC8C : 0xD68E);
            step = set ? psg_wave_offset_alt(number) : psg_wave_offset(number);
            jsr_pop();
            (void)step;
            sum = wave_address(0xA6, 0xD9, false, 0);
        } else {
            jsr_push(set ? 0xCCA1 : 0xD6A3);
            step = set ? psg_wave_offset_alt((uint8_t)(number - WAVE_BUILT_IN_COUNT)) : psg_wave_offset((uint8_t)(number - WAVE_BUILT_IN_COUNT));
            jsr_pop();
            (void)step;
            sum = wave_address(0, 0, true, WAVE_TABLE_USER);
        }
        e = with_v(with_c(exit_a(sum.value), sum.carry), sum.overflow);
        wr8(PSG_CONTROL, 0x40);
        wr8(PSG_CONTROL, 0);
        for (uint8_t y = 0; y < WAVE_BYTES; y++) {
            a = rd8_zp_y(ZP_E8, y);
            wr8(PSG_WAVE, a);
        }
        e = with_y(with_c(e, true), WAVE_BYTES);
        e.a = a;
    }
    if (channel >= 4) {
        value = rd8((uint16_t)(ar(set, NOISE_FLAGS) + channel));
        a = value;
        if ((value & 0x80) == 0) {
            a = (uint8_t)(value | 0x80);
            wr8(PSG_NOISE, a);
            noise = true;
        }
    }
    if (!noise) {
        wr8(PSG_NOISE, 0);
        a = rd8((uint16_t)(ar(set, FREQ_LOW) + channel));
        wr8(PSG_FREQ_LOW, a);
        a = rd8((uint16_t)(ar(set, FREQ_HIGH) + channel));
        wr8(PSG_FREQ_HIGH, a);
    }
    a = rd8((uint16_t)(ar(set, BALANCE) + channel));
    wr8(PSG_BALANCE, a);
    value = rd8((uint16_t)(ar(set, VOLUME) + channel));
    a = rd8(ar(set, TEMPO_TICKS));
    sum = sub8(value, a, true);
    if (sum.value == 0 || (sum.value & 0x80) != 0) {
        wr8(PSG_CONTROL, 0);
        a = sum.value;
    } else {
        a = (uint8_t)(sum.value | 0x80);
        wr8(PSG_CONTROL, a);
    }
    e.a = a;
    e.set_a = true;
    return with_v(with_c(e, sum.carry), sum.overflow);
}

static CardExit write_channels_body(uint8_t set)
{
    CardExit e = { 0 };

    for (int x = CHANNELS - 1; x >= 0; x--)
        e = card_exit_over(e, write_channel(set, (uint8_t)x));
    e.x = 0xFF;
    e.nz = 0xFF;
    e.set_x = e.set_nz = true;
    e.set_n = e.set_z = false;
    return e;
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
    return write_channels_body(0);
}

void abi_psg_write_channels(void)
{
    card_exit_apply(psg_write_channels());
}

/*
 * @card 02:CC50 psg_write_channels_alt
 * WHAT IT DOES: The second player's psg_write_channels (mode 0): Writes the six channels' state to the PSG, from channel 5 down to 0: a channel asked to stop is silenced and forgotten; a
 * new one gets its waveform, noise or frequency, balance and volume.
 * HOW: Each channel's state at $247C: below 1 skipped; 2 and up: key off, state cleared, and the channel's busy bit cleared in $2474
 * unless the player's mode is 1 or the channel is not used; 1: a new note: the waveform ($2474 bit 7 not yet set) copied as 32 bytes
 * from the built-in table at $D9A6 or the user's at $22D2, then noise (channels 4 and 5, if bit 7 of $2476 is clear) or frequency,
 * the balance, and the volume less the fade ($2615), written with the key on, or the key off if none is left.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N from it; A and the other flags as the last channel left them.
 * The PSG's registers at $0800 to $0807 and the arrays are by address; psg_wave_offset's frames are at $D68E and $D6A3.
 */
CardExit psg_write_channels_alt(void)
{
    return write_channels_body(1);
}

void abi_psg_write_channels_alt(void)
{
    card_exit_apply(psg_write_channels_alt());
}

/* The sum's top bit, which two ROLs and an AND of 1 bring round: the fraction's carry into the whole part. */
static uint8_t fraction_carry(uint8_t sum)
{
    return (uint8_t)(sum >> 7);
}

/* The pitch slide of psg_slide_channel: what it leaves in A, C and V. */
static CardExit slide_pitch(uint8_t x)
{
    uint8_t step = rd8((uint16_t)(PITCH_STEP + x));
    uint8_t high;
    uint8_t fraction;
    uint8_t carry;
    uint8_t keep;
    Sum sum;
    bool c;

    if (step == 0)
        return exit_a(0);
    if ((step & 0x80) == 0) {
        fraction = rd8((uint16_t)(PITCH_FRACTION + x));
        sum = add8(step, fraction, false);
        wr8(ZP_EA, sum.value);
        carry = fraction_carry(sum.value);
        high = rd8((uint16_t)(PITCH_HIGH + x));
        sum = add8(carry, high, false);
        c = sum.carry;
        keep = sum.value;
        if (sum.value != 0 && (sum.value & 0x80) == 0) {
            keep = 0;
            wr8((uint16_t)(PITCH_STEP + x), 0);
        }
    } else {
        fraction = rd8((uint16_t)(PITCH_FRACTION + x));
        sum = add8((uint8_t)((step ^ 0xFF) + 1), fraction, false);
        wr8(ZP_EA, sum.value);
        carry = fraction_carry(sum.value);
        wr8(ZP_EB, carry);
        high = rd8((uint16_t)(PITCH_HIGH + x));
        sum = sub8(high, rd8(ZP_EB), true);
        c = sum.carry;
        keep = sum.value;
        if (sum.value != 0) {
            c = sum.value >= 0xE1;
            if (sum.value < 0xE1) {
                keep = 0xE1;
                wr8((uint16_t)(PITCH_STEP + x), 0);
            }
        }
    }
    wr8((uint16_t)(PITCH_HIGH + x), keep);
    keep = rd8(ZP_EA);
    wr8((uint16_t)(PITCH_FRACTION + x), (uint8_t)(keep & 0x7F));
    return with_v(with_c(exit_a((uint8_t)(keep & 0x7F)), c), sum.overflow);
}

/* The left volume slide: what it leaves in A, C and V (nothing but A if there is none). */
static CardExit slide_left(uint8_t x)
{
    uint8_t step = rd8((uint16_t)(LEFT_STEP + x));
    uint8_t value;
    uint8_t carry;
    uint8_t level;
    Sum sum;
    bool c;

    if (step == 0)
        return exit_a(0);
    if ((step & 0x80) == 0) {
        value = rd8((uint16_t)(LEFT_FRACTION + x));
        sum = add8(step, value, false);
        wr8(ZP_EA, sum.value);
        carry = fraction_carry(sum.value);
        wr8(ZP_EB, carry);
        value = rd8((uint16_t)(BALANCE + x));
        sum = add8((uint8_t)(value & 0x0F), rd8(ZP_EB), false);
        c = sum.value >= 0x10;
        level = sum.value;
        if (sum.value >= 0x10) {
            level = 0x0F;
            wr8((uint16_t)(LEFT_STEP + x), 0);
        }
    } else {
        value = rd8((uint16_t)(LEFT_FRACTION + x));
        sum = add8((uint8_t)((step ^ 0xFF) + 1), value, false);
        wr8(ZP_EA, sum.value);
        carry = fraction_carry(sum.value);
        wr8(ZP_EB, carry);
        value = rd8((uint16_t)(BALANCE + x));
        sum = sub8((uint8_t)(value & 0x0F), rd8(ZP_EB), true);
        c = sum.carry;
        level = sum.value;
        if (sum.value == 0)
            wr8((uint16_t)(LEFT_STEP + x), 0);
    }
    wr8(ZP_EB, level);
    value = rd8((uint16_t)(BALANCE + x));
    level = (uint8_t)((value & 0xF0) | rd8(ZP_EB));
    wr8((uint16_t)(BALANCE + x), level);
    value = rd8(ZP_EA);
    wr8((uint16_t)(LEFT_FRACTION + x), (uint8_t)(value & 0x7F));
    return with_v(with_c(exit_a((uint8_t)(value & 0x7F)), c), sum.overflow);
}

/* The right volume slide: what it leaves in A, C and V (A only if there is none). */
static CardExit slide_right(uint8_t x)
{
    uint8_t step = rd8((uint16_t)(RIGHT_STEP + x));
    uint8_t value;
    uint8_t level;
    Sum sum;
    Sum fraction;

    if (step == 0)
        return exit_a(0);
    value = rd8((uint16_t)(RIGHT_FRACTION + x));
    if ((step & 0x80) == 0) {
        fraction = add8(step, value, false);
        wr8(ZP_EA, fraction.value);
        wr8(ZP_EB, (uint8_t)((fraction.value & 0x80) >> 3));
        value = rd8((uint16_t)(BALANCE + x));
        sum = add8((uint8_t)(value & 0xF0), rd8(ZP_EB), false);
        level = sum.value;
        if (sum.carry) {
            level = 0xF0;
            wr8((uint16_t)(RIGHT_STEP + x), 0);
        }
    } else {
        fraction = add8((uint8_t)((step ^ 0xFF) + 1), value, false);
        wr8(ZP_EA, fraction.value);
        wr8(ZP_EB, (uint8_t)((fraction.value & 0x80) >> 3));
        value = rd8((uint16_t)(BALANCE + x));
        sum = sub8((uint8_t)(value & 0xF0), rd8(ZP_EB), true);
        level = sum.value;
        if (sum.value == 0)
            wr8((uint16_t)(RIGHT_STEP + x), 0);
    }
    wr8(ZP_EB, level);
    value = rd8((uint16_t)(BALANCE + x));
    level = (uint8_t)((value & 0x0F) | rd8(ZP_EB));
    wr8((uint16_t)(BALANCE + x), level);
    value = rd8(ZP_EA);
    wr8((uint16_t)(RIGHT_FRACTION + x), (uint8_t)(value & 0x7F));
    return with_v(with_c(exit_a((uint8_t)(value & 0x7F)), sum.carry), sum.overflow);
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
    uint8_t x = rd8(CHANNEL_NOW);
    CardExit e = exit_x(x);

    e = card_exit_over(e, slide_pitch(x));
    e = card_exit_over(e, slide_left(x));
    return card_exit_over(e, slide_right(x));
}

void abi_psg_slide_channel(void)
{
    card_exit_apply(psg_slide_channel());
}

/* INC of a zero page pair: the low byte, and the high if it wrapped. */
static void step_pair(uint16_t low)
{
    uint8_t value = rd8(low);

    value = (uint8_t)(value + 1);
    wr8(low, value);
    if (value == 0) {
        value = rd8((uint16_t)(low + 1));
        wr8((uint16_t)(low + 1), (uint8_t)(value + 1));
    }
}

/* The pointer in $E8 and $E9 kept in the channel's arrays: A the high byte, with N and Z from it, is what the stream leaves. */
static uint8_t keep_sequence_pointer(uint8_t y)
{
    uint8_t value = rd8(ZP_E8);

    wr8((uint16_t)(SEQ_POINTER_LOW + y), value);
    value = rd8(ZP_E9);
    wr8((uint16_t)(SEQ_POINTER_HIGH + y), value);
    return value;
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
    CardExit e;
    CardExit seen = { 0 };
    uint8_t y = rd8(CHANNEL_NOW);
    uint8_t value;
    uint8_t note;
    uint8_t kind;
    Sum sum;

    kind = (uint8_t)(rd8((uint16_t)(SEQ_KIND + y)) & 0xF0);
    if (kind != 0x10)
        return with_y(exit_cmp(kind, 0x10), y);
    value = rd8((uint16_t)(SEQ_STATE + y));
    if (value == 0)
        return with_y(with_c(exit_a(0), true), y);
    value = rd8((uint16_t)(SEQ_POINTER_LOW + y));
    wr8(ZP_E8, value);
    value = rd8((uint16_t)(SEQ_POINTER_HIGH + y));
    wr8(ZP_E9, value);
    for (;;) {
        note = rd8_zp_y(ZP_E8, 0);
        step_pair(ZP_E8);
        if (note < 0xB0) {
            if (y >= 4)
                wr8((uint16_t)(NOISE_AT + y), (uint8_t)(note & 0x1F));
            value = keep_sequence_pointer(y);
            e = with_y(with_x(with_c(exit_a(value), y >= 4), note), y);
            return card_exit_over(seen, e);
        }
        kind = (uint8_t)(note & 0xF0);
        if (kind == 0xB0) {
            sum = sub8(note, 0xB0, true);
            wr8((uint16_t)(FREQ_HIGH_AT + y), sum.value);
            value = rd8_zp_y(ZP_E8, 0);
            step_pair(ZP_E8);
            wr8((uint16_t)(FREQ_LOW_AT + y), value);
            if (y >= 4)
                wr8((uint16_t)(NOISE_AT + y), 0x80);
            value = keep_sequence_pointer(y);
            e = with_v(with_y(with_x(with_c(exit_a(value), y >= 4), note), y), sum.overflow);
            return e;
        }
        if (kind == 0xC0) {
            value = rd8_zp_y(ZP_E8, 0);
            if (value < 0x10) {
                uint8_t index = (uint8_t)(value << 1);

                wr8((uint16_t)(SHAPE_LOW + y), rd8((uint16_t)(SHAPES_BUILT_IN + index)));
                wr8((uint16_t)(SHAPE_HIGH + y), rd8((uint16_t)(SHAPES_BUILT_IN + 1 + index)));
            } else {
                Sum high;
                uint8_t doubled = (uint8_t)((uint8_t)(value - 0x10) << 1);

                sum = add8(doubled, rd8(SHAPES_USER), false);
                wr8(ZP_EA, sum.value);
                high = add8(0, rd8((uint16_t)(SHAPES_USER + 1)), sum.carry);
                wr8(ZP_EB, high.value);
                seen = with_v(seen, high.overflow);
                value = rd8_zp_y(ZP_EA, 0);
                wr8((uint16_t)(SHAPE_LOW + y), value);
                step_pair(ZP_EA);
                value = rd8_zp_y(ZP_EA, 0);
                wr8((uint16_t)(SHAPE_HIGH + y), value);
            }
            step_pair(ZP_E8);
            continue;
        }
        if (kind == 0xD0) {
            value = rd8_zp_y(ZP_E8, 0);
            wr8((uint16_t)(BALANCE_AT + y), value);
            step_pair(ZP_E8);
            continue;
        }
        if (kind == 0xE0) {
            value = rd8_zp_y(ZP_E8, 0);
            wr8((uint16_t)(FLAGS_AT + y), value);
            step_pair(ZP_E8);
            continue;
        }
        wr8((uint16_t)(SEQ_STATE + y), 0);
        value = rd8(ZP_E8);
        sum = sub8(value, 1, true);
        wr8((uint16_t)(SEQ_POINTER_LOW + y), sum.value);
        value = rd8(ZP_E9);
        sum = sub8(value, 0, sum.carry);
        wr8((uint16_t)(SEQ_POINTER_HIGH + y), sum.value);
        e = with_v(with_c(with_y(with_x(exit_a(sum.value), note), y), sum.carry), sum.overflow);
        return e;
    }
}

void abi_psg_read_notes(void)
{
    card_exit_apply(psg_read_notes());
}

/*
 * @card 02:D611 psg_add_pitch
 * WHAT IT DOES: Adds the amount in $EA and $EB to the current channel's frequency.
 * HOW: A 16-bit add to $25E7 and $25ED, plus the channel.
 * WAITS: no.
 * Entry: X the channel, $EA and $EB the amount. Exit: A the high byte, C and V from its add, N and Z from it. X, Y kept.
 */
CardExit psg_add_pitch(uint8_t x)
{
    Sum low;
    Sum high;
    uint8_t value;

    value = rd8((uint16_t)(FREQ_NOW_LOW + x));
    low = add8(value, rd8(ZP_EA), false);
    wr8((uint16_t)(FREQ_NOW_LOW + x), low.value);
    value = rd8((uint16_t)(FREQ_NOW_HIGH + x));
    high = add8(value, rd8(ZP_EB), low.carry);
    wr8((uint16_t)(FREQ_NOW_HIGH + x), high.value);
    return with_v(with_c(exit_a(high.value), high.carry), high.overflow);
}

void abi_psg_add_pitch(void)
{
    card_exit_apply(psg_add_pitch(cpu.x));
}

/*
 * @card 02:D5E2 psg_scale_amount
 * WHAT IT DOES: Makes the byte in $EA a signed 16-bit amount in $EA and $EB, scaled by the channel's scale less its base.
 * HOW: $EB the sign of $EA; the channel's scale ($2533) less its base ($2497) as a shift count: left for plus, right for minus; a
 * right shift that leaves something in the high byte makes it $FF.
 * WAITS: no.
 * Entry: X the channel, $EA the byte. Exit: $EA and $EB the amount; A, Y and the flags as the last step left them. X kept.
 */
CardExit psg_scale_amount(uint8_t x)
{
    uint8_t value;
    uint8_t y;
    uint8_t low;
    uint8_t high;
    bool carry = false;
    Sum sum;
    CardExit e;

    wr8(ZP_EB, 0);
    value = rd8(ZP_EA);
    if ((value & 0x80) != 0)
        wr8(ZP_EB, (uint8_t)(rd8(ZP_EB) - 1));
    value = rd8((uint16_t)(SCALE + x));
    if (value == 0)
        return exit_a(0);
    sum = sub8(value, rd8((uint16_t)(SCALE_BASE + x)), true);
    y = sum.value;
    carry = sum.carry;
    e = with_v(exit_a(sum.value), sum.overflow);
    if ((y & 0x80) == 0) {
        while (y != 0) {
            low = rd8(ZP_EA);
            wr8(ZP_EA, (uint8_t)(low << 1));
            high = rd8(ZP_EB);
            wr8(ZP_EB, (uint8_t)((high << 1) | (low >> 7)));
            carry = (high & 0x80) != 0;
            y--;
        }
        e.nz = 0;
        e.set_nz = true;
        return with_y(with_c(e, carry), 0);
    }
    while (y != 0) {
        high = rd8(ZP_EB);
        wr8(ZP_EB, (uint8_t)(high >> 1));
        low = rd8(ZP_EA);
        wr8(ZP_EA, (uint8_t)((high << 7) | (low >> 1)));
        carry = (low & 1) != 0;
        y++;
    }
    value = rd8(ZP_EB);
    if (value != 0) {
        wr8(ZP_EB, 0xFF);
        value = 0xFF;
    }
    e = with_y(with_c(e, carry), 0);
    e.a = e.nz = value;
    return e;
}

void abi_psg_scale_amount(void)
{
    card_exit_apply(psg_scale_amount(cpu.x));
}

/* JSR psg_scale_amount and then psg_add_pitch: the channel in X; what the two leave, the second over the first. */
static CardExit scale_and_add(uint8_t x, uint16_t first_ret, uint16_t second_ret)
{
    CardExit first;
    CardExit second;

    jsr_push(first_ret);
    first = psg_scale_amount(x);
    jsr_pop();
    jsr_push(second_ret);
    second = psg_add_pitch(x);
    jsr_pop();
    return card_exit_over(first, second);
}

/* A memory byte counted down or up by one: the new value. */
static uint8_t step_byte(uint16_t at, int by)
{
    uint8_t value = rd8(at);

    value = (uint8_t)(value + by);
    wr8(at, value);
    return value;
}

/* The vibrato of psg_pitch_effects, the step in Y: what it leaves in C, V and Y (the A and flags the detune after it loads over). */
static CardExit pitch_vibrato(uint8_t x, uint8_t y)
{
    CardExit scale;
    CardExit add;
    uint8_t value;
    Sum low;
    Sum high;

    value = rd8((uint16_t)(VIB_ON + x));
    if (value != 0 && step_byte((uint16_t)(VIB_COUNT + x), -1) == 0) {
        (void)step_byte((uint16_t)(VIB_COUNT + x), 1);
        value = rd8((uint16_t)(VIB_LOW + x));
        wr8(ZP_EA, value);
        value = rd8((uint16_t)(VIB_HIGH + x));
        wr8(ZP_EB, value);
        jsr_push(0xD547);
        add = psg_add_pitch(x);
        jsr_pop();
        return add;
    }
    wr8(ZP_EA, y);
    jsr_push(0xD54E);
    scale = psg_scale_amount(x);
    jsr_pop();
    value = rd8((uint16_t)(VIB_LOW + x));
    low = add8(value, rd8(ZP_EA), false);
    wr8((uint16_t)(VIB_LOW + x), low.value);
    wr8(ZP_EA, low.value);
    value = rd8((uint16_t)(VIB_HIGH + x));
    high = add8(value, rd8(ZP_EB), low.carry);
    wr8((uint16_t)(VIB_HIGH + x), high.value);
    wr8(ZP_EB, high.value);
    jsr_push(0xD566);
    add = psg_add_pitch(x);
    jsr_pop();
    return card_exit_over(scale, add);
}

/* The pitch slide through a list of psg_pitch_effects: what it leaves in C, V and Y (the A and flags the portamento after it loads over). */
static CardExit pitch_slide(uint8_t x)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t y;
    uint8_t item;

    value = rd8((uint16_t)(SLIDE_ON + x));
    if (value == 0)
        return e;
    if (step_byte((uint16_t)(SLIDE_COUNT + x), -1) != 0)
        return e;
    (void)step_byte((uint16_t)(SLIDE_COUNT + x), 1);
    value = rd8((uint16_t)(SLIDE_LOW + x));
    wr8(ZP_E8, value);
    value = rd8((uint16_t)(SLIDE_HIGH + x));
    wr8(ZP_E9, value);
    y = rd8((uint16_t)(SLIDE_AT + x));
    e = with_y(e, y);
    item = rd8_zp_y(ZP_E8, y);
    if (item == 0x80) {
        if (y == 0)
            return with_c(e, true);
        y--;
        item = rd8_zp_y(ZP_E8, y);
    }
    wr8(ZP_EA, item);
    push8(y);
    e = card_exit_over(e, scale_and_add(x, 0xD5A0, 0xD5A3));
    y = pull8();
    y++;
    if (y == 0)
        y--;
    wr8((uint16_t)(SLIDE_AT + x), y);
    return with_y(e, y);
}

/* The portamento through a list of psg_pitch_effects: what it leaves in A, N, Z, C, V and Y. */
static CardExit pitch_portamento(uint8_t x)
{
    CardExit e;
    uint8_t value;
    uint8_t y;
    uint8_t item;

    value = rd8((uint16_t)(PORT_ON + x));
    if (value == 0)
        return exit_a(0);
    if (step_byte((uint16_t)(PORT_COUNT + x), -1) != 0) {
        e = exit_a(value);
        e.nz = rd8((uint16_t)(PORT_COUNT + x));
        return e;
    }
    (void)step_byte((uint16_t)(PORT_COUNT + x), 1);
    value = rd8((uint16_t)(PORT_LOW + x));
    wr8(ZP_E8, value);
    value = rd8((uint16_t)(PORT_HIGH + x));
    wr8(ZP_E9, value);
    y = rd8((uint16_t)(PORT_AT + x));
    for (;;) {
        item = rd8_zp_y(ZP_E8, y);
        if (item != 0x80)
            break;
        if (y == 0) {
            e = with_y(with_c(exit_a(0x80), true), 0);
            e.nz = 0;
            return e;
        }
        wr8((uint16_t)(PORT_AT + x), 0);
        y = 0;
    }
    wr8(ZP_EA, item);
    e = scale_and_add(x, 0xD5DB, 0xD5DE);
    e.nz = step_byte((uint16_t)(PORT_AT + x), 1);
    return with_y(e, e.set_y ? e.y : y);
}

/*
 * @card 02:D510 psg_pitch_effects
 * WHAT IT DOES: Works the current channel's pitch effects for the frame: the base pitch put back, then its vibrato, its detune, its pitch
 * slide through a list and its portamento through another.
 * HOW: Only for a channel of kind 0 ($25E1, high nibble). The frequency set from its base ($249D, $24A3), then: vibrato ($2581 step, $2593 on):
 * counted down in $2599, then the accumulated amount ($2587, $258D) added, else the step scaled, accumulated and added; detune ($2557):
 * scaled and added; slide ($254B on): each time $2551 reaches 0 the next byte of the list at ($2539, $253F) from $2545, $80 ends it
 * (stepping back one byte); portamento ($2527 on): the same with the list at ($2515, $251B) from $2521, $80 sending it back to its start.
 * WAITS: no.
 * Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
 */
CardExit psg_pitch_effects(void)
{
    uint8_t x = rd8(CHANNEL_NOW);
    uint8_t value;
    uint8_t kind;
    CardExit e = exit_x(x);

    kind = (uint8_t)(rd8((uint16_t)(ENV_KIND + x)) & 0xF0);
    if (kind != 0)
        return card_exit_over(e, exit_a(kind));
    value = rd8((uint16_t)(FREQ_BASE_LOW + x));
    wr8((uint16_t)(FREQ_NOW_LOW + x), value);
    value = rd8((uint16_t)(FREQ_BASE_HIGH + x));
    wr8((uint16_t)(FREQ_NOW_HIGH + x), value);
    value = rd8((uint16_t)(VIB_STEP + x));
    e = card_exit_over(e, with_y(exit_a(value), value));
    if (value != 0)
        e = card_exit_over(e, pitch_vibrato(x, value));
    value = rd8((uint16_t)(DETUNE + x));
    e = card_exit_over(e, exit_a(value));
    if (value != 0) {
        wr8(ZP_EA, value);
        e = card_exit_over(e, scale_and_add(x, 0xD570, 0xD573));
    }
    e = card_exit_over(e, pitch_slide(x));
    return card_exit_over(e, pitch_portamento(x));
}

void abi_psg_pitch_effects(void)
{
    card_exit_apply(psg_pitch_effects());
}

/* The end the volume envelope's paths join at, $D4A3: the level made the channel's volume. What it leaves: A the volume, N and Z from the sum. */
static CardExit env_volume(uint8_t x)
{
    CardExit e = { 0 };
    uint8_t level = rd8((uint16_t)(ENV_LEVEL_HIGH + x));
    uint8_t shifted = (uint8_t)(level >> 2);
    Sum sum = add8(shifted, rd8((uint16_t)(VOLUME_BASE + x)), false);
    uint8_t volume = (sum.value & 0x80) != 0 ? 0 : sum.value;

    wr8((uint16_t)(VOLUME_OUT + x), volume);
    e.a = volume;
    e.nz = sum.value;
    e.c = sum.carry;
    e.v = sum.overflow;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

/* From $D4CE: a segment's length and its two rates read from the list at $E8 from Y, the position kept; gives Y after them. */
static uint8_t env_segment(uint8_t x, uint8_t y, uint8_t length)
{
    uint8_t value;

    if (length == 0)
        length = 0xFF;
    wr8((uint16_t)(ENV_LEFT + x), length);
    y++;
    value = rd8_zp_y(ZP_E8, y);
    wr8((uint16_t)(ENV_RATE_LOW + x), value);
    y++;
    value = rd8_zp_y(ZP_E8, y);
    wr8((uint16_t)(ENV_RATE_HIGH + x), value);
    y++;
    wr8((uint16_t)(ENV_AT + x), y);
    return y;
}

/* From $D4B3: the next segment of the envelope's list; gives Y as it ends. */
static uint8_t env_next(uint8_t x)
{
    uint8_t y = rd8((uint16_t)(ENV_AT + x));
    uint8_t value;
    uint8_t item;

    value = rd8((uint16_t)(SHAPE_LOW + x));
    wr8(ZP_E8, value);
    value = rd8((uint16_t)(SHAPE_HIGH + x));
    wr8(ZP_E9, value);
    for (;;) {
        item = rd8_zp_y(ZP_E8, y);
        if (item < 0xFB)
            return env_segment(x, y, item);
        if (item == 0xFB) {
            y = (uint8_t)(y + 3);
            continue;
        }
        if (item != 0xFC) {
            wr8((uint16_t)(ENV_LEFT + x), 0xFF);
            wr8((uint16_t)(ENV_RATE_LOW + x), 0);
            wr8((uint16_t)(ENV_RATE_HIGH + x), 0);
            return y;
        }
        y++;
        value = rd8_zp_y(ZP_E8, y);
        wr8((uint16_t)(ENV_LEVEL_LOW + x), value);
        y++;
        value = rd8_zp_y(ZP_E8, y);
        wr8((uint16_t)(ENV_LEVEL_HIGH + x), value);
        y++;
        item = rd8_zp_y(ZP_E8, y);
        if (item < 0xFB)
            return env_segment(x, y, item);
        wr8((uint16_t)(ENV_LEFT + x), 0);
        wr8((uint16_t)(ENV_AT + x), y);
        return y;
    }
}

/* ADC of the level by a rate, clipped at 0 or at $7C: $D444 for the held part and $D470 for the release. Gives the Y the clip loads, or -1. */
static int env_move_level(uint8_t x, uint16_t rate_low, uint16_t rate_high)
{
    Sum low;
    Sum high;
    uint8_t level;
    uint8_t rate;
    int y = -1;

    level = rd8((uint16_t)(ENV_LEVEL_LOW + x));
    low = add8(level, rd8((uint16_t)(rate_low + x)), false);
    wr8((uint16_t)(ENV_LEVEL_LOW + x), low.value);
    level = rd8((uint16_t)(ENV_LEVEL_HIGH + x));
    high = add8(level, rd8((uint16_t)(rate_high + x)), low.carry);
    level = high.value;
    if ((high.value & 0x80) != 0) {
        level = 0;
        wr8((uint16_t)(ENV_LEVEL_LOW + x), 0);
        rate = rd8((uint16_t)(rate_high + x));
        y = rate;
        if ((rate & 0x80) == 0)
            level = ENV_LEVEL_MAX;
    }
    wr8((uint16_t)(ENV_LEVEL_HIGH + x), level);
    return y;
}

/*
 * @card 02:D422 psg_volume_envelope
 * WHAT IT DOES: Works the current channel's volume envelope for the frame: the held part moves the level by its rates until its length is
 * used up, the release moves it down by the release rates, and the next segment is read from the shape's list when the held part ends.
 * HOW: State $260D: 0 off (volume 0); bit 7 the release; 2 ending (state counted down, position cleared, next segment); else the held part:
 * $24BB counted down and the release bit set at 0, $250F the length left (0 reads the next segment), the level ($24EB, $24F1) moved by
 * the rates ($24F7, $24FD). The list at ($24D9, $24DF) from $24E5: $FB skips three bytes, $FC sets the level then reads a segment (length, two
 * rates), a length below $FB is a segment ($FF if 0), anything else holds. The volume is the level over 4 plus the channel's base ($24AF).
 * WAITS: no.
 * Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
 */
CardExit psg_volume_envelope(void)
{
    uint8_t x = rd8(CHANNEL_NOW);
    uint8_t state;
    uint8_t value;
    int y = -1;
    CardExit e = exit_x(x);

    state = rd8((uint16_t)(ENV_STATE + x));
    if (state == 0) {
        wr8((uint16_t)(VOLUME_OUT + x), 0);
        return card_exit_over(e, exit_a(0));
    }
    if ((state & 0x80) != 0) {
        y = env_move_level(x, ENV_RELEASE_LOW, ENV_RELEASE_HIGH);
    } else if (state == 2) {
        (void)step_byte((uint16_t)(ENV_STATE + x), -1);
        wr8((uint16_t)(ENV_AT + x), 0);
        y = env_next(x);
    } else {
        if (step_byte((uint16_t)(ENV_TIMER + x), -1) == 0)
            wr8((uint16_t)(ENV_STATE + x), (uint8_t)(state | 0x80));
        value = rd8((uint16_t)(ENV_LEFT + x));
        if (value == 0) {
            y = env_next(x);
        } else {
            y = env_move_level(x, ENV_RATE_LOW, ENV_RATE_HIGH);
            value = rd8((uint16_t)(ENV_LEFT + x));
            if (value != 0xFF)
                (void)step_byte((uint16_t)(ENV_LEFT + x), -1);
        }
    }
    e = card_exit_over(e, env_volume(x));
    return y >= 0 ? with_y(e, (uint8_t)y) : e;
}

void abi_psg_volume_envelope(void)
{
    card_exit_apply(psg_volume_envelope());
}

/* The arrays one channel of a song start clears, in the order the assembly clears them. */
static const uint16_t SONG_CLEARED[] = {
    0x260D, 0x24B5, 0x24AF, 0x2527, 0x254B, 0x2533, 0x2557, 0x2581, 0x2593, 0x25E1, 0x24CD, 0x255D, 0x2569, 0x2575, 0x24D3
};

/*
 * @card 02:CD45 psg_start_song
 * WHAT IT DOES: Starts the song that was asked for, if one was: sets up each channel the song's header names.
 * HOW: $22E1 the song asked for (bit 7 set: none, or already started); the song's header found through the pointer at $22D0 and the list
 * of headers, its first byte a mask of channels (bit 7 set: it is a song); for each channel in the mask the state, counts and effects
 * arrays cleared, the default shape, balance and scale set, the channel's start pointer taken from the header, the flags set from
 * $22E2 if the header's bit 6 says so; then the fade cleared, the song marked started and the player's done flag cleared.
 * WAITS: no.
 * Entry: nothing. Exit: A $40, X 6, Y the bytes of the header read, C 1, N and V from the done flags' old byte, Z from A and it.
 */
CardExit psg_start_song(void)
{
    CardExit e = { 0 };
    uint8_t value;
    uint8_t header;
    uint8_t y;
    uint8_t song;

    song = rd8(SONG_NEXT);
    if ((song & 0x80) != 0)
        return exit_a(song);
    y = (uint8_t)(song << 1);
    value = rd8(SONG_LIST);
    wr8(ZP_EA, value);
    value = rd8((uint16_t)(SONG_LIST + 1));
    wr8(ZP_EB, value);
    value = rd8_zp_y(ZP_EA, y);
    wr8(ZP_E8, value);
    y++;
    value = rd8_zp_y(ZP_EA, y);
    wr8(ZP_E9, value);
    header = rd8_zp_y(ZP_E8, 0);
    if ((header & 0x80) == 0)
        return with_y(with_c(exit_a(header), false), y);
    wr8(ZP_EA, header);
    wr8(ZP_EB, header);
    y = 0;
    for (uint8_t x = 0; x < CHANNELS; x++) {
        uint8_t mask = rd8(ZP_EA);

        wr8(ZP_EA, (uint8_t)(mask >> 1));
        if ((mask & 1) == 0)
            continue;
        wr8((uint16_t)(CHANNEL_STATE + x), 1);
        wr8((uint16_t)(NOTE_COUNT + x), 1);
        for (unsigned i = 0; i < sizeof SONG_CLEARED / sizeof SONG_CLEARED[0]; i++)
            wr8((uint16_t)(SONG_CLEARED[i] + x), 0);
        value = rd8(CHANNEL_DEFAULT_SHAPE);
        wr8((uint16_t)(SHAPE_LOW + x), value);
        value = rd8((uint16_t)(CHANNEL_DEFAULT_SHAPE + 1));
        wr8((uint16_t)(SHAPE_HIGH + x), value);
        wr8((uint16_t)(ENV_RELEASE_HIGH_AT + x), 0x84);
        wr8((uint16_t)(SCALE_BASE_AT + x), 4);
        y++;
        value = rd8_zp_y(ZP_E8, y);
        wr8((uint16_t)(CHANNEL_START_LOW + x), value);
        wr8((uint16_t)(CHANNEL_LOOP_LOW + x), value);
        y++;
        value = rd8_zp_y(ZP_E8, y);
        wr8((uint16_t)(CHANNEL_START_HIGH + x), value);
        wr8((uint16_t)(CHANNEL_LOOP_HIGH + x), value);
        if (x >= 4)
            wr8((uint16_t)(NOISE_AT + x), 0x80);
        wr8((uint16_t)(FLAGS_AT + x), 0x80);
        value = rd8(ZP_EB);
        if ((value & 0x40) != 0) {
            value = rd8(SONG_CHANNEL_FLAGS);
            wr8((uint16_t)(FLAGS_AT + x), value);
        }
    }
    wr8(TEMPO_ON, 0);
    wr8(TEMPO_TICKS, 0);
    value = rd8(SONG_NEXT);
    wr8(SONG_NEXT, (uint8_t)(value | 0x80));
    value = rd8(SONG_FLAGS);
    wr8(SONG_FLAGS, (uint8_t)(value & ~0x40));
    e.a = 0x40;
    e.x = CHANNELS;
    e.y = y;
    e.c = true;
    e.n = (value & 0x80) != 0;
    e.v = (value & 0x40) != 0;
    e.z = (uint8_t)(value & ~0x40) == 0;
    e.set_a = e.set_x = e.set_y = e.set_c = e.set_n = e.set_v = e.set_z = true;
    return e;
}

void abi_psg_start_song(void)
{
    card_exit_apply(psg_start_song());
}
