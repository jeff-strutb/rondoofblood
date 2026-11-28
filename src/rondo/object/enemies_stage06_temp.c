/*
 * TEMPORARY: the stage 06E1 enemies and their helpers (disc sectors 0729 to 072D), by routine until
 * each enemy kind is identified. Bodies return what they ended on (ObjArm, enemy_objects.h); each
 * routine's exit shim builds A, Y and the flags from it, as in 441de03, and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "tile_piece_b.h"

/* An accumulated exit: the later effect over the earlier. */
static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* A sum or difference as the last arithmetic left A, N, Z, C and V. */
static Ended of_sum(Sum sum)
{
    return ended_of_sum(sum);
}

/* The arms the routines below end on. */
enum { WD_OTHER, WD_FF_STAY, WD_FF_WRAP, WD_ONE_STAY, WD_ONE_WRAP };
enum { DB_MOVE, DB_START };
enum { DP_WAITING, DP_NO_SLOT, DP_DROPPED };
enum { CW_RISING, CW_FAR_X, CW_WIDE, CW_FAR_Y, CW_BELOW, CW_ABOVE, CW_EDGE, CW_ON_TOP, CW_LEFT, CW_RIGHT_WIDE, CW_RIGHT_NARROW };
enum { SO_UNTOUCHED, SO_TOUCHED };
enum { FP2_PUSHED, FP2_LOWERED };
enum { SX_ODD, SX_SHIFTED };
enum { AS_COUNTING, AS_NEXT };
enum { TC_RUNNING, TC_ENDED };
enum { SB_STRAIGHT, SB_REVERSED };
enum { PE_ENDED, PE_TIMER, PE_ANIM, PE_FRAME, PE_COUNT, PE_NO_SLOT, PE_SHOT };
enum { OF_LATER, OF_RECORD };
enum { DE_START, DE_FALL };
enum { FB_COPY, FB_NONE, FB_BY_HEIGHT };
enum { RF_NO_FLAG, RF_LATE, RF_RAISED };
enum { BH_HIGH, BH_NO_BURST, BH_BURST };
enum { SV_PLAIN, SV_RAISED };
enum { DR_STRAIGHT, DR_REVERSED };
enum { DA_FALLING, DA_BURST };
enum { FA_SHORT, FA_FALLING };
enum { BP_RECORD, BP_MILD, BP_BROKEN };
enum { SL_RECORD, SL_LIST };
enum { FV_NO_MASK, FV_HIGH, FV_LOW_Y, FV_HIGH_Y, FV_SET };
enum { TI_ENTRY, TI_FF_STAY, TI_OFF, TI_ONE_STAY, TI_DRAW };
enum { TO_RANGE, TO_CLEARED, TO_DRAWN };

/*
 * @routine 072B:153 WrapVerticalDrift
 * @run 70:B153
 * WHAT IT DOES: Keeps a drifting object's Y position between two wrap points: at high byte $FF with
 * the low byte under $80 it jumps to high byte 1; at high byte 1 with the low byte of $81 or more it
 * jumps back to $FF.
 * HOW: Compares the high byte with $FF and with 1, then the low byte, and stores the new high byte.
 * WAITS: no.
 * Entry: X object. Exit: A and flags of the last compare or load.
 */
ObjArm WrapVerticalDrift(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = WD_OTHER;
    if (r.first == 0xFF) {
        r.second = RD8(OBJ->y_lo[slot]);
        r.arm = WD_FF_STAY;
        if (r.second >= 0x80) {
            return r;
        }
        WR8(OBJ->y_hi[slot], 1);
        r.arm = WD_FF_WRAP;
        return r;
    }
    if (r.first == 1) {
        r.second = RD8(OBJ->y_lo[slot]);
        r.arm = WD_ONE_STAY;
        if (r.second < 0x81) {
            return r;
        }
        WR8(OBJ->y_hi[slot], 0xFF);
        r.arm = WD_ONE_WRAP;
        return r;
    }
    return r;
}

/* Each compare replaces the one before it; a wrap's load follows its own. */
static Ended wrap_vertical_drift_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case WD_FF_STAY:
        return ended_compare(r.second, 0x80);
    case WD_FF_WRAP:
        return then(ended_compare(r.second, 0x80), ended_load(1));
    case WD_ONE_STAY:
        return ended_compare(r.second, 0x81);
    case WD_ONE_WRAP:
        return then(ended_compare(r.second, 0x81), ended_load(0xFF));
    default:
        return ended_compare(r.first, 1);
    }
}

void abi_WrapVerticalDrift(void)
{
    abi_ended(wrap_vertical_drift_exit_shim(WrapVerticalDrift(cpu.x)));
}

/* A drifting block's move: ApplyVelocityY, then the wrap, each in its frame. */
static ObjArm move_and_wrap(uint8_t slot, uint16_t ret_move, uint16_t ret_wrap)
{
    ObjArm r = { 0 };

    jsr_push(ret_move);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    jsr_push(ret_wrap);
    r.sub = obj_sub(WrapVerticalDrift(slot));
    jsr_pop();
    return r;
}

static Ended move_and_wrap_exit_shim(ObjArm r)
{
    return then(ended_of_axis_move(r.move), wrap_vertical_drift_exit_shim(obj_arm(r.sub)));
}

/*
 * @routine 072B:14C MoveAndWrapDriftA
 * @run 70:B14C
 * WHAT IT DOES: The later frames of RisingDriftBlock: moves by the Y speed and wraps the position.
 * HOW: ApplyVelocityY, WrapVerticalDrift.
 * WAITS: no.
 * Entry: X object. Exit: the move's flags with the wrap's compare or load over them.
 */
ObjArm MoveAndWrapDriftA(uint8_t slot)
{
    return move_and_wrap(slot, 0xB14F, 0xB152);
}

void abi_MoveAndWrapDriftA(void)
{
    abi_ended(move_and_wrap_exit_shim(MoveAndWrapDriftA(cpu.x)));
}

/*
 * @routine 072B:1AA MoveAndWrapDriftB
 * @run 70:B1AA
 * WHAT IT DOES: The later frames of LevelDriftBlock: as MoveAndWrapDriftA.
 * HOW: ApplyVelocityY, WrapVerticalDrift.
 * WAITS: no.
 * Entry and exit: as MoveAndWrapDriftA.
 */
ObjArm MoveAndWrapDriftB(uint8_t slot)
{
    return move_and_wrap(slot, 0xB1AD, 0xB1B0);
}

void abi_MoveAndWrapDriftB(void)
{
    abi_ended(move_and_wrap_exit_shim(MoveAndWrapDriftB(cpu.x)));
}

/* A drifting block's start: record loaded, frame, start height from the table by next kind, speed. */
static ObjArm drift_start(uint8_t slot, uint8_t frame, uint16_t table, uint8_t vy_hi, uint16_t ret_load)
{
    ObjArm r = { 0 };
    uint8_t nz, byte;
    bool carry;

    jsr_push(ret_load);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->frame[slot], frame);
    r.first = RD8(OBJ->start_arg[slot]);
    r.second = (uint8_t)(r.first << 1);
    byte = rd8((uint16_t)(table + 1 + r.second));
    WR8(OBJ->y_hi[slot], byte);
    byte = rd8((uint16_t)(table + r.second));
    WR8(OBJ->y_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], vy_hi);
    WR8(OBJ->vy_lo[slot], 0x80);
    return r;
}

static Ended drift_start_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_load(0x80), r.second), (r.first & 0x80) != 0);
}

/*
 * @routine 072B:11B RisingDriftBlock
 * @run 70:B11B
 * @stop B144
 * WHAT IT DOES: A block that drifts up and down: from state 0 it loads its record, shows frame $12,
 * takes its start height from a table by its next kind (two bytes each, at $B144) and an upward speed
 * of $FF80; in any later state it goes on as MoveAndWrapDriftA.
 * HOW: drift_start with this block's frame, table and speed; tables read from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: started: A $80, Y the table index, C bit 7 of the next kind, N set, Z clear;
 * else as MoveAndWrapDriftA.
 */
ObjArm RisingDriftBlock(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = DB_MOVE;
        r.sub = obj_sub(MoveAndWrapDriftA(slot));
        return r;
    }
    r.arm = DB_START;
    r.sub = obj_sub(drift_start(slot, 0x12, 0xB144, 0xFF, 0xB123));
    return r;
}

static Ended drift_block_exit_shim(ObjArm r)
{
    if (r.arm == DB_MOVE) {
        return move_and_wrap_exit_shim(obj_arm(r.sub));
    }
    return drift_start_exit_shim(obj_arm(r.sub));
}

void abi_RisingDriftBlock(void)
{
    abi_ended(drift_block_exit_shim(RisingDriftBlock(cpu.x)));
}

/*
 * @routine 072B:179 LevelDriftBlock
 * @run 70:B179
 * @stop B1A2
 * WHAT IT DOES: The same drifting block with frame $13, its start from the table at $B1A2 and a
 * resting Y speed of $0080 (it drifts down).
 * HOW: drift_start; later frames go on as MoveAndWrapDriftB.
 * WAITS: no.
 * Entry: X object. Exit: as RisingDriftBlock.
 */
ObjArm LevelDriftBlock(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = DB_MOVE;
        r.sub = obj_sub(MoveAndWrapDriftB(slot));
        return r;
    }
    r.arm = DB_START;
    r.sub = obj_sub(drift_start(slot, 0x13, 0xB1A2, 0x00, 0xB181));
    return r;
}

void abi_LevelDriftBlock(void)
{
    abi_ended(drift_block_exit_shim(LevelDriftBlock(cpu.x)));
}

/* The start both falling pieces share: record loaded, speed $xx80, frame. */
static ObjArm piece_start(uint8_t slot, uint8_t vy_hi, uint8_t frame, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t nz;

    jsr_push(ret);
    r.first = LoadObjectRecord(slot, &nz, &r.flag);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], vy_hi);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->frame[slot], frame);
    r.second = frame;
    return r;
}

static Ended piece_start_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_load(r.second), 11), r.flag);
}

/*
 * @routine 072B:1C9 StartRisingPiece
 * @run 70:B1C9
 * WHAT IT DOES: Starts a piece that rises: record loaded, Y speed $FF80, frame $14.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $14, Y 11, C from the record, N and Z from $14.
 */
ObjArm StartRisingPiece(uint8_t slot)
{
    return piece_start(slot, 0xFF, 0x14, 0xB1CC);
}

void abi_StartRisingPiece(void)
{
    abi_ended(piece_start_exit_shim(StartRisingPiece(cpu.x)));
}

/*
 * @routine 072B:264 StartLevelPiece
 * @run 70:B264
 * WHAT IT DOES: Starts a piece that hangs: record loaded, Y speed $0080, frame $15.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $15, Y 11, C from the record, N and Z from $15.
 */
ObjArm StartLevelPiece(uint8_t slot)
{
    return piece_start(slot, 0x00, 0x15, 0xB267);
}

void abi_StartLevelPiece(void)
{
    abi_ended(piece_start_exit_shim(StartLevelPiece(cpu.x)));
}

/*
 * @routine 072B:208 DropPieceEvery32
 * @run 70:B208
 * WHAT IT DOES: Every 32nd frame of the wave counter, starts an object of kind $AD at this object's
 * place in the first free slot from $30.
 * HOW: Wave counter AND $1F; FindFreeSlotFrom30; StartObjectAtPlace.
 * Untraced: one byte (the later arm of the search).
 * WAITS: no.
 * Entry: X object. Exit: waiting: A the masked wave counter; no slot: as FindFreeSlotFrom30; started: as
 * StartObjectAtPlace with C and Y from the search.
 */
ObjArm DropPieceEvery32(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t wave;

    wave = RD8(ZP->wave_counter);
    r.first = (uint8_t)(wave & 0x1F);
    r.arm = DP_WAITING;
    if (r.first != 0) {
        return r;
    }
    jsr_push(0xB211);
    r.third = FindFreeSlotFrom30(&r.second, &r.flag);
    jsr_pop();
    r.arm = DP_NO_SLOT;
    if (!r.flag) {
        return r;
    }
    jsr_push(0xB218);
    r.fourth = StartObjectAtPlace(0xAD, slot, r.third);
    jsr_pop();
    r.arm = DP_DROPPED;
    return r;
}

static Ended drop_piece_every32_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == DP_WAITING) {
        return e;
    }
    e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.second), r.third), r.flag);
    if (r.arm == DP_NO_SLOT) {
        return e;
    }
    return then(e, ended_load(r.fourth));
}

void abi_DropPieceEvery32(void)
{
    abi_ended(drop_piece_every32_exit_shim(DropPieceEvery32(cpu.x)));
}

/* The contact's indices: the values each step of ContactWithPlayer read. */
enum {
    CV_CHAR, CV_WIDTH, CV_SPEED, CV_OFF0, CV_FACING, CV_OFF1, CV_H1, CV_H2,
    CV_XLO, CV_OFFF, CV_VLO, CV_XHI, CV_VHI, CV_NA, CV_NB, CV_DHI1,
    CV_W2, CV_DLO, CV_YLO, CV_YVLO, CV_YHI, CV_YVHI, CV_YA, CV_YB,
    CV_DHI2, CV_SIDES, CV_A, CV_H3, CV_BLOW, CV_BHIGH, CV_DLO2, CV_W3,
    CV_Y2LO, CV_Y2VLO, CV_PUSH, CV_H4, CV_DEPTH, CV_W4, CV_DLO3, CV_SIDES2,
    CV_PUSH2, CV_W5, CV_W6
};

/*
 * @routine 072A:235 ContactWithPlayer
 * @run 70:AA35
 * WHAT IT DOES: Tests whether the object, a block that carries or bars the player, touches the player
 * (slot 0): the width is in zTemp08 and the height in zTemp09. Says touching with C set (the player
 * stands on its top), and sets the player's push: zPlayerPush bits for the side or top touched, the
 * push depth, the way the player is pushed and PlayerAnchorObject.
 * HOW: The player's falling speed (not rising) is required; the horizontal distance is made positive
 * (the object's hitbox offset turned by its facing); the width must reach it; the vertical distance is
 * made positive and compared against the height band above and below the top; the first match decides
 * the side. The paths that ran in no script: the player a character other than 0 (width less 4, A$AA3E),
 * the push depth when the width is 3 or more on the left side (AAF8) and on the right (AB0A).
 * Curated cases: tools/rondo/curated/contact_with_player.txt (traced arms; 737 of 1240 cases checked, none differ).
 * WAITS: no.
 * Entry: X object, zTemp08 width, zTemp09 height. Exit: C set when standing on it; A and N, Z as the last
 * load or compare left them; V as the last add or subtract.
 */
ContactArm ContactWithPlayer(uint8_t slot)
{
    ContactArm a = { 0 };
    uint8_t *v = a.v;
    Sum x_sum, y_sum, sum;

    jsr_push(0xAA38);
    a.side = SideOfPlayer(slot);
    jsr_pop();
    WR8(ZP->temp0e, a.side.sides);
    v[CV_CHAR] = RD8(ZP->character);
    if (v[CV_CHAR] != 0) {
        v[CV_WIDTH] = RD8(ZP->temp08);
        sum = sub_bytes(v[CV_WIDTH], 4, true);
        WR8(ZP->temp08, sum.value);
    }
    v[CV_SPEED] = RD8(OBJ->vy_hi[0]);
    a.arm = CW_RISING;
    if ((v[CV_SPEED] & 0x80) != 0) {
        return a;
    }
    v[CV_OFF0] = RD8(OBJ->hitbox_dx[slot]);
    WR8(ZP->temp0d, v[CV_OFF0]);
    v[CV_FACING] = RD8(OBJ->facing_left[slot]);
    if (v[CV_FACING] != 0) {
        v[CV_OFF1] = RD8(ZP->temp0d);
        WR8(ZP->temp0d, (uint8_t)((uint8_t)(v[CV_OFF1] ^ 0xFF) + 1));
    }
    v[CV_H1] = RD8(ZP->temp09);
    sum = add_bytes(v[CV_H1], 5, false);
    WR8(ZP->temp0a, sum.value);
    v[CV_H2] = RD8(ZP->temp09);
    sum = sub_bytes(v[CV_H2], 4, true);
    WR8(ZP->temp0b, sum.value);
    v[CV_XLO] = RD8(OBJ->x_lo[slot]);
    v[CV_OFFF] = RD8(ZP->temp0d);
    v[CV_VLO] = RD8(OBJ->x_lo[0]);
    x_sum = sub_bytes(add_bytes(v[CV_XLO], v[CV_OFFF], false).value, v[CV_VLO], true);
    WR8(ZP->temp00, x_sum.value);
    v[CV_XHI] = RD8(OBJ->x_hi[slot]);
    v[CV_VHI] = RD8(OBJ->x_hi[0]);
    y_sum = sub_bytes(v[CV_XHI], v[CV_VHI], x_sum.carry);
    WR8(ZP->temp01, y_sum.value);
    if ((y_sum.value & 0x80) != 0) {
        v[CV_NA] = RD8(ZP->temp00);
        sum = add_bytes((uint8_t)(v[CV_NA] ^ 0xFF), 1, false);
        WR8(ZP->temp00, sum.value);
        v[CV_NB] = RD8(ZP->temp01);
        sum = add_bytes((uint8_t)(v[CV_NB] ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp01, sum.value);
    }
    v[CV_DHI1] = RD8(ZP->temp01);
    a.arm = CW_FAR_X;
    if (v[CV_DHI1] != 0) {
        return a;
    }
    v[CV_W2] = RD8(ZP->temp08);
    v[CV_DLO] = RD8(ZP->temp00);
    a.arm = CW_WIDE;
    if (v[CV_W2] < v[CV_DLO]) {
        return a;
    }
    v[CV_YLO] = RD8(OBJ->y_lo[slot]);
    v[CV_YVLO] = RD8(OBJ->y_lo[0]);
    x_sum = sub_bytes(v[CV_YLO], v[CV_YVLO], true);
    WR8(ZP->temp02, x_sum.value);
    v[CV_YHI] = RD8(OBJ->y_hi[slot]);
    v[CV_YVHI] = RD8(OBJ->y_hi[0]);
    y_sum = sub_bytes(v[CV_YHI], v[CV_YVHI], x_sum.carry);
    WR8(ZP->temp01, y_sum.value);
    if ((y_sum.value & 0x80) != 0) {
        v[CV_YA] = RD8(ZP->temp02);
        sum = add_bytes((uint8_t)(v[CV_YA] ^ 0xFF), 1, false);
        WR8(ZP->temp02, sum.value);
        v[CV_YB] = RD8(ZP->temp01);
        sum = add_bytes((uint8_t)(v[CV_YB] ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp01, sum.value);
    }
    v[CV_DHI2] = RD8(ZP->temp01);
    a.arm = CW_FAR_Y;
    if (v[CV_DHI2] != 0) {
        return a;
    }
    v[CV_SIDES] = RD8(ZP->temp0e);
    if ((v[CV_SIDES] & 0x02) != 0) {
        v[CV_A] = RD8(ZP->temp02);
        v[CV_H3] = RD8(ZP->temp09);
        a.arm = CW_BELOW;
        if (v[CV_A] >= v[CV_H3]) {
            return a;
        }
    } else {
        v[CV_A] = RD8(ZP->temp02);
        v[CV_BLOW] = RD8(ZP->temp0b);
        if (v[CV_A] >= v[CV_BLOW]) {
            v[CV_BHIGH] = RD8(ZP->temp0a);
            a.arm = CW_ABOVE;
            if (v[CV_A] >= v[CV_BHIGH]) {
                return a;
            }
            v[CV_DLO2] = RD8(ZP->temp00);
            v[CV_W3] = RD8(ZP->temp08);
            a.arm = CW_EDGE;
            if (v[CV_DLO2] == v[CV_W3]) {
                return a;
            }
            wr8(PlayerAnchorObject, slot);
            v[CV_Y2LO] = RD8(OBJ->y_lo[slot]);
            v[CV_Y2VLO] = RD8(OBJ->y_lo[0]);
            x_sum = sub_bytes(v[CV_Y2LO], v[CV_Y2VLO], true);
            WR8(ZP->temp02, x_sum.value);
            v[CV_PUSH] = RD8(ZP->player_push);
            WR8(ZP->player_push, (uint8_t)(v[CV_PUSH] | 0x01));
            v[CV_H4] = RD8(ZP->temp09);
            v[CV_DEPTH] = RD8(ZP->temp02);
            sum = sub_bytes(v[CV_H4], v[CV_DEPTH], true);
            WR8(ZP->field_ca, sum.value);
            a.contact = true;
            a.arm = CW_ON_TOP;
            return a;
        }
    }
    v[CV_W4] = RD8(ZP->temp08);
    v[CV_DLO3] = RD8(ZP->temp00);
    sum = sub_bytes(v[CV_W4], v[CV_DLO3], true);
    WR8(ZP->temp08, sum.value);
    v[CV_SIDES2] = RD8(ZP->temp0e);
    if ((v[CV_SIDES2] & 0x01) == 0) {
        v[CV_PUSH2] = RD8(ZP->player_push);
        WR8(ZP->player_push, (uint8_t)(v[CV_PUSH2] | 0x02));
        v[CV_W5] = RD8(ZP->temp08);
        a.arm = CW_LEFT;
        if (v[CV_W5] >= 3) {
            WR8(ZP->player_push_depth, 2);
        } else {
            WR8(ZP->player_push_depth, v[CV_W5]);
        }
        return a;
    }
    v[CV_PUSH2] = RD8(ZP->player_push);
    WR8(ZP->player_push, (uint8_t)(v[CV_PUSH2] | 0x04));
    v[CV_W5] = RD8(ZP->temp08);
    a.arm = CW_RIGHT_WIDE;
    if (v[CV_W5] >= 3) {
        WR8(ZP->player_push_depth, 5);
        return a;
    }
    v[CV_W6] = RD8(ZP->temp08);
    sum = sub_bytes(7, v[CV_W6], true);
    WR8(ZP->player_push_depth, sum.value);
    a.arm = CW_RIGHT_NARROW;
    return a;
}

/* The negation both distances make when the high byte came out negative: EOR #$FF, ADC #1, then the high byte's EOR and ADC #0. */
static Ended contact_negate(Ended e, uint8_t low, uint8_t high)
{
    Sum sum = add_bytes((uint8_t)(low ^ 0xFF), 1, false);

    e = then(e, of_sum(sum));
    return then(e, of_sum(add_bytes((uint8_t)(high ^ 0xFF), 0, sum.carry)));
}

/* What the routine left, and whether the player stands on it (C). */
Ended contact_exit_shim(ContactArm a)
{
    const uint8_t *v = a.v;
    Ended e = ended_of_side(a.side);
    Sum x_sum, y_sum, sum;
    uint8_t offset;

    e = then(e, ended_load(v[CV_CHAR]));
    if (v[CV_CHAR] != 0) {
        e = then(e, of_sum(sub_bytes(v[CV_WIDTH], 4, true)));
    }
    e = then(e, ended_load(v[CV_SPEED]));
    if (a.arm == CW_RISING) {
        return ended_with_carry(e, false);
    }
    offset = v[CV_OFF0];
    e = then(e, ended_load(offset));
    e = then(e, ended_load(v[CV_FACING]));
    if (v[CV_FACING] != 0) {
        offset = v[CV_OFF1];
        e = then(e, ended_load(offset));
        offset = (uint8_t)(offset ^ 0xFF);
        e = then(e, ended_load(offset));
        e = then(e, ended_load((uint8_t)(offset + 1)));
    }
    e = then(e, of_sum(add_bytes(v[CV_H1], 5, false)));
    e = then(e, of_sum(sub_bytes(v[CV_H2], 4, true)));
    x_sum = sub_bytes(add_bytes(v[CV_XLO], v[CV_OFFF], false).value, v[CV_VLO], true);
    e = then(e, of_sum(x_sum));
    y_sum = sub_bytes(v[CV_XHI], v[CV_VHI], x_sum.carry);
    e = then(e, of_sum(y_sum));
    if ((y_sum.value & 0x80) != 0) {
        e = contact_negate(e, v[CV_NA], v[CV_NB]);
    }
    e = then(e, ended_load(v[CV_DHI1]));
    if (a.arm == CW_FAR_X) {
        return ended_with_carry(e, false);
    }
    e = then(e, ended_compare(v[CV_W2], v[CV_DLO]));
    if (a.arm == CW_WIDE) {
        return ended_with_carry(e, false);
    }
    x_sum = sub_bytes(v[CV_YLO], v[CV_YVLO], true);
    e = then(e, of_sum(x_sum));
    y_sum = sub_bytes(v[CV_YHI], v[CV_YVHI], x_sum.carry);
    e = then(e, of_sum(y_sum));
    if ((y_sum.value & 0x80) != 0) {
        e = contact_negate(e, v[CV_YA], v[CV_YB]);
    }
    e = then(e, ended_load(v[CV_DHI2]));
    if (a.arm == CW_FAR_Y) {
        return ended_with_carry(e, false);
    }
    e = then(e, ended_load((uint8_t)(v[CV_SIDES] & 0x02)));
    if ((v[CV_SIDES] & 0x02) != 0) {
        e = then(e, ended_compare(v[CV_A], v[CV_H3]));
        if (a.arm == CW_BELOW) {
            return ended_with_carry(e, false);
        }
    } else {
        e = then(e, ended_compare(v[CV_A], v[CV_BLOW]));
        if (v[CV_A] >= v[CV_BLOW]) {
            e = then(e, ended_compare(v[CV_A], v[CV_BHIGH]));
            if (a.arm == CW_ABOVE) {
                return ended_with_carry(e, false);
            }
            e = then(e, ended_compare(v[CV_DLO2], v[CV_W3]));
            if (a.arm == CW_EDGE) {
                return ended_with_carry(e, false);
            }
            e = then(e, of_sum(sub_bytes(v[CV_Y2LO], v[CV_Y2VLO], true)));
            e = then(e, ended_load((uint8_t)(v[CV_PUSH] | 0x01)));
            e = then(e, of_sum(sub_bytes(v[CV_H4], v[CV_DEPTH], true)));
            return ended_with_carry(e, true);
        }
    }
    sum = sub_bytes(v[CV_W4], v[CV_DLO3], true);
    e = then(e, of_sum(sum));
    e = then(e, ended_load((uint8_t)(v[CV_SIDES2] & 0x01)));
    if ((v[CV_SIDES2] & 0x01) == 0) {
        e = then(e, ended_load((uint8_t)(v[CV_PUSH2] | 0x02)));
        e = then(e, ended_compare(v[CV_W5], 3));
        if (v[CV_W5] >= 3) {
            e = then(e, ended_load(2));
        }
        return ended_with_carry(e, false);
    }
    e = then(e, ended_load((uint8_t)(v[CV_PUSH2] | 0x04)));
    e = then(e, ended_compare(v[CV_W5], 3));
    if (a.arm == CW_RIGHT_WIDE) {
        return ended_with_carry(then(e, ended_load(5)), false);
    }
    return ended_with_carry(then(e, of_sum(sub_bytes(7, v[CV_W6], true))), false);
}

void abi_ContactWithPlayer(void)
{
    abi_ended(contact_exit_shim(ContactWithPlayer(cpu.x)));
}

/* The test every piece makes: width $14 and height $1D set, the contact asked. */
static ContactArm piece_contact(uint8_t slot, uint16_t ret)
{
    ContactArm contact;

    WR8(ZP->temp08, 0x14);
    WR8(ZP->temp09, 0x1D);
    jsr_push(ret);
    contact = ContactWithPlayer(slot);
    jsr_pop();
    return contact;
}

/* The frames of the piece's list at $B203, stepped. */
static ObjArm piece_frames(uint8_t slot, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    WR8(ZP->temp10, 0x03);
    WR8(ZP->temp11, 0xB2);
    jsr_push(ret);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    return r;
}

/* A piece on contact pushes the player along with it. */
static ObjArm piece_push(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t push, speed_hi;

    push = RD8(ZP->player_push);
    push = (uint8_t)(push | 0x21);
    WR8(ZP->player_push, push);
    speed_hi = RD8(OBJ->vy_hi[slot]);
    WR8(ZP->push_y.hi, speed_hi);
    r.first = RD8(OBJ->vy_lo[slot]);
    WR8(ZP->push_y.lo, r.first);
    return r;
}

static Ended piece_push_exit_shim(ObjArm r)
{
    return ended_load(r.first);
}

/*
 * @routine 072B:1DC StepPieceOnParent
 * @run 70:B1DC
 * WHAT IT DOES: A piece that rides the object 8 slots before it: steps its animation (list $B203), drops
 * a piece every 32 frames, takes the parent's place and, when the player touches it (width $14, height
 * $1D), moves on one state.
 * HOW: StepObjectFrames, DropPieceEvery32, CopyObjectPlaceBack from slot less 8, ContactWithPlayer.
 * WAITS: no.
 * Entry: X object. Exit: see the wrapper; the contact's registers, then N and Z of the state when touched.
 */
ContactStep StepPieceOnParent(uint8_t slot)
{
    ContactStep c = { { 0 } };
    ObjArm *r = &c.arm;

    r->sub = obj_sub(piece_frames(slot, 0xB1E7));
    jsr_push(0xB1EA);
    r->sub2 = obj_sub(DropPieceEvery32(slot));
    jsr_pop();
    r->sum = sub_bytes(slot, 8, true);
    jsr_push(0xB1F2);
    r->first = CopyObjectPlaceBack(r->sum.value, slot);
    jsr_pop();
    c.contact = piece_contact(slot, 0xB1FD);
    r->arm = SO_UNTOUCHED;
    if (!c.contact.contact) {
        return c;
    }
    r->second = INC8(OBJ->state[slot]);
    r->arm = SO_TOUCHED;
    return c;
}

static Ended step_piece_on_parent_exit_shim(ContactStep c)
{
    const ObjArm *r = &c.arm;
    Ended e = frame_step_exit_shim(obj_arm(r->sub));

    e = then(e, drop_piece_every32_exit_shim(obj_arm(r->sub2)));
    e = then(e, ended_with_y(of_sum(r->sum), r->sum.value));
    e = then(e, ended_load(r->first));
    e = then(e, contact_exit_shim(c.contact));
    if (r->arm == SO_UNTOUCHED) {
        return e;
    }
    return then(e, ended_nz(r->second));
}

void abi_StepPieceOnParent(void)
{
    abi_ended(step_piece_on_parent_exit_shim(StepPieceOnParent(cpu.x)));
}

/*
 * @routine 072B:277 FollowParentUntilTouched
 * @run 70:B277
 * WHAT IT DOES: Takes the place of the object 8 slots before it and, when the player touches it, moves on
 * one state.
 * HOW: CopyObjectPlaceBack from slot less 8, ContactWithPlayer (width $14, height $1D).
 * WAITS: no.
 * Entry: X object. Exit: as StepPieceOnParent.
 */
ContactStep FollowParentUntilTouched(uint8_t slot)
{
    ContactStep c = { { 0 } };
    ObjArm *r = &c.arm;

    r->sum = sub_bytes(slot, 8, true);
    jsr_push(0xB27F);
    r->first = CopyObjectPlaceBack(r->sum.value, slot);
    jsr_pop();
    c.contact = piece_contact(slot, 0xB28A);
    r->arm = SO_UNTOUCHED;
    if (!c.contact.contact) {
        return c;
    }
    r->second = INC8(OBJ->state[slot]);
    r->arm = SO_TOUCHED;
    return c;
}

static Ended follow_parent_until_touched_exit_shim(ContactStep c)
{
    const ObjArm *r = &c.arm;
    Ended e = ended_with_y(of_sum(r->sum), r->sum.value);

    e = then(e, ended_load(r->first));
    e = then(e, contact_exit_shim(c.contact));
    if (r->arm == SO_UNTOUCHED) {
        return e;
    }
    return then(e, ended_nz(r->second));
}

void abi_FollowParentUntilTouched(void)
{
    abi_ended(follow_parent_until_touched_exit_shim(FollowParentUntilTouched(cpu.x)));
}

/*
 * @routine 072B:290 FallingPieceMove
 * @run 70:B290
 * WHAT IT DOES: Moves a piece by its Y speed; if the player stands on it (width $14, height $1D) the
 * player is pushed along by its speed, else the piece's state goes back one.
 * HOW: ApplyVelocityY, ContactWithPlayer, DEC of the state or the push.
 * WAITS: no.
 * Entry: X object. Exit: the contact's registers, then the push's last load or the state's N and Z.
 */
ContactStep FallingPieceMove(uint8_t slot)
{
    ContactStep c = { { 0 } };
    ObjArm *r = &c.arm;

    jsr_push(0xB293);
    r->move = ApplyVelocityY(slot);
    jsr_pop();
    c.contact = piece_contact(slot, 0xB29E);
    r->arm = FP2_PUSHED;
    if (!c.contact.contact) {
        r->first = DEC8(OBJ->state[slot]);
        r->arm = FP2_LOWERED;
        return c;
    }
    r->sub = obj_sub(piece_push(slot));
    return c;
}

static Ended falling_piece_move_exit_shim(ContactStep c)
{
    const ObjArm *r = &c.arm;
    Ended e = then(ended_of_axis_move(r->move), contact_exit_shim(c.contact));

    if (r->arm == FP2_LOWERED) {
        return then(e, ended_nz(r->first));
    }
    return then(e, piece_push_exit_shim(obj_arm(r->sub)));
}

void abi_FallingPieceMove(void)
{
    abi_ended(falling_piece_move_exit_shim(FallingPieceMove(cpu.x)));
}

/*
 * @routine 072B:219 FallingPieceStep
 * @run 70:B219
 * WHAT IT DOES: FallingPieceMove with the animation (list $B203) stepped first and a piece dropped every 32
 * frames.
 * HOW: StepObjectFrames, DropPieceEvery32, then as FallingPieceMove.
 * WAITS: no.
 * Entry: X object. Exit: as FallingPieceMove.
 */
ContactStep FallingPieceStep(uint8_t slot)
{
    ContactStep c = { { 0 } };
    ObjArm *r = &c.arm;

    r->sub = obj_sub(piece_frames(slot, 0xB224));
    jsr_push(0xB227);
    r->sub2 = obj_sub(DropPieceEvery32(slot));
    jsr_pop();
    jsr_push(0xB22A);
    r->move = ApplyVelocityY(slot);
    jsr_pop();
    c.contact = piece_contact(slot, 0xB235);
    r->arm = FP2_PUSHED;
    if (!c.contact.contact) {
        r->first = DEC8(OBJ->state[slot]);
        r->arm = FP2_LOWERED;
        return c;
    }
    r->sub3 = obj_sub(piece_push(slot));
    return c;
}

static Ended falling_piece_step_exit_shim(ContactStep c)
{
    const ObjArm *r = &c.arm;
    Ended e = frame_step_exit_shim(obj_arm(r->sub));

    e = then(e, drop_piece_every32_exit_shim(obj_arm(r->sub2)));
    e = then(e, ended_of_axis_move(r->move));
    e = then(e, contact_exit_shim(c.contact));
    if (r->arm == FP2_LOWERED) {
        return then(e, ended_nz(r->first));
    }
    return then(e, piece_push_exit_shim(obj_arm(r->sub3)));
}

void abi_FallingPieceStep(void)
{
    abi_ended(falling_piece_step_exit_shim(FallingPieceStep(cpu.x)));
}

/*
 * @routine 072C:1BE HitboxByVerticalDirection
 * @run 70:B9BE
 * WHAT IT DOES: Sets the object's hitbox flag from a two-byte table at $B9CD by whether it is moving up
 * (its Y speed's top bit): index 1 when rising, 0 otherwise.
 * HOW: Two ROLs take the top bit to bit 0; the table is read from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y the index, N and Z from A, C bit 6 of the Y speed.
 */
ObjArm HitboxByVerticalDirection(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->vy_hi[slot]);
    r.second = (uint8_t)(r.first >> 7);
    r.third = rd8((uint16_t)(0xB9CD + r.second));
    WR8(OBJ->hitbox_on[slot], r.third);
    return r;
}

static Ended hitbox_by_vertical_direction_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_load(r.third), r.second), (r.first & 0x40) != 0);
}

void abi_HitboxByVerticalDirection(void)
{
    abi_ended(hitbox_by_vertical_direction_exit_shim(HitboxByVerticalDirection(cpu.x)));
}

/*
 * @routine 072C:1CF LiftObjectsBySlowFraction
 * @run 70:B9CF
 * WHAT IT DOES: For each object slot from $10 to $26, adds $E0 to this object's Y fraction and carries
 * the sum into that slot's Y position (low then high byte).
 * HOW: A loop of 23, the carry of each fraction add running through the two position adds.
 * WAITS: no.
 * Entry: X object. Exit: A the last high byte, Y $27, N 0, Z 1, C 1 from the loop's compare, V from the
 * last add.
 */
ObjArm LiftObjectsBySlowFraction(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t other, frac, low, high;
    Sum sum;

    for (other = 0x10; other < 0x27; other++) {
        frac = RD8(OBJ->y_frac[slot]);
        sum = add_bytes(frac, 0xE0, false);
        WR8(OBJ->y_frac[slot], sum.value);
        low = RD8(OBJ->y_lo[other]);
        sum = add_bytes(low, 0, sum.carry);
        WR8(OBJ->y_lo[other], sum.value);
        high = RD8(OBJ->y_hi[other]);
        r.sum = add_bytes(high, 0, sum.carry);
        WR8(OBJ->y_hi[other], r.sum.value);
    }
    return r;
}

static Ended lift_objects_by_slow_fraction_exit_shim(ObjArm r)
{
    return then(of_sum(r.sum), ended_with_carry(ended_with_y(ended_nz(0), 0x27), true));
}

void abi_LiftObjectsBySlowFraction(void)
{
    abi_ended(lift_objects_by_slow_fraction_exit_shim(LiftObjectsBySlowFraction(cpu.x)));
}

/*
 * @routine 072C:0E9 ShiftObjectsX
 * @run 70:B8E9
 * WHAT IT DOES: On every second frame of the object's life timer, moves every object slot from $10 to $26
 * along X by a signed 16-bit step from the table at $B912, picked by the object's OBJ_WORK_M (0 or 1).
 * HOW: The table index is kept in X while the loop runs (X is pushed and pulled).
 * WAITS: no.
 * Entry: X object. Exit: odd timer: A 1, N 0, Z 0; else A the last high byte, Y $27, C 1, N and Z from X, V
 * from the last add.
 */
ObjArm ShiftObjectsX(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t timer, side, index, other, low, high;
    Sum sum;

    timer = RD8(OBJ->life_timer[slot]);
    r.first = (uint8_t)(timer & 0x01);
    r.arm = SX_ODD;
    if (r.first != 0) {
        return r;
    }
    side = RD8(OBJ->work_m[slot]);
    index = (uint8_t)(side << 1);
    push8(slot);
    for (other = 0x10; other < 0x27; other++) {
        low = RD8(OBJ->x_lo[other]);
        sum = add_bytes(low, rd8((uint16_t)(0xB912 + index)), false);
        WR8(OBJ->x_lo[other], sum.value);
        high = RD8(OBJ->x_hi[other]);
        r.sum = add_bytes(high, rd8((uint16_t)(0xB913 + index)), sum.carry);
        WR8(OBJ->x_hi[other], r.sum.value);
    }
    r.second = pull8();
    r.arm = SX_SHIFTED;
    return r;
}

static Ended shift_objects_x_exit_shim(ObjArm r)
{
    if (r.arm == SX_ODD) {
        return ended_load(r.first);
    }
    return then(of_sum(r.sum), ended_with_carry(ended_with_y(ended_nz(r.second), 0x27), true));
}

void abi_ShiftObjectsX(void)
{
    abi_ended(shift_objects_x_exit_shim(ShiftObjectsX(cpu.x)));
}

/* The slot's X speed word with a signed table word added (low byte first). */
static Sum speed_x_add(uint8_t slot, uint16_t table, uint8_t index)
{
    uint8_t low, high;
    Sum sum;

    low = RD8(OBJ->vx_lo[slot]);
    sum = add_bytes(low, rd8((uint16_t)(table + index)), false);
    WR8(OBJ->vx_lo[slot], sum.value);
    high = RD8(OBJ->vx_hi[slot]);
    sum = add_bytes(high, rd8((uint16_t)(table + 1 + index)), sum.carry);
    WR8(OBJ->vx_hi[slot], sum.value);
    return sum;
}

/*
 * @routine 072C:09C OrbitingObject
 * @run 70:B89C
 * WHAT IT DOES: An object that circles: it shifts the others (ShiftObjectsX), sets its speed from its angle
 * (OBJ_WORK_L) with a length of $0400, adds a drift word from the table at $B8E3 by its side (OBJ_WORK_M),
 * moves, turns its angle on by a byte from the table at $B8E7, and when its X high byte reaches $FE or 2
 * it clears its OBJ_SUBSTATE flag.
 * HOW: SetVelocityFromAngle, ApplyVelocity, tables from the disc (bank 70).
 * Untraced: one byte (the second compare's exit).
 * WAITS: no.
 * Entry: X object. Exit: A the X high byte, N, Z and C from the last compare, Y the side, V from the angle's add.
 */
ObjArm OrbitingObject(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t angle, side, index, step;

    jsr_push(0xB89F);
    ShiftObjectsX(slot);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    angle = RD8(OBJ->work_l[slot]);
    jsr_push(0xB8AB);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    side = RD8(OBJ->work_m[slot]);
    index = (uint8_t)(side << 1);
    speed_x_add(slot, 0xB8E3, index);
    jsr_push(0xB8C6);
    ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->work_m[slot]);
    angle = RD8(OBJ->work_l[slot]);
    step = rd8((uint16_t)(0xB8E7 + r.first));
    r.sum = add_bytes(angle, step, false);
    WR8(OBJ->work_l[slot], r.sum.value);
    r.second = RD8(OBJ->x_hi[slot]);
    if (r.second == 0xFE || r.second == 0x02) {
        WR8(OBJ->substate[slot], 0);
    }
    return r;
}

/* The two compares: the second only when the high byte was not $FE. */
static Ended orbiting_object_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(of_sum(r.sum), r.first);

    e = then(e, ended_compare(r.second, 0xFE));
    if (r.second != 0xFE) {
        e = then(e, ended_compare(r.second, 0x02));
    }
    return e;
}

void abi_OrbitingObject(void)
{
    abi_ended(orbiting_object_exit_shim(OrbitingObject(cpu.x)));
}

/*
 * @routine 072C:242 AccelerateAndSway
 * @run 70:BA42
 * WHAT IT DOES: A thing that falls and sways: its Y speed gains $002C, it moves, its X speed gains a drift
 * word from the table at $BA7C by its side (OBJ_WORK_M); when its counter OBJ_WORK_J runs out it takes
 * OBJ_WORK_I 3 and moves on one OBJ_SUBSTATE state.
 * HOW: Adds, ApplyVelocity, DEC of the counter.
 * WAITS: no.
 * Entry: X object. Exit: counter not zero: N and Z from it, C and V from the X speed's add; zero: A 3, N and Z
 * from the new state.
 */
ObjArm AccelerateAndSway(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t low, high, side;
    Sum sum;

    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 0x2C, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    jsr_push(0xBA56);
    ApplyVelocity(slot);
    jsr_pop();
    side = RD8(OBJ->work_m[slot]);
    r.first = (uint8_t)(side << 1);
    r.sum = speed_x_add(slot, 0xBA7C, r.first);
    r.second = DEC8(OBJ->work_j[slot]);
    r.arm = AS_COUNTING;
    if (r.second != 0) {
        return r;
    }
    WR8(OBJ->work_i[slot], 3);
    r.third = INC8(OBJ->substate[slot]);
    r.arm = AS_NEXT;
    return r;
}

static Ended accelerate_and_sway_exit_shim(ObjArm r)
{
    Ended e = then(ended_with_y(of_sum(r.sum), r.first), ended_nz(r.second));

    if (r.arm == AS_COUNTING) {
        return e;
    }
    return then(e, ended_with_a(ended_nz(r.third), 3));
}

void abi_AccelerateAndSway(void)
{
    abi_ended(accelerate_and_sway_exit_shim(AccelerateAndSway(cpu.x)));
}

/* The frame list steps a piece makes, with its list's address in zTemp10-11; the exit as the step left it. */
static FrameStep step_list(uint8_t slot, uint8_t low, uint8_t high, uint16_t ret, uint8_t *y)
{
    FrameStep step;

    WR8(ZP->temp10, low);
    WR8(ZP->temp11, high);
    jsr_push(ret);
    step = StepObjectFrames(slot, y);
    jsr_pop();
    return step;
}

/*
 * @routine 072C:280 StepThenCount
 * @run 70:BA80
 * WHAT IT DOES: Steps the animation list at $BA96; when the list ends, sets the counter OBJ_WORK_J to $80 and
 * moves on one OBJ_SUBSTATE state.
 * HOW: StepObjectFrames, then the stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z from the new state.
 */
ObjArm StepThenCount(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0x96, 0xBA, 0xBA8B, &y);
    r.y = y;
    r.arm = TC_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->work_j[slot], 0x80);
    r.first = INC8(OBJ->substate[slot]);
    r.arm = TC_ENDED;
    return r;
}

static Ended step_then_count_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TC_RUNNING) {
        return e;
    }
    return then(e, ended_with_a(ended_nz(r.first), 0x80));
}

void abi_StepThenCount(void)
{
    abi_ended(step_then_count_exit_shim(StepThenCount(cpu.x)));
}

/*
 * @routine 072C:329 StepThenStop
 * @run 70:BB29
 * WHAT IT DOES: Steps the animation list at $BB45; when the list ends, sets OBJ_WORK_I to 4, stops the Y
 * speed and moves on one OBJ_SUBSTATE state.
 * HOW: StepObjectFrames, then the stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 4, N and Z from
 * the new state.
 */
ObjArm StepThenStop(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0x45, 0xBB, 0xBB34, &y);
    r.y = y;
    r.arm = TC_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->work_i[slot], 4);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    r.first = INC8(OBJ->substate[slot]);
    r.arm = TC_ENDED;
    return r;
}

static Ended step_then_stop_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == TC_RUNNING) {
        return e;
    }
    return then(e, ended_with_a(ended_nz(r.first), 4));
}

void abi_StepThenStop(void)
{
    abi_ended(step_then_stop_exit_shim(StepThenStop(cpu.x)));
}

/*
 * @routine 072C:34C FallUntilLevel2
 * @run 70:BB4C
 * WHAT IT DOES: Applies gravity and moves the object down; when its Y high byte is 2 its OBJ_SUBSTATE is cleared.
 * HOW: ApplyGravity, ApplyVelocityY, a compare.
 * WAITS: no.
 * Entry: X object. Exit: A the Y high byte, N, Z and C of the compare with 2, V from the move.
 */
ObjArm FallUntilLevel2(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBB4F);
    ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xBB52);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first == 2) {
        WR8(OBJ->substate[slot], 0);
    }
    return r;
}

static Ended fall_until_level2_exit_shim(ObjArm r)
{
    return then(ended_of_axis_move(r.move), ended_compare(r.first, 2));
}

void abi_FallUntilLevel2(void)
{
    abi_ended(fall_until_level2_exit_shim(FallUntilLevel2(cpu.x)));
}

/*
 * @routine 072C:6AE StopVerticalSpeed
 * @run 70:BEAE
 * WHAT IT DOES: Sets the object's Y speed to 0.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void StopVerticalSpeed(uint8_t slot)
{
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_StopVerticalSpeed(void)
{
    StopVerticalSpeed(cpu.x);
}

/*
 * @routine 072C:5AB JumpByStageStep
 * @run 70:BDAB
 * WHAT IT DOES: Goes to the code a table at $BDBD names for the stage's step counter OBJ_WORK_I slot $10.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: nothing. Exit: as the code jumped to; A the high byte, Y the doubled step.
 */
ObjArm JumpByStageStep(void)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.third = RD8(OBJ->work_i[0x10]);
    r.second = (uint8_t)(r.third << 1);
    low = rd8((uint16_t)(0xBDBD + r.second));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(0xBDBE + r.second));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_stage_step_exit_shim(ObjArm r)
{
    Jump jump = { { 0 }, r.target, true };

    jump.end = ended_with_carry(ended_with_y(ended_load(r.first), r.second), (r.third & 0x80) != 0);
    return jump;
}

void abi_JumpByStageStep(void)
{
    Jump jump = jump_by_stage_step_exit_shim(JumpByStageStep());

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072B:42E StartPieceChain
 * @run 70:B42E
 * WHAT IT DOES: Stops the object, clears four 192-byte work tables the overlay keeps at $B46E, $B52E,
 * $B5EE and $B6AE, then starts 22 objects of kind $25 in the slots after this one, each at this object's
 * place, each with its own number 0 to 21 in its life timer (the chain of pieces that follow it).
 * HOW: Clears the four speed bytes and OBJ_SPRITE_FLIP; a loop of 192 stores to each table; a loop of 22 of
 * ClearObjectInY and StartObjectAtPlace. The tables have no names yet.
 * WAITS: no.
 * Entry: X object. Exit: A 21, Y the slot plus 23, N 0, Z 1, C 1; V kept.
 */
ObjArm StartPieceChain(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, count, to;

    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->f2fe8[slot], 0);
    for (index = 0; index < 0xC0; index++) {
        wr8((uint16_t)(0xB46E + index), 0);
        wr8((uint16_t)(0xB52E + index), 0);
        wr8((uint16_t)(0xB5EE + index), 0);
        wr8((uint16_t)(0xB6AE + index), 0);
    }
    to = (uint8_t)(slot + 1);
    count = 0x16;
    WR8(ZP->temp00, count);
    WR8(ZP->temp01, 0);
    do {
        jsr_push(0xB45C);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB461);
        StartObjectAtPlace(0x25, slot, to);
        jsr_pop();
        r.first = RD8(ZP->temp01);
        WR8(OBJ->life_timer[to], r.first);
        to++;
        INC8(ZP->temp01);
        count = DEC8(ZP->temp00);
    } while (count != 0);
    r.second = to;
    return r;
}

static Ended start_piece_chain_exit_shim(ObjArm r)
{
    return ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.first), r.second), true);
}

void abi_StartPieceChain(void)
{
    abi_ended(start_piece_chain_exit_shim(StartPieceChain(cpu.x)));
}

/*
 * @routine 072C:053 StartPieceStreamA
 * @run 70:B853
 * WHAT IT DOES: Starts a stream of pieces from one of two sides: sound $88, OBJ_WORK_I 1, OBJ_WORK_N 2, frame $10;
 * the side (OBJ_WORK_M) is a random bit and picks the start position from a table at $B894 (four bytes a side);
 * then StartPieceChain, OBJ_WORK_L 0 and the next OBJ_SUBSTATE state.
 * HOW: SoundCommand, NextRandom, tables read from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
 */
ObjArm StartPieceStreamA(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t side, byte;

    jsr_push(0xB858);
    SoundCommand(0x88, slot, y);
    jsr_pop();
    WR8(OBJ->work_i[slot], 1);
    WR8(OBJ->work_n[slot], 2);
    WR8(OBJ->frame[slot], 0x10);
    jsr_push(0xB86A);
    r.random = NextRandom();
    jsr_pop();
    side = (uint8_t)(r.random.value & 0x01);
    WR8(OBJ->work_m[slot], side);
    r.first = (uint8_t)(side << 2);
    byte = rd8((uint16_t)(0xB895 + r.first));
    WR8(OBJ->x_hi[slot], byte);
    byte = rd8((uint16_t)(0xB894 + r.first));
    WR8(OBJ->x_lo[slot], byte);
    byte = rd8((uint16_t)(0xB897 + r.first));
    WR8(OBJ->y_hi[slot], byte);
    r.second = rd8((uint16_t)(0xB896 + r.first));
    WR8(OBJ->y_lo[slot], r.second);
    jsr_push(0xB88D);
    r.sub = obj_sub(StartPieceChain(slot));
    jsr_pop();
    WR8(OBJ->work_l[slot], 0);
    r.third = INC8(OBJ->substate[slot]);
    return r;
}

static Ended start_piece_stream_a_exit_shim(ObjArm r)
{
    Ended e = ended_of_random(r.random);

    e = then(e, ended_with_carry(ended_with_y(ended_load(r.first), r.first), false));
    e = then(e, ended_with_y(ended_load(r.second), r.first));
    e = then(e, start_piece_chain_exit_shim(obj_arm(r.sub)));
    return then(e, ended_nz(r.third));
}

void abi_StartPieceStreamA(void)
{
    abi_ended(start_piece_stream_a_exit_shim(StartPieceStreamA(cpu.x, cpu.y)));
}

/*
 * @routine 072C:116 StartPieceStreamB
 * @run 70:B916
 * WHAT IT DOES: Starts a stream of pieces from the top: OBJ_WORK_N 2, sound $89, animation reset, X 0 with a
 * random low byte, Y $0110, StartPieceChain, then random X and Y speeds from tables at $B970 and $B978 (two
 * bytes each, four of them) and the next OBJ_SUBSTATE state; when the object is on the player's left
 * (SideOfPlayer bit 0) its X speed is reversed.
 * HOW: SoundCommand, NextRandom three times, SideOfPlayer, NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: A the side bits (or the reversed speed's high byte), N and Z from it, C and V from the
 * reversal when made, else C from the side and V from the last random.
 */
ObjArm StartPieceStreamB(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t picked, byte;

    WR8(OBJ->work_n[slot], 2);
    jsr_push(0xB920);
    SoundCommand(0x89, slot, y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xB92C);
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->x_lo[slot], random.value);
    WR8(OBJ->y_hi[slot], 1);
    WR8(OBJ->y_lo[slot], 0x10);
    jsr_push(0xB93C);
    r.sub = obj_sub(StartPieceChain(slot));
    jsr_pop();
    jsr_push(0xB93F);
    r.random = NextRandom();
    jsr_pop();
    picked = (uint8_t)(r.random.value & 0x03);
    byte = rd8((uint16_t)(0xB971 + (uint8_t)(picked << 1)));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB970 + (uint8_t)(picked << 1)));
    WR8(OBJ->vx_lo[slot], byte);
    jsr_push(0xB952);
    r.random2 = NextRandom();
    jsr_pop();
    picked = (uint8_t)(r.random2.value & 0x03);
    r.first = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xB979 + r.first));
    WR8(OBJ->vy_hi[slot], byte);
    r.second = rd8((uint16_t)(0xB978 + r.first));
    WR8(OBJ->vy_lo[slot], r.second);
    r.third = INC8(OBJ->substate[slot]);
    jsr_push(0xB968);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 0x01);
    r.arm = SB_STRAIGHT;
    if (r.fourth == 0) {
        return r;
    }
    jsr_push(0xB96F);
    r.sum = NegateVelocityX(slot);
    jsr_pop();
    r.arm = SB_REVERSED;
    return r;
}

static Ended start_piece_stream_b_exit_shim(ObjArm r)
{
    Ended e = start_piece_chain_exit_shim(obj_arm(r.sub));

    e = then(e, ended_of_random(r.random));
    e = then(e, ended_of_random(r.random2));
    e = then(e, ended_with_y(ended_load(r.second), r.first));
    e = then(e, ended_nz(r.third));
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.fourth));
    if (r.arm == SB_STRAIGHT) {
        return e;
    }
    return then(e, of_sum(r.sum));
}

void abi_StartPieceStreamB(void)
{
    abi_ended(start_piece_stream_b_exit_shim(StartPieceStreamB(cpu.x, cpu.y)));
}

/*
 * @routine 072C:180 StepFallingPieceStream
 * @run 70:B980
 * WHAT IT DOES: One frame of a falling piece stream: steps the animation list at $B9B3, sets the hitbox
 * flag by its direction, lifts the other objects (LiftObjectsBySlowFraction), adds $002C to its Y speed, moves,
 * and when its Y high byte reaches 8 clears its flash flag and OBJ_SUBSTATE.
 * HOW: StepObjectFrames, HitboxByVerticalDirection, LiftObjectsBySlowFraction, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: A the Y high byte, N, Z and C from its compare with 8,
 * V from the move; Y as the lift left it ($27).
 */
ObjArm StepFallingPieceStream(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0, low, high;
    Sum sum;

    step_list(slot, 0xB3, 0xB9, 0xB98B, &y);
    jsr_push(0xB98E);
    r.sub = obj_sub(HitboxByVerticalDirection(slot));
    jsr_pop();
    jsr_push(0xB991);
    r.sub2 = obj_sub(LiftObjectsBySlowFraction(slot));
    jsr_pop();
    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 0x2C, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    jsr_push(0xB9A5);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first == 8) {
        WR8(OBJ->sprite_attr[slot], 0);
        WR8(OBJ->substate[slot], 0);
    }
    return r;
}

static Ended step_falling_piece_stream_exit_shim(ObjArm r)
{
    Ended e = hitbox_by_vertical_direction_exit_shim(obj_arm(r.sub));

    e = then(e, lift_objects_by_slow_fraction_exit_shim(obj_arm(r.sub2)));
    e = then(e, ended_of_axis_move(r.move));
    return then(e, ended_compare(r.first, 8));
}

void abi_StepFallingPieceStream(void)
{
    abi_ended(step_falling_piece_stream_exit_shim(StepFallingPieceStream(cpu.x)));
}

/*
 * @routine 072C:1F0 StartPieceStreamC
 * @run 70:B9F0
 * WHAT IT DOES: Starts a stream of pieces from the bottom: OBJ_WORK_N 3, OBJ_WORK_E 0, a random side (OBJ_WORK_M)
 * picks X from the table at $BA3E and the X speed from $BA40; Y is $0110; StartPieceChain; Y speed $F7;
 * frame $80, animation reset, OBJ_WORK_I 2, OBJ_WORK_J $30 and the next OBJ_SUBSTATE state.
 * HOW: NextRandom, tables from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
 */
ObjArm StartPieceStreamC(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t side, byte;

    WR8(OBJ->work_n[slot], 3);
    WR8(OBJ->work_e[slot], 0);
    jsr_push(0xB9FB);
    r.random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)(r.random.value & 0x01);
    WR8(OBJ->work_m[slot], r.first);
    byte = rd8((uint16_t)(0xBA3E + r.first));
    WR8(OBJ->x_lo[slot], byte);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 1);
    WR8(OBJ->y_lo[slot], 0x10);
    jsr_push(0xBA17);
    r.sub = obj_sub(StartPieceChain(slot));
    jsr_pop();
    side = RD8(OBJ->work_m[slot]);
    r.second = side;
    byte = rd8((uint16_t)(0xBA40 + side));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0xF7);
    WR8(OBJ->frame[slot], 0x80);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->work_i[slot], 2);
    WR8(OBJ->work_j[slot], 0x30);
    r.third = INC8(OBJ->substate[slot]);
    return r;
}

static Ended start_piece_stream_c_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_random(r.random), ended_with_y(ended_load(0x10), r.first));

    e = then(e, start_piece_chain_exit_shim(obj_arm(r.sub)));
    return then(e, ended_with_y(ended_with_a(ended_nz(r.third), 0x30), r.second));
}

void abi_StartPieceStreamC(void)
{
    abi_ended(start_piece_stream_c_exit_shim(StartPieceStreamC(cpu.x)));
}

/*
 * @routine 072C:29F StepPieceEmitter
 * @run 70:BA9F
 * WHAT IT DOES: An object that sheds pieces while it plays its animation (list $BAE6): when the list ends it
 * moves on one OBJ_SUBSTATE state; otherwise, every fourth frame of its life timer, once its animation step is
 * $10 or more, on frame $13 and while OBJ_WORK_E is under 4, it starts an object of kind $26 at its place in the
 * first free slot from $20, copies its facing and gives it the count OBJ_WORK_E, which then goes up one.
 * HOW: StepObjectFrames, FindFreeSlotFrom20, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the last compare, load or INC.
 */
ObjArm StepPieceEmitter(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0, timer, found_slot, facing, count;
    bool found;

    r.step = step_list(slot, 0xE6, 0xBA, 0xBAAA, &y);
    r.y = y;
    r.arm = PE_ENDED;
    if (r.step == FRAME_LIST_ENDED) {
        r.first = INC8(OBJ->substate[slot]);
        return r;
    }
    timer = RD8(OBJ->life_timer[slot]);
    r.second = (uint8_t)(timer & 0x03);
    r.arm = PE_TIMER;
    if (r.second != 0) {
        return r;
    }
    r.third = RD8(OBJ->anim_step[slot]);
    r.arm = PE_ANIM;
    if (r.third < 0x10) {
        return r;
    }
    r.fourth = RD8(OBJ->frame[slot]);
    r.arm = PE_FRAME;
    if (r.fourth != 0x13) {
        return r;
    }
    r.fifth = RD8(OBJ->work_e[slot]);
    r.arm = PE_COUNT;
    if (r.fifth >= 4) {
        return r;
    }
    jsr_push(0xBACB);
    found_slot = FindFreeSlotFrom20(&r.sixth, &found);
    jsr_pop();
    r.seventh = found_slot;
    r.flag = found;
    r.arm = PE_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xBAD2);
    r.eighth = StartObjectAtPlace(0x26, slot, found_slot);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    count = RD8(OBJ->work_e[slot]);
    WR8(OBJ->life_timer[found_slot], count);
    r.ninth = INC8(OBJ->work_e[slot]);
    r.arm = PE_SHOT;
    return r;
}

static Ended step_piece_emitter_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == PE_ENDED) {
        return then(e, ended_nz(r.first));
    }
    e = then(e, ended_load(r.second));
    if (r.arm == PE_TIMER) {
        return e;
    }
    e = then(e, ended_compare(r.third, 0x10));
    if (r.arm == PE_ANIM) {
        return e;
    }
    e = then(e, ended_compare(r.fourth, 0x13));
    if (r.arm == PE_FRAME) {
        return e;
    }
    e = then(e, ended_compare(r.fifth, 4));
    if (r.arm == PE_COUNT) {
        return e;
    }
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), r.sixth), r.seventh), r.flag));
    if (r.arm == PE_NO_SLOT) {
        return e;
    }
    e = then(e, ended_load(r.eighth));
    return then(e, ended_nz(r.ninth));
}

void abi_StepPieceEmitter(void)
{
    abi_ended(step_piece_emitter_exit_shim(StepPieceEmitter(cpu.x)));
}
/* RecordObjectEvent's exit: unmarked, A 0 and Z; else A the byte stored, Y its index, C whether the event is a multiple of 8. */
static Ended ended_of_object_event(bool marked, uint8_t stored, uint8_t event, uint8_t byte)
{
    if (!marked) {
        return ended_load(0);
    }
    return ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0);
}

/*
 * @routine 072B:31B StartOnFirstState
 * @run 70:B31B
 * WHAT IT DOES: An object that only needs its record loaded: in state 0 it does LoadObjectRecord, otherwise nothing.
 * HOW: A tail jump into LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else A the state, N and Z from it.
 */
ObjArm StartOnFirstState(uint8_t slot)
{
    ObjArm r = { 0 };

    r.third = RD8(OBJ->state[slot]);
    r.arm = OF_LATER;
    if (r.third != 0) {
        return r;
    }
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    r.arm = OF_RECORD;
    return r;
}

static Ended start_on_first_state_exit_shim(ObjArm r)
{
    if (r.arm == OF_LATER) {
        return ended_load(r.third);
    }
    return record_exit_shim(r);
}

void abi_StartOnFirstState(void)
{
    abi_ended(start_on_first_state_exit_shim(StartOnFirstState(cpu.x)));
}

/*
 * @routine 072B:324 FallingDebris
 * @run 70:B324
 * WHAT IT DOES: A piece of debris: from state 0 it loads its record and takes frame $19 or $1A at random; later it
 * falls under unlimited gravity and goes away once its Y high byte is not 0.
 * HOW: NextRandom, ApplyGravityUnlimited, ApplyVelocityY, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: state 0: A the frame, N and Z from it, C and V from the add; else the move's registers
 * then the compare of the Y high byte.
 */
ObjArm FallingDebris(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t state, nz;
    bool carry;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = DE_START;
        jsr_push(0xB32C);
        LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        jsr_push(0xB32F);
        random = NextRandom();
        jsr_pop();
        r.sum = add_bytes((uint8_t)(random.value & 0x01), 0x19, false);
        WR8(OBJ->frame[slot], r.sum.value);
        return r;
    }
    r.arm = DE_FALL;
    jsr_push(0xB33B);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xB33E);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_hi[slot]);
    if (r.first != 0) {
        ClearObject(slot);
    }
    return r;
}

static Ended falling_debris_exit_shim(ObjArm r)
{
    if (r.arm == DE_START) {
        return ended_with_y(of_sum(r.sum), 11);
    }
    return then(ended_of_axis_move(r.move), ended_load(r.first));
}

void abi_FallingDebris(void)
{
    abi_ended(falling_debris_exit_shim(FallingDebris(cpu.x)));
}

/*
 * @routine 072B:3A4 FlashByHeight
 * @run 70:B3A4
 * WHAT IT DOES: Sets the object's flash flag: $86 while slot $10's flash flag is $86; else, when slot $10 is in state 2,
 * a byte of a table at $B3CB picked by the object's height in sixteens; else 0.
 * HOW: Reads slot $10's flash flag and state, then the object's Y low byte shifted four times.
 * WAITS: no.
 * Entry: X object. Exit: A the byte stored, N and Z from it; Y the table index when read.
 */
ObjArm FlashByHeight(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->sprite_attr[0x10]);
    r.arm = FB_COPY;
    if (r.first == 0x86) {
        WR8(OBJ->sprite_attr[slot], 0x86);
        return r;
    }
    r.second = RD8(OBJ->state[0x10]);
    r.arm = FB_NONE;
    if (r.second != 2) {
        WR8(OBJ->sprite_attr[slot], 0);
        return r;
    }
    r.third = RD8(OBJ->y_lo[slot]);
    r.fourth = (uint8_t)(r.third >> 4);
    r.fifth = rd8((uint16_t)(0xB3CB + r.fourth));
    WR8(OBJ->sprite_attr[slot], r.fifth);
    r.arm = FB_BY_HEIGHT;
    return r;
}

static Ended flash_by_height_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x86);

    if (r.arm == FB_COPY) {
        return then(e, ended_load(0x86));
    }
    e = then(e, ended_compare(r.second, 2));
    if (r.arm == FB_NONE) {
        return e;
    }
    return then(e, ended_with_carry(ended_with_y(ended_load(r.fifth), r.fourth), (r.third & 0x08) != 0));
}

void abi_FlashByHeight(void)
{
    abi_ended(flash_by_height_exit_shim(FlashByHeight(cpu.x)));
}

/*
 * @routine 072B:427 StartWithEvent
 * @run 70:B427
 * WHAT IT DOES: Loads the object's record and records its event (when it is one the save remembers).
 * HOW: LoadObjectRecord, RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as RecordObjectEvent.
 */
ObjArm StartWithEvent(uint8_t slot)
{
    ObjArm r = { 0 };
    bool marked;

    jsr_push(0xB42A);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    jsr_push(0xB42D);
    marked = RecordObjectEvent(slot, &r.third, &r.fourth, &r.fifth);
    jsr_pop();
    r.other = marked;
    return r;
}

static Ended start_with_event_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_of_object_event(r.other, r.third, r.fourth, r.fifth));
}

void abi_StartWithEvent(void)
{
    abi_ended(start_with_event_exit_shim(StartWithEvent(cpu.x)));
}

/*
 * @routine 072B:38D RaiseStateOnFlag
 * @run 70:B38D
 * WHAT IT DOES: When bit 7 of the object's status flags is set, an object in a state below 3 enters state 3 and
 * clears OBJ_SUBSTATE.
 * HOW: Compares the state with 3.
 * Untraced: every arm past the flag test (B394 to B39A) ran in no script.
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load.
 */
ObjArm RaiseStateOnFlag(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->hit_status[slot]);
    r.first = (uint8_t)(r.first & 0x80);
    r.arm = RF_NO_FLAG;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = RF_LATE;
    if (r.second >= 3) {
        return r;
    }
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->substate[slot], 0);
    r.arm = RF_RAISED;
    return r;
}

static Ended raise_state_on_flag_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == RF_NO_FLAG) {
        return e;
    }
    e = then(e, ended_compare(r.second, 3));
    if (r.arm == RF_LATE) {
        return e;
    }
    return then(e, ended_load(3));
}

void abi_RaiseStateOnFlag(void)
{
    abi_ended(raise_state_on_flag_exit_shim(RaiseStateOnFlag(cpu.x)));
}

/*
 * @routine 072C:656 BurstAtHeightE0
 * @run 70:BE56
 * WHAT IT DOES: When the object is on the first screen row and its Y low byte is in $E0 to $EF it plays sound $23 and starts a burst of debris (StartDebrisBurst, timer 0).
 * HOW: Two compares, SoundCommand, a tail jump into StartDebrisBurst.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: no burst: the last compare or load; else as StartDebrisBurst.
 */
ObjArm BurstAtHeightE0(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = BH_HIGH;
    if (r.first != 0) {
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    r.second = (uint8_t)(r.second & 0xF0);
    r.arm = BH_NO_BURST;
    if (r.second != 0xE0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x23;
    jsr_push(0xBE69);
    r.call = SoundCommand(0x23, slot, y);
    jsr_pop();
    r.sub = obj_sub(StartDebrisBurst(slot, 0, y));
    r.arm = BH_BURST;
    return r;
}

static SoundEnd burst_at_height_e0_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_load(r.first);

    s.end = e;
    if (r.arm == BH_HIGH) {
        return s;
    }
    e = then(e, ended_load(r.second));
    e = then(e, ended_compare(r.second, 0xE0));
    s.end = e;
    if (r.arm == BH_NO_BURST) {
        return s;
    }
    s.before = then(e, ended_load(0x23));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = start_debris_burst_exit_shim(obj_arm(r.sub));
    return s;
}

void abi_BurstAtHeightE0(void)
{
    abi_sound_end(burst_at_height_e0_exit_shim(BurstAtHeightE0(cpu.x, cpu.y)));
}

/*
 * @routine 072C:5F5 ShowByVelocity
 * @run 70:BDF5
 * WHAT IT DOES: Runs BurstAtHeightE0; the object's frame comes from a table at $BE39 by its timer, its hitbox half width and height from a table at $BE4F by that frame less one;
 * when its X speed's high byte is not between 2 and $FD the frame goes up 7.
 * HOW: BurstAtHeightE0, table reads (bank 70).
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm ShowByVelocity(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t frame;

    jsr_push(0xBDF8);
    r.sub = obj_sub(BurstAtHeightE0(slot, y));
    jsr_pop();
    r.first = RD8(OBJ->life_timer[slot]);
    r.second = rd8((uint16_t)(0xBE39 + r.first));
    WR8(OBJ->frame[slot], r.second);
    r.sum = sub_bytes(r.second, 1, true);
    r.third = rd8((uint16_t)(0xBE4F + r.sum.value));
    WR8(OBJ->hitbox_half_w[slot], r.third);
    WR8(OBJ->hitbox_half_h[slot], r.third);
    r.fourth = RD8(OBJ->vx_hi[slot]);
    r.arm = SV_PLAIN;
    if (r.fourth >= 2 && r.fourth < 0xFE) {
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    r.sum2 = add_bytes(frame, 7, false);
    WR8(OBJ->frame[slot], r.sum2.value);
    r.arm = SV_RAISED;
    return r;
}

/* The burst's sound exit, then what the table reads and the frame's add left. */
static ShowEnd show_by_velocity_exit_shim(ObjArm r)
{
    ShowEnd s = { { 0 } };
    Ended e = ended_with_y(ended_load(r.second), r.first);

    s.burst = burst_at_height_e0_exit_shim(obj_arm(r.sub));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_with_y(ended_kept(), r.sum.value));
    e = then(e, ended_load(r.third));
    e = then(e, ended_compare(r.fourth, 2));
    if (r.fourth >= 2) {
        e = then(e, ended_compare(r.fourth, 0xFE));
    }
    if (r.arm == SV_PLAIN) {
        s.tail = e;
        return s;
    }
    s.tail = then(e, of_sum(r.sum2));
    return s;
}

void abi_ShowByVelocity(void)
{
    ShowEnd s = show_by_velocity_exit_shim(ShowByVelocity(cpu.x, cpu.y));

    abi_sound_end(s.burst);
    abi_ended(s.tail);
}

/* The move a drifting piece makes from the four work tables at $B46E, $B52E, $B5EE and $B6AE, by an index its own table gives. */
static ObjArm speed_from_tables(uint8_t slot, uint16_t index_table, uint16_t ret, uint8_t *y_out)
{
    ObjArm r = { 0 };
    uint8_t byte;

    r.first = RD8(OBJ->life_timer[slot]);
    r.second = rd8((uint16_t)(index_table + r.first));
    r.sum = add_bytes(r.second, 4, false);
    *y_out = r.sum.value;
    byte = rd8((uint16_t)(0xB46E + r.sum.value));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB52E + r.sum.value));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xB5EE + r.sum.value));
    WR8(OBJ->vy_hi[slot], byte);
    byte = rd8((uint16_t)(0xB6AE + r.sum.value));
    WR8(OBJ->vy_lo[slot], byte);
    jsr_push(ret);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    return r;
}

static Ended speed_from_tables_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_load(r.second), r.first);

    e = then(e, ended_with_y(of_sum(r.sum), r.sum.value));
    return then(e, ended_of_axis_move(r.move));
}

/* The exit of the follow routines: the table move's registers, then ShowByVelocity's sound exit and tail. */
static ShowEnd path_show_exit_shim(PathShow p)
{
    ShowEnd s = show_by_velocity_exit_shim(p.show);

    s.head = speed_from_tables_exit_shim(p.head);
    return s;
}

static void abi_path_show(PathShow p)
{
    ShowEnd s = path_show_exit_shim(p);

    abi_ended(s.head);
    abi_sound_end(s.burst);
    abi_ended(s.tail);
}

/*
 * @routine 072C:5D0 FollowPathAndShow
 * @run 70:BDD0
 * WHAT IT DOES: Takes the object's speed from the four path tables (index from the table at $BE24 by its timer, plus 4), moves it and goes on as ShowByVelocity.
 * HOW: Table reads (bank 70), ApplyVelocity, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ShowByVelocity, over the move's.
 */
PathShow FollowPathAndShow(uint8_t slot, uint8_t y)
{
    PathShow p;

    p.head = speed_from_tables(slot, 0xBE24, 0xBDF5, &y);
    p.show = ShowByVelocity(slot, y);
    return p;
}

void abi_FollowPathAndShow(void)
{
    abi_path_show(FollowPathAndShow(cpu.x, cpu.y));
}

/*
 * @routine 072C:5CD SetHitboxThenFollowPath
 * @run 70:BDCD
 * WHAT IT DOES: Sets the object's hitbox flag by its direction (HitboxByVerticalDirection) and goes on as FollowPathAndShow.
 * HOW: HitboxByVerticalDirection, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
 */
HitboxPathShow SetHitboxThenFollowPath(uint8_t slot, uint8_t y)
{
    HitboxPathShow h;

    jsr_push(0xBDD0);
    h.first = HitboxByVerticalDirection(slot);
    jsr_pop();
    h.path = FollowPathAndShow(slot, y);
    return h;
}

void abi_SetHitboxThenFollowPath(void)
{
    HitboxPathShow h = SetHitboxThenFollowPath(cpu.x, cpu.y);

    abi_ended(hitbox_by_vertical_direction_exit_shim(h.first));
    abi_path_show(h.path);
}

/*
 * @routine 072C:671 FollowPathB
 * @run 70:BE71
 * WHAT IT DOES: As FollowPathAndShow with the index table at $BE99, going on as ShowByVelocity by a jump.
 * HOW: Table reads (bank 70), ApplyVelocity, a tail jump.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
 */
PathShow FollowPathB(uint8_t slot, uint8_t y)
{
    PathShow p;

    p.head = speed_from_tables(slot, 0xBE99, 0xBE96, &y);
    p.show = ShowByVelocity(slot, y);
    return p;
}

void abi_FollowPathB(void)
{
    abi_path_show(FollowPathB(cpu.x, cpu.y));
}

/*
 * @routine 072C:77B StartDriftingPiece
 * @run 70:BF7B
 * WHAT IT DOES: Loads the object's record, takes its X speed from a table at $BFA2 by its timer, a Y speed of 4, and reverses the X speed with sound $25 when slot $10 faces left.
 * HOW: LoadObjectRecord, NegateVelocityX, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm StartDriftingPiece(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t byte;

    (void)y;
    jsr_push(0xBF7E);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = RD8(OBJ->life_timer[slot]);
    r.fourth = (uint8_t)(r.third << 1);
    byte = rd8((uint16_t)(0xBFA3 + r.fourth));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xBFA2 + r.fourth));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], 4);
    r.fifth = RD8(OBJ->facing_left[0x10]);
    r.arm = DR_STRAIGHT;
    if (r.fifth == 0) {
        return r;
    }
    jsr_push(0xBF9C);
    r.sum = NegateVelocityX(slot);
    jsr_pop();
    r.sounded = true;
    r.command = 0x25;
    jsr_push(0xBFA1);
    r.call = SoundCommand(0x25, slot, r.fourth);
    jsr_pop();
    r.arm = DR_REVERSED;
    return r;
}

static SoundEnd start_drifting_piece_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = then(record_exit_shim(r), ended_with_y(ended_with_carry(ended_load(r.fourth), (r.third & 0x80) != 0), r.fourth));

    e = then(e, ended_load(4));
    e = then(e, ended_load(r.fifth));
    s.end = e;
    if (r.arm == DR_STRAIGHT) {
        return s;
    }
    e = then(e, of_sum(r.sum));
    s.before = then(e, ended_load(0x25));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_kept();
    return s;
}

void abi_StartDriftingPiece(void)
{
    abi_sound_end(start_drifting_piece_exit_shim(StartDriftingPiece(cpu.x, cpu.y)));
}

/*
 * @routine 072C:7AA DropAndBurst
 * @run 70:BFAA
 * WHAT IT DOES: Steps the animation list at $BFEF and moves; once its Y low byte reaches $7F it sets it to $7F, plays sound $23, starts a burst of debris (timer 0), sets its hitbox
 * to $10 by $0C, stops sideways, sets a Y speed of $FF00, resets the animation and enters its next state.
 * HOW: StepObjectFrames, ApplyVelocity, SoundCommand, StartDebrisBurst.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm DropAndBurst(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.step = step_list(slot, 0xEF, 0xBF, 0xBFB5, &y);
    r.y = y;
    jsr_push(0xBFB8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.first = RD8(OBJ->y_lo[slot]);
    r.arm = DA_FALLING;
    if (r.first < 0x7F) {
        return r;
    }
    WR8(OBJ->y_lo[slot], 0x7F);
    r.sounded = true;
    r.command = 0x23;
    jsr_push(0xBFC9);
    r.call = SoundCommand(0x23, slot, y);
    jsr_pop();
    jsr_push(0xBFCD);
    r.sub = obj_sub(StartDebrisBurst(slot, 0, y));
    jsr_pop();
    WR8(OBJ->hitbox_half_w[slot], 0x10);
    WR8(OBJ->hitbox_half_h[slot], 0x0C);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    r.arm = DA_BURST;
    return r;
}

static BurstTail drop_and_burst_exit_shim(ObjArm r)
{
    BurstTail t = { { { 0 }, { 0 }, false, 0, { 0 } }, { 0 } };
    Ended e = then(then(frame_step_exit_shim(r), ended_of_axis_move(r.move)), ended_compare(r.first, 0x7F));

    t.pre.end = e;
    if (r.arm == DA_FALLING) {
        return t;
    }
    t.pre.before = then(e, ended_load(0x23));
    t.pre.sounded = true;
    t.pre.command = r.command;
    t.pre.call = r.call;
    t.pre.end = start_debris_burst_exit_shim(obj_arm(r.sub));
    t.tail = ended_with_a(ended_nz(r.second), 0xFF);
    return t;
}

void abi_DropAndBurst(void)
{
    BurstTail r = drop_and_burst_exit_shim(DropAndBurst(cpu.x, cpu.y));

    abi_sound_end(r.pre);
    abi_ended(r.tail);
}

/*
 * @routine 072C:7F4 StepFallAfterFour
 * @run 70:BFF4
 * WHAT IT DOES: Steps the animation list at $C010; once its animation step is 4 or more the hitbox goes off and the object falls (ApplyGravity, ApplyVelocity).
 * HOW: StepObjectFrames, ApplyGravity, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity when it falls; else the compare's flags.
 */
ObjArm StepFallAfterFour(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y = 0;

    r.step = step_list(slot, 0x10, 0xC0, 0xBFFF, &y);
    r.y = y;
    r.first = RD8(OBJ->anim_step[slot]);
    r.arm = FA_SHORT;
    if (r.first < 4) {
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    jsr_push(0xC00C);
    ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xC00F);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.arm = FA_FALLING;
    return r;
}

static Ended step_fall_after_four_exit_shim(ObjArm r)
{
    Ended e = then(frame_step_exit_shim(r), ended_compare(r.first, 4));

    if (r.arm == FA_SHORT) {
        return e;
    }
    return then(e, ended_of_axis_move(r.move));
}

void abi_StepFallAfterFour(void)
{
    abi_ended(step_fall_after_four_exit_shim(StepFallAfterFour(cpu.x)));
}

/*
 * @routine 072B:2B5 BreakIntoPieces
 * @run 70:B2B5
 * WHAT IT DOES: In state 0 loads the object's record. Once it has taken a hit of 5 or more it records its event, starts a kind $84 piece at its place in each of slots $3D to $3F with
 * a speed row (four bytes each) from the table at $B30F, plays sound $19, starts a burst of debris (timer 1) and becomes a kind $9C object in state 0.
 * HOW: LoadObjectRecord, RecordObjectEvent, ClearObjectInY and StartObjectAtPlace in a loop, SoundCommand, StartDebrisBurst.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
ObjArm BreakIntoPieces(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t state, byte, to, row, number;
    bool marked;

    (void)y_entry;
    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = BP_RECORD;
        jsr_push(0xB2BD);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        return r;
    }
    r.third = RD8(OBJ->hit_taken[slot]);
    r.arm = BP_MILD;
    if (r.third < 5) {
        return r;
    }
    jsr_push(0xB2C8);
    marked = RecordObjectEvent(slot, &r.fourth, &r.fifth, &r.sixth);
    jsr_pop();
    r.other = marked;
    WR8(ZP->temp00, 0);
    to = 0x3D;
    do {
        jsr_push(0xB2CF);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB2D4);
        StartObjectAtPlace(0x84, slot, to);
        jsr_pop();
        push8(slot);
        row = RD8(ZP->temp00);
        byte = rd8((uint16_t)(0xB310 + row));
        WR8(OBJ->vx_hi[to], byte);
        byte = rd8((uint16_t)(0xB30F + row));
        WR8(OBJ->vx_lo[to], byte);
        byte = rd8((uint16_t)(0xB312 + row));
        WR8(OBJ->vy_hi[to], byte);
        byte = rd8((uint16_t)(0xB311 + row));
        WR8(OBJ->vy_lo[to], byte);
        slot = pull8();
        number = RD8(ZP->temp00);
        number = (uint8_t)(number + 4);
        WR8(ZP->temp00, number);
        to++;
    } while (to < 0x40);
    r.sounded = true;
    r.command = 0x19;
    jsr_push(0xB301);
    r.call = SoundCommand(0x19, slot, 0x40);
    jsr_pop();
    jsr_push(0xB306);
    r.sub = obj_sub(StartDebrisBurst(slot, 1, 0x40));
    jsr_pop();
    WR8(OBJ->kind[slot], 0x9C);
    WR8(OBJ->state[slot], 0);
    r.arm = BP_BROKEN;
    return r;
}

static BreakEnd break_into_pieces_exit_shim(ObjArm r)
{
    BreakEnd b = { { { 0 }, { 0 }, false, 0, { 0 } }, { 0 }, false, { 0 }, 0, { 0 } };
    Ended e;

    if (r.arm == BP_RECORD) {
        b.sound.end = record_exit_shim(r);
        return b;
    }
    e = ended_compare(r.third, 5);
    b.sound.end = e;
    if (r.arm == BP_MILD) {
        return b;
    }
    e = then(e, ended_of_object_event(r.other, r.fourth, r.fifth, r.sixth));
    b.sound.before = then(e, ended_load(0x19));
    b.sound.sounded = true;
    b.sound.command = r.command;
    b.sound.call = r.call;
    b.sound.end = ended_kept();
    b.burst = start_debris_burst_exit_shim(obj_arm(r.sub));
    b.burst_run = true;
    b.tail = ended_load(0x9C);
    return b;
}

void abi_BreakIntoPieces(void)
{
    BreakEnd r = break_into_pieces_exit_shim(BreakIntoPieces(cpu.x, cpu.y));

    abi_sound_end(r.sound);
    if (r.burst_run) {
        abi_ended(r.burst);
        abi_ended(r.tail);
    }
}

/*
 * @routine 072D:09F StartScriptsAndLists
 * @run 71:C09F
 * WHAT IT DOES: In state 0 starts script slot 4 with script $91 and loads the object's record; later it starts list channel 4 with the list $AE plus the low nibble of ScrollYLo.
 * HOW: StartScriptSlotKind1, LoadObjectRecord (a tail jump), StartListChannelKind2; X is pushed around each start.
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else A the channel's kind, N and Z from X, C and V from the add.
 */
ObjArm StartScriptsAndLists(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, scroll;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.arm = SL_RECORD;
        push8(slot);
        jsr_push(0xC0AC);
        StartScriptSlotKind1(0x91, 4);
        jsr_pop();
        slot = pull8();
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    r.arm = SL_LIST;
    scroll = rd8(ScrollYLo);
    r.third = (uint8_t)(scroll & 0x0F);
    r.sum = add_bytes(r.third, 0xAE, false);
    push8(slot);
    jsr_push(0xC0BE);
    r.fourth = StartListChannelKind2(r.sum.value, 4);
    jsr_pop();
    r.fifth = pull8();
    return r;
}

static Ended start_scripts_and_lists_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == SL_RECORD) {
        return record_exit_shim(r);
    }
    e = then(ended_load(r.third), of_sum(r.sum));
    e = then(e, ended_load(r.fourth));
    return then(e, ended_nz(r.fifth));
}

void abi_StartScriptsAndLists(void)
{
    abi_ended(start_scripts_and_lists_exit_shim(StartScriptsAndLists(cpu.x)));
}

/*
 * @routine 072B:76E StepThenMusicAgain
 * @run 70:B76E
 * WHAT IT DOES: Steps the animation list at $B7AD; on the new frame with animation step 2 it sets bit 4 of zField8E and plays sound $FE; when the list ends it turns on the object's
 * hitbox flag (2), clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), clears the object flags (OBJ_2F38), sets bit 2 of zField8E and bit 4 and 6 of OBJ_PACE, and enters its next state.
 * HOW: StepObjectFrames, SoundCommand, CdPlayAgain.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm StepThenMusicAgain(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t flags;

    r.step = step_list(slot, 0xAD, 0xB7, 0xB779, &y);
    r.y = y;
    if (r.step == FRAME_HELD) {
        return r;
    }
    if (r.step == FRAME_CHANGED) {
        r.first = RD8(OBJ->anim_step[slot]);
        if (r.first != 2) {
            return r;
        }
        RMW8(ZP->field8_e, old, old | 0x10);
        r.sounded = true;
        r.command = 0xFE;
        jsr_push(0xB78B);
        r.call = SoundCommand(0xFE, slot, y);
        jsr_pop();
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    RMW8(ZP->field8_e, old, old & 0xF7);
    RMW8(ZP->field8_e, old, old & 0xEF);
    jsr_push(0xB798);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    r.flag = true;
    WR8(OBJ->scroll_band[slot], 0);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8_e, flags);
    r.second = RD8(OBJ->pace[slot]);
    r.second = (uint8_t)(r.second | 0x50);
    WR8(OBJ->pace[slot], r.second);
    r.third = INC8(OBJ->state[slot]);
    return r;
}

static MusicEnd step_then_music_again_exit_shim(ObjArm r)
{
    MusicEnd m = { { { 0 }, { 0 }, false, 0, { 0 } }, false, { 0 }, { 0 }, { 0 } };
    Ended e = frame_step_exit_shim(r);

    m.sound.end = e;
    if (r.step == FRAME_HELD) {
        return m;
    }
    if (r.step == FRAME_CHANGED) {
        e = then(e, ended_compare(r.first, 2));
        m.sound.end = e;
        if (r.first != 2) {
            return m;
        }
        m.sound.before = then(e, ended_load(0xFE));
        m.sound.sounded = true;
        m.sound.command = r.command;
        m.sound.call = r.call;
        m.sound.end = ended_kept();
        return m;
    }
    m.pre = then(e, ended_load(2));
    m.played = true;
    m.track = r.track;
    m.post = then(ended_load(r.second), ended_nz(r.third));
    return m;
}

void abi_StepThenMusicAgain(void)
{
    MusicEnd r = step_then_music_again_exit_shim(StepThenMusicAgain(cpu.x, cpu.y));

    if (r.played) {
        abi_ended(r.pre);
        play_track_exit_shim(r.track);
        abi_ended(r.post);
    } else {
        abi_sound_end(r.sound);
    }
}

/*
 * @routine 072B:3DB FaceByVelocityAndCatch
 * @run 70:B3DB
 * WHAT IT DOES: The object faces the way it moves (facing is the X speed's sign bit turned round) and its OBJ_SPRITE_FLIP is cleared; it is then set to $80 when the stage's flag word (OBJ_WORK_N of slot
 * $10) has the bit the Y speed's direction names (1 for falling, 2 for rising), the object is on the first screen row and its Y low byte is from $74 to $AF.
 * HOW: Shifts through carry for the sign bits, an AND with the byte at $3838 (OBJ_WORK_N slot $10), compares.
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load; C bit 6 of the Y speed.
 */
ObjArm FaceByVelocityAndCatch(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t mask;

    r.first = RD8(OBJ->vx_hi[slot]);
    r.second = (uint8_t)(((r.first >> 7) & 0x01) ^ 0x01);
    WR8(OBJ->facing_left[slot], r.second);
    WR8(OBJ->f2fe8[slot], 0);
    r.third = RD8(OBJ->vy_hi[slot]);
    r.fourth = (uint8_t)((r.third >> 7) & 0x01);
    mask = (uint8_t)(r.fourth + 1);
    r.fifth = RD8(OBJ->work_n[0x10]);
    r.sixth = (uint8_t)(mask & r.fifth);
    r.arm = FV_NO_MASK;
    if (r.sixth == 0) {
        return r;
    }
    r.seventh = RD8(OBJ->y_hi[slot]);
    r.arm = FV_HIGH;
    if (r.seventh != 0) {
        return r;
    }
    r.eighth = RD8(OBJ->y_lo[slot]);
    r.arm = FV_LOW_Y;
    if (r.eighth < 0x74) {
        return r;
    }
    r.arm = FV_HIGH_Y;
    if (r.eighth >= 0xB0) {
        return r;
    }
    WR8(OBJ->f2fe8[slot], 0x80);
    r.arm = FV_SET;
    return r;
}

static Ended face_by_velocity_and_catch_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_load(r.second), (r.first & 0x40) != 0);

    e = then(e, ended_with_carry(ended_load(r.fourth), (r.third & 0x40) != 0));
    e = then(e, ended_load((uint8_t)(r.fourth + 1)));
    e = then(e, ended_load(r.sixth));
    if (r.arm == FV_NO_MASK) {
        return e;
    }
    e = then(e, ended_load(r.seventh));
    if (r.arm == FV_HIGH) {
        return e;
    }
    e = then(e, ended_compare(r.eighth, 0x74));
    if (r.arm == FV_LOW_Y) {
        return e;
    }
    e = then(e, ended_compare(r.eighth, 0xB0));
    if (r.arm == FV_HIGH_Y) {
        return e;
    }
    return then(e, ended_load(0x80));
}

void abi_FaceByVelocityAndCatch(void)
{
    abi_ended(face_by_velocity_and_catch_exit_shim(FaceByVelocityAndCatch(cpu.x)));
}

/* A jump a state's handler table makes: the table's two bytes go to zTemp00-01 and the code is jumped through them. */
static ObjArm jump_by_state(uint8_t slot, uint16_t table)
{
    ObjArm r = { 0 };
    uint8_t low;

    r.third = RD8(OBJ->state[slot]);
    r.second = (uint8_t)(r.third << 1);
    low = rd8((uint16_t)(table + r.second));
    WR8(ZP->temp00, low);
    r.first = rd8((uint16_t)(table + 1 + r.second));
    WR8(ZP->temp01, r.first);
    r.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return r;
}

static Jump jump_by_state_exit_shim(ObjArm r)
{
    return jump_by_stage_step_exit_shim(r);
}

/*
 * @routine 072B:40D JumpByStateTableB
 * @run 70:B40D
 * WHAT IT DOES: Goes to the code a table at $B41F names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
ObjArm JumpByStateTableB(uint8_t slot)
{
    return jump_by_state(slot, 0xB41F);
}

void abi_JumpByStateTableB(void)
{
    Jump jump = jump_by_state_exit_shim(JumpByStateTableB(cpu.x));

    jump_shim(jump.end, jump.target);
}

/* What RunDriftObject's four calls have left once the jump's flags are in. */
static Ended run_drift_object_chain(ObjArm r)
{
    Ended e = raise_state_on_flag_exit_shim(obj_arm(r.sub));

    e = then(e, flash_by_height_exit_shim(obj_arm(r.sub2)));
    e = then(e, face_by_velocity_and_catch_exit_shim(obj_arm(r.sub3)));
    return then(e, jump_by_state_exit_shim(obj_arm(r.sub4)).end);
}

/* The registers the handler is entered with. */
static void run_drift_object_enter_shim(ObjArm r)
{
    abi_ended(run_drift_object_chain(r));
}

/*
 * @routine 072B:348 RunDriftObject
 * @run 70:B348
 * WHAT IT DOES: One frame of the drifting object: its life timer goes up, RaiseStateOnFlag, FlashByHeight and FaceByVelocityAndCatch run, the code its state names runs, then the four path tables at $B46E,
 * $B52E, $B5EE and $B6AE are shifted up one entry (a trail of its speeds, 192 entries each) and its speeds go into their first entries.
 * HOW: Four calls, one jump through a table run as the original, two loops of table stores.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y 0, N and Z of the last table copy.
 */
ObjArm RunDriftObject(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, byte;

    INC8(OBJ->life_timer[slot]);
    jsr_push(0xB34E);
    r.sub = obj_sub(RaiseStateOnFlag(slot));
    jsr_pop();
    jsr_push(0xB351);
    r.sub2 = obj_sub(FlashByHeight(slot));
    jsr_pop();
    jsr_push(0xB354);
    r.sub3 = obj_sub(FaceByVelocityAndCatch(slot));
    jsr_pop();
    jsr_push(0xB357);
    r.sub4 = obj_sub(JumpByStateTableB(slot));
    run_drift_object_enter_shim(r);
    slot = enter_state_handler(slot, jump_by_state_exit_shim(obj_arm(r.sub4)));
    jsr_pop();
    for (y = 0xBF; y != 0; y--) {
        byte = rd8((uint16_t)(0xB46D + y));
        wr8((uint16_t)(0xB46E + y), byte);
        byte = rd8((uint16_t)(0xB52D + y));
        wr8((uint16_t)(0xB52E + y), byte);
        byte = rd8((uint16_t)(0xB5ED + y));
        wr8((uint16_t)(0xB5EE + y), byte);
        byte = rd8((uint16_t)(0xB6AD + y));
        wr8((uint16_t)(0xB6AE + y), byte);
    }
    byte = RD8(OBJ->vx_hi[slot]);
    wr8(0xB46E, byte);
    byte = RD8(OBJ->vx_lo[slot]);
    wr8(0xB52E, byte);
    byte = RD8(OBJ->vy_hi[slot]);
    wr8(0xB5EE, byte);
    r.first = RD8(OBJ->vy_lo[slot]);
    wr8(0xB6AE, r.first);
    return r;
}

/* The last table copy's A, Y and N, Z are what the routine leaves. */
static void run_drift_object_exit_shim(ObjArm r)
{
    abi_ended(ended_with_y(ended_with_a(ended_nz(r.first), r.first), 0));
}

void abi_RunDriftObject(void)
{
    run_drift_object_exit_shim(RunDriftObject(cpu.x));
}

/* The exit of a routine that ends in a jump or in a return: the jump's target when it jumps. */
static void abi_jump_exit(Jump jump)
{
    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * DrawTilePiece's three tables in each of the 24 overlays that hold it at $A882 (the same instructions in all, only these
 * operands differ, read from the disc): the items by OBJ_WORK_E, the rows by OBJ_WORK_E, the entries by row plus OBJ_HIT_TAKEN.
 * TilePieceInRange at $A860 is the same bytes in all 24, so its C runs whichever is loaded; it and DrawTilePiece go to
 * the tables of the copy resident at $A882.
 */
typedef struct { uint16_t items, rows, entries; } PieceTables;

static const struct { uint16_t sector; PieceTables t; } PIECE_TABLES[] = {
    { 0x025A, { 0xAE9D, 0xAE9E, 0xAE9A } },
    { 0x051A, { 0xB081, 0xB082, 0xB0DE } },
    { 0x05CA, { 0xAE63, 0xAE64, 0xAE70 } },
    { 0x072A, { 0xAE97, 0xAE98, 0xAEA4 } },
    { 0x07DA, { 0xD39D, 0xD39E, 0xD3CA } },
    { 0x088A, { 0xC224, 0xC225, 0xC231 } },
    { 0x093A, { 0xB3AA, 0xB3AB, 0xB3B7 } },
    { 0x09EA, { 0xBCE4, 0xBCE5, 0xBCF1 } },
    { 0x0A9A, { 0xB67A, 0xB67B, 0xB6C7 } },
    { 0x0B4A, { 0xB371, 0xB372, 0xB36E } },
    { 0x0BFA, { 0xB039, 0xB03A, 0xB0A6 } },
    { 0x0CAA, { 0xAE9F, 0xAEA0, 0xAEBC } },
    { 0x0D5A, { 0xB01E, 0xB01F, 0xB07B } },
    { 0x0E0A, { 0xAEF6, 0xAEF7, 0xAF03 } },
    { 0x0EBA, { 0xAF1F, 0xAF20, 0xAF3C } },
    { 0x0F6A, { 0xADBF, 0xADC0, 0xADBC } },
    { 0x101A, { 0xB019, 0xB01A, 0xB046 } },
    { 0x10CA, { 0xAEF6, 0xAEF7, 0xAF03 } },
    { 0x117A, { 0xB075, 0xB076, 0xB0B2 } },
    { 0x12DA, { 0xAEE1, 0xAEE2, 0xAEFE } },
    { 0x13FA, { 0xAEA9, 0xAEAA, 0xAEC6 } },
    { 0x161A, { 0xB0A5, 0xB0A6, 0xB0F2 } },
    { 0x16CA, { 0xAE7F, 0xAE80, 0xAEBC } },
    { 0x18DA, { 0xAE87, 0xAE88, 0xAEC4 } },
};

static PieceTables piece_tables_loaded(void)
{
    uint32_t at = resident_at(0xA882);

    for (unsigned i = 0; i < sizeof PIECE_TABLES / sizeof PIECE_TABLES[0]; i++)
        if (at == DISC(PIECE_TABLES[i].sector, 0x082))
            return PIECE_TABLES[i].t;
    return (PieceTables){ 0xAE97, 0xAE98, 0xAEA4 };   /* none of the 24 at $A882: 072A's, this file's own */
}

/* The part of DrawTilePiece from $A890 on, which the position test at $A860 also reaches: the table entry decides whether the code at $A8A8 runs. */
static ObjArm piece_table_entry_at(uint8_t slot, PieceTables tables)
{
    ObjArm r = { 0 };
    uint8_t row;

    r.first = RD8(OBJ->f33b0[slot]);
    WR8(ZP->temp00, r.first);
    r.second = RD8(OBJ->work_e[slot]);
    r.third = rd8((uint16_t)(tables.rows + r.second));
    r.fourth = (uint8_t)(r.third << 4);
    row = RD8(ZP->temp00);
    r.sum = add_bytes(r.fourth, row, false);
    r.fifth = rd8((uint16_t)(tables.entries + r.sum.value));
    r.target = r.fifth != 0 ? 0xA8A8 : 0xA8ED;
    return r;
}

static ObjArm piece_table_entry(uint8_t slot)
{
    return piece_table_entry_at(slot, piece_tables_loaded());
}

/* The table entry's loads over the exit `e` the caller's instructions left. */
static Jump piece_table_entry_exit_shim(Ended e, ObjArm r)
{
    Jump jump = { ended_kept(), r.target, r.fifth != 0 };    /* entry 0: $A8ED is the RTS, no jump */

    e = then(e, ended_load(r.first));
    e = then(e, ended_with_y(ended_nz(r.second), r.second));
    e = then(e, ended_load(r.third));
    e = then(e, ended_with_carry(ended_load(r.fourth), (r.third & 0x10) != 0));
    e = then(e, of_sum(r.sum));
    e = then(e, ended_with_y(ended_nz(r.sum.value), r.sum.value));
    e = then(e, ended_load(r.fifth));
    jump.end = e;
    return jump;
}

/*
 * @routine 072A:055 BitMaskFromIndex072A
 * @run 70:A855
 * WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more). This is the overlay at sector 072A's copy of BitMaskFromIndex.
 * HOW: BitMaskFromIndex.
 * WAITS: no.
 * Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear.
 */
uint8_t BitMaskFromIndex072A(uint8_t count)
{
    return BitMaskFromIndex(count);
}

void abi_BitMaskFromIndex(void);

void abi_BitMaskFromIndex072A(void)
{
    abi_BitMaskFromIndex();
}

/*
 * @routine 072A:082 DrawTilePiece072A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: Draws the object's tile script (the item the table at $AE97 gives by OBJ_WORK_E, plus OBJ_WORK_D), then looks in the table at $AEA4 (the row $AE98 gives
 * by OBJ_WORK_E, plus OBJ_HIT_TAKEN) and goes on to the code at $A8A8 when the entry is not zero, else to the return at $A8ED (072A:0ED).
 * HOW: DrawTileScript, a table read (bank 70), then a tail jump into the original bytes at $A8A8 (never run in a script).
 * WAITS: no.
 * Entry: X object. Exit: entry zero: as the table read; else the jump's.
 */
static ObjArm draw_tile_piece_at(uint8_t slot, PieceTables tables)
{
    ObjArm r = { 0 };
    uint8_t item;

    r.first = RD8(OBJ->work_e[slot]);
    item = rd8((uint16_t)(tables.items + r.first));
    r.sum = add_bytes(item, RD8(OBJ->f34b8[slot]), false);
    jsr_push(0xA890);
    r.run = DrawTileScript(r.sum.value, 0);
    jsr_pop();
    r.sub = obj_sub(piece_table_entry_at(slot, tables));
    return r;
}

ObjArm DrawTilePiece072A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE97, 0xAE98, 0xAEA4 });
}

static Jump draw_tile_piece072a_exit_shim(ObjArm r)
{
    Ended e = then(then(ended_with_y(ended_nz(r.first), r.first), of_sum(r.sum)), ended_of_tile_run(r.run, r.sum.value));

    return piece_table_entry_exit_shim(e, obj_arm(r.sub));
}

void abi_DrawTilePiece072A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece072A(cpu.x)));
}

/*
 * @routine 072A:060 TilePieceInRange072A
 * @run 70:A860
 * WHAT IT DOES: Decides from the object's position whether its piece is drawn: with a high X it is; with high X 0 and high Y 0 it goes straight on to the table entry
 * (DrawTilePiece072A from $A890); high Y $FF draws it from low Y $C0 up, high Y 1 up to low Y $3F, and any other high Y not at all.
 * HOW: Compares of the position bytes, then DrawTilePiece072A or its second part.
 * WAITS: no.
 * Entry: X object. Exit: not drawn: the last compare; else as DrawTilePiece072A.
 */
ObjArm TilePieceInRange072A(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->x_hi[slot]);
    if (r.first == 0) {
        r.second = RD8(OBJ->y_hi[slot]);
        if (r.second == 0) {
            r.arm = TI_ENTRY;
            r.sub = obj_sub(piece_table_entry(slot));
            return r;
        }
        if (r.second == 0xFF) {
            r.third = RD8(OBJ->y_lo[slot]);
            r.arm = TI_FF_STAY;
            if (r.third < 0xC0) {
                return r;
            }
        } else {
            r.arm = TI_OFF;
            if (r.second != 1) {
                r.target = 0xA8ED;
                return r;
            }
            r.third = RD8(OBJ->y_lo[slot]);
            r.arm = TI_ONE_STAY;
            if (r.third >= 0x40) {
                return r;
            }
        }
    }
    r.arm = TI_DRAW;
    r.sub2 = obj_sub(draw_tile_piece_at(slot, piece_tables_loaded()));
    return r;
}

static Jump tile_piece_in_range072a_exit_shim(ObjArm r)
{
    Jump jump = { ended_kept(), 0, false };
    Ended e = ended_load(r.first);

    if (r.arm == TI_DRAW && r.first != 0) {
        return draw_tile_piece072a_exit_shim(obj_arm(r.sub2));
    }
    e = then(e, ended_load(r.second));
    if (r.arm == TI_ENTRY) {
        return piece_table_entry_exit_shim(e, obj_arm(r.sub));
    }
    e = then(e, ended_compare(r.second, 0xFF));
    if (r.second == 0xFF) {
        e = then(e, ended_load(r.third));
        e = then(e, ended_compare(r.third, 0xC0));
        if (r.arm == TI_FF_STAY) {
            jump.end = e;
            return jump;
        }
        return draw_tile_piece072a_exit_shim(obj_arm(r.sub2));
    }
    e = then(e, ended_compare(r.second, 0x01));
    if (r.arm == TI_OFF) {
        jump.end = e;       /* the branch to $A8ED, the RTS: the routine returns */
        return jump;
    }
    e = then(e, ended_load(r.third));
    e = then(e, ended_compare(r.third, 0x40));
    if (r.arm == TI_ONE_STAY) {
        jump.end = e;
        return jump;
    }
    return draw_tile_piece072a_exit_shim(obj_arm(r.sub2));
}

void abi_TilePieceInRange072A(void)
{
    abi_jump_exit(tile_piece_in_range072a_exit_shim(TilePieceInRange072A(cpu.x)));
}

/*
 * @routine 072A:01D TilePieceObject072A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script. In state 0 the object is cleared when the bit its OBJ_START_ARG names is set in zField8F; otherwise it loads
 * its record, takes OBJ_WORK_E as OBJ_START_ARG times 16, copies two bytes of the table at $AE95 into its hit box and draws the tile script the table's third byte (plus
 * OBJ_WORK_D) names. In later states TilePieceInRange072A.
 * HOW: BitMaskFromIndex, ClearObject, LoadObjectRecord, DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: as the draw, as ClearObject, or as TilePieceInRange072A.
 */
/* TilePieceObject072A's body with the overlay's tile piece records at `records`, for the stages whose copies differ
 * only there. */
static PieceObjArm tile_piece_object_at(uint8_t slot, uint16_t records)
{
    PieceObjArm p = { { 0 } };
    ObjArm *r = &p.arm;
    uint8_t state, y, mask, field, index, item;
    bool carry;
    uint8_t nz;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r->arm = TO_RANGE;
        p.range = TilePieceInRange072A(slot);
        return p;
    }
    y = RD8(OBJ->f3040[slot]);
    jsr_push(0xA828);
    mask = BitMaskFromIndex072A(y);
    jsr_pop();
    field = RD8(ZP->field8f);
    r->first = (uint8_t)(mask & field);
    r->arm = TO_CLEARED;
    if (r->first != 0) {
        ClearObject(slot);
        return p;
    }
    jsr_push(0xA832);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    index = RD8(OBJ->f3040[slot]);
    index = (uint8_t)(index << 4);
    WR8(OBJ->work_e[slot], index);
    y = index;
    WR8(OBJ->f3988[slot], rd8((uint16_t)(records + 0 + y)));
    WR8(OBJ->f3a38[slot], rd8((uint16_t)(records + 1 + y)));
    item = rd8((uint16_t)(records + 2 + y));
    r->sum = add_bytes(item, RD8(OBJ->f34b8[slot]), false);
    jsr_push(0xA854);
    r->run = DrawTileScript(r->sum.value, 0);
    jsr_pop();
    r->arm = TO_DRAWN;
    return p;
}

PieceObjArm TilePieceObject072A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xAE95);
}

static Jump tile_piece_object072a_exit_shim(PieceObjArm p)
{
    Jump jump = { ended_kept(), 0, false };

    if (p.arm.arm == TO_RANGE) {
        return tile_piece_in_range072a_exit_shim(p.range);
    }
    if (p.arm.arm == TO_CLEARED) {
        jump.end = ended_with_carry(ended_with_y(ended_load(p.arm.first), 0), true);
        return jump;
    }
    jump.end = then(of_sum(p.arm.sum), ended_of_tile_run(p.arm.run, p.arm.sum.value));
    return jump;
}

void abi_TilePieceObject072A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject072A(cpu.x)));
}

/* TilePieceObject072A's body and its exit, for the copies filed in object/tile_piece_b.c. */
PieceObjArm TilePieceObjectAt(uint8_t slot, uint16_t records)
{
    return tile_piece_object_at(slot, records);
}

void tile_piece_object_at_exit_shim(PieceObjArm p)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(p));
}

/* DrawTilePiece in the other 23 overlays that hold it at $A882: the same code, its own three tables. */
/*
 * @routine 025A:082 DrawTilePiece025A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 025A: draws the object's tile script (the item $AE9D gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AE9A at the row $AE9E gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece025A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE9D, 0xAE9E, 0xAE9A });
}

void abi_DrawTilePiece025A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece025A(cpu.x)));
}

/*
 * @routine 051A:082 DrawTilePiece051A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 051A: draws the object's tile script (the item $B081 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B0DE at the row $B082 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece051A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB081, 0xB082, 0xB0DE });
}

void abi_DrawTilePiece051A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece051A(cpu.x)));
}

/*
 * @routine 05CA:082 DrawTilePiece05CA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 05CA: draws the object's tile script (the item $AE63 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AE70 at the row $AE64 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece05CA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE63, 0xAE64, 0xAE70 });
}

void abi_DrawTilePiece05CA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece05CA(cpu.x)));
}

/*
 * @routine 07DA:082 DrawTilePiece07DA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 07DA: draws the object's tile script (the item $D39D gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $D3CA at the row $D39E gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece07DA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xD39D, 0xD39E, 0xD3CA });
}

void abi_DrawTilePiece07DA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece07DA(cpu.x)));
}

/*
 * @routine 088A:082 DrawTilePiece088A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 088A: draws the object's tile script (the item $C224 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $C231 at the row $C225 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece088A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xC224, 0xC225, 0xC231 });
}

void abi_DrawTilePiece088A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece088A(cpu.x)));
}

/*
 * @routine 093A:082 DrawTilePiece093A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 093A: draws the object's tile script (the item $B3AA gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B3B7 at the row $B3AB gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece093A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB3AA, 0xB3AB, 0xB3B7 });
}

void abi_DrawTilePiece093A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece093A(cpu.x)));
}

/*
 * @routine 09EA:082 DrawTilePiece09EA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 09EA: draws the object's tile script (the item $BCE4 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $BCF1 at the row $BCE5 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece09EA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xBCE4, 0xBCE5, 0xBCF1 });
}

void abi_DrawTilePiece09EA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece09EA(cpu.x)));
}

/*
 * @routine 0A9A:082 DrawTilePiece0A9A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0A9A: draws the object's tile script (the item $B67A gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B6C7 at the row $B67B gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0A9A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB67A, 0xB67B, 0xB6C7 });
}

void abi_DrawTilePiece0A9A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0A9A(cpu.x)));
}

/*
 * @routine 0B4A:082 DrawTilePiece0B4A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0B4A: draws the object's tile script (the item $B371 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B36E at the row $B372 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0B4A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB371, 0xB372, 0xB36E });
}

void abi_DrawTilePiece0B4A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0B4A(cpu.x)));
}

/*
 * @routine 0BFA:082 DrawTilePiece0BFA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0BFA: draws the object's tile script (the item $B039 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B0A6 at the row $B03A gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0BFA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB039, 0xB03A, 0xB0A6 });
}

void abi_DrawTilePiece0BFA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0BFA(cpu.x)));
}

/*
 * @routine 0CAA:082 DrawTilePiece0CAA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0CAA: draws the object's tile script (the item $AE9F gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AEBC at the row $AEA0 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0CAA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE9F, 0xAEA0, 0xAEBC });
}

void abi_DrawTilePiece0CAA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0CAA(cpu.x)));
}

/*
 * @routine 0D5A:082 DrawTilePiece0D5A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0D5A: draws the object's tile script (the item $B01E gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B07B at the row $B01F gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0D5A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB01E, 0xB01F, 0xB07B });
}

void abi_DrawTilePiece0D5A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0D5A(cpu.x)));
}

/*
 * @routine 0E0A:082 DrawTilePiece0E0A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0E0A: draws the object's tile script (the item $AEF6 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AF03 at the row $AEF7 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0E0A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAEF6, 0xAEF7, 0xAF03 });
}

void abi_DrawTilePiece0E0A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0E0A(cpu.x)));
}

/*
 * @routine 0EBA:082 DrawTilePiece0EBA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0EBA: draws the object's tile script (the item $AF1F gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AF3C at the row $AF20 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0EBA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAF1F, 0xAF20, 0xAF3C });
}

void abi_DrawTilePiece0EBA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0EBA(cpu.x)));
}

/*
 * @routine 0F6A:082 DrawTilePiece0F6A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 0F6A: draws the object's tile script (the item $ADBF gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $ADBC at the row $ADC0 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece0F6A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xADBF, 0xADC0, 0xADBC });
}

void abi_DrawTilePiece0F6A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece0F6A(cpu.x)));
}

/*
 * @routine 101A:082 DrawTilePiece101A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 101A: draws the object's tile script (the item $B019 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B046 at the row $B01A gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece101A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB019, 0xB01A, 0xB046 });
}

void abi_DrawTilePiece101A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece101A(cpu.x)));
}

/*
 * @routine 10CA:082 DrawTilePiece10CA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 10CA: draws the object's tile script (the item $AEF6 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AF03 at the row $AEF7 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece10CA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAEF6, 0xAEF7, 0xAF03 });
}

void abi_DrawTilePiece10CA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece10CA(cpu.x)));
}

/*
 * @routine 117A:082 DrawTilePiece117A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 117A: draws the object's tile script (the item $B075 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B0B2 at the row $B076 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece117A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB075, 0xB076, 0xB0B2 });
}

void abi_DrawTilePiece117A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece117A(cpu.x)));
}

/*
 * @routine 12DA:082 DrawTilePiece12DA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 12DA: draws the object's tile script (the item $AEE1 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AEFE at the row $AEE2 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece12DA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAEE1, 0xAEE2, 0xAEFE });
}

void abi_DrawTilePiece12DA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece12DA(cpu.x)));
}

/*
 * @routine 13FA:082 DrawTilePiece13FA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 13FA: draws the object's tile script (the item $AEA9 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AEC6 at the row $AEAA gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece13FA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAEA9, 0xAEAA, 0xAEC6 });
}

void abi_DrawTilePiece13FA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece13FA(cpu.x)));
}

/*
 * @routine 161A:082 DrawTilePiece161A
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 161A: draws the object's tile script (the item $B0A5 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $B0F2 at the row $B0A6 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece161A(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xB0A5, 0xB0A6, 0xB0F2 });
}

void abi_DrawTilePiece161A(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece161A(cpu.x)));
}

/*
 * @routine 16CA:082 DrawTilePiece16CA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 16CA: draws the object's tile script (the item $AE7F gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AEBC at the row $AE80 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece16CA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE7F, 0xAE80, 0xAEBC });
}

void abi_DrawTilePiece16CA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece16CA(cpu.x)));
}

/*
 * @routine 18DA:082 DrawTilePiece18DA
 * @run 70:A882
 * @stop A8A8
 * WHAT IT DOES: DrawTilePiece072A in the overlay at sector 18DA: draws the object's tile script (the item $AE87 gives by
 * OBJ_WORK_E, plus OBJ_WORK_D), then looks in $AEC4 at the row $AE88 gives plus OBJ_HIT_TAKEN and goes on to $A8A8 or $A8ED.
 * HOW: draw_tile_piece_at with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DrawTilePiece072A.
 */
ObjArm DrawTilePiece18DA(uint8_t slot)
{
    return draw_tile_piece_at(slot, (PieceTables){ 0xAE87, 0xAE88, 0xAEC4 });
}

void abi_DrawTilePiece18DA(void)
{
    abi_jump_exit(draw_tile_piece072a_exit_shim(DrawTilePiece18DA(cpu.x)));
}

/*
 * @routine 025A:01D TilePieceObject025A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 025A: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $AE9B.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject025A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xAE9B);
}

void abi_TilePieceObject025A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject025A(cpu.x)));
}

/*
 * @routine 051A:01D TilePieceObject051A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 051A: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $B07F.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject051A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xB07F);
}

void abi_TilePieceObject051A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject051A(cpu.x)));
}

/*
 * @routine 07DA:01D TilePieceObject07DA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 07DA: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $D39B.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject07DA(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xD39B);
}

void abi_TilePieceObject07DA(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject07DA(cpu.x)));
}

/*
 * @routine 093A:01D TilePieceObject093A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 093A: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $B3A8.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject093A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xB3A8);
}

void abi_TilePieceObject093A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject093A(cpu.x)));
}

/*
 * @routine 0A9A:01D TilePieceObject0A9A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 0A9A: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $B678.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0A9A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xB678);
}

void abi_TilePieceObject0A9A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject0A9A(cpu.x)));
}

/*
 * @routine 0BFA:01D TilePieceObject0BFA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 0BFA: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $B037.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0BFA(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xB037);
}

void abi_TilePieceObject0BFA(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject0BFA(cpu.x)));
}

/*
 * @routine 0D5A:01D TilePieceObject0D5A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: TilePieceObject072A in the overlay at sector 0D5A: a piece of the scenery drawn from a tile script,
 * with this overlay's tile piece records at $B01C.
 * HOW: tile_piece_object_at with this overlay's records.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0D5A(uint8_t slot)
{
    return tile_piece_object_at(slot, 0xB01C);
}

void abi_TilePieceObject0D5A(void)
{
    abi_jump_exit(tile_piece_object072a_exit_shim(TilePieceObject0D5A(cpu.x)));
}
