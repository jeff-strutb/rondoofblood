/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 1018 to 18DD (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it (the types in stage_enemies_l8.h); each routine's exit shim builds A, Y and the flags from them, and the
 * abi_ wrappers set the registers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

enum { DROP_FEW_HITS, DROP_NO_BITS, DROP_NOT_DRAWN, DROP_NO_SLOT, DROP_PLACED };  /* as in enemies_l8_st20.c (DropArm) */
enum { SETTLE_SET_OR_HIGH, SETTLE_LOW_ROW, SETTLE_SETTLED };
enum { CAP_OTHER_SCREEN, CAP_FAR_ENOUGH, CAP_CAPPED };
enum { DRIFT_START, DRIFT_LATER, DRIFT_COUNTING, DRIFT_WAITING, DRIFT_NEXT };
enum { SLOTS_LOADED, SLOTS_BEFORE_PAGE4, SLOTS_COUNTING, SLOTS_FULL, SLOTS_STARTED };
enum { CHASE_START, CHASE_SLOW_ENOUGH, CHASE_SLOWED, CHASE_CAPPED, CHASE_SPED };
enum {
    GATES_LOADED, GATES_ACTIVATED, GATES_WAITING, GATES_PAIRS_FULL, GATES_X_BEFORE, GATES_X_PAST, GATES_X_RIGHT, GATES_TYPE_2C, GATES_Y_HIGH,
    GATES_Y_LOW, GATES_SLOTS_FULL, GATES_TYPE_55
};

/* MaybeDropItemByPlayerDistance with the tables of the overlay. */
static DropArm drop_item_with_tables(uint8_t slot, uint16_t thresholds, uint16_t items)
{
    DropArm r = { 0 };
    uint8_t raw;

    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = DROP_FEW_HITS;
        return r;
    }
    raw = RD8(OBJ->f31f8[slot]);
    r.bits = (uint8_t)((raw >> 2) & 0x07);
    r.bit1 = ((raw >> 1) & 0x01) != 0;
    if (r.bits == 0) {
        r.arm = DROP_NO_BITS;
        return r;
    }
    r.level = (uint8_t)(r.bits - 1);
    r.row = (uint8_t)(r.level << 4);
    WR8(ZP->temp00, r.row);
    jsr_push(0xA2E0);
    r.random = NextRandom();
    jsr_pop();
    r.threshold = rd8((uint16_t)(thresholds + r.level));
    if (r.random.value >= r.threshold) {
        r.arm = DROP_NOT_DRAWN;
        return r;
    }
    jsr_push(0xA2E8);
    r.sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    r.value = (uint8_t)(r.sum.value & 0x0F);
    r.joined = add_bytes(r.value, RD8(ZP->temp00), false);
    r.item = rd8((uint16_t)(items + r.joined.value));
    WR8(ZP->temp00, r.item);
    jsr_push(0xA2F6);
    r.found = l8_free_slot30();
    jsr_pop();
    if (!r.found.found) {
        r.arm = DROP_NO_SLOT;
        return r;
    }
    r.item2 = RD8(ZP->temp00);
    jsr_push(0xA2FD);
    r.placed = StartObjectAtPlace(r.item2, slot, r.found.slot);
    jsr_pop();
    r.arm = DROP_PLACED;
    return r;
}

/*
 * @routine 1019:2C5 MaybeDropItemByPlayerDistance1019
 * @run 70:A2C5
 * WHAT IT DOES: MaybeDropItemByPlayerDistance with this overlay's tables at $AC4C and $AC54.
 * HOW: drop_item_with_tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_drop_item_exit_shim, enemies_l8_st20.c). The drop arm ran in no script; the draft stops there.
 */
DropArm MaybeDropItemByPlayerDistance1019(uint8_t slot)
{
    return drop_item_with_tables(slot, 0xAC4C, 0xAC54);
}

void abi_MaybeDropItemByPlayerDistance1019(void)
{
    abi_ended(l8_drop_item_exit_shim(MaybeDropItemByPlayerDistance1019(cpu.x)));
}

/*
 * @routine 1018:4D7 PlaySound58A
 * @run 6F:BCD7
 * WHAT IT DOES: Plays sound $58.
 * HOW: A load and a tail jump into SoundCommand.
 * WAITS: no.
 * Entry: Y kept for the sound. Exit: as SoundCommand, after A $58 (l8_sound58_exit_shim; the body returns the call).
 */
CommandCall PlaySound58A(uint8_t slot, uint8_t y_entry)
{
    return SoundCommand(0x58, slot, y_entry);
}

static SoundEnd l8_sound58_exit_shim(CommandCall call)
{
    SoundEnd r = l8_sound_none(ended_kept());

    r.sounded = true;
    r.command = 0x58;
    r.before = ended_load(0x58);
    r.call = call;
    return r;
}

void abi_PlaySound58A(void)
{
    abi_sound_end(l8_sound58_exit_shim(PlaySound58A(cpu.x, cpu.y)));
}

/*
 * @routine 1178:67A PlaySound58B
 * @run 6F:BE7A
 * WHAT IT DOES: Plays sound $58 (a copy of PlaySound58A for this overlay).
 * HOW: A load and a tail jump into SoundCommand.
 * WAITS: no.
 * Entry: Y kept for the sound. Exit: as SoundCommand, after A $58 (l8_sound58_exit_shim).
 */
CommandCall PlaySound58B(uint8_t slot, uint8_t y_entry)
{
    return PlaySound58A(slot, y_entry);
}

void abi_PlaySound58B(void)
{
    abi_sound_end(l8_sound58_exit_shim(PlaySound58B(cpu.x, cpu.y)));
}

/*
 * @routine 101B:426 SettleWhenLowOnFirstRow
 * @run 70:B426
 * WHAT IT DOES: When OBJ_3670 and the Y high byte are both 0 and the Y low byte is below $D0, sets OBJ_3670 1 and clears the flash flags.
 * HOW: An ORA, a compare, an INC, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_settle_low_exit_shim). The body returns the ORA (first), the Y low byte (second) and the new OBJ_3670 (third).
 */
L8Arm SettleWhenLowOnFirstRow(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t flag, high;

    flag = RD8(OBJ->f3670[slot]);
    high = RD8(OBJ->y_hi[slot]);
    r.first = (uint8_t)(flag | high);
    if (r.first != 0) {
        r.arm = SETTLE_SET_OR_HIGH;
        return r;
    }
    r.second = RD8(OBJ->y_lo[slot]);
    if (r.second >= 0xD0) {
        r.arm = SETTLE_LOW_ROW;
        return r;
    }
    r.third = INC8(OBJ->f3670[slot]);
    WR8(OBJ->flash_flags[slot], 0);
    r.arm = SETTLE_SETTLED;
    return r;
}

static Ended l8_settle_low_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == SETTLE_SET_OR_HIGH) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0xD0));
    if (r.arm == SETTLE_LOW_ROW) {
        return e;
    }
    return then(e, ended_nz(r.third));
}

void abi_SettleWhenLowOnFirstRow(void)
{
    abi_ended(l8_settle_low_exit_shim(SettleWhenLowOnFirstRow(cpu.x)));
}

/*
 * @routine 101F:199 LoadRecordFrameFromKind
 * @run 71:D199
 * WHAT IT DOES: In state 0 loads the record and takes OBJ_3040 as its frame.
 * HOW: LoadObjectRecord, a copy.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_frame_from_kind_exit_shim). The body returns the state step (load) and OBJ_3040 (first).
 */
L8Arm LoadRecordFrameFromKind(uint8_t slot)
{
    L8Arm r = { 0 };

    r.load = l8_load_new(slot, 0xD1A1);
    if (r.load.state != 0) {
        return r;
    }
    r.first = RD8(OBJ->f3040[slot]);
    WR8(OBJ->frame[slot], r.first);
    return r;
}

static Ended l8_frame_from_kind_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.load.state != 0) {
        return e;
    }
    return then(e, ended_load(r.first));
}

void abi_LoadRecordFrameFromKind(void)
{
    abi_ended(l8_frame_from_kind_exit_shim(LoadRecordFrameFromKind(cpu.x)));
}

/*
 * @routine 161B:112 CapPlayerXOnScreen1
 * @run 70:B112
 * WHAT IT DOES: On the scroll's screen 1 keeps slot 0's OBJ_X_LO at $36 or more.
 * HOW: Two compares and a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_cap_x_screen1_exit_shim). The body returns the screen byte (first) and slot 0's X low byte (second). The arm past the first compare ran in no script; the draft stops there.
 */
L8Arm CapPlayerXOnScreen1(uint8_t slot)
{
    L8Arm r = { 0 };

    (void)slot;
    r.first = RD8(SCROLLE->x_hi_screen[0]);
    if (r.first != 1) {
        r.arm = CAP_OTHER_SCREEN;
        return r;
    }
    r.second = RD8(OBJ->x_lo[0]);
    if (r.second >= 0x36) {
        r.arm = CAP_FAR_ENOUGH;
        return r;
    }
    WR8(OBJ->x_lo[0], 0x36);
    r.arm = CAP_CAPPED;
    return r;
}

static Ended l8_cap_x_screen1_exit_shim(L8Arm r)
{
    Ended e = ended_compare(r.first, 1);

    if (r.arm == CAP_OTHER_SCREEN) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x36));
    if (r.arm == CAP_FAR_ENOUGH) {
        return e;
    }
    return then(e, ended_load(0x36));
}

void abi_CapPlayerXOnScreen1(void)
{
    abi_ended(l8_cap_x_screen1_exit_shim(CapPlayerXOnScreen1(cpu.x)));
}

/*
 * @routine 161E:617 LoadRecordThenDriftAndCount
 * @run 71:CE17
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 6, frame $45, OBJ_3510 $10 and the speeds' low bytes 1. In state 1 it moves, counts OBJ_34B8 down through $FF and then OBJ_3510; when that goes below 0 it goes to the next state.
 * HOW: LoadObjectRecord, ApplyVelocity, memory decrements, a compare, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_drift_count_exit_shim).
 */
DriftArm LoadRecordThenDriftAndCount(uint8_t slot, L8Layout lay)
{
    DriftArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(l8_at(lay, 0xCE1F));
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        WR8(OBJ->frame[slot], 0x45);
        WR8(OBJ->f3510[slot], 0x10);
        WR8(OBJ->vx_lo[slot], 1);
        WR8(OBJ->vy_lo[slot], 1);
        r.arm = DRIFT_START;
        return r;
    }
    if (r.state != 1) {
        r.arm = DRIFT_LATER;
        return r;
    }
    jsr_push(l8_at(lay, 0xCE3D));
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.timer = DEC8(OBJ->f34b8[slot]);
    r.again = RD8(OBJ->f34b8[slot]);
    if (r.again != 0xFF) {
        r.arm = DRIFT_COUNTING;
        return r;
    }
    r.wait = DEC8(OBJ->f3510[slot]);
    if ((r.wait & 0x80) == 0) {
        r.arm = DRIFT_WAITING;
        return r;
    }
    r.counted = INC8(OBJ->state[slot]);
    r.arm = DRIFT_NEXT;
    return r;
}

static Ended l8_drift_count_exit_shim(DriftArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == DRIFT_START) {
        return then(then(e, l8_record_ended(r.record)), ended_load(1));
    }
    e = then(e, ended_load((uint8_t)(r.state - 1)));
    if (r.arm == DRIFT_LATER) {
        return e;
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_nz(r.timer));
    e = then(e, ended_compare(r.again, 0xFF));
    if (r.arm == DRIFT_COUNTING) {
        return e;
    }
    e = then(e, ended_nz(r.wait));
    if (r.arm == DRIFT_WAITING) {
        return e;
    }
    return then(e, ended_nz(r.counted));
}

void abi_LoadRecordThenDriftAndCount(void)
{
    abi_ended(l8_drift_count_exit_shim(LoadRecordThenDriftAndCount(cpu.x, L8_LOADED)));
}

/*
 * @routine 18DD:260 LoadRecordAtX80
 * @run 71:C260
 * WHAT IT DOES: In state 0 loads the record and places the object at X $0080.
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_record_at_x80_exit_shim). The body returns the state step.
 */
L8Load LoadRecordAtX80(uint8_t slot)
{
    L8Load r = l8_load_new(slot, 0xC268);

    if (r.state != 0) {
        return r;
    }
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->x_hi[slot], 0);
    return r;
}

static Ended l8_record_at_x80_exit_shim(L8Load r)
{
    Ended e = l8_load_ended(r);

    if (r.state != 0) {
        return e;
    }
    return then(e, ended_load(0x80));
}

void abi_LoadRecordAtX80(void)
{
    abi_ended(l8_record_at_x80_exit_shim(LoadRecordAtX80(cpu.x)));
}

/*
 * @routine 101B:296 CountDownThenStartPieceInSlots18To1B
 * @run 70:B296
 * WHAT IT DOES: In state 0 loads the record. Later, once the player's screen position (OBJ_X_LO of slot 0 plus the scroll) is past page 4, it counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero it clears the first empty slot of $18 to $1B and starts an object of type $28 there at its place with OBJ_34B8 1 and OBJ_3510 $20.
 * HOW: LoadObjectRecord, a 16-bit add and subtract, a search, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_piece_slots_exit_shim). The exit when no slot is empty ran in no script.
 */
PieceSlotsArm CountDownThenStartPieceInSlots18To1B(uint8_t slot)
{
    PieceSlotsArm r = { 0 };
    uint8_t low, scroll, high, timer, y, kind, placed;
    Sum diff;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = SLOTS_LOADED;
        return r;
    }
    low = RD8(OBJ->x_lo[0]);
    scroll = RD8(SCROLL->x_lo[0]);
    r.sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, r.sum.value);
    high = RD8(OBJ->x_hi[0]);
    scroll = RD8(SCROLLE->x_hi_screen[0]);
    r.sum_page = add_bytes(high, scroll, r.sum.carry);
    r.page = r.sum_page.value;
    if (r.page < 4) {
        r.arm = SLOTS_BEFORE_PAGE4;
        return r;
    }
    high = RD8(OBJ->f34b8[slot]);
    timer = RD8(OBJ->f3510[slot]);
    r.joined = (uint8_t)(high | timer);
    if (r.joined != 0) {
        timer = RD8(OBJ->f3510[slot]);
        diff = sub_bytes(timer, 1, true);
        WR8(OBJ->f3510[slot], diff.value);
        high = RD8(OBJ->f34b8[slot]);
        r.diff = sub_bytes(high, 0, diff.carry);
        WR8(OBJ->f34b8[slot], r.diff.value);
        r.arm = SLOTS_COUNTING;
        return r;
    }
    y = 0x18;
    for (;;) {
        kind = RD8(OBJ->kind[y]);
        r.scan.y = y;
        r.scan.kind = kind;
        if (kind == 0) {
            break;
        }
        y++;
        r.scan.stepped = true;
        if (y >= 0x1C) {
            r.scan.y = y;
            r.arm = SLOTS_FULL;
            return r;
        }
    }
    jsr_push(0xB2DB);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB2E0);
    placed = StartObjectAtPlace(0x28, slot, y);
    jsr_pop();
    WR8(OBJ->f34b8[slot], 1);
    WR8(OBJ->f3510[slot], 0x20);
    (void)placed;
    r.arm = SLOTS_STARTED;
    return r;
}

static Ended l8_piece_slots_exit_shim(PieceSlotsArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == SLOTS_LOADED) {
        return then(e, l8_record_ended(r.record));
    }
    e = then(e, ended_of_sum(r.sum));
    e = then(then(e, ended_of_sum(r.sum_page)), ended_compare(r.page, 4));
    if (r.arm == SLOTS_BEFORE_PAGE4) {
        return e;
    }
    e = then(e, ended_load(r.joined));
    if (r.arm == SLOTS_COUNTING) {
        return then(e, ended_of_sum(r.diff));
    }
    e = l8_scan_ended(e, r.scan, 1, 0x1C, r.arm == SLOTS_FULL);
    if (r.arm == SLOTS_FULL) {
        return e;
    }
    return then(e, ended_load(0x20));
}

void abi_CountDownThenStartPieceInSlots18To1B(void)
{
    abi_ended(l8_piece_slots_exit_shim(CountDownThenStartPieceInSlots18To1B(cpu.x)));
}

/* TurnAngleToward's exit (turn_angle_exit_shim): equal gives the angle with Z set and C set, else the last add. */
static Ended ended_of_turn_angle(TurnAngleResult r)
{
    if (r.equal) {
        return ended_with_carry(ended_with_a(ended_nz(0), r.angle), true);
    }
    return ended_of_sum(r.last);
}

/*
 * @routine 101B:2EB ChaseWanderingPointWithSpeedRamp
 * @run 70:B2EB
 * WHAT IT DOES: In state 0 loads the record, sets bit 4 of the flash flags and places the object at X $0020, Y $0110. Later it steps the list at $B3E3, runs SettleWhenLowOnFirstRow, faces the player, and every $30 to $6F frames (OBJ_3618) picks one of eight offsets (OBJ_35C0, table at $B406) around the player as its target. It moves by the speed made from the angle OBJ_34B8 and the length OBJ_3510:OBJ_3568, turns the angle two steps toward the target and speeds up by 8 when the angle is within 3 of it (slowing by $10 when it is not and it is going faster than $50).
 * HOW: LoadObjectRecord, StepObjectFrames, SettleWhenLowOnFirstRow, SideOfPlayer, NextRandom, SetVelocityFromAngle, ApplyVelocity, AngleToPoint, TurnAngleToward, 16-bit adds in memory.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_chase_point_exit_shim).
 */
ChaseArm ChaseWanderingPointWithSpeedRamp(uint8_t slot, uint8_t y_entry)
{
    ChaseArm r = { 0 };
    uint8_t value, kind, scale_hi, scale_lo, y, high, low;
    Sum sum, diff;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB2F3);
        r.record = l8_record(slot);
        jsr_pop();
        value = RD8(OBJ->flash_flags[slot]);
        r.flash = (uint8_t)(value | 0x10);
        WR8(OBJ->flash_flags[slot], r.flash);
        WR8(OBJ->x_hi[slot], 0);
        WR8(OBJ->x_lo[slot], 0x20);
        WR8(OBJ->y_hi[slot], 1);
        WR8(OBJ->y_lo[slot], 0x10);
        r.arm = CHASE_START;
        return r;
    }
    r.step = l8_list_step(slot, 0xB3E3, 0xB319, false, y_entry);
    jsr_push(0xB31C);
    r.settle = SettleWhenLowOnFirstRow(slot);
    jsr_pop();
    jsr_push(0xB31F);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.facing);
    r.wait = RD8(OBJ->f3618[slot]);
    if (r.wait == 0) {
        jsr_push(0xB32C);
        r.random = NextRandom();
        jsr_pop();
        r.wait_sum = add_bytes((uint8_t)(r.random.value & 0x3F), 0x30, false);
        WR8(OBJ->f3618[slot], r.wait_sum.value);
        jsr_push(0xB337);
        r.random2 = NextRandom();
        jsr_pop();
        r.pick = r.random2.value & 0x07;
        WR8(OBJ->f35c0[slot], r.pick);
        r.picked = true;
    }
    r.counted = DEC8(OBJ->f3618[slot]);
    scale_lo = RD8(OBJ->f3568[slot]);
    WR8(ZP->temp00, scale_lo);
    scale_hi = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp01, scale_hi);
    r.angle = RD8(OBJ->f34b8[slot]);
    jsr_push(0xB34F);
    r.scale = SetVelocityFromAngle(r.angle, slot);
    jsr_pop();
    jsr_push(0xB352);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    kind = RD8(OBJ->f35c0[slot]);
    r.index = (uint8_t)(kind << 2);
    r.index_carry = (uint8_t)(kind << 1) >= 0x80;
    y = r.index;
    value = RD8(OBJ->x_lo[0]);
    sum = add_bytes(value, rd8((uint16_t)(0xB406 + y)), false);
    WR8(ZP->temp08, sum.value);
    value = RD8(OBJ->x_hi[0]);
    sum = add_bytes(value, rd8((uint16_t)(0xB407 + y)), sum.carry);
    WR8(ZP->temp09, sum.value);
    value = RD8(OBJ->y_lo[0]);
    sum = add_bytes(value, rd8((uint16_t)(0xB408 + y)), false);
    WR8(ZP->temp0a, sum.value);
    value = RD8(OBJ->y_hi[0]);
    r.target = add_bytes(value, rd8((uint16_t)(0xB409 + y)), sum.carry);
    WR8(ZP->temp0b, r.target.value);
    jsr_push(0xB37D);
    r.toward = AngleToPoint(slot);
    jsr_pop();
    WR8(ZP->temp02, r.toward.a);
    WR8(ZP->temp00, 2);
    r.angle2 = RD8(OBJ->f34b8[slot]);
    WR8(ZP->temp01, r.angle2);
    jsr_push(0xB38B);
    r.turn = TurnAngleToward();
    jsr_pop();
    r.value = (uint8_t)((r.turn.equal ? r.turn.angle : r.turn.last.value) & 0xFE);
    WR8(OBJ->f34b8[slot], r.value);
    r.diff = sub_bytes(r.value, RD8(ZP->temp02), true);
    r.sum = add_bytes(r.diff.value, 4, false);
    if (r.sum.value >= 8) {
        r.high = RD8(OBJ->f3510[slot]);
        if (r.high == 0) {
            r.low = RD8(OBJ->f3568[slot]);
            if (r.low < 0x50) {
                r.arm = CHASE_SLOW_ENOUGH;
                return r;
            }
        }
        low = RD8(OBJ->f3568[slot]);
        diff = sub_bytes(low, 0x10, true);
        WR8(OBJ->f3568[slot], diff.value);
        high = RD8(OBJ->f3510[slot]);
        r.speed = sub_bytes(high, 0, diff.carry);
        WR8(OBJ->f3510[slot], r.speed.value);
        r.arm = CHASE_SLOWED;
        return r;
    }
    r.high = RD8(OBJ->f3510[slot]);
    if (r.high >= 1) {
        r.low = RD8(OBJ->f3568[slot]);
        if (r.low >= 0x10) {
            WR8(OBJ->f3510[slot], 1);
            WR8(OBJ->f3568[slot], 0x10);
            r.arm = CHASE_CAPPED;
            return r;
        }
    }
    low = RD8(OBJ->f3568[slot]);
    sum = add_bytes(low, 8, false);
    WR8(OBJ->f3568[slot], sum.value);
    high = RD8(OBJ->f3510[slot]);
    r.speed = add_bytes(high, 0, sum.carry);
    WR8(OBJ->f3510[slot], r.speed.value);
    r.arm = CHASE_SPED;
    return r;
}

static Ended l8_chase_point_exit_shim(ChaseArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == CHASE_START) {
        e = then(e, l8_record_ended(r.record));
        e = then(e, ended_load(r.flash));
        return then(e, ended_load(0x10));
    }
    e = then(e, l8_step_ended(r.step));
    e = then(e, l8_settle_low_exit_shim(r.settle));
    e = then(e, ended_of_side(r.side));
    e = then(e, ended_load(r.facing));
    e = then(e, ended_load(r.wait));
    if (r.picked) {
        e = then(e, ended_of_random(r.random));
        e = then(e, ended_of_sum(r.wait_sum));
        e = then(e, ended_of_random(r.random2));
        e = then(e, ended_load(r.pick));
    }
    e = then(e, ended_nz(r.counted));
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.index), r.index_carry), r.index));
    e = then(e, ended_of_sum(r.target));
    e = then(e, l8_of_angle(r.toward));
    e = then(e, ended_load(r.angle2));
    e = then(e, ended_of_turn_angle(r.turn));
    e = then(e, ended_load(r.value));
    e = then(then(then(e, ended_of_sum(r.diff)), ended_of_sum(r.sum)), ended_compare(r.sum.value, 8));
    if (r.arm == CHASE_SLOW_ENOUGH || r.arm == CHASE_SLOWED) {
        e = then(e, ended_load(r.high));
        if (r.high == 0) {
            e = then(e, ended_compare(r.low, 0x50));
            if (r.arm == CHASE_SLOW_ENOUGH) {
                return e;
            }
        }
        return then(e, ended_of_sum(r.speed));
    }
    e = then(e, ended_compare(r.high, 1));
    if (r.high >= 1) {
        e = then(e, ended_compare(r.low, 0x10));
        if (r.arm == CHASE_CAPPED) {
            return then(e, ended_load(0x10));
        }
    }
    return then(e, ended_of_sum(r.speed));
}

void abi_ChaseWanderingPointWithSpeedRamp(void)
{
    abi_ended(l8_chase_point_exit_shim(ChaseWanderingPointWithSpeedRamp(cpu.x, cpu.y)));
}

/*
 * @routine 161B:126 SpawnerWithScrollGates
 * @run 70:B126
 * WHAT IT DOES: In state 0 loads the record. State 1 starts list $8D on channel 3 when OBJ_3040 is not 0 and goes on. Later it runs CapPlayerXOnScreen1 when OBJ_3040 is 0, waits OBJ_34B8 down, then with OBJ_3040 0 and the scroll in the right window starts an object of type $2C in a free slot of $1A to $1F numbered by OBJ_3510 (which counts up) and takes a random wait of $80 to $BF; with OBJ_3040 not 0 and the scroll's Y in range clears and starts an object of type $55 in the first free slot of $11 to $1F and waits $D0.
 * HOW: LoadObjectRecord, StartListChannelKind2, CapPlayerXOnScreen1, searches of the slot kinds, StartObjectAtPlace, NextRandom, ClearObjectInY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_scroll_gates_exit_shim). Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
GatesArm SpawnerWithScrollGates(uint8_t slot)
{
    GatesArm r = { 0 };
    uint8_t kind, y, placed;

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB12E);
        r.record = l8_record(slot);
        jsr_pop();
        r.arm = GATES_LOADED;
        return r;
    }
    if (r.state == 1) {
        r.kind = RD8(OBJ->f3040[slot]);
        if (r.kind != 0) {
            r.x_pulled = l8_channel2(slot, 0x8D, 3, 0xB13F);
        }
        r.counted = INC8(OBJ->state[slot]);
        r.arm = GATES_ACTIVATED;
        return r;
    }
    r.kind = RD8(OBJ->f3040[slot]);
    if (r.kind == 0) {
        jsr_push(0xB14C);
        r.cap = CapPlayerXOnScreen1(slot);
        jsr_pop();
    }
    r.timer = RD8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.counted = DEC8(OBJ->f34b8[slot]);
        r.arm = GATES_WAITING;
        return r;
    }
    for (y = 0x1A;;) {
        kind = RD8(OBJ->kind[y]);
        r.scan.y = y;
        r.scan.kind = kind;
        if (kind == 0) {
            break;
        }
        y++;
        r.scan.stepped = true;
        if (y >= 0x20) {
            r.scan.y = y;
            r.arm = GATES_PAIRS_FULL;
            return r;
        }
    }
    r.kind2 = RD8(OBJ->f3040[slot]);
    if (r.kind2 == 0) {
        r.screen = RD8(SCROLLE->x_hi_screen[0]);
        if (r.screen < 9) {
            if (r.screen == 1) {
                r.low = RD8(SCROLL->x_lo[0]);
                if (r.low < 0xA0) {
                    r.arm = GATES_X_BEFORE;
                    return r;
                }
            }
        } else {
            if (r.screen != 9) {
                r.arm = GATES_X_PAST;
                return r;
            }
            r.low = RD8(SCROLL->x_lo[0]);
            if (r.low >= 0x20) {
                r.arm = GATES_X_RIGHT;
                return r;
            }
        }
        jsr_push(0xB188);
        r.placed = StartObjectAtPlace(0x2C, slot, y);
        jsr_pop();
        r.number = RD8(OBJ->f3510[slot]);
        WR8(OBJ->f3040[y], r.number);
        r.counted = INC8(OBJ->f3510[slot]);
        jsr_push(0xB194);
        r.random = NextRandom();
        jsr_pop();
        r.sum = add_bytes((uint8_t)(r.random.value & 0x3F), 0x80, false);
        WR8(OBJ->f34b8[slot], r.sum.value);
        r.arm = GATES_TYPE_2C;
        return r;
    }
    r.screen = RD8(SCROLLE->y_hi_screen[0]);
    if (r.screen >= 2) {
        r.arm = GATES_Y_HIGH;
        return r;
    }
    r.low = RD8(SCROLL->y_lo[0]);
    if (r.low >= 0xD0) {
        r.arm = GATES_Y_LOW;
        return r;
    }
    for (y = 0x11;;) {
        kind = RD8(OBJ->kind[y]);
        r.scan2.y = y;
        r.scan2.kind = kind;
        if (kind == 0) {
            break;
        }
        y++;
        r.scan2.stepped = true;
        if (y >= 0x20) {
            r.scan2.y = y;
            r.arm = GATES_SLOTS_FULL;
            return r;
        }
    }
    jsr_push(0xB1BB);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB1C2);
    placed = StartObjectAtPlace(0x55, slot, y);
    jsr_pop();
    WR8(OBJ->f34b8[slot], 0xD0);
    r.placed = placed;
    r.arm = GATES_TYPE_55;
    return r;
}

static Ended l8_scroll_gates_exit_shim(GatesArm r)
{
    Ended e = ended_load(r.state);

    if (r.arm == GATES_LOADED) {
        return then(e, l8_record_ended(r.record));
    }
    e = then(e, ended_load((uint8_t)(r.state - 1)));
    if (r.arm == GATES_ACTIVATED) {
        e = then(e, ended_load(r.kind));
        if (r.kind != 0) {
            e = then(e, l8_channel2_ended(r.x_pulled));
        }
        return then(e, ended_nz(r.counted));
    }
    e = then(e, ended_load(r.kind));
    if (r.kind == 0) {
        e = then(e, l8_cap_x_screen1_exit_shim(r.cap));
    }
    e = then(e, ended_load(r.timer));
    if (r.arm == GATES_WAITING) {
        return then(e, ended_nz(r.counted));
    }
    e = l8_scan_ended(e, r.scan, 1, 0x20, r.arm == GATES_PAIRS_FULL);
    if (r.arm == GATES_PAIRS_FULL) {
        return e;
    }
    e = then(e, ended_load(r.kind2));
    if (r.kind2 == 0) {
        e = then(e, ended_compare(r.screen, 9));
        if (r.screen < 9) {
            e = then(e, ended_compare(r.screen, 1));
            if (r.screen == 1) {
                e = then(e, ended_compare(r.low, 0xA0));
                if (r.arm == GATES_X_BEFORE) {
                    return e;
                }
            }
        } else {
            if (r.arm == GATES_X_PAST) {
                return e;
            }
            e = then(e, ended_compare(r.low, 0x20));
            if (r.arm == GATES_X_RIGHT) {
                return e;
            }
        }
        e = then(e, ended_load(r.placed));
        e = then(e, ended_load(r.number));
        e = then(e, ended_nz(r.counted));
        e = then(e, ended_of_random(r.random));
        return then(e, ended_of_sum(r.sum));
    }
    e = then(e, ended_compare(r.screen, 2));
    if (r.arm == GATES_Y_HIGH) {
        return e;
    }
    e = then(e, ended_compare(r.low, 0xD0));
    if (r.arm == GATES_Y_LOW) {
        return e;
    }
    e = l8_scan_ended(e, r.scan2, 1, 0x20, r.arm == GATES_SLOTS_FULL);
    if (r.arm == GATES_SLOTS_FULL) {
        return e;
    }
    return then(then(e, ended_load(r.placed)), ended_load(0xD0));
}

void abi_SpawnerWithScrollGates(void)
{
    abi_ended(l8_scroll_gates_exit_shim(SpawnerWithScrollGates(cpu.x)));
}
