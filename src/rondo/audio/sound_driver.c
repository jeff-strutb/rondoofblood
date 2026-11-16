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
    PSG_CHANNELS = 6, PSG_VOLUME_FULL = 0xFF, CHANNEL_RELEASED = 0x1F,
    EFFECT_SHARED_FIRST = 4, EFFECT_SHARED_END = 6, EFFECT_CHANNEL_OFFSET = 2,
    FADE_STEP_START = 0x10
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
 * @routine 004E:62D SoundMarkChannel
 * @run 75:8E2D
 * WHAT IT DOES: Marks a channel in SoundChannelMarked.
 * HOW: Stores 1.
 * Entry: X channel. Exit: A, X, Y, C and V kept; N and Z from A.
 */
void SoundMarkChannel(uint8_t channel)
{
    wr8((uint16_t)(SoundChannelMarked + channel), 1);
}

void abi_SoundMarkChannel(void)
{
    SoundMarkChannel(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 004E:635 SoundReleaseChannel
 * @run 75:8E35
 * WHAT IT DOES: Releases a channel: SoundChannel25A8 to $1F, the mark cleared, and the PSG
 * channel's control and balance cleared, unless it is 4 or 5 and an effect plays on it.
 * HOW: The PSG channel is the one already selected.
 * Entry: X channel. Exit: A, X and Y kept; N and Z from A; C set if the channel is 6 or more; V
 * bit 6 of the effect's command when channel 4 or 5, else kept.
 */
ChannelRelease SoundReleaseChannel(uint8_t channel)
{
    ChannelRelease release = { false, 0 };

    wr8((uint16_t)(SoundChannel25A8 + channel), CHANNEL_RELEASED);
    wr8((uint16_t)(SoundChannelMarked + channel), 0);
    release.looked = channel >= EFFECT_SHARED_FIRST && channel < EFFECT_SHARED_END;
    if (!effect_holds(channel, &release.effect)) {
        wr8(PsgControl, 0);
        wr8(PsgBalance, 0);
    }
    return release;
}

void abi_SoundReleaseChannel(void)
{
    ChannelRelease release = SoundReleaseChannel(cpu.x);

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
 * @routine 004D:7CC SoundVolumeSlideStep
 * @run 75:87CC
 * WHAT IT DOES: Moves a channel's volume on by its step, until it reaches the target.
 * HOW: SoundVolumeStepLo and Hi onto SoundVolumeLo and SoundVolume; SoundVolumeTarget.
 * Entry: X channel. Exit: as SoundSlideStep.
 */
SlideStep SoundVolumeSlideStep(uint8_t channel)
{
    return slide_step(channel, SoundVolumeStepLo, SoundVolumeStepHi, SoundVolumeLo, SoundVolume,
                      SoundVolumeTarget);
}

void abi_SoundVolumeSlideStep(void)
{
    abi_exit_slide(SoundVolumeSlideStep(cpu.x));
}
