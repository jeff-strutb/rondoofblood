/*
 * The stage's camera steps (sector 0255, bank $6F at $A000): each section's camera mode (CameraMode) moves the layer
 * entries' scroll by their steps, draws a column of blocks when an entry crosses one, and keeps the parallax layers'
 * steps at fractions of entry 0's. The modes are named by their number in the table at $A0E6; the pieces several
 * modes enter by name of what they do.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "layers.h"
#include "layer_columns.h"
#include "stage_map.h"
#include "stage_regs.h"

typedef StageRegs Regs;

enum { COLUMN_BIT = 0x20, BANDS_THREE = 3, BANDS_FOUR = 4, BANDS_EIGHT = 8, BANDS_Y = 0x60 };

/* JSR or tail JMP to AdvanceScrollX: A the screen, N, Z, C and V from its add, Y the step's sign. X kept. */
static Regs advanced_x(Regs r, AdvanceResult moved)
{
    r = regs_loaded(r, moved.sum.value);
    r.c = moved.sum.carry;
    r.v = moved.sum.overflow;
    r.y = moved.sign;
    return r;
}

static Regs advance_x(Regs r, uint8_t entry, uint16_t ret)
{
    r.x = entry;
    jsr_push(ret);
    r = advanced_x(r, AdvanceScrollX(entry));
    jsr_pop();
    return r;
}

/* ScrollYFromLevelWord by JSR: A its high byte, N and Z from it; X, Y, C and V kept. */
static Regs scroll_y_from_level(Regs r, uint16_t ret)
{
    uint8_t high;

    jsr_push(ret);
    high = ScrollYFromLevelWord();
    jsr_pop();
    return regs_loaded(r, high);
}

/* A as the block copy DrawBlockColumn ran through the machine left it. */
static uint8_t copy_a_shim(void)
{
    return cpu.a;
}

/* DrawBlockColumn reached by a branch: A $10 (or the copy's), N 0, Z 1, C, V and Y as it leaves them, X kept. */
static Regs draw_column_tail(Regs r)
{
    ColumnEnd end = DrawBlockColumn(r.x, r.y, r.c, r.v);

    r.a = end.from_original ? copy_a_shim() : 0x10;
    r.nz = 0;
    r.y = end.y;
    r.c = end.carry;
    r.v = end.overflow;
    return r;
}

/* The camera modes that run into one another, declared ahead of their callers. */
StageRegs CameraWaterWave(StageRegs r);
StageRegs CameraMode15(StageRegs r);
StageRegs CameraMode16(StageRegs r);
StageRegs CameraMode19(StageRegs r);
StageRegs CameraMode17(StageRegs r);
StageRegs CameraMode13(StageRegs r);
StageRegs CameraMode12(StageRegs r);
StageRegs CameraMode10(StageRegs r);
StageRegs CameraMode03(StageRegs r);
StageRegs CameraMode1C(StageRegs r);
StageRegs CameraMode04(StageRegs r);
StageRegs CameraMode06(StageRegs r);
StageRegs ScrollYHeldByX(StageRegs r);
StageRegs ScrollYAtTop(StageRegs r);
StageRegs ScrollYSlideDown(StageRegs r);
StageRegs ScrollYAtBottom(StageRegs r);
StageRegs ScrollYSlideBy84(StageRegs r);
StageRegs ScrollYSlideBy4(StageRegs r);
StageRegs CameraMode07(StageRegs r);
StageRegs CameraMode1A(StageRegs r);
StageRegs CameraMode08(StageRegs r);
StageRegs CameraMode1D(StageRegs r);
StageRegs CameraMode0E(StageRegs r);
StageRegs CameraMode09(StageRegs r);
StageRegs CameraMode00(StageRegs r);
StageRegs CameraMode00Set(StageRegs r);
StageRegs CameraMode00KeepY(StageRegs r);
StageRegs ScrollStepXHalfOf0(StageRegs r);
StageRegs ScrollStepXHalfOf0To1(StageRegs r);
StageRegs ColumnDue(StageRegs r);
StageRegs CameraParallaxAdvance(StageRegs r);

/*
 * @routine 0255:132 ScrollStepXHalf
 * @run 6F:A132
 * WHAT IT DOES: Sets entry Y's step along X to half entry X's, its sign kept.
 * HOW: ASL of the high byte for its sign, then ROR of the high byte and of the low byte into entry Y.
 * WAITS: no.
 * Entry: X the entry halved, Y the one set. Exit: A the low byte written, N and Z from it, C its bit shifted out. X, Y
 * and V kept.
 */
StageRegs ScrollStepXHalf(StageRegs r)
{
    uint8_t value;
    bool c;

    value = RD8(SCROLLE->step_x_hi[r.x]);
    c = (value & 0x80) != 0;
    value = RD8(SCROLLE->step_x_hi[r.x]);
    r.a = (uint8_t)((c ? 0x80 : 0) | (value >> 1));
    c = (value & 0x01) != 0;
    WR8(SCROLLE->step_x_hi[r.y], r.a);
    value = RD8(SCROLLE->step_x_lo[r.x]);
    r = regs_loaded(r, (uint8_t)((c ? 0x80 : 0) | (value >> 1)));
    r.c = (value & 0x01) != 0;
    WR8(SCROLLE->step_x_lo[r.y], r.a);
    return r;
}

void abi_ScrollStepXHalf(void)
{
    stage_regs_load_shim(ScrollStepXHalf(regs_entry_shim()));
}

/*
 * @routine 0255:131 ScrollStepXHalfOf0
 * @run 6F:A131
 * WHAT IT DOES: Sets entry Y's step along X to half entry 0's.
 * HOW: CLX, then ScrollStepXHalf by running into it.
 * WAITS: no.
 * Entry: Y the entry set. Exit: as ScrollStepXHalf, X 0.
 */
StageRegs ScrollStepXHalfOf0(StageRegs r)
{
    r.x = 0;
    return ScrollStepXHalf(r);
}

void abi_ScrollStepXHalfOf0(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf0(regs_entry_shim()));
}

/*
 * @routine 0255:12F ScrollStepXHalfOf0To1
 * @run 6F:A12F
 * WHAT IT DOES: Sets entry 1's step along X to half entry 0's.
 * HOW: LDY #1, then ScrollStepXHalfOf0 by running into it.
 * WAITS: no.
 * Entry: nothing. Exit: as ScrollStepXHalf, X 0, Y 1.
 */
StageRegs ScrollStepXHalfOf0To1(StageRegs r)
{
    r.y = 1;
    return ScrollStepXHalfOf0(r);
}

void abi_ScrollStepXHalfOf0To1(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf0To1(regs_entry_shim()));
}

/*
 * @routine 0255:6DA ScreenMapAtEntry
 * @run 6F:A6DA
 * WHAT IT DOES: Reads the level map's byte for the screen layer entry X has reached.
 * HOW: ScrollXHiScreen into zTemp00 as the column, ScrollYHiScreen as the row, then ScreenMapAt by running into it.
 * WAITS: no.
 * Entry: X the entry. Exit: as ScreenMapAt: A the byte, N and Z from it, C 0, V 0. X and Y kept.
 */
StageRegs ScreenMapAtEntry(StageRegs r)
{
    uint8_t column, row;

    column = RD8(SCROLLE->x_hi_screen[r.x]);
    WR8(ZP->temp00, column);
    row = RD8(SCROLLE->y_hi_screen[r.x]);
    r = regs_loaded(r, ScreenMapAt(row));
    r.c = false;
    r.v = false;
    return r;
}

void abi_ScreenMapAtEntry(void)
{
    stage_regs_load_shim(ScreenMapAtEntry(regs_entry_shim()));
}

/*
 * @routine 0255:784 ColumnDueLeft
 * @run 6F:A784
 * WHAT IT DOES: For an entry moving left: when one less than its scroll X and one less than its band's have bit 5
 * different, it crossed a column, which is drawn.
 * HOW: The two values less one in zTemp09 and A, EOR, AND #$20; the BNE and BRA to DrawBlockColumn.
 * WAITS: no.
 * Entry: X the entry, Y its index for DrawBlockColumn. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
 * DrawBlockColumn.
 */
StageRegs ColumnDueLeft(StageRegs r)
{
    uint8_t band, wanted;

    band = (uint8_t)(RD8(BANDS->bgx_lo[r.x]) - 1);
    WR8(ZP->temp09, band);
    wanted = (uint8_t)(RD8(SCROLL->x_lo[r.x]) - 1);
    r = regs_loaded(r, (uint8_t)((wanted ^ RD8(ZP->temp09)) & COLUMN_BIT));
    if (r.a == 0) {
        return r;
    }
    return draw_column_tail(r);
}

void abi_ColumnDueLeft(void)
{
    stage_regs_load_shim(ColumnDueLeft(regs_entry_shim()));
}

/*
 * @routine 0255:797 ColumnDue
 * @run 6F:A797
 * WHAT IT DOES: Draws the column an entry has crossed, if it has: moving left (Y zero) as ColumnDueLeft; else when
 * its scroll X and its band's differ in bit 5.
 * HOW: TYA; EOR of the two, AND #$20; the BNE to DrawBlockColumn.
 * WAITS: no.
 * Entry: X the entry, Y 0 moving left, 1 moving right. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
 * DrawBlockColumn.
 */
StageRegs ColumnDue(StageRegs r)
{
    uint8_t wanted;

    r = regs_loaded(r, r.y);
    if (r.a == 0) {
        return ColumnDueLeft(r);
    }
    wanted = RD8(SCROLL->x_lo[r.x]);
    r = regs_loaded(r, (uint8_t)((wanted ^ RD8(BANDS->bgx_lo[r.x])) & COLUMN_BIT));
    if (r.a == 0) {
        return r;
    }
    return draw_column_tail(r);
}

void abi_ColumnDue(void)
{
    stage_regs_load_shim(ColumnDue(regs_entry_shim()));
}

/*
 * @routine 0255:6A4 CameraMode00
 * @run 6F:A6A4
 * @stop A6AF
 * WHAT IT DOES: Camera mode 0: entry 0 moves along X by its step, the map byte of its screen is kept, and the column
 * it crosses is drawn.
 * HOW: AdvanceScrollX of entry 0; ScreenMapAtEntry into CameraScreenByte; the tail JMP to ColumnDue with the step's
 * sign in Y.
 * WAITS: no.
 * Entry: nothing. Exit: as ColumnDue, X 0.
 */
StageRegs CameraMode00(StageRegs r)
{
    r = advance_x(r, 0, 0xA6A7);
    jsr_push(0xA6A9);
    r = ScreenMapAtEntry(r);
    jsr_pop();
    wr8(CameraScreenByte, r.a);
    return ColumnDue(r);
}

void abi_CameraMode00(void)
{
    stage_regs_load_shim(CameraMode00(regs_entry_shim()));
}

/*
 * @routine 0255:69E CameraMode00Set
 * @run 6F:A69E
 * WHAT IT DOES: Sets entry 0's column blocks and scroll Y, then camera mode 0.
 * HOW: Two stores, then CameraMode00 by running into it.
 * WAITS: no.
 * Entry: A the blocks a column, Y the scroll Y's low byte. Exit: as CameraMode00.
 */
StageRegs CameraMode00Set(StageRegs r)
{
    WR8(SCROLLE->column_blocks[0], r.a);
    WR8(SCROLL->y_lo[0], r.y);
    return CameraMode00(r);
}

void abi_CameraMode00Set(void)
{
    stage_regs_load_shim(CameraMode00Set(regs_entry_shim()));
}

/*
 * @routine 0255:685 CameraParallaxAdvance
 * @run 6F:A685
 * WHAT IT DOES: Moves the parallax entries 1 to 6 along X by their steps.
 * HOW: AdvanceScrollX for X 1 to 6, the last by a tail JMP.
 * WAITS: no.
 * Entry: nothing. Exit: as AdvanceScrollX of entry 6, X 6.
 */
StageRegs CameraParallaxAdvance(StageRegs r)
{
    static const uint16_t rets[] = { 0xA68A, 0xA68E, 0xA692, 0xA696, 0xA69A };
    uint8_t entry;

    for (entry = 1; entry <= 5; entry++) {
        r = advance_x(r, entry, rets[entry - 1]);
    }
    r.x = 6;
    return advanced_x(r, AdvanceScrollX(6));
}

void abi_CameraParallaxAdvance(void)
{
    stage_regs_load_shim(CameraParallaxAdvance(regs_entry_shim()));
}

/* One parallax entry's step: X step low of entry `from` plus add into entry `to`, the carry into the high byte. */
static Regs parallax_step(Regs r, uint8_t from, uint8_t to, uint8_t add)
{
    Sum sum;

    sum = add_bytes(RD8(SCROLLE->step_x_lo[from]), add, false);
    WR8(SCROLLE->step_x_lo[to], sum.value);
    sum = add_bytes(0, RD8(SCROLLE->step_x_hi[from]), sum.carry);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(SCROLLE->step_x_hi[to], sum.value);
    return r;
}

/* LDX #from, LDY #to, JSR ScrollStepXHalf. */
static Regs half_step(Regs r, uint8_t from, uint8_t to, uint16_t ret)
{
    r.x = from;
    r.y = to;
    jsr_push(ret);
    r = ScrollStepXHalf(r);
    jsr_pop();
    return r;
}

/*
 * @routine 0255:644 CameraParallaxSteps
 * @run 6F:A644
 * WHAT IT DOES: Sets the parallax steps from entry 0's: entries 1, 2 and 3 each half the one before, entries 4, 5 and 6
 * entries 1, 2 and 3 plus $40, $20 and $10 in the low byte; then moves them all.
 * HOW: ScrollStepXHalfOf0To1, the adds with CLA, ADC of the high bytes, ScrollStepXHalf twice; CameraParallaxAdvance by
 * running into it.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraParallaxAdvance.
 */
StageRegs CameraParallaxSteps(StageRegs r)
{
    jsr_push(0xA647);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = parallax_step(r, 1, 4, 0x40);
    r = half_step(r, 1, 2, 0xA65E);
    r = parallax_step(r, 2, 5, 0x20);
    r = half_step(r, 2, 3, 0xA675);
    r = parallax_step(r, 3, 6, 0x10);
    return CameraParallaxAdvance(r);
}

void abi_CameraParallaxSteps(void)
{
    stage_regs_load_shim(CameraParallaxSteps(regs_entry_shim()));
}

/*
 * @routine 0255:636 CameraBandsY
 * @run 6F:A636
 * WHAT IT DOES: Camera mode 0 run with entry 0's scroll Y set for it, then cleared, entry 0's column blocks 8, and the
 * parallax steps.
 * HOW: STA ScrollYLo; CameraMode00; STZ ScrollYLo; then CameraParallaxSteps by running into it.
 * WAITS: no.
 * Entry: A the scroll Y's low byte. Exit: as CameraParallaxSteps.
 */
StageRegs CameraBandsY(StageRegs r)
{
    WR8(SCROLL->y_lo[0], r.a);
    jsr_push(0xA63C);
    r = CameraMode00(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, BANDS_EIGHT);
    WR8(SCROLLE->column_blocks[0], BANDS_EIGHT);
    return CameraParallaxSteps(r);
}

void abi_CameraBandsY(void)
{
    stage_regs_load_shim(CameraBandsY(regs_entry_shim()));
}

/*
 * @routine 0255:631 CameraBandsBlocks
 * @run 6F:A631
 * WHAT IT DOES: Entry 0's column blocks set, then CameraBandsY with scroll Y $60.
 * HOW: STA ScrollColumnBlocks, LDA #$60, running into CameraBandsY.
 * WAITS: no.
 * Entry: A the blocks. Exit: as CameraBandsY.
 */
StageRegs CameraBandsBlocks(StageRegs r)
{
    WR8(SCROLLE->column_blocks[0], r.a);
    return CameraBandsY(regs_loaded(r, BANDS_Y));
}

void abi_CameraBandsBlocks(void)
{
    stage_regs_load_shim(CameraBandsBlocks(regs_entry_shim()));
}

/*
 * @routine 0255:62F CameraMode0A
 * @run 6F:A62F
 * WHAT IT DOES: Camera mode $0A: entry 0 in columns of three blocks at scroll Y $60, then the parallax.
 * HOW: LDA #3, running into CameraBandsBlocks.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraBandsY.
 */
StageRegs CameraMode0A(StageRegs r)
{
    return CameraBandsBlocks(regs_loaded(r, BANDS_THREE));
}

void abi_CameraMode0A(void)
{
    stage_regs_load_shim(CameraMode0A(regs_entry_shim()));
}

/*
 * @routine 0255:62B CameraMode0B
 * @run 6F:A62B
 * WHAT IT DOES: Camera mode $0B: as mode $0A with columns of four blocks.
 * HOW: LDA #4, BRA into CameraBandsBlocks.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraBandsY.
 */
StageRegs CameraMode0B(StageRegs r)
{
    return CameraBandsBlocks(regs_loaded(r, BANDS_FOUR));
}

void abi_CameraMode0B(void)
{
    stage_regs_load_shim(CameraMode0B(regs_entry_shim()));
}

/*
 * @routine 0255:623 CameraMode0C
 * @run 6F:A623
 * WHAT IT DOES: Camera mode $0C: entry 0 in columns of eight blocks at scroll Y 0, then the parallax.
 * HOW: STA ScrollColumnBlocks of 8, CLA (no flags), BRA into CameraBandsY.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraBandsY.
 */
StageRegs CameraMode0C(StageRegs r)
{
    r = regs_loaded(r, BANDS_EIGHT);
    WR8(SCROLLE->column_blocks[0], BANDS_EIGHT);
    r.a = 0;
    return CameraBandsY(r);
}

void abi_CameraMode0C(void)
{
    stage_regs_load_shim(CameraMode0C(regs_entry_shim()));
}

/*
 * @routine 0255:3EB CameraMode0D
 * @run 6F:A3EB
 * WHAT IT DOES: Camera mode $0D: entry 0 moves along X, its scroll Y comes from the level word, and the fixed Y offset
 * is cleared.
 * HOW: AdvanceScrollX of entry 0, ScrollYFromLevelWord, STZ ScrollYOffset.
 * WAITS: no.
 * Entry: nothing. Exit: A the level word's high byte, N and Z from it, X 0, Y the step's sign, C and V from the X add.
 */
StageRegs CameraMode0D(StageRegs r)
{
    r = advance_x(r, 0, 0xA3EF);
    r = scroll_y_from_level(r, 0xA3F2);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode0D(void)
{
    stage_regs_load_shim(CameraMode0D(regs_entry_shim()));
}

/*
 * @routine 0255:3C5 CameraMode0F
 * @run 6F:A3C5
 * WHAT IT DOES: Camera mode $0F: as mode $0D, then entry 2's X step low $40 and the high steps of entries 3 to 6 1, 4,
 * 8 and $0A, and the parallax entries moved.
 * HOW: AdvanceScrollX, ScrollYFromLevelWord, STZ ScrollYOffset, five stores, the tail JMP to CameraParallaxAdvance.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraParallaxAdvance.
 */
StageRegs CameraMode0F(StageRegs r)
{
    static const uint8_t highs[] = { 0x01, 0x04, 0x08, 0x0A };

    r = advance_x(r, 0, 0xA3C9);
    r = scroll_y_from_level(r, 0xA3CC);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    r = regs_loaded(r, 0x40);
    WR8(SCROLLE->step_x_lo[2], 0x40);
    for (uint8_t i = 0; i < 4; i++) {
        r = regs_loaded(r, highs[i]);
        WR8(SCROLLE->step_x_hi[3 + i], highs[i]);
    }
    return CameraParallaxAdvance(r);
}

void abi_CameraMode0F(void)
{
    stage_regs_load_shim(CameraMode0F(regs_entry_shim()));
}

/*
 * @routine 0255:38D CameraMode11
 * @run 6F:A38D
 * WHAT IT DOES: Camera mode $11: entry 0 moves along X, then the parallax steps.
 * HOW: AdvanceScrollX of entry 0, the tail JMP to CameraParallaxSteps.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraParallaxSteps.
 */
StageRegs CameraMode11(StageRegs r)
{
    r = advance_x(r, 0, 0xA391);
    return CameraParallaxSteps(r);
}

void abi_CameraMode11(void)
{
    stage_regs_load_shim(CameraMode11(regs_entry_shim()));
}

/*
 * @routine 0255:26F CameraMode14
 * @run 6F:A26F
 * WHAT IT DOES: Camera mode $14: camera mode 0 with columns of eight blocks at scroll Y 0, entry 4's scroll Y high 1,
 * then the parallax steps.
 * HOW: CameraMode00Set; STZ ScrollYLo; the stores; the tail JMP to CameraParallaxSteps.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraParallaxSteps.
 */
StageRegs CameraMode14(StageRegs r)
{
    r = regs_loaded(r, BANDS_EIGHT);
    r.y = 0;
    jsr_push(0xA275);
    r = CameraMode00Set(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, BANDS_EIGHT);
    WR8(SCROLLE->column_blocks[0], BANDS_EIGHT);
    r = regs_loaded(r, 1);
    WR8(SCROLL->y_hi[4], 1);
    return CameraParallaxSteps(r);
}

void abi_CameraMode14(void)
{
    stage_regs_load_shim(CameraMode14(regs_entry_shim()));
}

/*
 * @routine 0255:145 CameraMode1B
 * @run 6F:A145
 * WHAT IT DOES: Camera mode $1B: camera mode 0 with columns of eight blocks at scroll Y 0; entry 1 at scroll Y $BF
 * with an X step of -$40, moved.
 * HOW: CameraMode00Set; STZ ScrollYLo; the stores; the tail JMP to AdvanceScrollX with X 1.
 * WAITS: no.
 * Entry: nothing. Exit: as AdvanceScrollX of entry 1, X 1.
 */
StageRegs CameraMode1B(StageRegs r)
{
    r = regs_loaded(r, BANDS_EIGHT);
    r.y = 0;
    jsr_push(0xA14B);
    r = CameraMode00Set(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, BANDS_EIGHT);
    WR8(SCROLLE->column_blocks[0], BANDS_EIGHT);
    r = regs_loaded(r, 0xBF);
    WR8(SCROLL->y_lo[1], 0xBF);
    r = regs_loaded(r, 0xC0);
    WR8(SCROLLE->step_x_lo[1], 0xC0);
    r = regs_loaded(r, 0xFF);
    WR8(SCROLLE->step_x_hi[1], 0xFF);
    r.x = 1;
    return advanced_x(r, AdvanceScrollX(1));
}

void abi_CameraMode1B(void)
{
    stage_regs_load_shim(CameraMode1B(regs_entry_shim()));
}

/* JSR or tail JMP to AdvanceScrollY: as advanced_x along Y. */
static Regs advance_y(Regs r, uint8_t entry, uint16_t ret)
{
    r.x = entry;
    jsr_push(ret);
    r = advanced_x(r, AdvanceScrollY(entry));
    jsr_pop();
    return r;
}

/* RowDueAt by JSR (ret) or by the tail JMP (ret 0): the registers it leaves laid over r. */
static Regs row_due(Regs r, uint16_t ret)
{
    RowDue due;

    if (ret != 0) {
        jsr_push(ret);
    }
    due = RowDueAt(r.x, r.y, r.c, r.v);
    if (ret != 0) {
        jsr_pop();
    }
    if (!due.drawn) {
        return regs_loaded(r, due.masked);
    }
    r.a = due.row.a;
    r.nz = 0;
    r.y = due.row.y;
    r.c = due.row.carry;
    r.v = due.row.overflow;
    return r;
}

/* LDA step,X; ROL; ROL; AND #1; TAY: Y the step's sign, C its bit 6, A and N, Z from the sign. */
static Regs sign_of(Regs r, uint8_t step)
{
    bool c_in = r.c;
    uint8_t rotated = (uint8_t)((step << 2) | (c_in ? 0x02 : 0) | (step >> 7));

    r.c = (step & 0x40) != 0;
    r = regs_loaded(r, (uint8_t)(rotated & 0x01));
    r.y = r.a;
    return r;
}

/*
 * @routine 0255:6AF CameraMode01
 * @run 6F:A6AF
 * WHAT IT DOES: Camera modes 1 and $18: entry 0 moves along Y by its step, the map byte of its screen is kept, and
 * the row it crosses is drawn.
 * HOW: AdvanceScrollY of entry 0; ScreenMapAtEntry into CameraScreenByte; the tail JMP to RowDueAt with the step's
 * sign in Y.
 * WAITS: no.
 * Entry: nothing. Exit: as RowDueAt, X 0.
 */
StageRegs CameraMode01(StageRegs r)
{
    r = advance_y(r, 0, 0xA6B2);
    jsr_push(0xA6B4);
    r = ScreenMapAtEntry(r);
    jsr_pop();
    wr8(CameraScreenByte, r.a);
    return row_due(r, 0);
}

void abi_CameraMode01(void)
{
    stage_regs_load_shim(CameraMode01(regs_entry_shim()));
}

/*
 * @routine 0255:6BA CameraMode02
 * @run 6F:A6BA
 * WHAT IT DOES: Camera mode 2: entry 0 moves along X and along Y, the map byte of its screen is kept, and the column
 * and then the row it crosses are drawn, each by the sign of its step.
 * HOW: AdvanceScrollX, AdvanceScrollY, ScreenMapAtEntry into CameraScreenByte; the X step's sign (two ROLs) for
 * ColumnDue, the Y step's for the tail JMP to RowDueAt.
 * WAITS: no.
 * Entry: nothing. Exit: as RowDueAt, X 0.
 */
StageRegs CameraMode02(StageRegs r)
{
    r = advance_x(r, 0, 0xA6BD);
    r = advance_y(r, 0, 0xA6BF);
    jsr_push(0xA6C1);
    r = ScreenMapAtEntry(r);
    jsr_pop();
    wr8(CameraScreenByte, r.a);
    r = sign_of(r, RD8(SCROLLE->step_x_hi[r.x]));
    jsr_push(0xA6CF);
    r = ColumnDue(r);
    jsr_pop();
    r = sign_of(r, RD8(SCROLLE->step_y_hi[r.x]));
    return row_due(r, 0);
}

void abi_CameraMode02(void)
{
    stage_regs_load_shim(CameraMode02(regs_entry_shim()));
}

/*
 * @routine 0255:54A CameraMode00KeepY
 * @run 6F:A54A
 * WHAT IT DOES: Camera mode 0 with entry 0's scroll Y and screen row kept as they were.
 * HOW: ScrollYLo, ScrollYHi and ScrollYHiScreen of entry 0 pushed, CameraMode00, pulled back in turn.
 * WAITS: no.
 * Entry: nothing. Exit: A entry 0's ScrollYLo, N and Z from it; X, Y, C and V as CameraMode00 left them.
 */
StageRegs CameraMode00KeepY(StageRegs r)
{
    uint8_t value;

    value = RD8(SCROLL->y_lo[0]);
    push8(value);
    value = RD8(SCROLL->y_hi[0]);
    push8(value);
    value = RD8(SCROLLE->y_hi_screen[0]);
    push8(value);
    jsr_push(0xA559);
    r = CameraMode00(r);
    jsr_pop();
    value = pull8();
    WR8(SCROLLE->y_hi_screen[0], value);
    value = pull8();
    WR8(SCROLL->y_hi[0], value);
    value = pull8();
    WR8(SCROLL->y_lo[0], value);
    return regs_loaded(r, value);
}

void abi_CameraMode00KeepY(void)
{
    stage_regs_load_shim(CameraMode00KeepY(regs_entry_shim()));
}


/* ColumnDue by JSR. */
static Regs column_due(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = ColumnDue(r);
    jsr_pop();
    return r;
}

/* One parallax entry's step: X step low of entry `from` less `less` into entry `to`, the borrow into the high byte. */
static Regs parallax_less(Regs r, uint8_t from, uint8_t to, uint8_t less)
{
    Sum sum;

    sum = add_bytes(RD8(SCROLLE->step_x_lo[from]), (uint8_t)~less, true);
    WR8(SCROLLE->step_x_lo[to], sum.value);
    sum = add_bytes(RD8(SCROLLE->step_x_hi[from]), 0xFF, sum.carry);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(SCROLLE->step_x_hi[to], sum.value);
    return r;
}

/* Entry `to`'s X step the sum of entries a's and b's: low bytes with CLC, then the high bytes. */
static Regs parallax_sum(Regs r, uint8_t a, uint8_t b, uint8_t to)
{
    Sum low, high;
    uint8_t value;

    value = RD8(SCROLLE->step_x_lo[a]);
    low = add_bytes(value, RD8(SCROLLE->step_x_lo[b]), false);
    WR8(SCROLLE->step_x_lo[to], low.value);
    value = RD8(SCROLLE->step_x_hi[a]);
    high = add_bytes(value, RD8(SCROLLE->step_x_hi[b]), low.carry);
    r = regs_loaded(r, high.value);
    r.c = high.carry;
    r.v = high.overflow;
    WR8(SCROLLE->step_x_hi[to], high.value);
    return r;
}

/*
 * @routine 0255:23D CameraWaterWave
 * @run 6F:A23D
 * WHAT IT DOES: Moves the water line the game wants on a wave: $0C00 scaled by the sine of the wave phase less $40, its high byte plus $0C into the word at RasterWaterWanted, and the phase counted on.
 * HOW: zTemp00-01 the value, ScaleBySine with the phase plus $C0, zProduct2 read twice for the sign and the sum, the phase's add.
 * WAITS: no.
 * Entry: nothing. Exit: A the new phase, N, Z, C and V from its add; Y as ScaleBySine left it (0). X kept.
 */
StageRegs CameraWaterWave(StageRegs r)
{
    Sum sum;
    uint8_t value;

    WR8(ZP->temp00, 0x00);
    r = regs_loaded(r, 0x0C);
    WR8(ZP->temp01, 0x0C);
    sum = add_bytes(rd8(CameraWavePhase), 0xC0, false);
    jsr_push(0xA24E);
    (void)ScaleBySine(sum.value);
    jsr_pop();
    r.y = 0;
    WR8(ZP->temp00, 0);
    value = RD8(ZP->product.hi);
    if (value & 0x80) {
        DEC8(ZP->temp00);
    }
    value = RD8(ZP->product.hi);
    sum = add_bytes(value, 0x0C, false);
    wr8(RasterWaterWanted, sum.value);
    value = RD8(ZP->temp00);
    sum = add_bytes(value, 0x00, sum.carry);
    wr8(LevelYHi, sum.value);
    sum = add_bytes(0x01, rd8(CameraWavePhase), false);
    wr8(CameraWavePhase, sum.value);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    return r;
}

void abi_CameraWaterWave(void)
{
    stage_regs_load_shim(CameraWaterWave(regs_entry_shim()));
}

/*
 * @routine 0255:224 CameraMode15
 * @run 6F:A224
 * WHAT IT DOES: Camera mode $15: the water wave, camera mode 0 at scroll Y 0, the scroll Y from the level word, and entry 0's screen row rounded up to an even one.
 * HOW: CameraWaterWave, STZs, CameraMode00, ScrollYFromLevelWord, INC and AND of ScrollYHiScreen.
 * WAITS: no.
 * Entry: nothing. Exit: A the screen row written, N and Z from it; C, V and Y as ScrollYFromLevelWord and the calls left them.
 */
StageRegs CameraMode15(StageRegs r)
{
    jsr_push(0xA227);
    r = CameraWaterWave(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLL->y_hi[0], 0);
    jsr_push(0xA230);
    r = CameraMode00(r);
    jsr_pop();
    r = scroll_y_from_level(r, 0xA233);
    r = regs_loaded(r, (uint8_t)((uint8_t)(RD8(SCROLLE->y_hi_screen[0]) + 1) & 0xFE));
    WR8(SCROLLE->y_hi_screen[0], r.a);
    return r;
}

void abi_CameraMode15(void)
{
    stage_regs_load_shim(CameraMode15(regs_entry_shim()));
}

/*
 * @routine 0255:210 CameraMode16
 * @run 6F:A210
 * WHAT IT DOES: Camera mode $16: entry 2's screen from entry 0's, its low byte crept along every fourth wave count, then camera mode $15.
 * HOW: TST of zWaveCounter (V from its bit 6), the INC and AND of entry 2's ScrollXLo; running into CameraMode15.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraMode15.
 */
StageRegs CameraMode16(StageRegs r)
{
    uint8_t value, count;

    r = regs_loaded(r, rd8(ScrollXHi));
    WR8(SCROLL->x_hi[2], r.a);
    count = RD8(ZP->wave_counter);
    r.v = (count & 0x40) != 0;
    r.nz = (uint8_t)(count & 0x03);
    if ((count & 0x03) == 0) {
        value = (uint8_t)((uint8_t)(RD8(SCROLL->x_lo[2]) + 1) & 0x3F);
        r = regs_loaded(r, value);
        WR8(SCROLL->x_lo[2], value);
    }
    return CameraMode15(r);
}

void abi_CameraMode16(void)
{
    stage_regs_load_shim(CameraMode16(regs_entry_shim()));
}

/*
 * @routine 0255:167 CameraMode19
 * @run 6F:A167
 * WHAT IT DOES: Camera mode $19: camera mode 0 with three blocks a column at scroll Y $40; entries 4, 5 and 6's X steps are entries 1, 2 and 3's less $40, $20 and $10; entries 1 and 2 are moved with six blocks a column at their own scroll Y and their columns drawn; then entries 3 to 6 move.
 * HOW: CameraMode00, ScrollStepXHalfOf0To1, the subtracts with the borrow into the high bytes, ScrollStepXHalf, AdvanceScrollX and ColumnDue, the last AdvanceScrollX by a tail JMP.
 * WAITS: no.
 * Entry: nothing. Exit: as AdvanceScrollX of entry 6, X 6.
 */
StageRegs CameraMode19(StageRegs r)
{
    r = regs_loaded(r, 0x03);
    WR8(SCROLLE->column_blocks[0], 0x03);
    r = regs_loaded(r, 0x40);
    WR8(SCROLL->y_lo[0], 0x40);
    jsr_push(0xA174);
    r = CameraMode00(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    jsr_push(0xA17F);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = parallax_less(r, 1, 4, 0x40);
    r = half_step(r, 1, 2, 0xA197);
    r = parallax_less(r, 2, 5, 0x20);
    r = half_step(r, 2, 3, 0xA1AF);
    r = parallax_less(r, 3, 6, 0x10);
    r = regs_loaded(r, 0x06);
    WR8(SCROLLE->column_blocks[1], 0x06);
    r = regs_loaded(r, 0xC0);
    WR8(SCROLL->y_lo[1], 0xC0);
    r = advance_x(r, 1, 0xA1CF);
    r = column_due(r, 0xA1D2);
    WR8(SCROLL->y_lo[1], 0);
    r = regs_loaded(r, 0x06);
    WR8(SCROLLE->column_blocks[2], 0x06);
    r = regs_loaded(r, 0xA0);
    WR8(SCROLL->y_lo[2], 0xA0);
    r = advance_x(r, 2, 0xA1E4);
    r = column_due(r, 0xA1E7);
    WR8(SCROLL->y_lo[2], 0);
    r = advance_x(r, 3, 0xA1EF);
    r = advance_x(r, 4, 0xA1F3);
    r = advance_x(r, 5, 0xA1F7);
    r.x = 6;
    return advanced_x(r, AdvanceScrollX(6));
}

void abi_CameraMode19(void)
{
    stage_regs_load_shim(CameraMode19(regs_entry_shim()));
}

/*
 * @routine 0255:1FB CameraMode17
 * @run 6F:A1FB
 * WHAT IT DOES: Camera mode $17: entry 0 moves along both axes with the column and the row it crosses drawn, and ScrollYOffset takes its scroll Y.
 * HOW: AdvanceScrollX, ColumnDue, AdvanceScrollY, RowDueAt with the registers AdvanceScrollY leaves.
 * WAITS: no.
 * Entry: nothing. Exit: A ScrollYLo, N and Z from it; X 0, the rest as RowDueAt left them.
 */
StageRegs CameraMode17(StageRegs r)
{
    r = advance_x(r, 0, 0xA1FF);
    r = column_due(r, 0xA202);
    r = advance_y(r, 0, 0xA206);
    r = row_due(r, 0xA209);
    r = regs_loaded(r, RD8(SCROLL->y_lo[0]));
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    return r;
}

void abi_CameraMode17(void)
{
    stage_regs_load_shim(CameraMode17(regs_entry_shim()));
}

/*
 * @routine 0255:285 CameraMode13
 * @run 6F:A285
 * WHAT IT DOES: Camera mode $13: the parallax steps of entries 3 to 5 halved from entry 0's, entries 2 and 1 the sums of their pairs, entry 1's plus $10; camera mode 0 with three blocks a column at scroll Y $A0, entry 2 moved and drawn, the scroll Y from the level word, and entry 1 moved.
 * HOW: ScrollStepXHalfOf0 and ScrollStepXHalf, two-byte adds, CameraMode00Set, AdvanceScrollX, ColumnDue, ScrollYFromLevelWord, the tail JMP.
 * WAITS: no.
 * Entry: nothing. Exit: as AdvanceScrollX of entry 1, X 1.
 */
StageRegs CameraMode13(StageRegs r)
{
    r.y = 3;
    jsr_push(0xA28A);
    r = ScrollStepXHalfOf0(r);
    jsr_pop();
    r = half_step(r, 3, 4, 0xA291);
    r = half_step(r, 4, 5, 0xA298);
    r = parallax_sum(r, 4, 5, 2);
    r = parallax_sum(r, 3, 5, 1);
    r = parallax_step(r, 1, 1, 0x10);
    r = regs_loaded(r, 0x03);
    r.y = 0xA0;
    jsr_push(0xA2D5);
    r = CameraMode00Set(r);
    jsr_pop();
    r.x = 2;
    r = regs_loaded(r, 0x05);
    WR8(SCROLLE->column_blocks[2], 0x05);
    r = advance_x(r, 2, 0xA2DF);
    r = column_due(r, 0xA2E2);
    r = scroll_y_from_level(r, 0xA2E5);
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[0], 0x01);
    r = regs_loaded(r, RD8(SCROLL->y_lo[0]));
    WR8(SCROLL->y_lo[2], r.a);
    r.x = 1;
    return advanced_x(r, AdvanceScrollX(1));
}

void abi_CameraMode13(void)
{
    stage_regs_load_shim(CameraMode13(regs_entry_shim()));
}

/*
 * @routine 0255:2F5 CameraMode12
 * @run 6F:A2F5
 * WHAT IT DOES: Camera mode $12: entries 3 and 4's steps from entries 0 and 6's, entry 4 with 6's added again, entry 6 stopped; entry 1 half of 0 and entries 5 and 2 halves again; camera mode 0 at its own rows, then entries 3, 1, 4 and 2 moved with their columns.
 * HOW: Two-byte adds, ScrollStepXHalfOf0To1 and ScrollStepXHalf, CameraMode00Set, AdvanceScrollX and ColumnDue, the last by a tail JMP.
 * WAITS: no.
 * Entry: nothing. Exit: as ColumnDue of entry 2.
 */
StageRegs CameraMode12(StageRegs r)
{
    Sum low, high;

    low = add_bytes(RD8(SCROLLE->step_x_lo[0]), RD8(SCROLLE->step_x_lo[6]), false);
    WR8(SCROLLE->step_x_lo[3], low.value);
    WR8(SCROLLE->step_x_lo[4], low.value);
    high = add_bytes(RD8(SCROLLE->step_x_hi[0]), RD8(SCROLLE->step_x_hi[6]), low.carry);
    WR8(SCROLLE->step_x_hi[3], high.value);
    WR8(SCROLLE->step_x_hi[4], high.value);
    r = parallax_sum(r, 4, 6, 4);
    WR8(SCROLLE->step_x_lo[6], 0);
    WR8(SCROLLE->step_x_hi[6], 0);
    jsr_push(0xA32A);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = regs_loaded(r, 0x3F);
    WR8(SCROLL->y_lo[1], 0x3F);
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[1], 0x01);
    r = half_step(r, 1, 5, 0xA33B);
    r = half_step(r, 5, 2, 0xA342);
    r = parallax_sum(r, 5, 2, 2);
    r = regs_loaded(r, 0x02);
    r.y = 0xC0;
    jsr_push(0xA35C);
    r = CameraMode00Set(r);
    jsr_pop();
    r.x = 3;
    r = regs_loaded(r, 0x04);
    WR8(SCROLLE->column_blocks[3], 0x04);
    r = regs_loaded(r, 0x40);
    WR8(SCROLL->y_lo[3], 0x40);
    r = advance_x(r, 3, 0xA36B);
    r = column_due(r, 0xA36E);
    r = regs_loaded(r, 0x47);
    WR8(SCROLL->y_lo[3], 0x47);
    r = advance_x(r, 1, 0xA378);
    r = advance_x(r, 4, 0xA37D);
    r.x = 2;
    r = regs_loaded(r, 0x02);
    WR8(SCROLLE->column_blocks[2], 0x02);
    WR8(SCROLL->y_lo[0], 0);
    r = advance_x(r, 2, 0xA38A);
    return ColumnDue(r);
}

void abi_CameraMode12(void)
{
    stage_regs_load_shim(CameraMode12(regs_entry_shim()));
}

/*
 * @routine 0255:394 CameraMode10
 * @run 6F:A394
 * WHAT IT DOES: Camera mode $10: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 half of 0, entries 4, 2, 5, 3 and 6 set, then the parallax entries moved.
 * HOW: CameraMode00Set, ScrollStepXHalfOf0To1 and ScrollStepXHalf with the low bytes of entries 4 to 6 stored between, the tail JMP to CameraParallaxAdvance.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraParallaxAdvance.
 */
StageRegs CameraMode10(StageRegs r)
{
    r = regs_loaded(r, 0x08);
    r.y = 0;
    jsr_push(0xA39A);
    r = CameraMode00Set(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    jsr_push(0xA3A5);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = regs_loaded(r, 0x80);
    WR8(SCROLLE->step_x_lo[4], 0x80);
    r = half_step(r, 1, 2, 0xA3B1);
    r = regs_loaded(r, 0x40);
    WR8(SCROLLE->step_x_lo[5], 0x40);
    r = half_step(r, 2, 3, 0xA3BD);
    r = regs_loaded(r, 0x20);
    WR8(SCROLLE->step_x_lo[6], 0x20);
    return CameraParallaxAdvance(r);
}

void abi_CameraMode10(void)
{
    stage_regs_load_shim(CameraMode10(regs_entry_shim()));
}

/*
 * @routine 0255:3F6 CameraMode03
 * @run 6F:A3F6
 * WHAT IT DOES: Camera mode 3: camera mode 0 with one block a column at scroll Y $C0; entry 3 half of 0 and entry 4 of 3, entry 2 their sum, entry 2 moved and drawn; entry 1 at half entry 0's scroll X, row $100, and ScrollYOffset cleared.
 * HOW: CameraMode00Set, the halves, a two-byte add, AdvanceScrollX, ColumnDue, then LSR of ScrollXHi and ROR of ScrollXLo into entry 1.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it, C ScrollXLo's bit 0; X, Y and V as the calls left them.
 */
StageRegs CameraMode03(StageRegs r)
{
    uint8_t value;
    bool c;

    r = regs_loaded(r, 0x01);
    r.y = 0xC0;
    jsr_push(0xA3FD);
    r = CameraMode00Set(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    r.y = 3;
    jsr_push(0xA40A);
    r = ScrollStepXHalfOf0(r);
    jsr_pop();
    r = half_step(r, 3, 4, 0xA411);
    r = parallax_sum(r, 3, 4, 2);
    r = regs_loaded(r, 0x06);
    WR8(SCROLLE->column_blocks[2], 0x06);
    r = advance_x(r, 2, 0xA42E);
    r = column_due(r, 0xA431);
    value = rd8(ScrollXHi);
    c = (value & 0x01) != 0;
    WR8(SCROLL->x_hi[1], (uint8_t)(value >> 1));
    value = rd8(ScrollXLo);
    r.c = (value & 0x01) != 0;
    WR8(SCROLL->x_lo[1], (uint8_t)((value >> 1) | (c ? 0x80 : 0)));
    WR8(SCROLL->y_lo[1], 0);
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[1], 0x01);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode03(void)
{
    stage_regs_load_shim(CameraMode03(regs_entry_shim()));
}

/*
 * @routine 0255:44B CameraMode1C
 * @run 6F:A44B
 * WHAT IT DOES: Camera mode $1C: camera mode 0 at scroll Y $100, the scroll Y from the level word when it is not zero, and ScrollYOffset 1.
 * HOW: STZ and store of the scroll Y, CameraMode00, ORA of the word's two bytes, ScrollYFromLevelWord.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V as the calls left them.
 */
StageRegs CameraMode1C(StageRegs r)
{
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[0], 0x01);
    jsr_push(0xA456);
    r = CameraMode00(r);
    jsr_pop();
    r = regs_loaded(r, rd8(RasterWaterWanted));
    r = regs_loaded(r, (uint8_t)(r.a | rd8(LevelYHi)));
    if (r.a != 0) {
        r = scroll_y_from_level(r, 0xA461);
    }
    r = regs_loaded(r, 0x01);
    WR8(SCROLLLIM->scroll_y_offset, 0x01);
    return r;
}

void abi_CameraMode1C(void)
{
    stage_regs_load_shim(CameraMode1C(regs_entry_shim()));
}

/*
 * @routine 0255:467 CameraMode04
 * @run 6F:A467
 * WHAT IT DOES: Camera modes 4 and 5: camera mode 0 at scroll Y 0, then the scroll Y from the level word and ScrollYOffset cleared.
 * HOW: Two STZs, CameraMode00, ScrollYFromLevelWord, STZ.
 * WAITS: no.
 * Entry: nothing. Exit: A the level word's high byte, N and Z from it; X, Y, C and V as the calls left them.
 */
StageRegs CameraMode04(StageRegs r)
{
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLL->y_hi[0], 0);
    jsr_push(0xA470);
    r = CameraMode00(r);
    jsr_pop();
    r = scroll_y_from_level(r, 0xA473);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode04(void)
{
    stage_regs_load_shim(CameraMode04(regs_entry_shim()));
}

/*
 * @routine 0255:477 CameraMode06
 * @run 6F:A477
 * WHAT IT DOES: Camera mode 6: band 1's scroll X from entry 1's, camera mode 0, then entry 1 at half entry 0's scroll X and ScrollYOffset cleared.
 * HOW: A copy to ScrollBgxLo+1, CameraMode00, LSR and ROR of the screen and low byte.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte written, N and Z from it, C ScrollXLo's bit 0; X, Y and V as CameraMode00 left them.
 */
StageRegs CameraMode06(StageRegs r)
{
    uint8_t value;
    bool c;

    r = regs_loaded(r, RD8(SCROLL->x_lo[1]));
    WR8(BANDS->bgx_lo[1], r.a);
    jsr_push(0xA480);
    r = CameraMode00(r);
    jsr_pop();
    value = rd8(ScrollXHi);
    c = (value & 0x01) != 0;
    WR8(SCROLL->x_hi[1], (uint8_t)(value >> 1));
    value = rd8(ScrollXLo);
    r = regs_loaded(r, (uint8_t)((value >> 1) | (c ? 0x80 : 0)));
    r.c = (value & 0x01) != 0;
    WR8(SCROLL->x_lo[1], r.a);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode06(void)
{
    stage_regs_load_shim(CameraMode06(regs_entry_shim()));
}

/*
 * @routine 0255:4C9 ScrollYHeldByX
 * @run 6F:A4C9
 * WHAT IT DOES: Holds the scroll Y at the top until entry 0's scroll X passes $F8, past which it slides down (ScrollYSlideDown).
 * HOW: CMP #$F8 of ScrollXLo; the BCS, else running into ScrollYAtTop.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it goes to; entered with C from the compare.
 */
StageRegs ScrollYHeldByX(StageRegs r)
{
    r = regs_loaded(r, rd8(ScrollXLo));
    r = regs_compared(r, r.a, 0xF8);
    if (r.c) {
        return ScrollYSlideDown(r);
    }
    return ScrollYAtTop(r);
}

void abi_ScrollYHeldByX(void)
{
    stage_regs_load_shim(ScrollYHeldByX(regs_entry_shim()));
}

/*
 * @routine 0255:4D0 ScrollYAtTop
 * @run 6F:A4D0
 * WHAT IT DOES: The scroll Y and its screen cleared, with ScrollYOffset.
 * HOW: Four STZs.
 * WAITS: no.
 * Entry: nothing. Exit: every register and flag kept.
 */
StageRegs ScrollYAtTop(StageRegs r)
{
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    WR8(SCROLL->y_hi[0], 0);
    WR8(SCROLLE->y_hi_screen[0], 0);
    return r;
}

void abi_ScrollYAtTop(void)
{
    stage_regs_load_shim(ScrollYAtTop(regs_entry_shim()));
}

/*
 * @routine 0255:4DD ScrollYSlideDown
 * @run 6F:A4DD
 * WHAT IT DOES: The scroll Y half of entry 0's scroll X past $F8, ScrollYOffset the same, on the top screen.
 * HOW: SEC, SBC #$F8, LSR into ScrollYLo and ScrollYOffset.
 * WAITS: no.
 * Entry: A entry 0's scroll X. Exit: A the scroll Y, N and Z from it, C bit 0 of the difference, V from the subtract. X and Y kept. Entered with A the scroll X.
 */
StageRegs ScrollYSlideDown(StageRegs r)
{
    Sum sum;

    sum = add_bytes(r.a, (uint8_t)~0xF8, true);
    r.v = sum.overflow;
    r.c = (sum.value & 0x01) != 0;
    r = regs_loaded(r, (uint8_t)(sum.value >> 1));
    WR8(SCROLL->y_lo[0], r.a);
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    WR8(SCROLL->y_hi[0], 0);
    WR8(SCROLLE->y_hi_screen[0], 0);
    return r;
}

void abi_ScrollYSlideDown(void)
{
    stage_regs_load_shim(ScrollYSlideDown(regs_entry_shim()));
}

/*
 * @routine 0255:4EE ScrollYAtBottom
 * @run 6F:A4EE
 * WHAT IT DOES: The scroll Y $100 (screen row 1) with ScrollYOffset cleared.
 * HOW: Stores of 1, STZs.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V kept.
 */
StageRegs ScrollYAtBottom(StageRegs r)
{
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[0], 0x01);
    WR8(SCROLLE->y_hi_screen[0], 0x01);
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_ScrollYAtBottom(void)
{
    stage_regs_load_shim(ScrollYAtBottom(regs_entry_shim()));
}

/*
 * @routine 0255:4FD ScrollYSlideBy84
 * @run 6F:A4FD
 * WHAT IT DOES: The scroll Y half of entry 0's scroll X plus $84, at the bottom (ScrollYAtBottom) past $F8.
 * HOW: CMP #$F8; the BCS to ScrollYAtBottom; LSR, CLC, ADC #$84.
 * WAITS: no.
 * Entry: nothing. Exit: past $F8 as ScrollYAtBottom; else A the scroll Y, N, Z, C and V from the add. X and Y kept.
 */
StageRegs ScrollYSlideBy84(StageRegs r)
{
    Sum sum;

    r = regs_loaded(r, rd8(ScrollXLo));
    r = regs_compared(r, r.a, 0xF8);
    if (r.c) {
        return ScrollYAtBottom(r);
    }
    sum = add_bytes((uint8_t)(r.a >> 1), 0x84, false);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(SCROLL->y_lo[0], r.a);
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    WR8(SCROLL->y_hi[0], 0);
    WR8(SCROLLE->y_hi_screen[0], 0);
    return r;
}

void abi_ScrollYSlideBy84(void)
{
    stage_regs_load_shim(ScrollYSlideBy84(regs_entry_shim()));
}

/*
 * @routine 0255:515 ScrollYSlideBy4
 * @run 6F:A515
 * WHAT IT DOES: The scroll Y half of entry 0's scroll X plus 4, on the top screen.
 * HOW: LSR, CLC, ADC #4.
 * WAITS: no.
 * Entry: nothing. Exit: A the scroll Y, N, Z, C and V from the add. X and Y kept.
 */
StageRegs ScrollYSlideBy4(StageRegs r)
{
    Sum sum;

    r = regs_loaded(r, rd8(ScrollXLo));
    sum = add_bytes((uint8_t)(r.a >> 1), 0x04, false);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(SCROLL->y_lo[0], r.a);
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    WR8(SCROLL->y_hi[0], 0);
    WR8(SCROLLE->y_hi_screen[0], 0);
    return r;
}

void abi_ScrollYSlideBy4(void)
{
    stage_regs_load_shim(ScrollYSlideBy4(regs_entry_shim()));
}

/*
 * @routine 0255:492 CameraMode07
 * @run 6F:A492
 * @stop A4C9
 * WHAT IT DOES: Camera mode 7: camera mode 0 with the scroll Y kept, then the scroll Y by the screen entry 0 is on: below 3 at the top, 3 held until past $F8, 4 plus 4, 5 plus $84, 6 at the bottom, else ScrollYOffset takes it.
 * HOW: As CameraMode00KeepY inline; CPYs of ScrollXHiScreen in A in turn; the branches into the shared pieces.
 * WAITS: no.
 * Entry: nothing. Exit: as the piece it goes to; past screen 6 A the scroll Y, N and Z from it, C from the compare with 6.
 */
StageRegs CameraMode07(StageRegs r)
{
    uint8_t value;

    value = RD8(SCROLL->y_lo[0]);
    push8(value);
    value = RD8(SCROLL->y_hi[0]);
    push8(value);
    value = RD8(SCROLLE->y_hi_screen[0]);
    push8(value);
    jsr_push(0xA4A1);
    r = CameraMode00(r);
    jsr_pop();
    value = pull8();
    WR8(SCROLLE->y_hi_screen[0], value);
    value = pull8();
    WR8(SCROLL->y_hi[0], value);
    value = pull8();
    WR8(SCROLL->y_lo[0], value);
    r = regs_loaded(r, rd8(ScrollXHiScreen));
    r = regs_compared(r, r.a, 0x03);
    if (r.a == 0x03) {
        return ScrollYHeldByX(r);
    }
    if (!r.c) {
        return ScrollYAtTop(r);
    }
    r = regs_compared(r, r.a, 0x04);
    if (r.a == 0x04) {
        return ScrollYSlideBy4(r);
    }
    r = regs_compared(r, r.a, 0x05);
    if (r.a == 0x05) {
        return ScrollYSlideBy84(r);
    }
    r = regs_compared(r, r.a, 0x06);
    if (r.a == 0x06) {
        return ScrollYAtBottom(r);
    }
    r = regs_loaded(r, RD8(SCROLL->y_lo[0]));
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    return r;
}

void abi_CameraMode07(void)
{
    stage_regs_load_shim(CameraMode07(regs_entry_shim()));
}

/*
 * @routine 0255:529 CameraMode1A
 * @run 6F:A529
 * WHAT IT DOES: Camera mode $1A: as mode 7 with the screens four further along.
 * HOW: CameraMode00KeepY, then the compares with 7 to $0A.
 * WAITS: no.
 * Entry: nothing. Exit: as the piece it goes to; past screen $0A A the scroll Y, N and Z from it.
 */
StageRegs CameraMode1A(StageRegs r)
{
    jsr_push(0xA52B);
    r = CameraMode00KeepY(r);
    jsr_pop();
    r = regs_loaded(r, rd8(ScrollXHiScreen));
    r = regs_compared(r, r.a, 0x07);
    if (r.a == 0x07) {
        return ScrollYHeldByX(r);
    }
    if (!r.c) {
        return ScrollYAtTop(r);
    }
    r = regs_compared(r, r.a, 0x08);
    if (r.a == 0x08) {
        return ScrollYSlideBy4(r);
    }
    r = regs_compared(r, r.a, 0x09);
    if (r.a == 0x09) {
        return ScrollYSlideBy84(r);
    }
    r = regs_compared(r, r.a, 0x0A);
    if (r.a == 0x0A) {
        return ScrollYAtBottom(r);
    }
    r = regs_loaded(r, RD8(SCROLL->y_lo[0]));
    WR8(SCROLLLIM->scroll_y_offset, r.a);
    return r;
}

void abi_CameraMode1A(void)
{
    stage_regs_load_shim(CameraMode1A(regs_entry_shim()));
}

/*
 * @routine 0255:566 CameraMode08
 * @run 6F:A566
 * WHAT IT DOES: Camera mode 8: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 at row $200 and half of entry 0's scroll X, with its step the distance to there, moved and drawn.
 * HOW: CameraMode00Set, the row stores, ScrollXLo into the step's high byte, ROR from the screen's bit 0, SBC of entry 1's scroll X.
 * WAITS: no.
 * Entry: nothing. Exit: as ColumnDue of entry 1; ScrollYOffset cleared.
 */
StageRegs CameraMode08(StageRegs r)
{
    Sum sum;
    bool c;

    r = regs_loaded(r, 0x08);
    r.y = 0;
    jsr_push(0xA56C);
    r = CameraMode00Set(r);
    jsr_pop();
    r = regs_loaded(r, 0x02);
    WR8(SCROLLE->y_hi_screen[1], 0x02);
    WR8(SCROLL->y_hi[1], 0);
    r = regs_loaded(r, 0x10);
    WR8(SCROLLE->column_blocks[0], 0x10);
    r = regs_loaded(r, rd8(ScrollXLo));
    WR8(SCROLLE->step_x_hi[1], r.a);
    r = regs_loaded(r, rd8(ScrollXHiScreen));
    c = (r.a & 0x01) != 0;
    r = regs_loaded(r, (uint8_t)(r.a >> 1));
    r.c = ROR8(SCROLLE->step_x_hi[1], c);
    r = regs_loaded(r, RD8(SCROLLE->step_x_hi[1]));
    sum = add_bytes(r.a, (uint8_t)~RD8(SCROLL->x_lo[1]), true);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(SCROLLE->step_x_hi[1], sum.value);
    WR8(SCROLLE->step_x_lo[1], 0);
    r = regs_loaded(r, 0x04);
    WR8(SCROLLE->column_blocks[1], 0x04);
    r = advance_x(r, 1, 0xA59D);
    r = column_due(r, 0xA5A0);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode08(void)
{
    stage_regs_load_shim(CameraMode08(regs_entry_shim()));
}

/*
 * @routine 0255:5A4 CameraMode1D
 * @run 6F:A5A4
 * WHAT IT DOES: Camera mode $1D: camera mode 0 with eight blocks a column at scroll Y 0, then entry 1 at half entry 0's scroll X, its screen one past the half.
 * HOW: Stores, CameraMode00, INC, LSR, INC of ScrollXHiScreen and ROR of ScrollXLo.
 * WAITS: no.
 * Entry: nothing. Exit: A the low byte written, N and Z from it, C ScrollXLo's bit 0; X, Y and V as CameraMode00 left them.
 */
StageRegs CameraMode1D(StageRegs r)
{
    uint8_t value;
    bool c;

    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    r.a = 0;
    WR8(SCROLL->y_lo[0], 0);
    jsr_push(0xA5B0);
    r = CameraMode00(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    value = (uint8_t)(rd8(ScrollXHiScreen) + 1);
    c = (value & 0x01) != 0;
    value = (uint8_t)((value >> 1) + 1);
    WR8(SCROLL->x_hi[1], value);
    value = rd8(ScrollXLo);
    r = regs_loaded(r, (uint8_t)((value >> 1) | (c ? 0x80 : 0)));
    r.c = (value & 0x01) != 0;
    WR8(SCROLL->x_lo[1], r.a);
    return r;
}

void abi_CameraMode1D(void)
{
    stage_regs_load_shim(CameraMode1D(regs_entry_shim()));
}

/*
 * @routine 0255:5C9 CameraMode0E
 * @run 6F:A5C9
 * WHAT IT DOES: Camera mode $0E: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 at row $100, half entry 0's speed, moved and drawn.
 * HOW: CameraMode00Set, the rows, ScrollStepXHalfOf0To1, AdvanceScrollX, ColumnDue.
 * WAITS: no.
 * Entry: nothing. Exit: as ColumnDue of entry 1; ScrollYOffset cleared.
 */
StageRegs CameraMode0E(StageRegs r)
{
    r = regs_loaded(r, 0x08);
    r.y = 0;
    jsr_push(0xA5CF);
    r = CameraMode00Set(r);
    jsr_pop();
    r = regs_loaded(r, 0x01);
    WR8(SCROLLE->y_hi_screen[1], 0x01);
    WR8(SCROLL->y_hi[1], 0x01);
    r = regs_loaded(r, 0x10);
    WR8(SCROLLE->column_blocks[0], 0x10);
    jsr_push(0xA5DF);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = regs_loaded(r, 0x05);
    WR8(SCROLLE->column_blocks[1], 0x05);
    r = advance_x(r, 1, 0xA5E9);
    r = column_due(r, 0xA5EC);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode0E(void)
{
    stage_regs_load_shim(CameraMode0E(regs_entry_shim()));
}

/*
 * @routine 0255:5F0 CameraMode09
 * @run 6F:A5F0
 * WHAT IT DOES: Camera mode 9: camera mode 0 with eight blocks a column at scroll Y 0; entry 2 a quarter of entry 0's speed, entries 1 and 2 at row $100 and moved, the scroll Y from the level word.
 * HOW: CameraMode00Set, ScrollStepXHalf twice, AdvanceScrollX of 1 and 2 (INX between), ScrollYFromLevelWord.
 * WAITS: no.
 * Entry: nothing. Exit: A the level word's high byte, N and Z from it; X 2, Y, C and V as the calls left them.
 */
StageRegs CameraMode09(StageRegs r)
{
    r = regs_loaded(r, 0x08);
    r.y = 0;
    jsr_push(0xA5F6);
    r = CameraMode00Set(r);
    jsr_pop();
    WR8(SCROLL->y_lo[0], 0);
    r = regs_loaded(r, 0x08);
    WR8(SCROLLE->column_blocks[0], 0x08);
    jsr_push(0xA601);
    r = ScrollStepXHalfOf0To1(r);
    jsr_pop();
    r = half_step(r, 1, 2, 0xA608);
    WR8(SCROLL->y_hi[0], 0);
    r = regs_loaded(r, 0x01);
    WR8(SCROLL->y_hi[1], 0x01);
    WR8(SCROLL->y_hi[2], 0x01);
    r = advance_x(r, 1, 0xA618);
    r = advance_x(r, 2, 0xA61C);
    r = scroll_y_from_level(r, 0xA61F);
    WR8(SCROLLLIM->scroll_y_offset, 0);
    return r;
}

void abi_CameraMode09(void)
{
    stage_regs_load_shim(CameraMode09(regs_entry_shim()));
}

/* A JSR to the routine loaded at run: the registers as they stand, and what it leaves. */
static Regs call_run(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* JMP (CameraModes,X) at $A0E3: the table's word read, low byte then high, and the C of the routine there run. */
static Regs jump_to_mode(Regs r)
{
    uint8_t lo = rd8((uint16_t)(CameraModes + r.x));
    uint8_t hi = rd8((uint16_t)(CameraModes + r.x + 1));

    stage_regs_load_shim(r);
    run_routine_from(0xA0E3, (uint16_t)(lo | (hi << 8)));
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 0255:0DB StageCameraModeDispatch
 * @run 6F:A0DB
 * @stop A0E6
 * WHAT IT DOES: Runs the section's camera mode; a mode of $80 or more does nothing.
 * HOW: CameraMode doubled; the bit out of the double returns; else TAX and the jump through CameraModes.
 * WAITS: no.
 * Entry: nothing. Exit, mode $80 up: A the mode doubled, N and Z from it, C 1; else as the mode leaves them, entered
 * with A and X the mode doubled, C 0.
 */
StageRegs StageCameraModeDispatch(StageRegs r)
{
    uint8_t mode = RD8(SCROLLLIM->camera_mode);

    r.c = (mode & 0x80) != 0;
    r = regs_loaded(r, (uint8_t)(mode << 1));
    if (r.c) {
        return r;
    }
    r.x = r.a;
    return jump_to_mode(r);
}

void abi_StageCameraModeDispatch(void)
{
    stage_regs_load_shim(StageCameraModeDispatch(regs_entry_shim()));
}

/*
 * @routine 0255:08D CameraStep
 * @run 6F:A08D
 * WHAT IT DOES: The camera's frame: the layers' speeds cleared, the section's camera mode run, the speeds taken from
 * the bands, and in the camera modes 4, 5, 7, 9, $0D, $13, $15, $16 and $1C the player moved against the scroll
 * along Y.
 * HOW: ClearScrollSpeeds, StageCameraModeDispatch, ScrollSpeedsFromBands, a CMP chain on CameraMode read once; the
 * speed's negative added to the player's Y, sign extended through zTemp00.
 * WAITS: no.
 * Entry: nothing. Exit: another mode: A CameraMode, N, Z and C from the last compare, X, Y and V as the calls left
 * them; moving: A the player's Y high byte, N, Z, C and V from its add, Y the negated speed.
 */
StageRegs CameraStep(StageRegs r)
{
    static const uint8_t modes[] = { 0x04, 0x05, 0x07, 0x09, 0x0D, 0x13, 0x15, 0x16, 0x1C };
    uint8_t mode, speed, value;
    Sum sum;
    unsigned i;

    r = call_run(r, 0xA090, 0xA067);
    jsr_push(0xA093);
    r = StageCameraModeDispatch(r);
    jsr_pop();
    r = call_run(r, 0xA096, 0xA073);
    mode = RD8(SCROLLLIM->camera_mode);
    r = regs_loaded(r, mode);
    for (i = 0; i < sizeof modes; i++) {
        r = regs_compared(r, mode, modes[i]);
        if (mode == modes[i]) {
            break;
        }
    }
    if (i == sizeof modes) {
        return r;
    }
    WR8(ZP->temp00, 0);
    speed = (uint8_t)(RD8(SCROLLE->speed_y[0]) ^ 0xFF);
    speed++;
    r.y = speed;
    r = regs_loaded(r, speed);
    if (speed & 0x80) {
        DEC8(ZP->temp00);
    }
    sum = add_bytes(speed, RD8(OBJ->y_lo[0]), false);
    WR8(OBJ->y_lo[0], sum.value);
    value = RD8(ZP->temp00);
    sum = add_bytes(value, RD8(OBJ->y_hi[0]), sum.carry);
    WR8(OBJ->y_hi[0], sum.value);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    return r;
}

void abi_CameraStep(void)
{
    stage_regs_load_shim(CameraStep(regs_entry_shim()));
}
