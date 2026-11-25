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

/* A body's result and the X it leaves. */
typedef struct {
    Ended end;
    uint8_t x;
} EndedX;

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
