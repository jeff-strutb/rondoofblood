/*
 * TEMPORARY: the stage 04D1 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
 * instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* RecordObjectEvent's exit: unmarked, A 0 and Z; else A the byte stored, Y its index, C whether the event is a multiple of 8. */
static Ended ended_of_object_event(bool marked, uint8_t stored, uint8_t event, uint8_t byte)
{
    if (!marked) {
        return ended_load(0);
    }
    return ended_with_carry(ended_with_y(ended_load(stored), byte), (event & 7) == 0);
}

/*
 * @routine 051B:23D LoadRecordA04
 * @run 70:B23D
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordA04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB240);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordA04(void)
{
    abi_ended(LoadRecordA04(cpu.x));
}

/*
 * @routine 051B:76A LoadRecordB04
 * @run 70:B76A
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordB04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB76D);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordB04(void)
{
    abi_ended(LoadRecordB04(cpu.x));
}

/*
 * @routine 051B:7C4 StepListB819
 * @run 70:B7C4
 * WHAT IT DOES: Steps the animation list at $B819, then goes on into the code that follows it ($B7CF).
 * HOW: StepObjectFrames; the code after it is entered by a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step, then the code at $B7CF.
 */
Jump StepListB819(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Jump jump = { ended_kept(), 0xB7CF, true };

    WR8(ZP->temp10, 0x19);
    WR8(ZP->temp11, 0xB8);
    jsr_push(0xB7CF);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    jump.end = ended_of_frame_step(step, step != FRAME_HELD, y);
    return jump;
}

void abi_StepListB819(void)
{
    Jump jump = StepListB819(cpu.x, cpu.y);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051E:109 RandomTimer10
 * @run 71:C909
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number from 0 to 15 plus 16.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N, Z, C and V from the add.
 */
Ended RandomTimer10(uint8_t slot)
{
    RandomResult random;
    Sum sum;
    Ended e;

    jsr_push(0xC90C);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    e = ended_then(e, ended_load((uint8_t)(random.value & 0x0F)));
    sum = add_bytes((uint8_t)(random.value & 0x0F), 0x10, false);
    WR8(OBJ->f34b8[slot], sum.value);
    return ended_then(e, ended_of_sum(sum));
}

void abi_RandomTimer10(void)
{
    abi_ended(RandomTimer10(cpu.x));
}

/*
 * @routine 051D:4E8 CopyScrollToTimer
 * @run 71:C4E8
 * WHAT IT DOES: Sets the object's OBJ_34B8 to the scroll's X fraction of slot 4 and then to its X low of slot 4 (the second store stays).
 * HOW: Two reads and two stores.
 * WAITS: no.
 * Entry: X object. Exit: A the scroll X low, N and Z from it.
 */
Ended CopyScrollToTimer(uint8_t slot)
{
    uint8_t value;

    value = RD8(SCROLL->x_frac[4]);
    WR8(OBJ->f34b8[slot], value);
    value = RD8(SCROLL->x_lo[4]);
    WR8(OBJ->f34b8[slot], value);
    return ended_load(value);
}

void abi_CopyScrollToTimer(void)
{
    abi_ended(CopyScrollToTimer(cpu.x));
}

/*
 * @routine 051B:4BF SetHitBox0C18
 * @run 70:B4BF
 * WHAT IT DOES: Sets the object's hit box: X offset 0, half width $0C, half height $18.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $18, N and Z from it.
 */
Ended SetHitBox0C18(uint8_t slot)
{
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_half_w[slot], 0x0C);
    WR8(OBJ->hitbox_half_h[slot], 0x18);
    return ended_load(0x18);
}

void abi_SetHitBox0C18(void)
{
    abi_ended(SetHitBox0C18(cpu.x));
}

/*
 * @routine 051B:5CD StartPieceAtNextSlot
 * @run 70:B5CD
 * WHAT IT DOES: Clears the object in the slot after this one, starts a kind $32 object there at this object's place and goes on to the next OBJ_2800.
 * HOW: ClearObjectInY, StartObjectAtPlace, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y the slot after; N and Z from the new OBJ_2800.
 */
Ended StartPieceAtNextSlot(uint8_t slot)
{
    uint8_t to = (uint8_t)(slot + 1), placed, phase;

    jsr_push(0xB5D3);
    ClearObjectInY(to);
    jsr_pop();
    jsr_push(0xB5D8);
    placed = StartObjectAtPlace(0x32, slot, to);
    jsr_pop();
    phase = INC8(OBJ->f2800[slot]);
    return ended_with_y(ended_with_a(ended_nz(phase), placed), to);
}

void abi_StartPieceAtNextSlot(void)
{
    abi_ended(StartPieceAtNextSlot(cpu.x));
}

/*
 * @routine 051B:690 DrawTileScriptByHighBits
 * @run 70:B690
 * WHAT IT DOES: Draws the tile script of group 0 numbered by bits 4 to 6 of OBJ_3040 (as 0 to 7) and goes on to the next OBJ_2800.
 * HOW: DrawTileScript, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the tile script's registers, then N and Z from the new OBJ_2800.
 */
Ended DrawTileScriptByHighBits(uint8_t slot)
{
    TileRun run;
    uint8_t item, phase;
    Ended e;

    item = (uint8_t)((RD8(OBJ->f3040[slot]) & 0x70) >> 4);
    e = ended_with_carry(ended_load(item), false);
    jsr_push(0xB69D);
    run = DrawTileScript(item, 0);
    jsr_pop();
    e = then(e, ended_of_tile_run(run, item));
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_DrawTileScriptByHighBits(void)
{
    abi_ended(DrawTileScriptByHighBits(cpu.x));
}

/*
 * @routine 051B:368 StepListB37FThenNext
 * @run 70:B368
 * WHAT IT DOES: Steps the animation list at $B37F; when it ends, the object's animation step and timer are cleared and it goes to the next OBJ_2800.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
 */
Ended StepListB37FThenNext(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0x7F);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB373);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    uint8_t phase;

    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_StepListB37FThenNext(void)
{
    abi_ended(StepListB37FThenNext(cpu.x, cpu.y));
}

/*
 * @routine 051B:30D StateSixOnStatusBit7
 * @run 70:B30D
 * WHAT IT DOES: An object in a state under 6 whose OBJ_31A0 has bit 7 set goes to state 6 and clears OBJ_2800.
 * HOW: A compare, a test, two stores.
 * WAITS: no.
 * Entry: X object. Exit: the compare, the test, or A 6.
 */
Ended StateSixOnStatusBit7(uint8_t slot)
{
    uint8_t state, status;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 6);
    if (state >= 6) {
        return e;
    }
    status = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    e = then(e, ended_load(status));
    if (status == 0) {
        return e;
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(6));
}

void abi_StateSixOnStatusBit7(void)
{
    abi_ended(StateSixOnStatusBit7(cpu.x));
}

/*
 * @routine 051B:7AC StartJumpUpLeft
 * @run 70:B7AC
 * WHAT IT DOES: Sets the object's speed to $FE80 in X and $FC80 in Y (high bytes $FE and $FC) and goes on to the next state.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from the new state.
 */
Ended StartJumpUpLeft(uint8_t slot)
{
    uint8_t state;

    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0x80);
    state = INC8(OBJ->state[slot]);
    return then(ended_load(0x80), ended_nz(state));
}

void abi_StartJumpUpLeft(void)
{
    abi_ended(StartJumpUpLeft(cpu.x));
}

/*
 * @routine 051B:135 StepListByTimerWithGravity
 * @run 70:B135
 * WHAT IT DOES: Steps the animation list the table at $B14D gives for OBJ_34B8, applies gravity without a limit and moves the object.
 * HOW: Two table reads (bank 70), StepObjectFrames, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y overwritten. Exit: as ApplyVelocity, Y as the step left it.
 */
Ended StepListByTimerWithGravity(uint8_t slot)
{
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    uint8_t index, y;

    index = (uint8_t)(RD8(OBJ->f34b8[slot]) << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xB14D + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB14E + y)));
    jsr_push(0xB147);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    jsr_push(0xB14A);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    move = ApplyVelocity(slot);
    (void)step;
    (void)gravity;
    return ended_with_y(ended_of_axis_move(move), y);
}

void abi_StepListByTimerWithGravity(void)
{
    abi_ended(StepListByTimerWithGravity(cpu.x));
}

/*
 * @routine 051B:3C3 StepListB37FUnlessFar
 * @run 70:B3C3
 * WHAT IT DOES: When the object's X high byte is not zero, or when the animation list at $B37F ends, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
 * HOW: StepObjectFrames, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the test or the step, or the new state's N and Z.
 */
Ended StepListB37FUnlessFar(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry, high, state;
    Ended e;

    high = RD8(OBJ->x_hi[slot]);
    e = ended_load(high);
    if (high == 0) {
        WR8(ZP->temp10, 0x7F);
        WR8(ZP->temp11, 0xB3);
        jsr_push(0xB3D3);
        step = StepObjectFrames(slot, &y);
        jsr_pop();
        e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
        if (step != FRAME_LIST_ENDED) {
            return e;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_nz(state));
}

void abi_StepListB37FUnlessFar(void)
{
    abi_ended(StepListB37FUnlessFar(cpu.x, cpu.y));
}

/*
 * @routine 051B:6ED StartFourPiecesWithCount
 * @run 70:B6ED
 * WHAT IT DOES: Starts up to four kind $33 objects at this object's place, each in a free slot from $30, and gives each its number (0 to 3) as OBJ_34B8.
 * HOW: A loop of FindFreeSlotFrom30, StartObjectAtPlace and a counter in zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, or as FindFreeSlotFrom30 when no slot was free.
 */
Ended StartFourPiecesWithCount(uint8_t slot)
{
    uint8_t count, found_slot, last;
    bool found;
    Ended e = ended_kept();

    WR8(ZP->temp00, 0);
    do {
        jsr_push(0xB6F2);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        e = then(e, ended_of_find_slot(found_slot, last, found));
        if (!found) {
            return e;
        }
        jsr_push(0xB6F9);
        StartObjectAtPlace(0x33, slot, found_slot);
        jsr_pop();
        count = RD8(ZP->temp00);
        WR8(OBJ->f34b8[found_slot], count);
        INC8(ZP->temp00);
        count = RD8(ZP->temp00);
        e = then(e, ended_with_a(ended_compare(count, 4), count));
    } while (count < 4);
    return e;
}

void abi_StartFourPiecesWithCount(void)
{
    abi_ended(StartFourPiecesWithCount(cpu.x));
}

/*
 * @routine 051B:648 StepListB66BToState2
 * @run 70:B648
 * WHAT IT DOES: Steps the animation list at $B66B; when it ends the object gets an X speed of $FEE0, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or A 2 when the list ended.
 */
Ended StepListB66BToState2(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0x6B);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB653);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0xE0);
    WR8(OBJ->angle_lo[slot], 0);
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(2));
}

void abi_StepListB66BToState2(void)
{
    abi_ended(StepListB66BToState2(cpu.x, cpu.y));
}

/*
 * @routine 051B:241 StartPhaseWhenScrolled
 * @run 70:B241
 * WHAT IT DOES: Once the level's X position high byte is 3 or more it clears the object in slot $41, clears the wave counter, sets OBJ_34B8 to $5B, sets bit 1 of the hit attack flags and zTemp $3C to 1, and goes on to the next state.
 * HOW: A compare, ClearObjectInY, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new state's N and Z (A 1).
 */
Ended StartPhaseWhenScrolled(uint8_t slot)
{
    uint8_t high, flags, state;
    Ended e;

    high = rd8(ScrollXHiScreen);
    e = then(ended_load(high), ended_compare(high, 3));
    if (high < 3) {
        return e;
    }
    jsr_push(0xB24D);
    ClearObjectInY(0x41);
    jsr_pop();
    WR8(ZP->wave_counter, 0);
    WR8(OBJ->f34b8[slot], 0x5B);
    flags = rd8(HitAttackFlags);
    wr8(HitAttackFlags, (uint8_t)(flags | 0x02));
    wr8(0x203C, 1);
    state = INC8(OBJ->state[slot]);
    e = then(e, ended_with_y(ended_load(0x5B), 0x41));
    e = then(e, ended_load((uint8_t)(flags | 0x02)));
    return then(then(e, ended_load(1)), ended_nz(state));
}

void abi_StartPhaseWhenScrolled(void)
{
    abi_ended(StartPhaseWhenScrolled(cpu.x));
}

/*
 * @routine 051B:0FE LoadOrFallAlongListByTimer
 * @run 70:B0FE
 * @stop B125
 * WHAT IT DOES: In state 0 loads the object's record and takes its two speeds from the four-byte rows of the table at $B125 by OBJ_34B8. Later it goes on as StepListByTimerWithGravity.
 * HOW: LoadObjectRecord, table reads (bank 70); a tail jump into StepListByTimerWithGravity.
 * WAITS: no.
 * Entry: X object. Exit: state 0: the last table byte, Y the row; else as StepListByTimerWithGravity.
 */
Ended LoadOrFallAlongListByTimer(uint8_t slot)
{
    uint8_t state, nz, last, timer, y;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return then(ended_load(state), StepListByTimerWithGravity(slot));
    }
    jsr_push(0xB106);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = then(ended_load(state), ended_of_record(last, nz, carry));
    timer = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(timer << 2);
    e = then(e, ended_with_carry(ended_with_y(ended_load((uint8_t)(timer << 2)), y), (timer & 0x40) != 0));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB126 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB125 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB128 + y)));
    last = rd8((uint16_t)(0xB127 + y));
    WR8(OBJ->vy_lo[slot], last);
    return then(e, ended_load(last));
}

void abi_LoadOrFallAlongListByTimer(void)
{
    abi_ended(LoadOrFallAlongListByTimer(cpu.x));
}

/*
 * @routine 051B:392 StepListB3B8ThenLand
 * @run 70:B392
 * WHAT IT DOES: Steps the animation list at $B3B8, then asks StopOnLanding with a probe height of 4; on landing the object gets an X speed of $FEE0, clears its animation step and timer and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, StopOnLanding, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column kept unless the step sets it, C passed on. Exit: the mover's registers, then the new OBJ_2800's N and Z when landed.
 */
LandWait StepListB3B8ThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry)
{
    LandWait wait = { true, { 0 }, { 0 } };
    FrameStep step;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0xB8);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB39D);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xB3A2);
    wait.mover = StopOnLanding(4, slot, y, ended_carry_or(e, carry_entry));
    jsr_pop();
    if (!mover_landed(wait.mover)) {
        return wait;
    }
    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0xE0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    phase = INC8(OBJ->f2800[slot]);
    wait.end = then(ended_load(0xE0), ended_nz(phase));
    return wait;
}

void abi_StepListB3B8ThenLand(void)
{
    land_wait_exit_shim(StepListB3B8ThenLand(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 051B:707 LandThenFaceAndRun
 * @run 70:B707
 * WHAT IT DOES: Asks StopOnLanding with a probe height of 4; on landing the object faces the player (bit 0 of SideOfPlayer), takes an X speed from the table at $B3BF by that side, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
 * HOW: StopOnLanding, SideOfPlayer, table reads (bank 70), stores.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then A 2 when landed.
 */
LandWait LandThenFaceAndRun(uint8_t slot, uint8_t column, bool carry_in)
{
    LandWait wait = { true, { 0 }, { 0 } };
    SideResult side;
    uint8_t bits, y;
    Ended e;

    jsr_push(0xB70C);
    wait.mover = StopOnLanding(4, slot, column, carry_in);
    jsr_pop();
    if (!mover_landed(wait.mover)) {
        return wait;
    }
    jsr_push(0xB711);
    side = SideOfPlayer(slot);
    jsr_pop();
    bits = (uint8_t)(side.sides & 0x01);
    WR8(OBJ->facing_left[slot], bits);
    y = (uint8_t)(bits << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB3C0 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB3BF + y)));
    WR8(OBJ->angle_lo[slot], 0);
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    e = then(ended_of_side(side), ended_load(bits));
    e = then(e, ended_with_carry(ended_with_y(ended_load(y), y), false));
    wait.end = then(e, ended_load(2));
    return wait;
}

void abi_LandThenFaceAndRun(void)
{
    land_wait_exit_shim(LandThenFaceAndRun(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 051B:324 LoadThenStateByKindByte
 * @run 70:B324
 * WHAT IT DOES: Loads the object's record and sets OBJ_2800 to 1; then by OBJ_3040: zero leaves it; positive goes to state 5 with OBJ_2800 0; negative clears OBJ_3250 and bit 6 of OBJ_30F0 and goes to state 4 with OBJ_2800 0.
 * HOW: LoadObjectRecord, tests, stores.
 * WAITS: no.
 * Entry: X object. Exit: A the state, or OBJ_3040 when zero.
 */
Ended LoadThenStateByKindByte(uint8_t slot)
{
    uint8_t nz, last, mode, flags;
    bool carry;
    Ended e;

    jsr_push(0xB327);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->f2800[slot], 1);
    mode = RD8(OBJ->f3040[slot]);
    e = then(then(e, ended_load(1)), ended_load(mode));
    if (mode == 0) {
        return e;
    }
    if ((mode & 0x80) != 0) {
        WR8(OBJ->hitbox_on[slot], 0);
        flags = (uint8_t)(RD8(OBJ->f30f0[slot]) & 0xBF);
        e = then(e, ended_load(flags));
        WR8(OBJ->f30f0[slot], flags);
        WR8(OBJ->state[slot], 4);
        WR8(OBJ->f2800[slot], 0);
        return then(e, ended_load(4));
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(5));
}

void abi_LoadThenStateByKindByte(void)
{
    abi_ended(LoadThenStateByKindByte(cpu.x));
}

/*
 * @routine 051E:090 LoadThenSinkAlongList
 * @run 71:C890
 * WHAT IT DOES: In state 0 loads the object's record, sets OBJ_2FE8 to $80, keeps its Y low in OBJ_34B8 and gives OBJ_3408 a random $00 to $0F. Each frame it adds $FE80 to its Y position (fraction and low byte) and steps the animation list at $C8D0; when the list ends it puts its Y low back from OBJ_34B8.
 * HOW: LoadObjectRecord, NextRandom, a 16-bit add, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the load of OBJ_34B8 when the list ended.
 */
Ended LoadThenSinkAlongList(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    RandomResult random;
    Sum low, high;
    uint8_t state, nz, last, value, y = y_entry;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xC898);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        WR8(OBJ->sprite_flip[slot], 0x80);
        value = RD8(OBJ->y_lo[slot]);
        WR8(OBJ->f34b8[slot], value);
        e = then(then(e, ended_load(0x80)), ended_load(value));
        jsr_push(0xC8A6);
        random = NextRandom();
        jsr_pop();
        WR8(OBJ->anim_step[slot], (uint8_t)(random.value & 0x0F));
        e = then(then(e, ended_of_random(random)), ended_load((uint8_t)(random.value & 0x0F)));
    }
    low = add_bytes(RD8(OBJ->y_frac[slot]), 0x80, false);
    WR8(OBJ->y_frac[slot], low.value);
    high = add_bytes(RD8(OBJ->y_lo[slot]), 0xFE, low.carry);
    WR8(OBJ->y_lo[slot], high.value);
    e = then(then(e, ended_of_sum(low)), ended_of_sum(high));
    WR8(ZP->temp10, 0xD0);
    WR8(ZP->temp11, 0xC8);
    jsr_push(0xC8C7);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(then(e, ended_load(0xC8)), ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    value = RD8(OBJ->f34b8[slot]);
    WR8(OBJ->y_lo[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadThenSinkAlongList(void)
{
    abi_ended(LoadThenSinkAlongList(cpu.x, cpu.y));
}

/*
 * @routine 051B:5DC WalkWhileNearPlayer
 * @run 70:B5DC
 * WHAT IT DOES: Counts toward the end of a walk: when the player is within $60 along X and $20 along Y, or when OBJ_3568's low nibble is zero and OBJ_3040 counts down to zero, it clears its animation step and timer and goes on to the next OBJ_2800. Otherwise it flips OBJ_35C0 when OBJ_3568 is zero, steps the animation list at $B62F and, on a changed frame, takes its X speed from the table at $B644 by OBJ_35C0 and moves along X.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, tests, StepObjectFrames, ApplyVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last test or step.
 */
Ended WalkWhileNearPlayer(uint8_t slot, uint8_t y_entry)
{
    PlayerDistance distance;
    FrameStep step;
    AxisMove move;
    uint8_t y = y_entry, value, phase, side;
    bool end_walk = false;
    Ended e;

    jsr_push(0xB5DF);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(ended_of_distance(distance), ended_compare(distance.distance, 0x60));
    if (distance.distance < 0x60) {
        jsr_push(0xB5E6);
        distance = DistanceYToPlayer(slot);
        jsr_pop();
        e = then(then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x20));
        end_walk = distance.distance < 0x20;
    }
    if (!end_walk) {
        value = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x0F);
        e = then(e, ended_load(value));
        if (value == 0) {
            value = DEC8(OBJ->f3040[slot]);
            e = then(e, ended_nz(value));
            end_walk = value == 0;
        }
    }
    if (end_walk) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        phase = INC8(OBJ->f2800[slot]);
        return then(e, ended_nz(phase));
    }
    value = RD8(OBJ->f3568[slot]);
    e = then(e, ended_load(value));
    if (value == 0) {
        side = (uint8_t)(RD8(OBJ->angle_lo[slot]) ^ 0x01);
        WR8(OBJ->angle_lo[slot], side);
        e = then(e, ended_load(side));
    }
    WR8(ZP->temp10, 0x2F);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB618);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_CHANGED) {
        return e;
    }
    side = RD8(OBJ->angle_lo[slot]);
    y = (uint8_t)(side << 1);
    e = then(e, ended_with_y(ended_load(y), y));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB644 + y)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB645 + y)));
    jsr_push(0xB62E);
    move = ApplyVelocityX(slot);
    jsr_pop();
    return then(e, ended_of_axis_move(move));
}

void abi_WalkWhileNearPlayer(void)
{
    abi_ended(WalkWhileNearPlayer(cpu.x, cpu.y));
}

/*
 * @routine 051B:286 LoadThenSpawnWhileScrolling
 * @run 70:B286
 * WHAT IT DOES: In state 0 loads the object's record and looks for a free pair of slots. Later, while the screen scrolls right, it counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero it looks in slots $30, $32 and $34 for one whose two kind bytes are zero, clears that object, starts a kind $22 object there at its place and sets its timer from a four-entry table at $B2DF by a random number.
 * HOW: LoadObjectRecord, a 16-bit subtract, a search loop, ClearObjectInY, StartObjectAtPlace, NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: the last test or load.
 */
Ended LoadThenSpawnWhileScrolling(uint8_t slot)
{
    RandomResult random;
    Sum low, high;
    uint8_t state, nz, last, speed, high_byte, low_byte, y, kinds, index, value;
    bool carry, search = false;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB28E);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        e = then(e, ended_of_record(last, nz, carry));
        search = true;
    } else {
        speed = rd8(ScrollSpeedX);
        e = then(e, ended_load(speed));
        if (speed == 0 || (speed & 0x80) != 0) {
            return e;
        }
        high_byte = RD8(OBJ->f34b8[slot]);
        low_byte = RD8(OBJ->f3510[slot]);
        e = then(e, ended_load((uint8_t)(high_byte | low_byte)));
        if ((high_byte | low_byte) == 0) {
            search = true;
        } else {
            low = sub_bytes(RD8(OBJ->f3510[slot]), 1, true);
            WR8(OBJ->f3510[slot], low.value);
            high = sub_bytes(RD8(OBJ->f34b8[slot]), 0, low.carry);
            WR8(OBJ->f34b8[slot], high.value);
            return then(e, ended_of_sum(high));
        }
    }
    (void)search;
    for (y = 0x30;; y = (uint8_t)(y + 2)) {
        kinds = rd8((uint16_t)(0x2750 + y));
        kinds = (uint8_t)(kinds | rd8((uint16_t)(0x2751 + y)));
        e = then(e, ended_load(kinds));
        if (kinds == 0) {
            break;
        }
        e = then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 2 - 0x36)), (uint8_t)(y + 2) >= 0x36), (uint8_t)(y + 2)));
        if ((uint8_t)(y + 2) >= 0x36) {
            return e;
        }
    }
    jsr_push(0xB2C6);
    ClearObjectInY(y);
    jsr_pop();
    jsr_push(0xB2CB);
    StartObjectAtPlace(0x22, slot, y);
    jsr_pop();
    jsr_push(0xB2CE);
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 0x03) << 1);
    e = then(then(e, ended_of_random(random)), ended_with_y(ended_load((uint8_t)(random.value & 0x03)), index));
    value = rd8((uint16_t)(0xB2DF + index));
    WR8(OBJ->f3510[slot], value);
    value = rd8((uint16_t)(0xB2E0 + index));
    WR8(OBJ->f34b8[slot], value);
    return then(e, ended_load(value));
}

void abi_LoadThenSpawnWhileScrolling(void)
{
    abi_ended(LoadThenSpawnWhileScrolling(cpu.x));
}

/*
 * @routine 051E:115 SpawnPointFromTables
 * @run 71:C915
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero it gives itself a new timer (RandomTimer10), finds a free slot from $30 and there starts a kind $AF object with this object's OBJ_3040, placed from the tables at $C97A and $CA99 by that byte and OBJ_3510's low three bits (four bytes a row: X low, X high, Y low, Y high; the X and Y low plus a random 0 to 7), and OBJ_3510 low two bits; then counts OBJ_3510 up.
 * HOW: RandomTimer10, FindFreeSlotFrom30, table reads (bank 70), NextRandom twice.
 * WAITS: no.
 * Entry: X object. Exit: the new OBJ_3510's N and Z, Y the new slot; or the count.
 */
Ended SpawnPointFromTables(uint8_t slot)
{
    RandomResult random;
    Sum sum;
    uint8_t left, found_slot, last, kind, temp, index, offset, value, y;
    bool found;
    Ended e;

    left = DEC8(OBJ->f34b8[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return e;
    }
    jsr_push(0xC91C);
    e = then(e, RandomTimer10(slot));
    jsr_pop();
    jsr_push(0xC91F);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, last, found));
    if (!found) {
        return e;
    }
    y = found_slot;
    WR8(OBJ->kind[y], 0xAF);
    push8(slot);
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->f3040[y], kind);
    WR8(ZP->temp10, kind);
    offset = (uint8_t)((RD8(OBJ->f3510[slot]) & 0x07) << 2);
    index = RD8(ZP->temp10);
    sum = add_bytes(offset, rd8((uint16_t)(0xC97A + index)), false);
    index = sum.value;
    WR8(OBJ->x_lo[y], rd8((uint16_t)(0xCA99 + index)));
    WR8(OBJ->x_hi[y], rd8((uint16_t)(0xCA9A + index)));
    WR8(OBJ->y_lo[y], rd8((uint16_t)(0xCA9B + index)));
    value = rd8((uint16_t)(0xCA9C + index));
    WR8(OBJ->y_hi[y], value);
    slot = pull8();
    jsr_push(0xC959);
    random = NextRandom();
    jsr_pop();
    sum = add_bytes((uint8_t)(random.value & 0x07), RD8(OBJ->x_lo[y]), false);
    WR8(OBJ->x_lo[y], sum.value);
    jsr_push(0xC965);
    random = NextRandom();
    jsr_pop();
    sum = add_bytes((uint8_t)(random.value & 0x07), RD8(OBJ->y_lo[y]), false);
    WR8(OBJ->y_lo[y], sum.value);
    value = (uint8_t)(RD8(OBJ->f3510[slot]) & 0x03);
    WR8(OBJ->f3510[y], value);
    e = then(ended_with_y(ended_of_sum(sum), y), ended_load(value));
    value = INC8(OBJ->f3510[slot]);
    return then(e, ended_nz(value));
}

void abi_SpawnPointFromTables(void)
{
    abi_ended(SpawnPointFromTables(cpu.x));
}

/*
 * @routine 051B:514 ResetAnimThenNextPhase
 * @run 70:B514
 * WHAT IT DOES: Clears OBJ_3250 and the animation step and timer, sets OBJ_34B8 to 4 and goes on to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A 4, N and Z from the new OBJ_2800.
 */
Ended ResetAnimThenNextPhase(uint8_t slot)
{
    uint8_t phase;

    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f34b8[slot], 4);
    phase = INC8(OBJ->f2800[slot]);
    return then(ended_load(4), ended_nz(phase));
}

void abi_ResetAnimThenNextPhase(void)
{
    abi_ended(ResetAnimThenNextPhase(cpu.x));
}

/*
 * @routine 051B:264 SetFirstObjectTimersThenCount
 * @run 70:B264
 * WHAT IT DOES: Clears the wave counter and sets the first object's OBJ_3510 to 4 and OBJ_3568 to $20 and the byte at $3BF0 to 2; counts OBJ_34B8 down and at zero sets zTemp $8C to $80, zTemp $38 to 3 and clears the hit attack flags.
 * HOW: Stores, DEC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or A 3 at zero.
 */
Ended SetFirstObjectTimersThenCount(uint8_t slot)
{
    uint8_t left;

    WR8(ZP->wave_counter, 0);
    WR8(OBJ->f3510[0], 4);
    wr8(PlayerByte3BF0, 2);
    WR8(OBJ->f3568[0], 0x20);
    left = DEC8(OBJ->f34b8[slot]);
    if (left != 0) {
        return ended_then(ended_load(0x20), ended_nz(left));
    }
    wr8(zField8C, 0x80);
    wr8(zSubStep, 3);
    wr8(HitAttackFlags, 0);
    return ended_then(ended_then(ended_load(0x20), ended_nz(left)), ended_load(3));
}

void abi_SetFirstObjectTimersThenCount(void)
{
    abi_ended(SetFirstObjectTimersThenCount(cpu.x));
}

/*
 * @routine 051B:76E FollowBlockWhileState5
 * @run 70:B76E
 * WHAT IT DOES: When the object before this one in the table is not in state 5 it sets bit 7 of OBJ_31A0 and stops. Otherwise it takes that object's place (CopyObjectPlaceBack), sets its speed to $0C00 in X and $E400 in Y (X $F600 and Y $DC00 and the next state when the object before it has frame $15), moves once and clears the speeds. (The first part ran in no script; it is written from the bytes.)
 * HOW: CopyObjectPlaceBack, stores, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the last clear (all registers as ApplyVelocity left them but A, N and Z).
 */
Ended FollowBlockWhileState5(uint8_t slot)
{
    AxisMove move;
    uint8_t before, status, last, frame, state;
    Ended e;

    before = rd8((uint16_t)(0x27A7 + slot));
    e = then(ended_load(before), ended_compare(before, 5));
    if (before != 5) {
        status = (uint8_t)(RD8(OBJ->status_flags[slot]) | 0x80);
        WR8(OBJ->status_flags[slot], status);
        return then(e, ended_load(status));
    }
    jsr_push(0xB784);
    last = CopyObjectPlaceBack((uint8_t)(slot - 1), slot);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(last), (uint8_t)(slot - 1)));
    WR8(OBJ->vx_hi[slot], 0x0C);
    WR8(OBJ->vy_hi[slot], 0xE4);
    frame = rd8((uint16_t)(0x2A67 + slot));
    e = then(then(e, ended_load(0xE4)), then(ended_load(frame), ended_compare(frame, 0x15)));
    if (frame == 0x15) {
        WR8(OBJ->vx_hi[slot], 0xF6);
        WR8(OBJ->vy_hi[slot], 0xDC);
        state = INC8(OBJ->state[slot]);
        e = then(then(e, ended_load(0xDC)), ended_nz(state));
    }
    jsr_push(0xB7A5);
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    return then(e, ended_of_axis_move(move));
}

void abi_FollowBlockWhileState5(void)
{
    abi_ended(FollowBlockWhileState5(cpu.x));
}

/*
 * @routine 051B:02F LoadThenStepListBreakAtFive
 * @run 70:B02F
 * WHAT IT DOES: In state 0 loads the object's record and sets sprite set 7 and frame 7. Later it steps the animation list at $B075; once its hits taken reach 5 it sends sound $82, records its event, becomes the kind in OBJ_3040 with OBJ_3A90 $10 and state 0, and starts a kind $9B object in a free slot from $50 at its place. (The part after the compare ran in no script; it is written from the decoded bytes.)
 * HOW: LoadObjectRecord, StepObjectFrames, SoundCommand, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
SoundEnd LoadThenStepListBreakAtFive(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    FrameStep step;
    uint8_t state, nz, last, hits, stored, event, byte, kind, found_slot, found_last, placed, y = y_entry;
    bool carry, marked, found;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xB037);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 7);
        r.end = then(then(e, ended_of_record(last, nz, carry)), ended_load(7));
        return r;
    }
    WR8(ZP->temp11, 0xB0);
    WR8(ZP->temp10, 0x75);
    jsr_push(0xB04D);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(then(then(e, ended_load(0xB0)), ended_load(0x75)), ended_of_frame_step(step, step != FRAME_HELD, y));
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(then(e, ended_load(hits)), ended_compare(hits, 5));
    if (hits < 5) {
        r.end = e;
        return r;
    }
    r.before = then(e, ended_load(0x82));
    r.sounded = true;
    r.command = 0x82;
    jsr_push(0xB059);
    r.call = SoundCommand(0x82, slot, y);
    jsr_pop();
    jsr_push(0xB05C);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = ended_of_object_event(marked, stored, event, byte);
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    e = then(then(e, ended_load(kind)), ended_load(0x10));
    jsr_push(0xB06D);
    found_slot = FindFreeSlotFrom50(&found_last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, found_last, found));
    if (!found) {
        r.end = e;
        return r;
    }
    placed = StartObjectAtPlace(0x9B, slot, found_slot);
    r.end = then(then(e, ended_load(0x9B)), ended_with_y(ended_load(placed), found_slot));
    return r;
}

void abi_LoadThenStepListBreakAtFive(void)
{
    abi_sound_end(LoadThenStepListBreakAtFive(cpu.x, cpu.y));
}

/*
 * @routine 051B:3E2 WalkTurnOrJumpOnLanding
 * @run 70:B3E2
 * WHAT IT DOES: When OBJ_3568's low five bits are zero and the player is within $24 along X and $1C along Y, the object faces the player's side (bit 0 of SideOfPlayer) in OBJ_35C0, clears its animation step and timer and goes to state 3. Otherwise it steps the animation list at $B45E and, on a changed frame, asks MoveAndLand with a probe height of 4: a wall hit gives it a Y speed of $FC80 and X speed of $FF00 and state 1; a floor stop clears its speeds; both clear the animation step and timer, with OBJ_2800 0 or 1.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, SideOfPlayer, StepObjectFrames, MoveAndLand, stores.
 * WAITS: no.
 * Entry: X object, Y the column kept unless the step sets it. Exit: see the wrapper.
 */
MoveWait WalkTurnOrJumpOnLanding(uint8_t slot, uint8_t y_entry)
{
    MoveWait r = { { 0 }, false, { 0 }, { 0 } };
    PlayerDistance distance;
    SideResult side;
    FrameStep step;
    uint8_t y = y_entry, low, bits, hits;
    bool wall;
    Ended e;

    low = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x1F);
    e = ended_load(low);
    if (low == 0) {
        jsr_push(0xB3EC);
        distance = DistanceXToPlayer(slot);
        jsr_pop();
        e = then(then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x24));
        if (distance.distance < 0x24) {
            jsr_push(0xB3F3);
            distance = DistanceYToPlayer(slot);
            jsr_pop();
            e = then(then(e, ended_of_distance(distance)), ended_compare(distance.distance, 0x1C));
            if (distance.distance < 0x1C) {
                jsr_push(0xB3FA);
                side = SideOfPlayer(slot);
                jsr_pop();
                bits = (uint8_t)(side.sides & 0x01);
                WR8(OBJ->angle_lo[slot], bits);
                WR8(OBJ->anim_step[slot], 0);
                WR8(OBJ->anim_timer[slot], 0);
                WR8(OBJ->state[slot], 3);
                r.pre = then(then(then(e, ended_of_side(side)), ended_load(bits)), ended_load(3));
                return r;
            }
        }
    }
    WR8(ZP->temp10, 0x5E);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB416);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(then(then(e, ended_load(0x5E)), ended_load(0xB4)), ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step != FRAME_CHANGED) {
        r.pre = e;
        return r;
    }
    jsr_push(0xB41D);
    r.mover = MoveAndLand(4, slot, y, true);
    jsr_pop();
    r.asked = true;
    hits = r.mover.hits;
    wall = mover_walled(r.mover);
    if (wall) {
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0x80);
        WR8(OBJ->vx_hi[slot], 0xFF);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 1);
        WR8(OBJ->f2800[slot], 0);
        r.end = ended_load(1);
        return r;
    }
    r.end = ended_load((uint8_t)(hits & 0x01));
    if ((hits & 0x01) != 0) {
        return r;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->f2800[slot], 1);
    r.end = ended_load(1);
    return r;
}

void abi_WalkTurnOrJumpOnLanding(void)
{
    MoveWait r = WalkTurnOrJumpOnLanding(cpu.x, cpu.y);

    abi_ended(r.pre);
    if (r.asked) {
        mover_exit_shim(r.mover, 0x02);
    }
    abi_ended(r.end);
}

/*
 * @routine 051A:7C5 LoadThenBreakIntoThrownPiece
 * @run 70:AFC5
 * @stop B018
 * WHAT IT DOES: In state 0 loads the object's record and sets sprite set 7, frame 1 and half height $16. Later it steps the animation list at $B018; once its hits taken reach 5 it records its event, becomes the kind in OBJ_3040 with OBJ_3A90 $10 and state 0 and starts a kind $9C object in a free slot from $50 at its place, with Y speed $FA00 for one move along Y.
 * HOW: LoadObjectRecord, StepObjectFrames, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityY.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last move, or the records' registers.
 */
Ended LoadThenBreakIntoThrownPiece(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    uint8_t state, nz, last, hits, stored, event, byte, kind, found_slot, found_last, y = y_entry;
    bool carry, marked, found;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        jsr_push(0xAFCD);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 1);
        WR8(OBJ->hitbox_half_h[slot], 0x16);
        return then(then(e, ended_of_record(last, nz, carry)), ended_load(0x16));
    }
    WR8(ZP->temp11, 0xB0);
    WR8(ZP->temp10, 0x18);
    jsr_push(0xAFE8);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = then(then(then(e, ended_load(0xB0)), ended_load(0x18)), ended_of_frame_step(step, step != FRAME_HELD, y));
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(then(e, ended_load(hits)), ended_compare(hits, 5));
    if (hits < 5) {
        return e;
    }
    jsr_push(0xAFF2);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    jsr_pop();
    e = ended_of_object_event(marked, stored, event, byte);
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], kind);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    e = then(then(e, ended_load(kind)), ended_load(0x10));
    jsr_push(0xB003);
    found_slot = FindFreeSlotFrom50(&found_last, &found);
    jsr_pop();
    e = then(e, ended_of_find_slot(found_slot, found_last, found));
    if (!found) {
        return e;
    }
    jsr_push(0xB00A);
    last = StartObjectAtPlace(0x9C, slot, found_slot);
    jsr_pop();
    WR8(OBJ->vy_hi[found_slot], 0xFA);
    e = then(then(e, ended_load(0x9C)), ended_with_y(ended_load(last), found_slot));
    e = then(e, ended_load(0xFA));
    jsr_push(0xB013);
    move = ApplyVelocityY(found_slot);
    jsr_pop();
    WR8(OBJ->vy_hi[found_slot], 0);
    return then(e, ended_of_axis_move(move));
}

void abi_LoadThenBreakIntoThrownPiece(void)
{
    abi_ended(LoadThenBreakIntoThrownPiece(cpu.x, cpu.y));
}
