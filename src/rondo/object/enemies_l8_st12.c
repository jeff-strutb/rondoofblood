/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025B (enemy kinds not yet identified), by routine. Bodies return
 * the arm they ended on and the values behind it (L8Arm or a struct of this file's section of stage_enemies_l8.h); each routine's exit
 * shim builds how its last instructions left A, Y and the flags (Ended, enemy_objects.h), and the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The arms the routines end on. */
enum { SCROLL_IDLE, SCROLL_NOT_STARTED, SCROLL_STARTED, SCROLL_STEPPED };
enum { S80_ZERO, S80_COUNTING, S80_STARTED };
enum { PLACE_OLD, PLACE_KEPT, PLACE_PLACED };
enum { AT_PLAYER_OLD, AT_PLAYER_PLACED };
enum { SINK_OLD, SINK_NEW };
enum { PHASE_NEW, PHASE_ADVANCE, PHASE_FAST, PHASE_SLOW, PHASE_TWO, PHASE_X_NEGATIVE, PHASE_X_ON, PHASE_GONE };
enum { DRIFT_NEW, DRIFT_WAIT, DRIFT_OTHER, DRIFT_SHOW, DRIFT_Y_NEGATIVE, DRIFT_Y_HIGH_ZERO, DRIFT_JITTER };
enum { SWING_EARLY, SWING_MOVED };
enum { RSCRIPT_OFF, RSCRIPT_NOT_YET, RSCRIPT_NO_LUCK, RSCRIPT_STARTED };
enum { PUSH_OFF, PUSH_ON };
enum { HIT_IDLE, HIT_COUNTED, HIT_NO_SLOT, HIT_SPAWNED };
enum { S0B_CLEAR, S0B_LATE, S0B_ENTERED };
enum { SCRIPTS_COUNTING, SCRIPTS_STARTED };
enum { S77_ZERO, S77_COUNTING, S77_SOUNDED };
enum { MARKS_IDLE, MARKS_NONE, MARKS_STARTED };
enum { SPIN_OUT_GOING, SPIN_OUT_ENDED };
enum { CIRCLE_TURNING, CIRCLE_WRAPPED };
enum { BURST_ENDED, BURST_HELD, BURST_OTHER_FRAME, BURST_SOUNDED };
enum { HOME_FAR, HOME_X_OFF, HOME_Y_OFF, HOME_ARRIVED };

/*
 * @routine 025B:0F0 StepVerticalScrollTable
 * @run 70:B0F0
 * WHAT IT DOES: While OBJ_3568 is not 0, steps through the table at $B129 and $B133: OBJ_3510 takes the entry and the scroll's Y (ScrollYLo+1, ScrollYHi+1) falls by it; OBJ_3568 counts up and wraps to 0 at 9. When it is 0 and the scroll byte ScrollXLo+5 is 0, a random number with its top bit set starts the table (OBJ_3568 1).
 * HOW: A table read by OBJ_3568 as the index, a 16-bit subtract, and NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction that ran (l8_vertical_scroll_table_exit_shim).
 */
L8Arm StepVerticalScrollTable(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t index, value, low, high;
    Sum diff;

    index = RD8(OBJ->f3568[slot]);
    r.first = index;
    if (index == 0) {
        r.second = RD8(SCROLL->x_lo[5]);
        if (r.second != 0) {
            r.arm = SCROLL_IDLE;
            return r;
        }
        jsr_push(0xB122);
        r.random = NextRandom();
        jsr_pop();
        if ((r.random.value & 0x80) == 0) {
            r.arm = SCROLL_NOT_STARTED;
            return r;
        }
        r.third = INC8(OBJ->f3568[slot]);
        r.arm = SCROLL_STARTED;
        return r;
    }
    value = rd8((uint16_t)(0xB129 + index));
    WR8(OBJ->f3510[slot], value);
    r.second = value;
    low = RD8(SCROLL->y_lo[1]);
    value = rd8((uint16_t)(0xB129 + index));
    diff = sub_bytes(low, value, true);
    WR8(SCROLL->y_lo[1], diff.value);
    high = RD8(SCROLL->y_hi[1]);
    value = rd8((uint16_t)(0xB133 + index));
    diff = sub_bytes(high, value, diff.carry);
    WR8(SCROLL->y_hi[1], diff.value);
    r.sum = diff;
    r.third = INC8(OBJ->f3568[slot]);
    if (index == 9) {
        WR8(OBJ->f3568[slot], 0);
    }
    r.arm = SCROLL_STEPPED;
    return r;
}

/* What StepVerticalScrollTable leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_vertical_scroll_table_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    switch (r.arm) {
    case SCROLL_IDLE:
        return then(e, ended_load(r.second));
    case SCROLL_NOT_STARTED:
    case SCROLL_STARTED:
        e = then(e, ended_load(r.second));
        e = then(e, ended_of_random(r.random));
        e = then(e, ended_with_carry(ended_load((uint8_t)(r.random.value << 1)), (r.random.value & 0x80) != 0));
        if (r.arm == SCROLL_STARTED) {
            e = then(e, ended_nz(r.third));
        }
        return e;
    default:
        e = ended_with_y(e, r.first);
        e = then(e, ended_load(r.second));
        e = then(e, ended_with_overflow(ended_with_carry(ended_load(r.sum.value), r.sum.carry), r.sum.overflow));
        e = then(e, ended_nz(r.third));
        return then(e, l8_compare_y(r.first, 9));
    }
}

void abi_StepVerticalScrollTable(void)
{
    abi_ended(l8_vertical_scroll_table_exit_shim(StepVerticalScrollTable(cpu.x)));
}

/*
 * @routine 025B:068 CountdownThenStartScript80
 * @run 70:B068
 * WHAT IT DOES: Sets fixed fields (OBJ_3510 slot 0 $10, the scroll bytes ScrollXLo+1 0 and ScrollXHi+1 1, a byte at $3BF0 to 2); counts OBJ_34B8 down; at $3F starts script $80 as kind 2 in slot 5, at 0 sets the sub-step 3 and zField8C $83.
 * HOW: Stores, a memory decrement, a compare, StartScriptSlotKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_script80_exit_shim). The byte at $3BF0 keeps its address form until it has a name.
 */
L8Arm CountdownThenStartScript80(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t timer;

    WR8(OBJ->f3510[0], 0x10);
    wr8(0x3BF0, 0x02);
    WR8(OBJ->f3568[0], 0);
    WR8(OBJ->frame[0], 0);
    WR8(SCROLL->x_lo[1], 0);
    WR8(SCROLL->x_hi[1], 1);
    timer = DEC8(OBJ->f34b8[slot]);
    if (timer == 0) {
        WR8(ZP->sub_step, 0x03);
        WR8(ZP->field8c, 0x83);
        r.arm = S80_ZERO;
        return r;
    }
    r.first = RD8(OBJ->f34b8[slot]);
    if (r.first != 0x3F) {
        r.arm = S80_COUNTING;
        return r;
    }
    push8(slot);
    jsr_push(0xB094);
    StartScriptSlotKind2(0x80, 5);
    jsr_pop();
    r.second = pull8();
    r.arm = S80_STARTED;
    return r;
}

/* What CountdownThenStartScript80 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_script80_exit_shim(L8Arm r)
{
    switch (r.arm) {
    case S80_ZERO:
        return ended_load(0x83);
    case S80_COUNTING:
        return ended_compare(r.first, 0x3F);
    default:
        return then(ended_compare(r.first, 0x3F), ended_with_y(ended_with_a(ended_nz(r.second), 0x80), 0x80));
    }
}

void abi_CountdownThenStartScript80(void)
{
    abi_ended(l8_script80_exit_shim(CountdownThenStartScript80(cpu.x)));
}

/*
 * @routine 025B:148 LoadRecordPlaceByKind
 * @run 70:B148
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 6 and the frame from a table by OBJ_START_ARG; when the object's flags are 0 it is placed at X $0120 with a Y from a second table, and its flags take the low nibble 6.
 * HOW: LoadObjectRecord, two table reads indexed by OBJ_START_ARG, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_place_by_kind_exit_shim).
 */
L8Arm LoadRecordPlaceByKind(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t kind, value, flags;

    r.load = l8_load_new(slot, 0xB150);
    if (r.load.state != 0) {
        r.arm = PLACE_OLD;
        return r;
    }
    WR8(OBJ->sprite_set[slot], 6);
    kind = RD8(OBJ->f3040[slot]);
    r.first = kind;
    value = rd8((uint16_t)(0xB17E + kind));
    WR8(OBJ->frame[slot], value);
    r.second = RD8(OBJ->scroll_band[slot]);
    if (r.second != 0) {
        r.arm = PLACE_KEPT;
        return r;
    }
    WR8(OBJ->x_lo[slot], 0x20);
    WR8(OBJ->x_hi[slot], 0x01);
    value = rd8((uint16_t)(0xB180 + kind));
    WR8(OBJ->y_lo[slot], value);
    flags = RD8(OBJ->scroll_band[slot]);
    r.third = (uint8_t)((flags & 0xF0) | 0x06);
    WR8(OBJ->scroll_band[slot], r.third);
    r.arm = PLACE_PLACED;
    return r;
}

/* What LoadRecordPlaceByKind leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_place_by_kind_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.arm == PLACE_OLD) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(6), r.first));
    e = then(e, ended_nz(r.first));
    e = then(e, ended_load(r.second));
    if (r.arm == PLACE_KEPT) {
        return e;
    }
    return then(e, ended_load(r.third));
}

void abi_LoadRecordPlaceByKind(void)
{
    abi_ended(l8_place_by_kind_exit_shim(LoadRecordPlaceByKind(cpu.x)));
}

/*
 * @routine 025B:182 LoadRecordAtPlayerPlace
 * @run 70:B182
 * WHAT IT DOES: In state 0 loads the record and takes slot 0's OBJ_X_LO and OBJ_Y_LO; the flags take the low nibble 1.
 * HOW: LoadObjectRecord, two copies, an AND and an ORA.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_at_player_place_exit_shim).
 */
L8Arm LoadRecordAtPlayerPlace(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t value, flags;

    r.load = l8_load_new(slot, 0xB18A);
    if (r.load.state != 0) {
        r.arm = AT_PLAYER_OLD;
        return r;
    }
    value = RD8(OBJ->x_lo[0]);
    WR8(OBJ->x_lo[slot], value);
    value = RD8(OBJ->y_lo[0]);
    WR8(OBJ->y_lo[slot], value);
    flags = RD8(OBJ->scroll_band[slot]);
    r.first = (uint8_t)((flags & 0xF0) | 0x01);
    WR8(OBJ->scroll_band[slot], r.first);
    r.arm = AT_PLAYER_PLACED;
    return r;
}

/* What LoadRecordAtPlayerPlace leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_at_player_place_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.arm == AT_PLAYER_OLD) {
        return e;
    }
    return then(e, ended_load(r.first));
}

void abi_LoadRecordAtPlayerPlace(void)
{
    abi_ended(l8_at_player_place_exit_shim(LoadRecordAtPlayerPlace(cpu.x)));
}

/*
 * @routine 025B:1A1 LoadRecordThenSinkByF3520
 * @run 70:B1A1
 * WHAT IT DOES: In state 0 loads the record, sets the frame 6 and moves the object one left; then adds OBJ_3510 of slot $10 to its OBJ_Y_LO.
 * HOW: LoadObjectRecord, a memory decrement, an add.
 * WAITS: no.
 * Entry: X object. Exit: the add's A, N, Z, C and V; Y 11 when the record was loaded (l8_sink_exit_shim).
 */
L8Arm LoadRecordThenSinkByF3520(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t fall, y;

    r.load = l8_load_new(slot, 0xB1A9);
    r.arm = SINK_OLD;
    if (r.load.state == 0) {
        WR8(OBJ->frame[slot], 6);
        r.first = DEC8(OBJ->x_hi[slot]);
        r.arm = SINK_NEW;
    }
    fall = RD8(OBJ->f3510[0x10]);
    y = RD8(OBJ->y_lo[slot]);
    r.sum = add_bytes(fall, y, false);
    WR8(OBJ->y_lo[slot], r.sum.value);
    return r;
}

/* What LoadRecordThenSinkByF3520 leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_sink_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.arm == SINK_NEW) {
        e = then(e, ended_load(6));
        e = then(e, ended_nz(r.first));
    }
    return then(e, ended_of_sum(r.sum));
}

void abi_LoadRecordThenSinkByF3520(void)
{
    abi_ended(l8_sink_exit_shim(LoadRecordThenSinkByF3520(cpu.x)));
}

/*
 * @routine 025B:1BC LoadRecordThenStepListByKind
 * @run 70:B1BC
 * WHAT IT DOES: In state 0 loads the record and moves the object one left; then steps an animation list picked by OBJ_START_ARG (two bytes an entry) and, from the table's second half when slot $10's state is below 4 and OBJ_34B8 is 0.
 * HOW: LoadObjectRecord, a decrement, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (l8_kind_list_exit_shim).
 */
L8St12KindListArm LoadRecordThenStepListByKind(uint8_t slot, uint8_t y_entry)
{
    L8St12KindListArm r = { 0 };
    uint8_t y, hi;

    (void)y_entry;
    r.load = l8_load_new(slot, 0xB1C4);
    if (r.load.state == 0) {
        r.moved = DEC8(OBJ->x_hi[slot]);
    }
    r.kind = RD8(OBJ->f3040[slot]);
    y = (uint8_t)(r.kind << 1);
    r.timer = RD8(OBJ->f34b8[slot]);
    if (r.timer == 0) {
        r.state = RD8(OBJ->state[0x10]);
        if (r.state >= 4) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            r.counted = INC8(OBJ->f34b8[slot]);
        } else {
            r.sum = add_bytes(y, 4, false);
            y = r.sum.value;
        }
    }
    r.lo = rd8((uint16_t)(0xB1F5 + y));
    WR8(ZP->temp10, r.lo);
    hi = rd8((uint16_t)(0xB1F6 + y));
    WR8(ZP->temp11, hi);
    r.step.stored = hi;
    r.step.y = y;
    r.step.step = StepObjectFrames(slot, &r.step.y);
    return r;
}

/* What LoadRecordThenStepListByKind leaves in A, Y and the flags, from the values it read on its path. */
static Ended l8_kind_list_exit_shim(L8St12KindListArm r)
{
    Ended e = l8_load_ended(r.load);
    uint8_t doubled = (uint8_t)(r.kind << 1);

    if (r.load.state == 0) {
        e = then(e, ended_nz(r.moved));
    }
    e = then(e, ended_with_carry(ended_load(doubled), r.kind >= 0x80));
    e = ended_with_y(e, doubled);
    e = then(e, ended_load(r.timer));
    if (r.timer == 0) {
        e = then(e, ended_compare(r.state, 4));
        if (r.state >= 4) {
            e = then(e, ended_nz(r.counted));
        } else {
            e = then(e, ended_load(doubled));
            e = ended_with_y(then(e, ended_of_sum(r.sum)), r.sum.value);
        }
    }
    e = then(e, ended_load(r.lo));
    return then(e, l8_step_ended(r.step));
}

void abi_LoadRecordThenStepListByKind(void)
{
    abi_ended(l8_kind_list_exit_shim(LoadRecordThenStepListByKind(cpu.x, cpu.y)));
}

/*
 * @routine 025B:229 LoadRecordSinkThenStepListByKind
 * @run 70:B229
 * WHAT IT DOES: In state 0 loads the record and moves the object one left; then adds OBJ_3510 of slot $10 to its OBJ_Y_LO and steps the animation list picked by OBJ_START_ARG from the table at $B250.
 * HOW: LoadObjectRecord, a decrement, an add, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step (l8_sink_list_exit_shim).
 */
L8Arm LoadRecordSinkThenStepListByKind(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    uint8_t fall, ypos, y, hi;

    (void)y_entry;
    r.load = l8_load_new(slot, 0xB231);
    if (r.load.state == 0) {
        r.first = DEC8(OBJ->x_hi[slot]);
    }
    fall = RD8(OBJ->f3510[0x10]);
    ypos = RD8(OBJ->y_lo[slot]);
    r.sum = add_bytes(fall, ypos, false);
    WR8(OBJ->y_lo[slot], r.sum.value);
    r.second = RD8(OBJ->f3040[slot]);
    y = (uint8_t)(r.second << 1);
    r.third = rd8((uint16_t)(0xB250 + y));
    WR8(ZP->temp10, r.third);
    hi = rd8((uint16_t)(0xB251 + y));
    WR8(ZP->temp11, hi);
    r.step.stored = hi;
    r.step.y = y;
    r.step.step = StepObjectFrames(slot, &r.step.y);
    return r;
}

/* What LoadRecordSinkThenStepListByKind leaves in A, Y and the flags, from the values it read on its path. */
static Ended l8_sink_list_exit_shim(L8Arm r)
{
    Ended e = l8_load_ended(r.load);
    uint8_t doubled = (uint8_t)(r.second << 1);

    if (r.load.state == 0) {
        e = then(e, ended_nz(r.first));
    }
    e = then(e, ended_of_sum(r.sum));
    e = then(e, ended_with_carry(ended_load(doubled), r.second >= 0x80));
    e = ended_with_y(e, doubled);
    e = then(e, ended_load(r.third));
    return then(e, l8_step_ended(r.step));
}

void abi_LoadRecordSinkThenStepListByKind(void)
{
    abi_ended(l8_sink_list_exit_shim(LoadRecordSinkThenStepListByKind(cpu.x, cpu.y)));
}

/*
 * @routine 025B:266 StateMachineWithSoundAndList
 * @run 70:B266
 * WHAT IT DOES: An object of several phases. State 0 loads the record. State 1 waits on the state of slot $10 (3 or 6): it steps the animation list at $B2FA or $B2FF, plays sound $72 when the scroll byte ScrollXLo+1 is clear, and at state 6 goes on. State 2 counts OBJ_3510 down and goes on. Later states fall, and once off screen to the left start a list channel and clear the object.
 * HOW: Compares of OBJ_STATE and slot $10's state, SoundCommand, StepObjectFrames, StartListChannelKind2 between a PHX and a PLX, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran (l8_phases_exit_shim).
 */
L8St12PhaseArm StateMachineWithSoundAndList(uint8_t slot, uint8_t y_entry)
{
    enum { LIST_FAST, SOUND_THEN_LIST_SLOW, LIST_SLOW, ADVANCE } act;
    L8St12PhaseArm r = { 0 };
    uint8_t state, flags;

    r.load = l8_load_new(slot, 0xB26E);
    if (r.load.state == 0) {
        r.moved = DEC8(OBJ->x_hi[slot]);
        r.arm = PHASE_NEW;
        return r;
    }
    state = r.load.state;
    if (state == 1) {
        r.other = RD8(OBJ->state[0x10]);
        if (r.other == 3) {
            r.timer = RD8(OBJ->f34b8[slot]);
            if (r.timer != 0) {
                act = LIST_SLOW;
            } else {
                r.scroll = RD8(SCROLL->x_lo[1]);
                act = ((r.scroll & 0x7F) == 0) ? SOUND_THEN_LIST_SLOW : LIST_FAST;
            }
        } else {
            if (r.other == 6) {
                act = ADVANCE;
            } else {
                r.timer = RD8(OBJ->f34b8[slot]);
                act = (r.timer != 0) ? LIST_SLOW : LIST_FAST;
            }
        }
        if (act == ADVANCE) {
            WR8(OBJ->frame[slot], 0x1C);
            r.counted = INC8(OBJ->state[slot]);
            WR8(OBJ->f3510[slot], 0x0A);
            r.arm = PHASE_ADVANCE;
            return r;
        }
        if (act == LIST_FAST) {
            WR8(ZP->temp11, 0xB2);
            WR8(ZP->temp10, 0xFA);
            r.step.y = y_entry;
            r.step.step = StepObjectFrames(slot, &r.step.y);
            r.arm = PHASE_FAST;
            return r;
        }
        if (act == SOUND_THEN_LIST_SLOW) {
            r.sounded = true;
            jsr_push(0xB2A3);
            r.call = SoundCommand(0x72, slot, y_entry);
            jsr_pop();
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            r.counted = INC8(OBJ->f34b8[slot]);
        }
        r.step = l8_list_step(slot, 0xB2FF, 0xB2B7, true, y_entry);
        if (r.step.step == FRAME_LIST_ENDED) {
            WR8(OBJ->f34b8[slot], 0);
        }
        r.arm = PHASE_SLOW;
        return r;
    }
    if (state == 2) {
        r.timer = DEC8(OBJ->f3510[slot]);
        if (r.timer == 0) {
            flags = RD8(OBJ->scroll_band[slot]);
            r.flags = (uint8_t)((flags & 0xF0) | 0x05);
            WR8(OBJ->scroll_band[slot], r.flags);
            r.counted = INC8(OBJ->state[slot]);
        }
        r.arm = PHASE_TWO;
        return r;
    }
    r.counted = INC8(OBJ->y_lo[slot]);
    r.x_lo = RD8(OBJ->x_lo[slot]);
    if ((r.x_lo & 0x80) != 0) {
        r.arm = PHASE_X_NEGATIVE;
        return r;
    }
    r.x_hi = RD8(OBJ->x_hi[slot]);
    if ((r.x_hi & 0x80) == 0) {
        r.arm = PHASE_X_ON;
        return r;
    }
    r.x = l8_channel2(slot, 0x82, 5, 0xB2F6);
    ClearObject(r.x);
    r.arm = PHASE_GONE;
    return r;
}

/* What StateMachineWithSoundAndList leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_phases_exit_shim(L8St12PhaseArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    uint8_t state = r.load.state;
    Ended e;

    if (r.arm == PHASE_NEW) {
        s.end = then(l8_load_ended(r.load), ended_nz(r.moved));
        return s;
    }
    e = ended_load((uint8_t)(state - 1));
    if (state == 1) {
        e = ended_compare(r.other, 3);
        if (r.other == 3) {
            e = then(e, ended_load(r.timer));
            if (r.timer == 0) {
                e = then(e, ended_load((uint8_t)(r.scroll & 0x7F)));
            }
        } else {
            e = ended_compare(r.other, 6);
            if (r.other != 6) {
                e = then(e, ended_load(r.timer));
            }
        }
        if (r.arm == PHASE_ADVANCE) {
            e = then(e, ended_load(0x1C));
            e = then(e, ended_nz(r.counted));
            s.end = then(e, ended_load(0x0A));
            return s;
        }
        if (r.arm == PHASE_FAST) {
            e = then(e, ended_load(0xFA));
            s.end = then(e, ended_of_frame_step(r.step.step, r.step.step != FRAME_HELD, r.step.y));
            return s;
        }
        if (r.sounded) {
            s.sounded = true;
            s.command = 0x72;
            s.before = then(e, ended_load(0x72));
            s.call = r.call;
            e = ended_nz(r.counted);
        }
        s.end = then(e, l8_step_ended(r.step));
        return s;
    }
    e = then(e, ended_load((uint8_t)(state - 2)));
    if (state == 2) {
        e = then(e, ended_nz(r.timer));
        if (r.timer == 0) {
            e = then(e, ended_load(r.flags));
            e = then(e, ended_nz(r.counted));
        }
        s.end = e;
        return s;
    }
    e = then(e, ended_nz(r.counted));
    e = then(e, ended_load(r.x_lo));
    if (r.arm == PHASE_X_NEGATIVE) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.x_hi));
    if (r.arm == PHASE_X_ON) {
        s.end = e;
        return s;
    }
    s.end = then(e, l8_channel2_ended(r.x));
    return s;
}

void abi_StateMachineWithSoundAndList(void)
{
    abi_sound_end(l8_phases_exit_shim(StateMachineWithSoundAndList(cpu.x, cpu.y)));
}

/*
 * @routine 025B:308 DriftRecordThenMoveWithJitter
 * @run 70:B308
 * WHAT IT DOES: In state 0 loads the record and sets the speed X -$18, Y $0C. When slot $10's state is 4 or more and OBJ_START_ARG is 0 it waits one round, shows frame $1D, then moves by its speed, nudging X and Y back by random amounts of up to 15 when they are off the edge.
 * HOW: LoadObjectRecord, ApplyVelocity, NextRandom, adds and a subtract with a zero page temporary.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_drift_exit_shim).
 */
L8St12DriftArm DriftRecordThenMoveWithJitter(uint8_t slot)
{
    L8St12DriftArm r = { 0 };
    uint8_t x, y, mask, temp;

    r.load = l8_load_new(slot, 0xB310);
    if (r.load.state == 0) {
        WR8(OBJ->vx_hi[slot], 0xE8);
        WR8(OBJ->vy_hi[slot], 0x0C);
        r.arm = DRIFT_NEW;
        return r;
    }
    r.state = RD8(OBJ->state[0x10]);
    if (r.state < 4) {
        r.arm = DRIFT_WAIT;
        return r;
    }
    r.kind = RD8(OBJ->f3040[slot]);
    if (r.kind != 0) {
        r.arm = DRIFT_OTHER;
        return r;
    }
    r.timer = RD8(OBJ->f34b8[slot]);
    if (r.timer == 0) {
        r.counted = INC8(OBJ->f34b8[slot]);
        WR8(OBJ->frame[slot], 0x1D);
        r.arm = DRIFT_SHOW;
        return r;
    }
    jsr_push(0xB338);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.x_lo = RD8(OBJ->x_lo[slot]);
    if ((r.x_lo & 0x80) != 0) {
        r.x_hi = RD8(OBJ->x_hi[slot]);
        if ((r.x_hi & 0x80) != 0) {
            r.x_step = INC8(OBJ->x_hi[slot]);
            jsr_push(0xB348);
            r.x_random = NextRandom();
            jsr_pop();
            mask = r.x_random.value & 0x0F;
            x = RD8(OBJ->x_lo[slot]);
            r.x_sum = add_bytes(mask, x, false);
            WR8(OBJ->x_lo[slot], r.x_sum.value);
        }
    }
    r.y_lo = RD8(OBJ->y_lo[slot]);
    if ((r.y_lo & 0x80) != 0) {
        r.arm = DRIFT_Y_NEGATIVE;
        return r;
    }
    r.y_hi = RD8(OBJ->y_hi[slot]);
    if (r.y_hi == 0) {
        r.arm = DRIFT_Y_HIGH_ZERO;
        return r;
    }
    r.y_step = DEC8(OBJ->y_hi[slot]);
    jsr_push(0xB361);
    r.y_random = NextRandom();
    jsr_pop();
    mask = r.y_random.value & 0x0F;
    WR8(ZP->temp00, mask);
    y = RD8(OBJ->y_lo[slot]);
    temp = RD8(ZP->temp00);
    r.y_sum = sub_bytes(y, temp, true);
    WR8(OBJ->y_lo[slot], r.y_sum.value);
    r.arm = DRIFT_JITTER;
    return r;
}

/* What DriftRecordThenMoveWithJitter leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_drift_exit_shim(L8St12DriftArm r)
{
    Ended e;

    if (r.arm == DRIFT_NEW) {
        return then(l8_load_ended(r.load), ended_load(0x0C));
    }
    e = ended_compare(r.state, 4);
    if (r.arm == DRIFT_WAIT) {
        return e;
    }
    e = then(e, ended_load(r.kind));
    if (r.arm == DRIFT_OTHER) {
        return e;
    }
    e = then(e, ended_load(r.timer));
    if (r.arm == DRIFT_SHOW) {
        e = then(e, ended_nz(r.counted));
        return then(e, ended_load(0x1D));
    }
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load(r.x_lo));
    if ((r.x_lo & 0x80) != 0) {
        e = then(e, ended_load(r.x_hi));
        if ((r.x_hi & 0x80) != 0) {
            e = then(e, ended_nz(r.x_step));
            e = then(e, ended_of_random(r.x_random));
            e = then(e, ended_load((uint8_t)(r.x_random.value & 0x0F)));
            e = then(e, ended_of_sum(r.x_sum));
        }
    }
    e = then(e, ended_load(r.y_lo));
    if (r.arm == DRIFT_Y_NEGATIVE) {
        return e;
    }
    e = then(e, ended_load(r.y_hi));
    if (r.arm == DRIFT_Y_HIGH_ZERO) {
        return e;
    }
    e = then(e, ended_nz(r.y_step));
    e = then(e, ended_of_random(r.y_random));
    e = then(e, ended_load((uint8_t)(r.y_random.value & 0x0F)));
    return then(e, ended_of_sum(r.y_sum));
}

void abi_DriftRecordThenMoveWithJitter(void)
{
    abi_ended(l8_drift_exit_shim(DriftRecordThenMoveWithJitter(cpu.x)));
}

/*
 * @routine 025B:487 SwingByCosineFromState0B
 * @run 70:B487
 * WHAT IT DOES: From state $0B on, sets the Y speed from the cosine of OBJ_3618 scaled by $0040, moves the object in Y and turns OBJ_3618 on by 2.
 * HOW: ScaleByCosine, two copies of the product, ApplyVelocityY, an add.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_swing_by_cosine_exit_shim).
 */
L8St12SwingArm SwingByCosineFromState0B(uint8_t slot)
{
    L8St12SwingArm r = { 0 };
    uint8_t angle;

    r.state = RD8(OBJ->state[slot]);
    if (r.state < 0x0B) {
        r.arm = SWING_EARLY;
        return r;
    }
    WR8(ZP->temp00, 0x40);
    WR8(ZP->temp01, 0);
    r.angle = RD8(OBJ->f3618[slot]);
    jsr_push(0xB49C);
    r.scale = ScaleByCosine(r.angle);
    jsr_pop();
    r.high = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], r.high);
    r.mid = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], r.mid);
    jsr_push(0xB4A9);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    angle = RD8(OBJ->f3618[slot]);
    r.sum = add_bytes(angle, 2, false);
    WR8(OBJ->f3618[slot], r.sum.value);
    r.arm = SWING_MOVED;
    return r;
}

/* What SwingByCosineFromState0B leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_swing_by_cosine_exit_shim(L8St12SwingArm r)
{
    Ended e = ended_compare(r.state, 0x0B);

    if (r.arm == SWING_EARLY) {
        return e;
    }
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_load(r.high));
    e = then(e, ended_load(r.mid));
    e = then(e, ended_of_axis_move(r.move));
    return then(e, ended_of_sum(r.sum));
}

void abi_SwingByCosineFromState0B(void)
{
    abi_ended(l8_swing_by_cosine_exit_shim(SwingByCosineFromState0B(cpu.x)));
}

/*
 * @routine 025B:4B3 StartRandomScriptWhenFlagged
 * @run 70:B4B3
 * WHAT IT DOES: When OBJ_3720 is set and OBJ_3568's low five bits are 0, with a chance of three in eight starts a script from the table at $B4E9 as kind 1 in slot 5.
 * HOW: Two NextRandom calls and StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_random_script_exit_shim).
 */
L8St12ScriptArm StartRandomScriptWhenFlagged(uint8_t slot)
{
    L8St12ScriptArm r = { 0 };
    uint8_t pick;

    r.flag = RD8(OBJ->f3720[slot]);
    if (r.flag == 0) {
        r.arm = RSCRIPT_OFF;
        return r;
    }
    r.timer = RD8(OBJ->f3568[slot]);
    if ((r.timer & 0x1F) != 0) {
        r.arm = RSCRIPT_NOT_YET;
        return r;
    }
    jsr_push(0xB4C2);
    r.random = NextRandom();
    jsr_pop();
    pick = r.random.value & 0x07;
    if (pick >= 3) {
        r.arm = RSCRIPT_NO_LUCK;
        return r;
    }
    push8(slot);
    jsr_push(0xB4CC);
    r.random2 = NextRandom();
    jsr_pop();
    pick = r.random2.value & 0x03;
    r.script = rd8((uint16_t)(0xB4E9 + pick));
    jsr_push(0xB4D7);
    StartScriptSlotKind1(r.script, 5);
    jsr_pop();
    r.x = pull8();
    r.arm = RSCRIPT_STARTED;
    return r;
}

/* What StartRandomScriptWhenFlagged leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_random_script_exit_shim(L8St12ScriptArm r)
{
    Ended e = ended_load(r.flag);

    if (r.arm == RSCRIPT_OFF) {
        return e;
    }
    e = then(e, ended_load((uint8_t)(r.timer & 0x1F)));
    if (r.arm == RSCRIPT_NOT_YET) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_compare((uint8_t)(r.random.value & 0x07), 3));
    if (r.arm == RSCRIPT_NO_LUCK) {
        return e;
    }
    e = then(e, ended_of_random(r.random2));
    return then(e, ended_with_y(ended_with_a(ended_nz(r.x), r.script), r.script));
}

void abi_StartRandomScriptWhenFlagged(void)
{
    abi_ended(l8_random_script_exit_shim(StartRandomScriptWhenFlagged(cpu.x)));
}

/*
 * @routine 025B:4ED PushPlayerWhenFlagged
 * @run 70:B4ED
 * WHAT IT DOES: When OBJ_3778 is set, pushes the player: zPlayerPush $10 and zPushX $FF60.
 * HOW: A load and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $60 when set; otherwise N and Z from OBJ_3778 (l8_push_player_exit_shim).
 */
L8Arm PushPlayerWhenFlagged(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->f3778[slot]);
    if (r.first == 0) {
        r.arm = PUSH_OFF;
        return r;
    }
    WR8(ZP->player_push, 0x10);
    WR8(ZP->push_x.hi, 0xFF);
    WR8(ZP->push_x.lo, 0x60);
    r.arm = PUSH_ON;
    return r;
}

/* What PushPlayerWhenFlagged leaves in A and the flags, from the arm it ended on. */
Ended l8_push_player_exit_shim(L8Arm r)
{
    if (r.arm == PUSH_OFF) {
        return ended_load(r.first);
    }
    return ended_load(0x60);
}

void abi_PushPlayerWhenFlagged(void)
{
    abi_ended(l8_push_player_exit_shim(PushPlayerWhenFlagged(cpu.x)));
}

/*
 * @routine 025B:4FF HitSoundThenSpawnPieces
 * @run 70:B4FF
 * WHAT IT DOES: When the object's hits taken reach 5, plays sound $75 and sets OBJ_3828 to $10. While OBJ_3828 runs it counts down and, whenever the wave counter's low three bits are 0, starts an object of type $2F in a free slot of $20 to $3F at the object's place with OBJ_34B8 1.
 * HOW: SoundCommand, FindFreeSlotFrom20, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_hit_sound_spawn_exit_shim).
 */
L8St12HitArm HitSoundThenSpawnPieces(uint8_t slot, uint8_t y_entry)
{
    L8St12HitArm r = { 0 };

    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits == 5) {
        r.sounded = true;
        jsr_push(0xB50B);
        r.call = SoundCommand(0x75, slot, y_entry);
        jsr_pop();
        WR8(OBJ->f3828[slot], 0x10);
    }
    r.timer = RD8(OBJ->f3828[slot]);
    if (r.timer == 0) {
        r.arm = HIT_IDLE;
        return r;
    }
    r.counted = DEC8(OBJ->f3828[slot]);
    r.wave = RD8(ZP->wave_counter);
    if ((r.wave & 0x07) != 0) {
        r.arm = HIT_COUNTED;
        return r;
    }
    jsr_push(0xB521);
    r.found.slot = FindFreeSlotFrom20(&r.found.last, &r.found.found);
    jsr_pop();
    if (!r.found.found) {
        r.arm = HIT_NO_SLOT;
        return r;
    }
    jsr_push(0xB528);
    r.placed = StartObjectAtPlace(0x2F, slot, r.found.slot);
    jsr_pop();
    WR8(OBJ->f34b8[r.found.slot], 1);
    r.arm = HIT_SPAWNED;
    return r;
}

/* What HitSoundThenSpawnPieces leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
SoundEnd l8_hit_sound_spawn_exit_shim(L8St12HitArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_compare(r.hits, 5);

    if (r.sounded) {
        s.sounded = true;
        s.command = 0x75;
        s.before = then(e, ended_load(0x75));
        s.call = r.call;
        e = ended_load(0x10);
    }
    e = then(e, ended_load(r.timer));
    if (r.arm == HIT_IDLE) {
        s.end = e;
        return s;
    }
    e = then(e, ended_nz(r.counted));
    e = then(e, ended_load((uint8_t)(r.wave & 0x07)));
    if (r.arm == HIT_COUNTED) {
        s.end = e;
        return s;
    }
    e = then(e, l8_slot_ended(r.found));
    if (r.arm == HIT_NO_SLOT) {
        s.end = e;
        return s;
    }
    e = then(e, ended_load(r.placed));
    s.end = then(e, ended_load(1));
    return s;
}

void abi_HitSoundThenSpawnPieces(void)
{
    abi_sound_end(l8_hit_sound_spawn_exit_shim(HitSoundThenSpawnPieces(cpu.x, cpu.y)));
}

/*
 * @routine 025B:52E EnterState0BWhenFlagged
 * @run 70:B52E
 * WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is below $0B, sets the state $0B and OBJ_SUBSTATE 0.
 * HOW: Two tests, two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_enter_state0b_exit_shim). The stores ran in no script; the draft stops there.
 */
L8Arm EnterState0BWhenFlagged(uint8_t slot)
{
    L8Arm r = { 0 };

    r.first = RD8(OBJ->hit_status[slot]);
    if ((r.first & 0x80) == 0) {
        r.arm = S0B_CLEAR;
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    if (r.second >= 0x0B) {
        r.arm = S0B_LATE;
        return r;
    }
    WR8(OBJ->state[slot], 0x0B);
    WR8(OBJ->substate[slot], 0);
    r.arm = S0B_ENTERED;
    return r;
}

/* What EnterState0BWhenFlagged leaves in A and the flags, from the arm it ended on. */
Ended l8_enter_state0b_exit_shim(L8Arm r)
{
    Ended e = ended_load((uint8_t)(r.first & 0x80));

    if (r.arm == S0B_CLEAR) {
        return e;
    }
    e = then(e, ended_compare(r.second, 0x0B));
    if (r.arm == S0B_LATE) {
        return e;
    }
    return then(e, ended_load(0x0B));
}

void abi_EnterState0BWhenFlagged(void)
{
    abi_ended(l8_enter_state0b_exit_shim(EnterState0BWhenFlagged(cpu.x)));
}

/*
 * @routine 025B:56F StartPieceAtTopLeft
 * @run 70:B56F
 * WHAT IT DOES: Loads the object's record, clears slot $18, starts an object of type $28 there at the object's place, moves the new object's source to X $003C, Y $0030 with X speed $80 (OBJ_VX_LO), OBJ_34B8 4 and OBJ_3720 1.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 1, Y $18, C from the record (l8_top_left_exit_shim).
 */
L8Arm StartPieceAtTopLeft(uint8_t slot)
{
    L8Arm r = { 0 };

    jsr_push(0xB572);
    r.record = l8_record(slot);
    jsr_pop();
    jsr_push(0xB577);
    ClearObjectInY(0x18);
    jsr_pop();
    jsr_push(0xB57C);
    r.first = StartObjectAtPlace(0x28, slot, 0x18);
    jsr_pop();
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x3C);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x30);
    WR8(OBJ->vx_lo[slot], 0x80);
    WR8(OBJ->f34b8[slot], 0x04);
    WR8(OBJ->f3720[slot], 0x01);
    return r;
}

/* What StartPieceAtTopLeft leaves in A, Y and the flags (one path). */
static Ended l8_top_left_exit_shim(L8Arm r)
{
    Ended e = l8_record_ended(r.record);

    e = then(e, ended_with_y(ended_nz(0x18), 0x18));
    e = then(e, ended_load(r.first));
    return then(e, ended_load(1));
}

void abi_StartPieceAtTopLeft(void)
{
    abi_ended(l8_top_left_exit_shim(StartPieceAtTopLeft(cpu.x)));
}

/*
 * @routine 025B:59C CountDownThenStartScripts
 * @run 70:B59C
 * WHAT IT DOES: Counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero starts scripts $84 (slot 5) and $86 (slot 4) as kind 1, clears OBJ_3720, sets OBJ_3510 $60 and goes to the next state.
 * HOW: A 16-bit subtract in memory, an ORA, StartScriptSlotKind1 twice between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_start_scripts_exit_shim).
 */
L8Arm CountDownThenStartScripts(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t low, high;
    Sum diff;

    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], diff.value);
    r.sum = diff;
    r.first = RD8(OBJ->f3510[slot]);
    if ((uint8_t)(diff.value | r.first) != 0) {
        r.arm = SCRIPTS_COUNTING;
        return r;
    }
    push8(slot);
    jsr_push(0xB5BA);
    StartScriptSlotKind1(0x84, 5);
    jsr_pop();
    jsr_push(0xB5C1);
    StartScriptSlotKind1(0x86, 4);
    jsr_pop();
    slot = pull8();
    WR8(OBJ->f3720[slot], 0);
    WR8(OBJ->f3510[slot], 0x60);
    r.second = INC8(OBJ->state[slot]);
    r.arm = SCRIPTS_STARTED;
    return r;
}

/* What CountDownThenStartScripts leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_start_scripts_exit_shim(L8Arm r)
{
    Ended e = then(ended_of_sum(r.sum), ended_load((uint8_t)(r.sum.value | r.first)));

    if (r.arm == SCRIPTS_COUNTING) {
        return e;
    }
    e = then(e, ended_with_y(ended_load(0x60), 0x86));
    return then(e, ended_nz(r.second));
}

void abi_CountDownThenStartScripts(void)
{
    abi_ended(l8_start_scripts_exit_shim(CountDownThenStartScripts(cpu.x)));
}

/*
 * @routine 025B:5CE CountDownWithSound77
 * @run 70:B5CE
 * WHAT IT DOES: Counts OBJ_3510 down; at $35 plays sound $77; at zero sets OBJ_3510 $80 and goes to the next state.
 * HOW: A memory decrement, a compare, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_sound77_exit_shim).
 */
L8St12Sound77Arm CountDownWithSound77(uint8_t slot, uint8_t y_entry)
{
    L8St12Sound77Arm r = { 0 };

    r.timer = DEC8(OBJ->f3510[slot]);
    if (r.timer == 0) {
        WR8(OBJ->f3510[slot], 0x80);
        r.counted = INC8(OBJ->state[slot]);
        r.arm = S77_ZERO;
        return r;
    }
    r.now = RD8(OBJ->f3510[slot]);
    if (r.now != 0x35) {
        r.arm = S77_COUNTING;
        return r;
    }
    jsr_push(0xB5DF);
    r.call = SoundCommand(0x77, slot, y_entry);
    jsr_pop();
    r.arm = S77_SOUNDED;
    return r;
}

/* What CountDownWithSound77 leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_sound77_exit_shim(L8St12Sound77Arm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e;

    if (r.arm == S77_ZERO) {
        s.end = then(then(ended_nz(r.timer), ended_load(0x80)), ended_nz(r.counted));
        return s;
    }
    e = ended_compare(r.now, 0x35);
    if (r.arm == S77_COUNTING) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x77;
    s.before = then(e, ended_load(0x77));
    s.call = r.call;
    return s;
}

void abi_CountDownWithSound77(void)
{
    abi_sound_end(l8_sound77_exit_shim(CountDownWithSound77(cpu.x, cpu.y)));
}

/*
 * @routine 025B:617 StartScriptAtTimerMarks
 * @run 70:B617
 * WHAT IT DOES: When OBJ_3510 is 0 sets OBJ_3720. Otherwise counts it down, and when it equals one of the eight marks at $B642 starts the script of the same index in the table at $B64A as kind 1 in slot 5.
 * HOW: A loop over the marks, StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_timer_marks_exit_shim).
 */
L8St12MarksArm StartScriptAtTimerMarks(uint8_t slot)
{
    L8St12MarksArm r = { 0 };
    uint8_t y;

    r.first = RD8(OBJ->f3510[slot]);
    if (r.first == 0) {
        WR8(OBJ->f3720[slot], 1);
        r.arm = MARKS_IDLE;
        return r;
    }
    y = 0;
    for (;;) {
        r.timer = RD8(OBJ->f3510[slot]);
        r.mark = rd8((uint16_t)(0xB642 + y));
        r.at = y;
        if (r.timer == r.mark) {
            break;
        }
        y++;
        r.next = y;
        if (y >= 8) {
            r.counted = DEC8(OBJ->f3510[slot]);
            r.arm = MARKS_NONE;
            return r;
        }
    }
    push8(slot);
    r.script = rd8((uint16_t)(0xB64A + y));
    jsr_push(0xB63D);
    StartScriptSlotKind1(r.script, 5);
    jsr_pop();
    slot = pull8();
    r.x = slot;
    r.counted = DEC8(OBJ->f3510[slot]);
    r.arm = MARKS_STARTED;
    return r;
}

/* What StartScriptAtTimerMarks leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_timer_marks_exit_shim(L8St12MarksArm r)
{
    Ended e;

    if (r.arm == MARKS_IDLE) {
        return ended_load(1);
    }
    e = ended_load(r.first);
    e = ended_with_y(ended_compare(r.timer, r.mark), r.at);
    if (r.arm == MARKS_NONE) {
        e = then(e, ended_with_y(ended_nz(r.next), r.next));
        e = then(e, l8_compare_y(r.next, 8));
        return then(e, ended_nz(r.counted));
    }
    return then(then(e, ended_with_y(ended_with_a(ended_nz(r.x), r.script), r.script)), ended_nz(r.counted));
}

void abi_StartScriptAtTimerMarks(void)
{
    abi_ended(l8_timer_marks_exit_shim(StartScriptAtTimerMarks(cpu.x)));
}

/*
 * @routine 025B:652 SpinOutThenLeave
 * @run 70:B652
 * WHAT IT DOES: Moves the object on a circle of radius $0200 by OBJ_35C0 (turning on 4 a frame) while it steps the animation list at $B6A4; when the list ends it starts the next phase: slot $10's state up, OBJ_3778 1, OBJ_3250 2, slot 0's OBJ_3510 2, the byte at $3BF0 2, OBJ_3568 of slot 0 0, OBJ_35C0 $C0, OBJ_3510 $20, OBJ_3670 1 and the next state.
 * HOW: SetVelocityFromAngle, ApplyVelocity, an add, StepObjectFrames and stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_spin_out_exit_shim). The byte at $3BF0 keeps its address form until it has a name.
 */
L8Arm SpinOutThenLeave(uint8_t slot, uint8_t y_entry)
{
    L8Arm r = { 0 };
    uint8_t angle;

    (void)y_entry;
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 2);
    r.first = RD8(OBJ->f35c0[slot]);
    jsr_push(0xB65E);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    jsr_push(0xB661);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->f35c0[slot]);
    r.sum = add_bytes(angle, 4, false);
    WR8(OBJ->f35c0[slot], r.sum.value);
    r.step = l8_list_step(slot, 0xB6A4, 0xB675, false, 0);
    if (r.step.step != FRAME_LIST_ENDED) {
        r.arm = SPIN_OUT_GOING;
        return r;
    }
    INC8(OBJ->state[0x10]);
    WR8(OBJ->f3778[slot], 1);
    WR8(OBJ->hitbox_on[slot], 2);
    WR8(OBJ->f3510[0], 2);
    wr8(0x3BF0, 2);
    WR8(OBJ->f3568[0], 0);
    WR8(OBJ->f35c0[slot], 0xC0);
    WR8(OBJ->f3510[slot], 0x20);
    WR8(OBJ->f3670[slot], 1);
    r.second = INC8(OBJ->state[slot]);
    r.arm = SPIN_OUT_ENDED;
    return r;
}

/* What SpinOutThenLeave leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_spin_out_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.sum));
    e = then(e, l8_step_ended(r.step));
    if (r.arm == SPIN_OUT_GOING) {
        return e;
    }
    return then(then(e, ended_load(1)), ended_nz(r.second));
}

void abi_SpinOutThenLeave(void)
{
    abi_ended(l8_spin_out_exit_shim(SpinOutThenLeave(cpu.x, cpu.y)));
}

/*
 * @routine 025B:6E7 SpinOnCircle
 * @run 70:B6E7
 * WHAT IT DOES: Moves the object on a circle of radius $0180 by OBJ_35C0, turning on 2 a frame; when the angle wraps to 0 it jumps to X $00D5, Y $002F with the fractions $4C and $83.
 * HOW: SetVelocityFromAngle, ApplyVelocity, an add, four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_spin_on_circle_exit_shim).
 */
L8Arm SpinOnCircle(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t angle;

    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 1);
    r.first = RD8(OBJ->f35c0[slot]);
    jsr_push(0xB6F5);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    jsr_push(0xB6F8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    angle = RD8(OBJ->f35c0[slot]);
    r.sum = add_bytes(angle, 2, false);
    WR8(OBJ->f35c0[slot], r.sum.value);
    if (r.sum.value != 0) {
        r.arm = CIRCLE_TURNING;
        return r;
    }
    WR8(OBJ->x_lo[slot], 0xD5);
    WR8(OBJ->x_frac[slot], 0x4C);
    WR8(OBJ->y_lo[slot], 0x2F);
    WR8(OBJ->y_frac[slot], 0x83);
    r.arm = CIRCLE_WRAPPED;
    return r;
}

/* What SpinOnCircle leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_spin_on_circle_exit_shim(L8Arm r)
{
    Ended e = ended_load(r.first);

    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_sum(r.sum));
    if (r.arm == CIRCLE_TURNING) {
        return e;
    }
    return then(e, ended_load(0x83));
}

void abi_SpinOnCircle(void)
{
    abi_ended(l8_spin_on_circle_exit_shim(SpinOnCircle(cpu.x)));
}

/*
 * @routine 025B:761 StepListThenBurstAtFrame18
 * @run 70:B761
 * WHAT IT DOES: Steps the animation list at $B79A. When the graphic changes and the frame is $18, plays sound $42, clears slot $50 and starts an object of type $29 there at the object's place. When the list ends it sets OBJ_34B8 1, OBJ_3510 $80, OBJ_3670 3 and goes to the next state.
 * HOW: StepObjectFrames, SoundCommand, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_burst18_exit_shim).
 */
L8St12BurstArm StepListThenBurstAtFrame18(uint8_t slot, uint8_t y_entry)
{
    L8St12BurstArm r = { 0 };

    r.step = l8_list_step(slot, 0xB79A, 0xB76C, false, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        WR8(OBJ->f34b8[slot], 1);
        WR8(OBJ->f3510[slot], 0x80);
        WR8(OBJ->f3670[slot], 3);
        r.counted = INC8(OBJ->state[slot]);
        r.arm = BURST_ENDED;
        return r;
    }
    if (r.step.step != FRAME_CHANGED) {
        r.arm = BURST_HELD;
        return r;
    }
    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame != 0x18) {
        r.arm = BURST_OTHER_FRAME;
        return r;
    }
    jsr_push(0xB77C);
    r.call = SoundCommand(0x42, slot, r.step.y);
    jsr_pop();
    jsr_push(0xB781);
    ClearObjectInY(0x50);
    jsr_pop();
    jsr_push(0xB786);
    r.placed = StartObjectAtPlace(0x29, slot, 0x50);
    jsr_pop();
    r.arm = BURST_SOUNDED;
    return r;
}

/* What StepListThenBurstAtFrame18 leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_burst18_exit_shim(L8St12BurstArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_of_frame_step(r.step.step, r.step.step != FRAME_HELD, r.step.y);

    switch (r.arm) {
    case BURST_ENDED:
        s.end = then(then(e, ended_load(3)), ended_nz(r.counted));
        return s;
    case BURST_HELD:
        s.end = e;
        return s;
    case BURST_OTHER_FRAME:
        s.end = then(e, ended_compare(r.frame, 0x18));
        return s;
    default:
        e = then(e, ended_compare(r.frame, 0x18));
        s.sounded = true;
        s.command = 0x42;
        s.before = then(e, ended_load(0x42));
        s.call = r.call;
        s.end = then(ended_with_y(ended_nz(0x50), 0x50), ended_load(r.placed));
        return s;
    }
}

void abi_StepListThenBurstAtFrame18(void)
{
    abi_sound_end(l8_burst18_exit_shim(StepListThenBurstAtFrame18(cpu.x, cpu.y)));
}

/*
 * @routine 025B:7F5 HomeOnPointThenRingAndSound
 * @run 70:B7F5
 * WHAT IT DOES: Steps the animation list at $B6DA while it flies toward the point X $00E0, Y $0030 on a circle of radius $0100; once on it (within 3) plays sound $73, sets OBJ_3670 4 and OBJ_3510 $B0, starts list $83 on channel 5, restarts the animation, and goes to state $0B with OBJ_SUBSTATE 2.
 * HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, two range tests, SoundCommand, StartListChannelKind2.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_home_exit_shim).
 */
L8St12HomeArm HomeOnPointThenRingAndSound(uint8_t slot, uint8_t y_entry)
{
    L8St12HomeArm r = { 0 };
    uint8_t y, lo;
    Sum diff;

    r.step = l8_list_step(slot, 0xB6DA, 0xB800, false, y_entry);
    WR8(ZP->temp09, 0);
    WR8(ZP->temp08, 0xE0);
    WR8(ZP->temp0b, 0);
    WR8(ZP->temp0a, 0x30);
    jsr_push(0xB80F);
    r.toward = AngleToPoint(slot);
    jsr_pop();
    y = r.toward.y;
    push8(r.toward.a);
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    r.angle = pull8();
    jsr_push(0xB81A);
    r.scale = SetVelocityFromAngle(r.angle, slot);
    jsr_pop();
    y = 0;
    jsr_push(0xB81D);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.x_hi = RD8(OBJ->x_hi[slot]);
    r.y_hi = RD8(OBJ->y_hi[slot]);
    if ((r.x_hi | r.y_hi) != 0) {
        r.arm = HOME_FAR;
        return r;
    }
    lo = RD8(OBJ->x_lo[slot]);
    diff = sub_bytes(lo, 0xE0, true);
    r.x_near = add_bytes(diff.value, 4, false);
    if (r.x_near.value >= 8) {
        r.arm = HOME_X_OFF;
        return r;
    }
    lo = RD8(OBJ->y_lo[slot]);
    diff = sub_bytes(lo, 0x30, true);
    r.y_near = add_bytes(diff.value, 4, false);
    if (r.y_near.value >= 8) {
        r.arm = HOME_Y_OFF;
        return r;
    }
    jsr_push(0xB844);
    r.call = SoundCommand(0x73, slot, y);
    jsr_pop();
    WR8(OBJ->f3670[slot], 4);
    WR8(OBJ->f3510[slot], 0xB0);
    push8(slot);
    jsr_push(0xB856);
    StartListChannelKind2(0x83, 5);
    jsr_pop();
    slot = pull8();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 0x0B);
    WR8(OBJ->substate[slot], 2);
    r.arm = HOME_ARRIVED;
    return r;
}

/* What HomeOnPointThenRingAndSound leaves in A, Y and the flags, with its sound call, from the arm it ended on. */
static SoundEnd l8_home_exit_shim(L8St12HomeArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_of_frame_step(r.step.step, r.step.step != FRAME_HELD, r.step.y);

    e = then(e, l8_of_angle(r.toward));
    e = then(e, ended_load(r.angle));
    e = then(e, l8_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_load((uint8_t)(r.x_hi | r.y_hi)));
    if (r.arm == HOME_FAR) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.x_near.value, 8));
    if (r.arm == HOME_X_OFF) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.y_near.value, 8));
    if (r.arm == HOME_Y_OFF) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x73;
    s.before = then(e, ended_load(0x73));
    s.call = r.call;
    s.end = ended_load(2);
    return s;
}

void abi_HomeOnPointThenRingAndSound(void)
{
    abi_sound_end(l8_home_exit_shim(HomeOnPointThenRingAndSound(cpu.x, cpu.y)));
}
