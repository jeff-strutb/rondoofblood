/* The overlay's hit test: an object's hitbox against the sixteen slots of the other side, and what a hit does. */
#include <stddef.h>
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    SLOT_OTHER_SIDE_FIRST = 0x0F, HANDLER_STORE_A = 0x9543, HANDLER_REPORT_A = 0x9568, HANDLER_STORE_B = 0x9855,
    HANDLER_REPORT_B = 0x987A, TABLE_A = 0x9525, TABLE_B = 0x9837, SEVEN_HITBOX_BIT = 0x80
};

/* An exit with every part given. */
static StageExit hit_exit(uint8_t a, uint8_t y, bool c, bool v, uint8_t nz, uint8_t set)
{
    StageExit end = { a, 0, y, c, v, nz, set };

    return end;
}

/*
 * The store handler's body, for the builds of it that keep their columns elsewhere or test a gate byte first: the
 * slot's value (kind, by Y) into the object's byte (taken, by X); when the first object's byte is empty and the gate
 * byte, if the build has one, is 0, the first object's byte and the byte at to_b take the object's from_a and from_b.
 * gate 0 is the build without the test (HitStoreHandler's), whose bytes have no load of a gate.
 */
StageExit HitStoreHandlerAt(uint8_t x, uint8_t y, const HitStoreColumns *c)
{
    uint8_t a = RD8_IDX(c->kind, y);

    WR8_IDX(c->taken, x, a);
    a = RD8_IDX(c->taken, 0);
    if (a == 0) {
        if (c->gate != 0) {
            a = RD8_IDX(c->gate, 0);
            if (a != 0) {
                return hit_exit(a, 0, false, false, a, 0);
            }
        }
        a = RD8_IDX(c->from_a, x);
        WR8_IDX(c->taken, 0, a);
        a = RD8_IDX(c->from_b, x);
        WR8_IDX(c->to_b, 0, a);
    }
    return hit_exit(a, 0, false, false, a, 0);
}

/*
 * @routine 024F:543 HitStoreHandler
 * @run 6B:9543
 * WHAT IT DOES: What a hit does for the kinds that take the hit's value: the hit slot's value
 * goes into the object's hit-taken byte, and when slot 0's hit-taken byte was empty it is given
 * the object's own two stored bytes.
 * HOW: The entry of the hit test's jump table for the first four kinds.
 * Eleven stage overlays hold this code, the ones whose hit test is at $9466.
 * WAITS: no.
 * Entry: X the object, Y the hit slot. Exit: A the last byte loaded, N and Z from it. C and V kept.
 */
static StageExit hit_store(uint8_t x, uint8_t y)
{
    const HitStoreColumns c = { COL(OBJ->work_n), COL(OBJ->hit_taken), COL(OBJ->work_c), COL(OBJ->work_b),
                                ADDR_OF8(OBJ->health[0]), 0 };

    return HitStoreHandlerAt(x, y, &c);
}

StageExit HitStoreHandler(uint8_t x, uint8_t y)
{
    return hit_store(x, y);
}

void abi_HitStoreHandler(void)
{
    stage_exit_shim(HitStoreHandler(cpu.x, cpu.y));
}

/*
 * @routine 024F:568 HitReportHandler
 * @run 6B:9568
 * WHAT IT DOES: What a hit does for the kinds that report it: the hit's position (the two
 * work bytes and the other side's hitbox position) go to the stage hit report, and the hit
 * slot's value to the object's hit-taken byte.
 * HOW: Four copies into StageHitReport, then the same store as HitStoreHandler's first.
 * Every stage overlay holds this code.
 * WAITS: no.
 * Entry: X the object, Y the hit slot. Exit: A the value stored, N and Z from it. C and V kept.
 */
static StageExit hit_report(uint8_t x, uint8_t y)
{
    uint8_t a;

    a = RD8(LEVEL->stream_count.hi);
    WR8(STAGEHIT->bytes[0], a);
    a = RD8(LEVEL->stream_count.lo);
    WR8(STAGEHIT->bytes[1], a);
    a = RD8(HITBOX->x_lo[3]);
    WR8(STAGEHIT->bytes[2], a);
    a = RD8(HITBOX->y_lo[3]);
    WR8(STAGEHIT->bytes[3], a);
    a = RD8(OBJ->work_n[y]);
    WR8(OBJ->hit_taken[x], a);
    return hit_exit(a, 0, false, false, a, 0);
}

StageExit HitReportHandler(uint8_t x, uint8_t y)
{
    return hit_report(x, y);
}

void abi_HitReportHandler(void)
{
    stage_exit_shim(HitReportHandler(cpu.x, cpu.y));
}

/*
 * @routine 05C0:055 HitStoreHandlerLong
 * @run 6B:9855
 * WHAT IT DOES: HitStoreHandler in 05BF's group of stage overlays: the hit slot's value into the object's hit-taken
 * byte, and slot 0's given the object's two stored bytes when its own was empty.
 * HOW: The same instructions as HitStoreHandler, the entry of HitTestObjectLong's jump table for the first four kinds.
 * The thirteen overlays of 05BF's group hold these bytes.
 * WAITS: no.
 * Entry: X the object, Y the hit slot. Exit: as HitStoreHandler.
 */
StageExit HitStoreHandlerLong(uint8_t x, uint8_t y)
{
    return hit_store(x, y);
}

void abi_HitStoreHandlerLong(void)
{
    stage_exit_shim(HitStoreHandlerLong(cpu.x, cpu.y));
}

/*
 * @routine 05C0:07A HitReportHandlerLong
 * @run 6B:987A
 * WHAT IT DOES: HitReportHandler in 05BF's group of stage overlays: the hit's position to the stage hit report and the
 * hit slot's value to the object's hit-taken byte.
 * HOW: The same instructions as HitReportHandler. The thirteen overlays of 05BF's group hold these bytes.
 * WAITS: no.
 * Entry: X the object, Y the hit slot. Exit: as HitReportHandler.
 */
StageExit HitReportHandlerLong(uint8_t x, uint8_t y)
{
    return hit_report(x, y);
}

void abi_HitReportHandlerLong(void)
{
    stage_exit_shim(HitReportHandlerLong(cpu.x, cpu.y));
}

/*
 * The JMP [temp00] into a handler other than the group's two above: the registers and flags as the test leaves them,
 * then the routine loaded at that address (routine_at), which returns to the test's caller with its own registers.
 */
static StageExit loaded_handler_shim(uint16_t handler, uint8_t a, uint8_t x, uint8_t y, bool c, bool v)
{
    StageExit done = { 0, 0, 0, false, false, 0, EXIT_CPU };

    cpu.a = a;
    cpu.x = x;
    cpu.y = y;
    abi_nz(y);
    abi_flag(P_C, c);
    abi_flag(P_V, v);
    run_routine(handler);
    return done;
}

/*
 * The hit test: the object X's hitbox, placed by its offsets and facing, against each slot of the
 * other side that is on and not yet hit this frame; the first it overlaps by both half sizes
 * records the object in its slot, and its kind (StageVar f3828) picks the handler from the
 * overlay's table.
 */
static const HitHandlers handlers_024f = { HANDLER_STORE_A, HANDLER_REPORT_A, HitStoreHandler, HitReportHandler };
static const HitHandlers handlers_05bf = { HANDLER_STORE_B, HANDLER_REPORT_B, HitStoreHandlerLong, HitReportHandlerLong };

StageExit HitTestObjectAt(uint8_t x, uint16_t table, const HitHandlers *handlers)
{
    uint8_t a;
    uint8_t m;
    uint8_t y;
    uint8_t nz;
    bool c;
    bool v;
    Sum sum;
    uint16_t handler;

    WR8(OBJ->hit_taken[x], 0);
    a = RD8(OBJ->hitbox_on[x]);
    if (a == 0) {
        return hit_exit(0, 0, false, false, 0, 0);
    }
    a = RD8(OBJ->hitbox_half_w[x]);
    m = RD8(OBJ->hitbox_half_h[x]);
    a = (uint8_t)(a | m);
    if (a == 0) {
        return hit_exit(0, 0, false, false, 0, 0);
    }
    a = RD8(OBJ->hitbox_dx[x]);
    WR8(ZP->temp01, a);
    a = RD8(OBJ->facing_left[x]);
    if (a != 0) {
        a = RD8(ZP->temp01);
        a = (uint8_t)(~a + 1);
        WR8(ZP->temp01, a);
    }
    WR8(ZP->temp00, 0);
    a = RD8(ZP->temp01);
    if ((a & 0x80) != 0) {
        DEC8(ZP->temp00);
    }
    m = RD8(OBJ->x_lo[x]);
    sum = add_bytes(a, m, false);
    a = sum.value;
    WR8(LEVEL->stream_count.hi, a);
    a = RD8(OBJ->x_hi[x]);
    m = RD8(ZP->temp00);
    sum = add_bytes(a, m, sum.carry);
    a = sum.value;
    if (a != 0) {
        return hit_exit(a, 0, sum.carry, sum.overflow, a, EXIT_C | EXIT_V);
    }
    WR8(ZP->temp00, 0);
    a = RD8(OBJ->hitbox_dy[x]);
    if ((a & 0x80) != 0) {
        DEC8(ZP->temp00);
    }
    m = RD8(OBJ->y_lo[x]);
    sum = add_bytes(a, m, false);
    a = sum.value;
    WR8(LEVEL->stream_count.lo, a);
    a = RD8(OBJ->y_hi[x]);
    m = RD8(ZP->temp00);
    sum = add_bytes(a, m, sum.carry);
    a = sum.value;
    c = sum.carry;
    v = sum.overflow;
    if (a != 0) {
        return hit_exit(a, 0, c, v, a, EXIT_C | EXIT_V);
    }
    a = RD8(OBJ->hitbox_on[x]);
    y = (a & SEVEN_HITBOX_BIT) != 0 ? 0 : SLOT_OTHER_SIDE_FIRST;
    nz = a;
    for (;;) {
        a = RD8(OBJ->hitbox_on[y]);
        nz = a;
        if (a != 0) {
            a = RD8(HITBOX->hi[y]);
            nz = a;
            if (a == 0) {
                bool inside = false;

                a = RD8(OBJ->hitbox_half_w[x]);
                m = RD8(OBJ->hitbox_half_w[y]);
                sum = add_bytes(a, m, false);
                a = sum.value;
                c = sum.carry;
                v = sum.overflow;
                WR8(LEVEL->stream_step.lo, a);
                a = RD8(OBJ->hitbox_half_h[x]);
                m = RD8(OBJ->hitbox_half_h[y]);
                sum = add_bytes(a, m, false);
                a = sum.value;
                c = sum.carry;
                v = sum.overflow;
                WR8(LEVEL->stream_step.hi, a);
                a = RD8(LEVEL->stream_count.hi);
                m = RD8(HITBOX->x_lo[y]);
                sum = sub_bytes(a, m, true);
                a = sum.value;
                c = sum.carry;
                v = sum.overflow;
                if (!c) {
                    a = (uint8_t)(~a + 1);
                }
                m = RD8(LEVEL->stream_step.lo);
                c = a >= m;
                nz = (uint8_t)(a - m);
                if (!c) {
                    a = RD8(LEVEL->stream_count.lo);
                    m = RD8(HITBOX->y_lo[y]);
                    sum = sub_bytes(a, m, true);
                    a = sum.value;
                    c = sum.carry;
                    v = sum.overflow;
                    if (!c) {
                        a = (uint8_t)(~a + 1);
                    }
                    m = RD8(LEVEL->stream_step.hi);
                    c = a >= m;
                    nz = (uint8_t)(a - m);
                    inside = !c;
                }
                if (inside) {
                    break;
                }
            }
        }
        y--;
        nz = y;
        if (y >= 0x80) {
            return hit_exit(a, y, c, v, nz, EXIT_Y | EXIT_C | EXIT_V);
        }
    }
    a = x;
    WR8(OBJ->work_o[y], a);
    WR8(ZP->hit_slot, y);
    a = RD8(OBJ->work_n[y]);
    if (a == 0) {
        return hit_exit(0, y, c, v, 0, EXIT_Y | EXIT_C | EXIT_V);
    }
    a = (uint8_t)(a - 1);
    c = (a & 0x80) != 0;
    a = (uint8_t)(a << 1);
    y = a;
    a = rd8((uint16_t)(table + y));
    WR8(ZP->temp00, a);
    a = rd8((uint16_t)(table + 1 + y));
    WR8(ZP->temp01, a);
    y = RD8(ZP->hit_slot);
    handler = RD16_LH2(ZP->temp00, ZP->temp01);
    if (handlers != NULL && (handler == handlers->store || handler == handlers->report)) {
        StageExit done = handler == handlers->store ? handlers->store_c(x, y) : handlers->report_c(x, y);

        done.y = y;
        done.c = c;
        done.v = v;
        done.set = EXIT_Y | EXIT_C | EXIT_V;
        return done;
    }
    return loaded_handler_shim(handler, a, x, y, c, v);
}

/*
 * @routine 024F:466 HitTestObject
 * @run 6B:9466
 * @stop 9525
 * WHAT IT DOES: Tests the object X's hitbox against the hitboxes of the other side's sixteen
 * slots (an object with its high bit set in hitbox_on tests the first, the rest the last)
 * and, on the first overlap, runs the handler its kind names.
 * HOW: The hitbox is placed by its offsets, mirrored when the object faces left; each live slot
 * not already hit is compared by the sum of the half sizes on both axes.
 * Eleven stage overlays hold this code, the ones whose handlers are at $9543 and $9568.
 * WAITS: no.
 * Entry: X the object. Exit: with no hit A the last value, Y $FF, N set; with one, as the handler.
 */
StageExit HitTestObject(uint8_t x)
{
    return HitTestObjectAt(x, TABLE_A, &handlers_024f);
}

void abi_HitTestObject(void)
{
    stage_exit_shim(HitTestObject(cpu.x));
}

/*
 * @routine 05BF:778 HitTestObjectLong
 * @run 6B:9778
 * @stop 9837
 * WHAT IT DOES: HitTestObject for the other stage group, which keeps its hit test at $9778 and its
 * handlers at $9855 and $987A.
 * HOW: The same code with its own table; its handlers with C are HitStoreHandlerLong and HitReportHandlerLong.
 * WAITS: no.
 * Entry, exit: as HitTestObject.
 */
StageExit HitTestObjectLong(uint8_t x)
{
    return HitTestObjectAt(x, TABLE_B, &handlers_05bf);
}

void abi_HitTestObjectLong(void)
{
    stage_exit_shim(HitTestObjectLong(cpu.x));
}

/*
 * @routine 18CF:778 HitTestObjectLongB
 * @run 6B:9778
 * @stop 9837
 * WHAT IT DOES: HitTestObjectLong for the overlay at 18CF, whose bytes the intake keeps apart from the
 * other overlays' because its handler table and what follows it differ.
 * HOW: The same code with the same table address.
 * WAITS: no.
 * Entry, exit: as HitTestObject.
 */
StageExit HitTestObjectLongB(uint8_t x)
{
    return HitTestObjectAt(x, TABLE_B, &handlers_05bf);
}

void abi_HitTestObjectLongB(void)
{
    stage_exit_shim(HitTestObjectLongB(cpu.x));
}

enum { LAST_OBJECT = 0x57, FIRST_OBJECT = 0x10 };

/*
 * Works out the hitbox positions, then tests each object from the last down to $10, clearing its
 * hit-taken byte and counting its timer down when the timer is not zero.
 */
StageExit HitTestAllObjectsAt(uint8_t entry_x, HitboxEnd (*compute)(void), StageExit (*test)(uint8_t x),
                             uint16_t compute_return, uint16_t test_return)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    StageExit step;
    HitboxEnd boxes;
    uint8_t a;
    uint8_t x;
    uint8_t nz;
    bool c;

    push8(entry_x);
    jsr_push(compute_return);
    boxes = compute();
    jsr_pop();
    a = boxes.a;
    if (boxes.v_seen) {
        end.v = boxes.v;
        end.set |= EXIT_V;
    }
    x = LAST_OBJECT;
    do {
        jsr_push(test_return);
        step = test(x);
        jsr_pop();
        if ((step.set & EXIT_CPU) != 0) {
            step = stage_exit_from_cpu_shim();
        }
        a = step.a;
        if ((step.set & EXIT_X) != 0) {
            x = step.x;
        }
        if ((step.set & EXIT_Y) != 0) {
            end.y = step.y;
            end.set |= EXIT_Y;
        }
        if ((step.set & EXIT_V) != 0) {
            end.v = step.v;
            end.set |= EXIT_V;
        }
        a = RD8(OBJ->invuln_timer[x]);
        if (a != 0) {
            WR8(OBJ->hit_taken[x], 0);
            DEC8(OBJ->invuln_timer[x]);
        }
        x--;
        c = x >= FIRST_OBJECT;
        nz = (uint8_t)(x - FIRST_OBJECT);
    } while (c);
    x = pull8();
    (void)nz;
    end.a = a;
    end.x = x;
    end.c = false;
    end.nz = x;
    end.set |= EXIT_X | EXIT_C;
    return end;
}

/*
 * @routine 024F:3F8 HitTestAllObjects
 * @run 6B:93F8
 * WHAT IT DOES: One frame of hit testing: places the first sixteen hitboxes and tests the
 * objects from $57 down to $10, clearing the hit-taken byte of each whose timer is running and
 * counting that timer down.
 * HOW: ComputeHitboxPositions, then HitTestObject for each object.
 * Eleven stage overlays hold this code, the ones whose hit test is at $9466.
 * WAITS: no.
 * Entry: X kept on the stack. Exit: as the last object left it; X kept, C 0.
 */
StageExit HitTestAllObjects(uint8_t x)
{
    return HitTestAllObjectsAt(x, ComputeHitboxPositions, HitTestObject, 0x93FC, 0x9401);
}

void abi_HitTestAllObjects(void)
{
    stage_exit_shim(HitTestAllObjects(cpu.x));
}

/*
 * @routine 05BF:70A HitTestAllObjectsLong
 * @run 6B:970A
 * WHAT IT DOES: HitTestAllObjects for the other stage group, whose hit test is at $9778.
 * HOW: The same code at other run addresses, calling ComputeHitboxPositionsLong and HitTestObjectLong.
 * WAITS: no.
 * Entry, exit: as HitTestAllObjects.
 */
StageExit HitTestAllObjectsLong(uint8_t x)
{
    return HitTestAllObjectsAt(x, ComputeHitboxPositionsLong, HitTestObjectLong, 0x970E, 0x9713);
}

void abi_HitTestAllObjectsLong(void)
{
    stage_exit_shim(HitTestAllObjectsLong(cpu.x));
}
