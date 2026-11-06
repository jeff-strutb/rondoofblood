/*
 * Tile scripts: byte code that draws text and background tiles straight into
 * VRAM. The stage overlay holds them. A tile is written as a low byte and the
 * current attribute byte; the script steps through RAM by a pointer in zTemp02.
 */
#include "machine.h"
#include "ram.h"
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
    uint16_t line_start;  /* where the last line step went */
} TileRun;

static TileRun run_tile_script(void);

/* A line step adds to the high byte with its carry: V is overflow from $7F to $80. */
static bool line_step_overflowed_shim(uint16_t line_start)
{
    uint16_t before = (uint16_t)(line_start - TILES_PER_LINE);

    return (before >> 8) == 0x7F && (line_start >> 8) == 0x80;
}

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
        abi_flag(P_V, line_step_overflowed_shim(run.line_start));
}

/*
 * @routine 0047:550 AdvanceTileScript
 * @run 6A:6550
 * WHAT IT DOES: Moves the script pointer on one byte.
 * HOW: A 16-bit increment in zero page.
 * Entry: zTemp02-03 pointer. Exit: N and Z from the last byte stepped. A, X, Y, C,
 * V kept.
 */
uint8_t AdvanceTileScript(void)
{
    uint8_t low = (uint8_t)(rd8(zTemp02) + 1);

    wr8(zTemp02, low);
    if (low == 0) {
        low = (uint8_t)(rd8(zTemp03) + 1);
        wr8(zTemp03, low);
    }
    return low;
}

void abi_AdvanceTileScript(void)
{
    abi_nz(AdvanceTileScript());
}

/* The next byte of the script, stepping past it. */
static uint8_t next_script_byte(void)
{
    uint8_t byte = rd8_zp(zTemp02);

    AdvanceTileScript();
    return byte;
}

/*
 * @routine 0047:513 WriteTile
 * @run 6A:6513
 * WHAT IT DOES: Writes one tile to VRAM.
 * HOW: The tile as the low byte, zTemp04 as the attribute byte.
 * Entry: A tile, VDC on its data port. Exit: A kept, N and Z from it. X, Y, C, V
 * kept.
 */
void WriteTile(uint8_t tile)
{
    uint8_t attribute;

    wr8(VDC_DATA_LO, tile);
    attribute = rd8(zTemp04);
    wr8(VDC_DATA_HI, attribute);
}

void abi_WriteTile(void)
{
    WriteTile(cpu.a);
    abi_nz(cpu.a);
}

/*
 * @routine 0047:509 WriteTileAscending
 * @run 6A:6509
 * WHAT IT DOES: Writes a tile and gives the next one up.
 * HOW: WriteTile, then adds one.
 * Entry: A tile. Exit: A plus one, N and Z from it. X, Y, C, V kept.
 */
uint8_t WriteTileAscending(uint8_t tile)
{
    WriteTile(tile);
    return (uint8_t)(tile + 1);
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
 * Entry: A tile. Exit: A minus one, N and Z from it. X, Y, C, V kept.
 */
uint8_t WriteTileDescending(uint8_t tile)
{
    WriteTile(tile);
    return (uint8_t)(tile - 1);
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
 * line's start, and selects the data port.
 * Entry: zTemp02-03 pointer. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 */
void SetVramFromScript(void)
{
    uint8_t byte;

    SelectVramWriteAddress();
    byte = rd8_zp(zTemp02);
    wr8(VDC_DATA_LO, byte);
    wr8(zTemp00, byte);
    AdvanceTileScript();
    byte = rd8_zp(zTemp02);
    wr8(VDC_DATA_HI, byte);
    wr8(zTemp01, byte);
    AdvanceTileScript();
    SelectVramData();
}

void abi_SetVramFromScript(void)
{
    SetVramFromScript();
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 0047:538 NextVramLine
 * @run 6A:6538
 * WHAT IT DOES: Moves drawing down to the start of the next background line, $40
 * tiles on from the line's start.
 * HOW: Adds $40 to the start kept in zTemp00-01, writes it to the write address
 * register and selects the data port.
 * Entry: zTemp00-01 line start. Exit: A 2, N 0, Z 0, C and V from the sum. X and Y
 * kept.
 */
uint16_t NextVramLine(void)
{
    uint8_t low = rd8(zTemp00);
    uint16_t start = (uint16_t)(low | (rd8(zTemp01) << 8));

    start = (uint16_t)(start + TILES_PER_LINE);
    SelectVramWriteAddress();
    wr8(zTemp00, (uint8_t)start);
    wr8(VDC_DATA_LO, (uint8_t)start);
    wr8(zTemp01, (uint8_t)(start >> 8));
    wr8(VDC_DATA_HI, (uint8_t)(start >> 8));
    SelectVramData();
    return start;
}

/* The carry out is of the high byte: only $FFxx wrapping to $00xx. */
static Alu next_line_exit_shim(uint16_t start)
{
    Alu exit = { 0 };
    uint16_t before = (uint16_t)(start - TILES_PER_LINE);

    exit.a = VDC_DATA_PORT_SELECTED;
    exit.nz = VDC_DATA_PORT_SELECTED;
    exit.c = (before >> 8) == 0xFF && (start >> 8) == 0x00;
    exit.v = line_step_overflowed_shim(start);
    exit.v_set = true;
    return exit;
}

void abi_NextVramLine(void)
{
    abi_alu(next_line_exit_shim(NextVramLine()));
}

static void set_attribute_high(uint8_t op)
{
    wr8(zTemp04, (uint8_t)((op << 4) | (rd8(zTemp04) & 0x0F)));
}

/*
 * @routine 0047:405 TileOpAttributeHigh
 * @run 6A:6405
 * WHAT IT DOES: Script ops $C0-$CF: sets the high nibble of the tile attribute, the
 * palette.
 * HOW: The op's low nibble shifted up, ORed with the attribute's low nibble.
 * Entry: A op. Exit: back to the loop, with Y the shifted nibble.
 */
void abi_TileOpAttributeHigh(void)
{
    set_attribute_high(cpu.a);
    abi_exit_tile_run(run_tile_script(), (uint8_t)(cpu.a << 4));
}

static void set_attribute_low(uint8_t op)
{
    wr8(zTemp04, (uint8_t)((op & 0x0F) | (rd8(zTemp04) & 0xF0)));
}

/*
 * @routine 0047:417 TileOpAttributeLow
 * @run 6A:6417
 * WHAT IT DOES: Script ops $D0-$FD not otherwise taken: sets the low nibble of the
 * tile attribute, the high bits of the tile number.
 * HOW: The op's low nibble ORed with the attribute's high nibble.
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
 * Entry: nothing. Exit: everything kept.
 */
void abi_TileScriptReturn(void)
{
}

/* Runs of tiles, value in the next script byte, counts in the op's low bits. */
static void run_repeated(uint8_t count)
{
    uint8_t tile = next_script_byte();

    do {
        WriteTile(tile);
        count--;
    } while (count != 0);
}

static void run_ascending(uint8_t count)
{
    uint8_t tile = next_script_byte();

    do {
        tile = WriteTileAscending(tile);
        count--;
    } while (count != 0);
}

static void run_literal(uint8_t count)
{
    do {
        uint8_t tile = next_script_byte();

        WriteTile(tile);
        count--;
    } while (count != 0);
}

/* Alternating up and down, for two-tile patterns. A count of zero runs 256. */
static void run_alternating(uint8_t count, bool up_first)
{
    uint8_t tile = next_script_byte();

    for (;;) {
        tile = up_first ? WriteTileAscending(tile) : WriteTileDescending(tile);
        count--;
        if (count == 0)
            return;
        tile = up_first ? WriteTileDescending(tile) : WriteTileAscending(tile);
        count--;
        if (count == 0)
            return;
    }
}

/*
 * @routine 0047:421 TileOpAlternateUp
 * @run 6A:6421
 * WHAT IT DOES: Script ops $40-$5F: a run of tiles alternating up and back, for a
 * two-tile pattern.
 * HOW: Next byte the first tile; WriteTileAscending, WriteTileDescending in turn.
 * Entry: A op. Exit: back to the loop, with Y 0.
 */
void abi_TileOpAlternateUp(void)
{
    run_alternating(cpu.a & 0x1F, true);
    abi_exit_tile_run(run_tile_script(), 0);
}

/*
 * @routine 0047:437 TileOpAlternateDown
 * @run 6A:6437
 * WHAT IT DOES: Script ops $60-$7F: a run of tiles alternating down and back.
 * HOW: Next byte the first tile; WriteTileDescending, WriteTileAscending in turn.
 * Entry: A op. Exit: back to the loop, with Y 0.
 */
void abi_TileOpAlternateDown(void)
{
    run_alternating(cpu.a & 0x1F, false);
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
 * Entry: zTemp02-03 the script, VDC on its data port, zTemp04 the attribute. Exit:
 * A $FF, N 0, Z 1, C 1, Y as the last op left it, V from the last line step. X
 * kept. Clobbers zTemp00-01, zTemp04.
 */
static TileRun run_tile_script(void)
{
    TileRun run = { TILE_OP_END, 0, false, 0 };

    for (;;) {
        uint8_t op = next_script_byte();

        if (op == TILE_OP_END)
            return run;
        if (op == TILE_OP_SET_ADDRESS) {
            SetVramFromScript();
            continue;
        }
        if (op == TILE_OP_NEXT_LINE) {
            run.line_start = NextVramLine();
            run.stepped_line = true;
            continue;
        }
        if (op == TILE_OP_LITERAL_WORD) {
            uint8_t byte = rd8_zp(zTemp02);

            wr8(VDC_DATA_LO, byte);
            AdvanceTileScript();
            byte = rd8_zp(zTemp02);
            wr8(VDC_DATA_HI, byte);
            AdvanceTileScript();
            continue;
        }
        run.last_op = op;
        if (op == TILE_OP_JUMP) {
            uint8_t low = rd8_zp(zTemp02);
            uint8_t high;

            run.jump_low = low;
            AdvanceTileScript();
            high = rd8_zp(zTemp02);
            wr8(zTemp02, low);
            wr8(zTemp03, high);
        } else if (op >= TILE_OP_ATTRIBUTE_LOW) {
            set_attribute_low(op);
        } else if (op >= TILE_OP_PALETTE) {
            set_attribute_high(op);
        } else if (op < TILE_OP_REPEATED) {
            run_repeated(op & 0x1F);
        } else if (op < TILE_OP_ASCENDING) {
            run_ascending(op & 0x1F);
        } else if (op < TILE_OP_ALTERNATE_UP) {
            run_alternating(op & 0x1F, true);
        } else if (op < TILE_OP_ALTERNATE_DOWN) {
            run_alternating(op & 0x1F, false);
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

static uint16_t tile_pointer_at(uint16_t address)
{
    uint8_t low = rd8(address);

    return (uint16_t)(low | (rd8((uint16_t)(address + 1)) << 8));
}

static void tile_pointer_to(uint16_t lo, uint16_t hi, uint16_t pointer)
{
    wr8(lo, (uint8_t)pointer);
    wr8(hi, (uint8_t)(pointer >> 8));
}

/*
 * @routine 0047:44D DrawTileScript
 * @run 6A:644D
 * WHAT IT DOES: Draws one of the stage's tile scripts: a group and an item within it.
 * HOW: The overlay's pointer at OverlayTileScripts leads to a table of groups, each
 * a table of scripts. Clears the VDC's high control byte, takes the address from
 * the script and runs it.
 * Entry: A item, Y group, stage overlay mapped, zTemp04 the attribute. Exit: as
 * RunTileScript. Clobbers zTemp00-03, zTemp08-09.
 */
static TileRun draw_tile_script(uint8_t item, uint8_t group)
{
    uint16_t groups = tile_pointer_at(OverlayTileScripts);
    uint16_t items = tile_pointer_at((uint16_t)(groups + (uint8_t)(group << 1)));
    uint16_t script = tile_pointer_at((uint16_t)(items + (uint8_t)(item << 1)));

    tile_pointer_to(zTemp08, zTemp09, groups);
    tile_pointer_to(zTemp00, zTemp01, items);
    tile_pointer_to(zTemp02, zTemp03, script);
    ClearVdcControlHigh();
    SetVramFromScript();
    return run_tile_script();
}

void abi_DrawTileScript(void)
{
    uint8_t item = cpu.a;
    uint8_t group = cpu.y;

    abi_exit_tile_run(draw_tile_script(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
}
