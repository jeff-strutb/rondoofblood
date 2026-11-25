/*
 * Lot L5: the stage-specific object code of the stage overlays, as typed C. A body takes the
 * object's slot and returns, as a plain value, the byte its last instruction left N and Z from;
 * its abi_ wrapper turns that into the flags the assembly leaves.
 */
#ifndef RONDO_ENEMY_OBJECTS_H
#define RONDO_ENEMY_OBJECTS_H

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
 * Bodies several stage files share (one C body for every near copy). Each takes the object's
 * slot and says how it ended.
 */

/* The timer counts down; at zero the object enters `state`. */
static inline Ended countdown_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    if (left != 0) {
        return ended_nz(left);
    }
    WR8(OBJ->state[slot], state);
    return ended_load(state);
}

/* The same, and at zero the object's f3408 and f3460 are cleared first. */
static inline Ended countdown_stop_then_state(uint8_t slot, uint16_t timer, uint8_t state)
{
    uint8_t left = RD8_IDX(timer, slot);

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    if (left != 0) {
        return ended_nz(left);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], state);
    return ended_load(state);
}

/*
 * Up to six objects of a type started at the object's place, each from a free slot from $30, each given a
 * life_timer, f3510 and speeds from a four byte entry of a table: the body SpawnSixPiecesA and B share.
 */
static inline Ended spawn_six_pieces(uint8_t slot, uint8_t type, uint16_t table, uint16_t ret_find, uint16_t ret_start)
{
    uint8_t piece = 0, found_slot, last, row, value;
    bool found;
    Ended e = ended_with_y(ended_kept(), 0);

    do {
        WR8(ZP->hit_slot, piece);
        jsr_push(ret_find);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = ended_then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            return e;
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
        e = ended_with_y(ended_with_carry(ended_with_a(ended_nz((uint8_t)(piece - 6)), value), piece >= 6), piece);
    } while (piece < 6);
    return e;
}

/* Sound $22 when the low two bits of the object's f3618 are zero: the body SoundEveryFourthFrame and its copy share. */
static inline SoundEnd sound_every_fourth_frame(uint8_t slot, uint8_t entry_y, uint16_t ret)
{
    SoundEnd result = { ended_kept(), ended_kept(), false, 0, { 0 } };
    uint8_t bits = RD8(OBJ->f3618[slot]);

    bits = (uint8_t)(bits & 3);
    if (bits != 0) {
        result.end = ended_load(bits);
        return result;
    }
    result.sounded = true;
    result.command = 0x22;
    result.before = ended_load(0x22);
    jsr_push(ret);
    result.call = SoundCommand(0x22, slot, entry_y);
    jsr_pop();
    return result;
}

/*
 * A hit that has set bit 7 of the status flags sends an object below a state into a tumble: its f3250 and animation
 * stopped, sprite set 3, frame 0, speed -$0102 upward (both bytes $FE) and the state given. The body 117C:1EA and
 * 1173:26A share.
 */
static inline Ended knock_up_when_flagged(uint8_t slot, uint8_t below, uint8_t state)
{
    uint8_t bit = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    uint8_t now;
    Ended e = ended_load(bit);

    if (bit == 0) {
        return e;
    }
    now = RD8(OBJ->state[slot]);
    e = ended_compare(now, below);
    if (now >= below) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f2a10[slot], 3);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0xFE);
    WR8(OBJ->state[slot], state);
    return ended_then(e, ended_load(state));
}

/*
 * A piece that follows the object one slot before it: in state 0 it loads its record; later it takes the place of
 * slot less 1, its f3250 from there, a hit rectangle from a table of four byte rows picked by that object's frame
 * less a base, and its facing. The body 117D:277 and 093D:6A4 share.
 */
static inline Ended follow_previous_piece(uint8_t slot, uint8_t base, uint16_t table, uint16_t ret_copy)
{
    uint8_t previous = (uint8_t)(slot - 1);
    uint8_t last, nz, placed, value, row;
    bool carry;
    Sum frame;
    Ended e;

    if (RD8(OBJ->state[slot]) == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    e = ended_with_a(ended_with_y(ended_nz(previous), previous), slot);
    jsr_push(ret_copy);
    placed = CopyObjectPlaceBack(previous, slot);
    jsr_pop();
    e = ended_then(e, ended_load(placed));
    value = RD8(OBJ->hitbox_on[slot - 1]);
    WR8(OBJ->hitbox_on[slot], value);
    frame = sub_bytes(RD8(OBJ->frame[slot - 1]), base, true);
    row = frame.value < 0x13 ? frame.value : 0x13;
    row = (uint8_t)(row << 2);
    WR8(OBJ->hitbox_dx[slot], rd8((uint16_t)(table + row)));
    WR8(OBJ->hitbox_dy[slot], rd8((uint16_t)(table + 1 + row)));
    WR8(OBJ->hitbox_half_w[slot], rd8((uint16_t)(table + 2 + row)));
    value = rd8((uint16_t)(table + 3 + row));
    WR8(OBJ->hitbox_half_h[slot], value);
    value = RD8(OBJ->facing_left[slot - 1]);
    WR8(OBJ->facing_left[slot], value);
    e = ended_with_overflow(ended_with_carry(ended_with_y(ended_load(value), row), false), frame.overflow);
    return e;
}

/*
 * An object throws itself on an arc toward the player: loads its record, an upward speed, a horizontal speed from a
 * table of word pairs by the player's distance along X (a pair per 32 pixels), then the arc's start speed, reversed
 * when the player is on its left. The body of StartLobTowardPlayer and its copy.
 */
static inline Ended start_lob(uint8_t slot, uint16_t table, const uint16_t ret[5])
{
    uint8_t last, nz, y, value, bit;
    bool carry;
    AxisMove move;
    PlayerDistance distance;
    SideResult side;
    Ended e;

    jsr_push(ret[0]);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->vy_hi[slot], 0xF0);
    e = ended_then(e, ended_load(0xF0));
    jsr_push(ret[1]);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    jsr_push(ret[2]);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(e, ended_of_distance(distance));
    y = (uint8_t)((distance.distance >> 5) << 1);
    value = rd8((uint16_t)(table + 1 + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(table + y));
    WR8(OBJ->vx_lo[slot], value);
    e = ended_then(e, ended_with_y(ended_with_carry(ended_load(value), false), y));
    WR8(OBJ->vy_hi[slot], 0xFB);
    WR8(OBJ->vy_lo[slot], 0x80);
    e = ended_then(e, ended_load(0x80));
    jsr_push(ret[3]);
    side = SideOfPlayer(slot);
    jsr_pop();
    bit = (uint8_t)(side.sides & 1);
    e = ended_then(ended_then(e, ended_of_side(side)), ended_load(bit));
    if (bit != 0) {
        jsr_push(ret[4]);
        e = ended_then(e, ended_of_sum(NegateVelocityX(slot)));
        jsr_pop();
    }
    return e;
}

/* The arc's later frames: the frame list steps, the Y speed grows by $24 and the object moves. */
static inline Ended step_lob_arc(uint8_t slot, uint8_t entry_y, uint16_t list, uint16_t ret_step)
{
    uint8_t exit_y = entry_y;
    FrameStep step;
    AxisMove move;
    Sum low, high;
    Ended e;

    WR8(ZP->temp11, (uint8_t)(list >> 8));
    WR8(ZP->temp10, (uint8_t)list);
    jsr_push(ret_step);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    low = add_bytes(RD8(OBJ->vy_lo[slot]), 0x24, false);
    WR8(OBJ->vy_lo[slot], low.value);
    high = add_bytes(RD8(OBJ->vy_hi[slot]), 0, low.carry);
    WR8(OBJ->vy_hi[slot], high.value);
    move = ApplyVelocity(slot);
    return ended_then(e, ended_of_axis_move(move));
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

/*
 * A hop: X speed from a byte table by an index the caller read (which way the object faces), Y speed -3.00, the frame, f35c0
 * set to 8 and the step byte advanced. Returns N, Z from the step; A 8; Y the index byte.
 */
static inline Ended hop_from_index(uint8_t slot, uint8_t index, uint16_t table, uint8_t frame)
{
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
    return ended_with_y(ended_with_a(ended_nz(step), 8), facing);
}


/* The speed-and-turn body three overlays share: see PickSpeedAndTurnFromTable. */
static inline Ended pick_speed_and_turn(uint8_t slot, uint16_t entries, uint16_t ret_side)
{
    uint8_t row = RD8(OBJ->f3670[slot]);
    uint8_t y = (uint8_t)(row << 2);
    uint8_t value, bit;
    SideResult side;
    Ended e;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(entries + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(entries + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(entries + 3 + y)));
    value = rd8((uint16_t)(entries + 2 + y));
    WR8(OBJ->vy_lo[slot], value);
    e = ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y);
    value = INC8(OBJ->f3510[slot]);
    e = ended_then(e, ended_nz(value));
    jsr_push(ret_side);
    side = SideOfPlayer(slot);
    jsr_pop();
    bit = (uint8_t)(side.sides & 1);
    e = ended_then(ended_then(e, ended_of_side(side)), ended_load(bit));
    if (bit == 0) {
        return e;
    }
    return ended_then(e, ended_of_sum(NegateVelocityX(slot)));
}

/* The state pick three overlays share: see PickStateByPlayerDistance. */
static inline Ended pick_state_by_distance(uint8_t slot, bool carry_in, uint16_t table, uint16_t ret_sum)
{
    uint8_t high, y, value;
    Sum sum;

    WR8(OBJ->f2800[slot], 0);
    high = RD8(OBJ->x_hi[slot]);
    if (high != 0) {
        WR8(OBJ->state[slot], 3);
        return ended_load(3);
    }
    high = RD8(OBJ->y_hi[slot]);
    if (high != 0) {
        WR8(OBJ->state[slot], 4);
        return ended_load(4);
    }
    jsr_push(ret_sum);
    sum = PlayerDistanceSum(slot, carry_in);
    jsr_pop();
    y = (uint8_t)(sum.value & 0x0F);
    value = rd8((uint16_t)(table + y));
    WR8(OBJ->state[slot], value);
    return ended_with_y(ended_of_sum((Sum){ value, sum.carry, sum.overflow }), y);
}

/* Turns the object to face the player in the states a table names: see FaceThePlayerInSomeStates. */
static inline SideEnd face_player_in_states(uint8_t slot, uint16_t table, uint16_t ret)
{
    SideEnd result = { ended_kept(), false, { 0, false, false } };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t entry = rd8((uint16_t)(table + state));
    uint8_t bit;

    result.end = ended_with_y(ended_load(entry), state);
    if (entry == 0) {
        return result;
    }
    jsr_push(ret);
    result.side = SideOfPlayer(slot);
    jsr_pop();
    result.side_used = true;
    bit = (uint8_t)(result.side.sides & 1);
    WR8(OBJ->facing_left[slot], bit);
    result.end = ended_with_y(ended_load(bit), state);
    return result;
}

/* Moves the object by a velocity built from its next_kind and clears the speeds: see ThrowByNextKind. */
static inline Ended throw_by_next_kind(uint8_t slot, uint16_t speeds, uint16_t ret)
{
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t index = (uint8_t)(kind & 1);
    uint8_t shifted;
    AxisMove move;

    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(speeds + index)));
    kind = RD8(OBJ->next_kind[slot]);
    shifted = (uint8_t)(kind << 4);
    WR8(OBJ->vy_hi[slot], (uint8_t)((uint8_t)(shifted ^ 0xFF) + 1));
    jsr_push(ret);
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    return ended_then(ended_with_y(ended_kept(), index), ended_of_axis_move(move));
}

/* Picks a pair of bytes from a table at random: see PickTimersAtRandom. */
static inline RandomEnd pick_pair_at_random(uint8_t slot, uint16_t table, uint16_t ret, bool angle)
{
    RandomEnd result = { ended_kept(), true, { 0, false, false } };
    uint8_t y, value;

    jsr_push(ret);
    result.random = NextRandom();
    jsr_pop();
    y = (uint8_t)((result.random.value & 3) << 1);
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
    result.end = ended_with_y(ended_with_carry(ended_load(value), false), y);
    return result;
}

/*
 * The turn-around step three overlays share (see TurnAroundAndRecordFacing): the facing byte is
 * kept, a random f3670 is made, a speed and turn picked, both speeds reversed and halved.
 */
static inline Ended turn_around_body(uint8_t slot, bool carry_in, Ended (*pick)(uint8_t), uint16_t base)
{
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t step;
    Sum sum;
    Ended e;

    WR8(OBJ->f3720[slot], facing);
    jsr_push((uint16_t)(base + 9));
    sum = PlayerDistanceSum(slot, carry_in);
    jsr_pop();
    WR8(OBJ->f3670[slot], (uint8_t)(sum.value & 0x0F));
    e = ended_with_overflow(ended_with_carry(ended_load((uint8_t)(sum.value & 0x0F)), sum.carry), sum.overflow);
    jsr_push((uint16_t)(base + 0x11));
    e = ended_then(e, pick(slot));
    jsr_pop();
    jsr_push((uint16_t)(base + 0x14));
    e = ended_then(e, ended_of_sum(NegateVelocityX(slot)));
    jsr_pop();
    jsr_push((uint16_t)(base + 0x17));
    e = ended_then(e, ended_of_sum(NegateVelocityY(slot)));
    jsr_pop();
    jsr_push((uint16_t)(base + 0x1A));
    e = ended_then(e, ended_of_arith(HalveObjectVelocity(slot)));
    jsr_pop();
    WR8(OBJ->life_timer[slot], 0x0C);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    step = INC8(OBJ->f2800[slot]);
    return ended_then(e, ended_with_a(ended_nz(step), 0x0C));
}

/* The first step of the walk-and-turn state three overlays share (see TurnOnRandomBeatWalkA). */
static inline Ended walk_on_random_beat_first(uint8_t slot, uint16_t base, uint16_t speeds, uint16_t timers)
{
    uint8_t y, value, phase;
    SideResult side;
    RandomResult random;
    Ended e;

    jsr_push((uint16_t)(base + 8));
    side = SideOfPlayer(slot);
    jsr_pop();
    y = (uint8_t)((side.sides & 1) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(speeds + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(speeds + 1 + y)));
    jsr_push((uint16_t)(base + 0x1B));
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)(random.value & 7);
    value = rd8((uint16_t)(timers + y));
    WR8(OBJ->life_timer[slot], value);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    INC8(OBJ->f3510[slot]);
    phase = INC8(OBJ->f2800[slot]);
    e = ended_with_y(ended_of_random(random), y);
    return ended_then(e, ended_with_a(ended_nz(phase), value));
}

/* The later steps: animate facing the player (list 2), count the timer down and walk; at zero go to state 5. */
static inline Ended walk_on_random_beat_follow(uint8_t slot, uint16_t ret_step, FrameStepEnd (*step)(uint8_t, uint8_t))
{
    uint8_t left;
    FrameStepEnd frames;
    AxisMove move;
    Ended e;

    jsr_push(ret_step);
    frames = step(slot, 2);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 2), frames.end);
    left = DEC8(OBJ->life_timer[slot]);
    e = ended_then(e, ended_nz(left));
    if (left != 0) {
        move = ApplyVelocityX(slot);
        return ended_then(e, ended_of_axis_move(move));
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    return ended_then(e, ended_load(5));
}

/* The object becomes `type` in state 0. */
static inline Ended become_type(uint8_t slot, uint8_t type)
{
    WR8(OBJ->kind[slot], type);
    WR8(OBJ->state[slot], 0);
    return ended_load(type);
}

/* The object enters `state` when bit 7 of its f31a0 is set. */
static inline Ended enter_state_if_flagged(uint8_t slot, uint8_t state)
{
    uint8_t flags = RD8(OBJ->status_flags[slot]);
    uint8_t bit = (uint8_t)(flags & 0x80);

    if (bit == 0) {
        return ended_load(0);
    }
    WR8(OBJ->state[slot], state);
    return ended_load(state);
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
SideEnd FaceThePlayerInSomeStatesC(uint8_t slot);
SideEnd FaceThePlayerInSomeStatesD(uint8_t slot);
Ended ThrowByNextKindB(uint8_t slot);
RandomEnd PickTimersAtRandomB(uint8_t slot);
RandomEnd PickAngleAtRandomB(uint8_t slot);
Ended WalkWhileFrameChangesD(uint8_t slot, uint8_t index);
/* object/enemies_stage04_temp.c */
Ended ClampSpeedXToTwo(uint8_t slot);
Ended ThrowUpward(uint8_t slot);
Ended CountdownThenState4(uint8_t slot);
Ended ChangeToType9B(uint8_t slot);
Ended EnterState6IfFlagged(uint8_t slot);
Ended CountdownStopThenState4(uint8_t slot);
Ended RightOfObject4DWithin(uint8_t slot, uint8_t limit);
Ended LoadRecordStartAngle40(uint8_t slot);
Ended LoadRecordTimer50(uint8_t slot);
FrameStepEnd StepFramesFacingPlayer(uint8_t slot, uint8_t index);
FrameStepEnd StepFramesFacingPlayerAlways(uint8_t slot, uint8_t index);
Ended WalkWhileFrameChanges(uint8_t slot);
Ended WaitThenState2(uint8_t slot);
Ended BecomeType9CWhenAnimationEnds(uint8_t slot);
Ended LoadRecordFacingAndRandomFrame(uint8_t slot);
Ended PickSpeedAndTurnFromTableB(uint8_t slot);
Ended SpawnSixPiecesA(uint8_t slot);
Ended OrbitStepBack(uint8_t slot);
Ended OrbitStepForward(uint8_t slot);
Ended StartTypeAndLaunchUp(uint8_t slot, uint8_t fresh);
Ended AnimateThenFireOrWait(uint8_t slot);
Ended BurstIntoSixPieces(uint8_t slot);
ScreenSoundEnd SoundEveryFourthFrameB(uint8_t slot, uint8_t entry_y);
ScreenSoundEnd BurstWhenFlaggedHit(uint8_t slot, uint8_t entry_y);
Ended PickStateByPlayerDistance(uint8_t slot, bool carry_in);
Ended AnimateMoveThenSetUpNextStep(uint8_t slot, bool carry_in, uint8_t entry_y);
Ended ChargeWhenPlayerNear(uint8_t slot);
Ended TurnOnRandomBeatWalkA(uint8_t slot);
Ended TurnOnRandomBeatWalkFollowA(uint8_t slot);
Ended FallAnimatedThenBecomeKind9B(uint8_t slot);
Ended StartLobTowardPlayer(uint8_t slot, uint8_t entry_y);
Ended StepLobArc(uint8_t slot, uint8_t entry_y);
/* object/enemies_stage05_temp.c */
Ended SelectRasterStage1(void);
Ended SetHitboxDxFromFrame(uint8_t slot);
Ended PhaseTimerToState1(uint8_t slot);
Ended SetHitboxFromFrameRows(uint8_t slot);
Ended EnterState7OnFlagOrYSignChange(uint8_t slot);
Ended SetHitboxFromFrameRowsIndirect(uint8_t slot);
Ended LaunchHopByF3510(uint8_t slot);
Ended CountdownStopThenState3(uint8_t slot);
Ended StartWithFrame1A(uint8_t slot);
Ended ThrowByNextKind(uint8_t slot);
Ended AnimateThenState1(uint8_t slot);
Ended AnimateThenNextStep(uint8_t slot);
Ended FollowParentYAndLeave(uint8_t slot);
Ended AnimateThenNextStepB(uint8_t slot);
Ended CountdownThenFallOrBecomeType9B(uint8_t slot);
RandomEnd PickTimersAtRandom(uint8_t slot);
RandomEnd PickAngleAtRandom(uint8_t slot);
Ended AnimateMoveAndSag(uint8_t slot);
FrameStepEnd StepFramesFacingPlayerC(uint8_t slot, uint8_t index);
SideEnd FaceThePlayerInSomeStates(uint8_t slot);
Ended NextPhaseAtRandom(uint8_t slot);
Ended ShowOnScreenAndWrapX(uint8_t slot);
Ended StartChargeWhenPlayerNear(uint8_t slot);
Ended WaitThenState2B(uint8_t slot);
Ended ChooseNextMoveWhenFacingAway(uint8_t slot);
Ended CountTimerPairDownOrRepick(uint8_t slot);
Ended CountAnglePairDownOrRepick(uint8_t slot);
Ended AnimateHitboxThenExpire(uint8_t slot);
Ended WalkWhileFrameChangesB(uint8_t slot, uint8_t index);
Ended StartThenBlinkAndExpire(uint8_t slot);
Ended LeapAwayFromPlayerTable(uint8_t slot);
Ended PickStateByPlayerDistanceB(uint8_t slot, bool carry_in);
Ended PickSpeedAndTurnFromTableC(uint8_t slot);
Ended TurnOnRandomBeatWalkB(uint8_t slot);
Ended TurnOnRandomBeatWalkFollowB(uint8_t slot);
Ended SpawnShotFromFrame(uint8_t slot);
Ended StartAndMakeNeighbour(uint8_t slot);
Ended StartWithFrameAndFace(uint8_t slot);
Ended StartThenAnimateAndExpire(uint8_t slot);
SoundEnd CountdownSoundThenNextState(uint8_t slot, uint8_t entry_y);
SoundEnd SoundEveryFourthFrame(uint8_t slot, uint8_t entry_y);
SoundEnd CountdownSoundThenFollowParent(uint8_t slot, uint8_t entry_y);
Ended AnimateThenNumberGroupAndLeave(uint8_t slot);
Ended FollowPreviousPieceB(uint8_t slot);
SoundEnd AnimateMoveThenSetUpAtRandom(uint8_t slot);
Ended StartSidewaysAndSpawn29(uint8_t slot);
Ended StartSidewaysAndSpawn29B(uint8_t slot);
Ended BurstIntoEightPieces(uint8_t slot);
RandomEnd StartGroupThenThrow(uint8_t slot);
Ended ContactWithPlayerBox14By20(uint8_t slot);
/* object/enemies_stage07_temp.c */
Ended EnterState4IfFlagged(uint8_t slot);
Ended ScrollNotAtRightEnd(void);
Ended SetHitboxSizeFromFrame(uint8_t slot);
Ended ChangeToType9C(uint8_t slot);
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
Ended EnterState6IfFlaggedB(uint8_t slot);
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
FrameStepEnd StepFramesFacingPlayerE(uint8_t slot, uint8_t index);
Ended WalkWhileFrameChangesC(uint8_t slot);
Ended AnimateAndSpawnThenPause(uint8_t slot);
Ended StartLobTowardPlayerB(uint8_t slot, uint8_t entry_y);
Ended StepLobArcB(uint8_t slot, uint8_t entry_y);
/* object/enemies_stage0b_temp.c */
Ended CountdownThenState4B(uint8_t slot);
Ended ChangeToType9BB(uint8_t slot);
Ended SixteenBitTimerToState8(uint8_t slot);
Ended NoOpStateA(void);
Ended SetXSpeedFromFacing(uint8_t slot);
Ended EnterState6WhenHitHard(uint8_t slot);
Ended EnterState7IfYHighOrFlagged(uint8_t slot);
Ended NoOpStateB(void);
Ended LaunchHopByFacing(uint8_t slot);
Ended CountdownStopThenState2(uint8_t slot);
FrameStepEnd StepFramesFacingPlayerD(uint8_t slot, uint8_t index);
Ended WaitThenState2C(uint8_t slot);
Ended BecomeType9CWhenAnimationEndsB(uint8_t slot);
Ended LoadRecordStartAngle40B(uint8_t slot);
Ended FrameOneBWhenFallingElseAnimate(uint8_t slot);
SideEnd FaceThePlayerInSomeStatesB(uint8_t slot);
Ended AnimateAlongListC3C2(uint8_t slot);
Ended SwingOnCosineB(uint8_t slot);
Ended PickSpeedAndTurnFromTable(uint8_t slot);
Ended TurnOnRandomBeatWalkC(uint8_t slot);
Ended TurnOnRandomBeatWalkFollowC(uint8_t slot);
Ended FlashFallAnimateThenRemove(uint8_t slot);
Ended StartAndMakeNeighbourB(uint8_t slot);
Ended EnterNextStateWhenHitOrPlayerNear(uint8_t slot);
Ended PlaceAtRandomOnEdge(uint8_t slot);
Ended StartFlyerAtRandom(uint8_t slot);
Ended SwoopOnCosineC(uint8_t slot);
Ended SpawnSixPiecesB(uint8_t slot);
SoundEnd BurstWhenFlaggedHitB(uint8_t slot, uint8_t entry_y);
SoundEnd SoundEveryFourthFrameC(uint8_t slot, uint8_t entry_y);
Ended StartFromScreenEdgeByPlayer(uint8_t slot);
Ended PickStateByPlayerDistanceC(uint8_t slot, bool carry_in);
Ended SpawnType2AThenRestartTimer(uint8_t slot);
Ended StartPieceInSlotsFrom28(uint8_t slot);
Ended KnockedUpWhenHit(uint8_t slot);
Ended KnockedUpWhenHitB(uint8_t slot);
Ended SetHitboxFromFrameRowsC(uint8_t slot);
Ended FollowPreviousPiece(uint8_t slot);
SoundEnd AnimateMoveThenSetUpNextStepB(uint8_t slot, bool carry_in);
Ended LoadObjectRecordFromTable(uint8_t slot);
SoundEnd LaunchWithSoundThenDrift(uint8_t slot, uint8_t entry_y);
Ended SwingOnCosineThenLeapAtPlayer(uint8_t slot, uint8_t entry_y);
Ended StartAtRandomSpeedAndHeight(uint8_t slot);
Ended ChargeWhenPlayerNearB(uint8_t slot);
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
