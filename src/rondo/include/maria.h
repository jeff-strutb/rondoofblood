/*
 * Maria's player overlay (sectors 01DD-01E4, loaded by scene $01 at $A000-$DFFF: $A000-$BFFF bank $6D, $C000-$DFFF
 * bank $6E): what her own files (object/maria_*.c) share.
 *
 * Regs is what one of her routines leaves in the registers and flags: each field counts only when its bit is in
 * `set`; the others are as they stood when the routine was entered, or as the call it ended in left them (the
 * abi_ wrapper gives that call's exit first, then this). It is the register file the assembly keeps, carried as a
 * value: a body sets it where the assembly's instruction sets the register, and nowhere else.
 */
#ifndef RONDO_MARIA_H
#define RONDO_MARIA_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum { R_A = 0x01, R_X = 0x02, R_Y = 0x04, R_NZ = 0x08, R_C = 0x10, R_V = 0x20 };

typedef struct {
    uint8_t a, x, y;
    uint8_t nz;     /* the value N and Z come from */
    bool n_on;      /* N set although nz is zero (a TST whose memory had bit 7) */
    bool c, v;
    uint8_t set;
} Regs;

/* Nothing set: every register and flag as it was. */
static inline Regs regs_kept(void)
{
    Regs r = { 0 };

    return r;
}

/* N and Z from a value (an INC, a DEC, an AND, a compare's difference). */
static inline Regs r_nz(Regs r, uint8_t value)
{
    r.nz = value;
    r.n_on = false;
    r.set |= R_NZ;
    return r;
}

/* LDA, PLA, TXA and the like: A, with N and Z from it. */
static inline Regs r_lda(Regs r, uint8_t value)
{
    r.a = value;
    r.set |= R_A;
    return r_nz(r, value);
}

/* LDX, INX, TAX: X, with N and Z from it. */
static inline Regs r_ldx(Regs r, uint8_t value)
{
    r.x = value;
    r.set |= R_X;
    return r_nz(r, value);
}

/* LDY, INY, TAY: Y, with N and Z from it. */
static inline Regs r_ldy(Regs r, uint8_t value)
{
    r.y = value;
    r.set |= R_Y;
    return r_nz(r, value);
}

/* CLA, CLX, CLY: the register only; the flags stand. */
static inline Regs r_a_only(Regs r, uint8_t value)
{
    r.a = value;
    r.set |= R_A;
    return r;
}

static inline Regs r_x_only(Regs r, uint8_t value)
{
    r.x = value;
    r.set |= R_X;
    return r;
}

static inline Regs r_y_only(Regs r, uint8_t value)
{
    r.y = value;
    r.set |= R_Y;
    return r;
}

static inline Regs r_c(Regs r, bool c)
{
    r.c = c;
    r.set |= R_C;
    return r;
}

static inline Regs r_v(Regs r, bool v)
{
    r.v = v;
    r.set |= R_V;
    return r;
}

/* CMP, CPX, CPY: N and Z from the difference, C from no borrow. */
static inline Regs r_cmp(Regs r, uint8_t reg, uint8_t operand)
{
    return r_c(r_nz(r, (uint8_t)(reg - operand)), reg >= operand);
}

/* TST #mask, memory: Z from the AND, N and V bits 7 and 6 of the memory. */
static inline Regs r_tst(Regs r, uint8_t mask, uint8_t memory)
{
    bool zero = (mask & memory) == 0;

    r = r_nz(r, zero ? 0 : (uint8_t)((memory & 0x80) | 0x01));
    r.n_on = zero && (memory & 0x80) != 0;
    return r_v(r, (memory & 0x40) != 0);
}

/* What `after` left over what `before` left: a field `after` did not set is `before`'s. */
static inline Regs r_then(Regs before, Regs after)
{
    if (!(after.set & R_A)) {
        after.a = before.a;
    }
    if (!(after.set & R_X)) {
        after.x = before.x;
    }
    if (!(after.set & R_Y)) {
        after.y = before.y;
    }
    if (!(after.set & R_NZ)) {
        after.nz = before.nz;
        after.n_on = before.n_on;
    }
    if (!(after.set & R_C)) {
        after.c = before.c;
    }
    if (!(after.set & R_V)) {
        after.v = before.v;
    }
    after.set |= before.set;
    return after;
}

/* The wrapper's last step: what the routine set, into the processor. */
static inline void regs_exit(Regs r)
{
    if (r.set & R_A) {
        cpu.a = r.a;
    }
    if (r.set & R_X) {
        cpu.x = r.x;
    }
    if (r.set & R_Y) {
        cpu.y = r.y;
    }
    if (r.set & R_NZ) {
        abi_nz(r.nz);
        if (r.n_on) {
            abi_flag(P_N, true);
        }
    }
    if (r.set & R_C) {
        abi_flag(P_C, r.c);
    }
    if (r.set & R_V) {
        abi_flag(P_V, r.v);
    }
}

/*
 * MExit: a routine's exit when it passes through calls whose exits have their own shims: the registers set before
 * each such call, the call's exit, and the registers set after the last. The wrapper gives them in that order.
 */
typedef enum { MX_ANIMATION, MX_SOUND, MX_SCRIPT, MX_COLUMNS, MX_FOOTING, MX_LANDING, MX_LEDGE, MX_CRASH, MX_TURN, MX_FLOOR, MX_UP_MARKED, MX_READ_BELOW } MxKind;

typedef struct {
    MxKind kind;
    Regs regs;              /* set before the call (a SoundCommand's A, X and Y among them) */
    AnimationStep animation;
    uint8_t command;
    CommandCall call;
    uint8_t script;
    union {
        ColumnsEnd columns;
        FootingCheck footing;
        LandingLift landing;
        MariaLedge ledge;
        ItemCrash crash;
        TurnWindow turn;
        FloorResult floor;
        UpOrMarked up_marked;
        TileRead read;
    } u;
    bool carry;             /* MX_READ_BELOW: the row's carry, which says no read was made */
} MxStep;

/*
 * elsewhere: a dispatch read a word that is none of its table's own and went to the routine at it by its address;
 * that routine's wrapper has left the processor, and nothing more is given. (The tables are the overlay's bytes from
 * the disc; only a change to them sends a jump elsewhere, and then the exits before the jump are not given again.)
 */
typedef struct {
    uint8_t n;
    MxStep step[6];
    Regs last;
    bool elsewhere;
} MExit;

void hit_script_exit_shim(uint8_t script);                         /* object/richter_state.c */
void item_crash_exit_shim(ItemCrash crash);                        /* object/player_input.c */

static inline MExit mx_none(void)
{
    MExit e = { 0 };

    return e;
}

static inline void mx_regs(MExit *e, Regs r)
{
    e->last = r_then(e->last, r);
}

static inline void mx_push(MExit *e, MxStep s)
{
    s.regs = e->last;
    if (e->n < 6) {
        e->step[e->n++] = s;
    }
    e->last = regs_kept();
}

static inline void mx_animation(MExit *e, AnimationStep animation)
{
    MxStep s = { 0 };

    s.kind = MX_ANIMATION;
    s.animation = animation;
    mx_push(e, s);
}

/* A SoundCommand: A the command (set here), X and Y as mx_regs gave them. */
static inline void mx_sound(MExit *e, uint8_t command, CommandCall call)
{
    MxStep s = { 0 };

    mx_regs(e, r_a_only(regs_kept(), command));
    s.kind = MX_SOUND;
    s.command = command;
    s.call = call;
    mx_push(e, s);
}

static inline void mx_columns(MExit *e, ColumnsEnd columns)
{
    MxStep s = { 0 };

    s.kind = MX_COLUMNS;
    s.u.columns = columns;
    mx_push(e, s);
}

static inline void mx_footing(MExit *e, FootingCheck footing)
{
    MxStep s = { 0 };

    s.kind = MX_FOOTING;
    s.u.footing = footing;
    mx_push(e, s);
}

static inline void mx_landing(MExit *e, LandingLift landing)
{
    MxStep s = { 0 };

    s.kind = MX_LANDING;
    s.u.landing = landing;
    mx_push(e, s);
}

static inline void mx_ledge(MExit *e, MariaLedge ledge)
{
    MxStep s = { 0 };

    s.kind = MX_LEDGE;
    s.u.ledge = ledge;
    mx_push(e, s);
}

static inline void mx_crash(MExit *e, ItemCrash crash)
{
    MxStep s = { 0 };

    s.kind = MX_CRASH;
    s.u.crash = crash;
    mx_push(e, s);
}

static inline void mx_turn(MExit *e, TurnWindow turn)
{
    MxStep s = { 0 };

    s.kind = MX_TURN;
    s.u.turn = turn;
    mx_push(e, s);
}

static inline void mx_floor(MExit *e, FloorResult floor)
{
    MxStep s = { 0 };

    s.kind = MX_FLOOR;
    s.u.floor = floor;
    mx_push(e, s);
}

static inline void mx_up_marked(MExit *e, UpOrMarked up)
{
    MxStep s = { 0 };

    s.kind = MX_UP_MARKED;
    s.u.up_marked = up;
    mx_push(e, s);
}

/* MariaReadBelow's exit, as its wrapper gives it. */
static inline void mx_read_below(MExit *e, TileRead read, bool carry)
{
    MxStep s = { 0 };

    s.kind = MX_READ_BELOW;
    s.u.read = read;
    s.carry = carry;
    mx_push(e, s);
}

static inline void mx_script(MExit *e, uint8_t script)
{
    MxStep s = { 0 };

    s.kind = MX_SCRIPT;
    s.script = script;
    mx_push(e, s);
}

/* Another routine's MExit, as it ended inside this one: its steps, then its last registers. */
static inline void mx_then(MExit *e, MExit inner)
{
    if (inner.elsewhere) {
        e->elsewhere = true;
        return;
    }
    for (uint8_t i = 0; i < inner.n; i++) {
        MxStep s = inner.step[i];

        s.regs = r_then(e->last, s.regs);
        e->last = regs_kept();
        if (e->n < 6) {
            e->step[e->n++] = s;
        }
    }
    e->last = r_then(e->last, inner.last);
}

static inline void mx_exit(MExit e)
{
    if (e.elsewhere) {
        return;
    }
    for (uint8_t i = 0; i < e.n; i++) {
        regs_exit(e.step[i].regs);
        switch (e.step[i].kind) {
        case MX_ANIMATION:
            animation_exit_shim(e.step[i].animation);
            break;
        case MX_SOUND:
            sound_command_exit_shim(e.step[i].command, e.step[i].call);
            break;
        case MX_SCRIPT:
            hit_script_exit_shim(e.step[i].script);
            break;
        case MX_COLUMNS:
            columns_exit_shim(e.step[i].u.columns);
            break;
        case MX_FOOTING:
            footing_exit_shim(e.step[i].u.footing);
            break;
        case MX_LANDING:
            landing_exit_shim(e.step[i].u.landing);
            break;
        case MX_LEDGE:
            maria_ledge_check_exit_shim(e.step[i].u.ledge);
            break;
        case MX_CRASH:
            item_crash_exit_shim(e.step[i].u.crash);
            break;
        case MX_FLOOR:
            floor_check_exit_shim(e.step[i].u.floor);
            break;
        case MX_UP_MARKED:
            marked_columns_exit_shim(e.step[i].u.up_marked.above);
            abi_flag(P_C, e.step[i].u.up_marked.c);
            break;
        case MX_READ_BELOW:
            read_or_nothing_exit_shim(e.step[i].u.read, !e.step[i].carry);
            break;
        case MX_TURN:
            abi_ended(e.step[i].u.turn.end);
            if (e.step[i].u.turn.x_set) {
                cpu.x = e.step[i].u.turn.x;
            }
            break;
        }
    }
    regs_exit(e.last);
}

/*
 * The processor status an MExit leaves, for a caller that branches on it: worked out from the exit the way its
 * wrapper gives it, over the processor as it stands. Used only where every path of the callee sets the flag read.
 */
static inline uint8_t mx_status(MExit e)
{
    Cpu saved = cpu;
    uint8_t p;

    mx_exit(e);
    p = cpu.p;
    cpu = saved;
    return p;
}

/* The carry or Z one call's exit leaves, for the branch after it (every path of these callees sets the flag). */
static inline bool mx_carry(MExit e)
{
    return (mx_status(e) & P_C) != 0;
}

static inline bool mx_zero(MExit e)
{
    return (mx_status(e) & P_Z) != 0;
}

static inline MExit mx_one_animation(AnimationStep animation)
{
    MExit e = mx_none();

    mx_animation(&e, animation);
    return e;
}

static inline MExit mx_one_footing(FootingCheck footing)
{
    MExit e = mx_none();

    mx_footing(&e, footing);
    return e;
}

/* The X or Y an MExit leaves, the same way. */
static inline uint8_t mx_x(MExit e)
{
    Cpu saved = cpu;
    uint8_t x;

    mx_exit(e);
    x = cpu.x;
    cpu = saved;
    return x;
}

static inline uint8_t mx_y(MExit e)
{
    Cpu saved = cpu;
    uint8_t y;

    mx_exit(e);
    y = cpu.y;
    cpu = saved;
    return y;
}


/* Her disc tables, read in the bank her overlay maps them in (TBL8, access.h). */
#define MariaAnimationSounds_TBL MariaAnimationSounds          /* $C1A8, bank $6E: the list by animation */
#define MariaFootYByState_TBL MariaFootYByState                /* $A118, bank $6D: the feet's row by state */
#define MariaStartHandlers_TBL MariaStartHandlers              /* $A131, bank $6D: four words by zField8A */
#define MariaAnimationHitboxIndex_TBL MariaAnimationHitboxIndex /* $C200, bank $6E: hitbox index by animation */
#define MariaAnimationHitbox_TBL MariaAnimationHitbox          /* $C258, bank $6E: four bytes a hitbox */
#define MariaHitTimers_TBL MariaHitTimers                      /* $A35E, bank $6D: hit timer by kind */

/* Defined elsewhere, typed, and not declared in routines.h. */
typedef struct { uint8_t keep; CommandCall sound; } FadeStop;      /* system/game_mode.c */
FadeStop FadeAndStopSound(uint8_t kept_x, uint8_t kept_y);         /* system/game_mode.c */
void abi_FadeAndStopSound(void);                                   /* system/game_mode.c */
void ClearObject(uint8_t object);                                  /* object/lifecycle.c */
void MariaClearWork(uint8_t a, uint8_t x, uint8_t y);              /* object/richter_state.c */
uint8_t MariaClearKeepPlace(void);                                 /* object/richter_state.c */
void MariaStartHitScript(uint8_t script);                          /* object/richter_state.c */
void MariaPadForcesOff(void);                                      /* object/richter_state.c */
AnimationStep MariaSetAnimation(uint8_t animation);                /* object/player_input.c */
AnimationStep MariaAnimationTick(void);                            /* object/player_input.c */
ColumnOffset MariaColumnOffset(uint8_t column);                    /* object/player_terrain.c */
TileRead MariaReadAbove(uint8_t row, bool on_the_map);             /* object/player_terrain.c */
AnimationStep MariaAnimationRunning(void);                         /* object/player_input.c */
FootingCheck MariaHasNoFooting(void);                              /* object/player_terrain.c */
LandingLift MariaLandOnFloor(void);                                /* object/player_terrain.c */
bool MariaJumpPressed(void);                                       /* object/player_terrain.c */
bool MariaUpHeld(void);                                            /* object/player_terrain.c */
ItemCrash MariaTryItemCrash(void);                                 /* object/player_input.c */
TurnWindow MariaUseItem(void);                                     /* object/player_input.c */
uint8_t MariaStepIndex(void);                                      /* object/richter_state.c */
AnimationWaitEnd MariaAnimationWait(void);                         /* object/player_input.c */
uint8_t MariaClearSlotAsKind(uint8_t slot, uint8_t kind);          /* object/richter_objects.c */
Ended MariaReplaceSlotsOneTwo(void);                               /* object/richter_objects.c */

/* object/maria_frame.c */
/* What an animation set left, then the registers set after it (its wrapper gives the animation's exit first). */
typedef struct { AnimationStep animation; Regs regs; } AnimThen;
Regs MariaAnimationSound(void);
void abi_MariaAnimationSound(void);
void abi_MariaStart(void);
AnimationStep MariaVecSetAnimation(uint8_t animation);
AnimationStep MariaVecAnimationTick(void);
Regs MariaVecAnimationSound(void);
void MariaStopX(void);
void MariaStopMoving(void);
AnimThen MariaStand(void);
AnimThen MariaCrouch(void);
AnimThen MariaToFall(void);
AnimThen MariaStartStand(void);
AnimThen MariaStartClear(uint8_t a, uint8_t x, uint8_t y);
AnimThen MariaStartHealth(uint8_t x, uint8_t y);
AnimThen MariaStartNew(uint8_t x, uint8_t y);
CommandCall MariaClearSlot5AsKind9(uint8_t y);
CommandCall MariaDeathSound(uint8_t x, uint8_t y);
CommandCall MariaDie(uint8_t x, uint8_t y);
/* object/maria_attack.c */
typedef struct { MExit e; bool claimed; } MariaClaim;
MariaClaim MariaAttackSlot(uint8_t y);
MExit MariaStandAttack(void);
MExit MariaSequenceMoveStart(uint8_t x, uint8_t y);
MExit MariaThrowItem(uint8_t x, uint8_t y);
MExit MariaAttackAndThrow(uint8_t x, uint8_t y);
MExit MariaAttack(uint8_t x, uint8_t y);
MExit MariaAttackFromUse(uint8_t x, uint8_t y);
MExit MariaCrashOnGround(uint8_t x, uint8_t y);
MExit MariaCrashInAir(uint8_t x, uint8_t y);
Regs MariaCrashRecover(void);
Regs MariaClimbHold(void);
MExit MariaCrashEnd(void);
MExit MariaCrashTurn(void);
MExit MariaCrashWait(void);
MExit MariaCrashSignal(void);
MExit MariaCrashRise(void);
MExit MariaCrashFall(void);
MExit MariaCrashLand(void);
MExit MariaCrashPhase(void);
MExit MariaCrashPhaseB(void);
MExit MariaCrashFrame(void);
MExit MariaSetAnimationZero(void);
Regs MariaSpeedWalk(void);
Regs MariaLiftThree(void);
Regs MariaJumpSpeed(void);
MExit MariaJump(void);
void MariaHitReact(void);

/* object/maria_stand.c */
/* A routine that may pull its caller's return address off the stack (PLA, PLA) and jump on: dropped says it did. */
typedef struct { MExit e; bool dropped; } MariaDrop;
MariaDrop MariaGroundChecks(uint8_t x, uint8_t y);
MExit MariaStandFrame(uint8_t x, uint8_t y);
MExit MariaStandAttackFrame(uint8_t x, uint8_t y);
MExit MariaStateStand(uint8_t x, uint8_t y);
MExit MariaClimbStart(uint8_t column);

/* object/maria_ground.c */
MExit MariaStateWalk(uint8_t x, uint8_t y);
Regs MariaStart9AObjects(uint8_t value, uint8_t from, uint8_t y);
MExit MariaBit3Burst(uint8_t x, uint8_t y);
MExit MariaCheckBit3Under(uint8_t x, uint8_t y);
Regs MariaReplaceSlotsOneTwoKind5(void);
MExit MariaReadAhead(uint8_t distance, uint8_t x, uint8_t y);
MExit MariaSlideCheck(uint8_t x, uint8_t y);
MExit MariaSlideStart(uint8_t x, uint8_t y);
Regs MariaFaceByPad(uint8_t pad);
MExit MariaCrouchRise(void);
MExit MariaCrouchAttack(uint8_t x, uint8_t y);
MariaDrop MariaCrouchChecks(uint8_t x, uint8_t y);
MExit MariaCrouchFrame(uint8_t x, uint8_t y);
void MariaCrouchIdle(void);
MExit MariaCrouchAttackFrame(uint8_t x, uint8_t y);
MExit MariaCrouchStart(uint8_t x, uint8_t y);
MExit MariaStandUp(uint8_t x, uint8_t y);
MExit MariaStateCrouch(uint8_t x, uint8_t y);

void abi_MariaObjectApplyVelocity(void);                           /* object/richter_objects.c */
void abi_MariaApplyMoveAndPush(void);                              /* object/richter_state.c */

#endif
