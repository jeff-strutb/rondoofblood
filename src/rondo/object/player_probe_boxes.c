/*
 * The probe of a stage overlay's four contact boxes, lot L2 (and the two copies of lot L3 that these call): each overlay holds
 * the same two routines at $98BD and $991D and differs only in the address of its table of box offsets, so one body
 * serves them and each overlay has its own routine naming its table.
 *
 * ProbeBoxes reads, for the object, four boxes' offsets from the table entry given and probes the map at each point with
 * ProbeTileRead9991, leaving the answer, the depth in X and the depth in Y of each in TileHit, TileDepthX and TileDepthY.
 * ProbeBoxesFlagged runs ProbeBoxes, then for each box whose answer has no bit 7 and some of the bits $A1 probes a second
 * point $08 above and clears the answer's bits but $5E when that second look finds bit 0.
 *
 * Checked with tools/rondo/curated/probe_boxes.txt: 60 cases that pin the handler table at $9A2C to handlers this code has (the
 * random cases send the read to the overlay's own handlers, which have no C and are untraced: the harness's stub for them returns
 * an arbitrary X where the C, by the handlers' contract, keeps it, so those cases differ and are not a verdict). The flagged
 * routine is compared against cdraft.py --whole (its loop tail is a second piece); 0F60:11D has no whole routine in the intake
 * and is checked as far as the first piece.
 *
 * Their pointer is zTemp0C-0D, their index zTemp0A (zTemp0A-0D have no names in the zero page record yet: temp0a to temp0d).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum { BOX_COUNT = 4, BOX_END = 0x80, BOX_SECOND_LOOK_BITS = 0xA1, BOX_KEEP_BITS = 0x5E, BOX_RAISE = 0xF8 };

/* The status byte a PHP pushes after an INC or INY of Y: B, the carry and overflow of the add before it, and N and Z from Y. */
static uint8_t status_after_y_step(bool carry, bool overflow, uint8_t y)
{
    uint8_t status = P_B;

    if (carry) {
        status |= P_C;
    }
    if (overflow) {
        status |= P_V;
    }
    if (y & 0x80) {
        status |= P_N;
    }
    if (y == 0) {
        status |= P_Z;
    }
    return status;
}

/* How a probe routine ended: the state its last instructions left, and the V and B bits that a PLP or an add left in P. */
typedef struct {
    Ended end;
    bool v;          /* V as the last add (or the PLP) left it */
    bool probed;     /* the last probe's own exit then follows the add's V */
    TileRead read;
} BoxesEnd;

typedef BoxesEnd (*BoxesFunction)(uint8_t entry, uint8_t slot);

/* The wrapper's one place that meets the registers and flags. */
static void abi_boxes_end(BoxesEnd e)
{
    if (!e.probed || e.read.kind != TILE_BY_ORIGINAL) {
        abi_flag(P_V, e.v);
        abi_flag(P_B, true);
    }
    if (e.probed) {
        terrain_read_exit_shim(e.read);
    }
    abi_ended(e.end);
}

/* The first routine's body: the entry's pointer from the table, then the four boxes. */
static BoxesEnd probe_boxes(uint8_t entry, uint8_t slot, uint16_t table)
{
    uint8_t index = (uint8_t)(entry << 1);
    uint8_t y, value, marker, status, answer, count;
    Sum low_x, high_x, low_y, high_y;
    TileRead read = { TILE_PLAIN, 0, 0, 0, 0, 0, { false, 0, false, false, false } };
    bool carry;
    bool probed = false;
    bool v = false;

    value = rd8((uint16_t)(table + index));
    WR8(ZP->temp0c, value);
    value = rd8((uint16_t)(table + 1 + index));
    WR8(ZP->temp0d, value);
    y = 0;
    WR8(ZP->temp0a, y);
    for (;;) {
        value = RD8_PTR_Y(ZP->temp0c, y);
        low_x = add_bytes(value, RD8(OBJ->x_lo[slot]), false);
        WR8(ZP->temp00, low_x.value);
        y++;
        push8(status_after_y_step(low_x.carry, low_x.overflow, y));
        marker = RD8_PTR_Y(ZP->temp0c, y);
        if (marker == BOX_END) {
            status = pull8();
            carry = (status & P_C) != 0;
            y = (uint8_t)(y + 3);
            push8(y);
            WR8(ZP->temp00, 0);
            WR8(ZP->temp02, 0);
            answer = 0;
            probed = false;
            v = (status & P_V) != 0;
        } else {
            status = pull8();
            carry = (status & P_C) != 0;
            high_x = add_bytes(marker, RD8(OBJ->x_hi[slot]), carry);
            WR8(ZP->temp01, high_x.value);
            y++;
            value = RD8_PTR_Y(ZP->temp0c, y);
            low_y = add_bytes(value, RD8(OBJ->y_lo[slot]), false);
            WR8(ZP->temp02, low_y.value);
            y++;
            value = RD8_PTR_Y(ZP->temp0c, y);
            high_y = add_bytes(value, RD8(OBJ->y_hi[slot]), low_y.carry);
            WR8(ZP->temp03, high_y.value);
            y++;
            push8(y);
            probed = true;
            v = high_y.overflow;
            jsr_push(0x98F8);
            abi_flag(P_B, true); /* the PLP above set B; the read may run the overlay's own code with it */
            read = ProbeTileRead9991();
            jsr_pop();
            answer = tile_answer(read);
        }
        y = RD8(ZP->temp0a);
        WR8(LEVEL->tile_hit[y], answer);
        value = RD8(ZP->temp00);
        WR8(LEVEL->tile_depth_x[y], value);
        value = RD8(ZP->temp02);
        WR8(LEVEL->tile_depth_y[y], value);
        INC8(ZP->temp0a);
        y = pull8();
        count = RD8(ZP->temp0a);
        if (count == BOX_COUNT) {
            BoxesEnd done = { ended_with_y(ended_compare(count, BOX_COUNT), y), v, probed, read };

            return done;
        }
    }
}

/* The second routine's body, with the first as the overlay has it. */
static BoxesEnd probe_boxes_flagged(uint8_t entry, uint8_t slot, uint16_t table, BoxesFunction boxes)
{
    uint8_t index = (uint8_t)(entry << 1);
    uint8_t y, value, hit, answer, count, taken;
    Sum low_x, high_x, low_y, high_y, raised, again;
    TileRead read;

    BoxesEnd called;
    bool v;
    bool probed;

    push8(entry);
    jsr_push(0x9921);
    called = boxes(entry, slot);
    jsr_pop();
    v = called.v;
    probed = called.probed;
    read = called.read;
    entry = pull8();
    index = (uint8_t)(entry << 1);
    value = rd8((uint16_t)(table + index));
    WR8(ZP->temp0c, value);
    value = rd8((uint16_t)(table + 1 + index));
    WR8(ZP->temp0d, value);
    y = 0;
    WR8(ZP->temp0a, y);
    for (;;) {
        push8(y);
        y = RD8(ZP->temp0a);
        hit = RD8(LEVEL->tile_hit[y]);
        y = pull8();
        taken = (uint8_t)(hit & BOX_SECOND_LOOK_BITS);
        if ((taken & 0x80) != 0 || taken == 0) {
            y = (uint8_t)(y + 4);
        } else {
            value = RD8_PTR_Y(ZP->temp0c, y);
            low_x = add_bytes(value, RD8(OBJ->x_lo[slot]), false);
            WR8(ZP->temp00, low_x.value);
            v = low_x.overflow;
            probed = false;
            y++;
            value = RD8_PTR_Y(ZP->temp0c, y);
            high_x = add_bytes(value, RD8(OBJ->x_hi[slot]), low_x.carry);
            WR8(ZP->temp01, high_x.value);
            v = high_x.overflow;
            y++;
            value = RD8_PTR_Y(ZP->temp0c, y);
            low_y = add_bytes(value, RD8(OBJ->y_lo[slot]), false);
            WR8(ZP->temp02, low_y.value);
            v = low_y.overflow;
            y++;
            value = RD8_PTR_Y(ZP->temp0c, y);
            high_y = add_bytes(value, RD8(OBJ->y_hi[slot]), low_y.carry);
            WR8(ZP->temp03, high_y.value);
            v = high_y.overflow;
            y++;
            value = RD8(ZP->temp02);
            raised = add_bytes(BOX_RAISE, value, false);
            WR8(ZP->temp02, raised.value);
            v = raised.overflow;
            value = RD8(ZP->temp02);
            again = add_bytes(0xFF, value, raised.carry);
            WR8(ZP->temp02, again.value);
            v = again.overflow;
            push8(y);
            jsr_push(0x9971);
            abi_flag(P_B, true); /* the PLP of the first routine set B */
            read = ProbeTileRead9991();
            jsr_pop();
            probed = true;
            answer = tile_answer(read);
            y = RD8(ZP->temp0a);
            if ((answer & 1) != 0) {
                y = RD8(ZP->temp0a);
                hit = RD8(LEVEL->tile_hit[y]);
                WR8(LEVEL->tile_hit[y], (uint8_t)(hit & BOX_KEEP_BITS));
            }
            y = pull8();
        }
        INC8(ZP->temp0a);
        count = RD8(ZP->temp0a);
        if (count == BOX_COUNT) {
            BoxesEnd done = { ended_with_y(ended_compare(count, BOX_COUNT), y), v, probed, read };

            return done;
        }
    }
}

/*
 * @routine 05C0:0BD ProbeBoxes05C0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $BCEC names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes05C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xBCEC);
}

void abi_ProbeBoxes05C0(void)
{
    abi_boxes_end(ProbeBoxes05C0(cpu.a, cpu.x));
}

/*
 * @routine 0720:0BD ProbeBoxes0720
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $C0D4 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0720(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xC0D4);
}

void abi_ProbeBoxes0720(void)
{
    abi_boxes_end(ProbeBoxes0720(cpu.a, cpu.x));
}

/*
 * @routine 0880:0BD ProbeBoxes0880
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $C251 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0880(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xC251);
}

void abi_ProbeBoxes0880(void)
{
    abi_boxes_end(ProbeBoxes0880(cpu.a, cpu.x));
}

/*
 * @routine 09E0:0BD ProbeBoxes09E0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $C186 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes09E0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xC186);
}

void abi_ProbeBoxes09E0(void)
{
    abi_boxes_end(ProbeBoxes09E0(cpu.a, cpu.x));
}

/*
 * @routine 0B40:0BD ProbeBoxes0B40
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $B38E names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0B40(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xB38E);
}

void abi_ProbeBoxes0B40(void)
{
    abi_boxes_end(ProbeBoxes0B40(cpu.a, cpu.x));
}

/*
 * @routine 0CA0:0BD ProbeBoxes0CA0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $C1A5 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0CA0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xC1A5);
}

void abi_ProbeBoxes0CA0(void)
{
    abi_boxes_end(ProbeBoxes0CA0(cpu.a, cpu.x));
}

/*
 * @routine 0E00:0BD ProbeBoxes0E00
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $AF23 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0E00(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xAF23);
}

void abi_ProbeBoxes0E00(void)
{
    abi_boxes_end(ProbeBoxes0E00(cpu.a, cpu.x));
}

/*
 * @routine 0F60:0BD ProbeBoxes0F60
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $B01F names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes0F60(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xB01F);
}

void abi_ProbeBoxes0F60(void)
{
    abi_boxes_end(ProbeBoxes0F60(cpu.a, cpu.x));
}

/*
 * @routine 10C0:0BD ProbeBoxes10C0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $C511 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes10C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xC511);
}

void abi_ProbeBoxes10C0(void)
{
    abi_boxes_end(ProbeBoxes10C0(cpu.a, cpu.x));
}

/*
 * @routine 12D0:0BD ProbeBoxes12D0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $D09B names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes12D0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xD09B);
}

void abi_ProbeBoxes12D0(void)
{
    abi_boxes_end(ProbeBoxes12D0(cpu.a, cpu.x));
}

/*
 * @routine 16C0:0BD ProbeBoxes16C0
 * @run 6B:98BD
 * WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
 * TileDepthX and TileDepthY.
 * HOW: The table at $BDA9 names an entry's box offsets; each is added to the object's position and read with
 * ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
 * place in the entry, X kept. Clobbers zTemp00-0D.
 */
BoxesEnd ProbeBoxes16C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes(entry, slot, 0xBDA9);
}

void abi_ProbeBoxes16C0(void)
{
    abi_boxes_end(ProbeBoxes16C0(cpu.a, cpu.x));
}

/*
 * @routine 05C0:11D ProbeBoxesFlagged05C0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes05C0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes05C0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged05C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xBCEC, ProbeBoxes05C0);
}

void abi_ProbeBoxesFlagged05C0(void)
{
    abi_boxes_end(ProbeBoxesFlagged05C0(cpu.a, cpu.x));
}

/*
 * @routine 0720:11D ProbeBoxesFlagged0720
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0720, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0720, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0720(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xC0D4, ProbeBoxes0720);
}

void abi_ProbeBoxesFlagged0720(void)
{
    abi_boxes_end(ProbeBoxesFlagged0720(cpu.a, cpu.x));
}

/*
 * @routine 0880:11D ProbeBoxesFlagged0880
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0880, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0880, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0880(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xC251, ProbeBoxes0880);
}

void abi_ProbeBoxesFlagged0880(void)
{
    abi_boxes_end(ProbeBoxesFlagged0880(cpu.a, cpu.x));
}

/*
 * @routine 09E0:11D ProbeBoxesFlagged09E0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes09E0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes09E0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged09E0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xC186, ProbeBoxes09E0);
}

void abi_ProbeBoxesFlagged09E0(void)
{
    abi_boxes_end(ProbeBoxesFlagged09E0(cpu.a, cpu.x));
}

/*
 * @routine 0B40:11D ProbeBoxesFlagged0B40
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0B40, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0B40, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0B40(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xB38E, ProbeBoxes0B40);
}

void abi_ProbeBoxesFlagged0B40(void)
{
    abi_boxes_end(ProbeBoxesFlagged0B40(cpu.a, cpu.x));
}

/*
 * @routine 0CA0:11D ProbeBoxesFlagged0CA0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0CA0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0CA0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0CA0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xC1A5, ProbeBoxes0CA0);
}

void abi_ProbeBoxesFlagged0CA0(void)
{
    abi_boxes_end(ProbeBoxesFlagged0CA0(cpu.a, cpu.x));
}

/*
 * @routine 0E00:11D ProbeBoxesFlagged0E00
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0E00, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0E00, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0E00(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xAF23, ProbeBoxes0E00);
}

void abi_ProbeBoxesFlagged0E00(void)
{
    abi_boxes_end(ProbeBoxesFlagged0E00(cpu.a, cpu.x));
}

/*
 * @routine 0F60:11D ProbeBoxesFlagged0F60
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0F60, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes0F60, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged0F60(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xB01F, ProbeBoxes0F60);
}

void abi_ProbeBoxesFlagged0F60(void)
{
    abi_boxes_end(ProbeBoxesFlagged0F60(cpu.a, cpu.x));
}

/*
 * @routine 10C0:11D ProbeBoxesFlagged10C0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes10C0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes10C0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged10C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xC511, ProbeBoxes10C0);
}

void abi_ProbeBoxesFlagged10C0(void)
{
    abi_boxes_end(ProbeBoxesFlagged10C0(cpu.a, cpu.x));
}

/*
 * @routine 12D0:11D ProbeBoxesFlagged12D0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes12D0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes12D0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged12D0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xD09B, ProbeBoxes12D0);
}

void abi_ProbeBoxesFlagged12D0(void)
{
    abi_boxes_end(ProbeBoxesFlagged12D0(cpu.a, cpu.x));
}

/*
 * @routine 16C0:11D ProbeBoxesFlagged16C0
 * @run 6B:991D
 * WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes16C0, then for each box whose answer has bits of $A1
 * and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
 * second look finds bit 0.
 * HOW: ProbeBoxes16C0, then the same table entry walked again.
 * WAITS: no.
 * Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
 */
BoxesEnd ProbeBoxesFlagged16C0(uint8_t entry, uint8_t slot)
{
    return probe_boxes_flagged(entry, slot, 0xBDA9, ProbeBoxes16C0);
}

void abi_ProbeBoxesFlagged16C0(void)
{
    abi_boxes_end(ProbeBoxesFlagged16C0(cpu.a, cpu.x));
}
