/*
 * TEMPORARY: the stage object routines of lot L8 for the overlays at sectors 1018 to 18DD (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/* MaybeDropItemByPlayerDistance with the tables of the overlay. */
static Ended drop_item_with_tables(uint8_t slot, uint16_t thresholds, uint16_t items)
{
    uint8_t hits, raw, bits, level, row, threshold, item, found_slot, last, placed, value;
    bool found;
    RandomResult random;
    Sum sum, joined;
    Ended e;

    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hits, 5);
    if (hits < 5) {
        return e;
    }
    raw = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)((raw >> 2) & 0x07);
    e = then(e, ended_with_carry(ended_load(bits), ((raw >> 1) & 0x01) != 0));
    if (bits == 0) {
        return e;
    }
    level = (uint8_t)(bits - 1);
    e = then(e, ended_with_y(ended_load(level), level));
    row = (uint8_t)(level << 4);
    e = then(e, ended_with_carry(ended_load(row), false));
    WR8(ZP->temp00, row);
    jsr_push(0xA2E0);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    threshold = rd8((uint16_t)(thresholds + level));
    e = then(e, ended_compare(random.value, threshold));
    if (random.value >= threshold) {
        return e;
    }
    jsr_push(0xA2E8);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    e = then(e, ended_of_sum(sum));
    value = (uint8_t)(sum.value & 0x0F);
    e = then(e, ended_load(value));
    joined = add_bytes(value, RD8(ZP->temp00), false);
    e = then(e, ended_of_sum(joined));
    level = joined.value;
    e = then(e, ended_with_y(ended_kept(), level));
    item = rd8((uint16_t)(items + level));
    WR8(ZP->temp00, item);
    e = then(e, ended_load(item));
    jsr_push(0xA2F6);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    item = RD8(ZP->temp00);
    jsr_push(0xA2FD);
    placed = StartObjectAtPlace(item, slot, found_slot);
    jsr_pop();
    return then(then(e, ended_load(item)), ended_load(placed));
}


/*
 * @routine 1019:2C5 MaybeDropItemByPlayerDistance1019
 * @run 70:A2C5
 * WHAT IT DOES: MaybeDropItemByPlayerDistance with this overlay's tables at $AC4C and $AC54.
 * HOW: drop_item_with_tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
 */
Ended MaybeDropItemByPlayerDistance1019(uint8_t slot)
{
    return drop_item_with_tables(slot, 0xAC4C, 0xAC54);
}

void abi_MaybeDropItemByPlayerDistance1019(void)
{
    abi_ended(MaybeDropItemByPlayerDistance1019(cpu.x));
}

/*
 * @routine 1018:4D7 PlaySound58A
 * @run 6F:BCD7
 * WHAT IT DOES: Plays sound $58.
 * HOW: A load and a tail jump into SoundCommand.
 * WAITS: no.
 * Entry: Y kept for the sound. Exit: as SoundCommand.
 */
SoundEnd PlaySound58A(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = l8_sound_none(ended_kept());

    r.sounded = true;
    r.command = 0x58;
    r.before = ended_load(0x58);
    r.call = SoundCommand(0x58, slot, y_entry);
    return r;
}

void abi_PlaySound58A(void)
{
    abi_sound_end(PlaySound58A(cpu.x, cpu.y));
}

/*
 * @routine 1178:67A PlaySound58B
 * @run 6F:BE7A
 * WHAT IT DOES: Plays sound $58 (a copy of PlaySound58A for this overlay).
 * HOW: A load and a tail jump into SoundCommand.
 * WAITS: no.
 * Entry: Y kept for the sound. Exit: as SoundCommand.
 */
SoundEnd PlaySound58B(uint8_t slot, uint8_t y_entry)
{
    return PlaySound58A(slot, y_entry);
}

void abi_PlaySound58B(void)
{
    abi_sound_end(PlaySound58B(cpu.x, cpu.y));
}

/*
 * @routine 101B:426 SettleWhenLowOnFirstRow
 * @run 70:B426
 * WHAT IT DOES: When OBJ_3670 and the Y high byte are both 0 and the Y low byte is below $D0, sets OBJ_3670 1 and clears the flash flags.
 * HOW: An ORA, a compare, an INC, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended SettleWhenLowOnFirstRow(uint8_t slot)
{
    uint8_t flag, high, low;
    Ended e;

    flag = RD8(OBJ->f3670[slot]);
    high = RD8(OBJ->y_hi[slot]);
    e = ended_load((uint8_t)(flag | high));
    if ((flag | high) != 0) {
        return e;
    }
    low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(low, 0xD0));
    if (low >= 0xD0) {
        return e;
    }
    e = then(e, ended_nz(INC8(OBJ->f3670[slot])));
    WR8(OBJ->flash_flags[slot], 0);
    return e;
}

void abi_SettleWhenLowOnFirstRow(void)
{
    abi_ended(SettleWhenLowOnFirstRow(cpu.x));
}

/*
 * @routine 101F:199 LoadRecordFrameFromKind
 * @run 71:D199
 * WHAT IT DOES: In state 0 loads the record and takes OBJ_3040 as its frame.
 * HOW: LoadObjectRecord, a copy.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordFrameFromKind(uint8_t slot)
{
    uint8_t first, kind;
    Ended e = l8_load_if_new(slot, 0xD1A1, &first);

    if (first != 0) {
        return e;
    }
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->frame[slot], kind);
    return then(e, ended_load(kind));
}

void abi_LoadRecordFrameFromKind(void)
{
    abi_ended(LoadRecordFrameFromKind(cpu.x));
}

/*
 * @routine 161B:112 CapPlayerXOnScreen1
 * @run 70:B112
 * WHAT IT DOES: On the scroll's screen 1 keeps slot 0's OBJ_X_LO at $36 or more.
 * HOW: Two compares and a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The arm past the first compare ran in no script; the draft stops there.
 */
Ended CapPlayerXOnScreen1(uint8_t slot)
{
    uint8_t screen, x;
    Ended e;

    (void)slot;
    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 1);
    if (screen != 1) {
        return e;
    }
    x = RD8(OBJ->x_lo[0]);
    e = then(e, ended_compare(x, 0x36));
    if (x >= 0x36) {
        return e;
    }
    WR8(OBJ->x_lo[0], 0x36);
    return then(e, ended_load(0x36));
}

void abi_CapPlayerXOnScreen1(void)
{
    abi_ended(CapPlayerXOnScreen1(cpu.x));
}

/*
 * @routine 161E:617 LoadRecordThenDriftAndCount
 * @run 71:CE17
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 6, frame $45, OBJ_3510 $10 and the speeds' low bytes 1. In state 1 it moves, counts OBJ_34B8 down through $FF and then OBJ_3510; when that goes below 0 it goes to the next state.
 * HOW: LoadObjectRecord, ApplyVelocity, memory decrements, a compare, an INC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordThenDriftAndCount(uint8_t slot)
{
    uint8_t state, last, nz, timer, again, wait;
    bool carry;
    AxisMove move;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xCE1F);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        WR8(OBJ->sprite_set[slot], 6);
        WR8(OBJ->frame[slot], 0x45);
        WR8(OBJ->f3510[slot], 0x10);
        WR8(OBJ->vx_lo[slot], 1);
        WR8(OBJ->vy_lo[slot], 1);
        return then(e, ended_load(1));
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state != 1) {
        return e;
    }
    jsr_push(0xCE3D);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    again = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_compare(again, 0xFF));
    if (again != 0xFF) {
        return e;
    }
    wait = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(wait));
    if ((wait & 0x80) == 0) {
        return e;
    }
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_LoadRecordThenDriftAndCount(void)
{
    abi_ended(LoadRecordThenDriftAndCount(cpu.x));
}

/*
 * @routine 18DD:260 LoadRecordAtX80
 * @run 71:C260
 * WHAT IT DOES: In state 0 loads the record and places the object at X $0080.
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordAtX80(uint8_t slot)
{
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xC268, &first);

    if (first != 0) {
        return e;
    }
    WR8(OBJ->x_lo[slot], 0x80);
    WR8(OBJ->x_hi[slot], 0);
    return then(e, ended_load(0x80));
}

void abi_LoadRecordAtX80(void)
{
    abi_ended(LoadRecordAtX80(cpu.x));
}

/*
 * @routine 101B:296 CountDownThenStartPieceInSlots18To1B
 * @run 70:B296
 * WHAT IT DOES: In state 0 loads the record. Later, once the player's screen position (OBJ_X_LO of slot 0 plus the scroll) is past page 4, it counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero it clears the first empty slot of $18 to $1B and starts an object of type $28 there at its place with OBJ_34B8 1 and OBJ_3510 $20.
 * HOW: LoadObjectRecord, a 16-bit add and subtract, a search, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The exit when no slot is empty ran in no script.
 */
Ended CountDownThenStartPieceInSlots18To1B(uint8_t slot)
{
    uint8_t state, last, nz, low, scroll, page, high, timer, y, kind, placed;
    bool carry;
    Sum sum, diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(e, ended_of_record(last, nz, carry));
    }
    low = RD8(OBJ->x_lo[0]);
    scroll = RD8(SCROLL->x_lo[0]);
    sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, sum.value);
    e = then(e, ended_of_sum(sum));
    high = RD8(OBJ->x_hi[0]);
    scroll = RD8(SCROLLE->x_hi_screen[0]);
    sum = add_bytes(high, scroll, sum.carry);
    page = sum.value;
    e = then(then(e, ended_of_sum(sum)), ended_compare(page, 4));
    if (page < 4) {
        return e;
    }
    high = RD8(OBJ->f34b8[slot]);
    timer = RD8(OBJ->f3510[slot]);
    e = then(e, ended_load((uint8_t)(high | timer)));
    if ((high | timer) != 0) {
        timer = RD8(OBJ->f3510[slot]);
        diff = sub_bytes(timer, 1, true);
        WR8(OBJ->f3510[slot], diff.value);
        high = RD8(OBJ->f34b8[slot]);
        diff = sub_bytes(high, 0, diff.carry);
        WR8(OBJ->f34b8[slot], diff.value);
        return then(e, ended_of_sum(diff));
    }
    y = 0x18;
    for (;;) {
        kind = RD8(OBJ->kind[y]);
        e = then(e, ended_with_y(ended_load(kind), y));
        if (kind == 0) {
            break;
        }
        y++;
        e = then(e, ended_with_y(l8_compare_y(y, 0x1C), y));
        if (y >= 0x1C) {
            return e;
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
    return then(e, ended_load(0x20));
}

void abi_CountDownThenStartPieceInSlots18To1B(void)
{
    abi_ended(CountDownThenStartPieceInSlots18To1B(cpu.x));
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
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended ChaseWanderingPointWithSpeedRamp(uint8_t slot, uint8_t y_entry)
{
    uint8_t state, last, nz, y = y_entry, value, facing, wait, pick, angle, kind, scale_hi, scale_lo, index, near, high, low;
    bool carry;
    FrameStep step;
    SideResult side;
    RandomResult random;
    ScaleResult scale;
    AxisMove move;
    AngleExit toward;
    TurnAngleResult turn;
    Sum sum, diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB2F3);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        value = RD8(OBJ->flash_flags[slot]);
        value = (uint8_t)(value | 0x10);
        WR8(OBJ->flash_flags[slot], value);
        e = then(e, ended_load(value));
        WR8(OBJ->x_hi[slot], 0);
        WR8(OBJ->x_lo[slot], 0x20);
        WR8(OBJ->y_hi[slot], 1);
        WR8(OBJ->y_lo[slot], 0x10);
        return then(e, ended_load(0x10));
    }
    e = then(e, l8_step_list(slot, 0xB3E3, 0xB319, false, &y, &step));
    jsr_push(0xB31C);
    e = then(e, SettleWhenLowOnFirstRow(slot));
    jsr_pop();
    jsr_push(0xB31F);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    facing = side.sides & 0x01;
    e = then(e, ended_load(facing));
    WR8(OBJ->facing_left[slot], facing);
    wait = RD8(OBJ->f3618[slot]);
    e = then(e, ended_load(wait));
    if (wait == 0) {
        jsr_push(0xB32C);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        sum = add_bytes((uint8_t)(random.value & 0x3F), 0x30, false);
        WR8(OBJ->f3618[slot], sum.value);
        e = then(e, ended_of_sum(sum));
        jsr_push(0xB337);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        pick = random.value & 0x07;
        WR8(OBJ->f35c0[slot], pick);
        e = then(e, ended_load(pick));
    }
    e = then(e, ended_nz(DEC8(OBJ->f3618[slot])));
    scale_lo = RD8(OBJ->f3568[slot]);
    WR8(ZP->temp00, scale_lo);
    scale_hi = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp01, scale_hi);
    angle = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(angle));
    jsr_push(0xB34F);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    jsr_push(0xB352);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    kind = RD8(OBJ->f35c0[slot]);
    index = (uint8_t)(kind << 2);
    y = index;
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), (uint8_t)(kind << 1) >= 0x80), y));
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
    sum = add_bytes(value, rd8((uint16_t)(0xB409 + y)), sum.carry);
    WR8(ZP->temp0b, sum.value);
    e = then(e, ended_of_sum(sum));
    jsr_push(0xB37D);
    toward = AngleToPoint(slot);
    jsr_pop();
    e = then(e, l8_of_angle(toward));
    WR8(ZP->temp02, toward.a);
    WR8(ZP->temp00, 2);
    angle = RD8(OBJ->f34b8[slot]);
    WR8(ZP->temp01, angle);
    e = then(e, ended_load(angle));
    jsr_push(0xB38B);
    turn = TurnAngleToward();
    jsr_pop();
    e = then(e, ended_of_turn_angle(turn));
    value = (uint8_t)((turn.equal ? turn.angle : turn.last.value) & 0xFE);
    WR8(OBJ->f34b8[slot], value);
    e = then(e, ended_load(value));
    diff = sub_bytes(value, RD8(ZP->temp02), true);
    sum = add_bytes(diff.value, 4, false);
    e = then(then(then(e, ended_of_sum(diff)), ended_of_sum(sum)), ended_compare(sum.value, 8));
    near = (uint8_t)(sum.value < 8);
    if (!near) {
        high = RD8(OBJ->f3510[slot]);
        e = then(e, ended_load(high));
        if (high == 0) {
            low = RD8(OBJ->f3568[slot]);
            e = then(e, ended_compare(low, 0x50));
            if (low < 0x50) {
                return e;
            }
        }
        low = RD8(OBJ->f3568[slot]);
        diff = sub_bytes(low, 0x10, true);
        WR8(OBJ->f3568[slot], diff.value);
        high = RD8(OBJ->f3510[slot]);
        diff = sub_bytes(high, 0, diff.carry);
        WR8(OBJ->f3510[slot], diff.value);
        return then(e, ended_of_sum(diff));
    }
    high = RD8(OBJ->f3510[slot]);
    e = then(e, ended_compare(high, 1));
    if (high >= 1) {
        low = RD8(OBJ->f3568[slot]);
        e = then(e, ended_compare(low, 0x10));
        if (low >= 0x10) {
            WR8(OBJ->f3510[slot], 1);
            WR8(OBJ->f3568[slot], 0x10);
            return then(e, ended_load(0x10));
        }
    }
    low = RD8(OBJ->f3568[slot]);
    sum = add_bytes(low, 8, false);
    WR8(OBJ->f3568[slot], sum.value);
    high = RD8(OBJ->f3510[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->f3510[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_ChaseWanderingPointWithSpeedRamp(void)
{
    abi_ended(ChaseWanderingPointWithSpeedRamp(cpu.x, cpu.y));
}

/*
 * @routine 161B:126 SpawnerWithScrollGates
 * @run 70:B126
 * WHAT IT DOES: In state 0 loads the record. State 1 starts list $8D on channel 3 when OBJ_3040 is not 0 and goes on. Later it runs CapPlayerXOnScreen1 when OBJ_3040 is 0, waits OBJ_34B8 down, then with OBJ_3040 0 and the scroll in the right window starts an object of type $2C in a free slot of $1A to $1F numbered by OBJ_3510 (which counts up) and takes a random wait of $80 to $BF; with OBJ_3040 not 0 and the scroll's Y in range clears and starts an object of type $55 in the first free slot of $11 to $1F and waits $D0.
 * HOW: LoadObjectRecord, StartListChannelKind2, CapPlayerXOnScreen1, searches of the slot kinds, StartObjectAtPlace, NextRandom, ClearObjectInY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
 */
Ended SpawnerWithScrollGates(uint8_t slot)
{
    uint8_t state, last, nz, kind, y, timer, screen, low, placed, number, value;
    bool carry;
    RandomResult random;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB12E);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        return then(e, ended_of_record(last, nz, carry));
    }
    e = then(e, ended_load((uint8_t)(state - 1)));
    if (state == 1) {
        kind = RD8(OBJ->f3040[slot]);
        e = then(e, ended_load(kind));
        if (kind != 0) {
            e = then(e, l8_list_channel2(slot, 0x8D, 3, 0xB13F));
        }
        return then(e, ended_nz(INC8(OBJ->state[slot])));
    }
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_load(kind));
    if (kind == 0) {
        jsr_push(0xB14C);
        e = then(e, CapPlayerXOnScreen1(slot));
        jsr_pop();
    }
    timer = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(timer));
    if (timer != 0) {
        return then(e, ended_nz(DEC8(OBJ->f34b8[slot])));
    }
    for (y = 0x1A;;) {
        kind = RD8(OBJ->kind[y]);
        e = then(e, ended_with_y(ended_load(kind), y));
        if (kind == 0) {
            break;
        }
        y++;
        e = then(e, ended_with_y(l8_compare_y(y, 0x20), y));
        if (y >= 0x20) {
            return e;
        }
    }
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_load(kind));
    if (kind == 0) {
        screen = RD8(SCROLLE->x_hi_screen[0]);
        e = then(e, ended_compare(screen, 9));
        if (screen < 9) {
            e = then(e, ended_compare(screen, 1));
            if (screen == 1) {
                low = RD8(SCROLL->x_lo[0]);
                e = then(e, ended_compare(low, 0xA0));
                if (low < 0xA0) {
                    return e;
                }
            }
        } else {
            if (screen != 9) {
                return e;
            }
            low = RD8(SCROLL->x_lo[0]);
            e = then(e, ended_compare(low, 0x20));
            if (low >= 0x20) {
                return e;
            }
        }
        jsr_push(0xB188);
        placed = StartObjectAtPlace(0x2C, slot, y);
        jsr_pop();
        e = then(e, ended_load(placed));
        number = RD8(OBJ->f3510[slot]);
        WR8(OBJ->f3040[y], number);
        e = then(e, ended_load(number));
        e = then(e, ended_nz(INC8(OBJ->f3510[slot])));
        jsr_push(0xB194);
        random = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(random));
        sum = add_bytes((uint8_t)(random.value & 0x3F), 0x80, false);
        WR8(OBJ->f34b8[slot], sum.value);
        return then(e, ended_of_sum(sum));
    }
    screen = RD8(SCROLLE->y_hi_screen[0]);
    e = then(e, ended_compare(screen, 2));
    if (screen >= 2) {
        return e;
    }
    low = RD8(SCROLL->y_lo[0]);
    e = then(e, ended_compare(low, 0xD0));
    if (low >= 0xD0) {
        return e;
    }
    for (y = 0x11;;) {
        kind = RD8(OBJ->kind[y]);
        e = then(e, ended_with_y(ended_load(kind), y));
        if (kind == 0) {
            break;
        }
        y++;
        e = then(e, ended_with_y(l8_compare_y(y, 0x20), y));
        if (y >= 0x20) {
            return e;
        }
    }
    jsr_push(0xB1BB);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB1C2);
    placed = StartObjectAtPlace(0x55, slot, y);
    jsr_pop();
    WR8(OBJ->f34b8[slot], 0xD0);
    (void)value;
    return then(then(e, ended_load(placed)), ended_load(0xD0));
}

void abi_SpawnerWithScrollGates(void)
{
    abi_ended(SpawnerWithScrollGates(cpu.x));
}
