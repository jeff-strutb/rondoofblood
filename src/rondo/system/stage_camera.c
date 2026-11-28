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
