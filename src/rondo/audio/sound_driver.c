/*
 * The sound driver: the PSG's six channels, music and effects, loaded with the kernel's second
 * block at sectors $4D to $56 and mapped at $8000-$DFFF (banks $75 to $77) only while the kernel
 * calls it. Channels 0 to 5 are the PSG's; the driver keeps eight, and its channels 6 and 7 are
 * effects that take over PSG channels 4 and 5.
 *
 * A body returns plain values; its wrapper works out what the assembly leaves in the registers
 * and flags.
 */
#include "machine.h"
#include "ram.h"
#include "syscard.h"
#include "access.h"
#include "routines.h"

enum {
    PSG_CHANNELS = 6, PSG_VOLUME_FULL = 0xFF, LEVEL_SILENT = 0x1F,
    EFFECT_SHARED_FIRST = 4, EFFECT_SHARED_END = 6, EFFECT_CHANNEL_OFFSET = 2,
    FADE_STEP_START = 0x10, SOUND_WAVES = 0xBD11, SEQUENCE_NOTES_END = 0x60, SEQUENCE_REUSE = 0x80,
    SEQUENCE_LONG_FIRST = 0xE0, OFFSET_NEGATIVE = 0x40, PSG_CHANNEL_ON = 0x80, BALANCE_SET_SIZE = 0x10,
    PSG_VOLUME_LIMIT = 0x20, WAVE_NOISE = 0xFF, NOISE_FREQUENCY = 0x1F, PSG_NOISE_ON = 0x80,
    PSG_WAVE_RESET = 0x40, OPCODE_TIN = 0xD3, OPCODE_RTS = 0x60, WAVE_SAMPLES = 0x20,
    SOUND_CHANNELS = 8, NOTE_VOLUME_BASE = 0x10, VOLUME_STEPS = 0x0F, RATE_TENS = 10,
    REPEAT_MARKED = 0, REPEAT_FIRST_PASS = 1, REPEAT_AGAIN = 2, SEQUENCE_HANDLERS_FIRST = 0xE2,
    PAUSE_EFFECT = 0x53, FADE_END = 0x14,
    CHANNEL_VOLUME_START = 0x1F, CHANNEL_PAN_START = 0x08, MUSIC_CHANNELS = 0x3F, MUSIC_FIRST = 0x55,
    VOICE_SECOND_SET = 0x80, VOICE_SET_FIRST = 0x62, VOICE_SET_END = 0x70, VOICE_BACK = 0x3FF6,
    VOICE_WAITING = 0x01, VOICE_SECOND_FIRST = 0x72, VOICE_SECOND_STAGES = 0x0C, VOICE_ENTRY = 6,
    COMMAND_STOP = 0xFF, COMMAND_FADE = 0xFE, COMMAND_ALT_BALANCE = 0xFD, COMMAND_FIRST_BALANCE = 0xFC,
    COMMAND_MUSIC_LAST = 0x61, COMMAND_EXCLUSIVE = 0x60, COMMAND_VOICES = 0x62, COMMAND_PAIRED = 0x3D,
    EFFECT_ENTRY = 3, EFFECT_CHANNEL_A = 6, EFFECT_CHANNEL_B = 7, EFFECT_CHANNEL_BITS = 0xC0,
    SEQUENCE_REST = 0xE0, SEQUENCE_LENGTH = 0xE1, SEQUENCE_VOLUMES = 0x70, SEQUENCE_GLIDE_TO = 0xF3,
    TEMPO_FULL = 0xFF
};

/* PSG channels 4 and 5 are left alone while effect channels 6 and 7 play on them. */
static bool effect_holds(uint8_t channel, uint8_t *effect)
{
    if (channel < EFFECT_SHARED_FIRST || channel >= EFFECT_SHARED_END)
        return false;
    *effect = rd8((uint16_t)(SoundChannelCommand + channel + EFFECT_CHANNEL_OFFSET));
    return *effect != 0;
}

/*
 * @routine 004E:614 SoundSilenceAll
 * @run 75:8E14
 * WHAT IT DOES: Turns every PSG channel off, and the main volume down.
 * HOW: Selects channels 5 down to 0 and clears each one's control; then the main volume.
 * WAITS: no.
 * Entry: X kept on the stack across it (a typed caller passes its own). Exit: A, X, Y, C and V kept;
 * N and Z from X. zSoundChannel 0.
 */
void SoundSilenceAll(uint8_t kept_x)
{
    push8(kept_x);
    for (int channel = PSG_CHANNELS - 1; channel >= 0; channel--) {
        wr8(zSoundChannel, (uint8_t)channel);
        wr8(PsgSelect, (uint8_t)channel);
        wr8(PsgControl, 0);
    }
    wr8(PsgMainVolume, 0);
    (void)pull8();
}

void abi_SoundSilenceAll(void)
{
    SoundSilenceAll(cpu.x);
    abi_nz(cpu.x);
}

/*
 * @routine 004E:627 SoundVolumeFull
 * @run 75:8E27
 * WHAT IT DOES: Turns the main volume full up, left and right.
 * HOW: $FF to the main volume.
 * Entry: nothing. Exit: A $FF, N set, Z clear. X, Y, C and V kept.
 */
void SoundVolumeFull(void)
{
    wr8(PsgMainVolume, PSG_VOLUME_FULL);
}

void abi_SoundVolumeFull(void)
{
    SoundVolumeFull();
    cpu.a = PSG_VOLUME_FULL;
    abi_nz(PSG_VOLUME_FULL);
}

/*
 * @routine 004E:62D SoundStartEnvelope
 * @run 75:8E2D
 * WHAT IT DOES: Starts a channel's envelope from its first phase.
 * HOW: SoundEnvelopePhase 1.
 * Entry: X channel. Exit: A, X, Y, C and V kept; N and Z from A.
 */
void SoundStartEnvelope(uint8_t channel)
{
    wr8((uint16_t)(SoundEnvelopePhase + channel), 1);
}

void abi_SoundStartEnvelope(void)
{
    SoundStartEnvelope(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 004E:635 SoundSilenceChannel
 * @run 75:8E35
 * WHAT IT DOES: Silences a channel: its envelope's level to $1F and its phase off, and the PSG
 * channel's control and balance cleared, unless it is 4 or 5 and an effect plays on it.
 * HOW: The PSG channel is the one already selected.
 * Entry: X channel. Exit: A, X and Y kept; N and Z from A; C set if the channel is 6 or more; V
 * bit 6 of the effect's command when channel 4 or 5, else kept.
 */
ChannelRelease SoundSilenceChannel(uint8_t channel)
{
    ChannelRelease release = { false, 0 };

    wr8((uint16_t)(SoundEnvelopeLevel + channel), LEVEL_SILENT);
    wr8((uint16_t)(SoundEnvelopePhase + channel), 0);
    release.looked = channel >= EFFECT_SHARED_FIRST && channel < EFFECT_SHARED_END;
    if (!effect_holds(channel, &release.effect)) {
        wr8(PsgControl, 0);
        wr8(PsgBalance, 0);
    }
    return release;
}

void abi_SoundSilenceChannel(void)
{
    ChannelRelease release = SoundSilenceChannel(cpu.x);

    abi_nz(cpu.a);
    abi_flag(P_C, cpu.x >= EFFECT_SHARED_END);
    if (release.looked)
        abi_flag(P_V, (release.effect & 0x40) != 0);
}

/*
 * A product by shift and add, high bit of the multiplier first, as the driver does it; what the
 * last add left is what its caller sees in A and V.
 */
static SoundProduct shift_add(uint8_t multiplier, int bits, uint8_t multiplicand)
{
    SoundProduct product = { 0, false, 0, false };

    for (int bit = bits - 1; bit >= 0; bit--) {
        product.value = (uint16_t)(product.value << 1);
        if (multiplier & (1u << bit)) {
            uint8_t before = (uint8_t)(product.value >> 8);

            product.value = (uint16_t)(product.value + multiplicand);
            product.added = true;
            product.high = (uint8_t)(product.value >> 8);
            product.overflow = before == 0x7F && product.high == 0x80;
        }
    }
    wr8(zSoundWordLo, (uint8_t)product.value);
    wr8(zSoundWordHi, (uint8_t)(product.value >> 8));
    wr8(zSoundScratch, 0);
    return product;
}

/* A the high byte at the last add, or what it held before; V from that add; C clear. */
static void abi_exit_product(SoundProduct product, uint8_t before)
{
    cpu.a = product.added ? product.high : before;
    if (product.added)
        abi_flag(P_V, product.overflow);
    abi_flag(P_C, false);
    abi_nz(cpu.x);
}

/*
 * @routine 004E:746 SoundMultiply
 * @run 75:8F46
 * WHAT IT DOES: Multiplies two bytes into a word in zSoundWordLo and zSoundWordHi.
 * HOW: Eight steps of shift and add, the multiplier from zSoundScratch.
 * WAITS: no.
 * Entry: A and Y the bytes. Exit: X and Y kept, N and Z from X; C clear; A the product's high
 * byte as of the last add, V from that add, or A and V kept when A was 0.
 */
SoundProduct SoundMultiply(uint8_t a, uint8_t y, uint8_t kept_x)
{
    SoundProduct product = { 0, false, 0, false };
    uint8_t step, low, high, new_low, new_high;
    uint16_t word = 0;
    bool carry;

    push8(kept_x);
    WR8(ZP->sound_scratch, a);
    WR8(ZP->sound_word.lo, 0);
    WR8(ZP->sound_word.hi, 0);
    for (step = 0; step != 8; step++) {
        carry = ASL8(ZP->sound_word.lo);
        ROL8(ZP->sound_word.hi, carry);
        word = (uint16_t)(word << 1);
        carry = ASL8(ZP->sound_scratch);
        if (carry) {
            low = RD8(ZP->sound_word.lo);
            new_low = (uint8_t)(low + y);
            WR8(ZP->sound_word.lo, new_low);
            carry = (unsigned)low + y > 0xFF;
            high = RD8(ZP->sound_word.hi);
            new_high = (uint8_t)(high + (carry ? 1 : 0));
            WR8(ZP->sound_word.hi, new_high);
            word = (uint16_t)(word + y);
            product.added = true;
            product.high = new_high;
            product.overflow = high == 0x7F && new_high == 0x80;
        }
    }
    product.value = word;
    (void)pull8();
    return product;
}

void abi_SoundMultiply(void)
{
    abi_exit_product(SoundMultiply(cpu.a, cpu.y, cpu.x), cpu.a);
}

/*
 * @routine 004E:768 SoundMultiplyNibble
 * @run 75:8F68
 * WHAT IT DOES: Multiplies the low nibble of A by Y into zSoundWordLo and zSoundWordHi.
 * HOW: A shifted up four places, then four steps of shift and add.
 * Entry: A and Y. Exit: as SoundMultiply, with A 0 when the nibble was 0.
 */
SoundProduct SoundMultiplyNibble(uint8_t a, uint8_t y)
{
    return shift_add((uint8_t)(a & 0x0F), 4, y);
}

void abi_SoundMultiplyNibble(void)
{
    abi_exit_product(SoundMultiplyNibble(cpu.a, cpu.y), (uint8_t)(cpu.a << 4));
}

/*
 * @routine 0053:62E SoundReadPair
 * @run 76:B62E
 * WHAT IT DOES: Reads a sequence's address for a channel: the next two bytes, its place low and
 * high.
 * HOW: Through the pointer in zSoundWordLo and zSoundWordHi, moved on past them.
 * Entry: X channel. Exit: A the second byte; N and Z from the pointer's low byte, or its high
 * when the low wrapped to 0. X, Y, C and V kept.
 */
uint8_t SoundReadPair(uint8_t channel)
{
    uint8_t second;

    wr8((uint16_t)(SoundSequenceLo + channel), rd8_zp(zSoundWordLo));
    sound_pointer_step();
    second = rd8_zp(zSoundWordLo);
    wr8((uint16_t)(SoundSequenceHi + channel), second);
    sound_pointer_step();
    return second;
}

void abi_SoundReadPair(void)
{
    uint8_t low;

    cpu.a = SoundReadPair(cpu.x);
    low = rd8(zSoundWordLo);
    abi_nz(low != 0 ? low : rd8(zSoundWordHi));
}

/* The sequence pointer on by one byte. */
void sound_pointer_step(void)
{
    uint8_t low = (uint8_t)(rd8(zSoundWordLo) + 1);

    wr8(zSoundWordLo, low);
    if (low == 0)
        wr8(zSoundWordHi, (uint8_t)(rd8(zSoundWordHi) + 1));
}

/*
 * @routine 0053:274 SoundStartFade
 * @run 76:B274
 * WHAT IT DOES: Starts the music fading out, unless it already is.
 * HOW: SoundFading set, SoundFadeStep $10.
 * Entry: nothing. Exit: A SoundFading when it was set, else $10; N and Z from A. X, Y, C and V
 * kept.
 */
uint8_t SoundStartFade(void)
{
    uint8_t fading = rd8(SoundFading);

    if (fading != 0)
        return fading;
    wr8(SoundFading, 1);
    wr8(SoundFadeStep, FADE_STEP_START);
    return FADE_STEP_START;
}

void abi_SoundStartFade(void)
{
    cpu.a = SoundStartFade();
    abi_nz(cpu.a);
}

/*
 * @routine 004D:5F1 SoundLoadSequence
 * @run 75:85F1
 * WHAT IT DOES: Takes up a channel's place in its sequence.
 * HOW: SoundSequenceLo and Hi into zSoundSequenceLo and Hi.
 * Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundLoadSequence(uint8_t channel)
{
    uint8_t high;

    wr8(zSoundSequenceLo, rd8((uint16_t)(SoundSequenceLo + channel)));
    high = rd8((uint16_t)(SoundSequenceHi + channel));
    wr8(zSoundSequenceHi, high);
    return high;
}

void abi_SoundLoadSequence(void)
{
    cpu.a = SoundLoadSequence(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 004D:5FC SoundSaveSequence
 * @run 75:85FC
 * WHAT IT DOES: Keeps a channel's place in its sequence.
 * HOW: zSoundSequenceLo and Hi into SoundSequenceLo and Hi.
 * Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundSaveSequence(uint8_t channel)
{
    uint8_t high;

    wr8((uint16_t)(SoundSequenceLo + channel), rd8(zSoundSequenceLo));
    high = rd8(zSoundSequenceHi);
    wr8((uint16_t)(SoundSequenceHi + channel), high);
    return high;
}

void abi_SoundSaveSequence(void)
{
    cpu.a = SoundSaveSequence(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 004D:607 SoundSequenceFromWord
 * @run 75:8607
 * WHAT IT DOES: Moves the place in the sequence to the address in zSoundWordLo and Hi.
 * HOW: Two bytes copied.
 * Entry: nothing. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundSequenceFromWord(void)
{
    uint8_t high;

    wr8(zSoundSequenceLo, rd8(zSoundWordLo));
    high = rd8(zSoundWordHi);
    wr8(zSoundSequenceHi, high);
    return high;
}

void abi_SoundSequenceFromWord(void)
{
    cpu.a = SoundSequenceFromWord();
    abi_nz(cpu.a);
}

/*
 * One step of a slide by channel: none while its step is 0; stopped, its step cleared, when the
 * value's high byte has reached the target; else the step added to the value. The array addresses
 * are the slide's.
 */
static SlideStep slide_step(uint8_t channel, uint16_t step_lo, uint16_t step_hi, uint16_t value_lo,
                            uint16_t value_hi, uint16_t target)
{
    SlideStep slide = { SLIDE_IDLE, 0, false, false };
    unsigned sum;
    uint8_t high;
    uint8_t step;

    if ((rd8((uint16_t)(step_lo + channel)) | rd8((uint16_t)(step_hi + channel))) == 0)
        return slide;
    slide.value = rd8((uint16_t)(value_hi + channel));
    if (slide.value == rd8((uint16_t)(target + channel))) {
        wr8((uint16_t)(step_lo + channel), 0);
        wr8((uint16_t)(step_hi + channel), 0);
        slide.how = SLIDE_REACHED;
        return slide;
    }
    sum = (unsigned)rd8((uint16_t)(step_lo + channel)) + rd8((uint16_t)(value_lo + channel));
    wr8((uint16_t)(value_lo + channel), (uint8_t)sum);
    step = rd8((uint16_t)(step_hi + channel));
    high = rd8((uint16_t)(value_hi + channel));
    sum = (unsigned)step + high + (sum >> 8);
    slide.value = (uint8_t)sum;
    slide.carry = sum > 0xFF;
    slide.overflow = (~(step ^ high) & (step ^ slide.value) & 0x80) != 0;
    wr8((uint16_t)(value_hi + channel), slide.value);
    slide.how = SLIDE_MOVED;
    return slide;
}

/* Idle, A 0 and Z set; reached, A the value with Z and C set; moved, A the value with its sum. */
void slide_exit_shim(SlideStep slide)
{
    switch (slide.how) {
    case SLIDE_IDLE:
        cpu.a = 0;
        abi_nz(0);
        break;
    case SLIDE_REACHED:
        cpu.a = slide.value;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case SLIDE_MOVED:
        cpu.a = slide.value;
        abi_nz(slide.value);
        abi_flag(P_C, slide.carry);
        abi_flag(P_V, slide.overflow);
        break;
    }
}

/*
 * @routine 004D:743 SoundSlideStep
 * @run 75:8743
 * WHAT IT DOES: Moves a channel's slide on by its step, until its high byte reaches the target.
 * HOW: SoundSlideStepLo and Hi onto SoundSlideLo and Hi; SoundSlideTarget.
 * Entry: X channel. Exit: X and Y kept. No slide, A 0, Z set, N clear, C and V kept. At the
 * target, A the high byte, Z and C set, N clear, V kept. Moved, A the high byte, N, Z, C and V
 * from its add.
 */
SlideStep SoundSlideStep(uint8_t channel)
{
    return slide_step(channel, SoundSlideStepLo, SoundSlideStepHi, SoundSlideLo, SoundSlideHi,
                      SoundSlideTarget);
}

void abi_SoundSlideStep(void)
{
    slide_exit_shim(SoundSlideStep(cpu.x));
}

/*
 * @routine 004D:7CC SoundPanSlideStep
 * @run 75:87CC
 * WHAT IT DOES: Moves a channel's pan on by its step, until it reaches the target.
 * HOW: SoundPanStepLo and Hi onto SoundPanLo and SoundPan; SoundPanTarget.
 * Entry: X channel. Exit: as SoundSlideStep.
 */
SlideStep SoundPanSlideStep(uint8_t channel)
{
    return slide_step(channel, SoundPanStepLo, SoundPanStepHi, SoundPanLo, SoundPan,
                      SoundPanTarget);
}

void abi_SoundPanSlideStep(void)
{
    slide_exit_shim(SoundPanSlideStep(cpu.x));
}

/*
 * @routine 004E:310 SoundSetPan
 * @run 75:8B10
 * WHAT IT DOES: Sets a channel's pan outright, as set and as it plays, with no fraction.
 * HOW: SoundPanSet and SoundPan, SoundPanLo cleared.
 * Entry: A pan, X channel. Exit: all registers and flags kept.
 */
void SoundSetPan(uint8_t channel, uint8_t pan)
{
    wr8((uint16_t)(SoundPanSet + channel), pan);
    wr8((uint16_t)(SoundPan + channel), pan);
    wr8((uint16_t)(SoundPanLo + channel), 0);
}

void abi_SoundSetPan(void)
{
    SoundSetPan(cpu.x, cpu.a);
}

/*
 * @routine 0053:721 SoundWaveAddress
 * @run 76:B721
 * WHAT IT DOES: Points the block-move stub's source at a waveform: 32 bytes each from $BD11.
 * HOW: The waveform times 32, plus $BD11, into BlockMoveSourceLo and Hi.
 * Entry: A waveform. Exit: A the address's high byte, N, Z, C and V from adding it. X and Y kept.
 */
uint16_t SoundWaveAddress(uint8_t wave)
{
    uint16_t address = (uint16_t)(SOUND_WAVES + (wave << 5));

    wr8(BlockMoveSourceLo, (uint8_t)address);
    wr8(BlockMoveSourceHi, (uint8_t)(address >> 8));
    return address;
}

void wave_address_exit_shim(uint8_t wave)
{
    uint8_t page = (uint8_t)(wave >> 3);
    uint8_t high = (uint8_t)((SOUND_WAVES + (wave << 5)) >> 8);
    bool carry = ((wave & 0x07) << 5) + (SOUND_WAVES & 0xFF) > 0xFF;

    cpu.a = high;
    abi_nz(high);
    abi_flag(P_C, page + (SOUND_WAVES >> 8) + carry > 0xFF);
    abi_flag(P_V, (~(page ^ (SOUND_WAVES >> 8)) & (page ^ high) & 0x80) != 0);
}

void abi_SoundWaveAddress(void)
{
    uint8_t wave = cpu.a;

    SoundWaveAddress(wave);
    wave_address_exit_shim(wave);
}

/*
 * @routine 0053:6F1 SoundCommandLength
 * @run 76:B6F1
 * WHAT IT DOES: How many argument bytes the command in SoundCommandByte takes. Below $60 one,
 * unless its bit 7 says to reuse the channel's default, which goes into SoundCommandArgs with none
 * read; $60 up none, except the commands from $E0 up, whose lengths are a table.
 * HOW: A with bit 7 off picks the class; SoundChannelDefaultArg; SoundCommandLengths.
 * WAITS: no.
 * Entry: A the command byte, X channel. Exit: A the count. One, N and Z from it, C clear. A
 * default, N and Z from the default, C set. None from $60 to $DF, N and Z from it less $E0, C
 * clear. From the table, N and Z from the count, C set, V clear, Y the command less $E0.
 * Otherwise X, Y and V kept.
 */
CommandLength SoundCommandLength(uint8_t byte, uint8_t channel)
{
    CommandLength length = { LENGTH_ONE, 1, 0, 0 };

    length.command = rd8(SoundCommandByte);
    if ((byte & 0x7F) < SEQUENCE_NOTES_END) {
        if (length.command < SEQUENCE_REUSE) {
            return length;
        }
        wr8(SoundCommandByte, (uint8_t)(length.command & 0x7F));
        length.value = rd8((uint16_t)(SoundChannelDefaultArg + channel));
        wr8(SoundCommandArgs, length.value);
        length.how = LENGTH_DEFAULT;
        length.count = 0;
        return length;
    }
    length.count = 0;
    if (length.command < SEQUENCE_LONG_FIRST) {
        length.how = LENGTH_NONE;
        return length;
    }
    length.value = (uint8_t)(length.command - SEQUENCE_LONG_FIRST);
    length.count = rd8((uint16_t)(SoundCommandLengths + length.value));
    length.how = LENGTH_TABLE;
    return length;
}

void command_length_exit_shim(CommandLength length)
{
    cpu.a = length.count;
    switch (length.how) {
    case LENGTH_ONE:
        abi_nz(length.count);
        abi_flag(P_C, false);
        break;
    case LENGTH_DEFAULT:
        abi_nz(length.value);
        abi_flag(P_C, true);
        break;
    case LENGTH_NONE:
        abi_nz((uint8_t)(length.command - SEQUENCE_LONG_FIRST));
        abi_flag(P_C, false);
        break;
    case LENGTH_TABLE:
        cpu.y = length.value;
        abi_nz(length.count);
        abi_flag(P_C, true);
        abi_flag(P_V, false);
        break;
    }
}

void abi_SoundCommandLength(void)
{
    command_length_exit_shim(SoundCommandLength(cpu.a, cpu.x));
}

/* The pointer in zero page moved on one byte: the low byte, and the high byte when it wraps. */
static void sequence_advance(uint16_t pointer)
{
    uint8_t low = (uint8_t)(rd8(pointer) + 1);

    wr8(pointer, low);
    if (low == 0)
        wr8((uint16_t)(pointer + 1), (uint8_t)(rd8((uint16_t)(pointer + 1)) + 1));
}

/* A command and its arguments into SoundCommandByte and SoundCommandArgs, through a pointer. */
static CommandRead read_command(uint16_t pointer, uint16_t length_ret, uint8_t channel)
{
    CommandRead read;
    uint8_t byte = rd8_zp(pointer);
    uint8_t left;

    wr8(SoundCommandByte, byte);
    sequence_advance(pointer);
    jsr_push(length_ret);
    read.length = SoundCommandLength(byte, channel);
    jsr_pop();
    /* The count is taken down until it goes negative, so above $80 it reads none. */
    left = read.length.count;
    read.args = 0;
    while ((int8_t)(left = (uint8_t)(left - 1)) >= 0) {
        push8(left);
        wr8((uint16_t)(SoundCommandArgs + read.args), rd8_zp(pointer));
        read.args++;
        left = pull8();
        sequence_advance(pointer);
    }
    read.left = left;
    return read;
}

/* As the count's exit, with A the count run out, N set, Z clear and Y the bytes read. */
void command_read_exit_shim(CommandRead read)
{
    command_length_exit_shim(read.length);
    cpu.a = read.left;
    cpu.y = read.args;
    abi_nz(read.left);
}

/*
 * @routine 0053:6A3 SoundReadCommand
 * @run 76:B6A3
 * WHAT IT DOES: Reads the channel's next sequence command and its arguments, moving its place on.
 * HOW: Through zSoundSequenceLo and Hi; SoundCommandLength says how many arguments follow.
 * WAITS: no.
 * Entry: X channel. Exit: A $FF, or the count less one when it is above $80; N set, Z clear; Y
 * the argument bytes read; C and V as SoundCommandLength left them. X kept.
 */
CommandRead SoundReadCommand(uint8_t channel)
{
    return read_command(zSoundSequenceLo, 0xB6B1, channel);
}

void abi_SoundReadCommand(void)
{
    command_read_exit_shim(SoundReadCommand(cpu.x));
}

/*
 * @routine 0053:6C6 SoundPeekCommand
 * @run 76:B6C6
 * WHAT IT DOES: Reads the channel's next sequence command and its arguments without moving its
 * place on.
 * HOW: A copy of the place in zSoundWordLo and Hi is read through instead.
 * WAITS: no.
 * Entry: X channel. Exit: as SoundReadCommand.
 */
CommandRead SoundPeekCommand(uint8_t channel)
{
    wr8(zSoundWordLo, rd8(zSoundSequenceLo));
    wr8(zSoundWordHi, rd8(zSoundSequenceHi));
    return read_command(zSoundWordLo, 0xB6DC, channel);
}

void abi_SoundPeekCommand(void)
{
    command_read_exit_shim(SoundPeekCommand(cpu.x));
}

/*
 * @routine 004E:1F2 SoundModulate
 * @run 75:89F2
 * WHAT IT DOES: Adds a channel's modulation to the word in zSoundWordLo and Hi: a table word at
 * its phase, masked by channel; the phase moves on by the channel's speed.
 * HOW: SoundModulationSpeed onto SoundModulationPhaseLo, carrying into SoundModulationPhase,
 * which indexes SoundModulationTable a byte a step; SoundModulationMaskLo and Hi.
 * Entry: X channel. Exit: no speed, A 0, Z set, N clear, Y, C and V kept. Otherwise A the word's
 * high byte, N, Z, C and V from adding to it, Y the phase before. X kept.
 */
Modulation SoundModulate(uint8_t channel)
{
    Modulation mod = { false, 0, 0, 0, 0, 0 };
    unsigned sum;

    mod.speed = rd8((uint16_t)(SoundModulationSpeed + channel));
    if (mod.speed == 0)
        return mod;
    mod.applied = true;
    mod.phase = rd8((uint16_t)(SoundModulationPhase + channel));
    sum = (unsigned)mod.speed + rd8((uint16_t)(SoundModulationPhaseLo + channel));
    wr8((uint16_t)(SoundModulationPhaseLo + channel), (uint8_t)sum);
    if (sum > 0xFF)
        wr8((uint16_t)(SoundModulationPhase + channel),
            (uint8_t)(rd8((uint16_t)(SoundModulationPhase + channel)) + 1));
    sum = (unsigned)(rd8((uint16_t)(SoundModulationTable + mod.phase)) &
                     rd8((uint16_t)(SoundModulationMaskLo + channel))) + rd8(zSoundWordLo);
    wr8(zSoundWordLo, (uint8_t)sum);
    mod.add = (uint8_t)(rd8((uint16_t)(SoundModulationTable + 1 + mod.phase)) &
                        rd8((uint16_t)(SoundModulationMaskHi + channel)));
    mod.before = rd8(zSoundWordHi);
    mod.carry_in = sum > 0xFF;
    wr8(zSoundWordHi, (uint8_t)(mod.add + mod.before + mod.carry_in));
    return mod;
}

void abi_SoundModulate(void)
{
    Modulation mod = SoundModulate(cpu.x);
    uint8_t high;

    if (!mod.applied) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    high = (uint8_t)(mod.add + mod.before + mod.carry_in);
    cpu.a = high;
    cpu.y = mod.phase;
    abi_nz(high);
    abi_flag(P_C, mod.add + mod.before + mod.carry_in > 0xFF);
    abi_flag(P_V, (~(mod.add ^ mod.before) & (mod.add ^ high) & 0x80) != 0);
}

/*
 * @routine 004E:21C SoundRampStep
 * @run 75:8A1C
 * WHAT IT DOES: Moves a channel's ramp on by its step, once the note's gate is over and only while
 * the volume less the slide is still at least the ramp.
 * HOW: SoundGateTimer, SoundRampBy, SoundVolumeBase less SoundSlideHi against SoundRampHi;
 * SoundRampBy onto SoundRampLo, carrying into SoundRampHi.
 * Entry: X channel. Exit: X and Y kept. Waiting, A the wait, N and Z from it. No step, A 0, Z
 * set, N clear. Too close, A the distance, N and Z from it less the ramp, C clear, V from the
 * distance's subtract. Moved, A the ramp, N, Z, C and V from its add.
 */
Ramp SoundRampStep(uint8_t channel)
{
    Ramp ramp = { RAMP_WAITING, 0, 0, 0, 0 };
    unsigned sum;

    ramp.value = rd8((uint16_t)(SoundGateTimer + channel));
    if (ramp.value != 0)
        return ramp;
    ramp.value = rd8((uint16_t)(SoundRampBy + channel));
    ramp.how = RAMP_NO_STEP;
    if (ramp.value == 0)
        return ramp;
    ramp.from = rd8((uint16_t)(SoundVolumeBase + channel));
    ramp.slide = rd8((uint16_t)(SoundSlideHi + channel));
    ramp.value = (uint8_t)(ramp.from - ramp.slide);
    ramp.high = rd8((uint16_t)(SoundRampHi + channel));
    ramp.how = RAMP_TOO_CLOSE;
    if (ramp.value < ramp.high)
        return ramp;
    sum = (unsigned)rd8((uint16_t)(SoundRampLo + channel)) + rd8((uint16_t)(SoundRampBy + channel));
    wr8((uint16_t)(SoundRampLo + channel), (uint8_t)sum);
    ramp.high = rd8((uint16_t)(SoundRampHi + channel));
    ramp.value = (uint8_t)(ramp.high + (sum >> 8));
    wr8((uint16_t)(SoundRampHi + channel), ramp.value);
    ramp.how = RAMP_MOVED;
    return ramp;
}

void ramp_exit_shim(Ramp ramp)
{
    cpu.a = ramp.value;
    switch (ramp.how) {
    case RAMP_WAITING:
    case RAMP_NO_STEP:
        abi_nz(ramp.value);
        break;
    case RAMP_TOO_CLOSE:
        abi_nz((uint8_t)(ramp.value - ramp.high));
        abi_flag(P_C, false);
        abi_flag(P_V, ((ramp.from ^ ramp.slide) & (ramp.from ^ ramp.value) & 0x80) != 0);
        break;
    case RAMP_MOVED:
        abi_nz(ramp.value);
        abi_flag(P_C, ramp.value < ramp.high);
        abi_flag(P_V, ramp.high == 0x7F && ramp.value == 0x80);
        break;
    }
}

void abi_SoundRampStep(void)
{
    ramp_exit_shim(SoundRampStep(cpu.x));
}

/*
 * @routine 004E:245 SoundTempoSlideStep
 * @run 75:8A45
 * WHAT IT DOES: Moves a channel's tempo toward its target by its step, as 8.8, and ends the slide
 * when it gets there.
 * HOW: SoundTempoSlideBy added to or taken from SoundTempoLo, carrying into SoundTempo;
 * SoundTempoTarget.
 * Entry: X channel. Exit: X and Y kept. No step, A 0, Z set, N clear. There, A the target, Z and
 * C set, N clear. Moved, A the fraction, C and V from its add or subtract, N and Z from it, or
 * from the value when that carried into it.
 */
TempoSlide SoundTempoSlideStep(uint8_t channel)
{
    TempoSlide approach = { TEMPO_SLIDE_IDLE, 0, 0, 0, 0, false };
    unsigned sum;

    approach.step = rd8((uint16_t)(SoundTempoSlideBy + channel));
    if (approach.step == 0)
        return approach;
    approach.target = rd8((uint16_t)(SoundTempoTarget + channel));
    approach.value = rd8((uint16_t)(SoundTempo + channel));
    if (approach.target == approach.value) {
        wr8((uint16_t)(SoundTempoSlideBy + channel), 0);
        approach.how = TEMPO_SLIDE_THERE;
        return approach;
    }
    approach.before = rd8((uint16_t)(SoundTempoLo + channel));
    approach.step = rd8((uint16_t)(SoundTempoSlideBy + channel));
    if (approach.target > approach.value) {
        sum = (unsigned)approach.before + approach.step;
        approach.how = TEMPO_SLIDE_UP;
        approach.carried = sum > 0xFF;
    } else {
        sum = (unsigned)approach.before - approach.step;
        approach.how = TEMPO_SLIDE_DOWN;
        approach.carried = approach.before < approach.step;
    }
    wr8((uint16_t)(SoundTempoLo + channel), (uint8_t)sum);
    if (approach.carried) {
        approach.value = (uint8_t)(rd8((uint16_t)(SoundTempo + channel)) +
                                   (approach.how == TEMPO_SLIDE_UP ? 1 : -1));
        wr8((uint16_t)(SoundTempo + channel), approach.value);
    }
    return approach;
}

void tempo_slide_exit_shim(TempoSlide approach)
{
    uint8_t fraction;

    switch (approach.how) {
    case TEMPO_SLIDE_IDLE:
        cpu.a = 0;
        abi_nz(0);
        return;
    case TEMPO_SLIDE_THERE:
        cpu.a = approach.target;
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    case TEMPO_SLIDE_UP:
        fraction = (uint8_t)(approach.before + approach.step);
        abi_flag(P_C, approach.carried);
        abi_flag(P_V, (~(approach.before ^ approach.step) & (approach.before ^ fraction) & 0x80) != 0);
        break;
    case TEMPO_SLIDE_DOWN:
        fraction = (uint8_t)(approach.before - approach.step);
        abi_flag(P_C, !approach.carried);
        abi_flag(P_V, ((approach.before ^ approach.step) & (approach.before ^ fraction) & 0x80) != 0);
        break;
    default:
        return;
    }
    cpu.a = fraction;
    abi_nz(approach.carried ? approach.value : fraction);
}

void abi_SoundTempoSlideStep(void)
{
    tempo_slide_exit_shim(SoundTempoSlideStep(cpu.x));
}

/* A timer set to a rate, with the phase moved on when asked. */
static uint8_t envelope_next(uint8_t channel, uint16_t rate, bool next_phase)
{
    uint8_t frames;

    if (next_phase)
        wr8((uint16_t)(SoundEnvelopePhase + channel), (uint8_t)(rd8((uint16_t)(SoundEnvelopePhase + channel)) + 1));
    frames = rd8((uint16_t)(rate + channel));
    wr8((uint16_t)(SoundEnvelopeTimer + channel), frames);
    return frames;
}

/* The phase's timer counted down; true when it ran out. */
static bool envelope_timer_out(uint8_t channel, EnvelopeStep *step)
{
    step->timer = (uint8_t)(rd8((uint16_t)(SoundEnvelopeTimer + channel)) - 1);
    wr8((uint16_t)(SoundEnvelopeTimer + channel), step->timer);
    if (step->timer != 0) {
        step->how = ENVELOPE_WAITING;
        return false;
    }
    step->how = ENVELOPE_TIMED;
    return true;
}

/*
 * @routine 004E:590 SoundEnvelopeStep
 * @run 75:8D90
 * WHAT IT DOES: One frame of a channel's volume envelope. Start: the attack's level and timer, or
 * straight to the decay with no attack. Attack: a step louder each time its timer runs out, to
 * the decay at full volume. Decay: a step quieter, to the release at the sustain level. Release:
 * a step quieter each time, without end. Off, and past the release, nothing.
 * HOW: SoundEnvelopePhase, SoundEnvelopeTimer and SoundEnvelopeLevel by channel, with the rates
 * and levels set for it. The release compares A, which is 0 there, with $1F, so it never stops
 * itself.
 * Entry: X channel. Exit: X, Y and V kept. Off, A $FF, N set, Z clear. Past the release, A the
 * phase less 4, N and Z from it. Timer still running, A 0, N and Z from the timer. Timer set, A
 * the frames it was set to, N and Z from them; C from comparing the level with the sustain in the
 * decay, clear in the release, else kept.
 */
EnvelopeStep SoundEnvelopeStep(uint8_t channel)
{
    EnvelopeStep step = { ENVELOPE_OFF, 0, 0, 0, ENVELOPE_COMPARED_NONE, 0, 0 };

    step.phase = rd8((uint16_t)(SoundEnvelopePhase + channel));
    switch (step.phase) {
    case ENVELOPE_OFF:
        return step;
    case ENVELOPE_START:
        wr8((uint16_t)(SoundEnvelopePhase + channel), ENVELOPE_ATTACK);
        wr8((uint16_t)(SoundEnvelopeLevel + channel), rd8((uint16_t)(SoundAttackLevel + channel)));
        step.how = ENVELOPE_TIMED;
        if (rd8((uint16_t)(SoundAttackRate + channel)) != 0) {
            step.frames = envelope_next(channel, SoundAttackRate, false);
            return step;
        }
        step.frames = envelope_next(channel, SoundDecayRate, true);
        return step;
    case ENVELOPE_ATTACK:
        if (!envelope_timer_out(channel, &step))
            return step;
        step.level = (uint8_t)(rd8((uint16_t)(SoundEnvelopeLevel + channel)) - 1);
        wr8((uint16_t)(SoundEnvelopeLevel + channel), step.level);
        step.frames = step.level == 0 ? envelope_next(channel, SoundDecayRate, true)
                                      : envelope_next(channel, SoundAttackRate, false);
        return step;
    case ENVELOPE_DECAY:
        if (!envelope_timer_out(channel, &step))
            return step;
        step.level = (uint8_t)(rd8((uint16_t)(SoundEnvelopeLevel + channel)) + 1);
        wr8((uint16_t)(SoundEnvelopeLevel + channel), step.level);
        step.level = rd8((uint16_t)(SoundEnvelopeLevel + channel));
        step.compared = ENVELOPE_COMPARED_SUSTAIN;
        step.sustain = rd8((uint16_t)(SoundSustainLevel + channel));
        step.frames = step.level == step.sustain ? envelope_next(channel, SoundReleaseRate, true)
                                                 : envelope_next(channel, SoundDecayRate, false);
        return step;
    case ENVELOPE_RELEASE:
        if (!envelope_timer_out(channel, &step))
            return step;
        wr8((uint16_t)(SoundEnvelopeLevel + channel), (uint8_t)(rd8((uint16_t)(SoundEnvelopeLevel + channel)) + 1));
        step.compared = ENVELOPE_COMPARED_RELEASE;
        step.frames = envelope_next(channel, SoundReleaseRate, false);
        return step;
    default:
        step.how = ENVELOPE_PAST;
        return step;
    }
}

void abi_SoundEnvelopeStep(void)
{
    EnvelopeStep step = SoundEnvelopeStep(cpu.x);

    switch (step.how) {
    case ENVELOPE_OFF:
        cpu.a = 0xFF;
        abi_nz(0xFF);
        return;
    case ENVELOPE_PAST:
        cpu.a = (uint8_t)(step.phase - ENVELOPE_RELEASE);
        abi_nz(cpu.a);
        return;
    case ENVELOPE_WAITING:
        cpu.a = 0;
        abi_nz(step.timer);
        return;
    default:
        cpu.a = step.frames;
        abi_nz(step.frames);
        if (step.compared == ENVELOPE_COMPARED_SUSTAIN)
            abi_flag(P_C, step.level >= step.sustain);
        else if (step.compared == ENVELOPE_COMPARED_RELEASE)
            abi_flag(P_C, false);
        return;
    }
}

/*
 * @routine 004E:0FF SoundScaledOffset
 * @run 75:88FF
 * WHAT IT DOES: A signed offset into zSoundWordLo and Hi: an entry of SoundOffsetTable times the
 * channel's SoundVibratoDepth, negated when bit 6 of the byte is set.
 * HOW: The byte's low six bits pick the entry; SoundMultiply.
 * WAITS: no.
 * Entry: A the byte, X channel. Exit: Y the scale; X kept. Positive, as SoundMultiply. Negative, A
 * the word's high byte, N, Z, C and V from subtracting it from 0 with the low byte's borrow.
 */
ScaledOffset SoundScaledOffset(uint8_t byte, uint8_t channel)
{
    ScaledOffset offset;
    uint8_t entry = (uint8_t)(byte & 0x7F);

    offset.negative = entry >= OFFSET_NEGATIVE;
    offset.scale = rd8((uint16_t)(SoundVibratoDepth + channel));
    offset.product = SoundMultiply(rd8((uint16_t)(SoundOffsetTable + (entry & 0x3F))), offset.scale, channel);
    if (offset.negative) {
        uint16_t word = (uint16_t)(0 - offset.product.value);

        wr8(zSoundWordLo, (uint8_t)word);
        wr8(zSoundWordHi, (uint8_t)(word >> 8));
    }
    return offset;
}

void abi_SoundScaledOffset(void)
{
    uint8_t factor = rd8((uint16_t)(SoundOffsetTable + (cpu.a & 0x3F)));
    ScaledOffset offset = SoundScaledOffset(cpu.a, cpu.x);
    uint8_t low = (uint8_t)offset.product.value;
    uint8_t high = (uint8_t)(offset.product.value >> 8);
    uint8_t result;
    bool borrow = low != 0;

    cpu.y = offset.scale;
    if (!offset.negative) {
        abi_exit_product(offset.product, factor);
        return;
    }
    result = (uint8_t)(0 - high - borrow);
    cpu.a = result;
    abi_nz(result);
    abi_flag(P_C, !borrow && high == 0);
    abi_flag(P_V, (high & result & 0x80) != 0);
}

/*
 * @routine 004E:6E5 SoundSetControl
 * @run 75:8EE5
 * WHAT IT DOES: Sets the selected PSG channel's volume, on when not 0, and its balance by the
 * channel's pan; channels 4 and 5 are left alone while an effect plays on them.
 * HOW: The volume with bit 7 to the control; SoundBalances by SoundPan, the second set when
 * zSoundAltBalance is set.
 * Entry: A volume, X channel. Exit: X kept; C set if the channel is 6 or more. Left alone, A and
 * Y kept, N and V the effect's bits 7 and 6, Z clear. Otherwise A the balance, N and Z from it, Y
 * the pan; V clear for channels 4 and 5, else kept.
 */
ControlSet SoundSetControl(uint8_t volume, uint8_t channel)
{
    ControlSet set = { false, false, 0, 0, 0, 0, 0 };

    set.looked = channel >= EFFECT_SHARED_FIRST && channel < EFFECT_SHARED_END;
    if (effect_holds(channel, &set.effect)) {
        set.held = true;
        return set;
    }
    wr8(PsgControl, volume != 0 ? (uint8_t)(volume | PSG_CHANNEL_ON) : 0);
    set.pan = rd8((uint16_t)(SoundPan + channel));
    set.alt = rd8(zSoundAltBalance);
    set.balance = rd8((uint16_t)(SoundBalances + (set.alt != 0 ? BALANCE_SET_SIZE : 0) + set.pan));
    wr8(PsgBalance, set.balance);
    return set;
}

void control_exit_shim(ControlSet set, uint8_t channel)
{
    abi_flag(P_C, channel >= EFFECT_SHARED_END);
    if (set.held) {
        abi_flag(P_N, (set.effect & 0x80) != 0);
        abi_flag(P_V, (set.effect & 0x40) != 0);
        abi_flag(P_Z, false);
        return;
    }
    cpu.a = set.balance;
    cpu.y = set.pan;
    abi_nz(set.balance);
    abi_flag(P_V, (set.alt & 0x40) != 0);
}

void abi_SoundSetControl(void)
{
    control_exit_shim(SoundSetControl(cpu.a, cpu.x), cpu.x);
}

/*
 * @routine 004E:27A SoundApplyVolume
 * @run 75:8A7A
 * WHAT IT DOES: A frame of the channel's envelope, then its PSG volume: the base, less the
 * envelope, the slide, the ramp and what is given, less the fade on the six PSG channels, plus
 * the two additions; below 0 it is 0, above $1F it is $1F.
 * HOW: zSoundWordLo holds what is given; SoundEnvelopeStep; SoundSetControl.
 * Entry: A what to take off, X channel. Exit: as SoundSetControl, with A the volume when left
 * alone.
 */
ControlSet SoundApplyVolume(uint8_t less, uint8_t channel)
{
    uint8_t volume;

    wr8(zSoundWordLo, less);
    SoundEnvelopeStep(channel);
    volume = (uint8_t)(rd8((uint16_t)(SoundVolumeBase + channel)) - rd8((uint16_t)(SoundEnvelopeLevel + channel)) -
                       rd8((uint16_t)(SoundSlideHi + channel)) - rd8((uint16_t)(SoundRampHi + channel)) -
                       rd8(zSoundWordLo));
    if (channel < EFFECT_SHARED_END)
        volume = (uint8_t)(volume - rd8(SoundFading));
    volume = (uint8_t)(volume + rd8((uint16_t)(SoundVolumeAddA + channel)) + rd8((uint16_t)(SoundVolumeAddB + channel)));
    if (volume & 0x80)
        volume = 0;
    else if (volume >= PSG_VOLUME_LIMIT)
        volume = PSG_VOLUME_LIMIT - 1;
    {
        ControlSet set = SoundSetControl(volume, channel);

        set.volume = volume;
        return set;
    }
}

void abi_SoundApplyVolume(void)
{
    uint8_t channel = cpu.x;
    ControlSet set = SoundApplyVolume(cpu.a, channel);

    if (set.held)
        cpu.a = set.volume;
    control_exit_shim(set, channel);
}

/*
 * @routine 004E:2B0 SoundRest
 * @run 75:8AB0
 * WHAT IT DOES: A rest: the channel silenced for the length in the command's argument, with no
 * gate and no volume.
 * HOW: SoundChannel2550 1, SoundSilenceChannel; the length into SoundChannelDefaultArg and
 * SoundNoteLength.
 * Entry: X channel. Exit: A the length, N and Z from it; C and V as SoundSilenceChannel. X and Y
 * kept.
 */
ChannelSilence SoundRest(uint8_t channel)
{
    ChannelSilence silence;
    uint8_t length;

    wr8((uint16_t)(SoundChannel2550 + channel), 1);
    silence.release = SoundSilenceChannel(channel);
    length = rd8(SoundCommandArgs);
    wr8((uint16_t)(SoundChannelDefaultArg + channel), length);
    wr8((uint16_t)(SoundNoteLength + channel), length);
    wr8((uint16_t)(SoundGate + channel), 0);
    wr8((uint16_t)(SoundGateTimer + channel), 0);
    wr8((uint16_t)(SoundVolumeBase + channel), 0);
    silence.length = length;
    return silence;
}

void abi_SoundRest(void)
{
    ChannelSilence silence = SoundRest(cpu.x);

    abi_flag(P_C, cpu.x >= EFFECT_SHARED_END);
    if (silence.release.looked)
        abi_flag(P_V, (silence.release.effect & 0x40) != 0);
    cpu.a = silence.length;
    abi_nz(silence.length);
}

/*
 * @routine 004E:2CB SoundSetLength
 * @run 75:8ACB
 * WHAT IT DOES: Sets a note's length and its gate from the command's two arguments: the gate is
 * that many sixteenths of the length, at least one frame; a gate of 0 sounds the whole length.
 * HOW: SoundNoteLength and SoundChannelDefaultArg, SoundGate and SoundGateSet;
 * SoundMultiplyNibble, its product shifted down four places, into SoundGateTimer.
 * Entry: X channel. Exit: A the gate timer, N and Z from it. No gate, C, V and Y kept. Otherwise
 * Y the length, C bit 3 of the product, V as SoundMultiplyNibble. X kept.
 */
NoteLength SoundSetLength(uint8_t channel)
{
    NoteLength note = { 0, 0, { 0, false, 0, false }, 0 };

    note.length = rd8(SoundCommandArgs);
    wr8((uint16_t)(SoundChannelDefaultArg + channel), note.length);
    wr8((uint16_t)(SoundNoteLength + channel), note.length);
    note.gate = rd8(SoundCommandArgs + 1);
    wr8((uint16_t)(SoundGate + channel), note.gate);
    wr8((uint16_t)(SoundGateSet + channel), note.gate);
    if (note.gate == 0) {
        note.timer = rd8(SoundCommandArgs);
    } else {
        note.product = SoundMultiplyNibble(note.gate, rd8(SoundCommandArgs));
        wr8(zSoundWordHi, (uint8_t)(note.product.value >> 12));
        note.timer = (uint8_t)(note.product.value >> 4);
        if (note.timer == 0)
            note.timer = 1;
    }
    wr8((uint16_t)(SoundGateTimer + channel), note.timer);
    return note;
}

void abi_SoundSetLength(void)
{
    NoteLength note = SoundSetLength(cpu.x);

    cpu.a = note.timer;
    abi_nz(note.timer);
    if (note.gate == 0)
        return;
    cpu.y = note.length;
    abi_flag(P_C, (note.product.value & 0x08) != 0);
    if (note.product.added)
        abi_flag(P_V, note.product.overflow);
}

/*
 * @routine 004E:69E SoundSetFrequency
 * @run 75:8E9E
 * WHAT IT DOES: Sets the selected PSG channel's pitch for a note: the note's period less a fine
 * share of the step to the next note; on a channel whose waveform is noise, the noise frequency
 * instead. Channels 4 and 5 are left alone while an effect plays on them.
 * HOW: SoundPeriods by note; the fine part times the step, SoundMultiply, its high byte off the
 * period; noise off for a tone. Noise takes the note less one, its low five bits.
 * WAITS: no.
 * Entry: A the fine part, X channel, Y note. Exit: X kept. Left alone, A and Y kept, N and V the
 * effect's bits 7 and 6, Z and C clear. Noise, A the noise byte, N and Z from it, C set, Y the
 * note less one. Tone, A the period's high byte, N, Z, C and V from its subtract, Y twice the
 * note. V for channels 4 and 5 clear otherwise.
 */
FrequencySet SoundSetFrequency(uint8_t fine, uint8_t channel, uint8_t note)
{
    FrequencySet set = { false, false, 0, false, 0, 0, 0 };
    uint16_t entry = (uint16_t)(SoundPeriods + (uint8_t)(note << 1));
    uint16_t period;
    uint8_t step;

    set.looked = channel >= EFFECT_SHARED_FIRST && channel < EFFECT_SHARED_END;
    if (effect_holds(channel, &set.effect)) {
        set.held = true;
        return set;
    }
    wr8(zSoundFine, fine);
    if (rd8((uint16_t)(SoundChannelWave + channel)) == WAVE_NOISE) {
        set.noise = true;
        set.value = (uint8_t)(((uint8_t)(note - 1) & NOISE_FREQUENCY) | PSG_NOISE_ON);
        wr8(PsgNoise, set.value);
        return set;
    }
    step = (uint8_t)(rd8(entry) - rd8((uint16_t)(entry + 2)));
    SoundMultiply(step, rd8(zSoundFine), 0);  /* X: not worked out yet */
    wr8(PsgNoise, 0);
    period = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    set.high_before = (uint8_t)(period >> 8);
    set.borrow = (period & 0xFF) < rd8(zSoundWordHi);
    period = (uint16_t)(period - rd8(zSoundWordHi));
    wr8(PsgFreqLo, (uint8_t)period);
    set.value = (uint8_t)(period >> 8);
    wr8(PsgFreqHi, set.value);
    return set;
}

void frequency_exit_shim(FrequencySet set, uint8_t note)
{
    if (set.held) {
        abi_flag(P_N, (set.effect & 0x80) != 0);
        abi_flag(P_V, (set.effect & 0x40) != 0);
        abi_flag(P_Z, false);
        abi_flag(P_C, false);
        return;
    }
    if (set.looked)
        abi_flag(P_V, false);
    cpu.a = set.value;
    abi_nz(set.value);
    if (set.noise) {
        cpu.y = (uint8_t)(note - 1);
        abi_flag(P_C, true);
        return;
    }
    cpu.y = (uint8_t)(note << 1);
    abi_flag(P_C, !(set.high_before == 0 && set.borrow));
    abi_flag(P_V, set.high_before == 0x80 && set.borrow);
}

void abi_SoundSetFrequency(void)
{
    uint8_t note = cpu.y;

    frequency_exit_shim(SoundSetFrequency(cpu.a, cpu.x, note), note);
}

/*
 * @routine 004E:654 SoundLoadWave
 * @run 75:8E54
 * WHAT IT DOES: Loads a waveform into the selected PSG channel: the channel off, its write place
 * reset, the 32 samples written. $FF loads none, and channels 4 and 5 are left alone while an
 * effect plays on them.
 * HOW: SoundWaveAddress, then the RAM block-move stub at $22E7 built as a TIN of 32 bytes to
 * PsgWaveData and called.
 * Entry: A waveform, X channel. Exit: X and Y kept. $FF, A $FF, Z and C set, N clear, V kept.
 * Left alone, A kept, N and V the effect's bits 7 and 6, Z and C clear. Loaded, A $60, N and Z
 * clear, C and V as SoundWaveAddress.
 */
WaveLoad SoundLoadWave(uint8_t wave, uint8_t channel)
{
    WaveLoad load = { WAVE_NONE, 0, 0 };
    uint16_t source;

    load.wave = wave;
    if (wave == WAVE_NOISE)
        return load;
    if (effect_holds(channel, &load.effect)) {
        load.how = WAVE_HELD;
        return load;
    }
    wr8(PsgBalance, 0);
    wr8(PsgControl, 0);
    wr8(PsgControl, PSG_WAVE_RESET);
    wr8(PsgControl, 0);
    source = SoundWaveAddress(wave);
    wr8(BlockMoveDestLo, (uint8_t)PsgWaveData);
    wr8(BlockMoveDestHi, (uint8_t)(PsgWaveData >> 8));
    wr8(BlockMoveStub, OPCODE_TIN);
    wr8(BlockMoveLengthLo, WAVE_SAMPLES);
    wr8(BlockMoveLengthHi, 0);
    wr8(BlockMoveReturn, OPCODE_RTS);
    for (uint16_t i = 0; i < WAVE_SAMPLES; i++)
        wr8(PsgWaveData, rd8((uint16_t)(source + i)));
    wr8(PsgControl, 0);
    load.how = WAVE_LOADED;
    return load;
}

void wave_load_exit_shim(WaveLoad load, uint8_t wave)
{
    switch (load.how) {
    case WAVE_NONE:
        cpu.a = wave;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case WAVE_HELD:
        abi_flag(P_N, (load.effect & 0x80) != 0);
        abi_flag(P_V, (load.effect & 0x40) != 0);
        abi_flag(P_Z, false);
        abi_flag(P_C, false);
        cpu.a = wave;
        break;
    case WAVE_LOADED:
        wave_address_exit_shim(wave);
        cpu.a = OPCODE_RTS;
        abi_nz(OPCODE_RTS);
        break;
    }
}

void abi_SoundLoadWave(void)
{
    uint8_t wave = cpu.a;

    wave_load_exit_shim(SoundLoadWave(wave, cpu.x), wave);
}

/*
 * @routine 0053:233 SoundInit
 * @run 76:B233
 * WHAT IT DOES: Starts the driver: every channel cleared, the PSG silent, no fade, nothing
 * playing, the LFO off.
 * HOW: SoundChannel25B0, SoundChannelCommand and SoundChannelPriority cleared by channel;
 * SoundSilenceAll.
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and V kept; N and Z from X; C set by the loop's last compare.
 */
void SoundInit(void)
{
    wr8(SoundInCommand, 0);
    for (uint8_t channel = 0; channel < SOUND_CHANNELS; channel++) {
        wr8((uint16_t)(SoundChannel25B0 + channel), 0);
        wr8((uint16_t)(SoundChannelCommand + channel), 0);
        wr8((uint16_t)(SoundChannelPriority + channel), 0);
        wr8(zSoundAltBalance, 0);
    }
    SoundSilenceAll(SOUND_CHANNELS);       /* X as the loop leaves it */
    wr8(SoundFading, 0);
    wr8(zSoundChannelsOn, 0);
    wr8(PsgLfoFreq, 0);
    wr8(PsgLfoControl, 0);
}

void abi_SoundInit(void)
{
    SoundInit();
    abi_nz(cpu.x);
    abi_flag(P_C, true);
}

/*
 * @routine 0053:25A SoundDriverStopAll
 * @run 76:B25A
 * WHAT IT DOES: Stops everything: no command or priority on any channel, no fade, nothing
 * playing, the PSG silent, noise and the LFO off.
 * HOW: The channels from 7 down; SoundSilenceAll.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear. A, Y, C and V kept.
 */
void SoundDriverStopAll(void)
{
    for (int channel = SOUND_CHANNELS - 1; channel >= 0; channel--) {
        wr8((uint16_t)(SoundChannelCommand + channel), 0);
        wr8((uint16_t)(SoundChannelPriority + channel), 0);
    }
    wr8(SoundFading, 0);
    wr8(zSoundChannelsOn, 0);
    jsr_push(0xB26D);
    SoundSilenceAll(0xFF);                 /* X as the loop leaves it */
    jsr_pop();
    wr8(PsgNoise, 0);
    wr8(PsgLfoControl, 0);
}

void abi_SoundDriverStopAll(void)
{
    SoundDriverStopAll();
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/* The pitch word in zSoundWordLo and Hi plus the channel's pitch; the high add is what shows. */
static Sum add_pitch(uint8_t channel)
{
    Sum low = add_bytes(rd8(zSoundWordLo), rd8((uint16_t)(SoundPitchFine + channel)), false);
    Sum high;

    wr8(zSoundWordLo, low.value);
    high = add_bytes(rd8(zSoundWordHi), rd8((uint16_t)(SoundPitchNote + channel)), low.carry);
    wr8(zSoundWordHi, high.value);
    return high;
}

/* The work word and the change count cleared, as a pitch tick starts. */
static void pitch_tick_start(void)
{
    wr8(zSoundWordLo, 0);
    wr8(zSoundWordHi, 0);
    wr8(zSoundScratch, 0);
}

static void count_change(void)
{
    wr8(zSoundScratch, (uint8_t)(rd8(zSoundScratch) + 1));
}

/* The vibrato's phase moved on, and its offset for that phase into the work word. */
static void vibrato_offset(uint8_t channel)
{
    uint8_t phase = (uint8_t)(rd8((uint16_t)(SoundVibratoPhase + channel)) +
                              rd8((uint16_t)(SoundVibratoSpeed + channel)));

    wr8((uint16_t)(SoundVibratoPhase + channel), phase);
    SoundScaledOffset(phase, channel);
    count_change();
}

/* What a pitch tick leaves before it sets the frequency: A the fine part, Y the note, N and Z. */
static void pitch_exit_shim(PitchTick tick, bool nz_from_note)
{
    cpu.a = tick.fine;
    cpu.y = tick.note;
    abi_nz(nz_from_note ? tick.note : tick.fine);
    abi_flag(P_C, tick.last.carry);
    abi_flag(P_V, tick.last.overflow);
    frequency_exit_shim(tick.frequency, tick.note);
}

/*
 * @routine 004E:02A SoundPitchTick
 * @run 75:882A
 * WHAT IT DOES: A frame of a channel's pitch: its glide, after its delay, a step toward the
 * target note and at the end onto it; its vibrato, after its delay, growing to its depth; its
 * modulation; then the PSG frequency for the pitch with those offsets.
 * HOW: SoundGlide*, SoundVibrato* and SoundModulate by channel; zSoundScratch counts what
 * changed; SoundSetFrequency with the fine part and the note.
 * Entry: X channel. Exit: as SoundSetFrequency, with A the fine part, Y the note and C and V from
 * adding the note where it keeps them.
 */
PitchTick SoundPitchTick(uint8_t channel)
{
    PitchTick tick;
    Sum low;

    pitch_tick_start();
    if (rd8((uint16_t)(SoundGlideFrames + channel)) != 0) {
        uint8_t delay = rd8((uint16_t)(SoundGlideDelay + channel));

        if (delay != 0) {
            wr8((uint16_t)(SoundGlideDelay + channel), (uint8_t)(delay - 1));
        } else {
            uint8_t left = (uint8_t)(rd8((uint16_t)(SoundGlideFrames + channel)) - 1);

            wr8((uint16_t)(SoundGlideFrames + channel), left);
            if (left == 0) {
                wr8((uint16_t)(SoundPitchFine + channel), rd8((uint16_t)(SoundGlideTargetFine + channel)));
                wr8((uint16_t)(SoundPitchNote + channel), rd8((uint16_t)(SoundGlideTargetNote + channel)));
            } else {
                low = add_bytes(rd8((uint16_t)(SoundPitchFine + channel)), rd8((uint16_t)(SoundGlideStepLo + channel)), false);
                wr8((uint16_t)(SoundPitchFine + channel), low.value);
                wr8((uint16_t)(SoundPitchNote + channel),
                    add_bytes(rd8((uint16_t)(SoundPitchNote + channel)), rd8((uint16_t)(SoundGlideStepHi + channel)),
                              low.carry).value);
            }
            count_change();
        }
    }
    if (rd8((uint16_t)(SoundVibratoDepthMax + channel)) != 0) {
        if (rd8((uint16_t)(SoundVibratoWait + channel)) != rd8((uint16_t)(SoundVibratoDelay + channel))) {
            wr8((uint16_t)(SoundVibratoWait + channel), (uint8_t)(rd8((uint16_t)(SoundVibratoWait + channel)) + 1));
        } else {
            if (rd8((uint16_t)(SoundVibratoDepth + channel)) < rd8((uint16_t)(SoundVibratoDepthMax + channel))) {
                low = add_bytes(rd8((uint16_t)(SoundVibratoDepthLo + channel)), rd8((uint16_t)(SoundVibratoGrowLo + channel)), false);
                wr8((uint16_t)(SoundVibratoDepthLo + channel), low.value);
                wr8((uint16_t)(SoundVibratoDepth + channel),
                    add_bytes(rd8((uint16_t)(SoundVibratoDepth + channel)), rd8((uint16_t)(SoundVibratoGrowHi + channel)),
                              low.carry).value);
            }
            vibrato_offset(channel);
        }
    }
    if (rd8((uint16_t)(SoundModulationSpeed + channel)) != 0) {
        SoundModulate(channel);
        count_change();
    }
    low = add_bytes(rd8((uint16_t)(SoundPitchFine + channel)), rd8(zSoundWordLo), false);
    tick.fine = low.value;
    tick.last = add_bytes(rd8((uint16_t)(SoundPitchNote + channel)), rd8(zSoundWordHi), low.carry);
    tick.note = tick.last.value;
    tick.frequency = SoundSetFrequency(tick.fine, channel, tick.note);
    return tick;
}

void abi_SoundPitchTick(void)
{
    pitch_exit_shim(SoundPitchTick(cpu.x), false);
}

/*
 * @routine 004E:159 SoundPitchTickPlain
 * @run 75:8959
 * WHAT IT DOES: A frame of a channel's pitch, the plain way: the vibrato at its depth with no
 * growing and no counting of its wait; the pitch; the glide's step scaled by the tempo's phase and
 * added, or taken off when it goes down; the modulation; then the PSG frequency.
 * HOW: The step is shifted down one place for each bit of SoundTempoPhase above its highest set bit,
 * and one more: a power of two near the share of the tick gone.
 * Entry: X channel. Exit: as SoundSetFrequency, with A the fine part, Y the note, N and Z from
 * the note, and C and V from the last add or subtract where it keeps them.
 */
PitchTick SoundPitchTickPlain(uint8_t channel)
{
    PitchTick tick;

    pitch_tick_start();
    if ((rd8((uint16_t)(SoundVibratoDepthLo + channel)) | rd8((uint16_t)(SoundVibratoDepth + channel))) != 0 &&
        rd8((uint16_t)(SoundVibratoWait + channel)) == rd8((uint16_t)(SoundVibratoDelay + channel)))
        vibrato_offset(channel);
    tick.last = add_pitch(channel);
    if (rd8((uint16_t)(SoundGlideFrames + channel)) != 0 && rd8((uint16_t)(SoundGlideDelay + channel)) == 0) {
        uint16_t step = (uint16_t)(rd8((uint16_t)(SoundGlideStepLo + channel)) |
                                   (rd8((uint16_t)(SoundGlideStepHi + channel)) << 8));
        bool down = (step & 0x8000) != 0;
        uint8_t shift = rd8((uint16_t)(SoundTempoPhase + channel));
        Sum low;

        count_change();
        if (down)
            step = (uint16_t)(0 - step);
        /* Shifted down a place for each bit the shift byte gives up before its set bit comes out. */
        for (;;) {
            bool last = (shift & 0x80) != 0;

            step = (uint16_t)(step >> 1);
            shift = (uint8_t)(shift << 1);
            if (last)
                break;
        }
        wr8(zSoundFine, (uint8_t)step);
        wr8(zSoundStepHi, (uint8_t)(step >> 8));
        if (!down) {
            low = add_bytes(rd8(zSoundWordLo), (uint8_t)step, false);
            wr8(zSoundWordLo, low.value);
            tick.last = add_bytes(rd8(zSoundWordHi), (uint8_t)(step >> 8), low.carry);
        } else {
            low = sub_bytes(rd8(zSoundWordLo), (uint8_t)step, true);
            wr8(zSoundWordLo, low.value);
            tick.last = sub_bytes(rd8(zSoundWordHi), (uint8_t)(step >> 8), low.carry);
        }
        wr8(zSoundWordHi, tick.last.value);
    }
    if (rd8((uint16_t)(SoundModulationSpeed + channel)) != 0) {
        Modulation mod = SoundModulate(channel);

        tick.last = add_bytes(mod.add, mod.before, mod.carry_in);
        count_change();
    }
    tick.fine = rd8(zSoundWordLo);
    tick.note = rd8(zSoundWordHi);
    tick.frequency = SoundSetFrequency(tick.fine, channel, tick.note);
    return tick;
}

void abi_SoundPitchTickPlain(void)
{
    pitch_exit_shim(SoundPitchTickPlain(cpu.x), true);
}

/*
 * @routine 004D:76F SoundVolumeTick
 * @run 75:876F
 * WHAT IT DOES: A frame of a channel's volume: its ramp, then, with a volume at all, its PSG
 * volume, less its tremolo once the tremolo's wait is over. Says whether the tremolo is still
 * waiting.
 * HOW: SoundRampStep; the tremolo's phase moved on, made positive and shifted up by its depth,
 * the high byte taken off by SoundApplyVolume.
 * Entry: X channel. Exit: no volume, A 0, Z set, N clear, C clear, Y and V as SoundRampStep.
 * Otherwise as SoundApplyVolume, with C set while the tremolo waits, else clear; Y 0 after a
 * tremolo where it keeps it.
 */
VolumeTick SoundVolumeTick(uint8_t channel)
{
    VolumeTick tick = { VOLUME_NONE, { RAMP_WAITING, 0, 0, 0, 0 }, 0, { false, false, 0, 0, 0, 0, 0 } };
    uint8_t depth;
    uint8_t less = 0;

    tick.ramp = SoundRampStep(channel);
    if (rd8((uint16_t)(SoundVolumeBase + channel)) == 0)
        return tick;
    depth = rd8((uint16_t)(SoundTremoloDepth + channel));
    tick.how = VOLUME_PLAIN;
    if (depth != 0) {
        if (rd8((uint16_t)(SoundTremoloWait + channel)) != rd8((uint16_t)(SoundTremoloDelay + channel))) {
            tick.how = VOLUME_WAITING;
        } else {
            uint8_t phase = (uint8_t)(rd8((uint16_t)(SoundTremoloPhase + channel)) +
                                      rd8((uint16_t)(SoundTremoloSpeed + channel)));
            uint16_t shifted;

            wr8((uint16_t)(SoundTremoloPhase + channel), phase);
            if (phase & 0x80)
                phase = (uint8_t)(0 - phase);
            shifted = phase;
            for (uint8_t place = depth; place != 0; place--)
                shifted = (uint16_t)(shifted << 1);
            less = (uint8_t)(shifted >> 8);
            tick.how = VOLUME_TREMOLO;
        }
    }
    tick.volume = SoundApplyVolume(less, channel);
    tick.less = less;
    return tick;
}

void volume_tick_exit_shim(VolumeTick tick, uint8_t channel)
{
    ramp_exit_shim(tick.ramp);
    if (tick.how == VOLUME_NONE) {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);
        return;
    }
    if (tick.how == VOLUME_TREMOLO)
        cpu.y = 0;
    cpu.a = tick.less;
    if (tick.volume.held)
        cpu.a = tick.volume.volume;
    control_exit_shim(tick.volume, channel);
    abi_flag(P_C, tick.how == VOLUME_WAITING);
}

void abi_SoundVolumeTick(void)
{
    uint8_t channel = cpu.x;

    volume_tick_exit_shim(SoundVolumeTick(channel), channel);
}

/*
 * @routine 004E:710 SoundDivide
 * @run 75:8F10
 * WHAT IT DOES: Divides the word in zSoundWordLo and Hi by A: the quotient in zSoundQuotientLo
 * and Hi, what is left in zSoundScratch and zSoundFine.
 * HOW: Sixteen steps of shift and subtract. Where the remainder's low byte is under the divisor
 * but its high byte is not 0, the high byte is taken down once before the subtract and again by
 * its borrow; the C keeps that.
 * Entry: A divisor. Exit: Y 0, Z set, N clear; X kept. A, C and V from the last step: after a
 * subtract, A the remainder's high byte with C and V from it; else A its low byte, C and V clear.
 */
Division SoundDivide(uint8_t divisor)
{
    Division division = { false, 0, 0, false, false, 0 };
    uint8_t low = rd8(zSoundWordLo);
    uint8_t high = rd8(zSoundWordHi);
    uint8_t left_lo = 0;
    uint8_t left_hi = 0;
    uint8_t quotient_lo = 0;
    uint8_t quotient_hi = 0;

    wr8(zSoundStepHi, divisor);
    for (int step = 0; step < 16; step++) {
        bool take;

        left_hi = (uint8_t)((left_hi << 1) | (left_lo >> 7));
        left_lo = (uint8_t)((left_lo << 1) | (high >> 7));
        high = (uint8_t)((high << 1) | (low >> 7));
        low = (uint8_t)(low << 1);
        quotient_hi = (uint8_t)((quotient_hi << 1) | (quotient_lo >> 7));
        quotient_lo = (uint8_t)(quotient_lo << 1);
        take = left_lo >= divisor;
        if (!take && left_hi != 0) {
            left_hi = (uint8_t)(left_hi - 1);
            take = true;
        }
        division.took = take;
        if (take) {
            bool borrow = left_lo < divisor;

            left_lo = (uint8_t)(left_lo - divisor);
            division.high_before = left_hi;
            division.borrow = borrow;
            left_hi = (uint8_t)(left_hi - borrow);
            quotient_lo = (uint8_t)(quotient_lo + 1);
        }
        division.last = take ? left_hi : left_lo;
    }
    wr8(zSoundWordLo, low);
    wr8(zSoundWordHi, high);
    wr8(zSoundScratch, left_lo);
    wr8(zSoundFine, left_hi);
    wr8(zSoundQuotientLo, quotient_lo);
    wr8(zSoundQuotientHi, quotient_hi);
    division.quotient = (uint16_t)(quotient_lo | (quotient_hi << 8));
    return division;
}

void division_exit_shim(Division division)
{
    cpu.a = division.last;
    cpu.y = 0;
    abi_nz(0);
    if (division.took) {
        abi_flag(P_C, !(division.high_before == 0 && division.borrow));
        abi_flag(P_V, division.high_before == 0x80 && division.borrow);
    } else {
        abi_flag(P_C, false);
        abi_flag(P_V, false);
    }
}

void abi_SoundDivide(void)
{
    division_exit_shim(SoundDivide(cpu.a));
}

/*
 * @routine 004D:6F6 SoundGlideSetup
 * @run 75:86F6
 * WHAT IT DOES: Aims a channel's glide at a pitch: the target kept, and the step a frame that
 * gets there from the pitch it has now in the glide's frames. Nothing without a glide.
 * HOW: The distance in zSoundWordLo and Hi, made positive for SoundDivide and the quotient made
 * negative again; SoundGlideStepLo and Hi.
 * Entry: A the target's fine part, Y its note, X channel. Exit: no glide, A and Y kept, Z set, N
 * and V clear. Otherwise A the step's low byte, N and Z from it, Y 0, C and V as SoundDivide, or
 * from negating the quotient's high byte when the glide goes down. X kept.
 */
GlideSetup SoundGlideSetup(uint8_t fine, uint8_t note, uint8_t channel)
{
    GlideSetup setup = { false, false, { false, 0, 0, false, false, 0 }, 0, 0 };
    uint8_t frames = rd8((uint16_t)(SoundGlideFrames + channel));
    uint16_t distance;

    setup.frames = frames;
    if (frames == 0)
        return setup;
    setup.aimed = true;
    wr8((uint16_t)(SoundGlideTargetFine + channel), fine);
    wr8((uint16_t)(SoundGlideTargetNote + channel), note);
    distance = (uint16_t)(((note << 8) | rd8((uint16_t)(SoundGlideTargetFine + channel))) -
                          ((rd8((uint16_t)(SoundPitchNote + channel)) << 8) |
                           rd8((uint16_t)(SoundPitchFine + channel))));
    setup.down = (distance & 0x8000) != 0;
    if (setup.down)
        distance = (uint16_t)(0 - distance);
    wr8(zSoundWordLo, (uint8_t)distance);
    wr8(zSoundWordHi, (uint8_t)(distance >> 8));
    setup.division = SoundDivide(rd8((uint16_t)(SoundGlideFrames + channel)));
    setup.step = setup.division.quotient;
    if (setup.down) {
        setup.step = (uint16_t)(0 - setup.step);
        wr8(zSoundQuotientLo, (uint8_t)setup.step);
        wr8(zSoundQuotientHi, (uint8_t)(setup.step >> 8));
    }
    wr8((uint16_t)(SoundGlideStepHi + channel), (uint8_t)(setup.step >> 8));
    wr8((uint16_t)(SoundGlideStepLo + channel), (uint8_t)setup.step);
    return setup;
}

void glide_setup_exit_shim(GlideSetup setup)
{
    uint8_t low = (uint8_t)setup.step;

    if (!setup.aimed) {
        abi_nz(0);
        abi_flag(P_V, false);
        return;
    }
    division_exit_shim(setup.division);
    if (setup.down) {
        uint8_t high = (uint8_t)(setup.division.quotient >> 8);
        bool borrow = (setup.division.quotient & 0xFF) != 0;
        uint8_t result = (uint8_t)(0 - high - borrow);

        abi_flag(P_C, !borrow && high == 0);
        abi_flag(P_V, (high & result & 0x80) != 0);
    }
    cpu.a = low;
    abi_nz(low);
}

void abi_SoundGlideSetup(void)
{
    glide_setup_exit_shim(SoundGlideSetup(cpu.a, cpu.y, cpu.x));
}

/*
 * @routine 004D:655 SoundNotePitch
 * @run 75:8655
 * WHAT IT DOES: Starts a channel's note at the pitch of the command: the note with the
 * transpose, the signed fine tune and both detunes; the vibrato from the start; a glide to it,
 * from an offset below it or from the last note; the frequency, the envelope from its start, the
 * tremolo and the ramp from the start; then a frame of the pitch.
 * HOW: SoundCommandByte is the note. The last note's pitch is kept in zSoundScratch and
 * zSoundFine for a glide from it; SoundGlideSetup; SoundSetFrequency; SoundStartEnvelope;
 * SoundPitchTick.
 * Entry: X channel. Exit: as SoundPitchTick.
 */
PitchTick SoundNotePitch(uint8_t channel)
{
    uint8_t note;
    uint8_t fine;
    Sum low;
    Sum high;

    rd8(SoundCommandByte);
    wr8(zSoundScratch, rd8((uint16_t)(SoundPitchFine + channel)));
    wr8(zSoundFine, rd8((uint16_t)(SoundPitchNote + channel)));
    note = (uint8_t)(rd8(SoundCommandByte) + rd8((uint16_t)(SoundTranspose + channel)));
    fine = rd8((uint16_t)(SoundFineTune + channel));
    if (fine & 0x80)
        note = (uint8_t)(note - 1);
    wr8(zSoundWordLo, note);
    low = add_bytes(fine, rd8((uint16_t)(SoundDetuneAFine + channel)), false);
    high = add_bytes(note, rd8((uint16_t)(SoundDetuneANote + channel)), low.carry);
    wr8(zSoundWordLo, high.value);
    low = add_bytes(low.value, rd8((uint16_t)(SoundDetuneBFine + channel)), false);
    wr8((uint16_t)(SoundPitchFine + channel), low.value);
    wr8((uint16_t)(SoundPitchNote + channel),
        add_bytes(rd8(zSoundWordLo), rd8((uint16_t)(SoundDetuneBNote + channel)), low.carry).value);
    wr8((uint16_t)(SoundVibratoPhase + channel), 0);
    wr8((uint16_t)(SoundVibratoWait + channel), 0);
    wr8((uint16_t)(SoundVibratoDepthLo + channel), 0);
    wr8((uint16_t)(SoundVibratoDepth + channel), 0);
    wr8((uint16_t)(SoundGlideFrames + channel), rd8((uint16_t)(SoundGlideFramesSet + channel)));
    if (rd8((uint16_t)(SoundGlideFrames + channel)) != 0) {
        uint8_t target_fine = rd8((uint16_t)(SoundPitchFine + channel));
        uint8_t target_note;

        wr8((uint16_t)(SoundGlideDelay + channel), rd8((uint16_t)(SoundGlideDelaySet + channel)));
        if (rd8((uint16_t)(SoundGlideFromLast + channel)) == 0) {
            low = sub_bytes(target_fine, rd8((uint16_t)(SoundGlideOffsetFine + channel)), true);
            wr8((uint16_t)(SoundPitchFine + channel), low.value);
            target_note = rd8((uint16_t)(SoundPitchNote + channel));
            wr8((uint16_t)(SoundPitchNote + channel),
                sub_bytes(target_note, rd8((uint16_t)(SoundGlideOffsetNote + channel)), low.carry).value);
        } else {
            target_note = rd8((uint16_t)(SoundPitchNote + channel));
            target_fine = rd8((uint16_t)(SoundPitchFine + channel));
            wr8((uint16_t)(SoundPitchFine + channel), rd8(zSoundScratch));
            wr8((uint16_t)(SoundPitchNote + channel), rd8(zSoundFine));
        }
        SoundGlideSetup(target_fine, target_note, channel);
    }
    SoundSetFrequency(rd8((uint16_t)(SoundPitchFine + channel)), channel, rd8((uint16_t)(SoundPitchNote + channel)));
    SoundStartEnvelope(channel);
    wr8((uint16_t)(SoundTremoloPhase + channel), 0);
    wr8((uint16_t)(SoundTremoloWait + channel), 0);
    wr8((uint16_t)(SoundRampLo + channel), 0);
    wr8((uint16_t)(SoundRampHi + channel), 0);
    return SoundPitchTick(channel);
}

void abi_SoundNotePitch(void)
{
    pitch_exit_shim(SoundNotePitch(cpu.x), false);
}

/*
 * @routine 004D:610 SoundStartNote
 * @run 75:8610
 * WHAT IT DOES: Starts a note from the command's three arguments: its length, its gate in
 * sixteenths of the length, and its volume, $10 up; its pitch; then its gate timer, at least a
 * frame, or the whole length with no gate.
 * HOW: SoundChannelDefaultArg, SoundGate and SoundVolumeBase; SoundNotePitch; SoundNoteLength;
 * SoundMultiplyNibble, its product shifted down four places, into SoundGateTimer.
 * Entry: X channel. Exit: A the gate timer, N and Z from it, Y the length. No gate, C and V as
 * SoundNotePitch left them; otherwise C bit 3 of the product, V as SoundMultiplyNibble. X kept.
 */
NoteStart SoundStartNote(uint8_t channel)
{
    NoteStart start;
    uint8_t length;
    uint8_t gate;

    wr8((uint16_t)(SoundChannelDefaultArg + channel), rd8(SoundCommandArgs));
    wr8((uint16_t)(SoundGate + channel), (uint8_t)(rd8(SoundCommandArgs + 1) & 0x0F));
    wr8((uint16_t)(SoundVolumeBase + channel), (uint8_t)((rd8(SoundCommandArgs + 2) & 0x0F) + NOTE_VOLUME_BASE));
    start.pitch = SoundNotePitch(channel);
    length = rd8((uint16_t)(SoundChannelDefaultArg + channel));
    wr8((uint16_t)(SoundNoteLength + channel), length);
    start.length = length;
    gate = rd8((uint16_t)(SoundGate + channel));
    start.gated = gate != 0;
    if (!start.gated) {
        start.timer = rd8((uint16_t)(SoundChannelDefaultArg + channel));
    } else {
        start.product = SoundMultiplyNibble(gate, length);
        wr8(zSoundWordHi, (uint8_t)(start.product.value >> 12));
        start.timer = (uint8_t)(start.product.value >> 4);
        if (start.timer == 0)
            start.timer = 1;
    }
    wr8((uint16_t)(SoundGateTimer + channel), start.timer);
    return start;
}

void note_start_exit_shim(NoteStart start)
{
    pitch_exit_shim(start.pitch, false);
    cpu.a = start.timer;
    cpu.y = start.length;
    abi_nz(start.timer);
    if (!start.gated)
        return;
    abi_flag(P_C, (start.product.value & 0x08) != 0);
    if (start.product.added)
        abi_flag(P_V, start.product.overflow);
}

void abi_SoundStartNote(void)
{
    note_start_exit_shim(SoundStartNote(cpu.x));
}

/* The command's arguments, as SoundReadCommand left them. */
static uint8_t argument(int index)
{
    return rd8((uint16_t)(SoundCommandArgs + index));
}

/* A byte set by channel from an argument; the last such load is what a handler's exit shows. */
static uint8_t set_from(uint16_t field, uint8_t channel, int index)
{
    uint8_t value = argument(index);

    wr8((uint16_t)(field + channel), value);
    return value;
}

/* A handler's exit that is the last byte it loaded: A that byte, N and Z from it. */
static void abi_exit_loaded(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

/*
 * @routine 004E:2FF SoundCommandWave
 * @run 75:8AFF
 * WHAT IT DOES: Command $E2: the channel's waveform, loaded into its PSG channel.
 * HOW: SoundChannelWave; SoundLoadWave.
 * Entry: X channel. Exit: as SoundLoadWave.
 */
WaveLoad SoundCommandWave(uint8_t channel)
{
    return SoundLoadWave(set_from(SoundChannelWave, channel, 0), channel);
}

void abi_SoundCommandWave(void)
{
    uint8_t wave = argument(0);

    cpu.a = wave;
    wave_load_exit_shim(SoundCommandWave(cpu.x), wave);
}

/*
 * @routine 004E:309 SoundCommandPan
 * @run 75:8B09
 * WHAT IT DOES: Command $E3: the channel's pan, unless it is 0.
 * HOW: SoundSetPan.
 * Entry: X channel. Exit: A the pan, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandPan(uint8_t channel)
{
    uint8_t pan = argument(0);

    if (pan != 0)
        SoundSetPan(channel, pan);
    return pan;
}

void abi_SoundCommandPan(void)
{
    abi_exit_loaded(SoundCommandPan(cpu.x));
}

/* A slide's start toward a target: up by the step, or down by it when the target is below. */
static SlideStart start_slide(uint8_t channel, uint16_t step_lo, uint16_t step_hi, uint16_t target,
                              uint16_t from, uint8_t step, uint8_t aim)
{
    SlideStart start = { false, 0, 0, 0 };

    wr8((uint16_t)(step_lo + channel), step);
    wr8((uint16_t)(step_hi + channel), 0);
    wr8((uint16_t)(target + channel), aim);
    start.aim = aim;
    start.from = rd8((uint16_t)(from + channel));
    if (aim >= start.from)
        return start;
    start.down = true;
    start.step = (uint8_t)(0 - argument(0));
    wr8((uint16_t)(step_lo + channel), start.step);
    wr8((uint16_t)(step_hi + channel), (uint8_t)(rd8((uint16_t)(step_hi + channel)) - 1));
    return start;
}

/* Up, A the target with the compare's flags; down, A the step, C and V from negating it, N set. */
static void abi_exit_slide_start(SlideStart start)
{
    if (!start.down) {
        cpu.a = start.aim;
        abi_nz((uint8_t)(start.aim - start.from));
        abi_flag(P_C, true);
        return;
    }
    cpu.a = start.step;
    abi_nz(0xFF);
    abi_flag(P_C, start.step == 0);
    abi_flag(P_V, (start.step & (uint8_t)(0 - start.step) & 0x80) != 0);
}

/*
 * @routine 004E:31A SoundCommandPanSlide
 * @run 75:8B1A
 * WHAT IT DOES: Command $F8: a pan slide by a step toward a target pan, down when the target is
 * below the pan as set.
 * HOW: SoundPanStepLo and Hi, SoundPanTarget against SoundPanSet.
 * Entry: X channel. Exit: X and Y kept. Up, A the target, N and Z from comparing it, C set, V
 * kept. Down, A the step made negative, N set, Z clear, C and V from negating it.
 */
SlideStart SoundCommandPanSlide(uint8_t channel)
{
    return start_slide(channel, SoundPanStepLo, SoundPanStepHi, SoundPanTarget, SoundPanSet,
                       argument(0), argument(1));
}

void abi_SoundCommandPanSlide(void)
{
    abi_exit_slide_start(SoundCommandPanSlide(cpu.x));
}

/*
 * @routine 004E:33B SoundCommandVibrato
 * @run 75:8B3B
 * WHAT IT DOES: Command $E4: the vibrato's delay, speed and the depth it grows to, by a whole
 * step a frame, from no fraction.
 * HOW: SoundVibratoDelay, SoundVibratoSpeed, SoundVibratoDepthMax and SoundVibratoGrowHi.
 * Entry: X channel. Exit: A the depth, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandVibrato(uint8_t channel)
{
    uint8_t depth;

    set_from(SoundVibratoDelay, channel, 0);
    set_from(SoundVibratoSpeed, channel, 1);
    depth = set_from(SoundVibratoDepthMax, channel, 2);
    wr8((uint16_t)(SoundVibratoGrowHi + channel), depth);
    wr8((uint16_t)(SoundVibratoDepthLo + channel), 0);
    return depth;
}

void abi_SoundCommandVibrato(void)
{
    abi_exit_loaded(SoundCommandVibrato(cpu.x));
}

/*
 * @routine 004E:354 SoundCommandVibratoGrow
 * @run 75:8B54
 * WHAT IT DOES: Command $F9: how fast the vibrato's depth grows, in sixteenths a frame.
 * HOW: The argument shifted up four places into SoundVibratoGrowLo and Hi.
 * Entry: X channel. Exit: A the low byte, N and Z from the high, C clear. X, Y and V kept.
 */
uint16_t SoundCommandVibratoGrow(uint8_t channel)
{
    uint16_t grow = (uint16_t)(argument(0) << 4);

    wr8((uint16_t)(SoundVibratoGrowHi + channel), (uint8_t)(grow >> 8));
    wr8((uint16_t)(SoundVibratoGrowLo + channel), (uint8_t)grow);
    return grow;
}

void abi_SoundCommandVibratoGrow(void)
{
    uint16_t grow = SoundCommandVibratoGrow(cpu.x);

    cpu.a = (uint8_t)grow;
    abi_nz((uint8_t)(grow >> 8));
    abi_flag(P_C, false);
}

/*
 * @routine 004E:36E SoundCommandModulation
 * @run 75:8B6E
 * WHAT IT DOES: Command $E5: the modulation's speed and its two masks, from the start.
 * HOW: SoundModulationSpeed, SoundModulationMaskHi and Lo; the phase cleared.
 * Entry: X channel. Exit: A the low mask, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandModulation(uint8_t channel)
{
    uint8_t mask;

    set_from(SoundModulationSpeed, channel, 0);
    set_from(SoundModulationMaskHi, channel, 1);
    mask = set_from(SoundModulationMaskLo, channel, 2);
    wr8((uint16_t)(SoundModulationPhaseLo + channel), 0);
    wr8((uint16_t)(SoundModulationPhase + channel), 0);
    return mask;
}

void abi_SoundCommandModulation(void)
{
    abi_exit_loaded(SoundCommandModulation(cpu.x));
}

/* A loop's start: the place after the command kept, its passes and what it adds cleared. */
static uint8_t loop_start(uint8_t channel, const SoundLoop *loop)
{
    uint8_t high;

    wr8((uint16_t)(loop->start_lo + channel), rd8(zSoundSequenceLo));
    high = rd8(zSoundSequenceHi);
    wr8((uint16_t)(loop->start_hi + channel), high);
    wr8((uint16_t)(loop->count + channel), 0);
    wr8((uint16_t)(loop->volume + channel), 0);
    wr8((uint16_t)(loop->detune_fine + channel), 0);
    wr8((uint16_t)(loop->detune_note + channel), 0);
    return high;
}

/*
 * A loop's end: after its passes, what it added cleared and on; otherwise the volume and the
 * detune moved by the arguments, the detune's sixteenths signed, and back to its start. 0 passes
 * loops for ever.
 */
static LoopEnd loop_end(uint8_t channel, const SoundLoop *loop)
{
    LoopEnd end = { false, 0, { 0, false, false } };
    uint8_t passes = argument(0);
    int16_t detune;
    Sum low;

    if (passes != 0) {
        uint8_t count = (uint8_t)(rd8((uint16_t)(loop->count + channel)) + 1);

        wr8((uint16_t)(loop->count + channel), count);
        if (passes == count) {
            wr8((uint16_t)(loop->volume + channel), 0);
            wr8((uint16_t)(loop->detune_fine + channel), 0);
            wr8((uint16_t)(loop->detune_note + channel), 0);
            end.done = true;
            end.value = passes;
            return end;
        }
    }
    wr8((uint16_t)(loop->volume + channel), (uint8_t)(argument(1) + rd8((uint16_t)(loop->volume + channel))));
    detune = (int16_t)((int8_t)argument(2) * 16);
    wr8(zSoundWordLo, (uint8_t)((uint16_t)detune >> 8));
    low = add_bytes((uint8_t)detune, rd8((uint16_t)(loop->detune_fine + channel)), false);
    wr8((uint16_t)(loop->detune_fine + channel), low.value);
    end.sum = add_bytes(rd8(zSoundWordLo), rd8((uint16_t)(loop->detune_note + channel)), low.carry);
    wr8((uint16_t)(loop->detune_note + channel), end.sum.value);
    wr8(zSoundSequenceLo, rd8((uint16_t)(loop->start_lo + channel)));
    end.value = rd8((uint16_t)(loop->start_hi + channel));
    wr8(zSoundSequenceHi, end.value);
    return end;
}

/* Done, A the passes, Z and C set, N clear; back, A the start's high byte, C and V from the detune. */
static void abi_exit_loop_end(LoopEnd end)
{
    cpu.a = end.value;
    if (end.done) {
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    }
    abi_nz(end.value);
    abi_flag(P_C, end.sum.carry);
    abi_flag(P_V, end.sum.overflow);
}

static const SoundLoop loop_a = {
    SoundLoopAStartLo, SoundLoopAStartHi, SoundLoopACount, SoundVolumeAddA, SoundDetuneAFine, SoundDetuneANote
};
static const SoundLoop loop_b = {
    SoundLoopBStartLo, SoundLoopBStartHi, SoundLoopBCount, SoundVolumeAddB, SoundDetuneBFine, SoundDetuneBNote
};

/*
 * @routine 004E:387 SoundCommandLoopA
 * @run 75:8B87
 * WHAT IT DOES: Command $E6: the start of loop A, with no passes made and nothing added yet.
 * HOW: The place after the command into SoundLoopAStartLo and Hi; SoundLoopACount,
 * SoundVolumeAddA and detune A cleared.
 * Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandLoopA(uint8_t channel)
{
    return loop_start(channel, &loop_a);
}

void abi_SoundCommandLoopA(void)
{
    abi_exit_loaded(SoundCommandLoopA(cpu.x));
}

/*
 * @routine 004E:39E SoundCommandLoopAEnd
 * @run 75:8B9E
 * WHAT IT DOES: Command $E7: the end of loop A: after its passes, on with what it added cleared;
 * else the volume and detune A moved by the arguments and back to its start. 0 passes loops for
 * ever.
 * HOW: SoundLoopACount; SoundVolumeAddA; the detune in sixteenths, signed, onto detune A.
 * Entry: X channel. Exit: done, A the passes, Z and C set, N clear, V kept. Back, A the start's
 * high byte, N and Z from it, C and V from adding the detune's notes. X and Y kept.
 */
LoopEnd SoundCommandLoopAEnd(uint8_t channel)
{
    return loop_end(channel, &loop_a);
}

void abi_SoundCommandLoopAEnd(void)
{
    abi_exit_loop_end(SoundCommandLoopAEnd(cpu.x));
}

/*
 * @routine 004E:3F1 SoundCommandLoopB
 * @run 75:8BF1
 * WHAT IT DOES: Command $E8: the start of loop B, as loop A's.
 * HOW: SoundLoopBStartLo and Hi; SoundLoopBCount, SoundVolumeAddB and detune B cleared.
 * Entry: X channel. Exit: as SoundCommandLoopA.
 */
uint8_t SoundCommandLoopB(uint8_t channel)
{
    return loop_start(channel, &loop_b);
}

void abi_SoundCommandLoopB(void)
{
    abi_exit_loaded(SoundCommandLoopB(cpu.x));
}

/*
 * @routine 004E:408 SoundCommandLoopBEnd
 * @run 75:8C08
 * WHAT IT DOES: Command $E9: the end of loop B, as loop A's.
 * HOW: SoundLoopBCount; SoundVolumeAddB; detune B.
 * Entry: X channel. Exit: as SoundCommandLoopAEnd.
 */
LoopEnd SoundCommandLoopBEnd(uint8_t channel)
{
    return loop_end(channel, &loop_b);
}

void abi_SoundCommandLoopBEnd(void)
{
    abi_exit_loop_end(SoundCommandLoopBEnd(cpu.x));
}

/*
 * @routine 004E:45B SoundCommandTempo
 * @run 75:8C5B
 * WHAT IT DOES: Command $EA: the channel's tempo.
 * HOW: SoundTempo.
 * Entry: X channel. Exit: A the value, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandTempo(uint8_t channel)
{
    return set_from(SoundTempo, channel, 0);
}

void abi_SoundCommandTempo(void)
{
    abi_exit_loaded(SoundCommandTempo(cpu.x));
}

/*
 * @routine 004E:462 SoundCommandTempoSlide
 * @run 75:8C62
 * WHAT IT DOES: Command $EB: a tempo slide by a step to a target tempo, from no fraction.
 * HOW: SoundTempoSlideBy, SoundTempoLo cleared, SoundTempoTarget.
 * Entry: X channel. Exit: A the target, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandTempoSlide(uint8_t channel)
{
    set_from(SoundTempoSlideBy, channel, 0);
    wr8((uint16_t)(SoundTempoLo + channel), 0);
    return set_from(SoundTempoTarget, channel, 1);
}

void abi_SoundCommandTempoSlide(void)
{
    abi_exit_loaded(SoundCommandTempoSlide(cpu.x));
}

/*
 * @routine 004E:472 SoundCommandTranspose
 * @run 75:8C72
 * WHAT IT DOES: Command $EC: the channel's transpose.
 * HOW: SoundTranspose.
 * Entry: X channel. Exit: A the transpose, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandTranspose(uint8_t channel)
{
    return set_from(SoundTranspose, channel, 0);
}

void abi_SoundCommandTranspose(void)
{
    abi_exit_loaded(SoundCommandTranspose(cpu.x));
}

/*
 * @routine 004E:479 SoundCommandTremolo
 * @run 75:8C79
 * WHAT IT DOES: Command $ED: the tremolo's delay, speed and depth.
 * HOW: SoundTremoloDelay, SoundTremoloSpeed, SoundTremoloDepth.
 * Entry: X channel. Exit: A the depth, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandTremolo(uint8_t channel)
{
    set_from(SoundTremoloDelay, channel, 0);
    set_from(SoundTremoloSpeed, channel, 1);
    return set_from(SoundTremoloDepth, channel, 2);
}

void abi_SoundCommandTremolo(void)
{
    abi_exit_loaded(SoundCommandTremolo(cpu.x));
}

/*
 * @routine 004E:48C SoundCommandVolume
 * @run 75:8C8C
 * WHAT IT DOES: Command $EE: the channel's volume, 0 to $0F, as the slide's attenuation $0F less
 * it, with no fraction.
 * HOW: SoundSlideHi, SoundSlideLo cleared.
 * Entry: X channel. Exit: A the attenuation, N, Z, C and V from its subtract. X and Y kept.
 */
uint8_t SoundCommandVolume(uint8_t channel)
{
    uint8_t attenuation = (uint8_t)(VOLUME_STEPS - argument(0));

    wr8((uint16_t)(SoundSlideHi + channel), attenuation);
    wr8((uint16_t)(SoundSlideLo + channel), 0);
    return attenuation;
}

void abi_SoundCommandVolume(void)
{
    uint8_t volume = argument(0);
    Sum sum = sub_bytes(VOLUME_STEPS, volume, true);

    cpu.a = SoundCommandVolume(cpu.x);
    abi_nz(cpu.a);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/*
 * @routine 004E:499 SoundCommandVolumeSlide
 * @run 75:8C99
 * WHAT IT DOES: Command $EF: a volume slide by a step to a target volume, as the attenuation $0F
 * less it; down in attenuation when the target is below the slide's.
 * HOW: SoundSlideStepLo and Hi, SoundSlideTarget against SoundSlideHi.
 * Entry: X channel. Exit: as SoundCommandPanSlide, with V from the target's subtract when up.
 */
SlideStart SoundCommandVolumeSlide(uint8_t channel)
{
    return start_slide(channel, SoundSlideStepLo, SoundSlideStepHi, SoundSlideTarget, SoundSlideHi,
                       argument(0), (uint8_t)(VOLUME_STEPS - argument(1)));
}

void abi_SoundCommandVolumeSlide(void)
{
    Sum aim = sub_bytes(VOLUME_STEPS, argument(1), true);

    abi_flag(P_V, aim.overflow);
    abi_exit_slide_start(SoundCommandVolumeSlide(cpu.x));
}

/*
 * @routine 004E:4BD SoundCommandGlideFromLast
 * @run 75:8CBD
 * WHAT IT DOES: Command $F0: glides from the last note to each new one, over the frames given,
 * with no delay.
 * HOW: SoundGlideFromLast 1, SoundGlideDelaySet 0, SoundGlideFramesSet.
 * Entry: X channel. Exit: A the frames, N and Z from them. X, Y, C and V kept.
 */
uint8_t SoundCommandGlideFromLast(uint8_t channel)
{
    wr8((uint16_t)(SoundGlideFromLast + channel), 1);
    wr8((uint16_t)(SoundGlideDelaySet + channel), 0);
    return set_from(SoundGlideFramesSet, channel, 0);
}

void abi_SoundCommandGlideFromLast(void)
{
    abi_exit_loaded(SoundCommandGlideFromLast(cpu.x));
}

/*
 * @routine 004E:4CC SoundCommandGlideFromBelow
 * @run 75:8CCC
 * WHAT IT DOES: Command $F1: glides into each new note from notes below it, after a delay, over
 * the frames given.
 * HOW: SoundGlideFromLast 0, SoundGlideDelaySet, SoundGlideFramesSet, SoundGlideOffsetNote, no
 * fine offset.
 * Entry: X channel. Exit: A the notes below, N and Z from them. X, Y, C and V kept.
 */
uint8_t SoundCommandGlideFromBelow(uint8_t channel)
{
    uint8_t below;

    wr8((uint16_t)(SoundGlideFromLast + channel), 0);
    set_from(SoundGlideDelaySet, channel, 0);
    set_from(SoundGlideFramesSet, channel, 1);
    below = set_from(SoundGlideOffsetNote, channel, 2);
    wr8((uint16_t)(SoundGlideOffsetFine + channel), 0);
    return below;
}

void abi_SoundCommandGlideFromBelow(void)
{
    abi_exit_loaded(SoundCommandGlideFromBelow(cpu.x));
}

/*
 * @routine 004E:4E5 SoundCommandFineTune
 * @run 75:8CE5
 * WHAT IT DOES: Command $F2: the channel's fine tune.
 * HOW: SoundFineTune.
 * Entry: X channel. Exit: A the fine tune, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandFineTune(uint8_t channel)
{
    return set_from(SoundFineTune, channel, 0);
}

void abi_SoundCommandFineTune(void)
{
    abi_exit_loaded(SoundCommandFineTune(cpu.x));
}

/*
 * @routine 004E:4EC SoundCommandRepeatMark
 * @run 75:8CEC
 * WHAT IT DOES: Command $F6: marks the start of a section to be played twice.
 * HOW: The place after the command into SoundRepeatMarkLo and Hi; SoundRepeatState 0.
 * Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandRepeatMark(uint8_t channel)
{
    uint8_t high;

    wr8((uint16_t)(SoundRepeatMarkLo + channel), rd8(zSoundSequenceLo));
    high = rd8(zSoundSequenceHi);
    wr8((uint16_t)(SoundRepeatMarkHi + channel), high);
    wr8((uint16_t)(SoundRepeatState + channel), 0);
    return high;
}

void abi_SoundCommandRepeatMark(void)
{
    abi_exit_loaded(SoundCommandRepeatMark(cpu.x));
}

/*
 * @routine 004E:4FA SoundCommandRepeat
 * @run 75:8CFA
 * WHAT IT DOES: Command $F7, met three times: the first goes on; the second keeps its place and
 * goes back to the mark, playing the section again; the third goes back to where the second was,
 * and the next pass goes on again.
 * HOW: SoundRepeatState 0, 1 and 2; SoundRepeatReturnLo and Hi.
 * Entry: X channel. Exit: X, Y, C and V kept. First, A 0, N and Z from the state 1. Second and
 * third, A the high byte of where it goes, N and Z from it. Past 2, A the state less 2, N and Z
 * from it.
 */
Repeat SoundCommandRepeat(uint8_t channel)
{
    Repeat repeat = { 0, 0 };

    repeat.state = rd8((uint16_t)(SoundRepeatState + channel));
    switch (repeat.state) {
    case REPEAT_MARKED:
        wr8((uint16_t)(SoundRepeatState + channel), REPEAT_FIRST_PASS);
        repeat.value = 0;
        break;
    case REPEAT_FIRST_PASS:
        wr8((uint16_t)(SoundRepeatState + channel), REPEAT_AGAIN);
        wr8((uint16_t)(SoundRepeatReturnLo + channel), rd8(zSoundSequenceLo));
        wr8((uint16_t)(SoundRepeatReturnHi + channel), rd8(zSoundSequenceHi));
        wr8(zSoundSequenceLo, rd8((uint16_t)(SoundRepeatMarkLo + channel)));
        repeat.value = rd8((uint16_t)(SoundRepeatMarkHi + channel));
        wr8(zSoundSequenceHi, repeat.value);
        break;
    case REPEAT_AGAIN:
        wr8((uint16_t)(SoundRepeatState + channel), REPEAT_FIRST_PASS);
        wr8(zSoundSequenceLo, rd8((uint16_t)(SoundRepeatReturnLo + channel)));
        repeat.value = rd8((uint16_t)(SoundRepeatReturnHi + channel));
        wr8(zSoundSequenceHi, repeat.value);
        break;
    default:
        repeat.value = (uint8_t)(repeat.state - REPEAT_AGAIN);
        break;
    }
    return repeat;
}

void abi_SoundCommandRepeat(void)
{
    Repeat repeat = SoundCommandRepeat(cpu.x);

    cpu.a = repeat.value;
    abi_nz(repeat.state == REPEAT_MARKED ? REPEAT_FIRST_PASS : repeat.value);
}

/*
 * @routine 004E:55B SoundCommandEnvelope
 * @run 75:8D5B
 * WHAT IT DOES: Command $FA: the channel's envelope: its attack from a preset, its decay and
 * release rates from the tens and units of the second argument, its sustain level.
 * HOW: SoundAttackPresets by the first; SoundRatePresets by the second's tens and units.
 * Entry: X channel. Exit: A the sustain level, N and Z from it, Y the units, C clear, V from the
 * last subtract of ten if there was one. X kept.
 */
Envelope SoundCommandEnvelope(uint8_t channel)
{
    Envelope envelope = { 0, 0, 0, false, false };
    uint8_t preset = (uint8_t)(argument(0) << 1);
    uint8_t rate = argument(1);

    wr8((uint16_t)(SoundAttackRate + channel), rd8((uint16_t)(SoundAttackPresets + preset)));
    wr8((uint16_t)(SoundAttackLevel + channel), rd8((uint16_t)(SoundAttackPresets + (uint8_t)(preset + 1))));
    while (rate >= RATE_TENS) {
        Sum less = sub_bytes(rate, RATE_TENS, true);

        envelope.subtracted = true;
        envelope.overflow = less.overflow;
        rate = less.value;
        envelope.tens++;
    }
    envelope.units = rate;
    wr8((uint16_t)(SoundDecayRate + channel), rd8((uint16_t)(SoundRatePresets + envelope.tens)));
    wr8((uint16_t)(SoundReleaseRate + channel), rd8((uint16_t)(SoundRatePresets + envelope.units)));
    envelope.sustain = set_from(SoundSustainLevel, channel, 2);
    return envelope;
}

void abi_SoundCommandEnvelope(void)
{
    Envelope envelope = SoundCommandEnvelope(cpu.x);

    cpu.y = envelope.units;
    abi_exit_loaded(envelope.sustain);
    abi_flag(P_C, false);
    if (envelope.subtracted)
        abi_flag(P_V, envelope.overflow);
}

/*
 * @routine 004E:60C SoundCommandRamp
 * @run 75:8E0C
 * WHAT IT DOES: Command $FB: the ramp's step.
 * HOW: SoundRampBy.
 * Entry: X channel. Exit: A the step, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandRamp(uint8_t channel)
{
    return set_from(SoundRampBy, channel, 0);
}

void abi_SoundCommandRamp(void)
{
    abi_exit_loaded(SoundCommandRamp(cpu.x));
}

/*
 * @routine 004E:613 SoundCommandNothing
 * @run 75:8E13
 * WHAT IT DOES: Commands $F3, $F4, $F5 and $FC: nothing.
 * HOW: Returns.
 * Entry: anything. Exit: all registers and flags kept.
 */
void SoundCommandNothing(void)
{
}

void abi_SoundCommandNothing(void)
{
    SoundCommandNothing();
}

/*
 * @routine 0053:745 SoundCommandJump
 * @run 76:B745
 * WHAT IT DOES: Command $FD: the sequence goes on at the address given.
 * HOW: The two arguments into zSoundSequenceLo and Hi.
 * Entry: X channel. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandJump(void)
{
    uint8_t high;

    wr8(zSoundSequenceLo, argument(0));
    high = argument(1);
    wr8(zSoundSequenceHi, high);
    return high;
}

void abi_SoundCommandJump(void)
{
    abi_exit_loaded(SoundCommandJump());
}

/*
 * @routine 0053:750 SoundCommandCall
 * @run 76:B750
 * WHAT IT DOES: Command $FE: plays the sequence at the address given, then comes back after the
 * command; one level only.
 * HOW: The place after the command into SoundReturnLo and Hi, then the address.
 * Entry: X channel. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
 */
uint8_t SoundCommandCall(uint8_t channel)
{
    wr8((uint16_t)(SoundReturnLo + channel), rd8(zSoundSequenceLo));
    wr8((uint16_t)(SoundReturnHi + channel), rd8(zSoundSequenceHi));
    return SoundCommandJump();
}

void abi_SoundCommandCall(void)
{
    abi_exit_loaded(SoundCommandCall(cpu.x));
}

/*
 * @routine 0053:765 SoundCommandReturn
 * @run 76:B765
 * WHAT IT DOES: Command $FF: back from a called sequence; with none to go back to, the end of the
 * channel's sequence: it is silenced, the main volume off while the sound is paused, the channel's
 * command cleared from 4 up, and from 6 up its priority too and the waveform given back to the
 * PSG channel it shared.
 * HOW: SoundReturnLo and Hi. The end drops the return into the command loop, so the loop's
 * caller gets C set; SoundSilenceChannel; SoundLoadWave with the shared channel's waveform.
 * Entry: X channel. Exit: back, A the high byte, N and Z from it, C clear. End, C set, and as the
 * last call left them; the return address dropped from the stack.
 */
SequenceEnd SoundCommandReturn(uint8_t channel)
{
    SequenceEnd end = { false, { false, 0 }, { WAVE_NONE, 0, 0 }, 0 };

    if ((rd8((uint16_t)(SoundReturnLo + channel)) | rd8((uint16_t)(SoundReturnHi + channel))) != 0) {
        wr8(zSoundSequenceLo, rd8((uint16_t)(SoundReturnLo + channel)));
        end.value = rd8((uint16_t)(SoundReturnHi + channel));
        wr8(zSoundSequenceHi, end.value);
        wr8((uint16_t)(SoundReturnLo + channel), 0);
        wr8((uint16_t)(SoundReturnHi + channel), 0);
        return end;
    }
    end.ended = true;
    end.release = SoundSilenceChannel(channel);
    end.value = rd8(SoundPaused);
    if (end.value != 0)
        wr8(PsgMainVolume, 0);
    if (channel >= EFFECT_SHARED_FIRST)
        wr8((uint16_t)(SoundChannelCommand + channel), 0);
    if (channel >= EFFECT_SHARED_END) {
        wr8((uint16_t)(SoundChannelPriority + channel), 0);
        end.wave = SoundLoadWave(rd8((uint16_t)(SoundChannelWave - EFFECT_CHANNEL_OFFSET + channel)), channel);
    }
    return end;
}

void abi_SoundCommandReturn(void)
{
    uint8_t channel = cpu.x;
    SequenceEnd end = SoundCommandReturn(channel);

    if (!end.ended) {
        abi_exit_loaded(end.value);
        abi_flag(P_C, false);
        return;
    }
    sequence_end_exit_shim(end, channel);
}

/* The end of a sequence: the loop's return dropped, C set, and the registers its last steps leave. */
void sequence_end_exit_shim(SequenceEnd end, uint8_t channel)
{
    cpu.s = (uint8_t)(cpu.s + 2);
    sequence_end_flags_shim(end, channel);
}

void sequence_end_flags_shim(SequenceEnd end, uint8_t channel)
{
    if (end.release.looked)
        abi_flag(P_V, (end.release.effect & 0x40) != 0);
    if (channel >= EFFECT_SHARED_END) {
        wave_load_exit_shim(end.wave, rd8((uint16_t)(SoundChannelWave - EFFECT_CHANNEL_OFFSET + channel)));
    } else {
        cpu.a = end.value;
        abi_nz((uint8_t)(channel - EFFECT_SHARED_END));
    }
    abi_flag(P_C, true);
}

/* A command from $E2 up, run by its handler; true when it ended the channel's sequence. */
static bool run_command(uint8_t command, uint8_t channel, SequenceEnd *end)
{
    switch (command) {
    case 0xE2: SoundCommandWave(channel); break;
    case 0xE3: SoundCommandPan(channel); break;
    case 0xE4: SoundCommandVibrato(channel); break;
    case 0xE5: SoundCommandModulation(channel); break;
    case 0xE6: SoundCommandLoopA(channel); break;
    case 0xE7: SoundCommandLoopAEnd(channel); break;
    case 0xE8: SoundCommandLoopB(channel); break;
    case 0xE9: SoundCommandLoopBEnd(channel); break;
    case 0xEA: SoundCommandTempo(channel); break;
    case 0xEB: SoundCommandTempoSlide(channel); break;
    case 0xEC: SoundCommandTranspose(channel); break;
    case 0xED: SoundCommandTremolo(channel); break;
    case 0xEE: SoundCommandVolume(channel); break;
    case 0xEF: SoundCommandVolumeSlide(channel); break;
    case 0xF0: SoundCommandGlideFromLast(channel); break;
    case 0xF1: SoundCommandGlideFromBelow(channel); break;
    case 0xF2: SoundCommandFineTune(channel); break;
    case 0xF6: SoundCommandRepeatMark(channel); break;
    case 0xF7: SoundCommandRepeat(channel); break;
    case 0xF8: SoundCommandPanSlide(channel); break;
    case 0xF9: SoundCommandVibratoGrow(channel); break;
    case 0xFA: SoundCommandEnvelope(channel); break;
    case 0xFB: SoundCommandRamp(channel); break;
    case 0xFD: SoundCommandJump(); break;
    case 0xFE: SoundCommandCall(channel); break;
    case 0xFF:
        *end = SoundCommandReturn(channel);
        return end->ended;
    default:
        SoundCommandNothing();
        break;
    }
    return false;
}

/*
 * @routine 0053:645 SoundReadSequence
 * @run 76:B645
 * WHAT IT DOES: Reads a channel's sequence on to its next note, rest or length: commands $E2 up
 * run their handlers, $60 to $6F set the gate notes take and $70 to $7F their volume, and the
 * channel's place is kept. Says whether the sequence ended.
 * HOW: SoundReadCommand; the handlers through SoundCommandHandlers, each pushed and returned to,
 * coming back to read again with X from zSoundChannel; $E0 SoundRest, $E1 SoundSetLength, a note
 * SoundStartNote with SoundGateSet and SoundVolumeDefault; SoundSaveSequence.
 * Entry: X channel. Exit: ended, C set, and as SoundCommandReturn left them on its way out.
 * Otherwise A the place's high byte, N and Z from it, C clear, X the channel, Y and V as the last
 * command left them.
 */
SequenceRead SoundReadSequence(uint8_t channel)
{
    SequenceRead read;

    read.channel = channel;
    for (;;) {
        uint8_t command;

        read.read = SoundReadCommand(read.channel);
        command = rd8(SoundCommandByte);
        read.command = command;
        if (command >= SEQUENCE_HANDLERS_FIRST) {
            uint16_t entry = (uint16_t)(SoundCommandHandlers + (uint8_t)((command - SEQUENCE_HANDLERS_FIRST) << 1));

            rd8((uint16_t)(entry + 1));
            rd8(entry);
            read.channel = rd8(zSoundChannel);
            if (run_command(command, read.channel, &read.end)) {
                read.how = SEQUENCE_ENDED;
                return read;
            }
            continue;
        }
        if (command == SEQUENCE_REST) {
            read.rest = SoundRest(read.channel);
            read.how = SEQUENCE_AT_REST;
            break;
        }
        if (command == SEQUENCE_LENGTH) {
            read.length = SoundSetLength(read.channel);
            read.how = SEQUENCE_AT_LENGTH;
            break;
        }
        if (command >= SEQUENCE_VOLUMES) {
            wr8((uint16_t)(SoundVolumeDefault + read.channel), (uint8_t)(command & 0x0F));
            continue;
        }
        if (command >= SEQUENCE_NOTES_END) {
            wr8((uint16_t)(SoundGateSet + read.channel), (uint8_t)(command & 0x0F));
            continue;
        }
        wr8((uint16_t)(SoundChannel2550 + read.channel), 0);
        wr8(SoundCommandArgs + 1, rd8((uint16_t)(SoundGateSet + read.channel)));
        wr8(SoundCommandArgs + 2, rd8((uint16_t)(SoundVolumeDefault + read.channel)));
        read.note = SoundStartNote(read.channel);
        read.how = SEQUENCE_AT_NOTE;
        break;
    }
    read.place_high = SoundSaveSequence(read.channel);
    return read;
}

void sequence_read_exit_shim(SequenceRead read)
{
    command_read_exit_shim(read.read);
    cpu.x = read.channel;
    switch (read.how) {
    case SEQUENCE_ENDED:
        sequence_end_flags_shim(read.end, read.channel);
        return;
    case SEQUENCE_AT_REST:
        abi_flag(P_C, read.channel >= EFFECT_SHARED_END);
        if (read.rest.release.looked)
            abi_flag(P_V, (read.rest.release.effect & 0x40) != 0);
        break;
    case SEQUENCE_AT_LENGTH:
        if (read.length.gate != 0) {
            cpu.y = read.length.length;
            if (read.length.product.added)
                abi_flag(P_V, read.length.product.overflow);
        }
        break;
    case SEQUENCE_AT_NOTE:
        note_start_exit_shim(read.note);
        break;
    }
    cpu.a = read.place_high;
    abi_nz(read.place_high);
    abi_flag(P_C, false);
}

void abi_SoundReadSequence(void)
{
    sequence_read_exit_shim(SoundReadSequence(cpu.x));
}

/*
 * @routine 004D:7F8 SoundGateStep
 * @run 75:87F8
 * WHAT IT DOES: A frame of the note's gate: while it sounds, its timer counts down; when it is
 * over, the ramp moves on while the volume less the slide is still at least the ramp, and the
 * channel is silenced once it is not, or at once with no ramp. Then a frame of the pitch.
 * HOW: SoundGateTimer; SoundRampBy onto SoundRampLo and Hi against SoundVolumeBase less
 * SoundSlideHi; SoundSilenceChannel; runs on into SoundPitchTick.
 * Entry: X channel. Exit: as SoundPitchTick.
 */
PitchTick SoundGateStep(uint8_t channel)
{
    uint8_t timer = rd8((uint16_t)(SoundGateTimer + channel));
    bool ramp = true;

    if (timer != 0) {
        timer = (uint8_t)(timer - 1);
        wr8((uint16_t)(SoundGateTimer + channel), timer);
        if (timer != 0)
            return SoundPitchTick(channel);
        ramp = rd8((uint16_t)(SoundRampBy + channel)) != 0;
    }
    if (ramp && (uint8_t)(rd8((uint16_t)(SoundVolumeBase + channel)) - rd8((uint16_t)(SoundSlideHi + channel))) >=
                    rd8((uint16_t)(SoundRampHi + channel))) {
        Sum low = add_bytes(rd8((uint16_t)(SoundRampLo + channel)), rd8((uint16_t)(SoundRampBy + channel)), false);

        wr8((uint16_t)(SoundRampLo + channel), low.value);
        wr8((uint16_t)(SoundRampHi + channel), (uint8_t)(rd8((uint16_t)(SoundRampHi + channel)) + low.carry));
    } else {
        SoundSilenceChannel(channel);
    }
    return SoundPitchTick(channel);
}

void abi_SoundGateStep(void)
{
    pitch_exit_shim(SoundGateStep(cpu.x), false);
}

/*
 * @routine 004E:129 SoundGlideLookahead
 * @run 75:8929
 * WHAT IT DOES: With no glide going, looks at the channel's next command: a $F3 is a glide to its
 * note, after its delay, over its frames, from the pitch now; it is taken and the glide aimed.
 * HOW: SoundPeekCommand; SoundSequenceFromWord takes it; SoundGlideDelay and SoundGlideFrames; the
 * note with the transpose, less one with a negative fine tune; SoundGlideSetup with the fine tune.
 * Entry: X channel. Exit: X kept. Gliding, A the frames, N and Z from them. Not $F3, as
 * SoundPeekCommand, with A the command, N, Z and C from comparing it with $F3. Taken, as
 * SoundGlideSetup, with A the fine tune, Y the note and C from adding the transpose where it
 * keeps them.
 */
Lookahead SoundGlideLookahead(uint8_t channel)
{
    Lookahead look = { LOOK_GLIDING, 0, { { LENGTH_ONE, 0, 0, 0 }, 0, 0 }, 0, 0, false, { false, false, { false, 0, 0, false, false, 0 }, 0, 0 } };
    Sum note;

    look.value = rd8((uint16_t)(SoundGlideFrames + channel));
    if (look.value != 0)
        return look;
    look.peek = SoundPeekCommand(channel);
    look.value = rd8(SoundCommandByte);
    look.how = LOOK_OTHER;
    if (look.value != SEQUENCE_GLIDE_TO)
        return look;
    look.how = LOOK_TAKEN;
    SoundSequenceFromWord();
    set_from(SoundGlideDelay, channel, 0);
    set_from(SoundGlideFrames, channel, 1);
    note = add_bytes(argument(2), rd8((uint16_t)(SoundTranspose + channel)), false);
    look.carry = note.carry;
    look.note = note.value;
    look.fine = rd8((uint16_t)(SoundFineTune + channel));
    if (look.fine & 0x80)
        look.note = (uint8_t)(look.note - 1);
    look.glide = SoundGlideSetup(look.fine, look.note, channel);
    return look;
}

void glide_lookahead_exit_shim(Lookahead look)
{
    if (look.how == LOOK_GLIDING) {
        abi_exit_loaded(look.value);
        return;
    }
    command_read_exit_shim(look.peek);
    if (look.how == LOOK_OTHER) {
        cpu.a = look.value;
        abi_nz((uint8_t)(look.value - SEQUENCE_GLIDE_TO));
        abi_flag(P_C, look.value >= SEQUENCE_GLIDE_TO);
        return;
    }
    cpu.a = look.fine;
    cpu.y = look.note;
    abi_flag(P_C, look.carry);
    glide_setup_exit_shim(look.glide);
}

void abi_SoundGlideLookahead(void)
{
    glide_lookahead_exit_shim(SoundGlideLookahead(cpu.x));
}

/*
 * @routine 004D:592 SoundChannelTick
 * @run 75:8592
 * WHAT IT DOES: A frame of one channel: its tempo is added to its phase, and a carry is a tick.
 * Between ticks, a playing note gets its volume and its pitch between ticks. On a tick, the note
 * counts down, and at its end the sequence is read on; the channel is switched off when its
 * sequence ends. A rest only runs its tempo, slide and pan; a note runs its gate, the tempo,
 * the glide's lookahead, the slide, the pan and the volume, and counts the tremolo's wait.
 * HOW: SoundTempo, or $FF when SoundFullTempo's bit 0 is set, onto SoundTempoPhase; SoundChannel2550
 * says a rest; SoundNoteLength; SoundReadSequence; the channel's bit in zSoundChannelBit taken
 * out of zSoundChannelsOn.
 * Entry: X channel. Exit: as the last call left them; a rest's tick or between ticks while
 * resting, A SoundChannel2550 with the tempo's add in C and V; switched off, A the channel's bit,
 * Z if no other bit is on, N and V the playing bits 7 and 6 before.
 */
ChannelTick SoundChannelTick(uint8_t channel)
{
    ChannelTick tick;
    uint8_t step = (rd8(SoundFullTempo) & 1) ? TEMPO_FULL : rd8((uint16_t)(SoundTempo + channel));

    tick.tempo = add_bytes(step, rd8((uint16_t)(SoundTempoPhase + channel)), false);
    wr8((uint16_t)(SoundTempoPhase + channel), tick.tempo.value);
    if (!tick.tempo.carry) {
        tick.rest = rd8((uint16_t)(SoundChannel2550 + channel));
        tick.how = CHANNEL_BETWEEN_RESTING;
        if (tick.rest != 0)
            return tick;
        tick.volume = SoundVolumeTick(channel);
        tick.pitch = SoundPitchTickPlain(channel);
        tick.how = CHANNEL_BETWEEN;
        return tick;
    }
    SoundLoadSequence(channel);
    tick.length = (uint8_t)(rd8((uint16_t)(SoundNoteLength + channel)) - 1);
    wr8((uint16_t)(SoundNoteLength + channel), tick.length);
    if (tick.length == 0) {
        tick.sequence = SoundReadSequence(channel);
        if (tick.sequence.how == SEQUENCE_ENDED) {
            tick.bit = rd8(zSoundChannelBit);
            tick.playing = rd8(zSoundChannelsOn);
            wr8(zSoundChannelsOn, (uint8_t)(tick.playing & ~tick.bit));
            tick.how = CHANNEL_ENDED;
            return tick;
        }
        tick.how = CHANNEL_READ;
    } else {
        tick.rest = rd8((uint16_t)(SoundChannel2550 + channel));
        if (tick.rest != 0) {
            tick.tempo_slide = SoundTempoSlideStep(channel);
            tick.slide = SoundSlideStep(channel);
            tick.pan = SoundPanSlideStep(channel);
            tick.how = CHANNEL_RESTING;
            return tick;
        }
        tick.gate = SoundGateStep(channel);
        tick.how = CHANNEL_GATED;
    }
    tick.tempo_slide = SoundTempoSlideStep(channel);
    tick.lookahead = SoundGlideLookahead(channel);
    tick.slide = SoundSlideStep(channel);
    tick.pan = SoundPanSlideStep(channel);
    tick.volume = SoundVolumeTick(channel);
    if (tick.volume.how == VOLUME_WAITING) {
        tick.wait = (uint8_t)(rd8((uint16_t)(SoundTremoloWait + channel)) + 1);
        wr8((uint16_t)(SoundTremoloWait + channel), tick.wait);
    }
    return tick;
}

void channel_tick_exit_shim(ChannelTick tick, uint8_t channel)
{
    abi_flag(P_C, tick.tempo.carry);
    abi_flag(P_V, tick.tempo.overflow);
    switch (tick.how) {
    case CHANNEL_BETWEEN_RESTING:
        abi_exit_loaded(tick.rest);
        return;
    case CHANNEL_BETWEEN:
        volume_tick_exit_shim(tick.volume, channel);
        pitch_exit_shim(tick.pitch, true);
        return;
    case CHANNEL_ENDED:
        sequence_read_exit_shim(tick.sequence);
        cpu.a = tick.bit;
        abi_flag(P_Z, (tick.playing & ~tick.bit) == 0);
        abi_flag(P_N, (tick.playing & 0x80) != 0);
        abi_flag(P_V, (tick.playing & 0x40) != 0);
        return;
    case CHANNEL_RESTING:
        abi_exit_loaded(tick.rest);
        tempo_slide_exit_shim(tick.tempo_slide);
        slide_exit_shim(tick.slide);
        slide_exit_shim(tick.pan);
        return;
    case CHANNEL_READ:
        sequence_read_exit_shim(tick.sequence);
        break;
    case CHANNEL_GATED:
        pitch_exit_shim(tick.gate, false);
        break;
    }
    tempo_slide_exit_shim(tick.tempo_slide);
    glide_lookahead_exit_shim(tick.lookahead);
    slide_exit_shim(tick.slide);
    slide_exit_shim(tick.pan);
    volume_tick_exit_shim(tick.volume, channel);
    if (tick.volume.how == VOLUME_WAITING)
        abi_nz(tick.wait);
}

void abi_SoundChannelTick(void)
{
    uint8_t channel = cpu.x;

    channel_tick_exit_shim(SoundChannelTick(channel), channel);
}

/*
 * @routine 004D:4EE SoundStartChannel
 * @run 75:84EE
 * WHAT IT DOES: Starts a channel from nothing: every effect off, its note due on the next tick, a
 * length of 1, the slide, envelope, loops and returns cleared, the volume base $1F, the pan 8,
 * the tempo 1 with its phase full so the first frame ticks.
 * HOW: Each field by channel cleared or set; SoundSetPan.
 * WAITS: no.
 * Entry: X channel. Exit: A 1, N and Z from it. X, Y, C and V kept.
 */
void SoundStartChannel(uint8_t channel)
{
    static const uint16_t cleared_first[] = {
        SoundModulationMaskLo, SoundGateTimer, SoundChannelDefaultArg, SoundGate, SoundLoopACount,
        SoundLoopBCount, SoundVolumeAddA, SoundVolumeAddB, SoundDetuneAFine, SoundDetuneANote,
        SoundDetuneBFine, SoundDetuneBNote, SoundSlideStepLo, SoundPanLo, SoundPanStepLo,
        SoundGlideFramesSet, SoundVibratoDepthLo, SoundVibratoDepthMax, SoundVibratoGrowHi,
        SoundVibratoGrowLo, SoundTremoloDepth, SoundChannelWave, SoundTranspose, SoundFineTune,
        SoundRampBy, SoundChannel2550, SoundModulationSpeed
    };
    static const uint16_t cleared_next[] = {
        SoundSlideHi, SoundSlideLo, SoundSlideStepLo, SoundSlideStepHi, SoundEnvelopeLevel,
        SoundAttackRate, SoundAttackLevel, SoundSustainLevel, SoundEnvelopePhase
    };
    static const uint16_t cleared_last[] = {
        SoundTempoSlideBy, SoundChannel25B0, SoundGateSet, SoundChannel25E0, SoundVolumeDefault,
        SoundReturnLo, SoundReturnHi
    };

    for (unsigned i = 0; i < sizeof cleared_first / sizeof cleared_first[0]; i++)
        wr8((uint16_t)(cleared_first[i] + channel), 0);
    wr8((uint16_t)(SoundChannel2430 + channel), 1);
    wr8((uint16_t)(SoundNoteLength + channel), 1);
    for (unsigned i = 0; i < sizeof cleared_next / sizeof cleared_next[0]; i++)
        wr8((uint16_t)(cleared_next[i] + channel), 0);
    wr8((uint16_t)(SoundVolumeBase + channel), CHANNEL_VOLUME_START);
    jsr_push(0x856C);
    SoundSetPan(channel, CHANNEL_PAN_START);
    jsr_pop();
    wr8((uint16_t)(SoundTempoPhase + channel), TEMPO_FULL);
    wr8((uint16_t)(SoundDecayRate + channel), 0);
    wr8((uint16_t)(SoundReleaseRate + channel), 0);
    wr8((uint16_t)(SoundTempo + channel), 1);
    for (unsigned i = 0; i < sizeof cleared_last / sizeof cleared_last[0]; i++)
        wr8((uint16_t)(cleared_last[i] + channel), 0);
}

void abi_SoundStartChannel(void)
{
    SoundStartChannel(cpu.x);
    abi_exit_loaded(1);
}

/*
 * @routine 0053:3B6 SoundStartMusic
 * @run 76:B3B6
 * @ends B4AF
 * WHAT IT DOES: Starts a piece of music, commands $55 to $61, on the six PSG channels: each takes
 * the command and its sequence from the piece's list and starts from nothing; no fade, the main
 * volume full.
 * HOW: zSoundChannelsOn gets the six channels' bits; SoundMusicTable by the command less $55 gives
 * the list, through zSoundTableLo and Hi; SoundReadPair and SoundStartChannel for each.
 * WAITS: no.
 * Entry: from SoundDriverCommand, SoundCommandIn the command. Exit, at the command's shared
 * end: A $FF, N set, Z clear, X 6, Y 1, C set, V from the table address's add.
 */
uint8_t SoundStartMusic(void)
{
    uint8_t command;
    uint16_t entry;

    wr8(zSoundChannelsOn, (uint8_t)(rd8(zSoundChannelsOn) | MUSIC_CHANNELS));
    command = rd8(SoundCommandIn);
    entry = (uint16_t)(SoundMusicTable + (uint8_t)((uint8_t)(command - MUSIC_FIRST) << 1));
    wr8(zSoundTableLo, (uint8_t)entry);
    wr8(zSoundTableHi, (uint8_t)(entry >> 8));
    wr8(zSoundWordLo, rd8_zp(zSoundTableLo));
    wr8(zSoundWordHi, rd8_zp_y(zSoundTableLo, 1));
    for (uint8_t channel = 0; channel < PSG_CHANNELS; channel++) {
        wr8((uint16_t)(SoundChannelCommand + channel), rd8(SoundCommandIn));
        jsr_push(0xB3DF);
        SoundReadPair(channel);
        jsr_pop();
        jsr_push(0xB3E2);
        SoundStartChannel(channel);
        jsr_pop();
    }
    wr8(SoundFading, 0);
    jsr_push(0xB3ED);
    SoundVolumeFull();
    jsr_pop();
    return command;
}

void music_start_exit_shim(uint8_t command)
{
    Sum low = add_bytes((uint8_t)((uint8_t)(command - MUSIC_FIRST) << 1), (uint8_t)SoundMusicTable, false);
    Sum high = add_bytes(0, (uint8_t)(SoundMusicTable >> 8), low.carry);

    cpu.a = PSG_VOLUME_FULL;
    cpu.x = PSG_CHANNELS;
    cpu.y = 1;
    abi_nz(PSG_VOLUME_FULL);
    abi_flag(P_C, true);
    abi_flag(P_V, high.overflow);
}

void abi_SoundStartMusic(void)
{
    music_start_exit_shim(SoundStartMusic());
}

/*
 * @routine 0053:4B5 SoundVoiceAddressBack
 * @run 76:B4B5
 * WHAT IT DOES: Moves a voice's ADPCM start back by 10, modulo the buffer's $4000: every voice of
 * the second set, and the first set's above $0E.
 * HOW: Bit 7 of SoundVoiceFlags is the set, SoundVoiceIndex the voice; $3FF6 added to zBiosBl and
 * zBiosBh.
 * Entry: nothing. Exit: not moved, A $0E, N, Z and C from comparing it with the voice, V clear.
 * Moved, A the high byte, N, Z, C and V from its add. X and Y kept.
 */
VoiceBack SoundVoiceAddressBack(void)
{
    VoiceBack back = { false, false, 0, { 0, false, false } };
    Sum low;

    if (!(rd8(SoundVoiceFlags) & VOICE_SECOND_SET)) {
        back.voice = rd8(SoundVoiceIndex);
        back.compared = true;
        if ((uint8_t)(VOICE_SET_END - VOICE_SET_FIRST) >= back.voice)
            return back;
    }
    back.moved = true;
    low = add_bytes((uint8_t)VOICE_BACK, rd8(zBiosBl), false);
    wr8(zBiosBl, low.value);
    back.high = add_bytes((uint8_t)(VOICE_BACK >> 8), rd8(zBiosBh), low.carry);
    wr8(zBiosBh, back.high.value);
    return back;
}

void voice_back_exit_shim(VoiceBack back)
{
    uint8_t limit = (uint8_t)(VOICE_SET_END - VOICE_SET_FIRST);

    if (!back.moved) {
        cpu.a = limit;
        abi_nz((uint8_t)(limit - back.voice));
        abi_flag(P_C, limit >= back.voice);
        abi_flag(P_V, false);
        return;
    }
    cpu.a = back.high.value;
    abi_nz(back.high.value);
    abi_flag(P_C, back.high.carry);
    abi_flag(P_V, back.high.overflow);
}

void abi_SoundVoiceAddressBack(void)
{
    voice_back_exit_shim(SoundVoiceAddressBack());
}

/* Whether the play started, which is when it leaves A 0. */
static bool kernel_play_started(KernelPlay play)
{
    if (play.how == KERNEL_PLAY_CARD)
        return play.card.how == AD_PLAY_STARTED || play.card.how == AD_PLAY_AGAIN;
    return play.how == KERNEL_PLAY_STARTED;
}

/*
 * @routine 0053:3F0 SoundStartVoice
 * @run 76:B3F0
 * @ends B4AF
 * WHAT IT DOES: Starts an ADPCM voice, commands $62 up: the second set from $72 on stages $0C
 * up, else the first. One of lower priority than the voice playing is dropped unless one is
 * waiting; otherwise the playing voice is stopped and this one played, or left waiting while the
 * unit is still busy. The second character's voices start ten bytes back.
 * HOW: ad_stat; SoundVoicePriority, cleared when nothing plays or waits; SoundVoicesFirst or
 * SoundVoicesSecond by the command, six bytes a voice, through zSoundTableLo and Hi; ad_stop,
 * ad_stat; the voice into zBiosBl to zBiosDh; SoundVoiceAddressBack for character 1;
 * KernelAdpcmPlay. Bit 0 of SoundVoiceFlags marks a voice waiting.
 * WAITS: no.
 * Entry: from SoundDriverCommand, SoundCommandIn the command. Exit, at the command's shared end,
 * as the last of those calls left them, then: dropped, A 0, Z set, N and C clear; waiting, A the
 * flags with bit 0, N and Z from them, C set; played, Z and C set, N clear.
 */
VoiceStart SoundStartVoice(void)
{
    VoiceStart voice;
    uint16_t table;

    jsr_push(0xB3F3);
    voice.stat_before = ad_stat();
    jsr_pop();
    if (voice.stat_before.playing == 0 && !(rd8(SoundVoiceFlags) & VOICE_WAITING))
        wr8(SoundVoicePriority, 0);
    voice.command = rd8(SoundCommandIn);
    voice.second = !((uint8_t)(voice.command - VOICE_SECOND_FIRST) & 0x80) &&
                   !((uint8_t)((rd8(zStage) & 0x7F) - VOICE_SECOND_STAGES) & 0x80);
    if (voice.second) {
        wr8(SoundVoiceFlags, (uint8_t)(rd8(SoundVoiceFlags) | VOICE_SECOND_SET));
        voice.index = (uint8_t)(rd8(SoundCommandIn) - VOICE_SECOND_FIRST);
        table = SoundVoicesSecond;
    } else {
        wr8(SoundVoiceFlags, (uint8_t)(rd8(SoundVoiceFlags) & ~VOICE_SECOND_SET));
        voice.index = (uint8_t)(rd8(SoundCommandIn) - VOICE_SET_FIRST);
        table = SoundVoicesFirst;
    }
    wr8(SoundVoiceIndex, voice.index);
    jsr_push(voice.second ? 0xB44B : 0xB426);
    voice.product = SoundMultiply(voice.index, VOICE_ENTRY, voice.stat_before.busy);
    jsr_pop();
    voice.table_low = add_bytes(rd8(zSoundWordLo), (uint8_t)table, false);
    wr8(zSoundTableLo, voice.table_low.value);
    voice.table_high = add_bytes(rd8(zSoundWordHi), (uint8_t)(table >> 8), voice.table_low.carry);
    wr8(zSoundTableHi, voice.table_high.value);
    voice.priority = rd8_zp(zSoundTableLo);
    voice.playing_priority = rd8(SoundVoicePriority);
    if (voice.priority < voice.playing_priority) {
        if (rd8(SoundVoiceFlags) == 0) {
            voice.how = VOICE_DROPPED;
            return voice;
        }
        voice.priority = rd8(SoundVoicePriority);
    }
    wr8(SoundVoicePriority, voice.priority);
    jsr_push(0xB46D);
    voice.stop = ad_stop();
    jsr_pop();
    jsr_push(0xB470);
    voice.stat = ad_stat();
    jsr_pop();
    if (voice.stat.playing != 0) {
        voice.flags = (uint8_t)(rd8(SoundVoiceFlags) | VOICE_WAITING);
        wr8(SoundVoiceFlags, voice.flags);
        voice.how = VOICE_WAITING_BUSY;
        return voice;
    }
    wr8(zBiosBl, rd8_zp_y(zSoundTableLo, 1));
    wr8(zBiosBh, rd8_zp_y(zSoundTableLo, 2));
    wr8(zBiosAl, rd8_zp_y(zSoundTableLo, 3));
    wr8(zBiosAh, rd8_zp_y(zSoundTableLo, 4));
    wr8(zBiosDh, rd8_zp_y(zSoundTableLo, 5));
    wr8(zBiosDl, 0);
    voice.moved_back = (rd8(zCharacter) & 1) != 0;
    if (voice.moved_back) {
        jsr_push(0xB4A3);
        voice.back = SoundVoiceAddressBack();
        jsr_pop();
    }
    jsr_push(0xB4A6);
    voice.play = KernelAdpcmPlay();
    jsr_pop();
    if (!kernel_play_started(voice.play)) {
        voice.flags = (uint8_t)(rd8(SoundVoiceFlags) | VOICE_WAITING);
        wr8(SoundVoiceFlags, voice.flags);
        voice.how = VOICE_WAITING_PLAY;
        return voice;
    }
    wr8(SoundVoiceFlags, 0);
    voice.how = VOICE_PLAYED;
    return voice;
}

void voice_start_exit_shim(VoiceStart voice)
{
    ad_stat_exit_shim(voice.stat_before);
    cpu.y = VOICE_ENTRY;
    abi_exit_product(voice.product, voice.index);
    cpu.a = voice.table_high.value;
    abi_nz(voice.table_high.value);
    abi_flag(P_C, voice.table_high.carry);
    abi_flag(P_V, voice.table_high.overflow);
    if (voice.how == VOICE_DROPPED) {
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);
        return;
    }
    ad_stop_exit_shim(voice.stop);
    ad_stat_exit_shim(voice.stat);
    if (voice.how == VOICE_WAITING_BUSY) {
        cpu.a = voice.flags;
        abi_nz(voice.flags);
        abi_flag(P_C, true);
        return;
    }
    cpu.y = VOICE_ENTRY - 1;
    if (voice.moved_back)
        voice_back_exit_shim(voice.back);
    kernel_play_exit_shim(voice.play);
    abi_flag(P_C, true);
    if (voice.how == VOICE_WAITING_PLAY) {
        cpu.a = voice.flags;
        abi_nz(voice.flags);
        return;
    }
    abi_nz(cpu.a);
}

void abi_SoundStartVoice(void)
{
    voice_start_exit_shim(SoundStartVoice());
}

/* An effect's entry: its priority, with zSoundTableLo and Hi at it. */
static EffectEntry effect_entry(uint16_t ret, uint8_t channel, uint8_t held_y)
{
    EffectEntry entry;
    uint8_t command;

    push8(held_y);
    command = rd8(SoundCommandIn);
    jsr_push(ret);
    entry.product = SoundMultiply(command, EFFECT_ENTRY, channel);
    jsr_pop();
    (void)pull8();
    entry.low = add_bytes(rd8(zSoundWordLo), (uint8_t)SoundEffectTable, false);
    wr8(zSoundTableLo, entry.low.value);
    entry.high = add_bytes(rd8(zSoundWordHi), (uint8_t)(SoundEffectTable >> 8), entry.low.carry);
    wr8(zSoundTableHi, entry.high.value);
    entry.priority = rd8_zp(zSoundTableLo);
    return entry;
}

/* An effect's sequence started on a channel from its entry. */
static void start_effect_channel(uint8_t channel, uint16_t ret_read, uint16_t ret_start)
{
    jsr_push(ret_read);
    SoundReadPair(channel);
    jsr_pop();
    jsr_push(ret_start);
    SoundStartChannel(channel);
    jsr_pop();
}

/* An effect on one channel: 7 when it is free or plays this one, 6 when 6 does, else by weight. */
static DriverHow start_effect(uint8_t command, DriverCommand *taken)
{
    uint8_t held_y;
    uint8_t channel = EFFECT_CHANNEL_B;
    uint8_t asked, playing, again, other, weight_a, weight_b;

    (void)command;
    asked = rd8(SoundCommandIn);
    playing = rd8(SoundChannelCommand + EFFECT_CHANNEL_B);     /* TST */
    if (playing != 0) {
        again = rd8(SoundChannelCommand + EFFECT_CHANNEL_B);   /* CMP */
        if (asked != again) {
            other = rd8(SoundChannelCommand + EFFECT_CHANNEL_A);
            if (asked == other) {
                channel = EFFECT_CHANNEL_A;
            } else {
                weight_a = rd8(SoundChannel25F6);
                weight_b = rd8(SoundChannel25F7);
                if (weight_a < weight_b) {
                    channel = EFFECT_CHANNEL_A;
                }
            }
        }
    }
    held_y = channel == EFFECT_CHANNEL_B ? 0x80 : 0x40;
    taken->entry = effect_entry(0xB31C, channel, held_y);
    if (taken->entry.priority < rd8((uint16_t)(SoundChannelPriority + channel)))
        return DRIVER_EFFECT_LOW;
    wr8((uint16_t)(SoundChannelPriority + channel), taken->entry.priority);
    taken->channels_before = rd8(zSoundChannelsOn);
    wr8(zSoundChannelsOn, (uint8_t)(taken->channels_before | (channel == EFFECT_CHANNEL_B ? 0x80 : 0x40)));
    wr8((uint16_t)(SoundChannelCommand + channel), rd8(SoundCommandIn));
    wr8(zSoundWordLo, rd8_zp_y(zSoundTableLo, 1));
    wr8(zSoundWordHi, rd8_zp_y(zSoundTableLo, 2));
    start_effect_channel(channel, 0xB34E, 0xB351);
    jsr_push(0xB354);
    SoundVolumeFull();
    jsr_pop();
    return DRIVER_EFFECT;
}

/* An effect on both channels, from $3D: its priority weighed against the lighter channel's. */
static DriverHow start_effect_pair(uint8_t command, DriverCommand *taken, uint8_t kept_y)
{
    uint8_t weight_a = rd8(SoundChannel25F6);
    uint8_t weight_b = rd8(SoundChannel25F7);
    uint8_t channel = weight_a >= weight_b ? EFFECT_CHANNEL_A : EFFECT_CHANNEL_B;
    uint8_t played;

    (void)command;
    taken->entry = effect_entry(0xB36B, channel, kept_y);
    if (taken->entry.priority < rd8((uint16_t)(SoundChannelPriority + channel)))
        return DRIVER_EFFECT_LOW;
    wr8(SoundChannelPriority + EFFECT_CHANNEL_A, taken->entry.priority);
    wr8(SoundChannelPriority + EFFECT_CHANNEL_B, taken->entry.priority);
    taken->channels_before = rd8(zSoundChannelsOn);
    wr8(zSoundChannelsOn, (uint8_t)(taken->channels_before | EFFECT_CHANNEL_BITS));
    played = rd8(SoundCommandIn);
    wr8(SoundChannelCommand + EFFECT_CHANNEL_A, played);
    wr8(SoundChannelCommand + EFFECT_CHANNEL_B, played);
    wr8(zSoundWordLo, rd8_zp_y(zSoundTableLo, 1));
    wr8(zSoundWordHi, rd8_zp_y(zSoundTableLo, 2));
    start_effect_channel(EFFECT_CHANNEL_A, 0xB3A6, 0xB3A9);
    start_effect_channel(EFFECT_CHANNEL_B, 0xB3AD, 0xB3B0);
    jsr_push(0xB3B3);
    SoundVolumeFull();
    jsr_pop();
    return DRIVER_EFFECT;
}

/*
 * @routine 0053:284 SoundDriverCommand
 * @run 76:B284
 * WHAT IT DOES: Takes a command: $FF stops everything, $FE fades the music, $FD and $FC choose
 * the balance tables, $55 to $61 start music and $62 up a voice; below $55 an effect starts, on
 * channel 6 or 7 or, from $3D, on both, if its priority is at least what plays there. Nothing
 * starts while music $60 plays.
 * HOW: SoundInCommand set while it works; SoundCommandIn keeps the command; SoundEffectTable, three
 * bytes an effect, through zSoundTableLo and Hi; SoundChannelPriority, SoundChannelCommand and
 * zSoundChannelsOn by channel; SoundReadPair, SoundStartChannel, SoundVolumeFull.
 * WAITS: no.
 * Entry: A command. Exit: X and Y kept, N and Z from X; A, C and V as the command's path left them.
 */
DriverCommand SoundDriverCommand(uint8_t command, uint8_t kept_x, uint8_t kept_y)
{
    DriverCommand taken;

    push8(kept_x);
    push8(kept_y);
    wr8(SoundInCommand, 1);
    taken.command = command;
    switch (command) {
    case COMMAND_STOP:
        jsr_push(0xB292);
        SoundDriverStopAll();
        jsr_pop();
        taken.how = DRIVER_STOPPED;
        break;
    case COMMAND_FADE:
        jsr_push(0xB29C);
        taken.fade = SoundStartFade();
        jsr_pop();
        taken.how = DRIVER_FADING;
        break;
    case COMMAND_ALT_BALANCE:
        wr8(zSoundAltBalance, 1);
        taken.how = DRIVER_BALANCE_ON;
        break;
    case COMMAND_FIRST_BALANCE:
        wr8(zSoundAltBalance, 0);
        taken.how = DRIVER_BALANCE_OFF;
        break;
    case COMMAND_MUSIC_LAST:
        wr8(SoundCommandIn, command);
        taken.music = SoundStartMusic();
        taken.how = DRIVER_MUSIC;
        break;
    default:
        push8(command);
        if (rd8(SoundCommandIn) == COMMAND_EXCLUSIVE && (rd8(zSoundChannelsOn) & 1)) {
            (void)pull8();
            taken.how = DRIVER_HELD;
            break;
        }
        command = pull8();
        wr8(SoundCommandIn, command);
        if (command >= COMMAND_VOICES) {
            taken.voice = SoundStartVoice();
            taken.how = DRIVER_VOICE;
        } else if (command >= MUSIC_FIRST) {
            taken.music = SoundStartMusic();
            taken.how = DRIVER_MUSIC;
        } else if (command >= COMMAND_PAIRED) {
            taken.how = start_effect_pair(command, &taken, kept_y);
        } else {
            taken.how = start_effect(command, &taken);
        }
        break;
    }
    kept_y = pull8();
    kept_x = pull8();
    wr8(SoundInCommand, 0);
    return taken;
}

void driver_command_exit_shim(DriverCommand taken)
{
    uint8_t x = cpu.x;
    uint8_t y = cpu.y;

    switch (taken.how) {
    case DRIVER_STOPPED:
    case DRIVER_BALANCE_OFF:
    case DRIVER_HELD:
        cpu.a = taken.command;
        abi_flag(P_C, true);
        break;
    case DRIVER_FADING:
        cpu.a = taken.fade;
        abi_flag(P_C, true);
        break;
    case DRIVER_BALANCE_ON:
        cpu.a = 1;
        abi_flag(P_C, true);
        break;
    case DRIVER_MUSIC:
        music_start_exit_shim(taken.music);
        break;
    case DRIVER_VOICE:
        voice_start_exit_shim(taken.voice);
        break;
    case DRIVER_EFFECT_LOW:
        cpu.a = taken.entry.priority;
        abi_flag(P_C, false);
        abi_flag(P_V, taken.entry.high.overflow);
        break;
    case DRIVER_EFFECT:
        cpu.a = PSG_VOLUME_FULL;
        abi_flag(P_C, true);
        abi_flag(P_V, (taken.channels_before & 0x40) != 0);
        break;
    }
    cpu.x = x;
    cpu.y = y;
    abi_nz(x);
}

void abi_SoundDriverCommand(void)
{
    driver_command_exit_shim(SoundDriverCommand(cpu.a, cpu.x, cpu.y));
}

/* A channel shared with an effect given back to its music: selected, its effect off, its wave. */
static WaveLoad give_back_channel(uint8_t channel, uint8_t bit, uint16_t weight, uint16_t ret)
{
    WaveLoad loaded;

    wr8(PsgSelect, channel);
    wr8(zSoundChannel, channel);
    wr8((uint16_t)(SoundChannelCommand + channel + EFFECT_CHANNEL_OFFSET), 0);
    wr8(zSoundChannelsOn, (uint8_t)(rd8(zSoundChannelsOn) & ~bit));
    wr8(weight, 0);
    jsr_push(ret);
    loaded = SoundLoadWave(rd8((uint16_t)(SoundChannelWave + channel)), channel);
    jsr_pop();
    return loaded;
}

/*
 * @routine 0053:4D4 SoundCheckPause
 * @run 76:B4D4
 * WHAT IT DOES: Follows the game's pause: on pausing, the PSG silenced, ADPCM stopped and effect
 * $53 played; on going on, the two effect channels let go and PSG channels 4 and 5 given their
 * music's waveforms back, the main volume full.
 * HOW: Bit 0 of zPaused against SoundPaused; SoundSilenceAll, ad_stop, SoundDriverCommand; or
 * SoundLoadWave on 4 and 5 and SoundVolumeFull.
 * WAITS: no.
 * Entry: nothing. Exit: unchanged, A the bit, Z and C set, N clear. Paused, as SoundDriverCommand.
 * Going on, as SoundVolumeFull after SoundLoadWave on channel 5, X 5. Y kept.
 */
PauseCheck SoundCheckPause(uint8_t kept_x, uint8_t kept_y)
{
    PauseCheck check;

    check.bit = (uint8_t)(rd8(zPaused) & 1);
    if (check.bit == rd8(SoundPaused)) {
        check.how = PAUSE_SAME;
        return check;
    }
    wr8(SoundPaused, check.bit);
    if (check.bit != 0) {
        jsr_push(0xB4E7);
        SoundSilenceAll(kept_x);
        jsr_pop();
        jsr_push(0xB4EA);
        check.stop = ad_stop();
        jsr_pop();
        jsr_push(0xB4EF);
        check.command = SoundDriverCommand(PAUSE_EFFECT, kept_x, kept_y);
        jsr_pop();
        check.how = PAUSE_ON;
        return check;
    }
    check.four = give_back_channel(EFFECT_SHARED_FIRST, 0x40, SoundChannel25F6, 0xB507);
    check.wave_five = rd8((uint16_t)(SoundChannelWave + EFFECT_SHARED_FIRST + 1));
    check.on_before_five = rd8(zSoundChannelsOn);
    check.five = give_back_channel(EFFECT_SHARED_FIRST + 1, 0x80, SoundChannel25F7, 0xB51E);
    SoundVolumeFull();
    check.how = PAUSE_OFF;
    return check;
}

void pause_check_exit_shim(PauseCheck check)
{
    switch (check.how) {
    case PAUSE_SAME:
        cpu.a = check.bit;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case PAUSE_ON:
        ad_stop_exit_shim(check.stop);
        cpu.a = PAUSE_EFFECT;
        driver_command_exit_shim(check.command);
        break;
    case PAUSE_OFF:
        cpu.x = EFFECT_SHARED_FIRST + 1;
        abi_flag(P_V, (check.on_before_five & 0x40) != 0);
        wave_load_exit_shim(check.five, check.wave_five);
        cpu.a = PSG_VOLUME_FULL;
        abi_nz(PSG_VOLUME_FULL);
        break;
    }
}

void abi_SoundCheckPause(void)
{
    pause_check_exit_shim(SoundCheckPause(cpu.x, cpu.y));
}

/* The waiting voice played, if the unit is free. */
static void play_waiting_voice(DriverTick *tick)
{
    uint16_t table = (rd8(SoundVoiceFlags) & VOICE_SECOND_SET) ? SoundVoicesSecond : SoundVoicesFirst;

    jsr_push((rd8(SoundVoiceFlags) & VOICE_SECOND_SET) ? 0xB58E : 0xB577);
    tick->product = SoundMultiply(rd8(SoundVoiceIndex), VOICE_ENTRY, tick->stat_again.busy);
    jsr_pop();
    tick->table_low = add_bytes(rd8(zSoundWordLo), (uint8_t)table, false);
    wr8(zSoundTableLo, tick->table_low.value);
    tick->table_high = add_bytes(rd8(zSoundWordHi), (uint8_t)(table >> 8), tick->table_low.carry);
    wr8(zSoundTableHi, tick->table_high.value);
    wr8(zBiosBl, rd8_zp_y(zSoundTableLo, 1));
    wr8(zBiosBh, rd8_zp_y(zSoundTableLo, 2));
    wr8(zBiosAl, rd8_zp_y(zSoundTableLo, 3));
    wr8(zBiosAh, rd8_zp_y(zSoundTableLo, 4));
    wr8(zBiosDh, rd8_zp_y(zSoundTableLo, 5));
    wr8(zBiosDl, 0);
    tick->moved_back = (rd8(zCharacter) & 1) != 0;
    if (tick->moved_back)
    {
        jsr_push(0xB5C0);
        tick->back = SoundVoiceAddressBack();
        jsr_pop();
    }
    jsr_push(0xB5C3);
    tick->play = KernelAdpcmPlay();
    jsr_pop();
    tick->played = kernel_play_started(tick->play);
    if (tick->played)
        wr8(SoundVoiceFlags, 0);
}

/* The music's fade a step on; at its end the six PSG channels silenced and let go. */
static void fade_step(DriverTick *tick)
{
    uint8_t step;

    if (rd8(SoundFading) == 0)
        return;
    step = (uint8_t)(rd8(SoundFadeStep) - 1);
    wr8(SoundFadeStep, step);
    if (step != 0)
        return;
    wr8(SoundFadeStep, FADE_STEP_START);
    wr8(SoundFading, (uint8_t)(rd8(SoundFading) + 1));
    if (rd8(SoundFading) != FADE_END)
        return;
    wr8(SoundFading, 0);
    for (int channel = PSG_CHANNELS - 1; channel >= 0; channel--) {
        wr8(zSoundChannel, (uint8_t)channel);
        wr8(PsgSelect, (uint8_t)channel);
        jsr_push(0xB5F3);
        tick->faded[channel] = SoundSilenceChannel((uint8_t)channel);
        jsr_pop();
    }
    tick->fade_ended = true;
    tick->on_before_fade = rd8(zSoundChannelsOn);
    wr8(zSoundChannelsOn, (uint8_t)(tick->on_before_fade & ~MUSIC_CHANNELS));
}

/* Each playing channel's frame, from the first given, its PSG channel selected first. */
static void run_channels(DriverTick *tick, uint8_t first, uint8_t bit)
{
    wr8(zSoundChannelBit, first == 0 ? 0 : bit);
    for (uint8_t channel = first; channel < SOUND_CHANNELS; channel++) {
        if (first == 0)
            rol_mem(zSoundChannelBit, channel == 0);
        else if (channel != first)
            rol_mem(zSoundChannelBit, false);
        wr8(zSoundChannel, channel);
        tick->ticked[channel] = (rd8(zSoundChannelBit) & rd8(zSoundChannelsOn)) != 0;
        if (!tick->ticked[channel])
            continue;
        wr8(PsgSelect, channel >= EFFECT_SHARED_END ? (uint8_t)(channel - EFFECT_CHANNEL_OFFSET) : channel);
        jsr_push(0xB61B);
        tick->channel[channel] = SoundChannelTick(channel);
        jsr_pop();
    }
    if (rd8(zSoundChannelsOn) == 0)
        wr8(PsgMainVolume, 0);
    wr8(SoundInCommand, 0);
}

/*
 * @routine 0053:521 SoundDriverTick
 * @run 76:B521
 * WHAT IT DOES: One frame of the driver, unless a command is going in: the pause followed; while
 * paused, ADPCM stopped and only the two effect channels run; otherwise a voice waiting played
 * once the unit is free, the music's fade a step on, and every playing channel's frame. With
 * nothing playing, the main volume off.
 * HOW: SoundInCommand set while it works; SoundCheckPause; ad_stat and ad_stop; the voice waiting
 * from its set's table, SoundVoiceAddressBack and KernelAdpcmPlay; SoundFading and SoundFadeStep,
 * $14 its end; zSoundChannelBit walks zSoundChannelsOn; channels 6 and 7 select PSG channels 4 and
 * 5; SoundChannelTick.
 * WAITS: no.
 * Entry: frame interrupt. Exit: A, X and Y kept, N and Z from A; C set, and C and V kept if a
 * command was going in; otherwise V as the last call left it.
 */
DriverTick SoundDriverTick(uint8_t kept_a, uint8_t kept_x, uint8_t kept_y)
{
    DriverTick tick;

    push8(kept_a);
    push8(kept_x);
    push8(kept_y);
    tick.ran = rd8(SoundInCommand) == 0;
    if (!tick.ran) {
        (void)pull8();
        (void)pull8();
        (void)pull8();
        return tick;
    }
    wr8(SoundInCommand, (uint8_t)(rd8(SoundInCommand) + 1));
    jsr_push(0xB532);
    tick.pause = SoundCheckPause(kept_x, kept_y);
    jsr_pop();
    tick.paused = rd8(SoundPaused) != 0;
    tick.fade_ended = false;
    tick.voice_waited = false;
    tick.stop_ran = false;
    jsr_push(tick.paused ? 0xB53A : 0xB54D);
    tick.stat = ad_stat();
    jsr_pop();
    if (tick.paused) {
        if (tick.stat.playing != 0) {
            jsr_push(0xB541);
            tick.stop = ad_stop();
            jsr_pop();
            tick.stop_ran = true;
        }
        run_channels(&tick, EFFECT_CHANNEL_A, 0x40);
        (void)pull8();
        (void)pull8();
        (void)pull8();
        return tick;
    }
    if (tick.stat.playing == 0 && (rd8(SoundVoiceFlags) & VOICE_WAITING)) {
        jsr_push(0xB55E);
        tick.stop = ad_stop();
        jsr_pop();
        tick.stop_ran = true;
        jsr_push(0xB561);
        tick.stat_again = ad_stat();
        jsr_pop();
        if (tick.stat_again.playing == 0) {
            tick.voice_waited = true;
            play_waiting_voice(&tick);
        }
    }
    fade_step(&tick);
    run_channels(&tick, 0, 1);
    (void)pull8();
    (void)pull8();
    (void)pull8();
    return tick;
}

void driver_tick_exit_shim(DriverTick tick)
{
    uint8_t a = cpu.a;
    uint8_t x = cpu.x;
    uint8_t y = cpu.y;
    uint8_t first;

    if (tick.ran) {
        pause_check_exit_shim(tick.pause);
        ad_stat_exit_shim(tick.stat);
        if (tick.stop_ran)
            ad_stop_exit_shim(tick.stop);
        if (!tick.paused && tick.stop_ran)
            ad_stat_exit_shim(tick.stat_again);
        if (tick.voice_waited) {
            cpu.y = VOICE_ENTRY;
            abi_exit_product(tick.product, rd8(SoundVoiceIndex));
            abi_flag(P_V, tick.table_high.overflow);
            if (tick.moved_back)
                voice_back_exit_shim(tick.back);
            kernel_play_exit_shim(tick.play);
        }
        if (tick.fade_ended)
            for (int channel = PSG_CHANNELS - 1; channel >= 0; channel--) {
                cpu.x = (uint8_t)channel;
                if (tick.faded[channel].looked)
                    abi_flag(P_V, (tick.faded[channel].effect & 0x40) != 0);
            }
        if (tick.fade_ended)
            abi_flag(P_V, (tick.on_before_fade & 0x40) != 0);
        first = tick.paused ? EFFECT_CHANNEL_A : 0;
        for (uint8_t channel = first; channel < SOUND_CHANNELS; channel++)
            if (tick.ticked[channel]) {
                cpu.x = channel;
                channel_tick_exit_shim(tick.channel[channel], channel);
            }
        abi_flag(P_C, true);
    }
    cpu.a = a;
    cpu.x = x;
    cpu.y = y;
    abi_nz(a);
}

void abi_SoundDriverTick(void)
{
    driver_tick_exit_shim(SoundDriverTick(cpu.a, cpu.x, cpu.y));
}
