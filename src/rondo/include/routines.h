/* Routines called across files. */
#ifndef RONDO_ROUTINES_H
#define RONDO_ROUTINES_H

#include "machine.h"

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
} PlayerDistance;

/*
 * What a terrain read found, and the point's place in the tile before any handler moved it.
 * TILE_BY_ORIGINAL: another overlay's copy, with no C yet, answered.
 */
typedef enum { TILE_EMPTY_PAGE, TILE_PLAIN, TILE_PAST_HANDLED, TILE_HANDLED, TILE_BY_ORIGINAL } TileKind;

typedef struct {
    TileKind kind;
    uint8_t attribute;  /* the byte read from the page */
    uint8_t page;       /* the page's entry */
    uint8_t high;       /* the tile's high bits, which pick the page */
    uint8_t column;     /* the point's pixel across the tile */
    uint8_t row;        /* the point's pixel down the tile */
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

LookupEnd StageLookup(uint8_t probe, uint8_t object, uint8_t column);
LookupEnd StageLookupAlt(uint8_t probe, uint8_t object, uint8_t column);
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
bool SoundCommand(uint8_t command, uint8_t *busy_or_bank);

/* What SoundSilenceChannel looked at: the effect on a shared channel, when 4 or 5. */
typedef struct { bool looked; uint8_t effect; } ChannelRelease;

/* A product of the driver's shift and add, with the last add's high byte and overflow. */
typedef struct { uint16_t value; bool added; uint8_t high; bool overflow; } SoundProduct;

void SoundSilenceAll(void);
void SoundVolumeFull(void);
void SoundStartEnvelope(uint8_t channel);
ChannelRelease SoundSilenceChannel(uint8_t channel);
SoundProduct SoundMultiply(uint8_t a, uint8_t y);
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

/* A byte add or subtract: the result, its carry as the processor's C, and its overflow. */
typedef struct { uint8_t value; bool carry; bool overflow; } Sum;

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
void SoundStartChannel(uint8_t channel);
uint8_t SoundStartMusic(void);

/* A voice's start moved back: whether it was, the voice compared, and the high byte's add. */
typedef struct { bool moved; bool compared; uint8_t voice; Sum high; } VoiceBack;

VoiceBack SoundVoiceAddressBack(void);
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
uint8_t axis_move_flags_shim(AxisMove move);
void ClearObject(uint8_t object);
void SelectVdcRegister(uint8_t reg);
void SelectVramWriteAddress(void);
void SelectVramData(void);
void ClearVdcControlHigh(void);
void SelectVdcControl(void);
uint8_t WriteScreenTiming(uint8_t index);
uint8_t WriteBandScroll(uint8_t band);
void ResetDisplayControl(void);
uint8_t SoundFrameTick(void);
uint8_t NextRandom(void);
bool RasterStart(uint8_t *effect, uint8_t *last);
void RasterInterrupt(void);
uint8_t BitOfIndex(uint8_t index, uint8_t *byte);
uint8_t RecordEvent(uint8_t event, uint8_t *byte);
void StartScriptSlotKind1(uint8_t script, uint8_t slot);
void StartListChannelKind2(uint8_t list, uint8_t channel);
void StartListChannelKind3(uint8_t list, uint8_t channel);
PlayerDistance DistanceXToPlayer(uint8_t object);
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
Alu ModeBoot(void);
Alu ModeLogo(void);
Alu ModeAttract(void);
Alu ModeAttractPlay(void);
Alu ModeStageLoad(void);
Alu ModeStageLoadScreens(Alu r);
Alu ModeStageLoadContinue(Alu r);
Alu ModeStageDone(Alu r);
Alu ModeStage(void);
Alu ModeStageCleared(void);
Alu StageClearedNext(Alu r);
Alu StageClearBit(Alu r);
Alu ModeDead(void);
Alu ModeBackToTitle(Alu r);
Alu ModeGameOverWait(void);
Alu ModeScreen(void);
Alu ModeEnding(void);
Alu StageEnterNext(Alu r);
uint8_t NextGameMode(void);
void SetGameMode(uint8_t mode);
void ResetScoreCounters(void);
void abi_NextGameMode(void);
void abi_SetGameMode(void);
void abi_ResetScoreCounters(void);
Alu SaveProgress(Alu r);
Alu SaveScore(Alu r);
Alu BackupRead(Alu r);
Alu BackupWrite(Alu r);
Alu AdpcmLoad(Alu r);
Alu AdpcmLoadFixed(Alu r);
Alu CdSearchTrack(Alu r);
Alu LoadSceneScript(Alu r);
Alu SceneCallOverlay(Alu r);
Alu SceneReadSectors(Alu r);
Alu StageEnter(Alu r);
Alu StageLoad(Alu r);
Alu SaveVramToRam(Alu r);

/* The kernel's ADPCM play: src/rondo/audio/adpcm.c. The card's types come from syscard.h. */
#endif
