/*
 * TEMPORARY: the stage 08 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* The arms the converted routines end on (ObjArm, enemy_objects.h); each routine's exit shim builds the registers and flags from them. */
enum { AS_HELD, AS_OTHER_FRAME, AS_NO_SLOT, AS_SPAWNED, AS_PAUSE };

/* Stores a constant in the object's f34b8 (here the phase index of the speed table below). */
static Ended set_phase(uint8_t slot, uint8_t phase)
{
    WR8(OBJ->life_timer[slot], phase);
    return ended_load(phase);
}

/*
 * @routine 0D5B:200 SetPhase2
 * @run 70:B200
 * WHAT IT DOES: Sets the object's f34b8 to 2, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 2, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase2(uint8_t slot)
{
    return set_phase(slot, 2);
}

void abi_SetPhase2(void)
{
    abi_ended(SetPhase2(cpu.x));
}

/*
 * @routine 0D5B:206 SetPhase3
 * @run 70:B206
 * WHAT IT DOES: Sets the object's f34b8 to 3, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase3(uint8_t slot)
{
    return set_phase(slot, 3);
}

void abi_SetPhase3(void)
{
    abi_ended(SetPhase3(cpu.x));
}

/*
 * @routine 0D5B:20C SetPhase4
 * @run 70:B20C
 * WHAT IT DOES: Sets the object's f34b8 to 4, the phase SetSpeedFromPhaseTable reads.
 * HOW: One store.
 * WAITS: no.
 * Entry: X object. Exit: A 4, N and Z clear; X, Y, C and V kept.
 */
Ended SetPhase4(uint8_t slot)
{
    return set_phase(slot, 4);
}

void abi_SetPhase4(void)
{
    abi_ended(SetPhase4(cpu.x));
}

/*
 * @routine 0D5B:212 SetSpeedFromPhaseTable
 * @run 70:B212
 * WHAT IT DOES: Sets the object's X and Y speeds from a table of four byte entries, the entry
 * picked by the object's phase (f34b8) from a block chosen by the way it faces, and moves the
 * object on to its next state.
 * HOW: A table at overlay address $B23F gives each facing's block start; the entry's bytes are
 * at $B241 to $B244 from there (X low, X high, Y low, Y high; the loads run high byte first).
 * The block start passes through zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from the new state; C and V from the add
 * of the block start; Y the entry's offset; X kept. Clobbers zTemp00.
 */
Ended SetSpeedFromPhaseTable(uint8_t slot)
{
    enum { BLOCKS = 0xB23F, ENTRIES = 0xB241 };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t phase, start, y, value, state, scaled;
    unsigned total;

    WR8(ZP->temp00, rd8((uint16_t)(BLOCKS + facing)));
    phase = RD8(OBJ->life_timer[slot]);
    scaled = (uint8_t)(phase << 2);
    start = RD8(ZP->temp00);
    total = (unsigned)scaled + start;
    y = (uint8_t)total;
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(ENTRIES + 1 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(ENTRIES + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(ENTRIES + 3 + y)));
    value = rd8((uint16_t)(ENTRIES + 2 + y));
    WR8(OBJ->vy_lo[slot], value);
    state = RD8(OBJ->state[slot]);
    state = (uint8_t)(state + 1);
    WR8(OBJ->state[slot], state);
    return ended_with_overflow(
        ended_with_y(ended_with_carry(ended_with_a(ended_nz(state), value), total > 0xFF), y),
        add_overflow(scaled, start, y));
}

void abi_SetSpeedFromPhaseTable(void)
{
    abi_ended(SetSpeedFromPhaseTable(cpu.x));
}

/*
 * @routine 0D5E:14F EnterState6IfFlaggedB
 * @run 71:C94F
 * WHAT IT DOES: Sends the object to state 6 when bit 7 of its status flags is set (the same as
 * EnterState6IfFlagged, in this stage's overlay).
 * HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
 */
ObjArm EnterState6IfFlaggedB(uint8_t slot)
{
    return enter_state_if_flagged(slot, 6);
}

void abi_EnterState6IfFlaggedB(void)
{
    abi_ended(load_exit_shim(EnterState6IfFlaggedB(cpu.x)));
}

/*
 * @routine 0D5E:171 AdvanceState
 * @run 71:C971
 * WHAT IT DOES: Moves the object on to its next state.
 * HOW: One increment of the state byte.
 * WAITS: no.
 * Entry: X object. Exit: N, Z from the new state; A, X, Y, C and V kept.
 */
Ended AdvanceState(uint8_t slot)
{
    uint8_t state = RD8(OBJ->state[slot]);

    state = (uint8_t)(state + 1);
    WR8(OBJ->state[slot], state);
    return ended_nz(state);
}

void abi_AdvanceState(void)
{
    abi_ended(AdvanceState(cpu.x));
}

/*
 * @routine 0D5E:6A6 EnterState3WithTallHitbox
 * @run 71:CEA6
 * WHAT IT DOES: Puts the object in state 3 with a narrow, tall hit rectangle (half width 6, half
 * height $18, no offsets), f3358 set to 2 and its step byte cleared.
 * HOW: Constant stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
 */
Ended EnterState3WithTallHitbox(uint8_t slot)
{
    WR8(OBJ->f3358[slot], 2);
    WR8(OBJ->hitbox_half_w[slot], 6);
    WR8(OBJ->hitbox_half_h[slot], 0x18);
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    WR8(OBJ->state[slot], 3);
    WR8(OBJ->f2800[slot], 0);
    return ended_load(3);
}

void abi_EnterState3WithTallHitbox(void)
{
    abi_ended(EnterState3WithTallHitbox(cpu.x));
}

/*
 * @routine 0D5E:6EE SetHitboxFromFrameRowsB
 * @run 71:CEEE
 * WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
 * chosen by the frame minus $5A; frames under $5A leave it alone.
 * HOW: The table is at overlay address $CF12.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded (or the frame minus $5A when under); N, Z from
 * it; C clear when under, else bit 6 of the row number; V from the subtract; Y the row's offset
 * when read; X kept.
 */
Ended SetHitboxFromFrameRowsB(uint8_t slot)
{
    enum { TABLE = 0xCF12 };
    uint8_t frame = RD8(OBJ->frame[slot]);
    uint8_t row = (uint8_t)(frame - 0x5A);
    uint8_t y, value;
    bool overflow = sub_overflow(frame, 0x5A, row);

    if (frame < 0x5A) {
        return ended_with_overflow(ended_with_carry(ended_load(row), false), overflow);
    }
    y = (uint8_t)(row << 2);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + y));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(TABLE + 2 + y));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(TABLE + 3 + y));
    WR8(OBJ->hitbox_half_h[slot], value);
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(value), (row & 0x40) != 0), y),
                               overflow);
}

void abi_SetHitboxFromFrameRowsB(void)
{
    abi_ended(SetHitboxFromFrameRowsB(cpu.x));
}

/*
 * @routine 0D5B:12B StartFacingPlayerThenCheck
 * @run 70:B12B
 * WHAT IT DOES: Starts the object from its record, turns it to face the player, sets f3510 to
 * $18, and, when its next_kind is 1, either removes the object (the player under $20 pixels
 * away along X) or sends it to state 6.
 * HOW: LoadObjectRecord, SideOfPlayer, DistanceXToPlayer, ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the last compare or load; V from the
 * distance's subtract on the near path; Y 11 (the record); X kept. Clobbers zTemp00, zTemp10-11.
 */
Ended StartFacingPlayerThenCheck(uint8_t slot)
{
    uint8_t nz, last, bit, kind;
    bool carry;
    SideResult sides;
    PlayerDistance distance;
    Ended e;

    jsr_push(0xB12E);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    jsr_push(0xB131);
    sides = SideOfPlayer(slot);
    jsr_pop();
    bit = (uint8_t)(sides.sides & 1);
    e = ended_then(e, ended_then(ended_of_side(sides), ended_load(bit)));
    WR8(OBJ->facing_left[slot], bit);
    WR8(OBJ->f3510[slot], 0x18);
    e = ended_then(e, ended_load(0x18));
    kind = RD8(OBJ->next_kind[slot]);
    e = ended_then(e, ended_compare(kind, 1));
    if (kind != 1) {
        return e;
    }
    jsr_push(0xB145);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = ended_then(e, ended_of_distance(distance));
    e = ended_then(e, ended_compare(distance.distance, 0x20));
    if (distance.distance < 0x20) {
        ClearObject(slot);
        return e;
    }
    WR8(OBJ->state[slot], 6);
    return ended_then(e, ended_load(6));
}

void abi_StartFacingPlayerThenCheck(void)
{
    abi_ended(StartFacingPlayerThenCheck(cpu.x));
}

/*
 * @routine 0D5B:152 AnimateThenCountToState3
 * @run 70:B152
 * WHAT IT DOES: Unless the object in slot $54 is showing frame 4, animates the object along the
 * list at overlay address $B17B and, when its next_kind is not $FF and its X high byte is zero,
 * counts f3510 down; at zero it enters state 3.
 * HOW: StepObjectFrames, then the tests in that order, each stopping at the first that fails.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the last compare, load or decrement; Y as the
 * animation step left it; X and V kept. Clobbers zTemp10-11.
 */
Ended AnimateThenCountToState3(uint8_t slot)
{
    uint8_t frame = RD8(OBJ->frame[0x54]);
    uint8_t exit_y = 0, kind, high, left;
    FrameStep step;
    Ended e;

    e = ended_compare(frame, 4);
    if (frame == 4) {
        return e;
    }
    WR8(ZP->temp11, 0xB1);
    WR8(ZP->temp10, 0x7B);
    jsr_push(0xB164);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(e, ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    kind = RD8(OBJ->next_kind[slot]);
    e = ended_then(e, ended_compare(kind, 0xFF));
    if (kind == 0xFF) {
        return e;
    }
    high = RD8(OBJ->x_hi[slot]);
    e = ended_then(e, ended_load(high));
    if (high != 0) {
        return e;
    }
    left = RD8(OBJ->f3510[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->f3510[slot], left);
    e = ended_then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->state[slot], 3);
    return ended_then(e, ended_load(3));
}

void abi_AnimateThenCountToState3(void)
{
    abi_ended(AnimateThenCountToState3(cpu.x));
}

/*
 * @routine 0D5B:1CE PickPhaseAtRandomA
 * @run 70:B1CE
 * WHAT IT DOES: Picks the object's phase (the f34b8 byte SetSpeedFromPhaseTable reads) from a
 * random number's low three bits: 0 and 1 give phase 3, 2 gives phase 2, 3 gives phase 1, any
 * other gives phase 0.
 * HOW: NextRandom, compares, and a tail into SetPhase3 or SetPhase2.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the phase (or the masked number for phase 0), N, Z from it; C
 * from the last compare (V from the random number's add).
 */
Ended PickPhaseAtRandomA(uint8_t slot)
{
    RandomResult random;
    uint8_t bits;
    Ended e;

    jsr_push(0xB1D1);
    random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(random.value & 7);
    e = ended_then(ended_of_random(random), ended_load(bits));
    e = ended_then(e, ended_compare(bits, 2));
    if (bits < 2) {
        return ended_then(e, SetPhase3(slot));
    }
    if (bits == 2) {
        return ended_then(e, SetPhase2(slot));
    }
    e = ended_then(e, ended_compare(bits, 3));
    if (bits == 3) {
        WR8(OBJ->life_timer[slot], 1);
        return ended_then(e, ended_load(1));
    }
    WR8(OBJ->life_timer[slot], 0);
    return e;
}

void abi_PickPhaseAtRandomA(void)
{
    abi_ended(PickPhaseAtRandomA(cpu.x));
}

/*
 * @routine 0D5B:1E3 PickPhaseAtRandomB
 * @run 70:B1E3
 * WHAT IT DOES: Picks the object's phase from a random number's low three bits: 0 gives phase 3,
 * 1 gives phase 2, 2 gives phase 0, 3 to 7 give phase 1 when the object faces the way the player
 * does and phase 0 when not.
 * HOW: NextRandom, compares, and a tail into SetPhase3 or SetPhase2.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the phase (or the facing difference), N, Z from it; C from the
 * last compare (V from the random number's add).
 */
Ended PickPhaseAtRandomB(uint8_t slot)
{
    RandomResult random;
    uint8_t bits, mine, theirs, difference;
    Ended e;

    jsr_push(0xB1E6);
    random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(random.value & 7);
    e = ended_then(ended_of_random(random), ended_load(bits));
    if (bits == 0) {
        return ended_then(e, SetPhase3(slot));
    }
    e = ended_then(e, ended_compare(bits, 1));
    if (bits == 1) {
        return ended_then(e, SetPhase2(slot));
    }
    e = ended_then(e, ended_compare(bits, 2));
    if (bits == 2) {
        WR8(OBJ->life_timer[slot], 0);
        return e;
    }
    mine = RD8(OBJ->facing_left[slot]);
    theirs = RD8(OBJ->facing_left[0]);
    difference = (uint8_t)(mine ^ theirs);
    e = ended_then(e, ended_load(difference));
    if (difference != 0) {
        WR8(OBJ->life_timer[slot], 0);
        return e;
    }
    WR8(OBJ->life_timer[slot], 1);
    return ended_then(e, ended_load(1));
}

void abi_PickPhaseAtRandomB(void)
{
    abi_ended(PickPhaseAtRandomB(cpu.x));
}

/*
 * @routine 0D5E:15C StartWithTimerByNextKind
 * @run 71:C95C
 * WHAT IT DOES: Starts the object from its record, sets its life_timer from a table at overlay
 * address $C96F by its next_kind, and clears f3510 and f3568.
 * HOW: LoadObjectRecord, one table read.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the timer, N, Z from it; C from the record; Y the next_kind.
 */
Ended StartWithTimerByNextKind(uint8_t slot)
{
    enum { TABLE = 0xC96F };
    uint8_t nz, last, kind, value;
    bool carry;
    Ended e;

    jsr_push(0xC95F);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    kind = RD8(OBJ->next_kind[slot]);
    value = rd8((uint16_t)(TABLE + kind));
    WR8(OBJ->life_timer[slot], value);
    WR8(OBJ->f3510[slot], 0);
    WR8(OBJ->f3568[slot], 0);
    return ended_then(e, ended_with_y(ended_load(value), kind));
}

void abi_StartWithTimerByNextKind(void)
{
    abi_ended(StartWithTimerByNextKind(cpu.x));
}

/*
 * @routine 0D5E:4B0 StartWithFrameByNextKind
 * @run 71:CCB0
 * WHAT IT DOES: Starts the object from its record, clears f3568, and gives it a frame from a two
 * byte table at overlay address $CCC7 by bit 4 of its next_kind.
 * HOW: LoadObjectRecord, one table read.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the frame, N, Z from it; C bit 3 of next_kind; Y 0 or 1.
 */
Ended StartWithFrameByNextKind(uint8_t slot)
{
    enum { TABLE = 0xCCC7 };
    uint8_t nz, last, kind, y, value;
    bool carry;
    Ended e;

    jsr_push(0xCCB3);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->f3568[slot], 0);
    kind = RD8(OBJ->next_kind[slot]);
    y = (uint8_t)((kind >> 4) & 1);
    value = rd8((uint16_t)(TABLE + y));
    WR8(OBJ->frame[slot], value);
    return ended_then(e, ended_with_y(ended_with_carry(ended_load(value), (kind & 8) != 0), y));
}

void abi_StartWithFrameByNextKind(void)
{
    abi_ended(StartWithFrameByNextKind(cpu.x));
}

/*
 * @routine 0D5E:4EC AnimateByKindThenNextState
 * @run 71:CCEC
 * WHAT IT DOES: Animates the object along one of two lists (pointers at overlay address $CD0F)
 * picked by bit 4 of its next_kind; when the list has ended it sets f3250 to 2 and moves the
 * object on to its next state.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as StepObjectFrames, or at the end A 2 and N, Z from the new
 * state. Clobbers zTemp10-11.
 */
Ended AnimateByKindThenNextState(uint8_t slot)
{
    enum { LISTS = 0xCD0F };
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t y = (uint8_t)(((kind >> 4) & 1) << 1);
    uint8_t exit_y = y, state;
    FrameStep step;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + y)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + y)));
    jsr_push(0xCD04);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), y), ended_of_frame_step(step, step != FRAME_HELD, exit_y));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    state = INC8(OBJ->state[slot]);
    return ended_then(e, ended_with_a(ended_nz(state), 2));
}

void abi_AnimateByKindThenNextState(void)
{
    abi_ended(AnimateByKindThenNextState(cpu.x));
}

/*
 * @routine 0D5E:1A4 StepFramesFacingPlayerE
 * @run 71:C9A4
 * WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
 * pointer is the pair of bytes at overlay address $C9B9 plus Y.
 * HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
 * zTemp10-11.
 */
ObjArm StepFramesFacingPlayerE(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xC9B9 };
    ObjArm r = { 0 };
    uint8_t sides, exit_y = index;

    jsr_push(0xC9A7);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    return r;
}

void abi_StepFramesFacingPlayerE(void)
{
    abi_ended(frame_step_exit_shim(StepFramesFacingPlayerE(cpu.x, cpu.y)));
}

/*
 * @routine 0D5E:18F WalkWhileFrameChangesC
 * @run 71:C98F
 * WHAT IT DOES: Animates the object facing the player and, on the frames where the picture
 * changed, gives it an X speed from a table at overlay address $C9A2 by its f3510 and moves it.
 * HOW: StepFramesFacingPlayerE with Y 0, then ApplyVelocityX.
 * WAITS: no.
 * Entry, exit: as WalkWhileFrameChanges.
 */
ObjArm WalkWhileFrameChangesC(uint8_t slot)
{
    enum { SPEEDS = 0xC9A2 };
    ObjArm r;
    uint8_t phase;

    jsr_push(0xC993);
    r = StepFramesFacingPlayerE(slot, 0);
    jsr_pop();
    r.arm = WW_HELD;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xC9A1);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.arm = WW_MOVED;
    r.first = phase;
    return r;
}

void abi_WalkWhileFrameChangesC(void)
{
    abi_ended(walk_while_frame_changes_exit_shim(WalkWhileFrameChangesC(cpu.x)));
}

/*
 * @routine 0D5E:286 AnimateAndSpawnThenPause
 * @run 71:CA86
 * WHAT IT DOES: While its next_kind is not 0, or the player is within $70 along X, it animates facing the player
 * (list 2) and, on the frame change to frame $6B, starts an object of type $41 at its place from a free slot from
 * $30. When the list ends, or the player is farther away with next_kind 0, it stops its animation, enters state 3 and
 * takes a life_timer from a table at overlay address $CAC7 by two bits of f3568, which it counts up.
 * HOW: DistanceXToPlayer, StepFramesFacingPlayerE, FindFreeSlotFrom30, StartObjectAtPlace as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: after a spawn as StartObjectAtPlace left them; pausing: A the timer, N, Z from it,
 * Y the table index; otherwise the failing test or the step's flags.
 */
ObjArm AnimateAndSpawnThenPause(uint8_t slot)
{
    enum { TIMERS = 0xCAC7 };
    ObjArm r = { 0 };
    ObjArm frames;
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t frame, last, found_slot, count, y, value;
    bool found, pause = false;

    r.first = kind;
    if (kind == 0) {
        jsr_push(0xCA8E);
        r.distance = DistanceXToPlayer(slot);
        jsr_pop();
        r.flag = true;
        pause = r.distance.distance >= 0x70;
    }
    if (!pause) {
        jsr_push(0xCA97);
        frames = StepFramesFacingPlayerE(slot, 2);
        jsr_pop();
        r.other = true;
        r.step = frames.step;
        r.y = frames.y;
        if (r.step != FRAME_LIST_ENDED) {
            r.arm = AS_HELD;
            if (r.step != FRAME_CHANGED) {
                return r;
            }
            frame = RD8(OBJ->frame[slot]);
            r.arm = AS_OTHER_FRAME;
            r.second = frame;
            if (frame != 0x6B) {
                return r;
            }
            jsr_push(0xCAA5);
            found_slot = FindFreeSlotFrom30(&last, &found);
            jsr_pop();
            r.arm = AS_NO_SLOT;
            r.third = found_slot;
            r.fourth = last;
            if (!found) {
                return r;
            }
            r.fifth = StartObjectAtPlace(0x41, slot, found_slot);
            r.arm = AS_SPAWNED;
            return r;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 3);
    INC8(OBJ->f3568[slot]);
    count = RD8(OBJ->f3568[slot]);
    y = (uint8_t)(count & 3);
    value = rd8((uint16_t)(TIMERS + y));
    WR8(OBJ->life_timer[slot], value);
    r.arm = AS_PAUSE;
    r.second = y;
    r.fifth = value;
    return r;
}

/* The LDA of next_kind; the distance and its CMP with $70 when it was 0; LDY #2 and the step when it animated; then the
 * CMP of the frame with $6B, the slot search, LDA #$41 and the start's A; or, pausing, the timer's LDA and TAY. */
static Ended animate_and_spawn_then_pause_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.flag) {
        e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x70));
    }
    if (r.other) {
        e = ended_then(e, ldy_frame_step_shim(2, r));
    }
    if (r.arm == AS_PAUSE) {
        return ended_then(e, ended_with_y(ended_load(r.fifth), r.second));
    }
    if (r.arm == AS_HELD) {
        return e;
    }
    e = ended_then(e, ended_compare(r.second, 0x6B));
    if (r.arm == AS_OTHER_FRAME) {
        return e;
    }
    e = ended_then(e, ended_of_find_slot(r.third, r.fourth, r.arm == AS_SPAWNED));
    if (r.arm == AS_NO_SLOT) {
        return e;
    }
    return ended_then(ended_then(e, ended_load(0x41)), ended_load(r.fifth));
}

void abi_AnimateAndSpawnThenPause(void)
{
    abi_ended(animate_and_spawn_then_pause_exit_shim(AnimateAndSpawnThenPause(cpu.x)));
}

/*
 * @routine 0D5E:3B2 StartLobTowardPlayerB
 * @run 71:CBB2
 * @stop CBED
 * WHAT IT DOES: StartLobTowardPlayer in this stage's overlay, with its table at overlay address $CBED.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as StartLobTowardPlayer.
 */
ObjArm StartLobTowardPlayerB(uint8_t slot, uint8_t entry_y)
{
    static const uint16_t RETURNS[5] = { 0xCBBA, 0xCBC2, 0xCBC5, 0xCBE5, 0xCBEC };
    ObjArm r = { 0 };

    if (RD8(OBJ->state[slot]) != 0) {
        r.arm = LB_ARC;
        r.sub = obj_sub(StepLobArcB(slot, entry_y));
        return r;
    }
    r.arm = LB_START;
    r.sub = obj_sub(start_lob(slot, 0xCBED, RETURNS));
    return r;
}

void abi_StartLobTowardPlayerB(void)
{
    abi_ended(start_lob_toward_player_exit_shim(StartLobTowardPlayerB(cpu.x, cpu.y)));
}

/*
 * @routine 0D5E:3FD StepLobArcB
 * @run 71:CBFD
 * WHAT IT DOES: StepLobArc with the frame list at overlay address $C9EF.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as StepLobArc.
 */
ObjArm StepLobArcB(uint8_t slot, uint8_t entry_y)
{
    return step_lob_arc(slot, entry_y, 0xC9EF, 0xCC08);
}

void abi_StepLobArcB(void)
{
    abi_ended(step_lob_arc_exit_shim(StepLobArcB(cpu.x, cpu.y)));
}
