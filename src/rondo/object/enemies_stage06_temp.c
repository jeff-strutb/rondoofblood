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
