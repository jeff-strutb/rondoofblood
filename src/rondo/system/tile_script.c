/*
 * Tile scripts: byte code that draws text and background tiles straight into
 * VRAM. The stage overlay holds them. A tile is written as a low byte and the
 * current attribute byte; the script steps through RAM by a pointer in zTemp02.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    TILE_OP_LITERAL_WORD = 0x00, TILE_OP_REPEATED = 0x20, TILE_OP_ASCENDING = 0x40,
    TILE_OP_ALTERNATE_UP = 0x60, TILE_OP_ALTERNATE_DOWN = 0x80, TILE_OP_PALETTE = 0xC0,
    TILE_OP_ATTRIBUTE_LOW = 0xD0, TILE_OP_JUMP = 0xE0, TILE_OP_NEXT_LINE = 0xF0,
    TILE_OP_SET_ADDRESS = 0xFE, TILE_OP_END = 0xFF,
    TILES_PER_LINE = 0x40, VDC_DATA_PORT_SELECTED = 2
};

/* What a script's run did last, for what its caller is left with. */
typedef struct {
    uint8_t last_op;      /* the last op that counted or set the attribute, or TILE_OP_END */
    uint8_t jump_low;     /* the low byte of the last jump */
    bool stepped_line;
    bool line_overflow;   /* V from the last line step's high byte add */
} TileRun;

static TileRun run_tile_script(void);

/* Y as the last op left it: a count run down to zero, a shifted nibble or a jump's low. */
static uint8_t tile_run_y_shim(TileRun run, uint8_t y_before)
{
    if (run.last_op == TILE_OP_END)
        return y_before;
    if (run.last_op == TILE_OP_JUMP)
        return run.jump_low;
    if (run.last_op >= TILE_OP_ATTRIBUTE_LOW)
        return run.last_op & 0x0F;
    if (run.last_op >= TILE_OP_PALETTE)
        return (uint8_t)(run.last_op << 4);
    return 0;
}

/* Every way out of the interpreter: the compare with $FF, and Y and V as the ops left them. */
static void abi_exit_tile_run(TileRun run, uint8_t y_before)
{
    cpu.a = TILE_OP_END;
    cpu.y = tile_run_y_shim(run, y_before);
    abi_nz(0);
    abi_flag(P_C, true);
    if (run.stepped_line)
        abi_flag(P_V, run.line_overflow);
}

/*
 * @routine 0047:550 AdvanceTileScript
 * @run 6A:6550
 * WHAT IT DOES: Moves the script pointer on one byte.
 * HOW: A 16-bit increment in zero page.
 * WAITS: no.
 * Entry: zTemp02-03 pointer. Exit: N and Z from the last byte stepped. A, X, Y, C,
 * V kept.
 */
uint8_t AdvanceTileScript(void)
{
    uint8_t low = INC8(ZP->temp02);

    if (low == 0) {
        low = INC8(ZP->temp03);
    }
    return low;
}

void abi_AdvanceTileScript(void)
{
    abi_nz(AdvanceTileScript());
}

/*
 * @routine 0047:513 WriteTile
 * @run 6A:6513
 * WHAT IT DOES: Writes one tile to VRAM.
 * HOW: The tile as the low byte, zTemp04 as the attribute byte. The tile is kept on the
 * stack while it is written.
 * WAITS: no.
 * Entry: A tile, VDC on its data port. Exit: A kept, N and Z from it. X, Y, C, V
 * kept.
 */
uint8_t WriteTile(uint8_t tile)
{
    uint8_t attribute;

    push8(tile);
    WR8(VDC->data_lo, tile);
    attribute = RD8(ZP->temp04);
    WR8(VDC->data_hi, attribute);
    return pull8();
}

void abi_WriteTile(void)
{
    cpu.a = WriteTile(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:509 WriteTileAscending
 * @run 6A:6509
 * WHAT IT DOES: Writes a tile and gives the next one up.
 * HOW: WriteTile, then adds one.
 * WAITS: no.
 * Entry: A tile. Exit: A plus one, N and Z from it. X, Y, C, V kept.
 */
uint8_t WriteTileAscending(uint8_t tile)
{
    uint8_t written;

    jsr_push(0x650C);
    written = WriteTile(tile);
    jsr_pop();
    return (uint8_t)(written + 1);
}

void abi_WriteTileAscending(void)
{
    cpu.a = WriteTileAscending(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:50E WriteTileDescending
 * @run 6A:650E
 * WHAT IT DOES: Writes a tile and gives the next one down.
 * HOW: WriteTile, then subtracts one.
 * WAITS: no.
 * Entry: A tile. Exit: A minus one, N and Z from it. X, Y, C, V kept.
 */
uint8_t WriteTileDescending(uint8_t tile)
{
    uint8_t written;

    jsr_push(0x6511);
    written = WriteTile(tile);
    jsr_pop();
    return (uint8_t)(written - 1);
}

void abi_WriteTileDescending(void)
{
    cpu.a = WriteTileDescending(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:51E SetVramFromScript
 * @run 6A:651E
 * WHAT IT DOES: Takes a VRAM address from the script and readies writes there.
 * HOW: Writes it to the write address register, keeps it in zTemp00-01 as the
 * line's start, and selects the data port, which this routine runs on into with no
 * return of its own.
 * WAITS: no.
 * Entry: zTemp02-03 pointer. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 */
void SetVramFromScript(void)
{
    uint8_t byte;

    jsr_push(0x6521);
    SelectVramWriteAddress();
    jsr_pop();
    byte = RD8_PTR(ZP->temp02);
    WR8(VDC->data_lo, byte);
    WR8(ZP->temp00, byte);
    jsr_push(0x652B);
    AdvanceTileScript();
    jsr_pop();
    byte = RD8_PTR(ZP->temp02);
    WR8(VDC->data_hi, byte);
    WR8(ZP->temp01, byte);
    jsr_push(0x6535);
    AdvanceTileScript();
    jsr_pop();
    SelectVramData();
}

void abi_SetVramFromScript(void)
{
    SetVramFromScript();
    cpu.a = 2;
    abi_nz(2);
}

/* A line step's exit: the start it set, and the carry and overflow of its high byte add. */
typedef struct { uint16_t start; Sum high; } LineStep;

/*
 * @routine 0047:538 NextVramLine
 * @run 6A:6538
 * WHAT IT DOES: Moves drawing down to the start of the next background line, $40
 * tiles on from the line's start.
 * HOW: Adds $40 to the start kept in zTemp00-01, writes it to the write address
 * register and selects the data port, which this routine runs on into with no return
 * of its own.
 * WAITS: no.
 * Entry: zTemp00-01 line start. Exit: A 2, N 0, Z 0, C and V from the high byte's add.
 * X and Y kept.
 */
static LineStep next_vram_line(void)
{
    LineStep step;
    Sum low;
    uint8_t byte;

    jsr_push(0x653B);
    SelectVramWriteAddress();
    jsr_pop();
    byte = RD8(ZP->temp00);
    low = add_bytes(byte, TILES_PER_LINE, false);
    WR8(ZP->temp00, low.value);
    WR8(VDC->data_lo, low.value);
    byte = RD8(ZP->temp01);
    step.high = add_bytes(0, byte, low.carry);
    WR8(ZP->temp01, step.high.value);
    WR8(VDC->data_hi, step.high.value);
    SelectVramData();
    step.start = (uint16_t)(low.value | (step.high.value << 8));
    return step;
}

uint16_t NextVramLine(void)
{
    return next_vram_line().start;
}

void abi_NextVramLine(void)
{
    LineStep step = next_vram_line();

    cpu.a = VDC_DATA_PORT_SELECTED;
    abi_nz(VDC_DATA_PORT_SELECTED);
    abi_flag(P_C, step.high.carry);
    abi_flag(P_V, step.high.overflow);
}

/* The attribute byte's two halves, each stored, then read back and joined, as the script ops do it. */
static void set_attribute_high(uint8_t op)
{
    uint8_t shifted = (uint8_t)(op << 4);
    uint8_t byte;

    byte = RD8(ZP->temp04);
    WR8(ZP->temp04, (uint8_t)(byte & 0x0F));
    byte = RD8(ZP->temp04);
    WR8(ZP->temp04, (uint8_t)(shifted | byte));
}

static void set_attribute_low(uint8_t op)
{
    uint8_t nibble = (uint8_t)(op & 0x0F);
    uint8_t byte;

    byte = RD8(ZP->temp04);
    WR8(ZP->temp04, (uint8_t)(byte & 0xF0));
    byte = RD8(ZP->temp04);
    WR8(ZP->temp04, (uint8_t)(nibble | byte));
}

/*
 * @routine 0047:405 TileOpAttributeHigh
 * @run 6A:6405
 * WHAT IT DOES: Script ops $C0-$CF: sets the high nibble of the tile attribute, the
 * palette.
 * HOW: The op's low nibble shifted up, ORed with the attribute's low nibble. Then on
 * into RunTileScript.
 * WAITS: no.
 * Entry: A op. Exit: back to the loop, with Y the shifted nibble.
 */
void abi_TileOpAttributeHigh(void)
{
    set_attribute_high(cpu.a);
    abi_exit_tile_run(run_tile_script(), (uint8_t)(cpu.a << 4));
}

/*
 * @routine 0047:417 TileOpAttributeLow
 * @run 6A:6417
 * WHAT IT DOES: Script ops $D0-$FD not otherwise taken: sets the low nibble of the
 * tile attribute, the high bits of the tile number.
 * HOW: The op's low nibble ORed with the attribute's high nibble. Then on into
 * RunTileScript.
 * WAITS: no.
 * Entry: A op. Exit: back to the loop, with Y the nibble.
 */
void abi_TileOpAttributeLow(void)
{
    set_attribute_low(cpu.a);
    abi_exit_tile_run(run_tile_script(), cpu.a & 0x0F);
}

/*
 * @routine 0047:420 TileScriptReturn
 * @run 6A:6420
 * WHAT IT DOES: Ends a tile script: op $FF.
 * HOW: Returns.
 * WAITS: no.
 * Entry: nothing. Exit: everything kept.
 */
void abi_TileScriptReturn(void)
{
}

/* The count of a run of tiles and the op's calls' return addresses differ per entry; each run is its own. */
static void run_repeated(uint8_t count)
{
    uint8_t tile = RD8_PTR(ZP->temp02);

    jsr_push(0x64EF);
    AdvanceTileScript();
    jsr_pop();
    do {
        jsr_push(0x64F2);
        WriteTile(tile);
        jsr_pop();
        count--;
    } while (count != 0);
}

static void run_ascending(uint8_t count)
{
    uint8_t tile = RD8_PTR(ZP->temp02);

    jsr_push(0x6500);
    AdvanceTileScript();
    jsr_pop();
    do {
        jsr_push(0x6503);
        tile = WriteTileAscending(tile);
        jsr_pop();
        count--;
    } while (count != 0);
}

static void run_literal(uint8_t count)
{
    do {
        uint8_t tile = RD8_PTR(ZP->temp02);

        jsr_push(0x64B5);
        AdvanceTileScript();
        jsr_pop();
        jsr_push(0x64B8);
        WriteTile(tile);
        jsr_pop();
        count--;
    } while (count != 0);
}

/* Alternating up then down, for two-tile patterns. A count of zero runs 256. */
static void run_alternating_up(uint8_t count)
{
    uint8_t tile = RD8_PTR(ZP->temp02);

    jsr_push(0x6429);
    AdvanceTileScript();
    jsr_pop();
    for (;;) {
        jsr_push(0x642C);
        tile = WriteTileAscending(tile);
        jsr_pop();
        count--;
        if (count == 0) {
            return;
        }
        jsr_push(0x6432);
        tile = WriteTileDescending(tile);
        jsr_pop();
        count--;
        if (count == 0) {
            return;
        }
    }
}

static void run_alternating_down(uint8_t count)
{
    uint8_t tile = RD8_PTR(ZP->temp02);

    jsr_push(0x643F);
    AdvanceTileScript();
    jsr_pop();
    for (;;) {
        jsr_push(0x6442);
        tile = WriteTileDescending(tile);
        jsr_pop();
        count--;
        if (count == 0) {
            return;
        }
        jsr_push(0x6448);
        tile = WriteTileAscending(tile);
        jsr_pop();
        count--;
        if (count == 0) {
            return;
        }
    }
}

/*
 * @routine 0047:421 TileOpAlternateUp
 * @run 6A:6421
 * WHAT IT DOES: Script ops $40-$5F: a run of tiles alternating up and back, for a
 * two-tile pattern.
 * HOW: Next byte the first tile; WriteTileAscending, WriteTileDescending in turn. Then on
 * into RunTileScript.
 * WAITS: no.
 * Entry: A op. Exit: back to the loop, with Y 0.
 */
void abi_TileOpAlternateUp(void)
{
    run_alternating_up(cpu.a & 0x1F);
    abi_exit_tile_run(run_tile_script(), 0);
}

/*
 * @routine 0047:437 TileOpAlternateDown
 * @run 6A:6437
 * WHAT IT DOES: Script ops $60-$7F: a run of tiles alternating down and back.
 * HOW: Next byte the first tile; WriteTileDescending, WriteTileAscending in turn. Then
 * on into RunTileScript.
 * WAITS: no.
 * Entry: A op. Exit: back to the loop, with Y 0.
 */
void abi_TileOpAlternateDown(void)
{
    run_alternating_down(cpu.a & 0x1F);
    abi_exit_tile_run(run_tile_script(), 0);
}

/*
 * @routine 0047:476 RunTileScript
 * @run 6A:6476
 * WHAT IT DOES: Runs a tile script until op $FF. Ops: $00 a literal tile word; $01-$1F
 * a tile repeated; $20-$3F tiles ascending; $40-$7F alternating; $80-$BF literal
 * tiles; $C0-$CF the palette; $E0 jump; $F0 next line; $FE a new VRAM address; the
 * rest of $D0-$FD the attribute's low nibble.
 * HOW: Reads an op, compares it down through the ranges in the order above from
 * the top, and does it.
 * WAITS: no.
 * Entry: zTemp02-03 the script, VDC on its data port, zTemp04 the attribute. Exit:
 * A $FF, N 0, Z 1, C 1, Y as the last op left it, V from the last line step. X
 * kept. Clobbers zTemp00-01, zTemp04.
 */
static TileRun run_tile_script(void)
{
    TileRun run = { TILE_OP_END, 0, false, false };

    for (;;) {
        uint8_t op = RD8_PTR(ZP->temp02);

        jsr_push(0x647B);
        AdvanceTileScript();
        jsr_pop();
        if (op == TILE_OP_END) {
            return run;
        }
        if (op == TILE_OP_SET_ADDRESS) {
            jsr_push(0x64E0);
            SetVramFromScript();
            jsr_pop();
            continue;
        }
        if (op == TILE_OP_NEXT_LINE) {
            LineStep step;

            jsr_push(0x64E5);
            step = next_vram_line();
            jsr_pop();
            run.stepped_line = true;
            run.line_overflow = step.high.overflow;
            continue;
        }
        if (op == TILE_OP_LITERAL_WORD) {
            uint8_t byte = RD8_PTR(ZP->temp02);

            WR8(VDC->data_lo, byte);
            jsr_push(0x64D3);
            AdvanceTileScript();
            jsr_pop();
            byte = RD8_PTR(ZP->temp02);
            WR8(VDC->data_hi, byte);
            jsr_push(0x64DB);
            AdvanceTileScript();
            jsr_pop();
            continue;
        }
        run.last_op = op;
        if (op == TILE_OP_JUMP) {
            uint8_t low = RD8_PTR(ZP->temp02);
            uint8_t high;

            run.jump_low = low;
            jsr_push(0x64C3);
            AdvanceTileScript();
            jsr_pop();
            high = RD8_PTR(ZP->temp02);
            WR8(ZP->temp02, low);
            WR8(ZP->temp03, high);
        } else if (op >= TILE_OP_ATTRIBUTE_LOW) {
            set_attribute_low(op);
        } else if (op >= TILE_OP_PALETTE) {
            set_attribute_high(op);
        } else if (op < TILE_OP_REPEATED) {
            run_repeated(op & 0x1F);
        } else if (op < TILE_OP_ASCENDING) {
            run_ascending(op & 0x1F);
        } else if (op < TILE_OP_ALTERNATE_UP) {
            run_alternating_up(op & 0x1F);
        } else if (op < TILE_OP_ALTERNATE_DOWN) {
            run_alternating_down(op & 0x1F);
        } else {
            run_literal(op & 0x3F);
        }
    }
}

void abi_RunTileScript(void)
{
    uint8_t y_before = cpu.y;

    abi_exit_tile_run(run_tile_script(), y_before);
}

/*
 * @routine 0047:44D DrawTileScript
 * @run 6A:644D
 * WHAT IT DOES: Draws one of the stage's tile scripts: a group and an item within it.
 * HOW: The overlay's pointer at OverlayTileScripts leads to a table of groups, each
 * a table of scripts. Clears the VDC's high control byte, takes the address from
 * the script and runs it. The item is kept on the stack meanwhile.
 * WAITS: no.
 * Entry: A item, Y group, stage overlay mapped, zTemp04 the attribute. Exit: as
 * RunTileScript. Clobbers zTemp00-03, zTemp08-09.
 * The overlay's pointer is read by its old name: it has no record.
 */
static TileRun draw_tile_script(uint8_t item, uint8_t group)
{
    uint8_t byte;
    uint8_t y;

    push8(item);
    byte = rd8(OverlayTileScripts);
    WR8(ZP->temp08, byte);
    byte = rd8((uint16_t)(OverlayTileScripts + 1));
    WR8(ZP->temp09, byte);
    y = (uint8_t)(group << 1);
    byte = RD8_PTR_Y(ZP->temp08, y);
    WR8(ZP->temp00, byte);
    y = (uint8_t)(y + 1);
    byte = RD8_PTR_Y(ZP->temp08, y);
    WR8(ZP->temp01, byte);
    item = pull8();
    y = (uint8_t)(item << 1);
    byte = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp02, byte);
    y = (uint8_t)(y + 1);
    byte = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp03, byte);
    jsr_push(0x6473);
    ClearVdcControlHigh();
    jsr_pop();
    jsr_push(0x6476);
    SetVramFromScript();
    jsr_pop();
    return run_tile_script();
}

void abi_DrawTileScript(void)
{
    uint8_t item = cpu.a;
    uint8_t group = cpu.y;

    abi_exit_tile_run(draw_tile_script(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
}
