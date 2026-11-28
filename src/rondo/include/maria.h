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
#include "richter.h"

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
typedef enum { MX_ANIMATION, MX_SOUND, MX_SCRIPT, MX_COLUMNS, MX_FOOTING, MX_LANDING, MX_LEDGE, MX_CRASH, MX_TURN, MX_FLOOR, MX_UP_MARKED, MX_READ_BELOW,
    MX_SOLID, MX_LIFT, MX_MARKED, MX_WALL, MX_TST, MX_GRAVITY, MX_UNDER_FEET, MX_ON_MARKED,
    MX_OFFSET_READ, MX_JUMP_OR_CLIMB, MX_LADDER } MxKind;

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
        ColumnRead column;
        ColumnLift lift;
        MarkedColumns marked;
        WallPush wall;
        EndedTst tst;
        Sum gravity;
        MarkedFloor on_marked;
        OffsetRead offset_read;
        JumpOrClimb jump_or_climb;
        LadderCheck ladder;
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

static inline void mx_solid(MExit *e, ColumnRead value)
{
    MxStep s = { 0 };

    s.kind = MX_SOLID;
    s.u.column = value;
    mx_push(e, s);
}

static inline void mx_lift(MExit *e, ColumnLift value)
{
    MxStep s = { 0 };

    s.kind = MX_LIFT;
    s.u.lift = value;
    mx_push(e, s);
}

static inline void mx_marked(MExit *e, MarkedColumns value)
{
    MxStep s = { 0 };

    s.kind = MX_MARKED;
    s.u.marked = value;
    mx_push(e, s);
}

static inline void mx_wall(MExit *e, WallPush value)
{
    MxStep s = { 0 };

    s.kind = MX_WALL;
    s.u.wall = value;
    mx_push(e, s);
}

static inline void mx_tst(MExit *e, EndedTst value)
{
    MxStep s = { 0 };

    s.kind = MX_TST;
    s.u.tst = value;
    mx_push(e, s);
}

static inline void mx_gravity(MExit *e, Sum value)
{
    MxStep s = { 0 };

    s.kind = MX_GRAVITY;
    s.u.gravity = value;
    mx_push(e, s);
}

static inline void mx_under_feet(MExit *e, ColumnRead value)
{
    MxStep s = { 0 };

    s.kind = MX_UNDER_FEET;
    s.u.column = value;
    mx_push(e, s);
}

static inline void mx_on_marked(MExit *e, MarkedFloor value)
{
    MxStep s = { 0 };

    s.kind = MX_ON_MARKED;
    s.u.on_marked = value;
    mx_push(e, s);
}

static inline void mx_offset_read(MExit *e, OffsetRead value)
{
    MxStep s = { 0 };

    s.kind = MX_OFFSET_READ;
    s.u.offset_read = value;
    mx_push(e, s);
}

static inline void mx_jump_or_climb(MExit *e, JumpOrClimb value)
{
    MxStep s = { 0 };

    s.kind = MX_JUMP_OR_CLIMB;
    s.u.jump_or_climb = value;
    mx_push(e, s);
}

static inline void mx_ladder(MExit *e, LadderCheck value)
{
    MxStep s = { 0 };

    s.kind = MX_LADDER;
    s.u.ladder = value;
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
        case MX_SOLID:
            column_read_exit_shim(e.step[i].u.column);
            cpu.a = solid_answer(e.step[i].u.column);
            abi_nz(cpu.a);
            break;
        case MX_LIFT:
            column_lift_exit_shim(e.step[i].u.lift);
            break;
        case MX_MARKED:
            marked_columns_exit_shim(e.step[i].u.marked);
            break;
        case MX_WALL:
            wall_push_exit_shim(e.step[i].u.wall);
            break;
        case MX_TST:
            abi_ended_tst(e.step[i].u.tst);
            break;
        case MX_GRAVITY:
            fall_gravity_exit_shim(e.step[i].u.gravity);
            break;
        case MX_UNDER_FEET:
            under_feet_exit_shim(e.step[i].u.column);
            break;
        case MX_ON_MARKED:
            marked_floor_exit_shim(e.step[i].u.on_marked);
            break;
        case MX_OFFSET_READ:
            offset_read_exit_shim(e.step[i].u.offset_read);
            break;
        case MX_JUMP_OR_CLIMB:
            jump_or_climb_exit_shim(e.step[i].u.jump_or_climb);
            break;
        case MX_LADDER:
            ladder_exit_shim(e.step[i].u.ladder);
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


/* The A, X or Y a chain leaves from known X and Y at its start, for the call or AND made after it. */
static inline uint8_t mx_a_from(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();
    Cpu saved = cpu;
    uint8_t a;

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    mx_exit(t);
    a = cpu.a;
    cpu = saved;
    return a;
}

static inline uint8_t mx_x_from(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_x(t);
}

static inline uint8_t mx_y_from(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_y(t);
}

/* PLA, PLA: the caller's return address taken off the stack; A the second byte pulled, N and Z from it. */
static inline Regs mx_drop_caller(void)
{
    uint8_t value;

    pull8();
    value = pull8();
    return r_lda(regs_kept(), value);
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
void under_feet_exit_shim(ColumnRead one);                         /* object/player_terrain.c */
ColumnRead MariaReadUnderFeet(void);                               /* object/player_terrain.c */
ColumnRead MariaSolidUnderColumn(uint8_t column);                  /* object/player_terrain.c */
MarkedFloor MariaOnMarkedFloor(void);                              /* object/richter_state.c */
Sum MariaFallGravity(void);                                        /* object/richter_state.c */
EndedTst MariaAirTurn(void);                                       /* object/richter_state.c */
ObjArm MariaNoteItemPress(void);                                   /* object/player_input.c */
uint8_t MariaClaimSlotsB(void);                                    /* object/richter_objects.c */
TileRead MariaReadOnScreen(uint8_t row, bool carry);               /* object/player_terrain.c */
TurnWindow MariaUseItemFromThrow(void);                            /* object/player_input.c */
void MariaClearPadOverrides(void);                                 /* object/player_input.c */

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
MExit stand_attack_animation(uint8_t animation_number);
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

/* object/maria_jump.c */
Regs MariaGravityLight(void);
MExit MariaJumpGravity(void);
Regs MariaLedgePose(uint8_t pose);
Regs MariaLedgeHoldAt(uint8_t y_lo);
MExit MariaLedgeFloorBelow(uint8_t x, uint8_t y);
MExit MariaLandOnMarked(uint8_t x, uint8_t y);
MExit MariaLandOnMarkedBack(uint8_t x, uint8_t y);
MExit MariaLandOnMarkedFeet(uint8_t x, uint8_t y);
MExit MariaFeetOnMarkedFloor(void);
MExit MariaJumpLand(uint8_t x, uint8_t y);
MExit MariaStateLanded(void);
MExit MariaSecondJump(void);
MExit MariaStateJump(uint8_t x, uint8_t y);
MExit MariaStateFall(uint8_t x, uint8_t y);

/* object/maria_climb.c */
OffsetRead MariaReadAtOffset(uint8_t offset);
JumpOrClimb MariaJumpOrClimb(void);
Regs MariaOrForcedPressB(uint8_t bits);
Regs MariaNotePressesB(void);
MExit MariaItemOrAttack(void);
MExit MariaClimbMove(void);
MExit MariaClimbPad(void);
MExit MariaClimbLetGo(void);
MExit MariaClimbAttack(uint8_t x, uint8_t y);
MExit MariaClimbLowFrame(uint8_t x, uint8_t y);
MExit MariaClimbHighFrame(uint8_t x, uint8_t y);
MExit MariaClimbWait(void);
MExit MariaClimbAcross(void);
MExit MariaClimbTurn(void);
MExit MariaClimbUp(void);
MExit MariaStateClimbLow(uint8_t x, uint8_t y);
MExit MariaStateClimbHigh(uint8_t x, uint8_t y);

/* object/maria_slide.c */
Regs MariaSlideEnd(void);
Regs MariaToSlideAir(void);
Regs MariaSlideSlow(void);
MExit MariaStateSlideHeld(uint8_t x, uint8_t y);
MExit MariaStateSlide(uint8_t x, uint8_t y);
MExit MariaStateSlideAir(uint8_t x, uint8_t y);

/* object/maria_hurt.c */
Regs MariaHurtSide(void);
MExit MariaHitEffects(uint8_t x, uint8_t y);
MExit MariaHitSoundAndEffects(uint8_t x, uint8_t y);
MExit MariaHurtKnockUp(uint8_t x, uint8_t y);
MExit MariaHurtDown(uint8_t x, uint8_t y);
MExit MariaHurtFromStand(uint8_t x, uint8_t y);
MExit MariaHurtFromCrouch(uint8_t x, uint8_t y);
MExit MariaHurtInAir(uint8_t x, uint8_t y);
MExit MariaHurtClimbKindFive(uint8_t x, uint8_t y);
MExit MariaHurtWhileClimbing(uint8_t x, uint8_t y);
MExit MariaHurtFromItem4(uint8_t x, uint8_t y);
MExit MariaHurtFromSequence(uint8_t y);
MExit MariaHurtKindThree(uint8_t x, uint8_t y);
MExit MariaHurtKindFour(uint8_t x, uint8_t y);
MExit MariaHurtStart(uint8_t x, uint8_t y);
Regs MariaKnockSpeedX(void);
MExit MariaHurtSlide(void);
MExit MariaHurtSlideStart(void);
MExit MariaHurtResume(void);
MExit MariaHurtHoldFrame(void);
MExit MariaHurtDownStep(void);
void MariaHurtHeldLanding(void);
void MariaHurtHeldLandingFeet(void);
MExit MariaHurtLandOnMarked(uint8_t x, uint8_t y);
MExit MariaHurtLandOnMarkedFeet(uint8_t x, uint8_t y);
MExit MariaHurtKnocked(uint8_t x, uint8_t y);
MExit MariaHurtThrown(uint8_t x, uint8_t y);
MExit MariaHurtThrownLanded(void);
MExit MariaStateHurt(uint8_t x, uint8_t y);

void abi_MariaObjectApplyVelocity(void);                           /* object/richter_objects.c */
void abi_MariaApplyMoveAndPush(void);                              /* object/richter_state.c */

#endif
