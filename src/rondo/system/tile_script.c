/*
 * Tile scripts: byte code that draws text and background tiles straight into
 * VRAM. The stage overlay holds them. A tile is written as a low byte and the
 * current attribute byte; the script steps through RAM by a pointer in zTemp02.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* What the interpreter's loop keeps in Y and V between ops. */
typedef struct {
    uint8_t y;
    bool v;
    bool v_set;
} TileRun;

static TileRun run_tile_script(uint8_t y);

/* Every way out of the interpreter. */
static void abi_exit_tile_run(TileRun run)
{
    cpu.a = 0xFF;
    cpu.y = run.y;
    abi_nz(0);
    abi_flag(P_C, true);
    if (run.v_set)
        abi_flag(P_V, run.v);
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
Alu NextVramLine(void)
{
    uint8_t byte;
    Alu r;

    SelectVramWriteAddress();
    byte = rd8(zTemp00);
    r = adc8(byte, 0x40, false);
    wr8(zTemp00, r.a);
    wr8(VDC_DATA_LO, r.a);
    byte = rd8(zTemp01);
    r = adc8(0, byte, r.c);
    wr8(zTemp01, r.a);
    wr8(VDC_DATA_HI, r.a);
    SelectVramData();
    r.a = 2;
    r.nz = 2;
    return r;
}

void abi_NextVramLine(void)
{
    abi_alu(NextVramLine());
}

static void set_attribute_high(uint8_t op)
{
    uint8_t high = (uint8_t)(op << 4);
    uint8_t byte = rd8(zTemp04) & 0x0F;

    wr8(zTemp04, byte);
    byte = rd8(zTemp04);
    wr8(zTemp04, (uint8_t)(high | byte));
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
    abi_exit_tile_run(run_tile_script((uint8_t)(cpu.a << 4)));
}

static void set_attribute_low(uint8_t op)
{
    uint8_t low = op & 0x0F;
    uint8_t byte = rd8(zTemp04) & 0xF0;

    wr8(zTemp04, byte);
    byte = rd8(zTemp04);
    wr8(zTemp04, (uint8_t)(low | byte));
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
    abi_exit_tile_run(run_tile_script(cpu.a & 0x0F));
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
    abi_exit_tile_run(run_tile_script(0));
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
    abi_exit_tile_run(run_tile_script(0));
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
static TileRun run_tile_script(uint8_t entry_y)
{
    TileRun run = { entry_y, false, false };

    for (;;) {
        uint8_t op = next_script_byte();

        if (op == 0xFF)
            return run;
        if (op == 0xFE) {
            SetVramFromScript();
        } else if (op == 0xF0) {
            Alu line = NextVramLine();

            run.v = line.v;
            run.v_set = line.v_set;
        } else if (op == 0xE0) {
            uint8_t low = rd8_zp(zTemp02);
            uint8_t high;

            run.y = low;
            AdvanceTileScript();
            high = rd8_zp(zTemp02);
            wr8(zTemp02, low);
            wr8(zTemp03, high);
        } else if (op >= 0xD0) {
            set_attribute_low(op);
            run.y = op & 0x0F;
        } else if (op >= 0xC0) {
            set_attribute_high(op);
            run.y = (uint8_t)(op << 4);
        } else if (op == 0x00) {
            uint8_t byte = rd8_zp(zTemp02);

            wr8(VDC_DATA_LO, byte);
            AdvanceTileScript();
            byte = rd8_zp(zTemp02);
            wr8(VDC_DATA_HI, byte);
            AdvanceTileScript();
        } else if (op < 0x20) {
            run_repeated(op & 0x1F);
            run.y = 0;
        } else if (op < 0x40) {
            run_ascending(op & 0x1F);
            run.y = 0;
        } else if (op < 0x60) {
            run_alternating(op & 0x1F, true);
            run.y = 0;
        } else if (op < 0x80) {
            run_alternating(op & 0x1F, false);
            run.y = 0;
        } else {
            run_literal(op & 0x3F);
            run.y = 0;
        }
    }
}

void abi_RunTileScript(void)
{
    abi_exit_tile_run(run_tile_script(cpu.y));
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
    uint8_t byte;
    uint8_t index;

    byte = rd8(OverlayTileScripts);
    wr8(zTemp08, byte);
    byte = rd8(OverlayTileScripts + 1);
    wr8(zTemp09, byte);
    index = (uint8_t)(group << 1);
    byte = rd8_zp_y(zTemp08, index);
    wr8(zTemp00, byte);
    byte = rd8_zp_y(zTemp08, (uint8_t)(index + 1));
    wr8(zTemp01, byte);
    index = (uint8_t)(item << 1);
    byte = rd8_zp_y(zTemp00, index);
    wr8(zTemp02, byte);
    byte = rd8_zp_y(zTemp00, (uint8_t)(index + 1));
    wr8(zTemp03, byte);
    ClearVdcControlHigh();
    SetVramFromScript();
    return run_tile_script((uint8_t)(index + 1));
}

void abi_DrawTileScript(void)
{
    abi_exit_tile_run(draw_tile_script(cpu.a, cpu.y));
}
