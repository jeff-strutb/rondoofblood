/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 025A (enemy kinds not yet identified), by routine. Bodies return the arm
 * they ended on and the values behind it; an exit shim per routine builds how the routine's last instructions left A, Y and the flags (Ended,
 * enemy_objects.h) and the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"
#include "front_end.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The arms the routines end on. */
enum { ST11_CAP_LOAD, ST11_CAP_KEPT, ST11_CAP_CAPPED };
enum { ST11_AT120_MOVE, ST11_AT120_START };
enum { ST11_COUNT_SPAWN, ST11_COUNT_NEXT };
enum { ST11_PIECES_NEXT, ST11_PIECES_ONE, ST11_PIECES_NO_RANDOM, ST11_PIECES_TWO };
enum { ST11_HOLD_NEXT, ST11_HOLD_SCROLLING, ST11_HOLD_NOT_80 };
enum { ST11_BOSS_LOAD, ST11_BOSS_C0 };
enum { ST11_FADE_BUSY, ST11_FADE_FALLING, ST11_FADE_NO_CARD, ST11_FADE_MOVED, ST11_FADE_NEXT, ST11_FADE_OUT };

/*
 * @routine 025A:6BA LoadRecordOrCapPlayerX
 * @run 70:AEBA
 * WHAT IT DOES: In state 0 loads the object's record; later it keeps the player's OBJ_X_LO below $C0.
 * HOW: A test of the state, a tail jump into LoadObjectRecord, or a compare and a store.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord in state 0; otherwise the compare, A $BF when it was capped (cap_exit_shim).
 */
L8Arm LoadRecordOrCapPlayerX(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        r.record = l8_record(slot);
        r.arm = ST11_CAP_LOAD;
        return r;
    }
    r.first = RD8(OBJ->x_lo[0]);
    if (r.first >= 0xC0) {
        WR8(OBJ->x_lo[0], 0xBF);
        r.arm = ST11_CAP_CAPPED;
        return r;
    }
    r.arm = ST11_CAP_KEPT;
    return r;
}

/* What LoadRecordOrCapPlayerX leaves in A, Y and the flags, from the arm it ended on. */
static Ended cap_exit_shim(L8Arm r)
{
    Ended e;

    if (r.arm == ST11_CAP_LOAD) {
        return l8_record_ended(r.record);
    }
    e = ended_compare(r.first, 0xC0);
    if (r.arm == ST11_CAP_CAPPED) {
        e = then(e, ended_load(0xBF));
    }
    return e;
}

void abi_LoadRecordOrCapPlayerX(void)
{
    abi_ended(cap_exit_shim(LoadRecordOrCapPlayerX(cpu.x)));
}

/*
 * @routine 025A:6CF StartAtX120OrApplyVelocity
 * @run 70:AECF
 * WHAT IT DOES: In state 0 loads the object's record and sets its frame $6F, X position $0120, Y position $0038 and X speed -$0020; later it moves by its speed.
 * HOW: A test of the state, a tail jump into ApplyVelocity, or LoadObjectRecord and eight stores.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, or A $E0 (N set, Z clear) with C and Y from LoadObjectRecord (start_at_120_exit_shim).
 */
L8Arm StartAtX120OrApplyVelocity(uint8_t slot)
{
    L8Arm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.move = ApplyVelocity(slot);
        r.arm = ST11_AT120_MOVE;
        return r;
    }
    jsr_push(0xAEDA);
    r.record = l8_record(slot);
    jsr_pop();
    WR8(OBJ->frame[slot], 0x6F);
    WR8(OBJ->x_hi[slot], 0x01);
    WR8(OBJ->x_lo[slot], 0x20);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x38);
    WR8(OBJ->vx_hi[slot], 0xFF);
    WR8(OBJ->vx_lo[slot], 0xE0);
    r.arm = ST11_AT120_START;
    return r;
}

/* What StartAtX120OrApplyVelocity leaves in A, Y and the flags, from the arm it ended on. */
static Ended start_at_120_exit_shim(L8Arm r)
{
    if (r.arm == ST11_AT120_MOVE) {
        return ended_of_axis_move(r.move);
    }
    return then(l8_record_ended(r.record), ended_load(0xE0));
}

void abi_StartAtX120OrApplyVelocity(void)
{
    abi_ended(start_at_120_exit_shim(StartAtX120OrApplyVelocity(cpu.x)));
}

/*
 * @routine 025A:722 SetBossFixedFields
 * @run 70:AF22
 * WHAT IT DOES: Sets the fixed fields of slot 0: OBJ_3510 $10, OBJ_FRAME 0 and OBJ_3568 $40.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $40, N and Z from it (l8_boss_fixed_exit_shim).
 */
uint8_t SetBossFixedFields(void)
{
    WR8(OBJ->f3510[0], 0x10);
    WR8(OBJ->frame[0], 0);
    WR8(OBJ->f3568[0], 0x40);
    return 0x40;
}

/* What SetBossFixedFields leaves in A and the flags: the value it loaded. */
Ended l8_boss_fixed_exit_shim(uint8_t value)
{
    return ended_load(value);
}

void abi_SetBossFixedFields(void)
{
    abi_ended(l8_boss_fixed_exit_shim(SetBossFixedFields()));
}

/*
 * @routine 025A:776 CountdownThenNextState
 * @run 70:AF76
 * WHAT IT DOES: Sets the fixed fields; counts the object's OBJ_34B8 down; at zero clears it and goes to the next state; before that it may spawn a piece.
 * HOW: SetBossFixedFields, a memory decrement, then SetTimerThenNextState with A 0 or the fall into SpawnPiece21Mask1F.
 * WAITS: no.
 * Entry: X object. Exit: as the routine it ends in (count_exit_shim).
 */
L8St11CountArm CountdownThenNextState(uint8_t slot)
{
    L8St11CountArm r = { 0 };

    jsr_push(0xAF79);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.spawn = SpawnPiece21Mask1F(slot);
        r.arm = ST11_COUNT_SPAWN;
        return r;
    }
    r.state = SetTimerThenNextState(slot, 0);
    r.arm = ST11_COUNT_NEXT;
    return r;
}

/* What CountdownThenNextState leaves in A, Y and the flags, from the arm it ended on. */
static Ended count_exit_shim(L8St11CountArm r)
{
    Ended e = then(l8_boss_fixed_exit_shim(r.fixed), ended_nz(r.timer));

    if (r.arm == ST11_COUNT_SPAWN) {
        return then(e, l8_spawn_piece21_mask1f_exit_shim(r.spawn));
    }
    e = ended_with_a(e, 0);
    return then(e, l8_next_state_exit_shim(r.state));
}

void abi_CountdownThenNextState(void)
{
    abi_ended(count_exit_shim(CountdownThenNextState(cpu.x)));
}

/*
 * @routine 025A:77F SetTimerThenNextState
 * @run 70:AF7F
 * WHAT IT DOES: Sets the object's OBJ_34B8 to A and goes to the next state.
 * HOW: A store and an INC.
 * WAITS: no.
 * Entry: A the timer, X object. Exit: the new state's N and Z (l8_next_state_exit_shim).
 */
uint8_t SetTimerThenNextState(uint8_t slot, uint8_t timer)
{
    WR8(OBJ->f34b8[slot], timer);
    return INC8(OBJ->state[slot]);
}

/* What SetTimerThenNextState leaves in the flags: N and Z of the new state. */
Ended l8_next_state_exit_shim(uint8_t state)
{
    return ended_nz(state);
}

void abi_SetTimerThenNextState(void)
{
    abi_ended(l8_next_state_exit_shim(SetTimerThenNextState(cpu.x, cpu.a)));
}

/*
 * @routine 025A:786 SpawnPiece21Mask1F
 * @run 70:AF86
 * WHAT IT DOES: SpawnPiece21UnlessMasked with the mask $1F.
 * HOW: A load, running into the next routine.
 * WAITS: no.
 * Entry: X object. Exit: as SpawnPiece21UnlessMasked (l8_spawn_piece21_mask1f_exit_shim).
 */
L8St11Spawn SpawnPiece21Mask1F(uint8_t slot)
{
    return SpawnPiece21UnlessMasked(slot, 0x1F);
}

/* What SpawnPiece21UnlessMasked leaves in A, Y and the flags, from the values it read and made. */
static Ended l8_spawn_piece21_exit_shim(L8St11Spawn r)
{
    Ended e = ended_load(r.anded);

    if (!r.started) {
        return e;
    }
    e = then(e, l8_slot_ended(r.found));
    return then(e, ended_load(r.placed));
}

/* What SpawnPiece21Mask1F leaves: the load of the mask, then the exit of SpawnPiece21UnlessMasked. */
Ended l8_spawn_piece21_mask1f_exit_shim(L8St11Spawn r)
{
    return then(ended_load(0x1F), l8_spawn_piece21_exit_shim(r));
}

void abi_SpawnPiece21Mask1F(void)
{
    abi_ended(l8_spawn_piece21_mask1f_exit_shim(SpawnPiece21Mask1F(cpu.x)));
}

/*
 * @routine 025A:788 SpawnPiece21UnlessMasked
 * @run 70:AF88
 * WHAT IT DOES: When A and the scroll byte ScrollXLo+5 have no bit in common, starts an object of type $21 in a free slot of $30 to $3F at the object's place.
 * HOW: An AND, FindFreeSlotFrom30 and a tail jump into StartObjectAtPlace (started in Y even when none was free).
 * WAITS: no.
 * Entry: A the mask, X object. Exit: the AND, or the start's A and N, Z with Y and C from the slot search (l8_spawn_piece21_exit_shim).
 */
L8St11Spawn SpawnPiece21UnlessMasked(uint8_t slot, uint8_t mask)
{
    L8St11Spawn r = { 0 };
    uint8_t scroll;

    scroll = RD8(SCROLL->x_lo[5]);
    r.anded = mask & scroll;
    if (r.anded != 0) {
        return r;
    }
    jsr_push(0xAF90);
    r.found = l8_free_slot30();
    jsr_pop();
    r.placed = StartObjectAtPlace(0x21, slot, r.found.slot);
    r.started = true;
    return r;
}

void abi_SpawnPiece21UnlessMasked(void)
{
    abi_ended(l8_spawn_piece21_exit_shim(SpawnPiece21UnlessMasked(cpu.x, cpu.a)));
}

/*
 * @routine 025A:795 CountdownThenSpawnPieces
 * @run 70:AF95
 * WHAT IT DOES: Sets the fixed fields and counts OBJ_34B8 down; at zero sets it to $40 and goes to the next state; before that it may spawn a piece and, once in a while, a second one.
 * HOW: SetBossFixedFields, SetTimerThenNextState or SpawnPiece21UnlessMasked, a compare of the scroll byte, NextRandom, FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction that ran (pieces_exit_shim).
 */
L8St11PiecesArm CountdownThenSpawnPieces(uint8_t slot)
{
    L8St11PiecesArm r = { 0 };
    uint8_t scroll;

    jsr_push(0xAF98);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer == 0) {
        r.state = SetTimerThenNextState(slot, 0x40);
        r.arm = ST11_PIECES_NEXT;
        return r;
    }
    jsr_push(0xAFA3);
    r.spawn = SpawnPiece21UnlessMasked(slot, 0x7F);
    jsr_pop();
    scroll = RD8(SCROLL->x_lo[5]);
    r.masked = scroll & 0x7F;
    if (r.masked != 0x40) {
        r.arm = ST11_PIECES_ONE;
        return r;
    }
    jsr_push(0xAFAF);
    r.random = NextRandom();
    jsr_pop();
    r.bits = r.random.value & 0x03;
    if (r.bits != 0) {
        r.arm = ST11_PIECES_NO_RANDOM;
        return r;
    }
    jsr_push(0xAFB6);
    r.found = l8_free_slot30();
    jsr_pop();
    jsr_push(0xAFBB);
    r.placed = StartObjectAtPlace(0x21, slot, r.found.slot);
    jsr_pop();
    WR8(OBJ->f3040[r.found.slot], 1);
    r.arm = ST11_PIECES_TWO;
    return r;
}

/* What CountdownThenSpawnPieces leaves in A, Y and the flags, from the arm it ended on. */
static Ended pieces_exit_shim(L8St11PiecesArm r)
{
    Ended e = then(l8_boss_fixed_exit_shim(r.fixed), ended_nz(r.timer));

    if (r.arm == ST11_PIECES_NEXT) {
        return then(e, l8_next_state_exit_shim(r.state));
    }
    e = then(e, ended_load(0x7F));
    e = then(e, l8_spawn_piece21_exit_shim(r.spawn));
    e = then(e, ended_compare(r.masked, 0x40));
    if (r.arm == ST11_PIECES_ONE) {
        return e;
    }
    e = then(e, ended_of_random(r.random));
    e = then(e, ended_load(r.bits));
    if (r.arm == ST11_PIECES_NO_RANDOM) {
        return e;
    }
    e = then(e, l8_slot_ended(r.found));
    e = then(e, ended_load(r.placed));
    return then(e, ended_load(1));
}

void abi_CountdownThenSpawnPieces(void)
{
    abi_ended(pieces_exit_shim(CountdownThenSpawnPieces(cpu.x)));
}

/*
 * @routine 025A:7C1 HoldScrollStepThenNextState
 * @run 70:AFC1
 * WHAT IT DOES: Sets the fixed fields, steps the scroll table (StepVerticalScrollTable), clears the scroll step ScrollStepXHi+1, and when the scroll's high byte is $FF and its low byte $80 sets the timer to $80 and goes to the next state; otherwise lowers the step by 2.
 * HOW: SetBossFixedFields, StepVerticalScrollTable, loads and compares of the scroll bytes, a tail jump into SetTimerThenNextState or two memory decrements.
 * WAITS: no.
 * Entry: X object. Exit: the second decrement's N and Z, or as SetTimerThenNextState (hold_exit_shim).
 */
L8St11HoldArm HoldScrollStepThenNextState(uint8_t slot)
{
    L8St11HoldArm r = { 0 };

    jsr_push(0xAFC4);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    jsr_push(0xAFC7);
    r.table = StepVerticalScrollTable(slot);
    jsr_pop();
    WR8(SCROLLE->step_x_hi[1], 0);
    r.high = RD8(SCROLL->x_hi[1]);
    if ((uint8_t)(r.high + 1) == 0) {
        r.low = RD8(SCROLL->x_lo[1]);
        if (r.low == 0x80) {
            r.state = SetTimerThenNextState(slot, 0x80);
            r.arm = ST11_HOLD_NEXT;
            return r;
        }
        r.arm = ST11_HOLD_NOT_80;
    } else {
        r.arm = ST11_HOLD_SCROLLING;
    }
    r.step = DEC8(SCROLLE->step_x_hi[1]);
    r.step = DEC8(SCROLLE->step_x_hi[1]);
    return r;
}

/* What HoldScrollStepThenNextState leaves in A, Y and the flags, from the arm it ended on. */
static Ended hold_exit_shim(L8St11HoldArm r)
{
    Ended e = then(l8_boss_fixed_exit_shim(r.fixed), l8_vertical_scroll_table_exit_shim(r.table));

    e = then(e, ended_load((uint8_t)(r.high + 1)));
    if (r.arm != ST11_HOLD_SCROLLING) {
        e = then(e, ended_compare(r.low, 0x80));
    }
    if (r.arm == ST11_HOLD_NEXT) {
        return then(e, l8_next_state_exit_shim(r.state));
    }
    return then(e, ended_nz(r.step));
}

void abi_HoldScrollStepThenNextState(void)
{
    abi_ended(hold_exit_shim(HoldScrollStepThenNextState(cpu.x)));
}

/*
 * @routine 025A:730 StartBossWhenStageC0OrLoad
 * @run 70:AF30
 * WHAT IT DOES: Sets the fixed fields (SetBossFixedFields). When the requested stage zStageRequested is $C0 it starts list $84 on channel 0 and script $8E in slot 5, clears the enemy slots, clears the frames of slots $56 and $57, starts five objects of type $2B from the slots 0 to 4 into free slots of $30 to $3F (OBJ_3040 the number), advances ScrollXHi+1 and plays the track again (CdPlayAgain). Otherwise it loads the object's record, sets OBJ_34B8 $80 and advances ScrollXHi+1.
 * HOW: SetBossFixedFields, StartListChannelKind2, StartScriptSlotKind1, ClearEnemySlots, a loop of FindFreeSlotFrom30 and StartObjectAtPlace, CdPlayAgain, LoadObjectRecord.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept for the track call. Exit: as the last instruction that ran (start_boss_exit_shim, then play_track_exit_shim in the $C0 arm). The $C0 arm ran in no script; the draft stops there and the arm is written from the decoded bytes.
 */
void ClearEnemySlots(uint8_t entry_x);  /* object/lifecycle.c */

L8St11StartBossArm StartBossWhenStageC0OrLoad(uint8_t slot, uint8_t y_entry)
{
    L8St11StartBossArm r = { 0 };
    uint8_t x, found_slot, last;
    bool found;

    jsr_push(0xAF33);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    r.stage = RD8(ZP->stage_requested);
    if (r.stage != 0xC0) {
        jsr_push(0xAF6D);
        r.record = l8_record(slot);
        jsr_pop();
        WR8(OBJ->f34b8[slot], 0x80);
        r.counted = INC8(SCROLL->x_hi[1]);
        r.arm = ST11_BOSS_LOAD;
        return r;
    }
    push8(slot);
    jsr_push(0xAF40);
    StartListChannelKind2(0x84, 0);
    jsr_pop();
    jsr_push(0xAF47);
    StartScriptSlotKind1(0x8E, 5);
    jsr_pop();
    jsr_push(0xAF4A);
    ClearEnemySlots(5);
    jsr_pop();
    WR8(OBJ->frame[0x56], 0);
    WR8(OBJ->frame[0x57], 0);
    for (x = 0; x != 5; x++) {
        jsr_push(0xAF54);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        jsr_push(0xAF59);
        (void)StartObjectAtPlace(0x2B, x, found_slot);
        jsr_pop();
        WR8(OBJ->f3040[found_slot], x);
    }
    r.pulled = pull8();
    r.counted = INC8(SCROLL->x_hi[1]);
    jsr_push(0xAF69);
    r.track = CdPlayAgain(r.pulled, y_entry);
    jsr_pop();
    r.arm = ST11_BOSS_C0;
    return r;
}

/* What StartBossWhenStageC0OrLoad leaves in A, Y and the flags before the track call, from the arm it ended on. */
static Ended start_boss_exit_shim(L8St11StartBossArm r)
{
    Ended e = then(l8_boss_fixed_exit_shim(r.fixed), ended_compare(r.stage, 0xC0));

    if (r.arm == ST11_BOSS_LOAD) {
        e = then(e, l8_record_ended(r.record));
        e = then(e, ended_load(0x80));
        return then(e, ended_nz(r.counted));
    }
    e = then(then(e, ended_with_y(l8_compare_y(5, 5), 0)), ended_nz(r.pulled));
    return then(e, ended_nz(r.counted));
}

void abi_StartBossWhenStageC0OrLoad(void)
{
    L8St11StartBossArm r = StartBossWhenStageC0OrLoad(cpu.x, cpu.y);

    abi_ended(start_boss_exit_shim(r));
    if (r.arm == ST11_BOSS_C0) {
        play_track_exit_shim(r.track);
    }
}

/*
 * @routine 025A:7DE StepScrollThenMoveOrFadeOut
 * @run 70:AFDE
 * WHAT IT DOES: Runs StepVerticalScrollTable. When slots 0's state and OBJ_2800 and Y speed high byte are all 0 and the byte at $3BC3 is not 0 it acts by OBJ_3670: at 0 it speeds up the object named by OBJ_3040 by $0010, sets ScrollStepXHi+1 to the negative of its X speed high byte, sets slot 0's OBJ_3510 $10 and OBJ_3568 0, hands slot 0's sprite set, frame and facing to the object named by OBJ_3040 and clears slot 0's frame; when ScrollXHi+1 is $FE it then sets the timer $40 and goes to the next state. At 1 it clears OBJ_3670, clears the object named by OBJ_3040, makes it kind $31, sets slot 0's OBJ_3510 $10 and OBJ_3568 0 and fades the CD with type $0C.
 * HOW: StepVerticalScrollTable, tests, 16-bit adds in memory, ClearObjectInY, cd_fade (the card's volume fade), a tail jump into SetTimerThenNextState.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran; with the fade, as cd_fade leaves them (fade_exit_shim, then cd_fade_exit_shim in the fade arm). The byte at $3BC3 and $3BF0 keep their address form until they have names. The OBJ_3670 not 0 arm ran in no script; the draft stops there.
 */
L8St11FadeArm StepScrollThenMoveOrFadeOut(uint8_t slot)
{
    L8St11FadeArm r = { 0 };
    uint8_t value, high, kind_hi;
    Sum sum;

    jsr_push(0xAFE1);
    r.table = StepVerticalScrollTable(slot);
    jsr_pop();
    value = RD8(OBJ->state[0]);
    high = RD8(OBJ->f2800[0]);
    r.joined = (uint8_t)(value | high);
    if (r.joined != 0) {
        r.arm = ST11_FADE_BUSY;
        return r;
    }
    r.vy = RD8(OBJ->vy_hi[0]);
    if (r.vy != 0) {
        r.arm = ST11_FADE_FALLING;
        return r;
    }
    r.card = rd8(0x3BC3);
    if (r.card == 0) {
        r.arm = ST11_FADE_NO_CARD;
        return r;
    }
    r.line = RD8(OBJ->f3670[slot]);
    if (r.line == 0) {
        push8(slot);
        r.target = RD8(OBJ->f3040[slot]);
        value = RD8(OBJ->vx_lo[r.target]);
        sum = add_bytes(0x10, value, false);
        WR8(OBJ->vx_lo[r.target], sum.value);
        high = RD8(OBJ->vx_hi[r.target]);
        sum = add_bytes(0, high, sum.carry);
        WR8(OBJ->vx_hi[r.target], sum.value);
        high = RD8(OBJ->vx_hi[r.target]);
        r.diff = sub_bytes(0, high, true);
        WR8(SCROLLE->step_x_hi[1], r.diff.value);
        r.pulled = pull8();
        slot = r.pulled;
        WR8(OBJ->f3510[0], 0x10);
        wr8(0x3BF0, 2);
        WR8(OBJ->f3568[0], 0);
        r.y = RD8(OBJ->f3040[slot]);
        value = RD8(OBJ->sprite_set[0]);
        WR8(OBJ->sprite_set[r.y], value);
        value = RD8(OBJ->frame[0]);
        WR8(OBJ->frame[r.y], value);
        value = RD8(OBJ->facing_left[0]);
        WR8(OBJ->facing_left[r.y], value);
        WR8(OBJ->frame[0], 0);
        r.scroll = RD8(SCROLL->x_hi[1]);
        if ((uint8_t)(r.scroll + 2) != 0) {
            r.arm = ST11_FADE_MOVED;
            return r;
        }
        r.state = SetTimerThenNextState(slot, 0x40);
        r.arm = ST11_FADE_NEXT;
        return r;
    }
    WR8(OBJ->f3670[slot], 0);
    r.y = RD8(OBJ->f3040[slot]);
    jsr_push(0xB051);
    ClearObjectInY(r.y);
    jsr_pop();
    WR8(OBJ->kind[r.y], 0x31);
    WR8(OBJ->f3510[0], 0x10);
    wr8(0x3BF0, 2);
    WR8(OBJ->f3568[0], 0);
    kind_hi = 0x0C;
    r.nibble = vec_cd_fade(kind_hi);
    r.arm = ST11_FADE_OUT;
    return r;
}

/* What StepScrollThenMoveOrFadeOut leaves in A, Y and the flags before the fade, from the arm it ended on. */
static Ended fade_exit_shim(L8St11FadeArm r)
{
    Ended e = l8_vertical_scroll_table_exit_shim(r.table);

    e = then(e, ended_load(r.joined));
    if (r.arm == ST11_FADE_BUSY) {
        return e;
    }
    e = then(e, ended_load(r.vy));
    if (r.arm == ST11_FADE_FALLING) {
        return e;
    }
    e = then(e, ended_load(r.card));
    if (r.arm == ST11_FADE_NO_CARD) {
        return e;
    }
    e = then(e, ended_load(r.line));
    if (r.arm == ST11_FADE_OUT) {
        return then(e, ended_with_y(ended_load(0x0C), r.y));
    }
    e = then(e, ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(r.pulled), r.diff.value), r.diff.carry), r.diff.overflow));
    e = then(e, ended_load(2));
    e = then(e, ended_with_y(ended_load(0x40), r.y));
    e = then(e, ended_with_y(ended_nz((uint8_t)(r.scroll + 2)), (uint8_t)(r.scroll + 2)));
    if (r.arm == ST11_FADE_MOVED) {
        return e;
    }
    return then(e, l8_next_state_exit_shim(r.state));
}

void abi_StepScrollThenMoveOrFadeOut(void)
{
    L8St11FadeArm r = StepScrollThenMoveOrFadeOut(cpu.x);

    abi_ended(fade_exit_shim(r));
    if (r.arm == ST11_FADE_OUT) {
        cd_fade_exit_shim(r.nibble);
    }
}
