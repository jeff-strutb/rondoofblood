/*
 * Small routines of the player's overlays, lot L2: the tile handlers of a stage whose overlay holds its own copy,
 * Richter's object-slot reads, the bank maps of the overlays, the velocity table writers and the empty handlers.
 *
 * A body returns plain values (an Ended beside any register the caller reads); its abi_ wrapper turns them into the
 * registers and flags the assembly leaves.
 *
 * Tables read by address here (StageTable9C20, the velocity tables) have no row in config/rondo/tables.csv yet: they
 * keep the old address form until the typed access owner lands the rows.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum { TILE_HANDLED_A = 0x81, STAGE_TABLE_9C20 = 0x9C20, VY_TABLE = 0xBFCB, VX_TABLE = 0xBFEC };

/* A row moved up by `amount`: SEC, SBC, and the handled answer in A. */
static Ended tile_row_up(uint8_t amount)
{
    uint8_t row = RD8(ZP->temp02);
    Sum sum = sub_bytes(row, amount, true);

    WR8(ZP->temp02, sum.value);
    return ended_with_overflow(ended_with_carry(ended_load(TILE_HANDLED_A), sum.carry), sum.overflow);
}

/*
 * @routine 024D:23B EmptyHandler823B
 * @run 6B:823B
 * WHAT IT DOES: Nothing: the handler an overlay's table names where an object has nothing to do.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: any. Exit: all registers and flags kept.
 */
void EmptyHandler823B(void)
{
}

void abi_EmptyHandler823B(void)
{
    EmptyHandler823B();
}

/*
 * @routine 0720:4FC EmptyHandler9CFC
 * @run 6B:9CFC
 * WHAT IT DOES: Nothing: the handler this overlay's table names where an object has nothing to do.
 * HOW: Returns at once.
 * WAITS: no.
 * Entry: any. Exit: all registers and flags kept.
 */
void EmptyHandler9CFC(void)
{
}

void abi_EmptyHandler9CFC(void)
{
    EmptyHandler9CFC();
}

/*
 * @routine 10C0:482 TileNudgeAt9C82
 * @run 6B:9C82
 * WHAT IT DOES: A tile's handler: the point's row one lower.
 * HOW: Increments zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81, N set, Z clear; C, V and Y kept.
 */
Ended TileNudgeAt9C82(void)
{
    INC8(ZP->temp02);
    return ended_load(TILE_HANDLED_A);
}

void abi_TileNudgeAt9C82(void)
{
    abi_ended(TileNudgeAt9C82());
}

/*
 * @routine 10C0:487 TileLift7At9C87
 * @run 6B:9C87
 * WHAT IT DOES: A tile's handler: the point's row seven higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
Ended TileLift7At9C87(void)
{
    return tile_row_up(7);
}

void abi_TileLift7At9C87(void)
{
    abi_ended(TileLift7At9C87());
}

/*
 * @routine 10C0:491 TileLift6At9C91
 * @run 6B:9C91
 * WHAT IT DOES: A tile's handler: the point's row six higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift6At9C91(void)
{
    return tile_row_up(6);
}

void abi_TileLift6At9C91(void)
{
    abi_ended(TileLift6At9C91());
}

/*
 * @routine 10C0:49B TileLift5At9C9B
 * @run 6B:9C9B
 * WHAT IT DOES: A tile's handler: the point's row five higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift5At9C9B(void)
{
    return tile_row_up(5);
}

void abi_TileLift5At9C9B(void)
{
    abi_ended(TileLift5At9C9B());
}

/*
 * @routine 10C0:4A5 TileLift4At9CA5
 * @run 6B:9CA5
 * WHAT IT DOES: A tile's handler: the point's row four higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift4At9CA5(void)
{
    return tile_row_up(4);
}

void abi_TileLift4At9CA5(void)
{
    abi_ended(TileLift4At9CA5());
}

/*
 * @routine 10C0:4AF TileLift3At9CAF
 * @run 6B:9CAF
 * WHAT IT DOES: A tile's handler: the point's row three higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift3At9CAF(void)
{
    return tile_row_up(3);
}

void abi_TileLift3At9CAF(void)
{
    abi_ended(TileLift3At9CAF());
}

/*
 * @routine 10C0:4B9 TileLift2At9CB9
 * @run 6B:9CB9
 * WHAT IT DOES: A tile's handler: the point's row two higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift2At9CB9(void)
{
    return tile_row_up(2);
}

void abi_TileLift2At9CB9(void)
{
    abi_ended(TileLift2At9CB9());
}

/*
 * @routine 10C0:4C3 TileLift1At9CC3
 * @run 6B:9CC3
 * WHAT IT DOES: A tile's handler: the point's row one higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
Ended TileLift1At9CC3(void)
{
    return tile_row_up(1);
}

void abi_TileLift1At9CC3(void)
{
    abi_ended(TileLift1At9CC3());
}

/*
 * @routine 01BC:7B4 RichterStateDoubled
 * @run 6D:BFB4
 * WHAT IT DOES: Twice the state of the object Richter's code is running, for a table of words.
 * HOW: Reads zRichterObject, the object's state, shifts it left.
 * WAITS: no.
 * Entry: nothing. Exit: A and X the doubled state, N and Z from it, C the state's bit 7. Y, V kept.
 */
EndedX RichterStateDoubled(void)
{
    uint8_t slot = RD8(ZP->richter_object);
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t doubled = (uint8_t)(state << 1);
    EndedX r;

    r.end = ended_with_carry(ended_load(doubled), (state & 0x80) != 0);
    r.x = doubled;
    return r;
}

void abi_RichterStateDoubled(void)
{
    EndedX r = RichterStateDoubled();

    abi_ended(r.end);
    cpu.x = r.x;
}

/*
 * @routine 01BD:01B RichterFlagsAgainstFour
 * @run 6E:C01B
 * WHAT IT DOES: Compares the player object's flag byte with 4, for the caller's branch.
 * HOW: Loads zRichterObject into X (unused here), the flags of object 0, CMP #4.
 * WAITS: no.
 * Entry: nothing. Exit: X zRichterObject, A the flags, N, Z, C from A less 4. Y, V kept.
 */
EndedX RichterFlagsAgainstFour(void)
{
    uint8_t slot = RD8(ZP->richter_object);
    uint8_t flags = RD8(OBJ->flags[0]);
    EndedX r;

    r.end = ended_compare(flags, 4);
    r.x = slot;
    return r;
}

void abi_RichterFlagsAgainstFour(void)
{
    EndedX r = RichterFlagsAgainstFour();

    abi_ended(r.end);
    cpu.x = r.x;
}

/*
 * @routine 024D:0B7 MapOverlayBank7
 * @run 6B:80B7
 * WHAT IT DOES: Maps the bank 7 above zRamBaseBank into MPR5.
 * HOW: ADC of the base bank to 7, TAM #$20.
 * WAITS: no.
 * Entry: nothing. Exit: A the bank, N, Z, C, V from the add; MPR5 set. X, Y kept.
 */
Ended MapOverlayBank7(void)
{
    uint8_t base = RD8(ZP->ram_base_bank);
    Sum sum = add_bytes(7, base, false);

    tam(0x20, sum.value);
    return ended_of_sum(sum);
}

void abi_MapOverlayBank7(void)
{
    abi_ended(MapOverlayBank7());
}

/*
 * @routine 024D:0A0 MapOverlayBanks5And6
 * @run 6B:80A0
 * WHAT IT DOES: Maps the banks 5 and 6 above zRamBaseBank into MPR5 and MPR6.
 * HOW: ADC of the base bank to 5, TAM #$20, INC A, TAM #$40.
 * WAITS: no.
 * Entry: nothing. Exit: A the second bank, N and Z from it, C and V from the add; MPR5 and MPR6 set.
 */
Ended MapOverlayBanks5And6(void)
{
    uint8_t base = RD8(ZP->ram_base_bank);
    Sum sum = add_bytes(5, base, false);
    uint8_t next = (uint8_t)(sum.value + 1);
    Ended e;

    tam(0x20, sum.value);
    tam(0x40, next);
    e = ended_with_overflow(ended_with_carry(ended_load(next), sum.carry), sum.overflow);
    return e;
}

void abi_MapOverlayBanks5And6(void)
{
    abi_ended(MapOverlayBanks5And6());
}

/*
 * @routine 050D:66A StepStageTable9C20
 * @run 6B:866A
 * WHAT IT DOES: Steps zField82 and sets zField83 from the byte of the table at $9C20 it indexes.
 * HOW: INC zField82, read it again as the index, one table read, one store.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. C, V kept.
 */
Ended StepStageTable9C20(void)
{
    uint8_t index;
    uint8_t value;

    INC8(ZP->field82);
    index = RD8(ZP->field82);
    value = rd8((uint16_t)(STAGE_TABLE_9C20 + index));
    WR8(ZP->field83, value);
    return ended_with_y(ended_load(value), index);
}

void abi_StepStageTable9C20(void)
{
    abi_ended(StepStageTable9C20());
}

/*
 * @routine 01BC:7BC SetVelocityYFromTable
 * @run 6D:BFBC
 * WHAT IT DOES: Gives the object a vertical velocity from a table of words, entry A.
 * HOW: Y = A doubled; the low byte from $BFCB+Y into OBJ_VY_LO, the high from $BFCC+Y into OBJ_VY_HI.
 * WAITS: no.
 * Entry: A the entry, X the object. Exit: A the high byte, N and Z from it, C the entry's bit 7, Y the doubled entry.
 */
Ended SetVelocityYFromTable(uint8_t slot, uint8_t entry)
{
    uint8_t y = (uint8_t)(entry << 1);
    uint8_t value;

    value = rd8((uint16_t)(VY_TABLE + y));
    WR8(OBJ->vy_lo[slot], value);
    value = rd8((uint16_t)(VY_TABLE + 1 + y));
    WR8(OBJ->vy_hi[slot], value);
    return ended_with_y(ended_with_carry(ended_load(value), (entry & 0x80) != 0), y);
}

void abi_SetVelocityYFromTable(void)
{
    abi_ended(SetVelocityYFromTable(cpu.x, cpu.a));
}

/*
 * @routine 01BC:7DD SetVelocityXFromTable
 * @run 6D:BFDD
 * WHAT IT DOES: Gives the object a horizontal velocity from a table of words, entry A.
 * HOW: Y = A doubled; the low byte from $BFEC+Y into OBJ_VX_LO, the high from $BFED+Y into OBJ_VX_HI.
 * WAITS: no.
 * Entry and exit: as SetVelocityYFromTable.
 */
Ended SetVelocityXFromTable(uint8_t slot, uint8_t entry)
{
    uint8_t y = (uint8_t)(entry << 1);
    uint8_t value;

    value = rd8((uint16_t)(VX_TABLE + y));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(VX_TABLE + 1 + y));
    WR8(OBJ->vx_hi[slot], value);
    return ended_with_y(ended_with_carry(ended_load(value), (entry & 0x80) != 0), y);
}

void abi_SetVelocityXFromTable(void)
{
    abi_ended(SetVelocityXFromTable(cpu.x, cpu.a));
}

/*
 * @routine 01C0:063 NegateVelocityXIfFacingLeft
 * @run 6E:D863
 * WHAT IT DOES: Turns the object's horizontal velocity round when its facing byte is set.
 * HOW: Reads the facing byte; when it is not zero, goes on to NegateVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: facing zero: A zero, Z. Otherwise as NegateVelocityX.
 */
Ended NegateVelocityXIfFacingLeft(uint8_t slot)
{
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    Ended e = ended_load(facing);

    if (facing == 0) {
        return e;
    }
    return ended_then(e, ended_of_sum(NegateVelocityX(slot)));
}

void abi_NegateVelocityXIfFacingLeft(void)
{
    abi_ended(NegateVelocityXIfFacingLeft(cpu.x));
}

/*
 * @routine 01BB:1AA CountDownThenStand
 * @run 6D:B1AA
 * WHAT IT DOES: Counts the player's timer in OBJ_2F90 down; when it reaches zero he stands.
 * HOW: DEC of the byte; zero goes on to RichterStand, any other value returns.
 * WAITS: no.
 * Entry: nothing. Exit: counting on, N and Z from the new value; at zero as RichterStand.
 * Checked with --callees wrappers: RichterStand's earlier C differs from its draft in the call frames of its own calls.
 */
typedef struct {
    uint8_t value;
    bool stands;
    AnimationStep stand;
} CountDown;

CountDown CountDownThenStand(void)
{
    CountDown r = { 0, false, { FRAME_STOPPED, false, false, 0, 0 } };

    r.value = DEC8(OBJ->f2f90[0]);
    if (r.value == 0) {
        r.stands = true;
        r.stand = RichterStand();
    }
    return r;
}

void abi_CountDownThenStand(void)
{
    CountDown r = CountDownThenStand();

    abi_nz(r.value);
    if (r.stands) {
        animation_exit_shim(r.stand);
    }
}

/*
 * @routine 01BE:174 ClearObjectIfOffRange
 * @run 6E:C974
 * WHAT IT DOES: Empties the object when it is off the map above or below, or far to the side.
 * HOW: Y high must be zero; X high zero keeps it; otherwise X low from $40 up to $BF empties it.
 * WAITS: no.
 * Entry: X object. Exit: kept: A the last byte read, N, Z, C from the last compare; emptied: as the load or compare
 * before the jump (ClearObject keeps every register).
 */
Ended ClearObjectIfOffRange(uint8_t slot)
{
    uint8_t value = RD8(OBJ->y_hi[slot]);
    Ended e = ended_load(value);

    if (value == 0) {
        value = RD8(OBJ->x_hi[slot]);
        e = ended_load(value);
        if (value == 0) {
            return e;
        }
        value = RD8(OBJ->x_lo[slot]);
        e = ended_compare(value, 0x40);
        if (value < 0x40) {
            return e;
        }
        e = ended_compare(value, 0xC0);
        if (value >= 0xC0) {
            return e;
        }
    }
    ClearObject(slot);
    return e;
}

void abi_ClearObjectIfOffRange(void)
{
    abi_ended(ClearObjectIfOffRange(cpu.x));
}

/*
 * @routine 01C0:129 ClearObjectInYAndCopyPlace
 * @run 6E:D929
 * WHAT IT DOES: Empties the object in Y, then gives it the position of the object in X.
 * HOW: ClearObjectInY, then four byte copies: X low, X high, Y low, Y high.
 * WAITS: no.
 * Entry: X the source object, Y the one to empty. Exit: A the last byte copied, N and Z from it. X, Y, C, V kept.
 */
Ended ClearObjectInYAndCopyPlace(uint8_t source, uint8_t target)
{
    uint8_t value;

    jsr_push(0xD92C);
    ClearObjectInY(target);
    jsr_pop();
    value = RD8(OBJ->x_lo[source]);
    WR8(OBJ->x_lo[target], value);
    value = RD8(OBJ->x_hi[source]);
    WR8(OBJ->x_hi[target], value);
    value = RD8(OBJ->y_lo[source]);
    WR8(OBJ->y_lo[target], value);
    value = RD8(OBJ->y_hi[source]);
    WR8(OBJ->y_hi[target], value);
    return ended_load(value);
}

void abi_ClearObjectInYAndCopyPlace(void)
{
    abi_ended(ClearObjectInYAndCopyPlace(cpu.x, cpu.y));
}

/*
 * @routine 01BB:0A0 RichterTurnOnPad
 * @run 6D:B0A0
 * WHAT IT DOES: Turns the player round when the pad says so: facing right and the pad's bit 7 held turns him left,
 * facing left and bit 5 held turns him right; the state then takes the next one from a table.
 * HOW: OBJ_2AC0 is the facing; a TST of zPadHeld; the state less 5 indexes the table at $B03B.
 * WAITS: no.
 * Entry: nothing. Exit: no turn: A the facing, N, V, Z from the TST. A turn: A the new state, N and Z from it, Y the
 * old state less 5, C and V from that subtract.
 */
EndedTst RichterTurnOnPad(void)
{
    uint8_t facing = RD8(OBJ->facing_left[0]);
    uint8_t pad;
    uint8_t state;
    Sum sum;
    uint8_t value;
    Ended e = ended_load(facing);

    if (facing == 0) {
        pad = RD8(ZP->pad_held);
        if ((pad & 0x80) == 0) {
            return ended_tst(e, pad, 0x80);
        }
        INC8(OBJ->facing_left[0]);
    } else {
        pad = RD8(ZP->pad_held);
        if ((pad & 0x20) == 0) {
            return ended_tst(e, pad, 0x20);
        }
        WR8(OBJ->facing_left[0], 0);
    }
    state = RD8(OBJ->state[0]);
    sum = sub_bytes(state, 5, true);
    value = rd8((uint16_t)(0xB03B + sum.value));
    WR8(OBJ->state[0], value);
    e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(value), sum.carry), sum.overflow), sum.value);
    return ended_tst_none(e);
}

void abi_RichterTurnOnPad(void)
{
    abi_ended_tst(RichterTurnOnPad());
}

/*
 * @routine 09EB:559 LoadRecordThenRecordEvent
 * @run 70:B559
 * WHAT IT DOES: Loads the object's record, then marks its event when it is one the save remembers.
 * HOW: LoadObjectRecord, then goes on to RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as RecordObjectEvent.
 */
Ended LoadRecordThenRecordEvent(uint8_t slot)
{
    uint8_t last, nz, stored, event, byte;
    bool carry;
    bool marked;
    Ended e;

    jsr_push(0xB55C);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    if (!marked) {
        return ended_then(e, ended_load(0));
    }
    return ended_then(e, ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0));
}

void abi_LoadRecordThenRecordEvent(void)
{
    abi_ended(LoadRecordThenRecordEvent(cpu.x));
}

/*
 * @routine 01BB:79E RichterReadAtOffset
 * @run 6D:B79E
 * WHAT IT DOES: Reads the terrain at a point offset from the player's own X and Y low bytes by a pair from a table.
 * HOW: X low plus the table's byte at $B7B1+Y into zTemp00; Y low plus the one at $B7B2+Y becomes the row, its carry
 * the Y high's; goes on to RichterReadOnScreen.
 * WAITS: no.
 * Entry: Y the table's offset. Exit: as RichterReadOnScreen.
 * Checked with --callees wrappers; no case reaches the read's own code, so the read made by C is not reached.
 */
typedef struct {
    Sum row;
    TileRead read;
} OffsetRead;

OffsetRead RichterReadAtOffset(uint8_t offset)
{
    OffsetRead r;
    uint8_t x_lo = RD8(OBJ->x_lo[0]);
    Sum column = add_bytes(x_lo, rd8((uint16_t)(0xB7B1 + offset)), false);
    uint8_t y_lo;

    WR8(ZP->temp00, column.value);
    y_lo = RD8(OBJ->y_lo[0]);
    r.row = add_bytes(y_lo, rd8((uint16_t)(0xB7B2 + offset)), false);
    abi_ended(ended_of_sum(r.row)); /* the read may run the overlay's own code, entered with this add's A and flags */
    r.read = RichterReadOnScreen(r.row.value, r.row.carry);
    return r;
}

void abi_RichterReadAtOffset(void)
{
    OffsetRead r = RichterReadAtOffset(cpu.y);

    if (r.read.kind != TILE_BY_ORIGINAL) {
        abi_ended(ended_of_sum(r.row));
    }
    terrain_read_exit_shim(r.read);
}

/* A tile's step from a table by the tile and the half of it the point lies in: the shared body. */
static Ended tile_half_step(uint16_t table, uint8_t first_tile, uint8_t tile, bool right_for_bit)
{
    uint8_t index = (uint8_t)((uint8_t)(tile - first_tile) << 1);
    uint8_t column = RD8(ZP->temp00);
    uint8_t step;
    uint8_t row;
    Sum sum;

    if (((column & 4) != 0) == right_for_bit) {
        index++;
    }
    step = rd8((uint16_t)(table + index));
    row = RD8(ZP->temp02);
    sum = add_bytes(step, row, false);
    WR8(ZP->temp02, sum.value);
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(TILE_HANDLED_A), sum.carry), sum.overflow), index);
}

/*
 * @routine 10C0:293 TileHalfStepAAt9A93
 * @run 6B:9A93
 * WHAT IT DOES: The tiles $83 and up of this overlay: a step of the row that differs between the tile's halves.
 * HOW: A pair of bytes in the table at $9AEF by tile, the second for the right half.
 * WAITS: no.
 * Entry: A tile, zTemp00 column, zTemp02 row. Exit: A $81; Y the index; C and V from the add.
 */
Ended TileHalfStepAAt9A93(uint8_t tile)
{
    return tile_half_step(0x9AEF, 0x83, tile, true);
}

void abi_TileHalfStepAAt9A93(void)
{
    abi_ended(TileHalfStepAAt9A93(cpu.a));
}

/*
 * @routine 10C0:2A7 TileHalfStepBAt9AA7
 * @run 6B:9AA7
 * WHAT IT DOES: The tiles $8C and up of this overlay: as TileHalfStepAAt9A93, the other way round.
 * HOW: A pair of bytes in the table at $9AF7 by tile, the second for the left half.
 * WAITS: no.
 * Entry and exit: as TileHalfStepAAt9A93.
 */
Ended TileHalfStepBAt9AA7(uint8_t tile)
{
    return tile_half_step(0x9AF7, 0x8C, tile, false);
}

void abi_TileHalfStepBAt9AA7(void)
{
    abi_ended(TileHalfStepBAt9AA7(cpu.a));
}

/*
 * @routine 01BE:567 MoveObjectUpBySigned
 * @run 6E:CD67
 * WHAT IT DOES: Moves the object up the screen by the signed amount in zTemp02: its Y position less the amount,
 * sign extended into the high byte.
 * HOW: A 16-bit subtract of zTemp02, read again for the low byte; the high byte takes the borrow, less 1 more when the
 * amount is negative ($FF subtracted). The negative arm is untraced (137 of 400 cases reach it, and the draft stops
 * there) and is written from the disc's bytes.
 * WAITS: no.
 * Entry: X object, zTemp02 the amount. Exit: A the Y high byte stored, N, Z, C, V from its subtract. X, Y kept.
 */
Ended MoveObjectUpBySigned(uint8_t slot)
{
    uint8_t amount = RD8(ZP->temp02);
    uint8_t high_operand = (amount & 0x80) ? 0xFF : 0x00;
    uint8_t y_low = RD8(OBJ->y_lo[slot]);
    uint8_t taken = RD8(ZP->temp02);
    Sum low, high;

    low = sub_bytes(y_low, taken, true);
    WR8(OBJ->y_lo[slot], low.value);
    high = sub_bytes(RD8(OBJ->y_hi[slot]), high_operand, low.carry);
    WR8(OBJ->y_hi[slot], high.value);
    return ended_of_sum(high);
}

void abi_MoveObjectUpBySigned(void)
{
    abi_ended(MoveObjectUpBySigned(cpu.x));
}

/*
 * @routine 0720:4ED LoadRecordThenFrameByKind
 * @run 6B:9CED
 * WHAT IT DOES: Loads the object's record, then gives it frame 0 or 1 of a two byte table by bit 0 of its next kind.
 * HOW: LoadObjectRecord, OBJ_3040 and 1, the table at $9CFD; falls into the RTS at $9CFC's neighbour.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N and Z from it, Y the index, C from the load of the record. X kept.
 */
Ended LoadRecordThenFrameByKind(uint8_t slot)
{
    uint8_t last, nz, kind, index, frame;
    bool carry;
    Ended e;

    jsr_push(0x9CF0);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    kind = RD8(OBJ->next_kind[slot]);
    index = (uint8_t)(kind & 1);
    frame = rd8((uint16_t)(0x9CFD + index));
    WR8(OBJ->frame[slot], frame);
    return ended_then(e, ended_with_y(ended_load(frame), index));
}

void abi_LoadRecordThenFrameByKind(void)
{
    abi_ended(LoadRecordThenFrameByKind(cpu.x));
}

/*
 * @routine 0E00:092 StoreHitFromTable
 * @run 6B:9892
 * WHAT IT DOES: Gives the object's hit timer OBJ_33B0 the byte of OBJ_3828 at the slot in Y.
 * HOW: One read, one write.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: A the byte, N and Z from it.
 */
Ended StoreHitFromTable(uint8_t slot, uint8_t y)
{
    uint8_t value = RD8(OBJ->f3828[y]);

    WR8(OBJ->hit_taken[slot], value);
    return ended_load(value);
}

void abi_StoreHitFromTable(void)
{
    abi_ended(StoreHitFromTable(cpu.x, cpu.y));
}

/*
 * @routine 0E00:07A ReportHitToStage
 * @run 6B:987A
 * WHAT IT DOES: Copies the stream count's two bytes and the hitbox X and Y of slot 3 to the stage's four byte report,
 * then gives the object's hit timer its byte.
 * HOW: Four read and write pairs, then goes on to StoreHitFromTable.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: as StoreHitFromTable.
 */
Ended ReportHitToStage(uint8_t slot, uint8_t y)
{
    uint8_t value;

    value = rd8(WorkByteA);
    wr8(StageHitReport, value);
    value = rd8(WorkByteB);
    wr8(StageHitReport + 1, value);
    value = RD8(HITBOX->x_lo[3]);
    wr8(StageHitReport + 2, value);
    value = RD8(HITBOX->y_lo[3]);
    wr8(StageHitReport + 3, value);
    return StoreHitFromTable(slot, y);
}

void abi_ReportHitToStage(void)
{
    abi_ended(ReportHitToStage(cpu.x, cpu.y));
}

/*
 * @routine 0720:076 ReportHitUnlessTwo
 * @run 6B:9876
 * WHAT IT DOES: Reports the hit to the stage unless the kind just stored in OBJ_38D8 is 2.
 * HOW: CMP #2 on A; equal returns, otherwise goes on to ReportHitToStage.
 * WAITS: no.
 * Entry: A the kind, X object, Y the slot. Exit: equal: A and N, Z, C from the compare; otherwise as StoreHitFromTable.
 */
Ended ReportHitUnlessTwo(uint8_t kind, uint8_t slot, uint8_t y)
{
    Ended e = ended_compare(kind, 2);

    if (kind == 2) {
        return e;
    }
    return ended_then(e, ReportHitToStage(slot, y));
}

void abi_ReportHitUnlessTwo(void)
{
    abi_ended(ReportHitUnlessTwo(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0720:099 HitKindFromBit1
 * @run 6B:9899
 * WHAT IT DOES: Gives the hit slot's kind (OBJ_38D8) 1 or 2 by bit 1 of the object's OBJ_3148, then reports the hit.
 * HOW: AND #2, LSR, INC, one store, then ReportHitUnlessTwo.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: as ReportHitUnlessTwo.
 */
Ended HitKindFromBit1(uint8_t slot, uint8_t y)
{
    uint8_t flags = RD8(OBJ->f3148[slot]);
    uint8_t kind = (uint8_t)(((flags & 2) >> 1) + 1);

    WR8(OBJ->f38d8[y], kind);
    return ReportHitUnlessTwo(kind, slot, y);
}

void abi_HitKindFromBit1(void)
{
    abi_ended(HitKindFromBit1(cpu.x, cpu.y));
}

/*
 * @routine 05C0:0A5 HitKindFromBit7
 * @run 6B:98A5
 * WHAT IT DOES: Gives the hit slot's kind (OBJ_38D8) 1 or 2 by bit 7 of the object's OBJ_3148; kind 2 also sets the
 * slot's OBJ_3828 to 5; then reports the hit.
 * HOW: AND #$80 rotated twice, AND #1, INC, one store, CMP #2; the report as ReportHitToStage. The kind 2 arm is
 * untraced (135 of 400 cases reach it and the draft stops there), written from the disc's bytes.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: as StoreHitFromTable.
 */
Ended HitKindFromBit7(uint8_t slot, uint8_t y)
{
    uint8_t flags = RD8(OBJ->f3148[slot]);
    uint8_t kind = (uint8_t)((((flags & 0x80) ? 1 : 0)) + 1);
    Ended e;

    WR8(OBJ->f38d8[y], kind);
    e = ended_compare(kind, 2);
    if (kind == 2) {
        WR8(OBJ->f3828[y], 5);
        e = ended_load(5);
    }
    return ended_then(e, ReportHitToStage(slot, y));
}

void abi_HitKindFromBit7(void)
{
    abi_ended(HitKindFromBit7(cpu.x, cpu.y));
}
