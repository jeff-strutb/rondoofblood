/*
 * TEMPORARY: the stage 06E1 enemies and their helpers (disc sectors 0729 to 072D), by routine until
 * each enemy kind is identified. Bodies return how the routine's last instructions left A, Y and the
 * flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

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
Ended WrapVerticalDrift(uint8_t slot)
{
    uint8_t high, low;
    Ended e;

    high = RD8(OBJ->y_hi[slot]);
    e = ended_compare(high, 0xFF);
    if (high == 0xFF) {
        low = RD8(OBJ->y_lo[slot]);
        e = ended_compare(low, 0x80);
        if (low >= 0x80) {
            return e;
        }
        WR8(OBJ->y_hi[slot], 1);
        return then(e, ended_load(1));
    }
    e = ended_compare(high, 1);
    if (high == 1) {
        low = RD8(OBJ->y_lo[slot]);
        e = ended_compare(low, 0x81);
        if (low < 0x81) {
            return e;
        }
        WR8(OBJ->y_hi[slot], 0xFF);
        return then(e, ended_load(0xFF));
    }
    return e;
}

void abi_WrapVerticalDrift(void)
{
    abi_ended(WrapVerticalDrift(cpu.x));
}

/* A drifting block's move: ApplyVelocityY, then the wrap, each in its frame. */
static Ended move_and_wrap(uint8_t slot, uint16_t ret_move, uint16_t ret_wrap)
{
    AxisMove move;
    Ended e, wrap;

    jsr_push(ret_move);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    jsr_push(ret_wrap);
    wrap = WrapVerticalDrift(slot);
    jsr_pop();
    return then(e, wrap);
}

/*
 * @routine 072B:14C MoveAndWrapDriftA
 * @run 70:B14C
 * WHAT IT DOES: The later frames of RisingDriftBlock: moves by the Y speed and wraps the position.
 * HOW: ApplyVelocityY, WrapVerticalDrift.
 * WAITS: no.
 * Entry: X object. Exit: the move's flags with the wrap's compare or load over them.
 */
Ended MoveAndWrapDriftA(uint8_t slot)
{
    return move_and_wrap(slot, 0xB14F, 0xB152);
}

void abi_MoveAndWrapDriftA(void)
{
    abi_ended(MoveAndWrapDriftA(cpu.x));
}

/*
 * @routine 072B:1AA MoveAndWrapDriftB
 * @run 70:B1AA
 * WHAT IT DOES: The later frames of LevelDriftBlock: as MoveAndWrapDriftA.
 * HOW: ApplyVelocityY, WrapVerticalDrift.
 * WAITS: no.
 * Entry and exit: as MoveAndWrapDriftA.
 */
Ended MoveAndWrapDriftB(uint8_t slot)
{
    return move_and_wrap(slot, 0xB1AD, 0xB1B0);
}

void abi_MoveAndWrapDriftB(void)
{
    abi_ended(MoveAndWrapDriftB(cpu.x));
}

/* A drifting block's start: record loaded, frame, start height from the table by next kind, speed. */
static Ended drift_start(uint8_t slot, uint8_t frame, uint16_t table, uint8_t vy_hi, uint16_t ret_load)
{
    uint8_t kind, index, byte, nz;
    bool carry;
    Ended e;

    jsr_push(ret_load);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->frame[slot], frame);
    kind = RD8(OBJ->next_kind[slot]);
    index = (uint8_t)(kind << 1);
    byte = rd8((uint16_t)(table + 1 + index));
    WR8(OBJ->y_hi[slot], byte);
    byte = rd8((uint16_t)(table + index));
    WR8(OBJ->y_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], vy_hi);
    WR8(OBJ->vy_lo[slot], 0x80);
    e = ended_with_y(ended_load(0x80), index);
    return ended_with_carry(e, (kind & 0x80) != 0);
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
Ended RisingDriftBlock(uint8_t slot)
{
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return MoveAndWrapDriftA(slot);
    }
    return drift_start(slot, 0x12, 0xB144, 0xFF, 0xB123);
}

void abi_RisingDriftBlock(void)
{
    abi_ended(RisingDriftBlock(cpu.x));
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
Ended LevelDriftBlock(uint8_t slot)
{
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return MoveAndWrapDriftB(slot);
    }
    return drift_start(slot, 0x13, 0xB1A2, 0x00, 0xB181);
}

void abi_LevelDriftBlock(void)
{
    abi_ended(LevelDriftBlock(cpu.x));
}

/* The start both falling pieces share: record loaded, speed $xx80, frame. */
static Ended piece_start(uint8_t slot, uint8_t vy_hi, uint8_t frame, uint16_t ret)
{
    uint8_t nz;
    bool carry;

    jsr_push(ret);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], vy_hi);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->frame[slot], frame);
    return ended_with_carry(ended_with_y(ended_load(frame), 11), carry);
}

/*
 * @routine 072B:1C9 StartRisingPiece
 * @run 70:B1C9
 * WHAT IT DOES: Starts a piece that rises: record loaded, Y speed $FF80, frame $14.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $14, Y 11, C from the record, N and Z from $14.
 */
Ended StartRisingPiece(uint8_t slot)
{
    return piece_start(slot, 0xFF, 0x14, 0xB1CC);
}

void abi_StartRisingPiece(void)
{
    abi_ended(StartRisingPiece(cpu.x));
}

/*
 * @routine 072B:264 StartLevelPiece
 * @run 70:B264
 * WHAT IT DOES: Starts a piece that hangs: record loaded, Y speed $0080, frame $15.
 * HOW: LoadObjectRecord and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $15, Y 11, C from the record, N and Z from $15.
 */
Ended StartLevelPiece(uint8_t slot)
{
    return piece_start(slot, 0x00, 0x15, 0xB267);
}

void abi_StartLevelPiece(void)
{
    abi_ended(StartLevelPiece(cpu.x));
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
Ended DropPieceEvery32(uint8_t slot)
{
    uint8_t wave, masked, last, found_slot, placed;
    bool found;
    Ended e;

    wave = RD8(ZP->wave_counter);
    masked = (uint8_t)(wave & 0x1F);
    e = ended_load(masked);
    if (masked != 0) {
        return e;
    }
    jsr_push(0xB211);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found);
    if (!found) {
        return e;
    }
    jsr_push(0xB218);
    placed = StartObjectAtPlace(0xAD, slot, found_slot);
    jsr_pop();
    return then(e, ended_load(placed));
}

void abi_DropPieceEvery32(void)
{
    abi_ended(DropPieceEvery32(cpu.x));
}

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
Contact ContactWithPlayer(uint8_t slot)
{
    Contact result = { false, { 0 } };
    SideResult side;
    uint8_t sides, character, speed, offset, facing, height, byte, depth, a;
    uint8_t dist_lo, dist_hi, v_lo, v_hi, band_low, band_high, width;
    Sum sum, x_sum, y_sum;
    Ended e = { 0 };

    jsr_push(0xAA38);
    side = SideOfPlayer(slot);
    jsr_pop();
    sides = side.sides;
    e = then(e, ended_of_side(side));
    WR8(ZP->temp0e, sides);
    character = RD8(ZP->character);
    e = then(e, ended_load(character));
    if (character != 0) {
        width = RD8(ZP->temp08);
        sum = sub_bytes(width, 4, true);
        WR8(ZP->temp08, sum.value);
        e = then(e, of_sum(sum));
    }
    speed = RD8(OBJ->vy_hi[0]);
    e = then(e, ended_load(speed));
    if ((speed & 0x80) != 0) {
        result.end = ended_with_carry(e, false);
        return result;
    }
    offset = RD8(OBJ->hitbox_dx[slot]);
    WR8(ZP->temp0d, offset);
    e = then(e, ended_load(offset));
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing != 0) {
        offset = RD8(ZP->temp0d);
        e = then(e, ended_load(offset));
        offset = (uint8_t)(offset ^ 0xFF);
        e = then(e, ended_load(offset));
        offset = (uint8_t)(offset + 1);
        WR8(ZP->temp0d, offset);
        e = then(e, ended_load(offset));
    }
    height = RD8(ZP->temp09);
    sum = add_bytes(height, 5, false);
    WR8(ZP->temp0a, sum.value);
    e = then(e, of_sum(sum));
    height = RD8(ZP->temp09);
    sum = sub_bytes(height, 4, true);
    WR8(ZP->temp0b, sum.value);
    e = then(e, of_sum(sum));
    byte = RD8(OBJ->x_lo[slot]);
    offset = RD8(ZP->temp0d);
    x_sum = add_bytes(byte, offset, false);
    v_lo = RD8(OBJ->x_lo[0]);
    x_sum = sub_bytes(x_sum.value, v_lo, true);
    WR8(ZP->temp00, x_sum.value);
    e = then(e, of_sum(x_sum));
    byte = RD8(OBJ->x_hi[slot]);
    v_hi = RD8(OBJ->x_hi[0]);
    y_sum = sub_bytes(byte, v_hi, x_sum.carry);
    WR8(ZP->temp01, y_sum.value);
    e = then(e, of_sum(y_sum));
    if ((y_sum.value & 0x80) != 0) {
        a = RD8(ZP->temp00);
        sum = add_bytes((uint8_t)(a ^ 0xFF), 1, false);
        WR8(ZP->temp00, sum.value);
        e = then(e, of_sum(sum));
        a = RD8(ZP->temp01);
        sum = add_bytes((uint8_t)(a ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp01, sum.value);
        e = then(e, of_sum(sum));
    }
    dist_hi = RD8(ZP->temp01);
    e = then(e, ended_load(dist_hi));
    if (dist_hi != 0) {
        result.end = ended_with_carry(e, false);
        return result;
    }
    width = RD8(ZP->temp08);
    dist_lo = RD8(ZP->temp00);
    e = then(e, ended_compare(width, dist_lo));
    if (width < dist_lo) {
        result.end = ended_with_carry(e, false);
        return result;
    }
    byte = RD8(OBJ->y_lo[slot]);
    v_lo = RD8(OBJ->y_lo[0]);
    x_sum = sub_bytes(byte, v_lo, true);
    WR8(ZP->temp02, x_sum.value);
    e = then(e, of_sum(x_sum));
    byte = RD8(OBJ->y_hi[slot]);
    v_hi = RD8(OBJ->y_hi[0]);
    y_sum = sub_bytes(byte, v_hi, x_sum.carry);
    WR8(ZP->temp01, y_sum.value);
    e = then(e, of_sum(y_sum));
    if ((y_sum.value & 0x80) != 0) {
        a = RD8(ZP->temp02);
        sum = add_bytes((uint8_t)(a ^ 0xFF), 1, false);
        WR8(ZP->temp02, sum.value);
        e = then(e, of_sum(sum));
        a = RD8(ZP->temp01);
        sum = add_bytes((uint8_t)(a ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp01, sum.value);
        e = then(e, of_sum(sum));
    }
    dist_hi = RD8(ZP->temp01);
    e = then(e, ended_load(dist_hi));
    if (dist_hi != 0) {
        result.end = ended_with_carry(e, false);
        return result;
    }
    a = RD8(ZP->temp0e);
    byte = (uint8_t)(a & 0x02);
    e = then(e, ended_load(byte));
    if (byte != 0) {
        a = RD8(ZP->temp02);
        height = RD8(ZP->temp09);
        e = then(e, ended_compare(a, height));
        if (a >= height) {
            result.end = ended_with_carry(e, false);
            return result;
        }
    } else {
        a = RD8(ZP->temp02);
        band_low = RD8(ZP->temp0b);
        e = then(e, ended_compare(a, band_low));
        if (a >= band_low) {
            band_high = RD8(ZP->temp0a);
            e = then(e, ended_compare(a, band_high));
            if (a >= band_high) {
                result.end = ended_with_carry(e, false);
                return result;
            }
            /* on top of the object: the bit 0 path */
            dist_lo = RD8(ZP->temp00);
            width = RD8(ZP->temp08);
            e = then(e, ended_compare(dist_lo, width));
            if (dist_lo == width) {
                result.end = ended_with_carry(e, false);
                return result;
            }
            wr8(PlayerAnchorObject, slot);
            byte = RD8(OBJ->y_lo[slot]);
            v_lo = RD8(OBJ->y_lo[0]);
            x_sum = sub_bytes(byte, v_lo, true);
            WR8(ZP->temp02, x_sum.value);
            e = then(e, of_sum(x_sum));
            a = RD8(ZP->player_push);
            a = (uint8_t)(a | 0x01);
            WR8(ZP->player_push, a);
            e = then(e, ended_load(a));
            height = RD8(ZP->temp09);
            depth = RD8(ZP->temp02);
            sum = sub_bytes(height, depth, true);
            WR8(ZP->field_ca, sum.value);
            e = then(e, of_sum(sum));
            result.contact = true;
            result.end = ended_with_carry(e, true);
            return result;
        }
    }
    /* the side: the width left after the distance, and the push */
    width = RD8(ZP->temp08);
    dist_lo = RD8(ZP->temp00);
    sum = sub_bytes(width, dist_lo, true);
    WR8(ZP->temp08, sum.value);
    e = then(e, of_sum(sum));
    a = RD8(ZP->temp0e);
    byte = (uint8_t)(a & 0x01);
    e = then(e, ended_load(byte));
    if (byte == 0) {
        a = RD8(ZP->player_push);
        a = (uint8_t)(a | 0x02);
        WR8(ZP->player_push, a);
        e = then(e, ended_load(a));
        width = RD8(ZP->temp08);
        e = then(e, ended_compare(width, 3));
        if (width >= 3) {
            WR8(ZP->player_push_depth, 2);
            e = then(e, ended_load(2));
        } else {
            WR8(ZP->player_push_depth, width);
        }
        result.end = ended_with_carry(e, false);
        return result;
    }
    a = RD8(ZP->player_push);
    a = (uint8_t)(a | 0x04);
    WR8(ZP->player_push, a);
    e = then(e, ended_load(a));
    width = RD8(ZP->temp08);
    e = then(e, ended_compare(width, 3));
    if (width >= 3) {
        WR8(ZP->player_push_depth, 5);
        result.end = ended_with_carry(then(e, ended_load(5)), false);
        return result;
    }
    width = RD8(ZP->temp08);
    sum = sub_bytes(7, width, true);
    WR8(ZP->player_push_depth, sum.value);
    result.end = ended_with_carry(then(e, of_sum(sum)), false);
    return result;
}

void abi_ContactWithPlayer(void)
{
    abi_ended(ContactWithPlayer(cpu.x).end);
}

/* The test every piece makes: width $14 and height $1D set, the contact asked. */
static Contact piece_contact(uint8_t slot, uint16_t ret)
{
    Contact contact;

    WR8(ZP->temp08, 0x14);
    WR8(ZP->temp09, 0x1D);
    jsr_push(ret);
    contact = ContactWithPlayer(slot);
    jsr_pop();
    return contact;
}

/* The frames of the piece's list at $B203, stepped. */
static Ended piece_frames(uint8_t slot, uint16_t ret)
{
    FrameStep step;
    uint8_t y = 0;

    WR8(ZP->temp10, 0x03);
    WR8(ZP->temp11, 0xB2);
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    return ended_of_frame_step(step, step != FRAME_HELD, y);
}

/* A piece on contact pushes the player along with it. */
static Ended piece_push(uint8_t slot)
{
    uint8_t push, speed_hi, speed_lo;

    push = RD8(ZP->player_push);
    push = (uint8_t)(push | 0x21);
    WR8(ZP->player_push, push);
    speed_hi = RD8(OBJ->vy_hi[slot]);
    WR8(ZP->push_y.hi, speed_hi);
    speed_lo = RD8(OBJ->vy_lo[slot]);
    WR8(ZP->push_y.lo, speed_lo);
    return ended_load(speed_lo);
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
Ended StepPieceOnParent(uint8_t slot)
{
    Contact contact;
    Sum from;
    uint8_t placed, state;
    Ended e;

    e = piece_frames(slot, 0xB1E7);
    jsr_push(0xB1EA);
    e = then(e, DropPieceEvery32(slot));
    jsr_pop();
    from = sub_bytes(slot, 8, true);
    e = then(e, ended_with_y(of_sum(from), from.value));
    jsr_push(0xB1F2);
    placed = CopyObjectPlaceBack(from.value, slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    contact = piece_contact(slot, 0xB1FD);
    e = then(e, contact.end);
    if (!contact.contact) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_StepPieceOnParent(void)
{
    abi_ended(StepPieceOnParent(cpu.x));
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
Ended FollowParentUntilTouched(uint8_t slot)
{
    Contact contact;
    Sum from;
    uint8_t placed, state;
    Ended e;

    from = sub_bytes(slot, 8, true);
    e = then(e, ended_with_y(of_sum(from), from.value));
    jsr_push(0xB27F);
    placed = CopyObjectPlaceBack(from.value, slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    contact = piece_contact(slot, 0xB28A);
    e = then(e, contact.end);
    if (!contact.contact) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_FollowParentUntilTouched(void)
{
    abi_ended(FollowParentUntilTouched(cpu.x));
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
Ended FallingPieceMove(uint8_t slot)
{
    Contact contact;
    AxisMove move;
    uint8_t state;
    Ended e;

    jsr_push(0xB293);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    contact = piece_contact(slot, 0xB29E);
    e = then(e, contact.end);
    if (!contact.contact) {
        state = DEC8(OBJ->state[slot]);
        return then(e, ended_nz(state));
    }
    return then(e, piece_push(slot));
}

void abi_FallingPieceMove(void)
{
    abi_ended(FallingPieceMove(cpu.x));
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
Ended FallingPieceStep(uint8_t slot)
{
    Contact contact;
    AxisMove move;
    uint8_t state;
    Ended e;

    e = piece_frames(slot, 0xB224);
    jsr_push(0xB227);
    e = then(e, DropPieceEvery32(slot));
    jsr_pop();
    jsr_push(0xB22A);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    contact = piece_contact(slot, 0xB235);
    e = then(e, contact.end);
    if (!contact.contact) {
        state = DEC8(OBJ->state[slot]);
        return then(e, ended_nz(state));
    }
    return then(e, piece_push(slot));
}

void abi_FallingPieceStep(void)
{
    abi_ended(FallingPieceStep(cpu.x));
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
Ended HitboxByVerticalDirection(uint8_t slot)
{
    uint8_t speed, index, byte;
    Ended e;

    speed = RD8(OBJ->vy_hi[slot]);
    index = (uint8_t)(speed >> 7);
    byte = rd8((uint16_t)(0xB9CD + index));
    WR8(OBJ->hitbox_on[slot], byte);
    e = ended_with_y(ended_load(byte), index);
    return ended_with_carry(e, (speed & 0x40) != 0);
}

void abi_HitboxByVerticalDirection(void)
{
    abi_ended(HitboxByVerticalDirection(cpu.x));
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
Ended LiftObjectsBySlowFraction(uint8_t slot)
{
    uint8_t other, frac, low, high;
    Sum sum, last = { 0, false, false };

    for (other = 0x10; other < 0x27; other++) {
        frac = RD8(OBJ->y_frac[slot]);
        sum = add_bytes(frac, 0xE0, false);
        WR8(OBJ->y_frac[slot], sum.value);
        low = RD8(OBJ->y_lo[other]);
        sum = add_bytes(low, 0, sum.carry);
        WR8(OBJ->y_lo[other], sum.value);
        high = RD8(OBJ->y_hi[other]);
        last = add_bytes(high, 0, sum.carry);
        WR8(OBJ->y_hi[other], last.value);
    }
    return then(of_sum(last), ended_with_carry(ended_with_y(ended_nz(0), 0x27), true));
}

void abi_LiftObjectsBySlowFraction(void)
{
    abi_ended(LiftObjectsBySlowFraction(cpu.x));
}

/*
 * @routine 072C:0E9 ShiftObjectsX
 * @run 70:B8E9
 * WHAT IT DOES: On every second frame of the object's life timer, moves every object slot from $10 to $26
 * along X by a signed 16-bit step from the table at $B912, picked by the object's OBJ_37D0 (0 or 1).
 * HOW: The table index is kept in X while the loop runs (X is pushed and pulled).
 * WAITS: no.
 * Entry: X object. Exit: odd timer: A 1, N 0, Z 0; else A the last high byte, Y $27, C 1, N and Z from X, V
 * from the last add.
 */
Ended ShiftObjectsX(uint8_t slot)
{
    uint8_t timer, side, index, other, low, high;
    Sum sum, last = { 0, false, false };

    timer = RD8(OBJ->life_timer[slot]);
    timer = (uint8_t)(timer & 0x01);
    if (timer != 0) {
        return ended_load(timer);
    }
    side = RD8(OBJ->f37d0[slot]);
    index = (uint8_t)(side << 1);
    push8(slot);
    for (other = 0x10; other < 0x27; other++) {
        low = RD8(OBJ->x_lo[other]);
        sum = add_bytes(low, rd8((uint16_t)(0xB912 + index)), false);
        WR8(OBJ->x_lo[other], sum.value);
        high = RD8(OBJ->x_hi[other]);
        last = add_bytes(high, rd8((uint16_t)(0xB913 + index)), sum.carry);
        WR8(OBJ->x_hi[other], last.value);
    }
    slot = pull8();
    return then(of_sum(last), ended_with_carry(ended_with_y(ended_nz(slot), 0x27), true));
}

void abi_ShiftObjectsX(void)
{
    abi_ended(ShiftObjectsX(cpu.x));
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
 * (OBJ_3778) with a length of $0400, adds a drift word from the table at $B8E3 by its side (OBJ_37D0),
 * moves, turns its angle on by a byte from the table at $B8E7, and when its X high byte reaches $FE or 2
 * it clears its OBJ_2800 flag.
 * HOW: SetVelocityFromAngle, ApplyVelocity, tables from the disc (bank 70).
 * Untraced: one byte (the second compare's exit).
 * WAITS: no.
 * Entry: X object. Exit: A the X high byte, N, Z and C from the last compare, Y the side, V from the angle's add.
 */
Ended OrbitingObject(uint8_t slot)
{
    uint8_t angle, side, index, step, x_high, turned;
    AxisMove move;
    Sum sum;
    Ended e;

    jsr_push(0xB89F);
    ShiftObjectsX(slot);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 4);
    angle = RD8(OBJ->f3778[slot]);
    jsr_push(0xB8AB);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    side = RD8(OBJ->f37d0[slot]);
    index = (uint8_t)(side << 1);
    speed_x_add(slot, 0xB8E3, index);
    jsr_push(0xB8C6);
    move = ApplyVelocity(slot);
    jsr_pop();
    side = RD8(OBJ->f37d0[slot]);
    angle = RD8(OBJ->f3778[slot]);
    step = rd8((uint16_t)(0xB8E7 + side));
    sum = add_bytes(angle, step, false);
    WR8(OBJ->f3778[slot], sum.value);
    e = ended_with_y(of_sum(sum), side);
    x_high = RD8(OBJ->x_hi[slot]);
    e = then(e, ended_compare(x_high, 0xFE));
    if (x_high != 0xFE) {
        e = then(e, ended_compare(x_high, 0x02));
        if (x_high != 0x02) {
            return e;
        }
    }
    WR8(OBJ->f2800[slot], 0);
    (void)turned;
    (void)move;
    return e;
}

void abi_OrbitingObject(void)
{
    abi_ended(OrbitingObject(cpu.x));
}

/*
 * @routine 072C:242 AccelerateAndSway
 * @run 70:BA42
 * WHAT IT DOES: A thing that falls and sways: its Y speed gains $002C, it moves, its X speed gains a drift
 * word from the table at $BA7C by its side (OBJ_37D0); when its counter OBJ_36C8 runs out it takes
 * OBJ_3670 3 and moves on one OBJ_2800 state.
 * HOW: Adds, ApplyVelocity, DEC of the counter.
 * WAITS: no.
 * Entry: X object. Exit: counter not zero: N and Z from it, C and V from the X speed's add; zero: A 3, N and Z
 * from the new state.
 */
Ended AccelerateAndSway(uint8_t slot)
{
    uint8_t low, high, side, index, left, state;
    AxisMove move;
    Sum sum;
    Ended e;

    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 0x2C, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    jsr_push(0xBA56);
    move = ApplyVelocity(slot);
    jsr_pop();
    side = RD8(OBJ->f37d0[slot]);
    index = (uint8_t)(side << 1);
    sum = speed_x_add(slot, 0xBA7C, index);
    e = ended_with_y(of_sum(sum), index);
    left = DEC8(OBJ->f36c8[slot]);
    e = then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->f3670[slot], 3);
    state = INC8(OBJ->f2800[slot]);
    (void)move;
    return then(e, ended_with_a(ended_nz(state), 3));
}

void abi_AccelerateAndSway(void)
{
    abi_ended(AccelerateAndSway(cpu.x));
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
 * WHAT IT DOES: Steps the animation list at $BA96; when the list ends, sets the counter OBJ_36C8 to $80 and
 * moves on one OBJ_2800 state.
 * HOW: StepObjectFrames, then the stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z from the new state.
 */
Ended StepThenCount(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, state;
    Ended e;

    step = step_list(slot, 0x96, 0xBA, 0xBA8B, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f36c8[slot], 0x80);
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_with_a(ended_nz(state), 0x80));
}

void abi_StepThenCount(void)
{
    abi_ended(StepThenCount(cpu.x));
}

/*
 * @routine 072C:329 StepThenStop
 * @run 70:BB29
 * WHAT IT DOES: Steps the animation list at $BB45; when the list ends, sets OBJ_3670 to 4, stops the Y
 * speed and moves on one OBJ_2800 state.
 * HOW: StepObjectFrames, then the stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 4, N and Z from
 * the new state.
 */
Ended StepThenStop(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, state;
    Ended e;

    step = step_list(slot, 0x45, 0xBB, 0xBB34, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f3670[slot], 4);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_with_a(ended_nz(state), 4));
}

void abi_StepThenStop(void)
{
    abi_ended(StepThenStop(cpu.x));
}

/*
 * @routine 072C:34C FallUntilLevel2
 * @run 70:BB4C
 * WHAT IT DOES: Applies gravity and moves the object down; when its Y high byte is 2 its OBJ_2800 is cleared.
 * HOW: ApplyGravity, ApplyVelocityY, a compare.
 * WAITS: no.
 * Entry: X object. Exit: A the Y high byte, N, Z and C of the compare with 2, V from the move.
 */
Ended FallUntilLevel2(uint8_t slot)
{
    GravityResult gravity;
    AxisMove move;
    uint8_t y_high;
    Ended e;

    jsr_push(0xBB4F);
    gravity = ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xBB52);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_compare(y_high, 2));
    if (y_high == 2) {
        WR8(OBJ->f2800[slot], 0);
    }
    (void)gravity;
    return e;
}

void abi_FallUntilLevel2(void)
{
    abi_ended(FallUntilLevel2(cpu.x));
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
 * WHAT IT DOES: Goes to the code a table at $BDBD names for the stage's step counter OBJ_3670 slot $10.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: nothing. Exit: as the code jumped to; A the high byte, Y the doubled step.
 */
Jump JumpByStageStep(void)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t step, index, low, high;

    step = RD8(OBJ->f3670[0x10]);
    index = (uint8_t)(step << 1);
    low = rd8((uint16_t)(0xBDBD + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xBDBE + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (step & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStageStep(void)
{
    Jump jump = JumpByStageStep();

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072B:42E StartPieceChain
 * @run 70:B42E
 * WHAT IT DOES: Stops the object, clears four 192-byte work tables the overlay keeps at $B46E, $B52E,
 * $B5EE and $B6AE, then starts 22 objects of kind $25 in the slots after this one, each at this object's
 * place, each with its own number 0 to 21 in its life timer (the chain of pieces that follow it).
 * HOW: Clears the four speed bytes and OBJ_2FE8; a loop of 192 stores to each table; a loop of 22 of
 * ClearObjectInY and StartObjectAtPlace. The tables have no names yet.
 * WAITS: no.
 * Entry: X object. Exit: A 21, Y the slot plus 23, N 0, Z 1, C 1; V kept.
 */
Ended StartPieceChain(uint8_t slot)
{
    uint8_t index, count, number, to, placed;

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
        placed = StartObjectAtPlace(0x25, slot, to);
        jsr_pop();
        number = RD8(ZP->temp01);
        WR8(OBJ->life_timer[to], number);
        to++;
        INC8(ZP->temp01);
        count = DEC8(ZP->temp00);
    } while (count != 0);
    (void)placed;
    return ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), number), to), true);
}

void abi_StartPieceChain(void)
{
    abi_ended(StartPieceChain(cpu.x));
}

/*
 * @routine 072C:053 StartPieceStreamA
 * @run 70:B853
 * WHAT IT DOES: Starts a stream of pieces from one of two sides: sound $88, OBJ_3670 1, OBJ_3828 2, frame $10;
 * the side (OBJ_37D0) is a random bit and picks the start position from a table at $B894 (four bytes a side);
 * then StartPieceChain, OBJ_3778 0 and the next OBJ_2800 state.
 * HOW: SoundCommand, NextRandom, tables read from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
 */
Ended StartPieceStreamA(uint8_t slot, uint8_t y)
{
    RandomResult random;
    uint8_t side, index, byte, state;
    Ended e;

    jsr_push(0xB858);
    SoundCommand(0x88, slot, y);
    jsr_pop();
    WR8(OBJ->f3670[slot], 1);
    WR8(OBJ->f3828[slot], 2);
    WR8(OBJ->frame[slot], 0x10);
    jsr_push(0xB86A);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    side = (uint8_t)(random.value & 0x01);
    WR8(OBJ->f37d0[slot], side);
    index = (uint8_t)(side << 2);
    e = then(e, ended_with_carry(ended_with_y(ended_load(index), index), false));
    byte = rd8((uint16_t)(0xB895 + index));
    WR8(OBJ->x_hi[slot], byte);
    byte = rd8((uint16_t)(0xB894 + index));
    WR8(OBJ->x_lo[slot], byte);
    byte = rd8((uint16_t)(0xB897 + index));
    WR8(OBJ->y_hi[slot], byte);
    byte = rd8((uint16_t)(0xB896 + index));
    WR8(OBJ->y_lo[slot], byte);
    e = then(e, ended_with_y(ended_load(byte), index));
    jsr_push(0xB88D);
    e = then(e, StartPieceChain(slot));
    jsr_pop();
    WR8(OBJ->f3778[slot], 0);
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(state));
}

void abi_StartPieceStreamA(void)
{
    abi_ended(StartPieceStreamA(cpu.x, cpu.y));
}

/*
 * @routine 072C:116 StartPieceStreamB
 * @run 70:B916
 * WHAT IT DOES: Starts a stream of pieces from the top: OBJ_3828 2, sound $89, animation reset, X 0 with a
 * random low byte, Y $0110, StartPieceChain, then random X and Y speeds from tables at $B970 and $B978 (two
 * bytes each, four of them) and the next OBJ_2800 state; when the object is on the player's left
 * (SideOfPlayer bit 0) its X speed is reversed.
 * HOW: SoundCommand, NextRandom three times, SideOfPlayer, NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: A the side bits (or the reversed speed's high byte), N and Z from it, C and V from the
 * reversal when made, else C from the side and V from the last random.
 */
Ended StartPieceStreamB(uint8_t slot, uint8_t y)
{
    RandomResult random;
    SideResult side;
    uint8_t picked, index, byte, bits, state;
    Sum negated;
    Ended e;

    WR8(OBJ->f3828[slot], 2);
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
    e = StartPieceChain(slot);
    jsr_pop();
    jsr_push(0xB93F);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    picked = (uint8_t)(random.value & 0x03);
    index = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xB971 + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB970 + index));
    WR8(OBJ->vx_lo[slot], byte);
    jsr_push(0xB952);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    picked = (uint8_t)(random.value & 0x03);
    index = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xB979 + index));
    WR8(OBJ->vy_hi[slot], byte);
    byte = rd8((uint16_t)(0xB978 + index));
    WR8(OBJ->vy_lo[slot], byte);
    e = then(e, ended_with_y(ended_load(byte), index));
    state = INC8(OBJ->f2800[slot]);
    e = then(e, ended_nz(state));
    jsr_push(0xB968);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    bits = (uint8_t)(side.sides & 0x01);
    e = then(e, ended_load(bits));
    if (bits == 0) {
        return e;
    }
    jsr_push(0xB96F);
    negated = NegateVelocityX(slot);
    jsr_pop();
    return then(e, of_sum(negated));
}

void abi_StartPieceStreamB(void)
{
    abi_ended(StartPieceStreamB(cpu.x, cpu.y));
}

/*
 * @routine 072C:180 StepFallingPieceStream
 * @run 70:B980
 * WHAT IT DOES: One frame of a falling piece stream: steps the animation list at $B9B3, sets the hitbox
 * flag by its direction, lifts the other objects (LiftObjectsBySlowFraction), adds $002C to its Y speed, moves,
 * and when its Y high byte reaches 8 clears its flash flag and OBJ_2800.
 * HOW: StepObjectFrames, HitboxByVerticalDirection, LiftObjectsBySlowFraction, ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: A the Y high byte, N, Z and C from its compare with 8,
 * V from the move; Y as the lift left it ($27).
 */
Ended StepFallingPieceStream(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, low, high, y_high;
    AxisMove move;
    Sum sum;
    Ended e;

    step = step_list(slot, 0xB3, 0xB9, 0xB98B, &y);
    jsr_push(0xB98E);
    e = HitboxByVerticalDirection(slot);
    jsr_pop();
    jsr_push(0xB991);
    e = then(e, LiftObjectsBySlowFraction(slot));
    jsr_pop();
    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 0x2C, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    jsr_push(0xB9A5);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_compare(y_high, 8));
    if (y_high == 8) {
        WR8(OBJ->flash_flags[slot], 0);
        WR8(OBJ->f2800[slot], 0);
    }
    (void)step;
    return e;
}

void abi_StepFallingPieceStream(void)
{
    abi_ended(StepFallingPieceStream(cpu.x));
}

/*
 * @routine 072C:1F0 StartPieceStreamC
 * @run 70:B9F0
 * WHAT IT DOES: Starts a stream of pieces from the bottom: OBJ_3828 3, OBJ_3510 0, a random side (OBJ_37D0)
 * picks X from the table at $BA3E and the X speed from $BA40; Y is $0110; StartPieceChain; Y speed $F7;
 * frame $80, animation reset, OBJ_3670 2, OBJ_36C8 $30 and the next OBJ_2800 state.
 * HOW: NextRandom, tables from the disc (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
 */
Ended StartPieceStreamC(uint8_t slot)
{
    RandomResult random;
    uint8_t side, byte, state;
    Ended e;

    WR8(OBJ->f3828[slot], 3);
    WR8(OBJ->f3510[slot], 0);
    jsr_push(0xB9FB);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    side = (uint8_t)(random.value & 0x01);
    WR8(OBJ->f37d0[slot], side);
    byte = rd8((uint16_t)(0xBA3E + side));
    WR8(OBJ->x_lo[slot], byte);
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 1);
    WR8(OBJ->y_lo[slot], 0x10);
    e = then(e, ended_with_y(ended_load(0x10), side));
    jsr_push(0xBA17);
    e = then(e, StartPieceChain(slot));
    jsr_pop();
    side = RD8(OBJ->f37d0[slot]);
    byte = rd8((uint16_t)(0xBA40 + side));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0xF7);
    WR8(OBJ->frame[slot], 0x80);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3670[slot], 2);
    WR8(OBJ->f36c8[slot], 0x30);
    state = INC8(OBJ->f2800[slot]);
    return then(e, ended_with_y(ended_with_a(ended_nz(state), 0x30), side));
}

void abi_StartPieceStreamC(void)
{
    abi_ended(StartPieceStreamC(cpu.x));
}

/*
 * @routine 072C:29F StepPieceEmitter
 * @run 70:BA9F
 * WHAT IT DOES: An object that sheds pieces while it plays its animation (list $BAE6): when the list ends it
 * moves on one OBJ_2800 state; otherwise, every fourth frame of its life timer, once its animation step is
 * $10 or more, on frame $13 and while OBJ_3510 is under 4, it starts an object of kind $26 at its place in the
 * first free slot from $20, copies its facing and gives it the count OBJ_3510, which then goes up one.
 * HOW: StepObjectFrames, FindFreeSlotFrom20, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the last compare, load or INC.
 */
Ended StepPieceEmitter(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, timer, bits, anim, frame, count, last, found_slot, placed, facing, state;
    bool found;
    Ended e;

    step = step_list(slot, 0xE6, 0xBA, 0xBAAA, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        state = INC8(OBJ->f2800[slot]);
        return then(e, ended_nz(state));
    }
    timer = RD8(OBJ->life_timer[slot]);
    bits = (uint8_t)(timer & 0x03);
    e = then(e, ended_load(bits));
    if (bits != 0) {
        return e;
    }
    anim = RD8(OBJ->anim_step[slot]);
    e = then(e, ended_compare(anim, 0x10));
    if (anim < 0x10) {
        return e;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_compare(frame, 0x13));
    if (frame != 0x13) {
        return e;
    }
    count = RD8(OBJ->f3510[slot]);
    e = then(e, ended_compare(count, 4));
    if (count >= 4) {
        return e;
    }
    jsr_push(0xBACB);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found));
    if (!found) {
        return e;
    }
    jsr_push(0xBAD2);
    placed = StartObjectAtPlace(0x26, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    count = RD8(OBJ->f3510[slot]);
    WR8(OBJ->life_timer[found_slot], count);
    count = INC8(OBJ->f3510[slot]);
    return then(e, ended_nz(count));
}

void abi_StepPieceEmitter(void)
{
    abi_ended(StepPieceEmitter(cpu.x));
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
Ended StartOnFirstState(uint8_t slot)
{
    uint8_t state, nz, last;
    bool carry;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return ended_load(state);
    }
    last = LoadObjectRecord(slot, &nz, &carry);
    return ended_of_record(last, nz, carry);
}

void abi_StartOnFirstState(void)
{
    abi_ended(StartOnFirstState(cpu.x));
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
Ended FallingDebris(uint8_t slot)
{
    RandomResult random;
    GravityResult gravity;
    AxisMove move;
    uint8_t state, nz, y_high;
    bool carry;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xB32C);
        LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        jsr_push(0xB32F);
        random = NextRandom();
        jsr_pop();
        sum = add_bytes((uint8_t)(random.value & 0x01), 0x19, false);
        WR8(OBJ->frame[slot], sum.value);
        return ended_with_y(of_sum(sum), 11);
    }
    jsr_push(0xB33B);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    jsr_push(0xB33E);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if (y_high != 0) {
        ClearObject(slot);
    }
    (void)gravity;
    return e;
}

void abi_FallingDebris(void)
{
    abi_ended(FallingDebris(cpu.x));
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
Ended FlashByHeight(uint8_t slot)
{
    uint8_t flash, state, height, index, byte;
    Ended e;

    flash = RD8(OBJ->flash_flags[0x10]);
    e = ended_compare(flash, 0x86);
    if (flash == 0x86) {
        WR8(OBJ->flash_flags[slot], 0x86);
        return then(e, ended_load(0x86));
    }
    state = RD8(OBJ->state[0x10]);
    e = then(e, ended_compare(state, 2));
    if (state != 2) {
        WR8(OBJ->flash_flags[slot], 0);
        return e;
    }
    height = RD8(OBJ->y_lo[slot]);
    index = (uint8_t)(height >> 4);
    byte = rd8((uint16_t)(0xB3CB + index));
    WR8(OBJ->flash_flags[slot], byte);
    e = then(e, ended_with_carry(ended_with_y(ended_load(byte), index), (height & 0x08) != 0));
    return e;
}

void abi_FlashByHeight(void)
{
    abi_ended(FlashByHeight(cpu.x));
}

/*
 * @routine 072B:427 StartWithEvent
 * @run 70:B427
 * WHAT IT DOES: Loads the object's record and records its event (when it is one the save remembers).
 * HOW: LoadObjectRecord, RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as RecordObjectEvent.
 */
Ended StartWithEvent(uint8_t slot)
{
    uint8_t nz, stored, event, byte;
    bool carry, marked;

    uint8_t last;

    jsr_push(0xB42A);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    jsr_push(0xB42D);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    return then(ended_of_record(last, nz, carry), ended_of_object_event(marked, stored, event, byte));
}

void abi_StartWithEvent(void)
{
    abi_ended(StartWithEvent(cpu.x));
}

/*
 * @routine 072B:38D RaiseStateOnFlag
 * @run 70:B38D
 * WHAT IT DOES: When bit 7 of the object's status flags is set, an object in a state below 3 enters state 3 and
 * clears OBJ_2800.
 * HOW: Compares the state with 3.
 * Untraced: every arm past the flag test (B394 to B39A) ran in no script.
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load.
 */
Ended RaiseStateOnFlag(uint8_t slot)
{
    uint8_t flags, state;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    flags = (uint8_t)(flags & 0x80);
    e = ended_load(flags);
    if (flags == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 3));
    if (state >= 3) {
        return e;
    }
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(3));
}

void abi_RaiseStateOnFlag(void)
{
    abi_ended(RaiseStateOnFlag(cpu.x));
}

/*
 * @routine 072C:656 BurstAtHeightE0
 * @run 70:BE56
 * WHAT IT DOES: When the object is on the first screen row and its Y low byte is in $E0 to $EF it plays sound $23 and starts a burst of debris (StartDebrisBurst, timer 0).
 * HOW: Two compares, SoundCommand, a tail jump into StartDebrisBurst.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: no burst: the last compare or load; else as StartDebrisBurst.
 */
SoundEnd BurstAtHeightE0(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t high, low;
    Ended e;

    high = RD8(OBJ->y_hi[slot]);
    e = ended_load(high);
    r.end = e;
    if (high != 0) {
        return r;
    }
    low = RD8(OBJ->y_lo[slot]);
    low = (uint8_t)(low & 0xF0);
    e = then(e, ended_load(low));
    e = then(e, ended_compare(low, 0xE0));
    r.end = e;
    if (low != 0xE0) {
        return r;
    }
    r.before = then(e, ended_load(0x23));
    r.sounded = true;
    r.command = 0x23;
    jsr_push(0xBE69);
    r.call = SoundCommand(0x23, slot, y);
    jsr_pop();
    r.end = StartDebrisBurst(slot, 0, y);
    return r;
}

void abi_BurstAtHeightE0(void)
{
    abi_sound_end(BurstAtHeightE0(cpu.x, cpu.y));
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
ShowEnd ShowByVelocity(uint8_t slot, uint8_t y)
{
    ShowEnd r = { { 0 } };
    uint8_t phase, frame, speed, size;
    Sum diff, sum;
    Ended e;

    jsr_push(0xBDF8);
    r.burst = BurstAtHeightE0(slot, y);
    jsr_pop();
    phase = RD8(OBJ->life_timer[slot]);
    frame = rd8((uint16_t)(0xBE39 + phase));
    WR8(OBJ->frame[slot], frame);
    e = ended_with_y(ended_load(frame), phase);
    diff = sub_bytes(frame, 1, true);
    e = then(e, of_sum(diff));
    e = then(e, ended_with_y(ended_kept(), diff.value));
    size = rd8((uint16_t)(0xBE4F + diff.value));
    WR8(OBJ->hitbox_half_w[slot], size);
    WR8(OBJ->hitbox_half_h[slot], size);
    e = then(e, ended_load(size));
    speed = RD8(OBJ->vx_hi[slot]);
    e = then(e, ended_compare(speed, 2));
    if (speed >= 2) {
        e = then(e, ended_compare(speed, 0xFE));
        if (speed < 0xFE) {
            r.tail = e;
            return r;
        }
    }
    frame = RD8(OBJ->frame[slot]);
    sum = add_bytes(frame, 7, false);
    WR8(OBJ->frame[slot], sum.value);
    r.tail = then(e, of_sum(sum));
    return r;
}

void abi_ShowByVelocity(void)
{
    ShowEnd r = ShowByVelocity(cpu.x, cpu.y);

    abi_sound_end(r.burst);
    abi_ended(r.tail);
}

/* The move a drifting piece makes from the four work tables at $B46E, $B52E, $B5EE and $B6AE, by an index its own table gives. */
static Ended speed_from_tables(uint8_t slot, uint16_t index_table, uint16_t ret, uint8_t *y_out)
{
    AxisMove move;
    uint8_t phase, base, byte;
    Sum sum;
    Ended e;

    phase = RD8(OBJ->life_timer[slot]);
    base = rd8((uint16_t)(index_table + phase));
    e = ended_with_y(ended_load(base), phase);
    sum = add_bytes(base, 4, false);
    e = then(e, ended_with_y(of_sum(sum), sum.value));
    *y_out = sum.value;
    byte = rd8((uint16_t)(0xB46E + sum.value));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB52E + sum.value));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xB5EE + sum.value));
    WR8(OBJ->vy_hi[slot], byte);
    byte = rd8((uint16_t)(0xB6AE + sum.value));
    WR8(OBJ->vy_lo[slot], byte);
    jsr_push(ret);
    move = ApplyVelocity(slot);
    jsr_pop();
    return then(e, ended_of_axis_move(move));
}

/*
 * @routine 072C:5D0 FollowPathAndShow
 * @run 70:BDD0
 * WHAT IT DOES: Takes the object's speed from the four path tables (index from the table at $BE24 by its timer, plus 4), moves it and goes on as ShowByVelocity.
 * HOW: Table reads (bank 70), ApplyVelocity, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ShowByVelocity, over the move's.
 */
ShowEnd FollowPathAndShow(uint8_t slot, uint8_t y)
{
    ShowEnd r = { { 0 } };
    Ended e;

    e = speed_from_tables(slot, 0xBE24, 0xBDF5, &y);
    r = ShowByVelocity(slot, y);
    r.head = e;
    return r;
}

void abi_FollowPathAndShow(void)
{
    ShowEnd r = FollowPathAndShow(cpu.x, cpu.y);

    abi_ended(r.head);
    abi_sound_end(r.burst);
    abi_ended(r.tail);
}

/*
 * @routine 072C:5CD SetHitboxThenFollowPath
 * @run 70:BDCD
 * WHAT IT DOES: Sets the object's hitbox flag by its direction (HitboxByVerticalDirection) and goes on as FollowPathAndShow.
 * HOW: HitboxByVerticalDirection, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
 */
ShowEnd SetHitboxThenFollowPath(uint8_t slot, uint8_t y)
{
    ShowEnd r = { { 0 } };
    Ended e;

    jsr_push(0xBDD0);
    e = HitboxByVerticalDirection(slot);
    jsr_pop();
    r = FollowPathAndShow(slot, y);
    r.first = e;
    return r;
}

void abi_SetHitboxThenFollowPath(void)
{
    ShowEnd r = SetHitboxThenFollowPath(cpu.x, cpu.y);

    abi_ended(r.first);
    abi_ended(r.head);
    abi_sound_end(r.burst);
    abi_ended(r.tail);
}

/*
 * @routine 072C:671 FollowPathB
 * @run 70:BE71
 * WHAT IT DOES: As FollowPathAndShow with the index table at $BE99, going on as ShowByVelocity by a jump.
 * HOW: Table reads (bank 70), ApplyVelocity, a tail jump.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
 */
ShowEnd FollowPathB(uint8_t slot, uint8_t y)
{
    ShowEnd r = { { 0 } };
    Ended e;

    e = speed_from_tables(slot, 0xBE99, 0xBE96, &y);
    r = ShowByVelocity(slot, y);
    r.head = e;
    return r;
}

void abi_FollowPathB(void)
{
    ShowEnd r = FollowPathB(cpu.x, cpu.y);

    abi_ended(r.head);
    abi_sound_end(r.burst);
    abi_ended(r.tail);
}

/*
 * @routine 072C:77B StartDriftingPiece
 * @run 70:BF7B
 * WHAT IT DOES: Loads the object's record, takes its X speed from a table at $BFA2 by its timer, a Y speed of 4, and reverses the X speed with sound $25 when slot $10 faces left.
 * HOW: LoadObjectRecord, NegateVelocityX, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
SoundEnd StartDriftingPiece(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t nz, last, phase, index, byte, facing;
    bool carry;
    Ended e;
    Sum negated;

    jsr_push(0xBF7E);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    phase = RD8(OBJ->life_timer[slot]);
    index = (uint8_t)(phase << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), (phase & 0x80) != 0), index));
    byte = rd8((uint16_t)(0xBFA3 + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xBFA2 + index));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], 4);
    e = then(e, ended_load(4));
    facing = RD8(OBJ->facing_left[0x10]);
    e = then(e, ended_load(facing));
    r.end = e;
    if (facing == 0) {
        return r;
    }
    jsr_push(0xBF9C);
    negated = NegateVelocityX(slot);
    jsr_pop();
    e = then(e, of_sum(negated));
    r.before = then(e, ended_load(0x25));
    r.sounded = true;
    r.command = 0x25;
    jsr_push(0xBFA1);
    r.call = SoundCommand(0x25, slot, index);
    jsr_pop();
    r.end = ended_kept();
    return r;
}

void abi_StartDriftingPiece(void)
{
    abi_sound_end(StartDriftingPiece(cpu.x, cpu.y));
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
BurstTail DropAndBurst(uint8_t slot, uint8_t y)
{
    BurstTail r = { { { 0 }, { 0 }, false, 0, { 0 } }, { 0 } };
    FrameStep step;
    AxisMove move;
    uint8_t low, state;
    Ended e;

    step = step_list(slot, 0xEF, 0xBF, 0xBFB5, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xBFB8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0x7F));
    r.pre.end = e;
    if (low < 0x7F) {
        return r;
    }
    WR8(OBJ->y_lo[slot], 0x7F);
    r.pre.before = then(e, ended_load(0x23));
    r.pre.sounded = true;
    r.pre.command = 0x23;
    jsr_push(0xBFC9);
    r.pre.call = SoundCommand(0x23, slot, y);
    jsr_pop();
    jsr_push(0xBFCD);
    r.pre.end = StartDebrisBurst(slot, 0, y);
    jsr_pop();
    WR8(OBJ->hitbox_half_w[slot], 0x10);
    WR8(OBJ->hitbox_half_h[slot], 0x0C);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0xFF);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    r.tail = ended_with_a(ended_nz(state), 0xFF);
    return r;
}

void abi_DropAndBurst(void)
{
    BurstTail r = DropAndBurst(cpu.x, cpu.y);

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
Ended StepFallAfterFour(uint8_t slot)
{
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    uint8_t y = 0, anim;
    Ended e;

    step = step_list(slot, 0x10, 0xC0, 0xBFFF, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    anim = RD8(OBJ->anim_step[slot]);
    e = then(e, ended_compare(anim, 4));
    if (anim < 4) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    jsr_push(0xC00C);
    gravity = ApplyGravity(slot);
    jsr_pop();
    jsr_push(0xC00F);
    move = ApplyVelocity(slot);
    jsr_pop();
    (void)gravity;
    return then(e, ended_of_axis_move(move));
}

void abi_StepFallAfterFour(void)
{
    abi_ended(StepFallAfterFour(cpu.x));
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
BreakEnd BreakIntoPieces(uint8_t slot, uint8_t y_entry)
{
    BreakEnd r = { { 0 }, { 0 }, false, { 0 }, 0, { 0 } };
    uint8_t state, nz, last, hit, stored, event, byte, to, row, placed, number;
    bool carry, marked;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xB2BD);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        r.sound.end = ended_of_record(last, nz, carry);
        return r;
    }
    hit = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hit, 5);
    r.sound.end = e;
    if (hit < 5) {
        return r;
    }
    jsr_push(0xB2C8);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = then(e, ended_of_object_event(marked, stored, event, byte));
    WR8(ZP->temp00, 0);
    to = 0x3D;
    do {
        jsr_push(0xB2CF);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB2D4);
        placed = StartObjectAtPlace(0x84, slot, to);
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
    (void)placed;
    r.sound.before = then(e, ended_load(0x19));
    r.sound.sounded = true;
    r.sound.command = 0x19;
    jsr_push(0xB301);
    r.sound.call = SoundCommand(0x19, slot, 0x40);
    jsr_pop();
    jsr_push(0xB306);
    r.burst = StartDebrisBurst(slot, 1, 0x40);
    jsr_pop();
    r.burst_run = true;
    WR8(OBJ->kind[slot], 0x9C);
    WR8(OBJ->state[slot], 0);
    r.sound.end = ended_kept();
    r.tail = ended_load(0x9C);
    return r;
}

void abi_BreakIntoPieces(void)
{
    BreakEnd r = BreakIntoPieces(cpu.x, cpu.y);

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
Ended StartScriptsAndLists(uint8_t slot)
{
    uint8_t state, nz, last, kind, scroll, list;
    bool carry;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        push8(slot);
        jsr_push(0xC0AC);
        StartScriptSlotKind1(0x91, 4);
        jsr_pop();
        slot = pull8();
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    scroll = rd8(ScrollYLo);
    scroll = (uint8_t)(scroll & 0x0F);
    e = ended_load(scroll);
    sum = add_bytes(scroll, 0xAE, false);
    list = sum.value;
    e = then(e, of_sum(sum));
    push8(slot);
    jsr_push(0xC0BE);
    kind = StartListChannelKind2(list, 4);
    jsr_pop();
    slot = pull8();
    e = then(e, ended_load(kind));
    return then(e, ended_nz(slot));
}

void abi_StartScriptsAndLists(void)
{
    abi_ended(StartScriptsAndLists(cpu.x));
}

/*
 * @routine 072B:76E StepThenMusicAgain
 * @run 70:B76E
 * WHAT IT DOES: Steps the animation list at $B7AD; on the new frame with animation step 2 it sets bit 4 of zField8E and plays sound $FE; when the list ends it turns on the object's
 * hitbox flag (2), clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), clears the object flags (OBJ_2F38), sets bit 2 of zField8E and bit 4 and 6 of OBJ_3148, and enters its next state.
 * HOW: StepObjectFrames, SoundCommand, CdPlayAgain.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
MusicEnd StepThenMusicAgain(uint8_t slot, uint8_t y)
{
    MusicEnd r = { { { 0 }, { 0 }, false, 0, { 0 } }, false, { 0 }, { 0 }, { 0 } };
    FrameStep step;
    uint8_t anim, flags, bits, state;
    Ended e;

    step = step_list(slot, 0xAD, 0xB7, 0xB779, &y);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    r.sound.end = e;
    if (step == FRAME_HELD) {
        return r;
    }
    if (step == FRAME_CHANGED) {
        anim = RD8(OBJ->anim_step[slot]);
        e = then(e, ended_compare(anim, 2));
        r.sound.end = e;
        if (anim != 2) {
            return r;
        }
        RMW8(ZP->field8_e, old, old | 0x10);
        r.sound.before = then(e, ended_load(0xFE));
        r.sound.sounded = true;
        r.sound.command = 0xFE;
        jsr_push(0xB78B);
        r.sound.call = SoundCommand(0xFE, slot, y);
        jsr_pop();
        r.sound.end = ended_kept();
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    RMW8(ZP->field8_e, old, old & 0xF7);
    RMW8(ZP->field8_e, old, old & 0xEF);
    r.pre = then(e, ended_load(2));
    jsr_push(0xB798);
    r.track = CdPlayAgain(slot, y);
    jsr_pop();
    r.played = true;
    WR8(OBJ->flags[slot], 0);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8_e, flags);
    bits = RD8(OBJ->f3148[slot]);
    bits = (uint8_t)(bits | 0x50);
    WR8(OBJ->f3148[slot], bits);
    state = INC8(OBJ->state[slot]);
    r.post = then(ended_load(bits), ended_nz(state));
    return r;
}

void abi_StepThenMusicAgain(void)
{
    MusicEnd r = StepThenMusicAgain(cpu.x, cpu.y);

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
 * WHAT IT DOES: The object faces the way it moves (facing is the X speed's sign bit turned round) and its OBJ_2FE8 is cleared; it is then set to $80 when the stage's flag word (OBJ_3828 of slot
 * $10) has the bit the Y speed's direction names (1 for falling, 2 for rising), the object is on the first screen row and its Y low byte is from $74 to $AF.
 * HOW: Shifts through carry for the sign bits, an AND with the byte at $3838 (OBJ_3828 slot $10), compares.
 * WAITS: no.
 * Entry: X object. Exit: the last AND, compare or load; C bit 6 of the Y speed.
 */
Ended FaceByVelocityAndCatch(uint8_t slot)
{
    uint8_t speed, facing, sign, mask, flags, y_high, y_low;
    Ended e;

    speed = RD8(OBJ->vx_hi[slot]);
    facing = (uint8_t)(((speed >> 7) & 0x01) ^ 0x01);
    e = ended_with_carry(ended_load(facing), (speed & 0x40) != 0);
    WR8(OBJ->facing_left[slot], facing);
    WR8(OBJ->f2fe8[slot], 0);
    speed = RD8(OBJ->vy_hi[slot]);
    sign = (uint8_t)((speed >> 7) & 0x01);
    e = then(e, ended_with_carry(ended_load(sign), (speed & 0x40) != 0));
    mask = (uint8_t)(sign + 1);
    e = then(e, ended_load(mask));
    flags = RD8(OBJ->f3828[0x10]);
    mask = (uint8_t)(mask & flags);
    e = then(e, ended_load(mask));
    if (mask == 0) {
        return e;
    }
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if (y_high != 0) {
        return e;
    }
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(y_low, 0x74));
    if (y_low < 0x74) {
        return e;
    }
    e = then(e, ended_compare(y_low, 0xB0));
    if (y_low >= 0xB0) {
        return e;
    }
    WR8(OBJ->f2fe8[slot], 0x80);
    return then(e, ended_load(0x80));
}

void abi_FaceByVelocityAndCatch(void)
{
    abi_ended(FaceByVelocityAndCatch(cpu.x));
}
