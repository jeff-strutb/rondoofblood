/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025A (enemy kinds not yet identified), by routine. Bodies return how the
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
 * @routine 025A:6BA LoadRecordOrCapPlayerX
 * @run 70:AEBA
 * WHAT IT DOES: In state 0 loads the object's record; later it keeps the player's OBJ_X_LO below $C0.
 * HOW: A test of the state, a tail jump into LoadObjectRecord, or a compare and a store.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord in state 0; otherwise the compare, A $BF when it was capped.
 */
Ended LoadRecordOrCapPlayerX(uint8_t slot)
{
    uint8_t state, x, last, nz;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    x = RD8(OBJ->x_lo[0]);
    e = ended_compare(x, 0xC0);
    if (x >= 0xC0) {
        WR8(OBJ->x_lo[0], 0xBF);
        e = then(e, ended_load(0xBF));
    }
    return e;
}

void abi_LoadRecordOrCapPlayerX(void)
{
    abi_ended(LoadRecordOrCapPlayerX(cpu.x));
}

/*
 * @routine 025A:6CF StartAtX120OrApplyVelocity
 * @run 70:AECF
 * WHAT IT DOES: In state 0 loads the object's record and sets its frame $6F, X position $0120, Y position $0038 and X speed -$0020; later it moves by its speed.
 * HOW: A test of the state, a tail jump into ApplyVelocity, or LoadObjectRecord and eight stores.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, or A $E0 (N set, Z clear) with C and Y from LoadObjectRecord.
 */
Ended StartAtX120OrApplyVelocity(uint8_t slot)
{
    uint8_t state, last, nz;
    bool carry;
    AxisMove move;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        move = ApplyVelocity(slot);
        return ended_of_axis_move(move);
    }
    jsr_push(0xAEDA);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->frame[slot], 0x6F);
    WR8(OBJ->x_hi[slot], 0x01);
    WR8(OBJ->x_lo[slot], 0x20);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x38);
    WR8(OBJ->vx_hi[slot], 0xFF);
    WR8(OBJ->vx_lo[slot], 0xE0);
    return then(e, ended_load(0xE0));
}

void abi_StartAtX120OrApplyVelocity(void)
{
    abi_ended(StartAtX120OrApplyVelocity(cpu.x));
}

/*
 * @routine 025A:722 SetBossFixedFields
 * @run 70:AF22
 * WHAT IT DOES: Sets the fixed fields of slot 0: OBJ_3510 $10, OBJ_FRAME 0 and OBJ_3568 $40.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $40, N and Z from it.
 */
Ended SetBossFixedFields(void)
{
    WR8(OBJ->f3510[0], 0x10);
    WR8(OBJ->frame[0], 0);
    WR8(OBJ->f3568[0], 0x40);
    return ended_load(0x40);
}

void abi_SetBossFixedFields(void)
{
    abi_ended(SetBossFixedFields());
}

/*
 * @routine 025A:776 CountdownThenNextState
 * @run 70:AF76
 * WHAT IT DOES: Sets the fixed fields; counts the object's OBJ_34B8 down; at zero clears it and goes to the next state; before that it may spawn a piece.
 * HOW: SetBossFixedFields, a memory decrement, then SetTimerThenNextState with A 0 or the fall into SpawnPiece21Mask1F.
 * WAITS: no.
 * Entry: X object. Exit: as the routine it ends in.
 */
Ended CountdownThenNextState(uint8_t slot)
{
    uint8_t timer;
    Ended e;

    jsr_push(0xAF79);
    e = SetBossFixedFields();
    jsr_pop();
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return then(e, SpawnPiece21Mask1F(slot));
    }
    e = ended_with_a(e, 0);
    return then(e, SetTimerThenNextState(slot, 0));
}

void abi_CountdownThenNextState(void)
{
    abi_ended(CountdownThenNextState(cpu.x));
}

/*
 * @routine 025A:77F SetTimerThenNextState
 * @run 70:AF7F
 * WHAT IT DOES: Sets the object's OBJ_34B8 to A and goes to the next state.
 * HOW: A store and an INC.
 * WAITS: no.
 * Entry: A the timer, X object. Exit: the new state's N and Z.
 */
Ended SetTimerThenNextState(uint8_t slot, uint8_t timer)
{
    uint8_t state;

    WR8(OBJ->f34b8[slot], timer);
    state = INC8(OBJ->state[slot]);
    return ended_nz(state);
}

void abi_SetTimerThenNextState(void)
{
    abi_ended(SetTimerThenNextState(cpu.x, cpu.a));
}

/*
 * @routine 025A:786 SpawnPiece21Mask1F
 * @run 70:AF86
 * WHAT IT DOES: SpawnPiece21UnlessMasked with the mask $1F.
 * HOW: A load, running into the next routine.
 * WAITS: no.
 * Entry: X object. Exit: as SpawnPiece21UnlessMasked.
 */
Ended SpawnPiece21Mask1F(uint8_t slot)
{
    return then(ended_load(0x1F), SpawnPiece21UnlessMasked(slot, 0x1F));
}

void abi_SpawnPiece21Mask1F(void)
{
    abi_ended(SpawnPiece21Mask1F(cpu.x));
}

/*
 * @routine 025A:788 SpawnPiece21UnlessMasked
 * @run 70:AF88
 * WHAT IT DOES: When A and the scroll byte ScrollXLo+5 have no bit in common, starts an object of type $21 in a free slot of $30 to $3F at the object's place.
 * HOW: An AND, FindFreeSlotFrom30 and a tail jump into StartObjectAtPlace (started in Y even when none was free).
 * WAITS: no.
 * Entry: A the mask, X object. Exit: the AND, or the start's A and N, Z with Y and C from the slot search.
 */
Ended SpawnPiece21UnlessMasked(uint8_t slot, uint8_t mask)
{
    uint8_t scroll, anded, found_slot, last, placed;
    bool found;
    Ended e;

    scroll = RD8(SCROLL->x_lo[5]);
    anded = mask & scroll;
    e = ended_load(anded);
    if (anded != 0) {
        return e;
    }
    jsr_push(0xAF90);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    placed = StartObjectAtPlace(0x21, slot, found_slot);
    return then(e, ended_load(placed));
}

void abi_SpawnPiece21UnlessMasked(void)
{
    abi_ended(SpawnPiece21UnlessMasked(cpu.x, cpu.a));
}

/*
 * @routine 025A:795 CountdownThenSpawnPieces
 * @run 70:AF95
 * WHAT IT DOES: Sets the fixed fields and counts OBJ_34B8 down; at zero sets it to $40 and goes to the next state; before that it may spawn a piece and, once in a while, a second one.
 * HOW: SetBossFixedFields, SetTimerThenNextState or SpawnPiece21UnlessMasked, a compare of the scroll byte, NextRandom, FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction that ran.
 */
Ended CountdownThenSpawnPieces(uint8_t slot)
{
    uint8_t timer, scroll, masked, random, found_slot, last, placed;
    bool found;
    RandomResult r;
    Ended e;

    jsr_push(0xAF98);
    e = SetBossFixedFields();
    jsr_pop();
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    if (timer == 0) {
        return then(e, SetTimerThenNextState(slot, 0x40));
    }
    e = then(e, ended_load(0x7F));
    jsr_push(0xAFA3);
    e = then(e, SpawnPiece21UnlessMasked(slot, 0x7F));
    jsr_pop();
    scroll = RD8(SCROLL->x_lo[5]);
    masked = scroll & 0x7F;
    e = then(e, ended_compare(masked, 0x40));
    if (masked != 0x40) {
        return e;
    }
    jsr_push(0xAFAF);
    r = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(r));
    random = r.value & 0x03;
    e = then(e, ended_load(random));
    if (random != 0) {
        return e;
    }
    jsr_push(0xAFB6);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    jsr_push(0xAFBB);
    placed = StartObjectAtPlace(0x21, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    WR8(OBJ->f3040[found_slot], 1);
    return then(e, ended_load(1));
}

void abi_CountdownThenSpawnPieces(void)
{
    abi_ended(CountdownThenSpawnPieces(cpu.x));
}

/*
 * @routine 025A:7C1 HoldScrollStepThenNextState
 * @run 70:AFC1
 * WHAT IT DOES: Sets the fixed fields, steps the scroll table (StepVerticalScrollTable), clears the scroll step ScrollStepXHi+1, and when the scroll's high byte is $FF and its low byte $80 sets the timer to $80 and goes to the next state; otherwise lowers the step by 2.
 * HOW: SetBossFixedFields, StepVerticalScrollTable, loads and compares of the scroll bytes, a tail jump into SetTimerThenNextState or two memory decrements.
 * WAITS: no.
 * Entry: X object. Exit: the second decrement's N and Z, or as SetTimerThenNextState.
 */
Ended HoldScrollStepThenNextState(uint8_t slot)
{
    uint8_t high, low;
    uint8_t step;
    Ended e;

    jsr_push(0xAFC4);
    e = SetBossFixedFields();
    jsr_pop();
    jsr_push(0xAFC7);
    e = then(e, StepVerticalScrollTable(slot));
    jsr_pop();
    WR8(SCROLLE->step_x_hi[1], 0);
    high = RD8(SCROLL->x_hi[1]);
    e = then(e, ended_load((uint8_t)(high + 1)));
    if ((uint8_t)(high + 1) == 0) {
        low = RD8(SCROLL->x_lo[1]);
        e = then(e, ended_compare(low, 0x80));
        if (low == 0x80) {
            return then(e, SetTimerThenNextState(slot, 0x80));
        }
    }
    step = DEC8(SCROLLE->step_x_hi[1]);
    step = DEC8(SCROLLE->step_x_hi[1]);
    return then(e, ended_nz(step));
}

void abi_HoldScrollStepThenNextState(void)
{
    abi_ended(HoldScrollStepThenNextState(cpu.x));
}
