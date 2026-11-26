/*
 * Lot L5: the stage-specific object code of the stage overlays, as typed C. A body takes the
 * object's slot and returns, as a plain value, the byte its last instruction left N and Z from;
 * its abi_ wrapper turns that into the flags the assembly leaves.
 */
#ifndef RONDO_ENEMY_OBJECTS_H
#define RONDO_ENEMY_OBJECTS_H

#include <string.h>
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

/*
 * How a routine's last instructions left the registers and flags the caller can see: a plain
 * value a body returns beside its work, turned into the processor's state by abi_ended. Only
 * what the routine changed is marked.
 */
typedef struct {
    bool a_set;      /* A holds a */
    uint8_t a;
    bool nz_set;     /* N and Z come from nz */
    uint8_t nz;
    bool c_set;      /* C is c */
    bool c;
    bool v_set;      /* V is v */
    bool v;
    bool y_set;      /* Y holds y */
    uint8_t y;
} Ended;

/* Nothing was changed: the caller's A, flags and Y stand. */
static inline Ended ended_kept(void)
{
    Ended e = { 0 };

    return e;
}

/* LDA, AND, ORA: A and N, Z from the value. */
static inline Ended ended_load(uint8_t value)
{
    Ended e = { 0 };

    e.a_set = true;
    e.a = value;
    e.nz_set = true;
    e.nz = value;
    return e;
}

/* INC, DEC of memory, LDY: N and Z from the value only. */
static inline Ended ended_nz(uint8_t value)
{
    Ended e = { 0 };

    e.nz_set = true;
    e.nz = value;
    return e;
}

/* CMP: A is a; N, Z and C from the compare. */
static inline Ended ended_compare(uint8_t a, uint8_t operand)
{
    Ended e = { 0 };

    e.a_set = true;
    e.a = a;
    e.nz_set = true;
    e.nz = (uint8_t)(a - operand);
    e.c_set = true;
    e.c = a >= operand;
    return e;
}

/* The same state with C as given (a SEC, a CLC, or the carry out of an add or a shift). */
static inline Ended ended_with_carry(Ended e, bool carry)
{
    e.c_set = true;
    e.c = carry;
    return e;
}

/* The same state with V as given (the overflow of an add or a subtract). */
static inline Ended ended_with_overflow(Ended e, bool overflow)
{
    e.v_set = true;
    e.v = overflow;
    return e;
}

/* The same state with A as given (an LDA that came before the last INC or DEC). */
static inline Ended ended_with_a(Ended e, uint8_t a)
{
    e.a_set = true;
    e.a = a;
    return e;
}

/* The same state with Y as given. */
static inline Ended ended_with_y(Ended e, uint8_t y)
{
    e.y_set = true;
    e.y = y;
    return e;
}

/* The C the state leaves, given the C the routine was entered with (it stands unless something set it). */
static inline bool ended_carry_or(Ended e, bool entry)
{
    return e.c_set ? e.c : entry;
}

/* The Y the state leaves, given the Y the routine was entered with. */
static inline uint8_t ended_y_or(Ended e, uint8_t entry)
{
    return e.y_set ? e.y : entry;
}

/* The abi_ wrappers' one place that meets the registers. */
static inline void abi_ended(Ended e)
{
    if (e.a_set) {
        cpu.a = e.a;
    }
    if (e.nz_set) {
        abi_nz(e.nz);
    }
    if (e.c_set) {
        abi_flag(P_C, e.c);
    }
    if (e.v_set) {
        abi_flag(P_V, e.v);
    }
    if (e.y_set) {
        cpu.y = e.y;
    }
}

/* The V an add or a subtract leaves: a, b the operands, result the byte it gave. */
static inline bool add_overflow(uint8_t a, uint8_t b, uint8_t result)
{
    return (((a ^ result) & (b ^ result)) & 0x80) != 0;
}

static inline bool sub_overflow(uint8_t a, uint8_t b, uint8_t result)
{
    return (((a ^ b) & (a ^ result)) & 0x80) != 0;
}


/* The exit state of what was before, with every field the later instruction set put over it. */
static inline Ended ended_then(Ended before, Ended after)
{
    if (after.a_set) {
        before.a_set = true;
        before.a = after.a;
    }
    if (after.nz_set) {
        before.nz_set = true;
        before.nz = after.nz;
    }
    if (after.c_set) {
        before.c_set = true;
        before.c = after.c;
    }
    if (after.v_set) {
        before.v_set = true;
        before.v = after.v;
    }
    if (after.y_set) {
        before.y_set = true;
        before.y = after.y;
    }
    return before;
}

/*
 * Callees that are typed C elsewhere and that no header declares. The defining file is named; each
 * ended_ builder below gives the state its abi_ wrapper leaves, from the typed result alone.
 */


/* A routine that may play a sound: what it ended with, and the call the wrapper must show the leak of. */
typedef struct {
    Ended before;   /* what the instructions before the call left (the A that holds the command) */
    Ended end;
    bool sounded;
    uint8_t command;
    CommandCall call;
} SoundEnd;

/* The wrapper of such a routine: the sound call's flags first, then what the routine's own instructions changed. */
static inline void abi_sound_end(SoundEnd r)
{
    if (r.sounded) {
        abi_ended(r.before);
        sound_command_exit_shim(r.command, r.call);
    }
    abi_ended(r.end);
}

/* An add or subtract's result: A the byte, N and Z from it, C and V from the add. */
static inline Ended ended_of_sum(Sum sum)
{
    return ended_with_overflow(ended_with_carry(ended_load(sum.value), sum.carry), sum.overflow);
}

/* HalveObjectVelocity: A, N and Z, C, and V when it set one. */
static inline Ended ended_of_arith(ArithExit a)
{
    Ended e = ended_with_carry(ended_with_a(ended_nz(a.nz), a.a), a.carry);

    return a.overflow_set ? ended_with_overflow(e, a.overflow) : e;
}

/* NextRandom: A the byte, N and Z from it, C and V from its last add. */
static inline Ended ended_of_random(RandomResult r)
{
    return ended_with_overflow(ended_with_carry(ended_load(r.value), r.carry), r.overflow);
}

/* SideOfPlayer: A the side bits, N and Z from them, C from the last compare when one ran. */
static inline Ended ended_of_side(SideResult r)
{
    Ended e = ended_load(r.sides);

    return r.carry_set ? ended_with_carry(e, r.carry) : e;
}

/* DistanceXToPlayer and DistanceYToPlayer: A the distance; C and V from the subtract when near. */
static inline Ended ended_of_distance(PlayerDistance d)
{
    Ended e = ended_load(d.distance);

    if (!d.far) {
        e = ended_with_carry(e, d.object_further);
        e = ended_with_overflow(e, sub_overflow(d.mine, d.player, (uint8_t)(d.mine - d.player)));
    }
    return e;
}

/* How a routine that may play a sound on the first screen ended: the state before the call, the call, the rest. */
typedef struct {
    Ended before;
    Ended end;
    bool called;
    SoundOnScreen sound;
} ScreenSoundEnd;

static inline void abi_screen_sound_end(ScreenSoundEnd r)
{
    abi_ended(r.before);
    if (r.called) {
        sound_if_on_first_screen_exit_shim(r.sound);
    }
    abi_ended(r.end);
}


/* FindFreeSlotFrom20, 30 and 50: A the last type read, N 0, Z 1, C found, Y the slot. */
static inline Ended ended_of_find_slot(uint8_t slot, uint8_t last, bool found)
{
    return ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), slot), found);
}

/* A routine whose exit keeps what NextRandom left in C and V; the wrapper shows that first. */
typedef struct {
    Ended end;
    bool random_used;
    RandomResult random;
} RandomEnd;

static inline void abi_random_end(RandomEnd r)
{
    if (r.random_used) {
        random_exit_shim(r.random);
    }
    abi_ended(r.end);
}

/* The same for SideOfPlayer's carry. */
typedef struct {
    Ended end;
    bool side_used;
    SideResult side;
} SideEnd;

static inline void abi_side_end(SideEnd r)
{
    if (r.side_used) {
        side_of_player_exit_shim(r.side);
    }
    abi_ended(r.end);
}

/* StepObjectFrames: A $FF and C 1 when the frame changed, A 0 and C 0 when the list ended, A $FF and C 0 when held. */
static inline Ended ended_of_frame_step(FrameStep step, bool y_changed, uint8_t y)
{
    Ended e = ended_load(step == FRAME_LIST_ENDED ? 0x00 : 0xFF);

    e = ended_with_carry(e, step == FRAME_CHANGED);
    if (y_changed) {
        e = ended_with_y(e, y);
    }
    return e;
}

/* ApplyVelocity, ApplyVelocityX and ApplyVelocityY: A the position's high byte, N, Z, C and V from its add. */
static inline Ended ended_of_axis_move(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);
    Ended e = ended_load(result);

    e = ended_with_carry(e, sum > 0xFFFFFF);
    return ended_with_overflow(e, (~(sign ^ high) & (sign ^ result) & 0x80) != 0);
}

/* LoadObjectRecord: A the record's last byte, Y 11, N and Z from the new state, C from the record. */
static inline Ended ended_of_record(uint8_t last, uint8_t nz, bool carry)
{
    Ended e = ended_with_y(ended_load(last), 11);

    e.nz = nz;
    return ended_with_carry(e, carry);
}

/* How a routine that steps the animation ended: what the step did and the state it leaves. */
typedef struct {
    FrameStep step;
    Ended end;
} FrameStepEnd;

/*
 * What a converted routine ended on (the 441de03 pattern): the arm, one enum per routine naming the
 * path its last instructions took; the values that path read or made; and the typed results of the
 * callees it ended after. A body fills it and returns it; it holds no register or flag. The
 * routine's own *_exit_shim, the one place that meets the assembly's convention, builds the
 * registers and flags from it with the ended_ builders above, and its abi_ wrapper applies them.
 */
#define OBJ_ARM_FIELDS \
    int arm; \
    uint8_t first, second, third, fourth, fifth, sixth, seventh, eighth, ninth, tenth; \
    uint8_t y; \
    bool flag, other; \
    FrameStep step; \
    AxisMove move; \
    RandomResult random; \
    SideResult side; \
    PlayerDistance distance, distance2; \
    AngleExit angle; \
    Sum sum; \
    bool sounded; \
    uint8_t command; \
    CommandCall call; \
    bool screen_called; \
    SoundOnScreen screen; \
    TileRun run;

/* A callee's ObjArm kept inside its caller's (one level: a callee kept this way keeps no callee of its own). */
typedef struct {
    OBJ_ARM_FIELDS
} ObjSub;

typedef struct {
    OBJ_ARM_FIELDS
    ObjSub sub;
} ObjArm;

/* A callee's ObjArm as its caller keeps it, and back for the callee's exit shim. */
static inline ObjSub obj_sub(ObjArm a)
{
    ObjSub s;

    memcpy(&s, &a, sizeof s);
    return s;
}

static inline ObjArm obj_arm(ObjSub s)
{
    ObjArm a = { 0 };

    memcpy(&a, &s, sizeof s);
    return a;
}

/*
 * Bodies several stage files share (one C body for every near copy). Each takes the object's
 * slot and says how it ended.
 */

/* The arms the shared bodies below end on. */
enum { CD_COUNTING, CD_STATE };
enum { SP_NO_SLOT, SP_DONE };
enum { SE_QUIET, SE_SOUNDED };
enum { KU_CLEAR, KU_ABOVE, KU_KNOCKED };
enum { FP_RECORD, FP_FOLLOW };
enum { PT_STEPPED, PT_TURNED };
enum { PS_X_HIGH, PS_Y_HIGH, PS_TABLE };
enum { FS_NONE, FS_FACED };
enum { WF_WALKING, WF_DONE };

/* The step of a frame list (StepObjectFrames, or a routine that ends on it): A, C and Y as ended_of_frame_step gives them. */
static inline Ended frame_step_exit_shim(ObjArm r)
{
    return ended_of_frame_step(r.step, r.step != FRAME_HELD, r.y);
}

/* LoadObjectRecord's exit, kept as first (the last byte), second (its N and Z) and flag (its C). */
static inline Ended record_exit_shim(ObjArm r)
{
    return ended_of_record(r.first, r.second, r.flag);
}

/* The C a callee is entered with right after such a step: the step's, or the entry's when it set none. */
static inline bool frame_step_carry_shim(ObjArm r, bool entry)
{
    return ended_carry_or(frame_step_exit_shim(r), entry);
}

/* An LDY of the list index before such a step: Y stands as loaded unless the step set it. */
static inline Ended ldy_frame_step_shim(uint8_t y, ObjArm r)
{
    return ended_then(ended_with_y(ended_kept(), y), frame_step_exit_shim(r));
}

/* The timer counts down; at zero the object enters `state`. */
static inline ObjArm countdown_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    ObjArm r = { 0 };
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    r.arm = CD_COUNTING;
    r.first = left;
    if (left != 0) {
        return r;
    }
    WR8(OBJ->state[slot], state);
    r.arm = CD_STATE;
    r.first = state;
    return r;
}

/* The same, and at zero the object's f3408 and f3460 are cleared first. */
static inline ObjArm countdown_stop_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    ObjArm r = { 0 };
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    r.arm = CD_COUNTING;
    r.first = left;
    if (left != 0) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], state);
    r.arm = CD_STATE;
    r.first = state;
    return r;
}

/* Both countdowns: the DEC's N and Z while counting; the LDA of the new state at zero. */
static inline Ended countdown_exit_shim(ObjArm r)
{
    return r.arm == CD_COUNTING ? ended_nz(r.first) : ended_load(r.first);
}

/*
 * Up to six objects of a type started at the object's place, each from a free slot from $30, each given a
 * life_timer, f3510 and speeds from a four byte entry of a table: the body SpawnSixPiecesA and B share.
 */
static inline ObjArm spawn_six_pieces(uint8_t slot, uint8_t type, uint16_t table, uint16_t ret_find, uint16_t ret_start)
{
    ObjArm r = { 0 };
    uint8_t piece = 0, found_slot, last, row, value;
    bool found;

    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(ret_find);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        if (!found) {
            r.arm = SP_NO_SLOT;
            r.first = found_slot;
            r.second = last;
            return r;
        }
        jsr_push(ret_start);
        StartObjectAtPlace(type, slot, found_slot);
        jsr_pop();
        WR8(ZP->richter_object, slot);
        row = (uint8_t)(RD8(ZP->hit_slot) << 2);
        value = rd8((uint16_t)(table + row));
        WR8(OBJ->life_timer[found_slot], value);
        value = rd8((uint16_t)(table + 1 + row));
        WR8(OBJ->f3510[found_slot], value);
        value = rd8((uint16_t)(table + 2 + row));
        WR8(OBJ->vx_hi[found_slot], value);
        value = rd8((uint16_t)(table + 3 + row));
        WR8(OBJ->vy_hi[found_slot], value);
        slot = RD8(ZP->richter_object);
        piece = RD8(ZP->hit_slot);
        piece++;
    } while (piece < 6);
    r.arm = SP_DONE;
    r.first = value;
    r.second = piece;
    return r;
}

/* No slot: FindFreeSlotFrom30's exit. Six started: A the last table byte, the INY and CPY #6 (Y 6, C 1). */
static inline Ended spawn_six_pieces_exit_shim(ObjArm r)
{
    if (r.arm == SP_NO_SLOT) {
        return ended_of_find_slot(r.first, r.second, false);
    }
    return ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(r.second - 6)), r.first), r.second >= 6), r.second);
}

/* Sound $22 when the low two bits of the object's f3618 are zero: the body SoundEveryFourthFrame and its copy share. */
static inline ObjArm sound_every_fourth_frame(uint8_t slot, uint8_t entry_y, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t bits = RD8(OBJ->f3618[slot]);

    bits = (uint8_t)(bits & 3);
    if (bits != 0) {
        r.arm = SE_QUIET;
        r.first = bits;
        return r;
    }
    r.arm = SE_SOUNDED;
    r.sounded = true;
    r.command = 0x22;
    jsr_push(ret);
    r.call = SoundCommand(0x22, slot, entry_y);
    jsr_pop();
    return r;
}

/* Quiet: the AND's A, N and Z. Sounded: LDA #$22, then SoundCommand's exit. */
static inline SoundEnd sound_every_fourth_frame_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };

    if (r.arm == SE_QUIET) {
        s.end = ended_load(r.first);
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_load(0x22);
    s.call = r.call;
    return s;
}

/*
 * A hit that has set bit 7 of the status flags sends an object below a state into a tumble: its f3250 and animation
 * stopped, sprite set 3, frame 0, speed -$0102 upward (both bytes $FE) and the state given. The body 117C:1EA and
 * 1173:26A share.
 */
static inline ObjArm knock_up_when_flagged(uint8_t slot, uint8_t below, uint8_t state)
{
    ObjArm r = { 0 };
    uint8_t bit = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    uint8_t now;

    r.second = below;
    r.third = state;
    r.arm = KU_CLEAR;
    r.first = bit;
    if (bit == 0) {
        return r;
    }
    now = RD8(OBJ->state[slot]);
    r.arm = KU_ABOVE;
    r.first = now;
    if (now >= below) {
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f2a10[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0xFE);
    WR8(OBJ->state[slot], state);
    r.arm = KU_KNOCKED;
    return r;
}

/* Clear: the AND's A, N, Z. Otherwise the CMP of the state with `below`, and when knocked the LDA of the new state over it. */
static inline Ended knock_up_when_flagged_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == KU_CLEAR) {
        return ended_load(r.first);
    }
    e = ended_compare(r.first, r.second);
    return r.arm == KU_ABOVE ? e : ended_then(e, ended_load(r.third));
}

/*
 * A piece that follows the object one slot before it: in state 0 it loads its record; later it takes the place of
 * slot less 1, its f3250 from there, a hit rectangle from a table of four byte rows picked by that object's frame
 * less a base, and its facing. The body 117D:277 and 093D:6A4 share.
 */
static inline ObjArm follow_previous_piece(uint8_t slot, uint8_t base, uint16_t table, uint16_t ret_copy)
{
    ObjArm r = { 0 };
    uint8_t previous = (uint8_t)(slot - 1);
    uint8_t value, row, frame;
    bool carry;

    if (RD8(OBJ->state[slot]) == 0) {
        r.arm = FP_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &carry);
        r.flag = carry;
        return r;
    }
    jsr_push(ret_copy);
    (void)CopyObjectPlaceBack(previous, slot);
    jsr_pop();
    value = RD8(OBJ->hitbox_on[slot - 1]);
    WR8(OBJ->hitbox_on[slot], value);
    r.third = RD8(OBJ->frame[slot - 1]);
    frame = (uint8_t)(r.third - base);
    row = frame < 0x13 ? frame : 0x13;
    row = (uint8_t)(row << 2);
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(table + row)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(table + 1 + row)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(table + 2 + row)));
    value = rd8((uint16_t)(table + 3 + row));
    WR8(OBJ->hitbox_half_h[slot], value);
    value = RD8(OBJ->facing_left[slot - 1]);
    WR8(OBJ->facing_left[slot], value);
    r.arm = FP_FOLLOW;
    r.first = value;
    r.second = row;
    r.fourth = base;
    return r;
}

/* State 0: LoadObjectRecord's exit. Later: A the facing byte, Y the row, C 0, V from the SEC and SBC of the base from the frame. */
static inline Ended follow_previous_piece_exit_shim(ObjArm r)
{
    if (r.arm == FP_RECORD) {
        return ended_of_record(r.first, r.second, r.flag);
    }
    return ended_with_overflow(ended_with_carry(ended_with_y(ended_load(r.first), r.second), false),
                               sub_bytes(r.third, r.fourth, true).overflow);
}

/*
 * An object throws itself on an arc toward the player: loads its record, an upward speed, a horizontal speed from a
 * table of word pairs by the player's distance along X (a pair per 32 pixels), then the arc's start speed, reversed
 * when the player is on its left. The body of StartLobTowardPlayer and its copy.
 */
static inline ObjArm start_lob(uint8_t slot, uint16_t table, const uint16_t ret[5])
{
    ObjArm r = { 0 };
    uint8_t y, value;
    bool carry;

    jsr_push(ret[0]);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->vy_hi[slot], 0xF0);
    jsr_push(ret[1]);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    jsr_push(ret[2]);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    y = (uint8_t)((r.distance.distance >> 5) << 1);
    value = rd8((uint16_t)(table + 1 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(table + y));
    WR8(OBJ->vx_lo[slot], value);
    r.third = value;
    r.y = y;
    WR8(OBJ->vy_hi[slot], 0xFB);
    WR8(OBJ->vy_lo[slot], 0x80);
    jsr_push(ret[3]);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 1);
    if (r.fourth != 0) {
        jsr_push(ret[4]);
        r.sum = NegateVelocityX(slot);
        jsr_pop();
    }
    return r;
}

/* Each step's exit in order: the record, LDA #$F0, the move, the distance, the speed's LDA with CLC and TAY, LDA #$80, SideOfPlayer, the AND, and NegateVelocityX when the player is on the left. */
static inline Ended start_lob_exit_shim(ObjArm r)
{
    Ended e = ended_of_record(r.first, r.second, r.flag);

    e = ended_then(e, ended_load(0xF0));
    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(e, ended_of_distance(r.distance));
    e = ended_then(e, ended_with_y(ended_with_carry(ended_load(r.third), false), r.y));
    e = ended_then(e, ended_load(0x80));
    e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.fourth));
    if (r.fourth != 0) {
        e = ended_then(e, ended_of_sum(r.sum));
    }
    return e;
}

/* The arc's later frames: the frame list steps, the Y speed grows by $24 and the object moves. */
static inline ObjArm step_lob_arc(uint8_t slot, uint8_t entry_y, uint16_t list, uint16_t ret_step)
{
    ObjArm r = { 0 };
    uint8_t exit_y = entry_y;
    Sum low, high;

    WR8(ZP->temp11, (uint8_t)(list >> 8));
    WR8(ZP->temp10, (uint8_t)list);
    jsr_push(ret_step);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    low = add_bytes(RD8(OBJ->vy_lo[slot]), 0x24, false);
    WR8(OBJ->vy_lo[slot], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[slot]), 0, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    r.move = ApplyVelocity(slot);
    return r;
}

/* The step's exit with the move's over it. */
static inline Ended step_lob_arc_exit_shim(ObjArm r)
{
    return ended_then(frame_step_exit_shim(r), ended_of_axis_move(r.move));
}

/*
 * A hop: X speed from a byte table by an index the caller read (which way the object faces), Y speed -3.00, the frame, f35c0
 * set to 8 and the step byte advanced. Ends with N, Z from the step; A 8; Y the index byte.
 */
static inline ObjArm hop_from_index(uint8_t slot, uint8_t index, uint16_t table, uint8_t frame)
{
    ObjArm r = { 0 };
    uint8_t facing = index;
    uint8_t step;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(table + facing)));
    WR8(OBJ->vy_hi[slot], 0xFD);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->frame[slot], frame);
    WR8(OBJ->f35c0[slot], 8);
    step = RD8(OBJ->f2800[slot]);
    step = (uint8_t)(step + 1);
    WR8(OBJ->f2800[slot], step);
    r.first = step;
    r.second = facing;
    return r;
}

static inline Ended hop_from_index_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_a(ended_nz(r.first), 8), r.second);
}

/* The speed-and-turn body three overlays share: see PickSpeedAndTurnFromTable. */
static inline ObjArm pick_speed_and_turn(uint8_t slot, uint16_t entries, uint16_t ret_side)
{
    ObjArm r = { 0 };
    uint8_t row = RD8(OBJ->f3670[slot]);
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(entries + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(entries + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(entries + 3 + y)));
    value = rd8((uint16_t)(entries + 2 + y));
    WR8(OBJ->vy_lo[slot], value);
    r.first = value;
    r.flag = (row & 0x40) != 0;
    r.y = y;
    r.second = INC8(OBJ->f3510[slot]);
    jsr_push(ret_side);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.third = (uint8_t)(r.side.sides & 1);
    r.arm = PT_STEPPED;
    if (r.third == 0) {
        return r;
    }
    r.arm = PT_TURNED;
    r.sum = NegateVelocityX(slot);
    return r;
}

/* The last LDA with the ASL's C and TAY, the INC, SideOfPlayer, the AND, and NegateVelocityX when turned. */
static inline Ended pick_speed_and_turn_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_with_carry(ended_load(r.first), r.flag), r.y);

    e = ended_then(e, ended_nz(r.second));
    e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.third));
    return r.arm == PT_STEPPED ? e : ended_then(e, ended_of_sum(r.sum));
}

/* The state pick three overlays share: see PickStateByPlayerDistance. */
static inline ObjArm pick_state_by_distance(uint8_t slot, bool carry_in, uint16_t table, uint16_t ret_sum)
{
    ObjArm r = { 0 };
    uint8_t high, y, value;

    WR8(OBJ->f2800[slot], 0);
    high = RD8(OBJ->x_hi[slot]);
    if (high != 0) {
        WR8(OBJ->state[slot], 3);
        r.arm = PS_X_HIGH;
        return r;
    }
    high = RD8(OBJ->y_hi[slot]);
    if (high != 0) {
        WR8(OBJ->state[slot], 4);
        r.arm = PS_Y_HIGH;
        return r;
    }
    jsr_push(ret_sum);
    r.sum = PlayerDistanceSum(slot, carry_in);
    jsr_pop();
    y = (uint8_t)(r.sum.value & 0x0F);
    value = rd8((uint16_t)(table + y));
    WR8(OBJ->state[slot], value);
    r.arm = PS_TABLE;
    r.first = value;
    r.y = y;
    return r;
}

/* LDA #3 or LDA #4; or the state's LDA from the table, with the sum's C and V and Y its index. */
static inline Ended pick_state_by_distance_exit_shim(ObjArm r)
{
    if (r.arm == PS_X_HIGH) {
        return ended_load(3);
    }
    if (r.arm == PS_Y_HIGH) {
        return ended_load(4);
    }
    return ended_with_y(ended_of_sum((Sum){ r.first, r.sum.carry, r.sum.overflow }), r.y);
}

/* Turns the object to face the player in the states a table names: see FaceThePlayerInSomeStates. */
static inline ObjArm face_player_in_states(uint8_t slot, uint16_t table, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t entry = rd8((uint16_t)(table + state));

    r.second = state;
    r.arm = FS_NONE;
    r.first = entry;
    if (entry == 0) {
        return r;
    }
    jsr_push(ret);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.first = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.first);
    r.arm = FS_FACED;
    return r;
}

/* Y the state; A the table entry, or SideOfPlayer's exit with the AND's A over it. */
static inline SideEnd face_player_in_states_exit_shim(ObjArm r)
{
    SideEnd s = { ended_kept(), false, { 0, false, false } };

    s.end = ended_with_y(ended_load(r.first), r.second);
    if (r.arm == FS_FACED) {
        s.side_used = true;
        s.side = r.side;
    }
    return s;
}

/* Moves the object by a velocity built from its next_kind and clears the speeds: see ThrowByNextKind. */
static inline ObjArm throw_by_next_kind(uint8_t slot, uint16_t speeds, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t index = (uint8_t)(kind & 1);
    uint8_t shifted;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(speeds + index)));
    kind = RD8(OBJ->next_kind[slot]);
    shifted = (uint8_t)(kind << 4);
    WR8(OBJ->vy_hi[slot], (uint8_t)((uint8_t)(shifted ^ 0xFF) + 1));
    jsr_push(ret);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    r.y = index;
    return r;
}

/* Y the index; the move's A and flags. */
static inline Ended throw_by_next_kind_exit_shim(ObjArm r)
{
    return ended_then(ended_with_y(ended_kept(), r.y), ended_of_axis_move(r.move));
}

/* Picks a pair of bytes from a table at random: see PickTimersAtRandom. */
static inline ObjArm pick_pair_at_random(uint8_t slot, uint16_t table, uint16_t ret, bool angle)
{
    ObjArm r = { 0 };
    uint8_t y, value;

    jsr_push(ret);
    r.random = NextRandom();
    jsr_pop();
    y = (uint8_t)((r.random.value & 3) << 1);
    value = rd8((uint16_t)(table + y));
    if (angle) {
        WR8(OBJ->f35c0[slot], value);
    } else {
        WR8(OBJ->f3510[slot], value);
    }
    value = rd8((uint16_t)(table + 1 + y));
    if (angle) {
        WR8(OBJ->f3568[slot], value);
    } else {
        WR8(OBJ->life_timer[slot], value);
    }
    r.first = value;
    r.y = y;
    return r;
}

/* NextRandom's exit, then A the second byte, C 0 from the ASL, Y the index. */
static inline RandomEnd pick_pair_at_random_exit_shim(ObjArm r)
{
    RandomEnd s = { ended_kept(), true, { 0, false, false } };

    s.random = r.random;
    s.end = ended_with_y(ended_with_carry(ended_load(r.first), false), r.y);
    return s;
}

/* The first step of the walk-and-turn state three overlays share (see TurnOnRandomBeatWalkA). */
static inline ObjArm walk_on_random_beat_first(uint8_t slot, uint16_t base, uint16_t speeds, uint16_t timers)
{
    ObjArm r = { 0 };
    uint8_t y, value;
    SideResult side;

    jsr_push((uint16_t)(base + 8));
    side = SideOfPlayer(slot);
    jsr_pop();
    y = (uint8_t)((side.sides & 1) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(speeds + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(speeds + 1 + y)));
    jsr_push((uint16_t)(base + 0x1B));
    r.random = NextRandom();
    jsr_pop();
    y = (uint8_t)(r.random.value & 7);
    value = rd8((uint16_t)(timers + y));
    WR8(OBJ->life_timer[slot], value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    INC8(OBJ->f3510[slot]);
    r.first = INC8(OBJ->f2800[slot]);
    r.second = value;
    r.y = y;
    return r;
}

/* NextRandom's exit with Y the timer's index; A the timer, N and Z from the last INC. */
static inline Ended walk_on_random_beat_first_exit_shim(ObjArm r)
{
    return ended_then(ended_with_y(ended_of_random(r.random), r.y), ended_with_a(ended_nz(r.first), r.second));
}

/* The later steps: animate facing the player (list 2), count the timer down and walk; at zero go to state 5. */
static inline ObjArm walk_on_random_beat_follow(uint8_t slot, uint16_t ret_step, ObjArm (*step)(uint8_t, uint8_t))
{
    ObjArm r = { 0 };
    ObjArm frames;

    jsr_push(ret_step);
    frames = step(slot, 2);
    jsr_pop();
    r.step = frames.step;
    r.y = frames.y;
    r.first = DEC8(OBJ->life_timer[slot]);
    if (r.first != 0) {
        r.arm = WF_WALKING;
        r.move = ApplyVelocityX(slot);
        return r;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    r.arm = WF_DONE;
    return r;
}

/* LDY #2 and the step's exit, the DEC's N and Z; then the move's exit, or LDA #5. */
static inline Ended walk_on_random_beat_follow_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_then(ended_with_y(ended_kept(), 2), frame_step_exit_shim(r)), ended_nz(r.first));

    return ended_then(e, r.arm == WF_WALKING ? ended_of_axis_move(r.move) : ended_load(5));
}

/* The two-step wait the WaitThenState2 copies share: the start, the timer counting, the new state. */
enum { WT_START, WT_COUNTING, WT_DONE };

/* Start: A $1C, the step's N and Z. Later: LDY #2 and the step, then the timer's N and Z, or LDA #2 at zero. */
static inline Ended wait_then_state2_exit_shim(ObjArm r)
{
    if (r.arm == WT_START) {
        return ended_with_a(ended_nz(r.first), 0x1C);
    }
    return ended_then(ldy_frame_step_shim(2, r), r.arm == WT_COUNTING ? ended_nz(r.first) : ended_load(2));
}

/* The walk on a random beat its copies dispatch to: the first step or the follow step. */
enum { TW_FIRST, TW_FOLLOW };

/* The part that ran: the follow step's exit, or the first step's. */
static inline Ended turn_on_random_beat_walk_exit_shim(ObjArm r)
{
    if (r.arm == TW_FOLLOW) {
        return walk_on_random_beat_follow_exit_shim(obj_arm(r.sub));
    }
    return walk_on_random_beat_first_exit_shim(obj_arm(r.sub));
}

/* The walk on changed frames its copies share (WalkWhileFrameChanges): held, or moved by the phase's speed. */
enum { WW_HELD, WW_MOVED };

/* LDY #0 and the step; after a change, the step, LDY of the phase and ApplyVelocityX's exit. */
static inline Ended walk_while_frame_changes_exit_shim(ObjArm r)
{
    if (r.arm == WW_HELD) {
        return ldy_frame_step_shim(0, r);
    }
    return ended_then(ended_then(frame_step_exit_shim(r), ended_with_y(ended_kept(), r.first)), ended_of_axis_move(r.move));
}

/* The lob its copies dispatch to (StartLobTowardPlayer): the start or the arc. */
enum { LB_START, LB_ARC };

/* The part that ran: StepLobArc's exit, or the start's. */
static inline Ended start_lob_toward_player_exit_shim(ObjArm r)
{
    return r.arm == LB_ARC ? step_lob_arc_exit_shim(obj_arm(r.sub)) : start_lob_exit_shim(obj_arm(r.sub));
}

/* The object becomes `type` in state 0. */
static inline ObjArm become_type(uint8_t slot, uint8_t type)
{
    ObjArm r = { 0 };

    WR8(OBJ->kind[slot], type);
    WR8(OBJ->state[slot], 0);
    r.first = type;
    return r;
}

/* The object enters `state` when bit 7 of its f31a0 is set. */
static inline ObjArm enter_state_if_flagged(uint8_t slot, uint8_t state)
{
    ObjArm r = { 0 };
    uint8_t flags = RD8(OBJ->status_flags[slot]);
    uint8_t bit = (uint8_t)(flags & 0x80);

    r.first = 0;
    if (bit == 0) {
        return r;
    }
    WR8(OBJ->state[slot], state);
    r.first = state;
    return r;
}

/* become_type and enter_state_if_flagged: the LDA of the byte they end on. */
static inline Ended load_exit_shim(ObjArm r)
{
    return ended_load(r.first);
}

/* What a sound-playing routine leaves when instructions before its sound call (and after it) are put around the call. */
static inline SoundEnd sound_end_after(Ended e, SoundEnd s)
{
    if (s.sounded) {
        s.before = ended_then(e, s.before);
    } else {
        s.end = ended_then(e, s.end);
    }
    return s;
}

static inline SoundEnd sound_end_then(SoundEnd s, Ended rest)
{
    s.end = ended_then(s.end, rest);
    return s;
}

/* An animation that ends in a new type (BecomeType9CWhenAnimationEnds and its copy). */
enum { BT_STEPPED, BT_BECAME };

/* LDY #8 and the step; when the list ended, become_type's LDA over it. */
static inline Ended become_type_when_animation_ends_exit_shim(ObjArm r)
{
    Ended e = ldy_frame_step_shim(8, r);

    return r.arm == BT_STEPPED ? e : ended_then(e, ended_load(r.first));
}


/* PROTOTYPES BEGIN */
/* object/enemies_common_temp.c */
ScreenSoundEnd PlayHitSoundA(uint8_t slot);
ScreenSoundEnd PlayHitSoundB(uint8_t slot);
ScreenSoundEnd PlayHitSoundC(uint8_t slot);
ScreenSoundEnd PlayHitSoundD(uint8_t slot);
ScreenSoundEnd PlayHitSoundE(uint8_t slot);
ScreenSoundEnd PlayHitSoundF(uint8_t slot);
ScreenSoundEnd PlayHitSoundG(uint8_t slot);
ScreenSoundEnd PlayHitSoundH(uint8_t slot);
ScreenSoundEnd PlayHitSoundI(uint8_t slot);
ScreenSoundEnd PlayHitSoundJ(uint8_t slot);
ScreenSoundEnd PlayHitSoundK(uint8_t slot);
ScreenSoundEnd PlayHitSoundL(uint8_t slot);
Ended DropItemWhenHitA(uint8_t slot);
Ended DropItemWhenHitB(uint8_t slot);
Ended DropItemWhenHitC(uint8_t slot);
Ended DropItemWhenHitD(uint8_t slot);
Ended DropItemWhenHitE(uint8_t slot);
Ended DropItemWhenHitF(uint8_t slot);
Ended DropItemWhenHitG(uint8_t slot);
Ended DropItemWhenHitH(uint8_t slot);
Ended DropItemWhenHitI(uint8_t slot);
ScreenSoundEnd PlayHitSoundM(uint8_t slot);
ObjArm FaceThePlayerInSomeStatesC(uint8_t slot);
ObjArm FaceThePlayerInSomeStatesD(uint8_t slot);
ObjArm ThrowByNextKindB(uint8_t slot);
ObjArm PickTimersAtRandomB(uint8_t slot);
ObjArm PickAngleAtRandomB(uint8_t slot);
Ended WalkWhileFrameChangesD(uint8_t slot, uint8_t index);
/* object/enemies_stage04_temp.c */
Ended ClampSpeedXToTwo(uint8_t slot);
Ended ThrowUpward(uint8_t slot);
ObjArm CountdownThenState4(uint8_t slot);
ObjArm ChangeToType9B(uint8_t slot);
ObjArm EnterState6IfFlagged(uint8_t slot);
ObjArm CountdownStopThenState4(uint8_t slot);
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit);
Ended LoadRecordStartAngle40(uint8_t slot);
Ended LoadRecordTimer50(uint8_t slot);
ObjArm StepFramesFacingPlayer(uint8_t slot, uint8_t index);
ObjArm StepFramesFacingPlayerAlways(uint8_t slot, uint8_t index);
ObjArm WalkWhileFrameChanges(uint8_t slot);
ObjArm WaitThenState2(uint8_t slot);
ObjArm BecomeType9CWhenAnimationEnds(uint8_t slot);
Ended LoadRecordFacingAndRandomFrame(uint8_t slot);
ObjArm PickSpeedAndTurnFromTableB(uint8_t slot);
ObjArm SpawnSixPiecesA(uint8_t slot);
Ended OrbitStepBack(uint8_t slot);
Ended OrbitStepForward(uint8_t slot);
Ended StartTypeAndLaunchUp(uint8_t slot, uint8_t fresh);
ObjArm AnimateThenFireOrWait(uint8_t slot);
Ended BurstIntoSixPieces(uint8_t slot);
ScreenSoundEnd SoundEveryFourthFrameB(uint8_t slot, uint8_t entry_y);
ObjArm BurstWhenFlaggedHit(uint8_t slot, uint8_t entry_y);
ObjArm PickStateByPlayerDistance(uint8_t slot, bool carry_in);
ObjArm AnimateMoveThenSetUpNextStep(uint8_t slot, bool carry_in, uint8_t entry_y);
ObjArm ChargeWhenPlayerNear(uint8_t slot);
ObjArm TurnOnRandomBeatWalkA(uint8_t slot);
ObjArm TurnOnRandomBeatWalkFollowA(uint8_t slot);
Ended FallAnimatedThenBecomeKind9B(uint8_t slot);
ObjArm StartLobTowardPlayer(uint8_t slot, uint8_t entry_y);
ObjArm StepLobArc(uint8_t slot, uint8_t entry_y);
/* object/enemies_stage05_temp.c */
Ended SelectRasterStage1(void);
Ended SetHitboxDxFromFrame(uint8_t slot);
Ended PhaseTimerToState1(uint8_t slot);
Ended SetHitboxFromFrameRows(uint8_t slot);
Ended EnterState7OnFlagOrYSignChange(uint8_t slot);
Ended SetHitboxFromFrameRowsIndirect(uint8_t slot);
ObjArm LaunchHopByF3510(uint8_t slot);
ObjArm CountdownStopThenState3(uint8_t slot);
Ended StartWithFrame1A(uint8_t slot);
Ended ThrowByNextKind(uint8_t slot);
Ended AnimateThenState1(uint8_t slot);
Ended AnimateThenNextStep(uint8_t slot);
Ended FollowParentYAndLeave(uint8_t slot);
Ended AnimateThenNextStepB(uint8_t slot);
ObjArm CountdownThenFallOrBecomeType9B(uint8_t slot);
RandomEnd PickTimersAtRandom(uint8_t slot);
RandomEnd PickAngleAtRandom(uint8_t slot);
Ended AnimateMoveAndSag(uint8_t slot);
ObjArm StepFramesFacingPlayerC(uint8_t slot, uint8_t index);
SideEnd FaceThePlayerInSomeStates(uint8_t slot);
Ended NextPhaseAtRandom(uint8_t slot);
Ended ShowOnScreenAndWrapX(uint8_t slot);
ObjArm StartChargeWhenPlayerNear(uint8_t slot);
ObjArm WaitThenState2B(uint8_t slot);
Ended ChooseNextMoveWhenFacingAway(uint8_t slot);
Ended CountTimerPairDownOrRepick(uint8_t slot);
Ended CountAnglePairDownOrRepick(uint8_t slot);
Ended AnimateHitboxThenExpire(uint8_t slot);
Ended WalkWhileFrameChangesB(uint8_t slot, uint8_t index);
Ended StartThenBlinkAndExpire(uint8_t slot);
Ended LeapAwayFromPlayerTable(uint8_t slot);
ObjArm PickStateByPlayerDistanceB(uint8_t slot, bool carry_in);
ObjArm PickSpeedAndTurnFromTableC(uint8_t slot);
ObjArm TurnOnRandomBeatWalkB(uint8_t slot);
ObjArm TurnOnRandomBeatWalkFollowB(uint8_t slot);
Ended SpawnShotFromFrame(uint8_t slot);
Ended StartAndMakeNeighbour(uint8_t slot);
ObjArm StartWithFrameAndFace(uint8_t slot);
Ended StartThenAnimateAndExpire(uint8_t slot);
SoundEnd CountdownSoundThenNextState(uint8_t slot, uint8_t entry_y);
ObjArm SoundEveryFourthFrame(uint8_t slot, uint8_t entry_y);
SoundEnd CountdownSoundThenFollowParent(uint8_t slot, uint8_t entry_y);
Ended AnimateThenNumberGroupAndLeave(uint8_t slot);
ObjArm FollowPreviousPieceB(uint8_t slot);
ObjArm AnimateMoveThenSetUpAtRandom(uint8_t slot);
Ended StartSidewaysAndSpawn29(uint8_t slot);
Ended StartSidewaysAndSpawn29B(uint8_t slot);
Ended BurstIntoEightPieces(uint8_t slot);
RandomEnd StartGroupThenThrow(uint8_t slot);
Ended ContactWithPlayerBox14By20(uint8_t slot);
/* object/enemies_stage07_temp.c */
ObjArm EnterState4IfFlagged(uint8_t slot);
Ended ScrollNotAtRightEnd(void);
Ended SetHitboxSizeFromFrame(uint8_t slot);
ObjArm ChangeToType9C(uint8_t slot);
Ended EnterNextStateWhenPlayerNear(uint8_t slot);
Ended SwingOnCosine(uint8_t slot);
Ended AnimateThenFrameRecord(uint8_t slot);
Ended StartThenAnimate(uint8_t slot);
Ended StartWithRecordTableRow(uint8_t slot);
Ended FallAndLeaveWhenLow(uint8_t slot);
Ended AnimateThenRemove(uint8_t slot);
Ended RemoveFirstFreeSlotFrom1C(void);
Ended SpawnEightPiecesType37(uint8_t slot);
Ended SpawnPiecesWhenFlaggedHit(uint8_t slot);
Ended StartAtRandomSpeedTowardPlayer(uint8_t slot);
Ended CountTimerPairDownThenSpawn36(uint8_t slot);
Ended StartFromScreenEdgeByScroll(uint8_t slot);
/* object/enemies_stage08_temp.c */
Ended SetPhase2(uint8_t slot);
Ended SetPhase3(uint8_t slot);
Ended SetPhase4(uint8_t slot);
Ended SetSpeedFromPhaseTable(uint8_t slot);
ObjArm EnterState6IfFlaggedB(uint8_t slot);
Ended AdvanceState(uint8_t slot);
Ended EnterState3WithTallHitbox(uint8_t slot);
Ended SetHitboxFromFrameRowsB(uint8_t slot);
Ended StartFacingPlayerThenCheck(uint8_t slot);
Ended AnimateThenCountToState3(uint8_t slot);
Ended PickPhaseAtRandomA(uint8_t slot);
Ended PickPhaseAtRandomB(uint8_t slot);
Ended StartWithTimerByNextKind(uint8_t slot);
Ended StartWithFrameByNextKind(uint8_t slot);
Ended AnimateByKindThenNextState(uint8_t slot);
ObjArm StepFramesFacingPlayerE(uint8_t slot, uint8_t index);
ObjArm WalkWhileFrameChangesC(uint8_t slot);
ObjArm AnimateAndSpawnThenPause(uint8_t slot);
ObjArm StartLobTowardPlayerB(uint8_t slot, uint8_t entry_y);
ObjArm StepLobArcB(uint8_t slot, uint8_t entry_y);
/* object/enemies_stage0b_temp.c */
ObjArm CountdownThenState4B(uint8_t slot);
ObjArm ChangeToType9BB(uint8_t slot);
Ended SixteenBitTimerToState8(uint8_t slot);
Ended NoOpStateA(void);
Ended SetXSpeedFromFacing(uint8_t slot);
Ended EnterState6WhenHitHard(uint8_t slot);
Ended EnterState7IfYHighOrFlagged(uint8_t slot);
Ended NoOpStateB(void);
ObjArm LaunchHopByFacing(uint8_t slot);
ObjArm CountdownStopThenState2(uint8_t slot);
ObjArm StepFramesFacingPlayerD(uint8_t slot, uint8_t index);
ObjArm WaitThenState2C(uint8_t slot);
ObjArm BecomeType9CWhenAnimationEndsB(uint8_t slot);
Ended LoadRecordStartAngle40B(uint8_t slot);
Ended FrameOneBWhenFallingElseAnimate(uint8_t slot);
SideEnd FaceThePlayerInSomeStatesB(uint8_t slot);
Ended AnimateAlongListC3C2(uint8_t slot);
Ended SwingOnCosineB(uint8_t slot);
ObjArm PickSpeedAndTurnFromTable(uint8_t slot);
ObjArm TurnOnRandomBeatWalkC(uint8_t slot);
ObjArm TurnOnRandomBeatWalkFollowC(uint8_t slot);
Ended FlashFallAnimateThenRemove(uint8_t slot);
Ended StartAndMakeNeighbourB(uint8_t slot);
Ended EnterNextStateWhenHitOrPlayerNear(uint8_t slot);
Ended PlaceAtRandomOnEdge(uint8_t slot);
Ended StartFlyerAtRandom(uint8_t slot);
Ended SwoopOnCosineC(uint8_t slot);
ObjArm SpawnSixPiecesB(uint8_t slot);
ObjArm BurstWhenFlaggedHitB(uint8_t slot, uint8_t entry_y);
ObjArm SoundEveryFourthFrameC(uint8_t slot, uint8_t entry_y);
Ended StartFromScreenEdgeByPlayer(uint8_t slot);
ObjArm PickStateByPlayerDistanceC(uint8_t slot, bool carry_in);
Ended SpawnType2AThenRestartTimer(uint8_t slot);
Ended StartPieceInSlotsFrom28(uint8_t slot);
ObjArm KnockedUpWhenHit(uint8_t slot);
ObjArm KnockedUpWhenHitB(uint8_t slot);
Ended SetHitboxFromFrameRowsC(uint8_t slot);
ObjArm FollowPreviousPiece(uint8_t slot);
ObjArm AnimateMoveThenSetUpNextStepB(uint8_t slot, bool carry_in);
Ended LoadObjectRecordFromTable(uint8_t slot);
SoundEnd LaunchWithSoundThenDrift(uint8_t slot, uint8_t entry_y);
Ended SwingOnCosineThenLeapAtPlayer(uint8_t slot, uint8_t entry_y);
Ended StartAtRandomSpeedAndHeight(uint8_t slot);
ObjArm ChargeWhenPlayerNearB(uint8_t slot);
/* object/state_countdown.c */
uint8_t CountdownNextStepA(uint8_t slot);
uint8_t CountdownNextStateA(uint8_t slot);
uint8_t CountdownNextStateB(uint8_t slot);
uint8_t CountdownNextStepB(uint8_t slot);
uint8_t CountdownNextStateC(uint8_t slot);
uint8_t CountdownNextStateD(uint8_t slot);
uint8_t CountdownNextStateE(uint8_t slot);
uint8_t CountdownNextStepC(uint8_t slot);
/* PROTOTYPES END */

#endif
