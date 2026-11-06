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
Alu NextGameMode(Alu r);
Alu SetGameMode(Alu r);
Alu ResetScoreCounters(Alu r);
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

#endif
