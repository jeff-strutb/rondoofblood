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

enum { TILE_HANDLED_A = 0x81, STAGE_TABLE_9C20 = 0x9C20, VY_TABLE = 0xBFCB, VX_TABLE = 0xBFEC, MARIA_VY_TABLE = 0xC29A,
       MARIA_VX_TABLE = 0xC2ED };

/* Arms: one enum per routine that has more than one way out. */
enum { PL_CLEAR_LOAD, PL_CLEAR_COMPARE_40, PL_CLEAR_COMPARE_C0 };
enum { PL_NEG_FACING_RIGHT, PL_NEG_NEGATED };
enum { PL_REC_NOT_MARKED, PL_REC_MARKED };
enum { PL_REPORT_UNLESS_EQUAL_TWO, PL_REPORT_UNLESS_REPORTED };
enum { PL_BIT1_REPORTED };
enum { PL_BIT7_SECOND_OF_TWO, PL_BIT7_FIRST };
enum { PL_TURN_FACING_RIGHT_NOT, PL_TURN_FACING_LEFT_NOT, PL_TURN_FLIPPED_RIGHT, PL_TURN_FLIPPED_LEFT };

/* A row moved up by `amount`: SEC, SBC, and the handled answer in A. */
static ObjArm tile_row_up(uint8_t amount)
{
    ObjArm r = { 0 };
    uint8_t row = RD8(ZP->temp02);

    r.sum = sub_bytes(row, amount, true);
    WR8(ZP->temp02, r.sum.value);
    return r;
}

/* The tile handlers' result: A $81, C and V from the row's subtraction. */
Ended tile_row_up_exit_shim(ObjArm r)
{
    return ended_with_overflow(ended_with_carry(ended_load(TILE_HANDLED_A), r.sum.carry), r.sum.overflow);
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
ObjArm TileNudgeAt9C82(void)
{
    ObjArm r = { 0 };

    INC8(ZP->temp02);
    return r;
}

Ended tile_nudge_exit_shim(ObjArm r)
{
    (void)r;
    return ended_load(TILE_HANDLED_A);
}

void abi_TileNudgeAt9C82(void)
{
    abi_ended(tile_nudge_exit_shim(TileNudgeAt9C82()));
}

/*
 * @routine 10C0:487 TileLift7At9C87
 * @run 6B:9C87
 * WHAT IT DOES: A tile's handler: the point's row seven higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
 */
ObjArm TileLift7At9C87(void)
{
    return tile_row_up(7);
}

void abi_TileLift7At9C87(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift7At9C87()));
}

/*
 * @routine 10C0:491 TileLift6At9C91
 * @run 6B:9C91
 * WHAT IT DOES: A tile's handler: the point's row six higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift6At9C91(void)
{
    return tile_row_up(6);
}

void abi_TileLift6At9C91(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift6At9C91()));
}

/*
 * @routine 10C0:49B TileLift5At9C9B
 * @run 6B:9C9B
 * WHAT IT DOES: A tile's handler: the point's row five higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift5At9C9B(void)
{
    return tile_row_up(5);
}

void abi_TileLift5At9C9B(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift5At9C9B()));
}

/*
 * @routine 10C0:4A5 TileLift4At9CA5
 * @run 6B:9CA5
 * WHAT IT DOES: A tile's handler: the point's row four higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift4At9CA5(void)
{
    return tile_row_up(4);
}

void abi_TileLift4At9CA5(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift4At9CA5()));
}

/*
 * @routine 10C0:4AF TileLift3At9CAF
 * @run 6B:9CAF
 * WHAT IT DOES: A tile's handler: the point's row three higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift3At9CAF(void)
{
    return tile_row_up(3);
}

void abi_TileLift3At9CAF(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift3At9CAF()));
}

/*
 * @routine 10C0:4B9 TileLift2At9CB9
 * @run 6B:9CB9
 * WHAT IT DOES: A tile's handler: the point's row two higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift2At9CB9(void)
{
    return tile_row_up(2);
}

void abi_TileLift2At9CB9(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift2At9CB9()));
}

/*
 * @routine 10C0:4C3 TileLift1At9CC3
 * @run 6B:9CC3
 * WHAT IT DOES: A tile's handler: the point's row one higher.
 * HOW: A subtract from zTemp02; the handled answer $81 in A.
 * WAITS: no.
 * Entry and exit: as TileLift7At9C87.
 */
ObjArm TileLift1At9CC3(void)
{
    return tile_row_up(1);
}

void abi_TileLift1At9CC3(void)
{
    abi_ended(tile_row_up_exit_shim(TileLift1At9CC3()));
}

/*
 * @routine 01BC:7B4 RichterStateDoubled
 * @run 6D:BFB4
 * WHAT IT DOES: Twice the state of the object Richter's code is running, for a table of words.
 * HOW: Reads zRichterObject, the object's state, shifts it left.
 * WAITS: no.
 * Entry: nothing. Exit: A and X the doubled state, N and Z from it, C the state's bit 7. Y, V kept.
 */
static ObjArm state_doubled_body(void)
{
    ObjArm r = { 0 };
    uint8_t slot = RD8(ZP->richter_object);
    uint8_t state = RD8(OBJ->state[slot]);

    r.first = (uint8_t)(state << 1);
    r.flag = (state & 0x80) != 0;
    return r;
}

ObjArm RichterStateDoubled(void)
{
    return state_doubled_body();
}

/* A and X the doubled state; C its bit 7. */
static EndedX richter_state_doubled_exit_shim(ObjArm r)
{
    EndedX e;

    e.end = ended_with_carry(ended_load(r.first), r.flag);
    e.x = r.first;
    return e;
}

void abi_RichterStateDoubled(void)
{
        EndedX r = richter_state_doubled_exit_shim(RichterStateDoubled());
    abi_ended(r.end);
    cpu.x = r.x;
}

/*
 * @routine 01E1:4E4 MariaStateDoubled
 * @run 6E:C4E4
 * WHAT IT DOES: RichterStateDoubled in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterStateDoubled's body.
 * WAITS: no.
 * Entry, exit: as RichterStateDoubled.
 */
ObjArm MariaStateDoubled(void)
{
    return state_doubled_body();
}

void abi_MariaStateDoubled(void)
{
        EndedX r = richter_state_doubled_exit_shim(MariaStateDoubled());
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
ObjArm RichterFlagsAgainstFour(void)
{
    ObjArm r = { 0 };

    r.second = RD8(ZP->richter_object);
    r.first = RD8(OBJ->scroll_band[0]);
    return r;
}

/* The flags compared with 4; X the Richter's slot. */
static EndedX richter_flags_against_four_exit_shim(ObjArm r)
{
    EndedX e;

    e.end = ended_compare(r.first, 4);
    e.x = r.second;
    return e;
}

void abi_RichterFlagsAgainstFour(void)
{
        EndedX r = richter_flags_against_four_exit_shim(RichterFlagsAgainstFour());

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
ObjArm MapOverlayBank7(void)
{
    ObjArm r = { 0 };
    uint8_t base = RD8(ZP->ram_base_bank);

    r.sum = add_bytes(7, base, false);
    tam(0x20, r.sum.value);
    return r;
}

static Ended map_overlay_bank7_exit_shim(ObjArm r)
{
    return ended_of_sum(r.sum);
}

void abi_MapOverlayBank7(void)
{
    abi_ended(map_overlay_bank7_exit_shim(MapOverlayBank7()));
}

/*
 * @routine 024D:0A0 MapOverlayBanks5And6
 * @run 6B:80A0
 * WHAT IT DOES: Maps the banks 5 and 6 above zRamBaseBank into MPR5 and MPR6.
 * HOW: ADC of the base bank to 5, TAM #$20, INC A, TAM #$40.
 * WAITS: no.
 * Entry: nothing. Exit: A the second bank, N and Z from it, C and V from the add; MPR5 and MPR6 set.
 */
ObjArm MapOverlayBanks5And6(void)
{
    ObjArm r = { 0 };
    uint8_t base = RD8(ZP->ram_base_bank);

    r.sum = add_bytes(5, base, false);
    r.first = (uint8_t)(r.sum.value + 1);
    tam(0x20, r.sum.value);
    tam(0x40, r.first);
    return r;
}

/* A the second bank; C and V from the add. */
static Ended map_overlay_banks_exit_shim(ObjArm r)
{
    return ended_with_overflow(ended_with_carry(ended_load(r.first), r.sum.carry), r.sum.overflow);
}

void abi_MapOverlayBanks5And6(void)
{
    abi_ended(map_overlay_banks_exit_shim(MapOverlayBanks5And6()));
}

/*
 * @routine 050D:66A StepStageTable9C20
 * @run 6B:866A
 * WHAT IT DOES: Steps zField82 and sets zField83 from the byte of the table at $9C20 it indexes.
 * HOW: INC zField82, read it again as the index, one table read, one store.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. C, V kept.
 */
ObjArm StepStageTable9C20(void)
{
    ObjArm r = { 0 };

    INC8(ZP->field82);
    r.y = RD8(ZP->field82);
    r.first = rd8((uint16_t)(STAGE_TABLE_9C20 + r.y));
    WR8(ZP->field83, r.first);
    return r;
}

/* A the table's value, Y the index. */
static Ended step_stage_table_exit_shim(ObjArm r)
{
    return ended_with_y(ended_load(r.first), r.y);
}

void abi_StepStageTable9C20(void)
{
    abi_ended(step_stage_table_exit_shim(StepStageTable9C20()));
}

/*
 * @routine 01BC:7BC SetVelocityYFromTable
 * @run 6D:BFBC
 * WHAT IT DOES: Gives the object a vertical velocity from a table of words, entry A.
 * HOW: Y = A doubled; the low byte from $BFCB+Y into OBJ_VY_LO, the high from $BFCC+Y into OBJ_VY_HI.
 * WAITS: no.
 * Entry: A the entry, X the object. Exit: A the high byte, N and Z from it, C the entry's bit 7, Y the doubled entry.
 */
/* A the high byte read, C the entry's bit 7, Y twice the entry. */
static Ended set_velocity_from_table_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_carry(ended_load(r.first), r.flag), r.y);
}

static ObjArm set_velocity_y_from_table(uint8_t slot, uint8_t entry, uint16_t table)
{
    ObjArm r = { 0 };

    r.y = (uint8_t)(entry << 1);
    r.first = rd8((uint16_t)(table + r.y));
    WR8(OBJ->vy_lo[slot], r.first);
    r.first = rd8((uint16_t)(table + 1 + r.y));
    WR8(OBJ->vy_hi[slot], r.first);
    r.flag = (entry & 0x80) != 0;
    return r;
}

ObjArm SetVelocityYFromTable(uint8_t slot, uint8_t entry)
{
    return set_velocity_y_from_table(slot, entry, VY_TABLE);
}

void abi_SetVelocityYFromTable(void)
{
    abi_ended(set_velocity_from_table_exit_shim(SetVelocityYFromTable(cpu.x, cpu.a)));
}

/*
 * @routine 01E1:28B MariaSetVelocityYFromTable
 * @run 6E:C28B
 * WHAT IT DOES: SetVelocityYFromTable in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of words at $C29A.
 * WAITS: no.
 * Entry, exit: as SetVelocityYFromTable.
 */
ObjArm MariaSetVelocityYFromTable(uint8_t slot, uint8_t entry)
{
    return set_velocity_y_from_table(slot, entry, MARIA_VY_TABLE);
}

void abi_MariaSetVelocityYFromTable(void)
{
    abi_ended(set_velocity_from_table_exit_shim(MariaSetVelocityYFromTable(cpu.x, cpu.a)));
}

/*
 * @routine 01BC:7DD SetVelocityXFromTable
 * @run 6D:BFDD
 * WHAT IT DOES: Gives the object a horizontal velocity from a table of words, entry A.
 * HOW: Y = A doubled; the low byte from $BFEC+Y into OBJ_VX_LO, the high from $BFED+Y into OBJ_VX_HI.
 * WAITS: no.
 * Entry and exit: as SetVelocityYFromTable.
 */
static ObjArm set_velocity_x_from_table(uint8_t slot, uint8_t entry, uint16_t table)
{
    ObjArm r = { 0 };

    r.y = (uint8_t)(entry << 1);
    r.first = rd8((uint16_t)(table + r.y));
    WR8(OBJ->vx_lo[slot], r.first);
    r.first = rd8((uint16_t)(table + 1 + r.y));
    WR8(OBJ->vx_hi[slot], r.first);
    r.flag = (entry & 0x80) != 0;
    return r;
}

ObjArm SetVelocityXFromTable(uint8_t slot, uint8_t entry)
{
    return set_velocity_x_from_table(slot, entry, VX_TABLE);
}

void abi_SetVelocityXFromTable(void)
{
    abi_ended(set_velocity_from_table_exit_shim(SetVelocityXFromTable(cpu.x, cpu.a)));
}

/*
 * @routine 01E1:2BC MariaSetVelocityXFromTable
 * @run 6E:C2BC
 * WHAT IT DOES: SetVelocityXFromTable in Maria's player overlay (01DD).
 * HOW: The same instructions with her table of words at $C2ED.
 * WAITS: no.
 * Entry, exit: as SetVelocityXFromTable.
 */
ObjArm MariaSetVelocityXFromTable(uint8_t slot, uint8_t entry)
{
    return set_velocity_x_from_table(slot, entry, MARIA_VX_TABLE);
}

void abi_MariaSetVelocityXFromTable(void)
{
    abi_ended(set_velocity_from_table_exit_shim(MariaSetVelocityXFromTable(cpu.x, cpu.a)));
}

/*
 * @routine 01C0:063 NegateVelocityXIfFacingLeft
 * @run 6E:D863
 * WHAT IT DOES: Turns the object's horizontal velocity round when its facing byte is set.
 * HOW: Reads the facing byte; when it is not zero, goes on to NegateVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: facing zero: A zero, Z. Otherwise as NegateVelocityX.
 */
ObjArm NegateVelocityXIfFacingLeft(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->facing_left[slot]);
    r.arm = PL_NEG_FACING_RIGHT;
    if (r.first == 0) {
        return r;
    }
    r.sum = NegateVelocityX(slot);
    r.arm = PL_NEG_NEGATED;
    return r;
}

/* The facing's load; when it was set, the negation's sum after it. */
static Ended negate_velocity_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == PL_NEG_FACING_RIGHT) {
        return e;
    }
    return ended_then(e, ended_of_sum(r.sum));
}

void abi_NegateVelocityXIfFacingLeft(void)
{
    abi_ended(negate_velocity_exit_shim(NegateVelocityXIfFacingLeft(cpu.x)));
}

/*
 * @routine 01BB:1AA CountDownThenStand
 * @run 6D:B1AA
 * WHAT IT DOES: Counts the player's timer in OBJ_HURT_TIMER down; when it reaches zero he stands.
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

    r.value = DEC8(OBJ->hurt_timer[0]);
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
ObjArm ClearObjectIfOffRange(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->y_hi[slot]);
    r.arm = PL_CLEAR_LOAD;
    if (r.first == 0) {
        r.first = RD8(OBJ->x_hi[slot]);
        if (r.first == 0) {
            return r;
        }
        r.first = RD8(OBJ->x_lo[slot]);
        r.arm = PL_CLEAR_COMPARE_40;
        if (r.first < 0x40) {
            return r;
        }
        r.arm = PL_CLEAR_COMPARE_C0;
        if (r.first >= 0xC0) {
            return r;
        }
    }
    ClearObject(slot);
    return r;
}

/* The last load or compare the walk made: of y_hi or x_hi, or of x_lo with $40 or $C0. */
static Ended clear_object_exit_shim(ObjArm r)
{
    if (r.arm == PL_CLEAR_LOAD) {
        return ended_load(r.first);
    }
    return ended_compare(r.first, r.arm == PL_CLEAR_COMPARE_40 ? 0x40 : 0xC0);
}

void abi_ClearObjectIfOffRange(void)
{
    abi_ended(clear_object_exit_shim(ClearObjectIfOffRange(cpu.x)));
}

/*
 * @routine 01C0:129 ClearObjectInYAndCopyPlace
 * @run 6E:D929
 * WHAT IT DOES: Empties the object in Y, then gives it the position of the object in X.
 * HOW: ClearObjectInY, then four byte copies: X low, X high, Y low, Y high.
 * WAITS: no.
 * Entry: X the source object, Y the one to empty. Exit: A the last byte copied, N and Z from it. X, Y, C, V kept.
 */
ObjArm ClearObjectInYAndCopyPlace(uint8_t source, uint8_t target)
{
    ObjArm r = { 0 };
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
    r.first = value;
    return r;
}

/* A the last byte copied. */
static Ended clear_in_y_exit_shim(ObjArm r)
{
    return ended_load(r.first);
}

void abi_ClearObjectInYAndCopyPlace(void)
{
    abi_ended(clear_in_y_exit_shim(ClearObjectInYAndCopyPlace(cpu.x, cpu.y)));
}

/*
 * @routine 01BB:0A0 RichterTurnOnPad
 * @run 6D:B0A0
 * WHAT IT DOES: Turns the player round when the pad says so: facing right and the pad's bit 7 held turns him left,
 * facing left and bit 5 held turns him right; the state then takes the next one from a table.
 * HOW: OBJ_FACING_LEFT is the facing; a TST of zPadHeld; the state less 5 indexes the table at $B03B.
 * WAITS: no.
 * Entry: nothing. Exit: no turn: A the facing, N, V, Z from the TST. A turn: A the new state, N and Z from it, Y the
 * old state less 5, C and V from that subtract.
 */
ObjArm RichterTurnOnPad(void)
{
    ObjArm r = { 0 };
    uint8_t state;

    r.first = RD8(OBJ->facing_left[0]);
    if (r.first == 0) {
        r.second = RD8(ZP->pad_held);
        r.arm = PL_TURN_FACING_RIGHT_NOT;
        if ((r.second & 0x80) == 0) {
            return r;
        }
        INC8(OBJ->facing_left[0]);
        r.arm = PL_TURN_FLIPPED_RIGHT;
    } else {
        r.second = RD8(ZP->pad_held);
        r.arm = PL_TURN_FACING_LEFT_NOT;
        if ((r.second & 0x20) == 0) {
            return r;
        }
        WR8(OBJ->facing_left[0], 0);
        r.arm = PL_TURN_FLIPPED_LEFT;
    }
    state = RD8(OBJ->state[0]);
    r.sum = sub_bytes(state, 5, true);
    r.third = rd8((uint16_t)(0xB03B + r.sum.value));
    WR8(OBJ->state[0], r.third);
    return r;
}

/* Without a turn, the facing's load and a TST of the pad; after one, the state's load with Y, C and V of its subtraction. */
static EndedTst richter_turn_on_pad_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == PL_TURN_FACING_RIGHT_NOT) {
        return ended_tst(e, r.second, 0x80);
    }
    if (r.arm == PL_TURN_FACING_LEFT_NOT) {
        return ended_tst(e, r.second, 0x20);
    }
    e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.third), r.sum.carry), r.sum.overflow), r.sum.value);
    return ended_tst_none(e);
}

void abi_RichterTurnOnPad(void)
{
    abi_ended_tst(richter_turn_on_pad_exit_shim(RichterTurnOnPad()));
}

/*
 * @routine 09EB:559 LoadRecordThenRecordEvent
 * @run 70:B559
 * WHAT IT DOES: Loads the object's record, then marks its event when it is one the save remembers.
 * HOW: LoadObjectRecord, then goes on to RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as RecordObjectEvent.
 */
ObjArm LoadRecordThenRecordEvent(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, stored, event, byte;
    bool carry;
    bool marked;

    jsr_push(0xB55C);
    r.first = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.second = nz;
    r.flag = carry;
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    r.third = stored;
    r.fourth = event;
    r.y = byte;
    r.arm = marked ? PL_REC_MARKED : PL_REC_NOT_MARKED;
    return r;
}

/* The record's exit, then the event's: A 0 when none was marked, else the byte stored, Y its offset, C the event's low bits clear. */
static Ended load_record_then_event_exit_shim(ObjArm r)
{
    Ended e = record_exit_shim(r);

    if (r.arm == PL_REC_NOT_MARKED) {
        return ended_then(e, ended_load(0));
    }
    return ended_then(e, ended_with_carry(ended_with_y(ended_load(r.third), r.y), (r.fourth & 7) == 0));
}

void abi_LoadRecordThenRecordEvent(void)
{
    abi_ended(load_record_then_event_exit_shim(LoadRecordThenRecordEvent(cpu.x)));
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

/* The registers the overlay's own code is entered with: the add's A and flags. */
static void read_at_offset_entry_shim(Sum row)
{
    abi_ended(ended_of_sum(row));
}

OffsetRead RichterReadAtOffset(uint8_t offset)
{
    OffsetRead r;
    uint8_t x_lo = RD8(OBJ->x_lo[0]);
    Sum column = add_bytes(x_lo, rd8((uint16_t)(0xB7B1 + offset)), false);
    uint8_t y_lo;

    WR8(ZP->temp00, column.value);
    y_lo = RD8(OBJ->y_lo[0]);
    r.row = add_bytes(y_lo, rd8((uint16_t)(0xB7B2 + offset)), false);
        read_at_offset_entry_shim(r.row); /* the read may run the overlay's own code, entered with this add's A and flags */
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
static ObjArm tile_half_step(uint16_t table, uint8_t first_tile, uint8_t tile, bool right_for_bit)
{
    ObjArm r = { 0 };
    uint8_t column = RD8(ZP->temp00);
    uint8_t step;
    uint8_t row;

    r.y = (uint8_t)((uint8_t)(tile - first_tile) << 1);
    if (((column & 4) != 0) == right_for_bit) {
        r.y++;
    }
    step = rd8((uint16_t)(table + r.y));
    row = RD8(ZP->temp02);
    r.sum = add_bytes(step, row, false);
    WR8(ZP->temp02, r.sum.value);
    return r;
}

/* A $81, C and V from the row's add, Y the table's index. */
Ended tile_half_step_exit_shim(ObjArm r)
{
    return ended_with_y(tile_row_up_exit_shim(r), r.y);
}

/*
 * @routine 10C0:293 TileHalfStepAAt9A93
 * @run 6B:9A93
 * WHAT IT DOES: The tiles $83 and up of this overlay: a step of the row that differs between the tile's halves.
 * HOW: A pair of bytes in the table at $9AEF by tile, the second for the right half.
 * WAITS: no.
 * Entry: A tile, zTemp00 column, zTemp02 row. Exit: A $81; Y the index; C and V from the add.
 */
ObjArm TileHalfStepAAt9A93(uint8_t tile)
{
    return tile_half_step(0x9AEF, 0x83, tile, true);
}

void abi_TileHalfStepAAt9A93(void)
{
    abi_ended(tile_half_step_exit_shim(TileHalfStepAAt9A93(cpu.a)));
}

/*
 * @routine 10C0:2A7 TileHalfStepBAt9AA7
 * @run 6B:9AA7
 * WHAT IT DOES: The tiles $8C and up of this overlay: as TileHalfStepAAt9A93, the other way round.
 * HOW: A pair of bytes in the table at $9AF7 by tile, the second for the left half.
 * WAITS: no.
 * Entry and exit: as TileHalfStepAAt9A93.
 */
ObjArm TileHalfStepBAt9AA7(uint8_t tile)
{
    return tile_half_step(0x9AF7, 0x8C, tile, false);
}

void abi_TileHalfStepBAt9AA7(void)
{
    abi_ended(tile_half_step_exit_shim(TileHalfStepBAt9AA7(cpu.a)));
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
ObjArm MoveObjectUpBySigned(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t amount = RD8(ZP->temp02);
    uint8_t high_operand = (amount & 0x80) ? 0xFF : 0x00;
    uint8_t y_low = RD8(OBJ->y_lo[slot]);
    uint8_t taken = RD8(ZP->temp02);
    Sum low;

    low = sub_bytes(y_low, taken, true);
    WR8(OBJ->y_lo[slot], low.value);
    r.sum = sub_bytes(RD8(OBJ->y_hi[slot]), high_operand, low.carry);
    WR8(OBJ->y_hi[slot], r.sum.value);
    return r;
}

/* The high byte's subtraction. */
static Ended move_object_up_exit_shim(ObjArm r)
{
    return ended_of_sum(r.sum);
}

void abi_MoveObjectUpBySigned(void)
{
    abi_ended(move_object_up_exit_shim(MoveObjectUpBySigned(cpu.x)));
}

/*
 * @routine 0720:4ED LoadRecordThenFrameByKind
 * @run 6B:9CED
 * WHAT IT DOES: Loads the object's record, then gives it frame 0 or 1 of a two byte table by bit 0 of its next kind.
 * HOW: LoadObjectRecord, OBJ_START_ARG and 1, the table at $9CFD; falls into the RTS at $9CFC's neighbour.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N and Z from it, Y the index, C from the load of the record. X kept.
 */
ObjArm LoadRecordThenFrameByKind(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, kind;
    bool carry;

    jsr_push(0x9CF0);
    r.first = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    r.second = nz;
    r.flag = carry;
    kind = RD8(OBJ->start_arg[slot]);
    r.y = (uint8_t)(kind & 1);
    r.third = rd8((uint16_t)(0x9CFD + r.y));
    WR8(OBJ->frame[slot], r.third);
    return r;
}

/* The record's exit, then the frame's load with Y its index. */
static Ended load_record_then_frame_exit_shim(ObjArm r)
{
    return ended_then(record_exit_shim(r), ended_with_y(ended_load(r.third), r.y));
}

void abi_LoadRecordThenFrameByKind(void)
{
    abi_ended(load_record_then_frame_exit_shim(LoadRecordThenFrameByKind(cpu.x)));
}

/*
 * @routine 0E00:092 StoreHitFromTable
 * @run 6B:9892
 * WHAT IT DOES: Gives the object's hit timer OBJ_HIT_TAKEN the byte of OBJ_3828 at the slot in Y.
 * HOW: One read, one write.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: A the byte, N and Z from it.
 */
ObjArm StoreHitFromTable(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->f3828[y]);
    WR8(OBJ->hit_taken[slot], r.first);
    return r;
}

/* A the byte stored. */
static Ended store_hit_exit_shim(ObjArm r)
{
    return ended_load(r.first);
}

void abi_StoreHitFromTable(void)
{
    abi_ended(store_hit_exit_shim(StoreHitFromTable(cpu.x, cpu.y)));
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
ObjArm ReportHitToStage(uint8_t slot, uint8_t y)
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
    abi_ended(store_hit_exit_shim(ReportHitToStage(cpu.x, cpu.y)));
}

/*
 * @routine 0720:076 ReportHitUnlessTwo
 * @run 6B:9876
 * WHAT IT DOES: Reports the hit to the stage unless the kind just stored in OBJ_38D8 is 2.
 * HOW: CMP #2 on A; equal returns, otherwise goes on to ReportHitToStage.
 * WAITS: no.
 * Entry: A the kind, X object, Y the slot. Exit: equal: A and N, Z, C from the compare; otherwise as StoreHitFromTable.
 */
ObjArm ReportHitUnlessTwo(uint8_t kind, uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.first = kind;
    r.arm = PL_REPORT_UNLESS_EQUAL_TWO;
    if (kind == 2) {
        return r;
    }
    r.sub = obj_sub(ReportHitToStage(slot, y));
    r.arm = PL_REPORT_UNLESS_REPORTED;
    return r;
}

/* The compare with 2; when it was not equal, the report's store after it. */
static Ended report_hit_unless_two_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == PL_REPORT_UNLESS_EQUAL_TWO) {
        return e;
    }
    return ended_then(e, store_hit_exit_shim(obj_arm(r.sub)));
}

void abi_ReportHitUnlessTwo(void)
{
    abi_ended(report_hit_unless_two_exit_shim(ReportHitUnlessTwo(cpu.a, cpu.x, cpu.y)));
}

/*
 * @routine 0720:099 HitKindFromBit1
 * @run 6B:9899
 * WHAT IT DOES: Gives the hit slot's kind (OBJ_38D8) 1 or 2 by bit 1 of the object's OBJ_PACE, then reports the hit.
 * HOW: AND #2, LSR, INC, one store, then ReportHitUnlessTwo.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: as ReportHitUnlessTwo.
 */
ObjArm HitKindFromBit1(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t flags = RD8(OBJ->pace[slot]);
    uint8_t kind = (uint8_t)(((flags & 2) >> 1) + 1);

    WR8(OBJ->f38d8[y], kind);
    r.sub = obj_sub(ReportHitUnlessTwo(kind, slot, y));
    r.arm = PL_BIT1_REPORTED;
    return r;
}

void abi_HitKindFromBit1(void)
{
    abi_ended(report_hit_unless_two_exit_shim(obj_arm(HitKindFromBit1(cpu.x, cpu.y).sub)));
}

/*
 * @routine 05C0:0A5 HitKindFromBit7
 * @run 6B:98A5
 * WHAT IT DOES: Gives the hit slot's kind (OBJ_38D8) 1 or 2 by bit 7 of the object's OBJ_PACE; kind 2 also sets the
 * slot's OBJ_3828 to 5; then reports the hit.
 * HOW: AND #$80 rotated twice, AND #1, INC, one store, CMP #2; the report as ReportHitToStage. The kind 2 arm is
 * untraced (135 of 400 cases reach it and the draft stops there), written from the disc's bytes.
 * WAITS: no.
 * Entry: X object, Y the slot. Exit: as StoreHitFromTable.
 */
ObjArm HitKindFromBit7(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t flags = RD8(OBJ->pace[slot]);
    uint8_t kind = (uint8_t)((((flags & 0x80) ? 1 : 0)) + 1);

    r.first = kind;
    WR8(OBJ->f38d8[y], kind);
    r.arm = PL_BIT7_FIRST;
    if (kind == 2) {
        WR8(OBJ->f3828[y], 5);
        r.arm = PL_BIT7_SECOND_OF_TWO;
    }
    r.sub = obj_sub(ReportHitToStage(slot, y));
    return r;
}

/* The compare with 2, or the 5 stored when it was equal, then the report's store after it. */
static Ended hit_kind_from_bit7_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == PL_BIT7_SECOND_OF_TWO) {
        e = ended_load(5);
    }
    return ended_then(e, store_hit_exit_shim(obj_arm(r.sub)));
}

void abi_HitKindFromBit7(void)
{
    abi_ended(hit_kind_from_bit7_exit_shim(HitKindFromBit7(cpu.x, cpu.y)));
}
