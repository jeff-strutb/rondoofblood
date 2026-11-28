/*
 * The stage set-ups InitStage runs from the overlay's bank 7 (sector 0256, bank $6F at $A000): the first section's
 * layer columns drawn across the screen before it is shown.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "layers.h"
#include "layer_columns.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_camera_moved.h"

typedef StageRegs Regs;

enum {
    CAMERA_NARROW_A = 0x0D, CAMERA_NARROW_B = 0x11, COLUMNS_NARROW = 5, COLUMNS_WIDE = 0x0C, WIDE_SHIFT = 0x80,
    FIRST_COLUMN_BLOCKS = 0x90, COLUMN_STEP = 0x20, SECOND_SHIFT = 0xC0, COLUMNS_SECOND = 0x0F, COLUMNS_SPARE = 0x10
};

/* A as the block copy DrawBlockColumn ran through the machine left it. */
static uint8_t copy_a_shim(void)
{
    return cpu.a;
}

/* The registers DrawBlockColumn leaves: A $10, or as the block copy it ran through left it; N 0, Z 1; C, V, Y its. */
static Regs column_end(Regs r, ColumnEnd end)
{
    r.a = end.from_original ? copy_a_shim() : 0x10;
    r.nz = 0;
    r.y = end.y;
    r.c = end.carry;
    r.v = end.overflow;
    return r;
}

/*
 * From $A96B, the set-ups' shared end: entry 0's X step $20, then zSetUpColumnsLeft columns drawn, each with the
 * scroll moved on by its step, and one more column.
 */
static Regs draw_columns(Regs r, uint16_t moved)
{
    AdvanceResult advance;

    r = regs_loaded(r, COLUMN_STEP);
    WR8(SCROLLE->step_x_hi[0], COLUMN_STEP);
    do {
        r.y = 0;
        jsr_push((uint16_t)(0xA974 + moved));
        r = column_end(r, DrawBlockColumnMoved(0, 0, r.c, r.v, moved));
        jsr_pop();
        if (moved != 0) {
            r.x = 0;
            stage_regs_load_shim(r);
            call_routine((uint16_t)(0xA977 + moved), (uint16_t)(0xA6F4 + moved));
            r = stage_regs_from_cpu_shim(r);
        } else {
            jsr_push(0xA977);
            advance = AdvanceScrollX(0);
            jsr_pop();
            r.a = advance.sum.value;
            r.c = advance.sum.carry;
            r.v = advance.sum.overflow;
            r.y = advance.sign;
        }
        r.nz = DEC8(ZP->setup_columns_left);
    } while (r.nz != 0);
    r.y = 0;
    jsr_push((uint16_t)(0xA97F + moved));
    r = column_end(r, DrawBlockColumnMoved(0, 0, r.c, r.v, moved));
    jsr_pop();
    return r;
}

/*
 * @routine 0256:14C StageSetUpScroll
 * @run 6F:A94C
 * WHAT IT DOES: The section's first set-up: layer entry 0 drawn in columns of $10 blocks with the offset, five columns
 * for the narrow camera modes ($0D, $11), else twelve with the scroll first moved back $80.
 * HOW: ScrollColumnBlocks of entry 0 $90; CameraMode; ScrollXAddWrap; then the columns (draw_columns): DrawBlockColumn
 * and AdvanceScrollX by turns, zSetUpColumnsLeft counting them.
 * WAITS: no.
 * Entry: nothing (V the caller's). Exit: X 0; A, Y, N, Z, C and V as the last DrawBlockColumn left them.
 */
StageRegs StageSetUpScroll(StageRegs r)
{
    return StageSetUpScrollMoved(r, 0);
}

/* StageSetUpScroll's body, with its camera module `moved` on from $A000. */
StageRegs StageSetUpScrollMoved(StageRegs r, uint16_t moved)
{
    uint8_t mode;
    Sum shifted;

    r.x = 0;
    r = regs_loaded(r, FIRST_COLUMN_BLOCKS);
    WR8(SCROLLE->column_blocks[0], FIRST_COLUMN_BLOCKS);
    mode = RD8(SCROLLLIM->camera_mode);
    r = regs_loaded(r, mode);
    if (mode == CAMERA_NARROW_A || mode == CAMERA_NARROW_B) {
        r.c = true;                                 /* the CMP that matched */
        r = regs_loaded(r, COLUMNS_NARROW);
        WR8(ZP->setup_columns_left, COLUMNS_NARROW);
        return draw_columns(r, moved);
    }
    r = regs_loaded(r, COLUMNS_WIDE);
    WR8(ZP->setup_columns_left, COLUMNS_WIDE);
    jsr_push((uint16_t)(0xA96B + moved));
    shifted = ScrollXAddWrap(WIDE_SHIFT, 0);
    jsr_pop();
    r = regs_loaded(r, shifted.value);
    r.c = shifted.carry;
    r.v = shifted.overflow;
    return draw_columns(r, moved);
}

void abi_StageSetUpScroll(void)
{
    stage_regs_load_shim(StageSetUpScroll(regs_entry_shim()));
}

/*
 * @routine 0256:1B1 StageSetUpScrollByEffect
 * @run 6F:A9B1
 * WHAT IT DOES: The section's second set-up, when its raster effect has one: layer entry 0's column blocks from
 * EffectColumnBlocks by the effect, the scroll moved back $C0, its screen Y counted on but for effects 6, 7, $0C,
 * $10 and $18, and fifteen columns drawn from $A96B.
 * HOW: zRasterEffect in Y through a chain of CPYs; ScrollXAddWrap; zSetUpColumnsLeft $10 first for effects $0C, $0F
 * and $18, then $0F; the tail JMP into StageSetUpScroll's end.
 * WAITS: no.
 * Entry: nothing. Exit: X 0; A, Y, N, Z, C and V as the last DrawBlockColumn left them.
 */
StageRegs StageSetUpScrollByEffect(StageRegs r)
{
    return StageSetUpScrollByEffectMoved(r, 0);
}

/* StageSetUpScrollByEffect's body, with its camera module `moved` on from $A000. */
StageRegs StageSetUpScrollByEffectMoved(StageRegs r, uint16_t moved)
{
    uint8_t effect, blocks;
    bool count_screen = true;
    Sum shifted;

    effect = RD8(ZP->raster_effect);
    r.y = effect;
    blocks = rd8((uint16_t)(EffectColumnBlocks + moved + effect));
    r.x = 0;
    WR8(SCROLLE->column_blocks[0], blocks);
    jsr_push((uint16_t)(0xA9BE + moved));
    shifted = ScrollXAddWrap(SECOND_SHIFT, 0);
    jsr_pop();
    r = regs_loaded(r, shifted.value);
    r.v = shifted.overflow;
    /* CPY #$0F, #$18, #$0C, #$06, #$07, #$10 in turn, to the first that matches: C from the last one made. */
    {
        static const uint8_t tried[] = { 0x0F, 0x18, 0x0C, 0x06, 0x07, 0x10 };

        for (unsigned i = 0; i < sizeof tried; i++) {
            r = regs_compared(r, effect, tried[i]);
            if (effect == tried[i]) {
                break;
            }
        }
    }
    if (effect == 0x0F || effect == 0x18 || effect == 0x0C) {
        WR8(ZP->setup_columns_left, COLUMNS_SPARE);
    }
    count_screen = !(effect == 0x18 || effect == 0x0C || effect == 0x06 || effect == 0x07 || effect == 0x10);
    if (count_screen) {
        INC8(SCROLLE->y_hi_screen[0]);
    }
    r = regs_loaded(r, COLUMNS_SECOND);
    WR8(ZP->setup_columns_left, COLUMNS_SECOND);
    return draw_columns(r, moved);
}

void abi_StageSetUpScrollByEffect(void)
{
    stage_regs_load_shim(StageSetUpScrollByEffect(regs_entry_shim()));
}
