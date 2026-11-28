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
typedef enum {
    INVINCIBLE_OFF, INVINCIBLE_OFF_FRAME, INVINCIBLE_ENDED, INVINCIBLE_RUNNING, INVINCIBLE_MARK
} InvincibleHow;
/* A frame of the invincibility: how it went, the A compared with $10, the sounds and the mark. */
typedef struct {
    InvincibleHow how; uint8_t value; uint8_t compared; bool first_sound; bool second_sound;
    CommandCall first; CommandCall second; uint8_t mark; uint16_t scripts;
} InvincibleTick;
void invincible_exit_shim(InvincibleTick tick);
typedef struct { uint8_t facing; uint8_t high; } FacingSpeed;
typedef struct { uint8_t facing; uint8_t step; uint8_t was; } Nudge;
typedef struct { uint8_t state; uint8_t index; } CrashEnd;
typedef struct { Lift lift; FacingSpeed speed; uint8_t marks; } Knock;
typedef struct {
    Lift lift;
    bool moving;
    bool same_way;
    AnimationStep animation;
    FacingSpeed speed;
} AirStart;
/* An item crash's frame and the add that found its entry. */
typedef struct { Sum entry; uint8_t frame; } CrashFrame;
/* A hurt off a ledge: item 5's crash with its animation, or the nudge and the hurt jump. */
typedef struct { bool crash; AnimationStep animation; Nudge nudge; HurtAir hurt; } LedgeHurt;
/* A jump off a ledge: the nudge, and taking to the air. */
typedef struct { Nudge nudge; AirStart air; } LedgeJump;
typedef enum { UP_WHIP_JUMPED, UP_WHIP_NO_LADDER, UP_WHIP_LADDER } UpWhipHow;
/* The whip's reach up a ledge: the pad, the jump off it, or the ladder and the nudge onto it. */
typedef struct {
    UpWhipHow how; uint8_t held; LedgeJump jump; LadderCheck ladder; Nudge nudge; AnimationStep fall;
} UpWhip;
/* object/player_leaves.c and object/player_terrain.c */
typedef struct {
    Sum row;
    TileRead read;
} OffsetRead;
/* How the jump-or-climb check ended. */
typedef struct {
    bool pressed;
    bool up_held;
    uint8_t held;
    LadderCheck ladder;
} JumpOrClimb;
OffsetRead RichterReadAtOffset(uint8_t offset);
void offset_read_exit_shim(OffsetRead r);
JumpOrClimb RichterJumpOrClimb(void);
void jump_or_climb_exit_shim(JumpOrClimb result);
ObjArm RichterTurnOnPad(void);
EndedTst richter_turn_on_pad_exit_shim(ObjArm r);
uint8_t RichterLatchButtons(void);
CrashFrame RichterCrashFrame(void);
void crash_frame_exit_shim(CrashFrame frame);
FacingSpeed RichterSpeedWalk(void);
void speed_exit_shim(FacingSpeed set);
LedgeHurt RichterLedgeHurtA(void);
LedgeHurt RichterLedgeHurtB(void);
void ledge_hurt_exit_shim(LedgeHurt hurt);
UpWhip RichterUpWhipLeft(void);
UpWhip RichterUpWhipRight(void);
void up_whip_exit_shim(UpWhip whip);
AirStart RichterToAir(void);
void air_exit_shim(AirStart air);
bool RichterJumpPressed(void);
typedef struct {
    uint8_t value;
    bool stands;
    AnimationStep stand;
} CountDown;
CountDown CountDownThenStand(void);
void count_down_stand_exit_shim(CountDown r);
ItemCrash RichterTryItemCrash(void);
void item_crash_exit_shim(ItemCrash crash);
typedef enum {
    CRASH_STAND_STOOD, CRASH_STAND_ENDED, CRASH_STAND_FELL, CRASH_STAND_JUMPED, CRASH_STAND_UP_WHIP,
    CRASH_STAND_SPAWNED, CRASH_STAND_NO_SLOT
} CrashStandHow;
/* Standing through the item crash: the turn, the timer, each check, and the slot spawned. */
typedef struct {
    CrashStandHow how;
    uint8_t facing; uint8_t held; uint8_t timer; uint8_t state; uint8_t bumped;
    ColumnsEnd columns; FootingCheck footing; bool jump;
    AnimationStep stand; CrashEnd end; AirStart air; UpWhip whip; uint8_t slot; uint8_t seen;
    bool from_timer;    /* entered at $A371, RichterStandStep2Timer: no turn first */
} CrashStand;
CrashStand RichterStandStep2Timer(void);
void crash_stand_exit_shim(CrashStand st);
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

/* object/player_input.c */
ItemUse RichterUseItemPaid(void);
/* RichterUseItemOrUpPress: the use, whether the up press was tested and used, and zPadPressed as read. */
typedef struct { ItemUse use; bool tested; bool paid; uint8_t pressed; } UpPressUse;
UpPressUse RichterUseItemOrUpPress(void);
void up_press_use_exit_shim(UpPressUse r);
void abi_RichterUseItemPaid(void);
void abi_RichterUseItemOrUpPress(void);

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

/*
 * object/richter_exits.c: what a routine's instructions and its callees left in the registers and
 * flags, in the order they ran, for its abi_ wrapper to give back. A callee's step replays that
 * callee's own exit; an ENDED step is the routine's own instructions after it (Ended, and X).
 */
typedef enum {
    RX_ENDED, RX_COLUMNS, RX_FLOOR, RX_LANDING, RX_ANIMATION, RX_SOUND, RX_SCRIPT, RX_LIFT, RX_NUDGE,
    RX_FALL, RX_GRAVITY, RX_CRASH_END, RX_COLUMN_READ, RX_TST, RX_SOLID, RX_HIT_EFFECTS, RX_HIT_SOUND,
    RX_SLOTS, RX_DEBRIS, RX_LOG, RX_NOTE_PRESS, RX_ITEM_USE, RX_UNDER_FEET, RX_COLUMN_LIFT, RX_MOVE_STOPPED,
    RX_READ_BELOW, RX_ATTACK, RX_KNOCK, RX_MARKED, RX_FOOTING, RX_LEDGE, RX_HURT_LAND, RX_HURT_RESUME, RX_OFFSET_READ, RX_ITEM_CRASH,
    RX_LEDGE_HURT, RX_UP_WHIP, RX_JUMP_CLIMB, RX_TURN_PAD, RX_CRASH_FRAME, RX_SPEED, RX_AIR, RX_CRASH_STAND,
    RX_UP_PRESS, RX_JUMP_PRESSED, RX_COUNTDOWN
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
        OffsetRead offset_read;
        ItemCrash item_crash;
        LedgeHurt ledge_hurt;
        UpWhip up_whip;
        JumpOrClimb jump_climb;
        CrashFrame crash_frame;
        FacingSpeed speed;
        AirStart air;
        CrashStand crash_stand;
        UpPressUse up_press;
        bool jump_pressed;
        CountDown countdown;
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
void rl_offset_read(RegLog *log, OffsetRead r);
void rl_item_crash(RegLog *log, ItemCrash r);
void rl_ledge_hurt(RegLog *log, LedgeHurt r);
void rl_up_whip(RegLog *log, UpWhip r);
void rl_jump_climb(RegLog *log, JumpOrClimb r);
void rl_turn_pad(RegLog *log, ObjArm r);
void rl_crash_frame(RegLog *log, CrashFrame r);
void rl_speed(RegLog *log, FacingSpeed r);
void rl_air(RegLog *log, AirStart r);
void rl_crash_stand(RegLog *log, CrashStand r);
void rl_up_press(RegLog *log, UpPressUse r);
void rl_jump_pressed(RegLog *log, bool r);
void rl_countdown(RegLog *log, CountDown r);

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

/* object/richter_climb.c, states 5 to 8 */
void RichterClimbBack(RegLog *log, uint8_t way);
void RichterClimbOn(RegLog *log, uint8_t way);
void RichterClimbPad(RegLog *log);
void RichterClimbStepSpeed(RegLog *log);
void RichterClimbAttack(RegLog *log);
void RichterClimbLowStep0(RegLog *log);
void RichterClimbHighStep0(RegLog *log);
void RichterClimbStep1(RegLog *log);
void RichterClimbReach(RegLog *log);
void RichterClimbPullUp(RegLog *log);
void RichterClimbStandTimer(RegLog *log);
void RichterStateClimbLow(RegLog *log);
void RichterStateClimbHigh(RegLog *log);
void abi_RichterClimbBack(void);
void abi_RichterClimbOn(void);
void abi_RichterClimbPad(void);
void abi_RichterClimbStepSpeed(void);
void abi_RichterClimbAttack(void);
void abi_RichterClimbLowStep0(void);
void abi_RichterClimbHighStep0(void);
void abi_RichterClimbStep1(void);
void abi_RichterClimbReach(void);
void abi_RichterClimbPullUp(void);
void abi_RichterClimbStandTimer(void);
void abi_RichterStateClimbLow(void);
void abi_RichterStateClimbHigh(void);


/* object/player_leaves.c, object/player_states.c, object/motion.c, object/richter_state.c: leaves and exits */
ObjArm SetVelocityXFromTable(uint8_t slot, uint8_t entry);
ObjArm SetVelocityYFromTable(uint8_t slot, uint8_t entry);
Ended set_velocity_from_table_exit_shim(ObjArm r);
ObjArm NegateVelocityXIfFacingLeft(uint8_t slot);
Ended negate_velocity_exit_shim(ObjArm r);
ObjArm ClearObjectInYAndCopyPlace(uint8_t source, uint8_t target);
Ended clear_in_y_exit_shim(ObjArm r);
ObjArm ClearObjectIfOffRange(uint8_t slot);
Ended clear_object_exit_shim(ObjArm r);
ObjArm ObjectTouchesPlayerBox(uint8_t slot);
Ended object_touches_player_box_exit_shim(ObjArm r);
ObjArm NudgeAngleByVelocityAndScroll(uint8_t slot);
Ended nudge_angle_by_velocity_and_scroll_exit_shim(ObjArm r);
ObjArm MoveObjectUpBySigned(uint8_t slot);
Ended move_object_up_exit_shim(ObjArm r);
ObjArm RichterFlagsAgainstFour(void);
EndedX richter_flags_against_four_exit_shim(ObjArm r);
ObjArm RichterStateDoubled(void);
EndedX richter_state_doubled_exit_shim(ObjArm r);
void axis_move_exit_shim(AxisMove move);
void negate_exit_shim(Sum last);
typedef struct { CommandCall sound; } Death;
Death RichterDeathSound(uint8_t x, uint8_t y);
void death_exit_shim(Death death);
TileRead StageTileRead(void);

/* object/richter_objects.c, bank $6E: bodies on the register file */
void RichterDeathPointStep(void);
void abi_RichterDeathPointStep(void);
void RichterDeathPointRow(void);
void abi_RichterDeathPointRow(void);
void RichterDeathFrame(void);
void abi_RichterDeathFrame(void);
void RichterDeathPieceFrame(void);
void abi_RichterDeathPieceFrame(void);
void RichterDeathObjects(void);
void abi_RichterDeathObjects(void);
void RichterDeathPieceStart(void);
void abi_RichterDeathPieceStart(void);
void RichterDeathPieceMove(void);
void abi_RichterDeathPieceMove(void);
void RichterDeathPieceFly(void);
void abi_RichterDeathPieceFly(void);
void RichterDeathPieceFall(void);
void abi_RichterDeathPieceFall(void);
void RichterDeathPieceEnd(void);
void abi_RichterDeathPieceEnd(void);
void RichterCompanionKind1(void);
void abi_RichterCompanionKind1(void);
void RichterCompanionKind3(void);
void abi_RichterCompanionKind3(void);
void RichterCompanionAtPlayer(void);
void abi_RichterCompanionAtPlayer(void);
void RichterCompanionKind4(void);
void abi_RichterCompanionKind4(void);
void RichterCompanionByKind(void);
void abi_RichterCompanionByKind(void);
void RichterCompanions(void);
void abi_RichterCompanions(void);
void RichterObjectByKind(void);
void abi_RichterObjectByKind(void);
void RichterObjects(void);
void abi_RichterObjects(void);
void RichterObjectMarkThenFromPlayer(void);
void abi_RichterObjectMarkThenFromPlayer(void);
void RichterWhipSpark(void);
void abi_RichterWhipSpark(void);
void RichterWhipStrike(void);
void abi_RichterWhipStrike(void);
void RichterWhipFrame(void);
void abi_RichterWhipFrame(void);
void RichterKind22Frame(void);
void abi_RichterKind22Frame(void);
void RichterObjectStartAnimation(void);
void abi_RichterObjectStartAnimation(void);
void RichterObjectFallCapped(void);
void abi_RichterObjectFallCapped(void);
void RichterKind2Fly(void);
void abi_RichterKind2Fly(void);
void RichterKind2Frame(void);
void abi_RichterKind2Frame(void);
void RichterKind3Trail(void);
void abi_RichterKind3Trail(void);
void RichterKind3Count(void);
void abi_RichterKind3Count(void);
void RichterKind3Move(void);
void abi_RichterKind3Move(void);
void RichterKind3Out(void);
void abi_RichterKind3Out(void);
void RichterKind3Back(void);
void abi_RichterKind3Back(void);
void RichterKind3Drop(void);
void abi_RichterKind3Drop(void);
void RichterKind3Rise(void);
void abi_RichterKind3Rise(void);
void RichterKind3Start(void);
void abi_RichterKind3Start(void);
void RichterKind3Frame(void);
void abi_RichterKind3Frame(void);
void RichterKind9Frame(void);
void abi_RichterKind9Frame(void);
void RichterKind5Throw(void);
void abi_RichterKind5Throw(void);
void RichterKind5Spread(void);
void abi_RichterKind5Spread(void);
void RichterKind5Start(void);
void abi_RichterKind5Start(void);
void RichterKind5Fly(void);
void abi_RichterKind5Fly(void);
void RichterKind5Stuck(void);
void abi_RichterKind5Stuck(void);
void RichterObjectFall38(void);
void abi_RichterObjectFall38(void);
void RichterKind5Frame(void);
void abi_RichterKind5Frame(void);
void RichterKind6Start(void);
void abi_RichterKind6Start(void);
void RichterKind6Circle(void);
void abi_RichterKind6Circle(void);
void RichterKind6Hold(void);
void abi_RichterKind6Hold(void);
void RichterKind6Count(void);
void abi_RichterKind6Count(void);
void RichterKind6Wait(void);
void abi_RichterKind6Wait(void);
void RichterKind6Gather(void);
void abi_RichterKind6Gather(void);
void RichterKind6Fade(void);
void abi_RichterKind6Fade(void);
void RichterKind6Frame(void);
void abi_RichterKind6Frame(void);
void RichterKind7End(void);
void abi_RichterKind7End(void);
void RichterKind7Frame(void);
void abi_RichterKind7Frame(void);
void RichterClaimKind3Slot(void);
void abi_RichterClaimKind3Slot(void);
void RichterObjectProbe(void);
void abi_RichterObjectProbe(void);
void RichterObjectProbeBelow(void);
void abi_RichterObjectProbeBelow(void);
void RichterObjectHitAndMove(void);
void abi_RichterObjectHitAndMove(void);
void RichterKind4LandMarked(void);
void abi_RichterKind4LandMarked(void);
void RichterKind4Fall(void);
void abi_RichterKind4Fall(void);
void RichterKind4Start(void);
void abi_RichterKind4Start(void);
void RichterKind4Landed(void);
void abi_RichterKind4Landed(void);
void RichterKind4Splash(void);
void abi_RichterKind4Splash(void);
void RichterKind4Spawn(void);
void abi_RichterKind4Spawn(void);
void RichterKind4Burn(void);
void abi_RichterKind4Burn(void);
void RichterKind4Drop(void);
void abi_RichterKind4Drop(void);
void RichterKind4Frame(void);
void abi_RichterKind4Frame(void);
void RichterKind10Frame(void);
void abi_RichterKind10Frame(void);
void RichterObjectMove(void);
void abi_RichterObjectMove(void);
void RichterObjectScrollAndMove(void);
void abi_RichterObjectScrollAndMove(void);
void RichterAngleToObject(void);
void abi_RichterAngleToObject(void);
void RichterKind11Start(void);
void abi_RichterKind11Start(void);
void RichterKind11Move(void);
void abi_RichterKind11Move(void);
void RichterKind11Fly(void);
void abi_RichterKind11Fly(void);
void RichterKind11Frame(void);
void abi_RichterKind11Frame(void);
void RichterKind6CircleStart(void);
void abi_RichterKind6CircleStart(void);
void RichterKind6CircleStep(void);
void abi_RichterKind6CircleStep(void);
void RichterKind12Start(void);
void abi_RichterKind12Start(void);
void RichterKind12Frame(void);
void abi_RichterKind12Frame(void);
void RichterKind8Fly(void);
void abi_RichterKind8Fly(void);
void RichterKind8Frame(void);
void abi_RichterKind8Frame(void);
void RichterKind13Pieces(void);
void abi_RichterKind13Pieces(void);
void RichterKind13Place(void);
void abi_RichterKind13Place(void);
void RichterKind13AtPlayer(void);
void abi_RichterKind13AtPlayer(void);
void RichterKind13Start(void);
void abi_RichterKind13Start(void);
void RichterKind13Fly(void);
void abi_RichterKind13Fly(void);
void RichterKind13Hold(void);
void abi_RichterKind13Hold(void);
void RichterKind13Swing(void);
void abi_RichterKind13Swing(void);
void RichterKind13Frame(void);
void abi_RichterKind13Frame(void);
void RichterKind14Frame(void);
void abi_RichterKind14Frame(void);
void RichterKind15Shot(void);
void abi_RichterKind15Shot(void);
void RichterKind15Frame(void);
void abi_RichterKind15Frame(void);
void RichterFindTarget(void);
void abi_RichterFindTarget(void);
void RichterKind16Start(void);
void abi_RichterKind16Start(void);
void RichterKind16Aim(void);
void abi_RichterKind16Aim(void);
void RichterKind16Fly(void);
void abi_RichterKind16Fly(void);
void RichterKind16Frame(void);
void abi_RichterKind16Frame(void);
void RichterKind17Frame(void);
void abi_RichterKind17Frame(void);
void RichterKind18Pair(void);
void abi_RichterKind18Pair(void);
void RichterKind18Mark(void);
void abi_RichterKind18Mark(void);
void RichterKind18End(void);
void abi_RichterKind18End(void);
void RichterKind18Start(void);
void abi_RichterKind18Start(void);
void RichterKind18Fly(void);
void abi_RichterKind18Fly(void);
void RichterKind18Hold(void);
void abi_RichterKind18Hold(void);
void RichterKind18Rise(void);
void abi_RichterKind18Rise(void);
void RichterKind18Fade(void);
void abi_RichterKind18Fade(void);
void RichterKind18Points(void);
void abi_RichterKind18Points(void);
void RichterKind18Frame(void);
void abi_RichterKind18Frame(void);
void RichterObjectNegateVelocityY(void);
void abi_RichterObjectNegateVelocityY(void);
void RichterKind19AtPlayer(void);
void abi_RichterKind19AtPlayer(void);
void RichterKind19Start(void);
void abi_RichterKind19Start(void);
void RichterKind19Hold(void);
void abi_RichterKind19Hold(void);
void RichterKind19Fly(void);
void abi_RichterKind19Fly(void);
void RichterKind19Frame(void);
void abi_RichterKind19Frame(void);
void RichterKind20Struck(void);
void abi_RichterKind20Struck(void);
void RichterKind20Swing(void);
void abi_RichterKind20Swing(void);
void RichterKind20Frame(void);
void abi_RichterKind20Frame(void);
uint8_t RichterObjectSlotLast(void);
void abi_RichterObjectSlotLast(void);
uint8_t RichterObjectSlot(void);

#endif
