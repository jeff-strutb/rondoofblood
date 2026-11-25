/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025B (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/*
 * @routine 025B:0F0 StepVerticalScrollTable
 * @run 70:B0F0
 * WHAT IT DOES: While OBJ_3568 is not 0, steps through the table at $B129 and $B133: OBJ_3510 takes the entry and the scroll's Y (ScrollYLo+1, ScrollYHi+1) falls by it; OBJ_3568 counts up and wraps to 0 at 9. When it is 0 and the scroll byte ScrollXLo+5 is 0, a random number with its top bit set starts the table (OBJ_3568 1).
 * HOW: A table read by OBJ_3568 as the index, a 16-bit subtract, and NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction that ran.
 */
Ended StepVerticalScrollTable(uint8_t slot)
{
    uint8_t index, scroll, value, low, high, shifted, step;
    Sum diff;
    RandomResult r;
    Ended e;

    index = RD8(OBJ->f3568[slot]);
    e = ended_load(index);
    if (index == 0) {
        scroll = RD8(SCROLL->x_lo[5]);
        e = then(e, ended_load(scroll));
        if (scroll != 0) {
            return e;
        }
        jsr_push(0xB122);
        r = NextRandom();
        jsr_pop();
        e = then(e, ended_of_random(r));
        shifted = (uint8_t)(r.value << 1);
        e = then(e, ended_with_carry(ended_load(shifted), (r.value & 0x80) != 0));
        if ((r.value & 0x80) == 0) {
            return e;
        }
        step = INC8(OBJ->f3568[slot]);
        return then(e, ended_nz(step));
    }
    e = ended_with_y(e, index);
    value = rd8((uint16_t)(0xB129 + index));
    WR8(OBJ->f3510[slot], value);
    e = then(e, ended_load(value));
    low = RD8(SCROLL->y_lo[1]);
    value = rd8((uint16_t)(0xB129 + index));
    diff = sub_bytes(low, value, true);
    WR8(SCROLL->y_lo[1], diff.value);
    high = RD8(SCROLL->y_hi[1]);
    value = rd8((uint16_t)(0xB133 + index));
    diff = sub_bytes(high, value, diff.carry);
    WR8(SCROLL->y_hi[1], diff.value);
    e = then(e, ended_with_overflow(ended_with_carry(ended_load(diff.value), diff.carry), diff.overflow));
    step = INC8(OBJ->f3568[slot]);
    e = then(e, ended_nz(step));
    e = then(e, l8_compare_y(index, 9));
    if (index != 9) {
        return e;
    }
    WR8(OBJ->f3568[slot], 0);
    return e;
}

void abi_StepVerticalScrollTable(void)
{
    abi_ended(StepVerticalScrollTable(cpu.x));
}

/*
 * @routine 025B:068 CountdownThenStartScript80
 * @run 70:B068
 * WHAT IT DOES: Sets fixed fields (OBJ_3510 slot 0 $10, the scroll bytes ScrollXLo+1 0 and ScrollXHi+1 1, a byte at $3BF0 to 2); counts OBJ_34B8 down; at $3F starts script $80 as kind 2 in slot 5, at 0 sets the sub-step 3 and zField8C $83.
 * HOW: Stores, a memory decrement, a compare, StartScriptSlotKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
 */
Ended CountdownThenStartScript80(uint8_t slot)
{
    uint8_t timer, now;
    Ended e;

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
        return ended_load(0x83);
    }
    now = RD8(OBJ->f34b8[slot]);
    e = ended_compare(now, 0x3F);
    if (now != 0x3F) {
        return e;
    }
    push8(slot);
    jsr_push(0xB094);
    StartScriptSlotKind2(0x80, 5);
    jsr_pop();
    slot = pull8();
    return then(e, ended_with_y(ended_with_a(ended_nz(slot), 0x80), 0x80));
}

void abi_CountdownThenStartScript80(void)
{
    abi_ended(CountdownThenStartScript80(cpu.x));
}

/*
 * @routine 025B:148 LoadRecordPlaceByKind
 * @run 70:B148
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 6 and the frame from a table by OBJ_3040; when the object's flags are 0 it is placed at X $0120 with a Y from a second table, and its flags take the low nibble 6.
 * HOW: LoadObjectRecord, two table reads indexed by OBJ_3040, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordPlaceByKind(uint8_t slot)
{
    uint8_t kind, value, flags;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB150, &first);

    if (first != 0) {
        return e;
    }
    WR8(OBJ->sprite_set[slot], 6);
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_with_y(ended_load(6), kind));
    e = then(e, ended_nz(kind));
    value = rd8((uint16_t)(0xB17E + kind));
    WR8(OBJ->frame[slot], value);
    flags = RD8(OBJ->flags[slot]);
    e = then(e, ended_load(flags));
    if (flags != 0) {
        return e;
    }
    WR8(OBJ->x_lo[slot], 0x20);
    WR8(OBJ->x_hi[slot], 0x01);
    value = rd8((uint16_t)(0xB180 + kind));
    WR8(OBJ->y_lo[slot], value);
    flags = RD8(OBJ->flags[slot]);
    value = (uint8_t)((flags & 0xF0) | 0x06);
    WR8(OBJ->flags[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadRecordPlaceByKind(void)
{
    abi_ended(LoadRecordPlaceByKind(cpu.x));
}

/*
 * @routine 025B:182 LoadRecordAtPlayerPlace
 * @run 70:B182
 * WHAT IT DOES: In state 0 loads the record and takes slot 0's OBJ_X_LO and OBJ_Y_LO; the flags take the low nibble 1.
 * HOW: LoadObjectRecord, two copies, an AND and an ORA.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended LoadRecordAtPlayerPlace(uint8_t slot)
{
    uint8_t value, flags;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB18A, &first);

    if (first != 0) {
        return e;
    }
    value = RD8(OBJ->x_lo[0]);
    WR8(OBJ->x_lo[slot], value);
    value = RD8(OBJ->y_lo[0]);
    WR8(OBJ->y_lo[slot], value);
    flags = RD8(OBJ->flags[slot]);
    value = (uint8_t)((flags & 0xF0) | 0x01);
    WR8(OBJ->flags[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadRecordAtPlayerPlace(void)
{
    abi_ended(LoadRecordAtPlayerPlace(cpu.x));
}

/*
 * @routine 025B:1A1 LoadRecordThenSinkByF3520
 * @run 70:B1A1
 * WHAT IT DOES: In state 0 loads the record, sets the frame 6 and moves the object one left; then adds OBJ_3510 of slot $10 to its OBJ_Y_LO.
 * HOW: LoadObjectRecord, a memory decrement, an add.
 * WAITS: no.
 * Entry: X object. Exit: the add's A, N, Z, C and V; Y 11 when the record was loaded.
 */
Ended LoadRecordThenSinkByF3520(uint8_t slot)
{
    uint8_t fall, y;
    Sum sum;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB1A9, &first);

    if (first == 0) {
        WR8(OBJ->frame[slot], 6);
        e = then(e, ended_load(6));
        e = then(e, ended_nz(DEC8(OBJ->x_hi[slot])));
    }
    fall = RD8(OBJ->f3510[0x10]);
    y = RD8(OBJ->y_lo[slot]);
    sum = add_bytes(fall, y, false);
    WR8(OBJ->y_lo[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_LoadRecordThenSinkByF3520(void)
{
    abi_ended(LoadRecordThenSinkByF3520(cpu.x));
}

/*
 * @routine 025B:1BC LoadRecordThenStepListByKind
 * @run 70:B1BC
 * WHAT IT DOES: In state 0 loads the record and moves the object one left; then steps an animation list picked by OBJ_3040 (two bytes an entry) and, from the table's second half when slot $10's state is below 4 and OBJ_34B8 is 0.
 * HOW: LoadObjectRecord, a decrement, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step.
 */
Ended LoadRecordThenStepListByKind(uint8_t slot, uint8_t y_entry)
{
    uint8_t kind, doubled, timer, state, lo, hi, y = y_entry;
    FrameStep step;
    Sum sum;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB1C4, &first);

    if (first == 0) {
        e = then(e, ended_nz(DEC8(OBJ->x_hi[slot])));
    }
    kind = RD8(OBJ->f3040[slot]);
    doubled = (uint8_t)(kind << 1);
    e = then(e, ended_with_carry(ended_load(doubled), kind >= 0x80));
    y = doubled;
    e = ended_with_y(e, y);
    timer = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(timer));
    if (timer == 0) {
        state = RD8(OBJ->state[0x10]);
        e = then(e, ended_compare(state, 4));
        if (state >= 4) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            e = then(e, ended_nz(INC8(OBJ->f34b8[slot])));
        } else {
            e = then(e, ended_load(y));
            sum = add_bytes(y, 4, false);
            y = sum.value;
            e = ended_with_y(then(e, ended_of_sum(sum)), y);
        }
    }
    lo = rd8((uint16_t)(0xB1F5 + y));
    WR8(ZP->temp10, lo);
    e = then(e, ended_load(lo));
    hi = rd8((uint16_t)(0xB1F6 + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_LoadRecordThenStepListByKind(void)
{
    abi_ended(LoadRecordThenStepListByKind(cpu.x, cpu.y));
}

/*
 * @routine 025B:229 LoadRecordSinkThenStepListByKind
 * @run 70:B229
 * WHAT IT DOES: In state 0 loads the record and moves the object one left; then adds OBJ_3510 of slot $10 to its OBJ_Y_LO and steps the animation list picked by OBJ_3040 from the table at $B250.
 * HOW: LoadObjectRecord, a decrement, an add, table reads, a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step.
 */
Ended LoadRecordSinkThenStepListByKind(uint8_t slot, uint8_t y_entry)
{
    uint8_t fall, ypos, kind, doubled, lo, hi, y = y_entry;
    FrameStep step;
    Sum sum;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB231, &first);

    if (first == 0) {
        e = then(e, ended_nz(DEC8(OBJ->x_hi[slot])));
    }
    fall = RD8(OBJ->f3510[0x10]);
    ypos = RD8(OBJ->y_lo[slot]);
    sum = add_bytes(fall, ypos, false);
    WR8(OBJ->y_lo[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    kind = RD8(OBJ->f3040[slot]);
    doubled = (uint8_t)(kind << 1);
    e = then(e, ended_with_carry(ended_load(doubled), kind >= 0x80));
    y = doubled;
    e = ended_with_y(e, y);
    lo = rd8((uint16_t)(0xB250 + y));
    WR8(ZP->temp10, lo);
    e = then(e, ended_load(lo));
    hi = rd8((uint16_t)(0xB251 + y));
    WR8(ZP->temp11, hi);
    e = then(e, ended_load(hi));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_LoadRecordSinkThenStepListByKind(void)
{
    abi_ended(LoadRecordSinkThenStepListByKind(cpu.x, cpu.y));
}

/*
 * @routine 025B:266 StateMachineWithSoundAndList
 * @run 70:B266
 * WHAT IT DOES: An object of several phases. State 0 loads the record. State 1 waits on the state of slot $10 (3 or 6): it steps the animation list at $B2FA or $B2FF, plays sound $72 when the scroll byte ScrollXLo+1 is clear, and at state 6 goes on. State 2 counts OBJ_3510 down and goes on. Later states fall, and once off screen to the left start a list channel and clear the object.
 * HOW: Compares of OBJ_27A8 and slot $10's state, SoundCommand, StepObjectFrames, StartListChannelKind2 between a PHX and a PLX, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StateMachineWithSoundAndList(uint8_t slot, uint8_t y_entry)
{
    enum { LIST_FAST, SOUND_THEN_LIST_SLOW, LIST_SLOW, ADVANCE } act;
    uint8_t state, other, timer, scroll, flags, x, hi, y = y_entry;
    FrameStep step;
    SoundEnd r = l8_sound_none(ended_kept());
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB26E, &first);

    if (first == 0) {
        r.end = then(e, ended_nz(DEC8(OBJ->x_hi[slot])));
        return r;
    }
    state = first;
    e = ended_load((uint8_t)(state - 1));
    if (state == 1) {
        other = RD8(OBJ->state[0x10]);
        e = ended_compare(other, 3);
        if (other == 3) {
            timer = RD8(OBJ->f34b8[slot]);
            e = then(e, ended_load(timer));
            if (timer != 0) {
                act = LIST_SLOW;
            } else {
                scroll = RD8(SCROLL->x_lo[1]);
                e = then(e, ended_load((uint8_t)(scroll & 0x7F)));
                act = ((scroll & 0x7F) == 0) ? SOUND_THEN_LIST_SLOW : LIST_FAST;
            }
        } else {
            e = ended_compare(other, 6);
            if (other == 6) {
                act = ADVANCE;
            } else {
                timer = RD8(OBJ->f34b8[slot]);
                e = then(e, ended_load(timer));
                act = (timer != 0) ? LIST_SLOW : LIST_FAST;
            }
        }
        if (act == ADVANCE) {
            WR8(OBJ->frame[slot], 0x1C);
            e = then(e, ended_load(0x1C));
            e = then(e, ended_nz(INC8(OBJ->state[slot])));
            WR8(OBJ->f3510[slot], 0x0A);
            r.end = then(e, ended_load(0x0A));
            return r;
        }
        if (act == LIST_FAST) {
            WR8(ZP->temp11, 0xB2);
            WR8(ZP->temp10, 0xFA);
            e = then(e, ended_load(0xFA));
            step = StepObjectFrames(slot, &y);
            r.end = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
            return r;
        }
        if (act == SOUND_THEN_LIST_SLOW) {
            r.sounded = true;
            r.command = 0x72;
            r.before = then(e, ended_load(0x72));
            jsr_push(0xB2A3);
            r.call = SoundCommand(0x72, slot, y);
            jsr_pop();
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            e = ended_nz(INC8(OBJ->f34b8[slot]));
        }
        WR8(ZP->temp11, 0xB2);
        WR8(ZP->temp10, 0xFF);
        e = then(e, ended_load(0xFF));
        jsr_push(0xB2B7);
        step = StepObjectFrames(slot, &y);
        jsr_pop();
        e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
        if (step == FRAME_LIST_ENDED) {
            WR8(OBJ->f34b8[slot], 0);
        }
        r.end = e;
        return r;
    }
    e = then(e, ended_load((uint8_t)(state - 2)));
    if (state == 2) {
        timer = DEC8(OBJ->f3510[slot]);
        e = then(e, ended_nz(timer));
        if (timer == 0) {
            flags = RD8(OBJ->flags[slot]);
            flags = (uint8_t)((flags & 0xF0) | 0x05);
            WR8(OBJ->flags[slot], flags);
            e = then(e, ended_load(flags));
            e = then(e, ended_nz(INC8(OBJ->state[slot])));
        }
        r.end = e;
        return r;
    }
    e = then(e, ended_nz(INC8(OBJ->y_lo[slot])));
    x = RD8(OBJ->x_lo[slot]);
    e = then(e, ended_load(x));
    if ((x & 0x80) != 0) {
        r.end = e;
        return r;
    }
    hi = RD8(OBJ->x_hi[slot]);
    e = then(e, ended_load(hi));
    if ((hi & 0x80) == 0) {
        r.end = e;
        return r;
    }
    push8(slot);
    jsr_push(0xB2F6);
    StartListChannelKind2(0x82, 5);
    jsr_pop();
    slot = pull8();
    ClearObject(slot);
    r.end = then(e, ended_with_a(ended_nz(slot), 2));
    return r;
}

void abi_StateMachineWithSoundAndList(void)
{
    abi_sound_end(StateMachineWithSoundAndList(cpu.x, cpu.y));
}

/*
 * @routine 025B:308 DriftRecordThenMoveWithJitter
 * @run 70:B308
 * WHAT IT DOES: In state 0 loads the record and sets the speed X -$18, Y $0C. When slot $10's state is 4 or more and OBJ_3040 is 0 it waits one round, shows frame $1D, then moves by its speed, nudging X and Y back by random amounts of up to 15 when they are off the edge.
 * HOW: LoadObjectRecord, ApplyVelocity, NextRandom, adds and a subtract with a zero page temporary.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended DriftRecordThenMoveWithJitter(uint8_t slot)
{
    uint8_t state, kind, timer, x, hi, y, mask, temp;
    RandomResult random;
    AxisMove move;
    Sum sum;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB310, &first);

    if (first == 0) {
        WR8(OBJ->vx_hi[slot], 0xE8);
        WR8(OBJ->vy_hi[slot], 0x0C);
        return then(e, ended_load(0x0C));
    }
    state = RD8(OBJ->state[0x10]);
    e = ended_compare(state, 4);
    if (state < 4) {
        return e;
    }
    kind = RD8(OBJ->f3040[slot]);
    e = then(e, ended_load(kind));
    if (kind != 0) {
        return e;
    }
    timer = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(timer));
    if (timer == 0) {
        e = then(e, ended_nz(INC8(OBJ->f34b8[slot])));
        WR8(OBJ->frame[slot], 0x1D);
        return then(e, ended_load(0x1D));
    }
    jsr_push(0xB338);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    x = RD8(OBJ->x_lo[slot]);
    e = then(e, ended_load(x));
    if ((x & 0x80) != 0) {
        hi = RD8(OBJ->x_hi[slot]);
        e = then(e, ended_load(hi));
        if ((hi & 0x80) != 0) {
            e = then(e, ended_nz(INC8(OBJ->x_hi[slot])));
            jsr_push(0xB348);
            random = NextRandom();
            jsr_pop();
            e = then(e, ended_of_random(random));
            mask = random.value & 0x0F;
            e = then(e, ended_load(mask));
            x = RD8(OBJ->x_lo[slot]);
            sum = add_bytes(mask, x, false);
            WR8(OBJ->x_lo[slot], sum.value);
            e = then(e, ended_of_sum(sum));
        }
    }
    y = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_load(y));
    if ((y & 0x80) != 0) {
        return e;
    }
    hi = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(hi));
    if (hi == 0) {
        return e;
    }
    e = then(e, ended_nz(DEC8(OBJ->y_hi[slot])));
    jsr_push(0xB361);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    mask = random.value & 0x0F;
    e = then(e, ended_load(mask));
    WR8(ZP->temp00, mask);
    y = RD8(OBJ->y_lo[slot]);
    temp = RD8(ZP->temp00);
    sum = sub_bytes(y, temp, true);
    WR8(OBJ->y_lo[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_DriftRecordThenMoveWithJitter(void)
{
    abi_ended(DriftRecordThenMoveWithJitter(cpu.x));
}

/*
 * @routine 025B:487 SwingByCosineFromState0B
 * @run 70:B487
 * WHAT IT DOES: From state $0B on, sets the Y speed from the cosine of OBJ_3618 scaled by $0040, moves the object in Y and turns OBJ_3618 on by 2.
 * HOW: ScaleByCosine, two copies of the product, ApplyVelocityY, an add.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended SwingByCosineFromState0B(uint8_t slot)
{
    uint8_t state, angle, value;
    ScaleResult scale;
    AxisMove move;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 0x0B);
    if (state < 0x0B) {
        return e;
    }
    WR8(ZP->temp00, 0x40);
    WR8(ZP->temp01, 0);
    angle = RD8(OBJ->f3618[slot]);
    e = then(e, ended_load(angle));
    jsr_push(0xB49C);
    scale = ScaleByCosine(angle);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    value = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], value);
    e = then(e, ended_load(value));
    value = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    jsr_push(0xB4A9);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    angle = RD8(OBJ->f3618[slot]);
    sum = add_bytes(angle, 2, false);
    WR8(OBJ->f3618[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_SwingByCosineFromState0B(void)
{
    abi_ended(SwingByCosineFromState0B(cpu.x));
}

/*
 * @routine 025B:4B3 StartRandomScriptWhenFlagged
 * @run 70:B4B3
 * WHAT IT DOES: When OBJ_3720 is set and OBJ_3568's low five bits are 0, with a chance of three in eight starts a script from the table at $B4E9 as kind 1 in slot 5.
 * HOW: Two NextRandom calls and StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StartRandomScriptWhenFlagged(uint8_t slot)
{
    uint8_t flag, timer, low, pick, script;
    RandomResult random;
    Ended e;

    flag = RD8(OBJ->f3720[slot]);
    e = ended_load(flag);
    if (flag == 0) {
        return e;
    }
    timer = RD8(OBJ->f3568[slot]);
    low = timer & 0x1F;
    e = then(e, ended_load(low));
    if (low != 0) {
        return e;
    }
    jsr_push(0xB4C2);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x07;
    e = then(e, ended_compare(pick, 3));
    if (pick >= 3) {
        return e;
    }
    push8(slot);
    jsr_push(0xB4CC);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = random.value & 0x03;
    script = rd8((uint16_t)(0xB4E9 + pick));
    jsr_push(0xB4D7);
    StartScriptSlotKind1(script, 5);
    jsr_pop();
    slot = pull8();
    return then(e, ended_with_y(ended_with_a(ended_nz(slot), script), script));
}

void abi_StartRandomScriptWhenFlagged(void)
{
    abi_ended(StartRandomScriptWhenFlagged(cpu.x));
}

/*
 * @routine 025B:4ED PushPlayerWhenFlagged
 * @run 70:B4ED
 * WHAT IT DOES: When OBJ_3778 is set, pushes the player: zPlayerPush $10 and zPushX $FF60.
 * HOW: A load and three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $60 when set; otherwise N and Z from OBJ_3778.
 */
Ended PushPlayerWhenFlagged(uint8_t slot)
{
    uint8_t flag;

    flag = RD8(OBJ->f3778[slot]);
    if (flag == 0) {
        return ended_load(flag);
    }
    WR8(ZP->player_push, 0x10);
    WR8(ZP->push_x.hi, 0xFF);
    WR8(ZP->push_x.lo, 0x60);
    return ended_load(0x60);
}

void abi_PushPlayerWhenFlagged(void)
{
    abi_ended(PushPlayerWhenFlagged(cpu.x));
}

/*
 * @routine 025B:4FF HitSoundThenSpawnPieces
 * @run 70:B4FF
 * WHAT IT DOES: When the object's hits taken reach 5, plays sound $75 and sets OBJ_3828 to $10. While OBJ_3828 runs it counts down and, whenever the wave counter's low three bits are 0, starts an object of type $2F in a free slot of $20 to $3F at the object's place with OBJ_34B8 1.
 * HOW: SoundCommand, FindFreeSlotFrom20, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd HitSoundThenSpawnPieces(uint8_t slot, uint8_t y_entry)
{
    uint8_t hits, timer, wave, found_slot, last, placed;
    bool found;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hits, 5);
    if (hits == 5) {
        r.sounded = true;
        r.command = 0x75;
        r.before = then(e, ended_load(0x75));
        jsr_push(0xB50B);
        r.call = SoundCommand(0x75, slot, y_entry);
        jsr_pop();
        WR8(OBJ->f3828[slot], 0x10);
        e = ended_load(0x10);
    }
    timer = RD8(OBJ->f3828[slot]);
    e = then(e, ended_load(timer));
    if (timer == 0) {
        r.end = e;
        return r;
    }
    e = then(e, ended_nz(DEC8(OBJ->f3828[slot])));
    wave = RD8(ZP->wave_counter) & 0x07;
    e = then(e, ended_load(wave));
    if (wave != 0) {
        r.end = e;
        return r;
    }
    jsr_push(0xB521);
    found_slot = FindFreeSlotFrom20(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xB528);
    placed = StartObjectAtPlace(0x2F, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(OBJ->f34b8[found_slot], 1);
    r.end = then(e, ended_load(1));
    return r;
}

void abi_HitSoundThenSpawnPieces(void)
{
    abi_sound_end(HitSoundThenSpawnPieces(cpu.x, cpu.y));
}

/*
 * @routine 025B:52E EnterState0BWhenFlagged
 * @run 70:B52E
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is below $0B, sets the state $0B and OBJ_2800 0.
 * HOW: Two tests, two stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The stores ran in no script; the draft stops there.
 */
Ended EnterState0BWhenFlagged(uint8_t slot)
{
    uint8_t flags, state;
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) == 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 0x0B));
    if (state >= 0x0B) {
        return e;
    }
    WR8(OBJ->state[slot], 0x0B);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(0x0B));
}

void abi_EnterState0BWhenFlagged(void)
{
    abi_ended(EnterState0BWhenFlagged(cpu.x));
}

/*
 * @routine 025B:56F StartPieceAtTopLeft
 * @run 70:B56F
 * WHAT IT DOES: Loads the object's record, clears slot $18, starts an object of type $28 there at the object's place, moves the new object's source to X $003C, Y $0030 with X speed $80 (OBJ_VX_LO), OBJ_34B8 4 and OBJ_3720 1.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 1, Y $18, C from the record.
 */
Ended StartPieceAtTopLeft(uint8_t slot)
{
    uint8_t last, nz, placed;
    bool carry;
    Ended e;

    jsr_push(0xB572);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB577);
    ClearObjectInY(0x18);
    jsr_pop();
    e = then(e, ended_with_y(ended_nz(0x18), 0x18));
    jsr_push(0xB57C);
    placed = StartObjectAtPlace(0x28, slot, 0x18);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0x3C);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x30);
    WR8(OBJ->vx_lo[slot], 0x80);
    WR8(OBJ->f34b8[slot], 0x04);
    WR8(OBJ->f3720[slot], 0x01);
    return then(e, ended_load(1));
}

void abi_StartPieceAtTopLeft(void)
{
    abi_ended(StartPieceAtTopLeft(cpu.x));
}

/*
 * @routine 025B:59C CountDownThenStartScripts
 * @run 70:B59C
 * WHAT IT DOES: Counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero starts scripts $84 (slot 5) and $86 (slot 4) as kind 1, clears OBJ_3720, sets OBJ_3510 $60 and goes to the next state.
 * HOW: A 16-bit subtract in memory, an ORA, StartScriptSlotKind1 twice between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended CountDownThenStartScripts(uint8_t slot)
{
    uint8_t low, high, again, joined, state;
    Sum diff;
    Ended e;

    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], diff.value);
    again = RD8(OBJ->f3510[slot]);
    joined = (uint8_t)(diff.value | again);
    e = then(ended_of_sum(diff), ended_load(joined));
    if (joined != 0) {
        return e;
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
    state = INC8(OBJ->state[slot]);
    e = then(e, ended_with_y(ended_load(0x60), 0x86));
    return then(e, ended_nz(state));
}

void abi_CountDownThenStartScripts(void)
{
    abi_ended(CountDownThenStartScripts(cpu.x));
}

/*
 * @routine 025B:5CE CountDownWithSound77
 * @run 70:B5CE
 * WHAT IT DOES: Counts OBJ_3510 down; at $35 plays sound $77; at zero sets OBJ_3510 $80 and goes to the next state.
 * HOW: A memory decrement, a compare, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
SoundEnd CountDownWithSound77(uint8_t slot, uint8_t y_entry)
{
    uint8_t timer, now;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    timer = DEC8(OBJ->f3510[slot]);
    if (timer == 0) {
        WR8(OBJ->f3510[slot], 0x80);
        r.end = then(then(ended_nz(timer), ended_load(0x80)), ended_nz(INC8(OBJ->state[slot])));
        return r;
    }
    now = RD8(OBJ->f3510[slot]);
    e = ended_compare(now, 0x35);
    if (now != 0x35) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x77;
    r.before = then(e, ended_load(0x77));
    jsr_push(0xB5DF);
    r.call = SoundCommand(0x77, slot, y_entry);
    jsr_pop();
    return r;
}

void abi_CountDownWithSound77(void)
{
    abi_sound_end(CountDownWithSound77(cpu.x, cpu.y));
}

/*
 * @routine 025B:617 StartScriptAtTimerMarks
 * @run 70:B617
 * WHAT IT DOES: When OBJ_3510 is 0 sets OBJ_3720. Otherwise counts it down, and when it equals one of the eight marks at $B642 starts the script of the same index in the table at $B64A as kind 1 in slot 5.
 * HOW: A loop over the marks, StartScriptSlotKind1 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StartScriptAtTimerMarks(uint8_t slot)
{
    uint8_t timer, mark, y, script;
    Ended e;

    timer = RD8(OBJ->f3510[slot]);
    if (timer == 0) {
        WR8(OBJ->f3720[slot], 1);
        return ended_load(1);
    }
    e = ended_load(timer);
    y = 0;
    for (;;) {
        timer = RD8(OBJ->f3510[slot]);
        mark = rd8((uint16_t)(0xB642 + y));
        e = ended_with_y(ended_compare(timer, mark), y);
        if (timer == mark) {
            break;
        }
        y++;
        e = then(e, ended_with_y(ended_nz(y), y));
        e = then(e, l8_compare_y(y, 8));
        if (y >= 8) {
            return then(e, ended_nz(DEC8(OBJ->f3510[slot])));
        }
    }
    push8(slot);
    script = rd8((uint16_t)(0xB64A + y));
    jsr_push(0xB63D);
    StartScriptSlotKind1(script, 5);
    jsr_pop();
    slot = pull8();
    return then(then(e, ended_with_y(ended_with_a(ended_nz(slot), script), script)), ended_nz(DEC8(OBJ->f3510[slot])));
}

void abi_StartScriptAtTimerMarks(void)
{
    abi_ended(StartScriptAtTimerMarks(cpu.x));
}

/*
 * @routine 025B:652 SpinOutThenLeave
 * @run 70:B652
 * WHAT IT DOES: Moves the object on a circle of radius $0200 by OBJ_35C0 (turning on 4 a frame) while it steps the animation list at $B6A4; when the list ends it starts the next phase: slot $10's state up, OBJ_3778 1, OBJ_3250 2, slot 0's OBJ_3510 2, the byte at $3BF0 2, OBJ_3568 of slot 0 0, OBJ_35C0 $C0, OBJ_3510 $20, OBJ_3670 1 and the next state.
 * HOW: SetVelocityFromAngle, ApplyVelocity, an add, StepObjectFrames and stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
 */
Ended SpinOutThenLeave(uint8_t slot, uint8_t y_entry)
{
    uint8_t angle, y = y_entry, state;
    ScaleResult scale;
    AxisMove move;
    FrameStep step;
    Sum sum;
    Ended e;

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 2);
    angle = RD8(OBJ->f35c0[slot]);
    e = ended_load(angle);
    jsr_push(0xB65E);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    y = 0;
    jsr_push(0xB661);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    angle = RD8(OBJ->f35c0[slot]);
    sum = add_bytes(angle, 4, false);
    WR8(OBJ->f35c0[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    WR8(ZP->temp10, 0xA4);
    WR8(ZP->temp11, 0xB6);
    e = then(e, ended_load(0xB6));
    jsr_push(0xB675);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_LIST_ENDED) {
        return e;
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
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(1)), ended_nz(state));
}

void abi_SpinOutThenLeave(void)
{
    abi_ended(SpinOutThenLeave(cpu.x, cpu.y));
}

/*
 * @routine 025B:6E7 SpinOnCircle
 * @run 70:B6E7
 * WHAT IT DOES: Moves the object on a circle of radius $0180 by OBJ_35C0, turning on 2 a frame; when the angle wraps to 0 it jumps to X $00D5, Y $002F with the fractions $4C and $83.
 * HOW: SetVelocityFromAngle, ApplyVelocity, an add, four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended SpinOnCircle(uint8_t slot)
{
    uint8_t angle;
    ScaleResult scale;
    AxisMove move;
    Sum sum;
    Ended e;

    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->f35c0[slot]);
    e = ended_load(angle);
    jsr_push(0xB6F5);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    jsr_push(0xB6F8);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    angle = RD8(OBJ->f35c0[slot]);
    sum = add_bytes(angle, 2, false);
    WR8(OBJ->f35c0[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    if (sum.value != 0) {
        return e;
    }
    WR8(OBJ->x_lo[slot], 0xD5);
    WR8(OBJ->x_frac[slot], 0x4C);
    WR8(OBJ->y_lo[slot], 0x2F);
    WR8(OBJ->y_frac[slot], 0x83);
    return then(e, ended_load(0x83));
}

void abi_SpinOnCircle(void)
{
    abi_ended(SpinOnCircle(cpu.x));
}

/*
 * @routine 025B:761 StepListThenBurstAtFrame18
 * @run 70:B761
 * WHAT IT DOES: Steps the animation list at $B79A. When the graphic changes and the frame is $18, plays sound $42, clears slot $50 and starts an object of type $29 there at the object's place. When the list ends it sets OBJ_34B8 1, OBJ_3510 $80, OBJ_3670 3 and goes to the next state.
 * HOW: StepObjectFrames, SoundCommand, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListThenBurstAtFrame18(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, frame, placed;
    FrameStep step;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    WR8(ZP->temp10, 0x9A);
    WR8(ZP->temp11, 0xB7);
    jsr_push(0xB76C);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->f34b8[slot], 1);
        WR8(OBJ->f3510[slot], 0x80);
        WR8(OBJ->f3670[slot], 3);
        r.end = then(then(e, ended_load(3)), ended_nz(INC8(OBJ->state[slot])));
        return r;
    }
    if (step != FRAME_CHANGED) {
        r.end = e;
        return r;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_compare(frame, 0x18));
    if (frame != 0x18) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x42;
    r.before = then(e, ended_load(0x42));
    jsr_push(0xB77C);
    r.call = SoundCommand(0x42, slot, y);
    jsr_pop();
    jsr_push(0xB781);
    ClearObjectInY(0x50);
    jsr_pop();
    jsr_push(0xB786);
    placed = StartObjectAtPlace(0x29, slot, 0x50);
    jsr_pop();
    r.end = then(ended_with_y(ended_nz(0x50), 0x50), ended_load(placed));
    return r;
}

void abi_StepListThenBurstAtFrame18(void)
{
    abi_sound_end(StepListThenBurstAtFrame18(cpu.x, cpu.y));
}

/*
 * @routine 025B:7F5 HomeOnPointThenRingAndSound
 * @run 70:B7F5
 * WHAT IT DOES: Steps the animation list at $B6DA while it flies toward the point X $00E0, Y $0030 on a circle of radius $0100; once on it (within 3) plays sound $73, sets OBJ_3670 4 and OBJ_3510 $B0, starts list $83 on channel 5, restarts the animation, and goes to state $0B with OBJ_2800 2.
 * HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, two range tests, SoundCommand, StartListChannelKind2.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd HomeOnPointThenRingAndSound(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, hi, lo, angle;
    FrameStep step;
    AngleExit toward;
    ScaleResult scale;
    AxisMove move;
    Sum diff, sum;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    WR8(ZP->temp10, 0xDA);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB800);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    WR8(ZP->temp09, 0);
    WR8(ZP->temp08, 0xE0);
    WR8(ZP->temp0b, 0);
    WR8(ZP->temp0a, 0x30);
    jsr_push(0xB80F);
    toward = AngleToPoint(slot);
    jsr_pop();
    e = then(e, l8_of_angle(toward));
    y = toward.y;
    push8(toward.a);
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    angle = pull8();
    e = then(e, ended_load(angle));
    jsr_push(0xB81A);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    e = then(e, l8_of_scale(scale));
    y = 0;
    jsr_push(0xB81D);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    hi = RD8(OBJ->x_hi[slot]);
    lo = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load((uint8_t)(hi | lo)));
    if ((hi | lo) != 0) {
        r.end = e;
        return r;
    }
    lo = RD8(OBJ->x_lo[slot]);
    diff = sub_bytes(lo, 0xE0, true);
    sum = add_bytes(diff.value, 4, false);
    e = then(e, ended_compare(sum.value, 8));
    if (sum.value >= 8) {
        r.end = e;
        return r;
    }
    lo = RD8(OBJ->y_lo[slot]);
    diff = sub_bytes(lo, 0x30, true);
    sum = add_bytes(diff.value, 4, false);
    e = then(e, ended_compare(sum.value, 8));
    if (sum.value >= 8) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x73;
    r.before = then(e, ended_load(0x73));
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
    WR8(OBJ->f2800[slot], 2);
    r.end = ended_load(2);
    return r;
}

void abi_HomeOnPointThenRingAndSound(void)
{
    abi_sound_end(HomeOnPointThenRingAndSound(cpu.x, cpu.y));
}
