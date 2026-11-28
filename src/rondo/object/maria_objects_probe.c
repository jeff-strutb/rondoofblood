/*
 * Maria's objects (her overlay, bank $6E): the probes that go to the stage overlay's entries, $8006 (the terrain
 * read at a point, StageTileRead) and $8009.
 *
 * These routines hold the processor: each is entered with the registers and flags as its caller left them in it (a
 * C caller gives its exit so far first), sets in it what its assembly sets, and leaves in it what the stage's code
 * left. Their callers read the answer from it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum { OBJECT_PLAYER = 0, PROBE_ROW_STEP = 0x40, PROBE_ROW_LO = 0x80, PROBE_ROW_HI = 0x60, PROBE_ROW_POINTS = 6,
       STAGE_STEP_DONE = 1 };

/* LDA: A, N and Z from it. */
static void p_lda(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

/* An ADC's result into the processor: A, N and Z, C, V. */
static void p_sum(Sum s)
{
    p_lda(s.value);
    abi_flag(P_C, s.carry);
    abi_flag(P_V, s.overflow);
}

/*
 * @routine 01E2:451 MariaObjectProbe
 * @run 6E:CC51
 * WHAT IT DOES: Reads the terrain at a point offset from an object: zTemp04-05 across, zTemp06-07 down. A point below
 * the bottom of the first screen down reads as nothing.
 * HOW: The point to zTemp00-03 (its Y first; a carry out of the low byte is the bottom), then the tail JMP to the
 * stage overlay's read at $8006 (StageTileRead).
 * WAITS: no.
 * Entry: X the object, zTemp04-07 the offset. Exit: past the bottom, A 0, N and Z from the Y add, C set, V from it.
 * Else as the stage's read left the processor.
 */
void MariaObjectProbe(void)
{
    uint8_t object = cpu.x;
    uint8_t value, offset;
    Sum s;

    value = RD8(OBJ->y_lo[object]);
    offset = RD8(ZP->temp06);
    s = add_bytes(value, offset, false);
    WR8(ZP->temp02, s.value);
    if (s.carry) {
        p_sum(s);
        cpu.a = 0;
        return;
    }
    value = RD8(OBJ->y_hi[object]);
    offset = RD8(ZP->temp07);
    s = add_bytes(value, offset, s.carry);
    WR8(ZP->temp03, s.value);
    value = RD8(OBJ->x_lo[object]);
    offset = RD8(ZP->temp04);
    s = add_bytes(value, offset, false);
    WR8(ZP->temp00, s.value);
    value = RD8(OBJ->x_hi[object]);
    offset = RD8(ZP->temp05);
    s = add_bytes(value, offset, s.carry);
    WR8(ZP->temp01, s.value);
    p_sum(s);
    terrain_read_exit_shim(StageTileRead());
}

void abi_MariaObjectProbe(void)
{
    MariaObjectProbe();
}

/*
 * @routine 01E2:480 MariaObjectProbeAt
 * @run 6E:CC80
 * WHAT IT DOES: Reads the terrain at a point offset from an object by A across and Y down, each a signed byte.
 * HOW: The offsets to zTemp04 and zTemp06 (read back for their signs, which make zTemp05 and zTemp07), then the tail
 * JMP to MariaObjectProbe.
 * WAITS: no.
 * Entry: A across, Y down, X the object. Exit: as MariaObjectProbe.
 */
void MariaObjectProbeAt(void)
{
    uint8_t value;

    WR8(ZP->temp07, 0);
    WR8(ZP->temp05, 0);
    WR8(ZP->temp04, cpu.a);
    value = RD8(ZP->temp04);
    p_lda(value);
    if (value & 0x80) {
        abi_nz(DEC8(ZP->temp05));
    }
    WR8(ZP->temp06, cpu.y);
    cpu.y = RD8(ZP->temp06);
    abi_nz(cpu.y);
    if (cpu.y & 0x80) {
        abi_nz(DEC8(ZP->temp07));
    }
    MariaObjectProbe();
}

void abi_MariaObjectProbeAt(void)
{
    MariaObjectProbeAt();
}

/*
 * @routine 01E2:476 MariaObjectProbeAtBit1
 * @run 6E:CC76
 * WHAT IT DOES: MariaObjectProbeAt's answer, bit 1 only.
 * HOW: A BSR to MariaObjectProbeAt, then AND #$02.
 * WAITS: no.
 * Entry: as MariaObjectProbeAt. Exit: A the bit, N and Z from it; the rest as MariaObjectProbe left them.
 */
void MariaObjectProbeAtBit1(void)
{
    jsr_push(0xCC78);
    MariaObjectProbeAt();
    jsr_pop();
    p_lda((uint8_t)(cpu.a & 0x02));
}

void abi_MariaObjectProbeAtBit1(void)
{
    MariaObjectProbeAtBit1();
}

/*
 * @routine 01E2:47B MariaObjectProbeAtSolid
 * @run 6E:CC7B
 * WHAT IT DOES: MariaObjectProbeAt's answer, bits 0 and 4 only.
 * HOW: A BSR to MariaObjectProbeAt, then AND #$11.
 * WAITS: no.
 * Entry: as MariaObjectProbeAt. Exit: A the bits, N and Z from them; the rest as MariaObjectProbe left them.
 */
void MariaObjectProbeAtSolid(void)
{
    jsr_push(0xCC7D);
    MariaObjectProbeAt();
    jsr_pop();
    p_lda((uint8_t)(cpu.a & 0x11));
}

void abi_MariaObjectProbeAtSolid(void)
{
    MariaObjectProbeAtSolid();
}

/*
 * @routine 01E2:431 MariaObjectProbeBelow
 * @run 6E:CC31
 * WHAT IT DOES: Reads the terrain A rows below an object: bits 0 and 4 of the answer, or nothing for an answer with
 * bit 7 set.
 * HOW: The offset to zTemp06 (zTemp04, zTemp05 and zTemp07 zero), MariaObjectProbe, then AND #$11, or CLA on N set.
 * WAITS: no.
 * Entry: A rows down, X the object. Exit: A the bits, N and Z from them; or A 0 with N set; the rest as
 * MariaObjectProbe left them.
 */
void MariaObjectProbeBelow(void)
{
    WR8(ZP->temp06, cpu.a);
    WR8(ZP->temp07, 0);
    WR8(ZP->temp04, 0);
    WR8(ZP->temp05, 0);
    jsr_push(0xCC3C);
    MariaObjectProbe();
    jsr_pop();
    if (cpu.p & P_N) {
        cpu.a = 0;
        return;
    }
    p_lda((uint8_t)(cpu.a & 0x11));
}

void abi_MariaObjectProbeBelow(void)
{
    MariaObjectProbeBelow();
}

/*
 * @routine 01E2:443 MariaObjectProbeBelowAny
 * @run 6E:CC43
 * WHAT IT DOES: Reads the terrain A rows below an object: bits 0, 4 and 5 of the answer.
 * HOW: The offset to zTemp06 (zTemp04, zTemp05 and zTemp07 zero), MariaObjectProbe, then AND #$31.
 * WAITS: no.
 * Entry: A rows down, X the object. Exit: A the bits, N and Z from them; the rest as MariaObjectProbe left them.
 */
void MariaObjectProbeBelowAny(void)
{
    WR8(ZP->temp06, cpu.a);
    WR8(ZP->temp07, 0);
    WR8(ZP->temp04, 0);
    WR8(ZP->temp05, 0);
    jsr_push(0xCC4E);
    MariaObjectProbe();
    jsr_pop();
    p_lda((uint8_t)(cpu.a & 0x31));
}

void abi_MariaObjectProbeBelowAny(void)
{
    MariaObjectProbeBelowAny();
}

/*
 * @routine 01E1:473 MariaStagePointStep
 * @run 6E:C473
 * WHAT IT DOES: Gives the stage overlay's entry at $8009 the point in zTemp08-09, then moves the point $40 on and
 * counts X down.
 * HOW: The JSR to $8009 (the C of the routine resident there), a 16-bit add of $0040, DEX.
 * WAITS: no.
 * Entry: X the count, the rest as the stage's code takes them. Exit: A the point's high byte, C and V from its add, X
 * one less, N and Z from it; Y as the stage's code left it.
 */
void MariaStagePointStep(void)
{
    uint8_t value;
    Sum lo, hi;

    call_routine(0xC476, FrameOverlay8009);
    value = RD8(ZP->temp08);
    lo = add_bytes(value, PROBE_ROW_STEP, false);
    WR8(ZP->temp08, lo.value);
    value = RD8(ZP->temp09);
    hi = add_bytes(0, value, lo.carry);
    WR8(ZP->temp09, hi.value);
    p_sum(hi);
    cpu.x = (uint8_t)(cpu.x - 1);
    abi_nz(cpu.x);
}

void abi_MariaStagePointStep(void)
{
    MariaStagePointStep();
}

/*
 * @routine 01E1:456 MariaStagePointRow
 * @run 6E:C456
 * WHAT IT DOES: Gives the stage overlay's entry at $8009 six points $40 apart from $6080, each with her OBJ_WORK_K in
 * A; whatever the stage's code leaves in zTemp0F is her OBJ_WORK_K after; her OBJ_WORK_J counts one down.
 * HOW: MariaStagePointStep six times (X 6 down to 0).
 * WAITS: no.
 * Entry: as the stage's code takes them. Exit: A zTemp0F, N and Z from the new OBJ_WORK_J, X 0; C and V from the
 * last point's add; Y as the stage's code left it.
 */
void MariaStagePointRow(void)
{
    uint8_t value;

    WR8(ZP->temp09, PROBE_ROW_HI);
    WR8(ZP->temp08, PROBE_ROW_LO);
    cpu.x = PROBE_ROW_POINTS;
    do {
        value = RD8(OBJ->work_k[OBJECT_PLAYER]);
        p_lda(value);
        WR8(ZP->temp0f, value);
        jsr_push(0xC468);
        MariaStagePointStep();
        jsr_pop();
    } while (!(cpu.p & P_Z));
    value = RD8(ZP->temp0f);
    cpu.a = value;
    WR8(OBJ->work_k[OBJECT_PLAYER], value);
    abi_nz(DEC8(OBJ->work_j[OBJECT_PLAYER]));
}

void abi_MariaStagePointRow(void)
{
    MariaStagePointRow();
}

/*
 * @routine 01E1:443 MariaStagePointFrame
 * @run 6E:C443
 * WHAT IT DOES: A frame of her stage's point row (step 2 of the state that runs it): the columns under her, the floor
 * and the landing, then MariaStagePointRow; when its count in OBJ_WORK_J runs out, zField89 is set to 1.
 * HOW: MariaReadColumns, MariaFloorCheck, MariaLandOnFloor; a BSR to MariaStagePointRow, whose Z says the count ran
 * out.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaStagePointRow; at the end A 1, N and Z from it.
 */
void MariaStagePointFrame(void)
{
    ColumnsEnd columns;
    FloorResult floor;
    LandingLift landing;

    jsr_push(0xC446);
    columns = MariaReadColumns();
    jsr_pop();
    columns_exit_shim(columns);
    jsr_push(0xC449);
    floor = MariaFloorCheck();
    jsr_pop();
    floor_check_exit_shim(floor);
    jsr_push(0xC44C);
    landing = MariaLandOnFloor();
    jsr_pop();
    landing_exit_shim(landing);
    jsr_push(0xC44E);
    MariaStagePointRow();
    jsr_pop();
    if (!(cpu.p & P_Z)) {
        return;
    }
    p_lda(STAGE_STEP_DONE);
    WR8(ZP->field89, STAGE_STEP_DONE);
}

void abi_MariaStagePointFrame(void)
{
    MariaStagePointFrame();
}
