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
