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
