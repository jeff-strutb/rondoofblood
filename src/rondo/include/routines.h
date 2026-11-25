/* Routines called across files. */
#ifndef RONDO_ROUTINES_H
#define RONDO_ROUTINES_H

#include "machine.h"
#include "syscard.h"

/* A byte add or subtract: the result, its carry as the processor's C, and its overflow. */
typedef struct { uint8_t value; bool carry; bool overflow; } Sum;

/* An add of two bytes with a carry in: the sum, its carry out and its signed overflow. */
static inline Sum add_bytes(uint8_t a, uint8_t b, bool carry_in)
{
    Sum sum;
    unsigned total = (unsigned)a + b + carry_in;

    sum.value = (uint8_t)total;
    sum.carry = total > 0xFF;
    sum.overflow = (~(a ^ b) & (a ^ sum.value) & 0x80) != 0;
    return sum;
}

/* A subtract of b from a with the carry in as no borrow: the difference, C as no borrow out. */
static inline Sum sub_bytes(uint8_t a, uint8_t b, bool carry_in)
{
    Sum sum;
    int total = (int)a - b - (carry_in ? 0 : 1);

    sum.value = (uint8_t)total;
    sum.carry = total >= 0;
    sum.overflow = ((a ^ b) & (a ^ sum.value) & 0x80) != 0;
    return sum;
}

/* An object's move along one axis: its 16.8 position before, and the velocity added. */
typedef struct {
    uint32_t before;
    int16_t velocity;
} AxisMove;

/* How far an object is from the player: far means past the first screen, distance $FF. */
typedef struct {
    uint8_t distance;
    bool far;
    bool object_further;
    uint8_t mine;       /* the object's low byte, when it was compared */
    uint8_t player;     /* the player's low byte, when it was compared */
} PlayerDistance;

/*
 * What a terrain read found, and the point's place in the tile before any handler moved it.
 * TILE_BY_ORIGINAL: another overlay's copy, with no C yet, answered.
 */
typedef enum { TILE_EMPTY_PAGE, TILE_PLAIN, TILE_PAST_HANDLED, TILE_HANDLED, TILE_BY_ORIGINAL } TileKind;

/*
 * What a tile handler leaves besides the new row in zTemp02: A is always $81. Y and the carry and
 * overflow of its last add or subtract, where it makes one.
 */
typedef struct {
    bool y_set;
    uint8_t y;
    bool flags_set;
    bool carry;
    bool overflow;
} HandlerExit;

typedef struct {
    TileKind kind;
    uint8_t attribute;  /* the byte read from the page */
    uint8_t page;       /* the page's entry */
    uint8_t high;       /* the tile's high bits, which pick the page */
    uint8_t column;     /* the point's pixel across the tile */
    uint8_t row;        /* the point's pixel down the tile */
    HandlerExit handler; /* what the tile's handler left, when it ran */
} TileRead;

TileRead StageTileRead(void);
uint8_t tile_answer(TileRead read);
void terrain_read_exit_shim(TileRead read);

/* How a terrain lookup's last column ended, and where it left the column index. */
typedef struct {
    bool by_original;
    bool last_skipped;
    uint8_t offset_low;
    uint8_t object_low;
    bool read_any;
    TileRead last;
    uint8_t column_after;
} LookupEnd;

LookupEnd StageLookup(uint8_t probe, uint8_t object, uint8_t column, bool carry_in);
LookupEnd StageLookupAlt(uint8_t probe, uint8_t object, uint8_t column, bool carry_in);
void lookup_exit_shim(LookupEnd end);

/* How the player overlay's wall probe ended. *_BY_ORIGINAL: another overlay answered. */
typedef enum { WALL_BARRED, WALL_PUSHED, WALL_FOUND, WALL_NONE, WALL_BY_ORIGINAL } WallEnd;

typedef struct {
    WallEnd how;
    uint8_t answer;       /* what another overlay answered */
    uint8_t column;       /* the column it stopped on */
    bool read_made;       /* the last column was read, not off the page */
    bool any_read;
    TileRead last_read;   /* the last column that was read */
    uint16_t last_row, last_lift;
} WallProbe;

/* How a read of the five columns ended. */
typedef struct {
    bool by_original;
    bool read_made;
    TileRead last;
    uint8_t row;
    uint8_t below;
} ColumnsEnd;

/* How the check for a floor came out, and by which test. */
typedef enum {
    FLOOR_HIGH_ON_ODD_SCREEN, FLOOR_FOUND, FLOOR_FROM_PUSH, FLOOR_NONE
} FloorCheck;

typedef struct {
    FloorCheck how;
    uint8_t column;
    uint8_t push_before;
} FloorResult;

/* How the player overlay's floor probe ended. */
typedef enum {
    FLOOR_BARRED, FLOOR_NOTHING_UNDER, FLOOR_COLUMN_SOLID, FLOOR_LANDED, FLOOR_LANDED_PUSHED,
    FLOOR_OFF_THE_TOP, FLOOR_BY_ORIGINAL
} FloorEnd;

typedef struct {
    FloorEnd how;
    uint8_t answer;       /* what another overlay answered */
    ColumnsEnd columns;
    FloorResult check;
    bool read_made;
    TileRead read;
    uint8_t row;
    uint16_t y;
    uint16_t depth;
} FloorProbe;

/* What a step of an animation did. */
typedef enum { FRAME_STOPPED, FRAME_END_MARK, FRAME_SHOWN } FrameEnd;

typedef struct {
    FrameEnd how;
    bool looped;        /* the list started over, so its step before that doubles to zero */
    bool same_frame;    /* the timer ran on: the list was not read for a new one */
    uint8_t step;
    uint8_t frame;
} AnimationStep;

/* A lift out of the floor by how deep a column read. */
typedef struct {
    bool off_the_top;
    uint16_t y;
    uint16_t depth;
} ColumnLift;

/* How the ladder check ended. */
typedef enum {
    LADDER_CAMERA_NO, LADDER_CAMERA_YES, LADDER_OFF_SCREEN, LADDER_TOO_LOW, LADDER_OFF_PAGE,
    LADDER_BLOCKED, LADDER_REACH_OUT, LADDER_ANSWERED
} LadderEnd;

typedef struct {
    LadderEnd how;
    uint16_t reach;     /* the last reach below him */
    bool any_read;
    TileRead last;
    uint8_t answer;
} LadderCheck;

AnimationStep RichterAnimationRunning(void);
void animation_running_exit_shim(AnimationStep step);
AnimationStep RichterAnimationTick(void);
AnimationStep RichterSetAnimation(uint8_t animation);
void animation_exit_shim(AnimationStep result);
void RichterClearPadOverrides(void);
void RichterFaceByPad(void);
typedef enum { CRASH_NO_PRESS, CRASH_ITEM_OUT, CRASH_TOO_FEW_HEARTS, CRASH_STARTED } CrashHow;

/* What RichterTryItemCrash did, with what its exit leaves in the registers. */
typedef struct {
    CrashHow how;
    uint8_t pressed;
    uint8_t slot;
    uint8_t out;
    uint8_t item;
    uint8_t hearts;
    uint8_t cost;
    uint8_t script;
} ItemCrash;

ItemCrash RichterTryItemCrash(void);

typedef enum {
    USE_NO_PRESS, USE_NOT_HELD, USE_WHIP, USE_TOO_FEW_HEARTS, USE_ITEM_OUT, USE_SLOTS_FULL,
    USE_STARTED
} ItemUseHow;

/* What using the item did, and what it looked at last, which its exit leaves in the registers. */
typedef struct {
    ItemUseHow how;
    uint8_t item;
    uint8_t hearts;
    uint8_t cost;
    uint8_t seen;
    uint8_t slot;
    uint8_t script;
    bool by_index;
    bool scanned;
    bool script_started;
} ItemUse;

typedef struct { bool found; uint8_t slot; uint8_t seen; } ItemScan;

ItemUse RichterUseItem(void);
ItemUse RichterUseThrownItem(uint8_t item);
ItemUse RichterUseItemSix(uint8_t item);
ItemUse RichterUseItemSeven(uint8_t item);
ItemUse RichterUseItemEight(uint8_t item);
ItemScan RichterFindItemOut(void);
void item_use_exit_shim(ItemUse use);
ColumnsEnd RichterReadColumns(void);
TileRead RichterReadAbove(uint8_t row, bool on_the_map);
void read_above_exit_shim(TileRead read, bool on_the_map);
uint8_t RichterColumnOffset(uint8_t column);
ColumnLift RichterLiftByColumn(uint8_t column);
void column_lift_exit_shim(ColumnLift lift);
LadderCheck RichterLadderCheck(void);
bool ladder_found(LadderCheck check);
void ladder_exit_shim(LadderCheck check);

/* What SoundSilenceChannel looked at: the effect on a shared channel, when 4 or 5. */
typedef struct { bool looked; uint8_t effect; } ChannelRelease;

/* A product of the driver's shift and add, with the last add's high byte and overflow. */
typedef struct { uint16_t value; bool added; uint8_t high; bool overflow; } SoundProduct;

void SoundSilenceAll(uint8_t kept_x);
void SoundVolumeFull(void);
void SoundStartEnvelope(uint8_t channel);
ChannelRelease SoundSilenceChannel(uint8_t channel);
SoundProduct SoundMultiply(uint8_t a, uint8_t y, uint8_t kept_x);
SoundProduct SoundMultiplyNibble(uint8_t a, uint8_t y);
uint8_t SoundReadPair(uint8_t channel);
void sound_pointer_step(void);
uint8_t SoundStartFade(void);
uint8_t SoundLoadSequence(uint8_t channel);
uint8_t SoundSaveSequence(uint8_t channel);
uint8_t SoundSequenceFromWord(void);

typedef enum { SLIDE_IDLE, SLIDE_REACHED, SLIDE_MOVED } SlideHow;

/* A slide's step: what it did, and the high byte with its add's carry and overflow. */
typedef struct { SlideHow how; uint8_t value; bool carry; bool overflow; } SlideStep;

SlideStep SoundSlideStep(uint8_t channel);
SlideStep SoundPanSlideStep(uint8_t channel);
void slide_exit_shim(SlideStep slide);
void SoundSetPan(uint8_t channel, uint8_t pan);
uint16_t SoundWaveAddress(uint8_t wave);

typedef enum { LENGTH_ONE, LENGTH_DEFAULT, LENGTH_NONE, LENGTH_TABLE } LengthHow;

/* A command's argument count, how it was found, the command, and the default or table index. */
typedef struct { LengthHow how; uint8_t count; uint8_t command; uint8_t value; } CommandLength;

/* A command read: its length, the argument bytes read, and the count as it ran out. */
typedef struct { CommandLength length; uint8_t args; uint8_t left; } CommandRead;

CommandLength SoundCommandLength(uint8_t byte, uint8_t channel);
void command_length_exit_shim(CommandLength length);
CommandRead SoundReadCommand(uint8_t channel);
CommandRead SoundPeekCommand(uint8_t channel);
void command_read_exit_shim(CommandRead read);

/* A modulation step: whether it ran, the phase before, and the high byte's add. */
typedef struct {
    bool applied;
    uint8_t speed;
    uint8_t phase;
    uint8_t add;
    uint8_t before;
    bool carry_in;
} Modulation;

Modulation SoundModulate(uint8_t channel);

typedef enum { RAMP_WAITING, RAMP_NO_STEP, RAMP_TOO_CLOSE, RAMP_MOVED } RampHow;

/* A ramp step: what stopped or moved it, the value its exit shows, and what was compared. */
typedef struct { RampHow how; uint8_t value; uint8_t from; uint8_t slide; uint8_t high; } Ramp;

Ramp SoundRampStep(uint8_t channel);
void ramp_exit_shim(Ramp ramp);

typedef enum { TEMPO_SLIDE_IDLE, TEMPO_SLIDE_THERE, TEMPO_SLIDE_UP, TEMPO_SLIDE_DOWN } TempoSlideHow;

/* A tempo slide step: which way, the step, the fraction before, and the tempo after any carry. */
typedef struct {
    TempoSlideHow how;
    uint8_t step;
    uint8_t target;
    uint8_t value;
    uint8_t before;
    bool carried;
} TempoSlide;

TempoSlide SoundTempoSlideStep(uint8_t channel);
void tempo_slide_exit_shim(TempoSlide approach);

typedef enum {
    ENVELOPE_OFF, ENVELOPE_START, ENVELOPE_ATTACK, ENVELOPE_DECAY, ENVELOPE_RELEASE,
    ENVELOPE_PAST, ENVELOPE_WAITING, ENVELOPE_TIMED
} EnvelopeHow;

typedef enum { ENVELOPE_COMPARED_NONE, ENVELOPE_COMPARED_SUSTAIN, ENVELOPE_COMPARED_RELEASE } EnvelopeCompare;

/* An envelope frame: the phase it was in, what happened, and what its exit shows. */
typedef struct {
    uint8_t how;
    uint8_t phase;
    uint8_t timer;
    uint8_t frames;
    EnvelopeCompare compared;
    uint8_t level;
    uint8_t sustain;
} EnvelopeStep;

EnvelopeStep SoundEnvelopeStep(uint8_t channel);

/* A scaled offset: its sign, the scale, and the product before any negation. */
typedef struct { bool negative; uint8_t scale; SoundProduct product; } ScaledOffset;

ScaledOffset SoundScaledOffset(uint8_t byte, uint8_t channel);

/* A PSG control set: whether an effect held the channel, what it read, and the pan and balance. */
typedef struct {
    bool looked;
    bool held;
    uint8_t effect;
    uint8_t pan;
    uint8_t alt;
    uint8_t balance;
    uint8_t volume;
} ControlSet;

ControlSet SoundSetControl(uint8_t volume, uint8_t channel);
void control_exit_shim(ControlSet set, uint8_t channel);
ControlSet SoundApplyVolume(uint8_t less, uint8_t channel);

/* A rest: what silencing the channel looked at, and the length. */
typedef struct { ChannelRelease release; uint8_t length; } ChannelSilence;

ChannelSilence SoundRest(uint8_t channel);

/* A note's length and gate, the product the gate came from, and the gate timer. */
typedef struct { uint8_t length; uint8_t gate; SoundProduct product; uint8_t timer; } NoteLength;

NoteLength SoundSetLength(uint8_t channel);

void wave_address_exit_shim(uint8_t wave);

/* A frequency set: whether an effect held the channel, noise or tone, the byte written last. */
typedef struct {
    bool looked;
    bool held;
    uint8_t effect;
    bool noise;
    uint8_t value;
    uint8_t high_before;
    bool borrow;
} FrequencySet;

FrequencySet SoundSetFrequency(uint8_t fine, uint8_t channel, uint8_t note);
void frequency_exit_shim(FrequencySet set, uint8_t note);

/* A pitch tick: the fine part and note set, the last add's flags, and the frequency's set. */
typedef struct { uint8_t fine; uint8_t note; Sum last; FrequencySet frequency; } PitchTick;

PitchTick SoundPitchTick(uint8_t channel);
PitchTick SoundPitchTickPlain(uint8_t channel);

typedef enum { VOLUME_NONE, VOLUME_PLAIN, VOLUME_WAITING, VOLUME_TREMOLO } VolumeHow;

/* A volume tick: what ran, the ramp's step, the tremolo taken off and the control set. */
typedef struct { VolumeHow how; Ramp ramp; uint8_t less; ControlSet volume; } VolumeTick;

VolumeTick SoundVolumeTick(uint8_t channel);
void volume_tick_exit_shim(VolumeTick tick, uint8_t channel);

/* A division: whether its last step subtracted, the remainder's high byte before and its borrow. */
typedef struct { bool took; uint8_t last; uint8_t high_before; bool borrow; bool unused; uint16_t quotient; } Division;

Division SoundDivide(uint8_t divisor);
void division_exit_shim(Division division);

/* A glide aimed: whether it was, which way, the division, the step and the frames. */
typedef struct { bool aimed; bool down; Division division; uint16_t step; uint8_t frames; } GlideSetup;

GlideSetup SoundGlideSetup(uint8_t fine, uint8_t note, uint8_t channel);
PitchTick SoundNotePitch(uint8_t channel);

/* A note started: its pitch tick, length, gate timer and the product it came from. */
typedef struct { PitchTick pitch; uint8_t length; bool gated; SoundProduct product; uint8_t timer; } NoteStart;

NoteStart SoundStartNote(uint8_t channel);

typedef enum { WAVE_NONE, WAVE_HELD, WAVE_LOADED } WaveHow;

typedef struct { WaveHow how; uint8_t wave; uint8_t effect; } WaveLoad;

WaveLoad SoundLoadWave(uint8_t wave, uint8_t channel);
void SoundInit(void);
void abi_SoundInit(void);
void SoundDriverStopAll(void);
void wave_load_exit_shim(WaveLoad load, uint8_t wave);

/* A slide started: down or up, its target and what it was compared with, the step made negative. */
typedef struct { bool down; uint8_t aim; uint8_t from; uint8_t step; } SlideStart;

/* A loop's arrays by channel: where it starts, its passes, and the volume and detune it adds. */
typedef struct {
    uint16_t start_lo;
    uint16_t start_hi;
    uint16_t count;
    uint16_t volume;
    uint16_t detune_fine;
    uint16_t detune_note;
} SoundLoop;

/* A loop's end: done or back, the byte its exit shows, and the detune's add. */
typedef struct { bool done; uint8_t value; Sum sum; } LoopEnd;

typedef struct { uint8_t state; uint8_t value; } Repeat;

/* An envelope set: the rate's tens and units, the sustain, and the last subtract of ten. */
typedef struct { uint8_t tens; uint8_t units; uint8_t sustain; bool subtracted; bool overflow; } Envelope;

/* A return or an end: whether it ended, and what silencing and the waveform did. */
typedef struct { bool ended; ChannelRelease release; WaveLoad wave; uint8_t value; } SequenceEnd;

void sequence_end_exit_shim(SequenceEnd end, uint8_t channel);
void sequence_end_flags_shim(SequenceEnd end, uint8_t channel);
WaveLoad SoundCommandWave(uint8_t channel);
uint8_t SoundCommandPan(uint8_t channel);
SlideStart SoundCommandPanSlide(uint8_t channel);
uint8_t SoundCommandVibrato(uint8_t channel);
uint16_t SoundCommandVibratoGrow(uint8_t channel);
uint8_t SoundCommandModulation(uint8_t channel);
uint8_t SoundCommandLoopA(uint8_t channel);
LoopEnd SoundCommandLoopAEnd(uint8_t channel);
uint8_t SoundCommandLoopB(uint8_t channel);
LoopEnd SoundCommandLoopBEnd(uint8_t channel);
uint8_t SoundCommandTempo(uint8_t channel);
uint8_t SoundCommandTempoSlide(uint8_t channel);
uint8_t SoundCommandTranspose(uint8_t channel);
uint8_t SoundCommandTremolo(uint8_t channel);
uint8_t SoundCommandVolume(uint8_t channel);
SlideStart SoundCommandVolumeSlide(uint8_t channel);
uint8_t SoundCommandGlideFromLast(uint8_t channel);
uint8_t SoundCommandGlideFromBelow(uint8_t channel);
uint8_t SoundCommandFineTune(uint8_t channel);
uint8_t SoundCommandRepeatMark(uint8_t channel);
Repeat SoundCommandRepeat(uint8_t channel);
Envelope SoundCommandEnvelope(uint8_t channel);
uint8_t SoundCommandRamp(uint8_t channel);
void SoundCommandNothing(void);
uint8_t SoundCommandJump(void);

/* Richter's own objects: src/rondo/object/richter_objects.c. */
typedef struct { uint8_t player_2fe8; uint8_t facing; } ObjectFromPlayer;

typedef enum { QUICKEN_DONE_BEFORE, QUICKEN_NOT_STEP, QUICKEN_NOT_ASKED, QUICKEN_DONE } QuickenHow;

/* What stopped or made the quickening, the value its exit shows, the mark and the add. */
typedef struct { QuickenHow how; uint8_t value; uint8_t mark; Sum sum; } Quicken;

typedef enum { FLASH_NOT_CHARGED, FLASH_OTHER_FRAME, FLASH_SHOWN } FlashHow;

/* A flash: what happened, the frame or flags its exit shows, the frame code, and OBJ_34B8. */
typedef struct { FlashHow how; uint8_t value; uint8_t code; uint8_t base; } Flash;

typedef enum { CHARGE_TOO_SOON, CHARGE_ALREADY, CHARGE_NOT_PRESSED, CHARGE_NOW } ChargeHow;

/* A charge: what stopped or made it, the value its exit shows, the facing and the new charge. */
typedef struct { ChargeHow how; uint8_t value; uint8_t facing; uint8_t charge; } Charge;

uint8_t RichterObjectMarkOn(uint8_t object);
uint8_t RichterObjectMarkOff(uint8_t object);
uint8_t RichterObjectTake2FE8(uint8_t object);
ObjectFromPlayer RichterObjectFromPlayer(uint8_t object);
uint8_t RichterObjectSlot(void);
uint8_t RichterObjectNextState(uint8_t object);
Quicken RichterObjectQuicken(uint8_t object);
Flash RichterObjectFlash(uint8_t object);
Charge RichterObjectCharge(uint8_t object);
Sum RichterObjectAtPlayer(uint8_t object);
Sum RichterObjectAddXLow(uint8_t object, uint8_t amount);
Sum RichterObjectAddXHigh(uint8_t object, uint8_t amount, bool carry);
Sum RichterObjectSetFall(uint8_t object, uint8_t low, bool carry);
Sum RichterObjectFall(uint8_t object);
Sum RichterObjectFallSlow(uint8_t object);
uint8_t RichterObjectAtPlayerPlace(uint8_t object);
uint8_t RichterObjectToPoint(uint8_t object);
void RichterObjectHitboxOff(uint8_t object);

/* A hitbox given or not: the hitbox, the frame it came from, and the last byte written. */
typedef struct { bool given; uint8_t hitbox; uint8_t last; uint8_t frame; } Hitbox;

Hitbox RichterObjectHitboxFrom(uint8_t object, uint8_t hitbox, bool none);
void hitbox_exit_shim(Hitbox box);
Hitbox RichterObjectHitboxForFrame(uint8_t object);
Hitbox RichterObjectHitboxOn(uint8_t object);
uint8_t RichterObjectAnimationList(uint8_t object);
AnimationStep RichterObjectSetAnimation(uint8_t object, uint8_t animation);
AnimationStep RichterObjectAnimationTick(uint8_t object);
AnimationStep RichterObjectAnimationRunning(uint8_t object);
AnimationStep RichterObjectAnimationWait(uint8_t object);
uint8_t SoundCommandCall(uint8_t channel);
SequenceEnd SoundCommandReturn(uint8_t channel);
void glide_setup_exit_shim(GlideSetup setup);
void note_start_exit_shim(NoteStart start);

typedef enum { SEQUENCE_ENDED, SEQUENCE_AT_REST, SEQUENCE_AT_LENGTH, SEQUENCE_AT_NOTE } SequenceHow;

/* A sequence read on: how it stopped, the channel it ran on, the last command, and what it did. */
typedef struct {
    SequenceHow how;
    uint8_t channel;
    uint8_t command;
    CommandRead read;
    ChannelSilence rest;
    NoteLength length;
    NoteStart note;
    SequenceEnd end;
    uint8_t place_high;
} SequenceRead;

SequenceRead SoundReadSequence(uint8_t channel);
void sequence_read_exit_shim(SequenceRead read);
PitchTick SoundGateStep(uint8_t channel);

typedef enum { LOOK_GLIDING, LOOK_OTHER, LOOK_TAKEN } LookHow;

/* A look at the next command: a glide going, another command, or a glide taken and aimed. */
typedef struct {
    LookHow how;
    uint8_t value;
    CommandRead peek;
    uint8_t note;
    uint8_t fine;
    bool carry;
    GlideSetup glide;
} Lookahead;

Lookahead SoundGlideLookahead(uint8_t channel);
void glide_lookahead_exit_shim(Lookahead look);

typedef enum {
    CHANNEL_BETWEEN_RESTING, CHANNEL_BETWEEN, CHANNEL_ENDED, CHANNEL_RESTING, CHANNEL_READ, CHANNEL_GATED
} ChannelHow;

/* A channel's frame: what ran, and what each step it ran did. */
typedef struct {
    ChannelHow how;
    Sum tempo;
    uint8_t rest;
    VolumeTick volume;
    PitchTick pitch;
    uint8_t length;
    SequenceRead sequence;
    uint8_t bit;
    uint8_t playing;
    TempoSlide tempo_slide;
    SlideStep slide;
    SlideStep pan;
    PitchTick gate;
    Lookahead lookahead;
    uint8_t wait;
} ChannelTick;

ChannelTick SoundChannelTick(uint8_t channel);
void channel_tick_exit_shim(ChannelTick tick, uint8_t channel);
void SoundStartChannel(uint8_t channel);
uint8_t SoundStartMusic(void);

/* A voice's start moved back: whether it was, the voice compared, and the high byte's add. */
typedef struct { bool moved; bool compared; uint8_t voice; Sum high; } VoiceBack;

VoiceBack SoundVoiceAddressBack(void);
void voice_back_exit_shim(VoiceBack back);
WallProbe PlayerWall(uint8_t side);
FloorProbe PlayerFloor(void);
ColumnsEnd PlayerMoved(void);
bool wall_found(WallProbe probe);
bool floor_found(FloorProbe probe);
uint8_t wall_answer(WallProbe probe);
uint8_t floor_answer(FloorProbe probe);
void wall_exit_shim(WallProbe probe);
void floor_exit_shim(FloorProbe probe);
void columns_exit_shim(ColumnsEnd end);
AxisMove ApplyVelocity(uint8_t object);
AxisMove ApplyVelocityX(uint8_t object);
AxisMove ApplyVelocityY(uint8_t object);
Sum PlayerDistanceSum(uint8_t object, bool carry_in);
void distance_sum_exit_shim(Sum last);
Sum NegateVelocityX(uint8_t object);
Sum NegateVelocityY(uint8_t object);
void negate_exit_shim(Sum last);
uint8_t axis_move_flags_shim(AxisMove move);
void ClearObject(uint8_t object);
void ClearObjectInY(uint8_t object);
void SelectVdcRegister(uint8_t reg);
void SelectVramWriteAddress(void);
void SelectVramData(void);
void SelectVramReadAddress(void);
void ClearVdcControlHigh(void);
void SelectVdcControl(void);
uint8_t WriteScreenTiming(uint8_t index);
uint8_t WriteBandScroll(uint8_t band);
void ResetDisplayControl(void);
/* NextRandom: the byte, and the carry and overflow of the last add. */
typedef struct { uint8_t value; bool carry; bool overflow; } RandomResult;
RandomResult NextRandom(void);
/* math/trig.c: the angle routines' results. */
typedef struct { uint8_t a; uint8_t y; bool c; bool v; } AngleExit;
typedef struct { uint8_t angle; bool equal; Sum last; } TurnAngleResult;
AngleExit AngleOfFirstQuadrant(void);
AngleExit AngleFromDelta(void);
AngleExit AngleToPlayer(uint8_t object);
AngleExit AngleToPoint(uint8_t object);
AngleExit AngleToPlayerOffset(uint8_t object);
TurnAngleResult TurnAngleToward(void);
void angle_exit_shim(AngleExit result);
void turn_angle_exit_shim(TurnAngleResult result);

/* math/trig.c: a multiply or a scale and what it leaves: A, N/Z source, C, and V when an add ran. */
typedef struct { uint8_t a; uint8_t nz; bool carry; bool v_set; bool overflow; } ScaleResult;
ScaleResult MultiplyBySineWord(void);
ScaleResult ScaleBySine(uint8_t angle);
ScaleResult ScaleByCosine(uint8_t angle);
ScaleResult SetVelocityFromAngle(uint8_t angle, uint8_t object);
void scale_exit_shim(ScaleResult result);

/* object/spawn.c: the record start, the place copies, the animation step. */
typedef enum { FRAME_HELD, FRAME_CHANGED, FRAME_LIST_ENDED } FrameStep;
uint8_t LoadObjectRecord(uint8_t object, uint8_t *exit_nz, bool *exit_c);
void load_record_exit_shim(uint8_t last, uint8_t state, bool carry);
uint8_t CopyObjectPlace(uint8_t from, uint8_t to);
uint8_t StartObjectAtPlace(uint8_t type, uint8_t from, uint8_t to);
uint8_t CopyObjectPlaceBack(uint8_t from, uint8_t to);
void copy_place_exit_shim(uint8_t last);
FrameStep StepObjectFrames(uint8_t object, uint8_t *exit_y);
void step_frames_exit_shim(FrameStep step, uint8_t y);
uint8_t FindFreeSlotFrom30(uint8_t *exit_a, bool *found);
uint8_t FindFreeSlotFrom20(uint8_t *exit_a, bool *found);
uint8_t FindFreeSlotFrom50(uint8_t *exit_a, bool *found);
void find_slot_exit_shim(uint8_t slot, uint8_t last_type, bool found);
/* SideOfPlayer: the side bits, and the carry of the last compare when one was made. */
typedef struct { uint8_t sides; bool carry_set; bool carry; } SideResult;
SideResult SideOfPlayer(uint8_t object);
void side_of_player_exit_shim(SideResult result);
void random_exit_shim(RandomResult result);
bool RasterStart(uint8_t *effect, uint8_t *last);
void RasterInterrupt(void);
uint8_t BitOfIndex(uint8_t index, uint8_t x, uint8_t *byte);
void bit_of_index_exit_shim(uint8_t index, uint8_t mask, uint8_t byte);
void score_exit_shim(uint8_t changed);
void record_event_exit_shim(uint8_t event, uint8_t merged, uint8_t byte);
uint8_t RecordEvent(uint8_t event, uint8_t x, uint8_t *byte);
bool RecordObjectEvent(uint8_t object, uint8_t *stored, uint8_t *event, uint8_t *byte);
/* system/channels.c: each returns what it leaves in A (the kind, or the script); its exit shim applies it. */
typedef struct { uint8_t script; uint8_t busy_kind; bool started; } SoleSlot;
void ClearListChannels(void);
uint8_t StartListChannel0Kind1(uint8_t list);
uint8_t StartListChannelKind1(uint8_t list, uint8_t channel);
uint8_t StartListChannel0Kind2(uint8_t list);
uint8_t StartListChannelKind2(uint8_t list, uint8_t channel);
uint8_t StartListChannelKind3(uint8_t list, uint8_t channel);
void list_channel_exit_shim(uint8_t kind);
void ClearScriptSlot(uint8_t slot);
uint8_t StartScriptSlot0Kind1(uint8_t script);
uint8_t StartScriptSlotKind1(uint8_t script, uint8_t slot);
uint8_t StartScriptSlot0Kind2(uint8_t script);
uint8_t StartScriptSlotKind2(uint8_t script, uint8_t slot);
uint8_t StartScriptSlot0Kind3(uint8_t script);
uint8_t StartScriptSlotKind3(uint8_t script, uint8_t slot);
uint8_t StartScriptSlotKind4(uint8_t script, uint8_t slot);
SoleSlot StartSoleScriptSlot(uint8_t script, uint8_t slot);
uint8_t StartScriptSlotKind6(uint8_t script, uint8_t slot);
uint8_t StartScriptSlotKind7(uint8_t script, uint8_t slot);
void script_slot_exit_shim(uint8_t script);
void sole_slot_exit_shim(SoleSlot result);
PlayerDistance DistanceXToPlayer(uint8_t object);
PlayerDistance DistanceYToPlayer(uint8_t object);
void distance_exit_shim(PlayerDistance d);

/* ApplyGravity and ApplyGravityUnlimited: whether the speed gained; the add's low byte, carry and overflow;
 * the new high byte when it carried, else (not gained) the high byte read. */
typedef struct { bool gained; uint8_t low; uint8_t high; bool carried; bool overflow; bool unused; } GravityResult;
GravityResult ApplyGravityUnlimited(uint8_t object);
GravityResult ApplyGravity(uint8_t object);
void gravity_exit_shim(GravityResult result);
void abi_SoundCommand(void);
void PlayerAtRightEdge(void);
void PlayerAtLeftEdge(void);
void PlayerAtTop(void);
void StopPlayerX(void);
void abi_ResetGameState(void);
void abi_SoundStopAll(void);
void abi_WaitVblankDisplayOff(void);
void abi_SelectVramWriteAddress(void);
void abi_SelectVramData(void);
void abi_BackupFailed(void);
void abi_FrameModeDispatch(void);
void abi_LoadSceneScript(void);
void abi_AdpcmLoad(void);
/* A stage's bit in the visited tables: the byte, the bit, and zStage as read. */
typedef struct { uint8_t index; uint8_t bit; uint8_t stage; } StageBit;

StageBit StageClearBit(void);
void abi_StageClearBit(void);
void stage_bit_exit_shim(StageBit found);
uint8_t NextGameMode(void);
void SetGameMode(uint8_t mode);
void ResetScoreCounters(void);
void abi_NextGameMode(void);
void abi_SetGameMode(void);
void abi_ResetScoreCounters(void);
BackupAccess BackupRead(void);
void backup_read_exit_shim(BackupAccess read);
void abi_BackupRead(void);
Sum SaveVramToRam(void);
void abi_SaveVramToRam(void);

/* The kernel's ADPCM play: src/rondo/audio/adpcm.c. The card's types come from syscard.h. */
typedef enum { KERNEL_PLAY_CARD, KERNEL_PLAY_BUSY, KERNEL_PLAY_BAD_RATE, KERNEL_PLAY_STARTED } KernelPlayHow;

/* The kernel's play: the card's version, and what the card's ad_play or its own ports did. */
typedef struct {
    KernelPlayHow how;
    CardVersion version;
    AdPlay card;
    AdStat stat;
    uint8_t rate;
    uint8_t enables;
} KernelPlay;

CardVersion CardIsVersionOne(void);
AdStat KernelAdpcmStat(void);
uint8_t KernelAdpcmLatchWrite(void);
KernelPlay KernelAdpcmPlay(void);
void kernel_play_exit_shim(KernelPlay play);

typedef enum { VOICE_DROPPED, VOICE_WAITING_BUSY, VOICE_WAITING_PLAY, VOICE_PLAYED } VoiceHow;

/* A voice started or not, with what each call on the way did, for its exit. */
typedef struct {
    VoiceHow how;
    AdStat stat_before;
    uint8_t command;
    bool second;
    uint8_t index;
    SoundProduct product;
    Sum table_low;
    Sum table_high;
    uint8_t priority;
    uint8_t playing_priority;
    uint8_t stop;
    AdStat stat;
    uint8_t flags;
    bool moved_back;
    VoiceBack back;
    KernelPlay play;
} VoiceStart;

VoiceStart SoundStartVoice(void);
void voice_start_exit_shim(VoiceStart voice);
void music_start_exit_shim(uint8_t command);

typedef enum {
    DRIVER_STOPPED, DRIVER_FADING, DRIVER_BALANCE_ON, DRIVER_BALANCE_OFF, DRIVER_MUSIC, DRIVER_HELD,
    DRIVER_VOICE, DRIVER_EFFECT_LOW, DRIVER_EFFECT
} DriverHow;

/* An effect's entry in SoundEffectTable: the multiply and adds that found it, and its priority. */
typedef struct { SoundProduct product; Sum low; Sum high; uint8_t priority; } EffectEntry;

/* A command taken: its path, and what that path's calls did. */
typedef struct {
    DriverHow how;
    uint8_t command;
    uint8_t fade;
    uint8_t music;
    VoiceStart voice;
    EffectEntry entry;
    uint8_t channels_before;
} DriverCommand;

DriverCommand SoundDriverCommand(uint8_t command, uint8_t kept_x, uint8_t kept_y);
void driver_command_exit_shim(DriverCommand taken);

typedef enum { PAUSE_SAME, PAUSE_ON, PAUSE_OFF } PauseHow;

/* The pause followed: what changed, and what each call it made did. */
typedef struct {
    PauseHow how;
    uint8_t bit;
    uint8_t stop;
    DriverCommand command;
    WaveLoad four;
    WaveLoad five;
    uint8_t wave_five;
    uint8_t on_before_five;
} PauseCheck;

PauseCheck SoundCheckPause(uint8_t kept_x, uint8_t kept_y);
void pause_check_exit_shim(PauseCheck check);

/* A frame of the driver: what ran, and what each call it made did, channel by channel. */
typedef struct {
    bool ran;
    PauseCheck pause;
    bool paused;
    AdStat stat;
    bool stop_ran;
    uint8_t stop;
    AdStat stat_again;
    bool voice_waited;
    SoundProduct product;
    Sum table_low;
    Sum table_high;
    bool moved_back;
    VoiceBack back;
    KernelPlay play;
    bool played;
    bool fade_ended;
    ChannelRelease faded[6];
    uint8_t on_before_fade;
    bool ticked[8];
    ChannelTick channel[8];
} DriverTick;

DriverTick SoundDriverTick(uint8_t kept_a, uint8_t kept_x, uint8_t kept_y);
void driver_tick_exit_shim(DriverTick tick);

/* The kernel's calls into the driver: the add that mapped it, the MPR4 bank given back, and the driver's work. */
typedef struct { Sum bank; uint8_t mpr4; uint8_t x; uint8_t y; } StopCall;
typedef struct { uint8_t busy; Sum bank; uint8_t mpr4; DriverTick tick; } TickCall;
typedef struct { bool taken; Sum bank; uint8_t mpr4; DriverCommand command; } CommandCall;

StopCall SoundStopAll(uint8_t kept_x, uint8_t kept_y);
void stop_call_exit_shim(StopCall call);
TickCall SoundFrameTick(uint8_t kept_x, uint8_t kept_y);
CommandCall SoundCommand(uint8_t command, uint8_t kept_x, uint8_t kept_y);
void sound_command_exit_shim(uint8_t command, CommandCall call);
/* SoundIfOnFirstScreen (object/camera.c): the command pulled back, and whether it went to SoundCommand. */
typedef struct { bool played; uint8_t command; CommandCall call; } SoundOnScreen;
SoundOnScreen SoundIfOnFirstScreen(uint8_t command, uint8_t object, uint8_t kept_y);
void sound_if_on_first_screen_exit_shim(SoundOnScreen result);

/* An attack started: its animation, its kind, and the whip's sound or the item and its slot. */
typedef struct {
    AnimationStep animation;
    uint8_t kind;
    CommandCall sound;
    uint8_t item;
    uint8_t slot;
    uint8_t tested_b;
} AttackStart;

AttackStart RichterAttack(void);
void attack_exit_shim(AttackStart attack);

/* A lift by the jump's speed: the position's high byte before and after, and the borrow. */
typedef struct { uint8_t high_before; uint8_t high; bool borrow; } Lift;

/* Item 5's crash: its animation and its sound. */
typedef struct { AnimationStep animation; CommandCall sound; } CrashFive;

/* A hurt jump: item 5's crash at once, or the lift, animation and sound, then the reaction. */
typedef struct {
    bool crash;
    CrashFive crash_five;
    Lift lift;
    AnimationStep animation;
    CommandCall sound;
    uint8_t item;
} HurtAir;

CrashFive RichterItemCrashFive(void);
void crash_five_exit_shim(CrashFive crash);
HurtAir RichterToHurtAir(void);
void RichterReactFillD(void);
void RichterReactFillB(void);
void RichterReactE(void);
void RichterReact10(void);
void RichterReact11(void);
void RichterReact12(void);
void RichterReact13(void);
void hurt_air_exit_shim(HurtAir hurt);

typedef enum { DYING_WAITING, DYING_OBJECT_LEFT, DYING_OVER } DyingHow;

/* A frame of dying: still waiting, an object still out, or over; the value its exit shows. */
typedef struct { DyingHow how; uint8_t value; uint8_t object; } DyingStep;

/* A turn on a press: the facing before, the presses, and whether it turned. */
typedef struct { uint8_t facing; uint8_t pressed; bool turned; } TurnPress;

DyingStep RichterDyingStep(void);
TurnPress RichterTurnOnPress(void);
/* A CD track played: its entry's address, and the BIOS's play tried until it succeeded. */
enum { CD_PLAY_KEPT = 32 };

typedef struct { MaMul8u mul; Sum high; } TrackEntry;

typedef struct {
    uint8_t track;
    uint8_t mode;
    uint8_t dl;
    uint8_t x, y;   /* X and Y as pulled back */
    TrackEntry entry;
    unsigned tries;
    uint8_t results[CD_PLAY_KEPT];
    CdPlay attempts[CD_PLAY_KEPT];
} TrackPlay;

TrackPlay CdPlayOnce(uint8_t track, uint8_t kept_x, uint8_t kept_y);
TrackPlay CdPlayAgain(uint8_t kept_x, uint8_t kept_y);
void play_track_exit_shim(TrackPlay play);
TrackPlay CdPlayRepeat(uint8_t track, uint8_t kept_x, uint8_t kept_y);

/* The title: src/rondo/menu/title.c. */
typedef struct { CommandCall sound; uint8_t step; } LogoStart;

typedef enum { LOGO_HOLDING, LOGO_SECOND, LOGO_SCRIPT_STARTED, LOGO_DONE, LOGO_ODD_FRAME, LOGO_THIRD } LogoHow;

/* A frame of the logo's fades: which step ran, and what its exit shows. */
typedef struct {
    LogoHow how;
    bool first;
    uint8_t moved;
    uint8_t value;
    uint8_t held;
    uint8_t count;
    uint8_t tick;
} LogoFade;


/* menu/title.c: the sub-modes; each returns what it leaves in the registers (Exit, in front_end.h). */
uint8_t TitleWait(void);
uint8_t TitleEnd(void);
void TitleLastScript(void);
LogoStart TitleLogoStart(void);

/*
 * The disc loaders: each BIOS call they made, in order, with what the game loaded before it,
 * for their wrappers to replay as the registers and flags the calls leave.
 */
typedef enum {
    DISC_AD_STOP, DISC_AD_STAT, DISC_SUM, DISC_MUL, DISC_CD_STAT, DISC_CD_PAUSE, DISC_AD_TRANS,
    DISC_AD_CPLAY, DISC_CD_SEARCH, DISC_CD_READ, DISC_OVERLAY, DISC_SCENE_BANKS, DISC_SCENE_GOTO,
    DISC_SCENE_END
} DiscCallKind;


typedef struct {
    DiscCallKind kind;
    uint8_t given;   /* the byte the game loaded last before the call */
    uint8_t index;   /* DISC_SCENE_GOTO: the list's new low byte, which goes by X */
    bool summed;     /* a subtract since the last call left C and V: sum */
    Sum sum;         /* DISC_SUM: an address's last add, its value left in A */
    union {
        uint8_t control;
        AdStat ad_stat;
        MaMul8u mul;
        CdStat cd_stat;
        CdPause cd_pause;
        AdTrans ad_trans;
        AdCplay ad_cplay;
        CdSearch cd_search;
        CdRead cd_read;
        Cpu after;   /* DISC_OVERLAY: what original code left, which has no C yet */
    } as;
} DiscCall;

enum { DISC_CALLS_KEPT = 32 };

typedef struct {
    unsigned count;
    DiscCall calls[DISC_CALLS_KEPT];
} DiscLog;

typedef struct { bool already; uint8_t sound; DiscLog log; } AdpcmLoadRun;
typedef struct { bool skipped; uint8_t x, y; DiscLog log; } TrackSearch;

AdpcmLoadRun AdpcmLoad(uint8_t sound);
DiscLog AdpcmLoadFixed(uint8_t entry_x);
DiscLog AdpcmPlayFromDisc(uint8_t sound);
TrackSearch CdSearchTrack(uint8_t track, uint8_t kept_x, uint8_t kept_y);
/* A scene overlay's entry run, and the bank add that mapped it, which leaves C and V for it. */
typedef struct { Sum bank; uint8_t pointer_high; Cpu after; } SceneCall;
SceneCall SceneCallOverlay(uint8_t offset);
DiscLog LoadSceneScript(uint8_t scene);
DiscLog SceneReadSectors(uint8_t op);
void abi_SceneCallOverlay(void);
void abi_SceneReadSectors(void);
void disc_log_exit_shim(const DiscLog *log);
uint8_t disc_log_x(const DiscLog *log, uint8_t entry_x);
void adpcm_load_exit_shim(const AdpcmLoadRun *run);
void abi_AdpcmLoadFixed(void);
void abi_CdSearchTrack(void);
void track_search_exit_shim(const TrackSearch *search);
void save_vram_exit_shim(Sum low);
void reset_game_state_exit_shim(void);
void ResetGameState(void);
uint8_t WaitVblankDisplayOff(void);
void abi_StageEnter(void);
void abi_StageLoad(void);

/* The backup file written: the old one deleted, the room counted, and the write if it fits. */
typedef struct {
    BackupAccess removal;
    BackupFree free_space;
    Sum room;          /* the free count's high byte less the borrow from its low byte's compare */
    bool fits;
    BackupAccess write;
} BackupWriteRun;

BackupWriteRun BackupWrite(void);
void backup_write_exit_shim(const BackupWriteRun *run);
BackupFormat BackupFormatMemory(void);
void abi_BackupWrite(void);

/* A stage's load: each call it made, with what the game loaded before it, for the replay. */
typedef struct {
    bool character_loaded;
    uint8_t character;
    uint8_t loaded_before;
    DiscLog character_scene;
    Sum first_sum;
    DiscLog first_scene;
    uint8_t second;
    DiscLog second_scene;
    bool saved;
    Sum vram;
    uint8_t adpcm_index;
    Sum adpcm_sum;
    AdpcmLoadRun adpcm;
    uint8_t track_index;
    uint8_t track;
    TrackSearch search;
} StageLoadRun;

/* A stage entered: its load, or for stage 2 the title's scenes and ADPCM. */
typedef struct {
    StopCall stop;
    bool route_read;
    uint8_t route;
    bool field83_read;
    uint8_t field83;
    bool to_title;
    DiscLog menu_a;
    DiscLog menu_b;
    AdpcmLoadRun menu_adpcm;
    StageLoadRun load;
} StageEnterRun;

StageLoadRun StageLoad(void);
StageEnterRun StageEnter(void);
void stage_load_exit_shim(const StageLoadRun *run);
void stage_enter_exit_shim(const StageEnterRun *run);

/* Richter's ground checks, as his states call them (object/player_terrain.c). */
/* How a read of the two low columns ended. */
typedef struct {
    int blocked_column;
    bool read_made;
    TileRead last;
    uint8_t answer;
    uint8_t row;
} LowColumns;

/* How a read of one column ended, for what the wrapper leaves. */
typedef struct {
    uint8_t column;
    uint8_t row;
    uint8_t below;
    bool read_made;
    TileRead read;
} ColumnRead;

/* How a fall-through check ended. */
typedef struct {
    FloorResult check;
    bool read_made;
    ColumnRead under;
    bool no_floor;
} FootingCheck;

typedef enum { LEDGE_PUSHED, LEDGE_LOW, LEDGE_HIGH, LEDGE_NONE } LedgeEnd;

typedef struct {
    LedgeEnd how;
    uint8_t push;
    LowColumns low;
    int high_column;
} LedgeCheck;

/* How a landing lift ended. */
typedef struct {
    FloorResult check;
    bool lifted;
    ColumnLift lift;
} LandingLift;

void floor_check_exit_shim(FloorResult result);
void column_read_exit_shim(ColumnRead one);
void low_columns_exit_shim(LowColumns low);
FootingCheck RichterHasNoFooting(void);
void footing_exit_shim(FootingCheck footing);
LedgeCheck RichterLedgeCheck(void);
void ledge_check_exit_shim(LedgeCheck check);
LandingLift RichterLandOnFloor(void);
void landing_exit_shim(LandingLift landing);
bool RichterJumpPressed(void);
bool RichterUpHeld(void);

#endif
