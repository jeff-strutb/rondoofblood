/* A stage's gauge: a value kept in Field2742 drawn as a bar of VRAM words that grows and shrinks towards a target. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    BAR_FULL = 0x5E, BAR_SECOND_HALF = 0x3F, BAR_SECOND_BASE = 0xC0, BAR_COLUMN_BASE = 0x40, BAR_HIGH_BASE = 0x7D,
    TARGET_SLOT = 16, OBJECT_SLOT = 85, FRAME_LIMIT = 0xD8, ROW_SLOT = 2, SPAWN_RETURN = 0x8C71, SET_RETURN = 0x8D0C,
    DOWN_ADDRESS_RETURN = 0x8C99, DOWN_RETURN = 0x8CA0, TIMER_RETURN = 0x8CAA, UP_ADDRESS_RETURN = 0x8CBC,
    UP_RETURN = 0x8CC3, OTHER_RUN = 0x8D25, DOWN_X = 0x0F, DOWN_Y = 0xFF, UP_X = 0xF0, UP_Y = 0x00
};

/* The parts of an exit the later one sets win over the earlier. */
static StageExit merge_exit(StageExit before, StageExit after)
{
    StageExit now = before;

    now.a = after.a;
    now.nz = after.nz;
    if ((after.set & EXIT_X) != 0) {
        now.x = after.x;
    }
    if ((after.set & EXIT_Y) != 0) {
        now.y = after.y;
    }
    if ((after.set & EXIT_C) != 0) {
        now.c = after.c;
    }
    if ((after.set & EXIT_V) != 0) {
        now.v = after.v;
    }
    now.set = (uint8_t)(before.set | after.set);
    return now;
}

/*
 * @routine 05BE:4C4 SetGaugeAddress
 * @run 6B:8CC4
 * WHAT IT DOES: Points the VDC at the gauge's cell for its value (Field2742): none when the value is zero.
 * HOW: $5E minus the value gives a cell number; its low nibble plus $40 and its high nibble times eight make
 * an offset from $7D00, plus $C0 for the second half; SetVramAddressesLong does the rest.
 * Ten stage overlays hold this code, the ones whose VRAM routine is at $968F.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X and Y the address, N 0, Z 0, C and V from the high add; with a zero
 * value A 0 and the rest kept.
 */
StageExit SetGaugeAddress(void)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    uint8_t a = RD8(LEVEL->field2742);
    uint8_t m;
    uint8_t low;
    uint8_t high;
    bool carry;
    Sum sum;
    Sum offset;
    Sum column;
    Sum row;

    if (a == 0) {
        return end;
    }
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    if (a >= BAR_SECOND_HALF) {
        WR8(ZP->temp08, BAR_SECOND_BASE);
        INC8(ZP->temp09);
    }
    m = RD8(LEVEL->field2742);
    sum = sub_bytes(BAR_FULL, m, true);
    push8(sum.value);
    a = (uint8_t)(sum.value & 0x0F);
    offset = add_bytes(a, BAR_COLUMN_BASE, false);
    WR8(ZP->temp00, offset.value);
    a = pull8();
    a = (uint8_t)(a & 0xF0);
    WR8(ZP->temp01, 0);
    carry = (a & 0x80) != 0;
    a = (uint8_t)(a << 1);
    ROL8(ZP->temp01, carry);
    carry = (a & 0x80) != 0;
    a = (uint8_t)(a << 1);
    ROL8(ZP->temp01, carry);
    carry = (a & 0x80) != 0;
    a = (uint8_t)(a << 1);
    ROL8(ZP->temp01, carry);
    low = RD8(ZP->temp00);
    sum = add_bytes(a, low, false);
    WR8(ZP->temp00, sum.value);
    high = RD8(ZP->temp01);
    column = add_bytes(high, BAR_HIGH_BASE, sum.carry);
    WR8(ZP->temp01, column.value);
    m = RD8(ZP->temp08);
    low = RD8(ZP->temp00);
    offset = add_bytes(m, low, false);
    m = RD8(ZP->temp09);
    high = RD8(ZP->temp01);
    row = add_bytes(m, high, offset.carry);
    jsr_push(SET_RETURN);
    SetVramAddressesLong(offset.value, row.value);
    jsr_pop();
    end.a = 2;
    end.nz = 2;
    end.x = offset.value;
    end.y = row.value;
    end.c = row.carry;
    end.v = row.overflow;
    end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    return end;
}

void abi_SetGaugeAddress(void)
{
    stage_exit_shim(SetGaugeAddress());
}

/*
 * @routine 05BE:4B7 GaugeStepUp
 * @run 6B:8CB7
 * WHAT IT DOES: Grows the gauge one: its value up by one, its cell found, and its bits set.
 * HOW: SetGaugeAddress, then OrVramData with $F0 and 0.
 * Ten stage overlays hold this code.
 * WAITS: no.
 * Entry: nothing. Exit: A the high byte written, N and Z from it, X $F0, Y 0.
 */
StageExit GaugeStepUp(void)
{
    StageExit end = { 0, UP_X, UP_Y, false, false, 0, EXIT_X | EXIT_Y };
    StageExit address;
    uint8_t a;

    INC8(LEVEL->field2742);
    jsr_push(UP_ADDRESS_RETURN);
    address = SetGaugeAddress();
    jsr_pop();
    end = merge_exit(end, address);
    end.x = UP_X;
    end.y = UP_Y;
    end.set |= EXIT_X | EXIT_Y;
    jsr_push(UP_RETURN);
    a = OrVramData(UP_X, UP_Y);
    jsr_pop();
    end.a = a;
    end.nz = a;
    return end;
}

void abi_GaugeStepUp(void)
{
    stage_exit_shim(GaugeStepUp());
}

static StageExit gauge_timer_check(StageExit held);

/*
 * @routine 05BE:48B GaugeStep
 * @run 6B:8C8B
 * WHAT IT DOES: Moves the gauge one step towards its target (slot 16 of the f32a8 column): up when the target
 * is above the value, down when below, nothing when equal.
 * HOW: Compares the target with Field2742; GaugeStepUp for up; for down SetGaugeAddress, AndVramData with
 * $0F and $FF and the value down by one. A negative target is held, the timer check run and the
 * target put back (untraced).
 * Ten stage overlays hold this code.
 * WAITS: no.
 * Entry: nothing. Exit: as the step taken.
 */
StageExit GaugeStep(void)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    StageExit address;
    uint8_t a = RD8(OBJ->f32a8[TARGET_SLOT]);
    uint8_t m;
    uint8_t y;

    if ((a & 0x80) != 0) {
        push8(a);
        WR8(OBJ->f32a8[TARGET_SLOT], 0);
        jsr_push(TIMER_RETURN);
        end = gauge_timer_check(end);
        jsr_pop();
        a = pull8();
        WR8(OBJ->f32a8[TARGET_SLOT], a);
        y = RD8(LEVEL->field2742);
        end.a = a;
        end.y = y;
        end.nz = y;
        end.set |= EXIT_Y;
        if (y == 0) {
            WR8(OBJ->life_timer[OBJECT_SLOT], a);
        }
        return end;
    }
    m = RD8(LEVEL->field2742);
    end.a = a;
    end.c = a >= m;
    end.nz = (uint8_t)(a - m);
    end.set = EXIT_C;
    if (a == m) {
        return end;
    }
    if (a >= m) {
        return merge_exit(end, GaugeStepUp());
    }
    jsr_push(DOWN_ADDRESS_RETURN);
    address = SetGaugeAddress();
    jsr_pop();
    end = merge_exit(end, address);
    jsr_push(DOWN_RETURN);
    a = AndVramData(DOWN_X, DOWN_Y);
    jsr_pop();
    end.a = a;
    end.x = DOWN_X;
    end.y = DOWN_Y;
    end.set |= EXIT_X | EXIT_Y;
    end.nz = DEC8(LEVEL->field2742);
    return end;
}

void abi_GaugeStep(void)
{
    stage_exit_shim(GaugeStep());
}

/* The code the stage runs on after its timer says to, which has no C: it gets A and the flags. */
static StageExit other_stage_code_shim(uint8_t a, StageExit held)
{
    StageExit done = { 0, 0, 0, false, false, 0, EXIT_CPU };

    cpu.a = a;
    abi_nz(a);
    if ((held.set & EXIT_X) != 0) {
        cpu.x = held.x;
    }
    if ((held.set & EXIT_Y) != 0) {
        cpu.y = held.y;
    }
    if ((held.set & EXIT_C) != 0) {
        abi_flag(P_C, held.c);
    }
    if ((held.set & EXIT_V) != 0) {
        abi_flag(P_V, held.v);
    }
    call_original(OTHER_RUN);
    return done;
}

/* From the timer on: when the life timer of slot 85 is zero, the gauge steps; otherwise the stage's other code runs. */
static StageExit gauge_timer_check(StageExit held)
{
    uint8_t a = RD8(OBJ->life_timer[OBJECT_SLOT]);

    if (a == 0) {
        return GaugeStep();
    }
    return other_stage_code_shim(a, held);
}

/*
 * @routine 05BE:46C RunStageGauge
 * @run 6B:8C6C
 * WHAT IT DOES: The gauge object's frame: spawns row 2 until the object's frame (slot 85) is set, clears the
 * gauge value then, lets the object's X low byte down by one unless it is $D8, and then the timer check.
 * HOW: SpawnStageObjectLong for row 2 in a loop; the rest as above.
 * Ten stage overlays hold this code; the stage's frame code reaches it by a jump.
 * WAITS: no.
 * Entry: nothing. Exit: as the step the timer check took.
 */
StageExit RunStageGauge(void)
{
    StageExit end = { 0, ROW_SLOT, 0, false, false, 0, EXIT_X | EXIT_Y | EXIT_C };
    SpawnedObject spawned;
    uint8_t a;
    uint8_t m;

    do {
        jsr_push(SPAWN_RETURN);
        spawned = SpawnStageObjectLong(ROW_SLOT);
        jsr_pop();
        WR8(LEVEL->field2742, 0);
        a = RD8(OBJ->frame[OBJECT_SLOT]);
    } while (a == 0);
    end.y = spawned.slot;
    a = RD8(OBJ->x_lo[OBJECT_SLOT]);
    end.c = a >= FRAME_LIMIT;
    if (a != FRAME_LIMIT) {
        DEC8(OBJ->x_lo[OBJECT_SLOT]);
    }
    m = a;
    (void)m;
    return merge_exit(end, gauge_timer_check(end));
}

void abi_RunStageGauge(void)
{
    stage_exit_shim(RunStageGauge());
}
