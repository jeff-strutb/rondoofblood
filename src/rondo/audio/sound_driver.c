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
#include "routines.h"

enum {
    PSG_CHANNELS = 6, PSG_VOLUME_FULL = 0xFF, LEVEL_SILENT = 0x1F,
    EFFECT_SHARED_FIRST = 4, EFFECT_SHARED_END = 6, EFFECT_CHANNEL_OFFSET = 2,
    FADE_STEP_START = 0x10, SOUND_WAVES = 0xBD11, SEQUENCE_NOTES_END = 0x60, SEQUENCE_REUSE = 0x80,
    SEQUENCE_LONG_FIRST = 0xE0, OFFSET_NEGATIVE = 0x40, PSG_CHANNEL_ON = 0x80, BALANCE_SET_SIZE = 0x10,
    PSG_VOLUME_LIMIT = 0x20
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
 * Entry: nothing. Exit: A, X, Y, C and V kept; N and Z from X. zSoundChannel 0.
 */
void SoundSilenceAll(void)
{
    for (int channel = PSG_CHANNELS - 1; channel >= 0; channel--) {
        wr8(zSoundChannel, (uint8_t)channel);
        wr8(PsgSelect, (uint8_t)channel);
        wr8(PsgControl, 0);
    }
    wr8(PsgMainVolume, 0);
}

void abi_SoundSilenceAll(void)
{
    SoundSilenceAll();
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
    wr8(zSoundMultiplier, 0);
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
 * HOW: Eight steps of shift and add, the multiplier from zSoundMultiplier.
 * Entry: A and Y the bytes. Exit: X and Y kept, N and Z from X; C clear; A the product's high
 * byte as of the last add, V from that add, or A and V kept when A was 0.
 */
SoundProduct SoundMultiply(uint8_t a, uint8_t y)
{
    return shift_add(a, 8, y);
}

void abi_SoundMultiply(void)
{
    abi_exit_product(SoundMultiply(cpu.a, cpu.y), cpu.a);
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
static void abi_exit_slide(SlideStep slide)
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
    abi_exit_slide(SoundSlideStep(cpu.x));
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
    abi_exit_slide(SoundPanSlideStep(cpu.x));
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

void abi_SoundWaveAddress(void)
{
    uint8_t page = (uint8_t)(cpu.a >> 3);
    uint8_t high = (uint8_t)(SoundWaveAddress(cpu.a) >> 8);
    bool carry = ((cpu.a & 0x07) << 5) + (SOUND_WAVES & 0xFF) > 0xFF;

    cpu.a = high;
    abi_nz(high);
    abi_flag(P_C, page + (SOUND_WAVES >> 8) + carry > 0xFF);
    abi_flag(P_V, (~(page ^ (SOUND_WAVES >> 8)) & (page ^ high) & 0x80) != 0);
}

/*
 * @routine 0053:6F1 SoundCommandLength
 * @run 76:B6F1
 * WHAT IT DOES: How many argument bytes the command in SoundCommandByte takes. Below $60 one,
 * unless its bit 7 says to reuse the channel's default, which goes into SoundCommandArgs with none
 * read; $60 up none, except the commands from $E0 up, whose lengths are a table.
 * HOW: A with bit 7 off picks the class; SoundChannelDefaultArg; SoundCommandLengths.
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

/* The next byte of a sequence through a pointer in zero page, and the pointer on past it. */
static uint8_t sequence_byte(uint16_t pointer)
{
    uint8_t byte = rd8_zp(pointer);
    uint8_t low = (uint8_t)(rd8(pointer) + 1);

    wr8(pointer, low);
    if (low == 0)
        wr8((uint16_t)(pointer + 1), (uint8_t)(rd8((uint16_t)(pointer + 1)) + 1));
    return byte;
}

/* A command and its arguments into SoundCommandByte and SoundCommandArgs, through a pointer. */
static CommandRead read_command(uint16_t pointer, uint8_t channel)
{
    CommandRead read;
    uint8_t byte = sequence_byte(pointer);
    uint8_t left;

    wr8(SoundCommandByte, byte);
    read.length = SoundCommandLength(byte, channel);
    /* The count is taken down until it goes negative, so above $80 it reads none. */
    left = read.length.count;
    read.args = 0;
    while ((int8_t)(left = (uint8_t)(left - 1)) >= 0) {
        wr8((uint16_t)(SoundCommandArgs + read.args), sequence_byte(pointer));
        read.args++;
    }
    read.left = left;
    return read;
}

/* As the count's exit, with A the count run out, N set, Z clear and Y the bytes read. */
static void abi_exit_command(CommandRead read)
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
 * Entry: X channel. Exit: A $FF, or the count less one when it is above $80; N set, Z clear; Y
 * the argument bytes read; C and V as SoundCommandLength left them. X kept.
 */
CommandRead SoundReadCommand(uint8_t channel)
{
    return read_command(zSoundSequenceLo, channel);
}

void abi_SoundReadCommand(void)
{
    abi_exit_command(SoundReadCommand(cpu.x));
}

/*
 * @routine 0053:6C6 SoundPeekCommand
 * @run 76:B6C6
 * WHAT IT DOES: Reads the channel's next sequence command and its arguments without moving its
 * place on.
 * HOW: A copy of the place in zSoundWordLo and Hi is read through instead.
 * Entry: X channel. Exit: as SoundReadCommand.
 */
CommandRead SoundPeekCommand(uint8_t channel)
{
    wr8(zSoundWordLo, rd8(zSoundSequenceLo));
    wr8(zSoundWordHi, rd8(zSoundSequenceHi));
    return read_command(zSoundWordLo, channel);
}

void abi_SoundPeekCommand(void)
{
    abi_exit_command(SoundPeekCommand(cpu.x));
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

void abi_SoundRampStep(void)
{
    Ramp ramp = SoundRampStep(cpu.x);

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

/*
 * @routine 004E:245 SoundApproachStep
 * @run 75:8A45
 * WHAT IT DOES: Moves a channel's approaching value toward its target by its step, as 8.8, and
 * ends the approach when it gets there.
 * HOW: SoundApproachBy added to or taken from SoundApproachLo, carrying into SoundApproach;
 * SoundApproachTarget.
 * Entry: X channel. Exit: X and Y kept. No step, A 0, Z set, N clear. There, A the target, Z and
 * C set, N clear. Moved, A the fraction, C and V from its add or subtract, N and Z from it, or
 * from the value when that carried into it.
 */
Approach SoundApproachStep(uint8_t channel)
{
    Approach approach = { APPROACH_IDLE, 0, 0, 0, 0, false };
    unsigned sum;

    approach.step = rd8((uint16_t)(SoundApproachBy + channel));
    if (approach.step == 0)
        return approach;
    approach.target = rd8((uint16_t)(SoundApproachTarget + channel));
    approach.value = rd8((uint16_t)(SoundApproach + channel));
    if (approach.target == approach.value) {
        wr8((uint16_t)(SoundApproachBy + channel), 0);
        approach.how = APPROACH_THERE;
        return approach;
    }
    approach.before = rd8((uint16_t)(SoundApproachLo + channel));
    approach.step = rd8((uint16_t)(SoundApproachBy + channel));
    if (approach.target > approach.value) {
        sum = (unsigned)approach.before + approach.step;
        approach.how = APPROACH_UP;
        approach.carried = sum > 0xFF;
    } else {
        sum = (unsigned)approach.before - approach.step;
        approach.how = APPROACH_DOWN;
        approach.carried = approach.before < approach.step;
    }
    wr8((uint16_t)(SoundApproachLo + channel), (uint8_t)sum);
    if (approach.carried) {
        approach.value = (uint8_t)(rd8((uint16_t)(SoundApproach + channel)) +
                                   (approach.how == APPROACH_UP ? 1 : -1));
        wr8((uint16_t)(SoundApproach + channel), approach.value);
    }
    return approach;
}

void abi_SoundApproachStep(void)
{
    Approach approach = SoundApproachStep(cpu.x);
    uint8_t fraction;

    switch (approach.how) {
    case APPROACH_IDLE:
        cpu.a = 0;
        abi_nz(0);
        return;
    case APPROACH_THERE:
        cpu.a = approach.target;
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    case APPROACH_UP:
        fraction = (uint8_t)(approach.before + approach.step);
        abi_flag(P_C, approach.carried);
        abi_flag(P_V, (~(approach.before ^ approach.step) & (approach.before ^ fraction) & 0x80) != 0);
        break;
    case APPROACH_DOWN:
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
 * channel's SoundOffsetScale, negated when bit 6 of the byte is set.
 * HOW: The byte's low six bits pick the entry; SoundMultiply.
 * Entry: A the byte, X channel. Exit: Y the scale; X kept. Positive, as SoundMultiply. Negative, A
 * the word's high byte, N, Z, C and V from subtracting it from 0 with the low byte's borrow.
 */
ScaledOffset SoundScaledOffset(uint8_t byte, uint8_t channel)
{
    ScaledOffset offset;
    uint8_t entry = (uint8_t)(byte & 0x7F);

    offset.negative = entry >= OFFSET_NEGATIVE;
    offset.scale = rd8((uint16_t)(SoundOffsetScale + channel));
    offset.product = SoundMultiply(rd8((uint16_t)(SoundOffsetTable + (entry & 0x3F))), offset.scale);
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
