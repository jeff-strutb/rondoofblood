/*
 * Richter's player overlay (sector $1B9, run at $A000): the routines the files filed from it in
 * name order declare for each other, beside player_objects.h. The defining file is named at each.
 */
#ifndef RONDO_RICHTER_H
#define RONDO_RICHTER_H

#include "machine.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

/* object/richter_state.c */
typedef struct { uint8_t facing; uint8_t high; } FacingSpeed;
typedef struct { uint8_t facing; uint8_t step; uint8_t was; } Nudge;
typedef struct { uint8_t state; uint8_t index; } CrashEnd;
typedef struct { Lift lift; FacingSpeed speed; uint8_t marks; } Knock;
/* RichterMoveStopped: the move's add and the facing it loads into X. */
typedef struct { Sum moved; uint8_t facing; } MovedStopped;
MovedStopped RichterMoveStopped(uint8_t move);
typedef enum { FLOOR_TOO_HIGH, FLOOR_UNMARKED, FLOOR_MARKED, FLOOR_MARKED_DOWN, FLOOR_NOT_DOWN } FloorHow;
/* A look at the floor's marks: what it found, the value its exit shows, and the pad held. */
typedef struct { FloorHow how; uint8_t value; uint8_t held; } MarkedFloor;
MarkedFloor RichterOnMarkedFloor(void);
void marked_floor_exit_shim(MarkedFloor floor);
void nudge_exit_shim(Nudge nudge);
Nudge RichterNudgeA(void);
Nudge RichterNudgeB(void);
Lift RichterLiftThree(void);
void lift_exit_shim(Lift lift);
Sum RichterFallGravity(void);
void fall_gravity_exit_shim(Sum high);
AnimationStep RichterToFall(void);
void to_fall_exit_shim(AnimationStep fall);
void RichterPadOverridesOff(void);
void abi_RichterReadColumns(void);
void abi_RichterSetAnimation(void);
void abi_RichterAnimationTick(void);
void abi_RichterWall(void);
void abi_RichterFloor(void);
uint8_t RichterStepIndex(void);
CrashEnd RichterEndItemCrash(void);
void crash_end_exit_shim(CrashEnd end);
void RichterStopMoving(void);
void RichterStopX(void);
void RichterStartHitScript(uint8_t script);
void hit_script_exit_shim(uint8_t script);
void sound_command_exit_shim(uint8_t command, CommandCall call);   /* audio/sound_calls.c */

/* object/richter_objects.c */
void RichterObjectStopX(uint8_t slot);
void RichterObjectStop(uint8_t slot);
void RichterObjectStopY(uint8_t slot);

/* object/richter_hurt.c */
typedef struct { Ended slots; CommandCall sound; } HurtLandSound;
typedef struct { uint8_t side; uint8_t high; } KnockSpeed;
typedef enum { HIT_EFFECT_NONE, HIT_EFFECT_SLOTS, HIT_EFFECT_COUNTED, HIT_EFFECT_MARKED } HitEffectHow;
/* What RichterHitEffects did: the OBJ_HIT_TAKEN its last TST read, the script, the sound. */
typedef struct { HitEffectHow how; uint8_t taken; uint8_t script; uint8_t command; CommandCall sound; } HitEffects;
typedef struct { bool sounded; CommandCall sound; HitEffects effects; } HitSound;
/* RichterHurtResume: the state before the hit and its frame, the count, and what was given back. */
typedef struct {
    bool given_back; bool ended; uint8_t state; uint8_t index; uint8_t frame; uint8_t count;
    uint8_t resumed_state; uint8_t step; CrashEnd end;
} HurtResume;

/* RichterDebrisOnMarkedColumns: whether the burst ran, its result, the V of the Y add, his Y pulled back. */
typedef struct { bool burst_started; ObjArm burst; bool overflow; uint8_t y_low; } DebrisColumns;

/*
 * object/richter_exits.c: what a routine's instructions and its callees left in the registers and
 * flags, in the order they ran, for its abi_ wrapper to give back. A callee's step replays that
 * callee's own exit; an ENDED step is the routine's own instructions after it (Ended, and X).
 */
typedef enum {
    RX_ENDED, RX_COLUMNS, RX_FLOOR, RX_LANDING, RX_ANIMATION, RX_SOUND, RX_SCRIPT, RX_LIFT, RX_NUDGE,
    RX_FALL, RX_GRAVITY, RX_CRASH_END, RX_COLUMN_READ, RX_TST, RX_SOLID, RX_HIT_EFFECTS, RX_HIT_SOUND,
    RX_SLOTS, RX_DEBRIS, RX_LOG, RX_NOTE_PRESS, RX_ITEM_USE, RX_UNDER_FEET, RX_COLUMN_LIFT, RX_MOVE_STOPPED,
    RX_READ_BELOW, RX_ATTACK, RX_KNOCK, RX_MARKED, RX_FOOTING, RX_LEDGE, RX_HURT_LAND, RX_HURT_RESUME
} RegKind;
typedef struct {
    RegKind kind;
    union {
        struct { Ended e; bool x_set; uint8_t x; } ended;
        ColumnsEnd columns;
        FloorResult floor;
        LandingLift landing;
        AnimationStep animation;
        struct { uint8_t command; CommandCall call; } sound;
        uint8_t script;
        Lift lift;
        Nudge nudge;
        Sum gravity;
        CrashEnd crash_end;
        ColumnRead column;
        struct { uint8_t mask; uint8_t memory; } tst;
        HitEffects hit_effects;
        HitSound hit_sound;
        Ended slots;
        DebrisColumns debris;
        ObjArm arm;
        ItemUse use;
        ColumnLift column_lift;
        struct { Sum moved; uint8_t facing; } move_stopped;
        struct { TileRead read; bool read_made; } read_below;
        AttackStart attack;
        Knock knock;
        MarkedFloor marked;
        FootingCheck footing;
        LedgeCheck ledge;
        HurtLandSound hurt_land;
        HurtResume hurt_resume;
    };
} RegStep;
typedef struct { uint8_t count; RegStep step[24]; } RegLog;
void reg_log_replay(const RegLog *log);
/* The X and Y the log leaves, from the X and Y the routine was entered with: for a callee that takes them
 * (SoundCommand pushes them). Replays on a copy of the registers; cpu is as it was after. */
void rl_xy(const RegLog *log, uint8_t x_in, uint8_t y_in, uint8_t *x, uint8_t *y);
void rl_ended(RegLog *log, Ended e);
void rl_x(RegLog *log, uint8_t x);
void rl_ended_x(RegLog *log, Ended e, uint8_t x);
void rl_tst(RegLog *log, uint8_t mask, uint8_t memory);
void rl_columns(RegLog *log, ColumnsEnd r);
void rl_floor(RegLog *log, FloorResult r);
void rl_landing(RegLog *log, LandingLift r);
void rl_animation(RegLog *log, AnimationStep r);
void rl_sound(RegLog *log, uint8_t command, CommandCall r);
void rl_script(RegLog *log, uint8_t script);
void rl_lift(RegLog *log, Lift r);
void rl_nudge(RegLog *log, Nudge r);
void rl_fall(RegLog *log, AnimationStep r);
void rl_gravity(RegLog *log, Sum r);
void rl_crash_end(RegLog *log, CrashEnd r);
void rl_column(RegLog *log, ColumnRead r);
void rl_solid(RegLog *log, ColumnRead r);
void rl_hit_effects(RegLog *log, HitEffects r);
void rl_hit_sound(RegLog *log, HitSound r);
void rl_slots(RegLog *log, Ended r);
void rl_debris(RegLog *log, DebrisColumns r);
void rl_note_press(RegLog *log, ObjArm r);
void rl_item_use(RegLog *log, ItemUse r);
void rl_under_feet(RegLog *log, ColumnRead r);
void rl_column_lift(RegLog *log, ColumnLift r);
void rl_move_stopped(RegLog *log, Sum moved, uint8_t facing);
void rl_read_below(RegLog *log, TileRead r, bool read_made);
void rl_attack(RegLog *log, AttackStart r);
void rl_knock(RegLog *log, Knock r);
void rl_marked(RegLog *log, MarkedFloor r);
void rl_footing(RegLog *log, FootingCheck r);
void rl_ledge(RegLog *log, LedgeCheck r);
void rl_hurt_land(RegLog *log, HurtLandSound r);
void rl_hurt_resume(RegLog *log, HurtResume r);
void hurt_resume_exit_shim(HurtResume r);
/* object/player_terrain.c and object/player_input.c */
uint8_t solid_answer(ColumnRead one);
uint8_t column_answer(ColumnRead one);
void read_or_nothing_exit_shim(TileRead read, bool read_made);
ColumnRead RichterReadUnderFeet(void);
TileRead RichterReadBelow(uint8_t row, bool carry);
ObjArm RichterNoteItemPress(void);
EndedTst richter_note_item_press_exit_shim(ObjArm r);
void attack_exit_shim(AttackStart attack);
AttackStart RichterAttackWithAnimation(uint8_t animation);
Knock RichterKnockBack(void);
void knock_exit_shim(Knock knock);
FloorResult RichterFloorCheck(void);
ColumnRead RichterSolidUnderColumn(uint8_t column);

void RichterClearSlotKindFour(uint8_t slot);
Ended RichterReplaceSlotsOneTwo(void);
HurtLandSound RichterHurtLandSound(uint8_t y);
void hurt_land_sound_exit_shim(HurtLandSound r);
uint8_t RichterKnockSpeedX(uint8_t side);
void knock_speed_x_exit_shim(uint8_t side, uint8_t high);
KnockSpeed RichterKnockSpeedXFromHit(void);
void RichterHurtToCrouch(void);
HitEffects RichterHitEffects(uint8_t x, uint8_t y);
void hit_effects_exit_shim(HitEffects r);
HitSound RichterHitSoundAndEffects(uint8_t x, uint8_t y);
void hit_sound_exit_shim(HitSound r);
HurtResume RichterHurtResume(void);
void RichterHurtKnockUp(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtDown(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtFromStand(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtFromCrouch(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtInAir(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtClimbKindFive(RegLog *log, uint8_t y);
void RichterHurtWhileClimbing(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtKindThree(RegLog *log, uint8_t y);
void RichterHurtKindFour(RegLog *log, uint8_t y);
void RichterHurtStart(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtThrown(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtThrownLanded(RegLog *log);
void RichterHurtSlide(RegLog *log);
void RichterHurtSlideStart(RegLog *log);
void RichterHurtHoldFrame(RegLog *log);
void RichterHurtDownStep(RegLog *log);

/* object/debris_burst.c */
ObjArm RichterStartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry);
Ended start_debris_burst_exit_shim(ObjArm r);
void abi_RichterStartDebrisBurst(void);

/* object/richter_fall.c */
DebrisColumns RichterDebrisOnMarkedColumns(uint8_t x, uint8_t y);
void debris_columns_exit_shim(DebrisColumns r);
void abi_RichterDebrisOnMarkedColumns(void);

void abi_RichterObjectStopX(void);
void abi_RichterObjectStop(void);
void abi_RichterObjectStopY(void);
void abi_RichterClearSlotKindFour(void);
void abi_RichterReplaceSlotsOneTwo(void);
void abi_RichterHurtLandSound(void);
void abi_RichterKnockSpeedX(void);
void abi_RichterKnockSpeedXFromHit(void);
void abi_RichterHurtToCrouch(void);
void abi_RichterHitEffects(void);
void abi_RichterHitSoundAndEffects(void);
void abi_RichterHurtResume(void);
void abi_RichterHurtKnockUp(void);
void abi_RichterHurtDown(void);
void abi_RichterHurtFromStand(void);
void abi_RichterHurtFromCrouch(void);
void abi_RichterHurtInAir(void);
void abi_RichterHurtClimbKindFive(void);
void abi_RichterHurtWhileClimbing(void);
void abi_RichterHurtKindThree(void);
void abi_RichterHurtKindFour(void);
void abi_RichterHurtStart(void);
void abi_RichterHurtThrown(void);
void abi_RichterHurtThrownLanded(void);
void abi_RichterHurtSlide(void);
void abi_RichterHurtSlideStart(void);
void abi_RichterHurtHoldFrame(void);
void abi_RichterHurtDownStep(void);
void RichterHurtLandOnMarked(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtLandOnMarkedFeet(RegLog *log, uint8_t x, uint8_t y);
void RichterHurtKnocked(RegLog *log, uint8_t x, uint8_t y);
void RichterStateHurt(RegLog *log, uint8_t y);
void abi_RichterHurtLandOnMarked(void);
void abi_RichterHurtLandOnMarkedFeet(void);
void abi_RichterHurtKnocked(void);
void abi_RichterStateHurt(void);

/* object/richter_fall.c, the states in the air */
MarkedFloor RichterOnMarkedUnderFeet(RegLog *log);
void RichterAirTurnSpeed(RegLog *log);
void RichterAirTurnFacingLeft(RegLog *log);
void RichterAirTurnStill(RegLog *log);
void RichterAirControl(RegLog *log);
void RichterFallAttacking(RegLog *log);
bool RichterDownOnFirstColumnFloor(RegLog *log);
void RichterAirStep2(RegLog *log);
void RichterAirStep3(RegLog *log);
void RichterStateLandCrouch(RegLog *log);
void RichterLandOnMarkedFeet(RegLog *log, uint8_t x, uint8_t y);
void RichterLandOnMarked(RegLog *log, uint8_t x, uint8_t y);
void RichterLandAttacking(RegLog *log);
void RichterLandHard(RegLog *log, uint8_t x, uint8_t y);
void RichterLand(RegLog *log, uint8_t x, uint8_t y);
void RichterLandOnColumn(RegLog *log, uint8_t column, uint8_t x, uint8_t y);
void RichterAirAttack(RegLog *log);
void RichterStateFall(RegLog *log, uint8_t x, uint8_t y);
void RichterAirStep1(RegLog *log, uint8_t x, uint8_t y);
void RichterAirStep0(RegLog *log, uint8_t x, uint8_t y);
void RichterStateAir(RegLog *log, uint8_t y);
void abi_RichterOnMarkedUnderFeet(void);
void abi_RichterAirTurnSpeed(void);
void abi_RichterAirTurnFacingLeft(void);
void abi_RichterAirTurnStill(void);
void abi_RichterAirControl(void);
void abi_RichterFallAttacking(void);
void abi_RichterDownOnFirstColumnFloor(void);
void abi_RichterAirStep2(void);
void abi_RichterAirStep3(void);
void abi_RichterStateLandCrouch(void);
void abi_RichterLandOnMarkedFeet(void);
void abi_RichterLandOnMarked(void);
void abi_RichterLandAttacking(void);
void abi_RichterLandHard(void);
void abi_RichterLand(void);
void abi_RichterLandOnColumn(void);
void abi_RichterAirAttack(void);
void abi_RichterStateFall(void);
void abi_RichterAirStep1(void);
void abi_RichterAirStep0(void);
void abi_RichterStateAir(void);

#endif
