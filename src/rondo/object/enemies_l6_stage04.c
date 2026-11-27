/*
 * TEMPORARY: the stage 04D1 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return what they ended on
 * (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"


/* The arms the converted routines end on (ObjArm, enemy_objects.h); each routine's exit shim builds the registers and flags from them. */
enum { SS_ABOVE, SS_CLEAR, SS_SET };
enum { UF_STEPPED, UF_NEXT };
enum { FC_NO_SLOT, FC_DONE };
enum { PW_EARLY, PW_STARTED };
enum { LF_FALL, LF_LOADED };
enum { LW_FLYING, LW_LANDED };
enum { KB_ZERO, KB_NEGATIVE, KB_POSITIVE };
enum { WN_ENDED, WN_STEPPED, WN_MOVED };
enum { LS_STILL, LS_COUNTING, LS_FULL, LS_SPAWNED };
enum { PT_COUNTING, PT_NO_SLOT, PT_SPAWNED };
enum { FB_STOPPED, FB_MOVED };
enum { BK_LOADED, BK_STEPPED, BK_NO_SLOT, BK_STARTED };
enum { WJ_TURNED, WJ_STEPPED, WJ_MOVED };

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
ObjArm LoadRecordA04(uint8_t slot)
{
    ObjArm r = { 0 };
    bool carry;

    jsr_push(0xB240);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    return r;
}

void abi_LoadRecordA04(void)
{
    abi_ended(record_exit_shim(LoadRecordA04(cpu.x)));
}

/*
 * @routine 051B:76A LoadRecordB04
 * @run 70:B76A
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
ObjArm LoadRecordB04(uint8_t slot)
{
    ObjArm r = { 0 };
    bool carry;

    jsr_push(0xB76D);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    return r;
}

void abi_LoadRecordB04(void)
{
    abi_ended(record_exit_shim(LoadRecordB04(cpu.x)));
}

/*
 * @routine 051B:7C4 StepListB819
 * @run 70:B7C4
 * WHAT IT DOES: Steps the animation list at $B819, then asks StopOnLanding with a probe height of 6. Against a wall it
 * sets bit 7 of OBJ_31A0. On landing it takes its speeds from the four-byte row of the table at $B83A that OBJ_34B8
 * picks (X high, X low, Y high, Y low at +1, +0, +3, +2) and, while OBJ_34B8 is under 5, counts it up, with sound
 * $2C the first time.
 * HOW: StepObjectFrames, StopOnLanding (its flags kept across the wall test by PHP and PLP), table reads (bank 70),
 * SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column kept unless the step sets it, C passed on. Exit: the mover's registers, then
 * by the arm: the ORA (wall); A the AND's 0 (in the air); the CMP #5 (bounced 5 times); the INC's N and Z.
 */
enum { SL_WALLED, SL_FLYING, SL_SPED, SL_COUNTED };

typedef struct {
    ObjArm arm;          /* step and y: the frame step; first: the value the routine ended on; second: the row index */
    Mover mover;
    bool sounded;
    CommandCall sound;
} StepListB819Run;

StepListB819Run StepListB819(uint8_t slot, uint8_t y_entry, bool carry_entry)
{
    StepListB819Run run = { { 0 }, { 0 }, false, { 0 } };
    uint8_t y = y_entry, timer, row;

    WR8(ZP->temp10, 0x19);
    WR8(ZP->temp11, 0xB8);
    jsr_push(0xB7CF);
    run.arm.step = StepObjectFrames(slot, &y);
    jsr_pop();
    run.arm.y = y;
    jsr_push(0xB7D4);
    run.mover = StopOnLanding(6, slot, y, frame_step_carry_shim(run.arm, carry_entry));
    jsr_pop();
    /* PHP: the flags the mover left, which the PLP on either side of the wall test puts back. */
    abi_ended(frame_step_exit_shim(run.arm));
    mover_exit_shim(run.mover, 0x01);
    push8((uint8_t)((cpu.p | P_B) & (uint8_t)~P_T));
    if (mover_walled(run.mover)) {
        (void)pull8();
        run.arm.first = (uint8_t)(RD8(OBJ->f31a0[slot]) | 0x80);
        WR8(OBJ->f31a0[slot], run.arm.first);
        run.arm.arm = SL_WALLED;
        return run;
    }
    (void)pull8();
    run.arm.arm = SL_FLYING;
    if (!mover_landed(run.mover)) {
        return run;
    }
    timer = RD8(OBJ->f34b8[slot]);
    row = (uint8_t)(timer << 2);
    run.arm.second = row;
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB83B + row)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB83A + row)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB83D + row)));
    WR8(OBJ->vy_lo[slot], rd8((uint16_t)(0xB83C + row)));
    timer = RD8(OBJ->f34b8[slot]);
    run.arm.first = timer;
    run.arm.arm = SL_SPED;
    if (timer >= 5) {
        return run;
    }
    if (RD8(OBJ->f34b8[slot]) == 0) {
        jsr_push(0xB815);
        run.sound = SoundCommand(0x2C, slot, row);
        jsr_pop();
        run.sounded = true;
    }
    run.arm.first = INC8(OBJ->f34b8[slot]);
    run.arm.arm = SL_COUNTED;
    return run;
}

void abi_StepListB819(void)
{
    StepListB819Run run = StepListB819(cpu.x, cpu.y, (cpu.p & P_C) != 0);

    abi_ended(frame_step_exit_shim(run.arm));
    mover_exit_shim(run.mover, 0x01);
    switch (run.arm.arm) {
    case SL_WALLED:
        abi_ended(ended_load(run.arm.first));                 /* LDA, ORA #$80, STA: A and N, Z; C and V as the PLP left */
        return;
    case SL_FLYING:
        cpu.a = (uint8_t)(run.mover.hits & 0x02);             /* AND #$02 (0); then PLP puts the mover's flags back */
        return;
    case SL_SPED:
        abi_ended(ended_with_y(ended_compare(run.arm.first, 0x05), run.arm.second));
        return;
    default:
        abi_ended(ended_with_y(ended_compare((uint8_t)(run.arm.first - 1), 0x05), run.arm.second));   /* the timer before the INC */
        if (run.sounded) {
            sound_command_exit_shim(0x2C, run.sound);
        }
        abi_ended(ended_nz(run.arm.first));                   /* the INC: N and Z; C as before it */
        return;
    }
}

/*
 * @routine 051E:109 RandomTimer10
 * @run 71:C909
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number from 0 to 15 plus 16.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N, Z, C and V from the add.
 */
ObjArm RandomTimer10(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;

    jsr_push(0xC90C);
    random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)(random.value & 0x0F);
    WR8(OBJ->f34b8[slot], (uint8_t)(r.first + 0x10));
    return r;
}

/* The CLC and ADC #$10 to the random's low four bits: A, N, Z, C and V (over NextRandom's, which it replaces). */
static Ended random_timer10_exit_shim(ObjArm r)
{
    return ended_of_sum(add_bytes(r.first, 0x10, false));
}

void abi_RandomTimer10(void)
{
    abi_ended(random_timer10_exit_shim(RandomTimer10(cpu.x)));
}

/*
 * @routine 051D:4E8 CopyScrollToTimer
 * @run 71:C4E8
 * WHAT IT DOES: Sets the object's OBJ_34B8 to the scroll's X fraction of slot 4 and then to its X low of slot 4 (the second store stays).
 * HOW: Two reads and two stores.
 * WAITS: no.
 * Entry: X object. Exit: A the scroll X low, N and Z from it.
 */
ObjArm CopyScrollToTimer(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t value;

    value = RD8(SCROLL->x_frac[4]);
    WR8(OBJ->f34b8[slot], value);
    value = RD8(SCROLL->x_lo[4]);
    WR8(OBJ->f34b8[slot], value);
    r.first = value;
    return r;
}

void abi_CopyScrollToTimer(void)
{
    abi_ended(load_exit_shim(CopyScrollToTimer(cpu.x)));
}

/*
 * @routine 051B:4BF SetHitBox0C18
 * @run 70:B4BF
 * WHAT IT DOES: Sets the object's hit box: X offset 0, half width $0C, half height $18.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $18, N and Z from it.
 */
ObjArm SetHitBox0C18(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_half_w[slot], 0x0C);
    WR8(OBJ->hitbox_half_h[slot], 0x18);
    r.first = 0x18;
    return r;
}

void abi_SetHitBox0C18(void)
{
    abi_ended(load_exit_shim(SetHitBox0C18(cpu.x)));
}

/*
 * @routine 051B:5CD StartPieceAtNextSlot
 * @run 70:B5CD
 * WHAT IT DOES: Clears the object in the slot after this one, starts a kind $32 object there at this object's place and goes on to the next OBJ_2800.
 * HOW: ClearObjectInY, StartObjectAtPlace, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y the slot after; N and Z from the new OBJ_2800.
 */
ObjArm StartPieceAtNextSlot(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t to = (uint8_t)(slot + 1);

    jsr_push(0xB5D3);
    ClearObjectInY(to);
    jsr_pop();
    jsr_push(0xB5D8);
    r.second = StartObjectAtPlace(0x32, slot, to);
    jsr_pop();
    r.first = INC8(OBJ->f2800[slot]);
    r.y = to;
    return r;
}

/* A the last byte copied, Y the slot after, N and Z from the INC. */
static Ended start_piece_at_next_slot_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_a(ended_nz(r.first), r.second), r.y);
}

void abi_StartPieceAtNextSlot(void)
{
    abi_ended(start_piece_at_next_slot_exit_shim(StartPieceAtNextSlot(cpu.x)));
}

/*
 * @routine 051B:690 DrawTileScriptByHighBits
 * @run 70:B690
 * WHAT IT DOES: Draws the tile script of group 0 numbered by bits 4 to 6 of OBJ_3040 (as 0 to 7) and goes on to the next OBJ_2800.
 * HOW: DrawTileScript, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the tile script's registers, then N and Z from the new OBJ_2800.
 */
ObjArm DrawTileScriptByHighBits(uint8_t slot)
{
    ObjArm r = { 0 };

    r.second = (uint8_t)((RD8(OBJ->f3040[slot]) & 0x70) >> 4);
    jsr_push(0xB69D);
    r.run = DrawTileScript(r.second, 0);
    jsr_pop();
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The item's LDA after the shifts (C 0), DrawTileScript's exit, the INC's N and Z. */
static Ended draw_tile_script_by_high_bits_exit_shim(ObjArm r)
{
    Ended e = ended_with_carry(ended_load(r.second), false);

    e = ended_then(e, ended_of_tile_run(r.run, r.second));
    return ended_then(e, ended_nz(r.first));
}

void abi_DrawTileScriptByHighBits(void)
{
    abi_ended(draw_tile_script_by_high_bits_exit_shim(DrawTileScriptByHighBits(cpu.x)));
}

/*
 * @routine 051B:368 StepListB37FThenNext
 * @run 70:B368
 * WHAT IT DOES: Steps the animation list at $B37F; when it ends, the object's animation step and timer are cleared and it goes to the next OBJ_2800.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
 */
ObjArm StepListB37FThenNext(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x7F);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB373);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* The step's exit; when the list ended, the INC's N and Z over it. */
static Ended step_then_next_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    return r.step == FRAME_LIST_ENDED ? ended_then(e, ended_nz(r.first)) : e;
}

void abi_StepListB37FThenNext(void)
{
    abi_ended(step_then_next_exit_shim(StepListB37FThenNext(cpu.x, cpu.y)));
}

/*
 * @routine 051B:30D StateSixOnStatusBit7
 * @run 70:B30D
 * WHAT IT DOES: An object in a state under 6 whose OBJ_31A0 has bit 7 set goes to state 6 and clears OBJ_2800.
 * HOW: A compare, a test, two stores.
 * WAITS: no.
 * Entry: X object. Exit: the compare, the test, or A 6.
 */
ObjArm StateSixOnStatusBit7(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.arm = SS_ABOVE;
    if (r.first >= 6) {
        return r;
    }
    r.second = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    r.arm = SS_CLEAR;
    if (r.second == 0) {
        return r;
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    r.arm = SS_SET;
    return r;
}

/* The CMP of the state with 6; the AND's A; LDA #6. */
static Ended state_six_on_status_bit7_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 6);

    if (r.arm == SS_ABOVE) {
        return e;
    }
    e = ended_then(e, ended_load(r.second));
    return r.arm == SS_CLEAR ? e : ended_then(e, ended_load(6));
}

void abi_StateSixOnStatusBit7(void)
{
    abi_ended(state_six_on_status_bit7_exit_shim(StateSixOnStatusBit7(cpu.x)));
}

/*
 * @routine 051B:7AC StartJumpUpLeft
 * @run 70:B7AC
 * WHAT IT DOES: Sets the object's speed to $FE80 in X and $FC80 in Y (high bytes $FE and $FC) and goes on to the next state.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from the new state.
 */
ObjArm StartJumpUpLeft(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFC);
    WR8(OBJ->vy_lo[slot], 0x80);
    r.first = INC8(OBJ->state[slot]);
    return r;
}

/* LDA #$80, the INC's N and Z. */
static Ended start_jump_up_left_exit_shim(ObjArm r)
{
    return ended_then(ended_load(0x80), ended_nz(r.first));
}

void abi_StartJumpUpLeft(void)
{
    abi_ended(start_jump_up_left_exit_shim(StartJumpUpLeft(cpu.x)));
}

/*
 * @routine 051B:135 StepListByTimerWithGravity
 * @run 70:B135
 * WHAT IT DOES: Steps the animation list the table at $B14D gives for OBJ_34B8, applies gravity without a limit and moves the object.
 * HOW: Two table reads (bank 70), StepObjectFrames, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object, Y overwritten. Exit: as ApplyVelocity, Y as the step left it.
 */
ObjArm StepListByTimerWithGravity(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t index, y;

    index = (uint8_t)(RD8(OBJ->f34b8[slot]) << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xB14D + y)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB14E + y)));
    jsr_push(0xB147);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    jsr_push(0xB14A);
    (void)ApplyGravityUnlimited(slot);
    jsr_pop();
    r.move = ApplyVelocity(slot);
    r.y = y;
    return r;
}

/* ApplyVelocity's exit, Y as the step left it. */
static Ended step_list_by_timer_with_gravity_exit_shim(ObjArm r)
{
    return ended_with_y(ended_of_axis_move(r.move), r.y);
}

void abi_StepListByTimerWithGravity(void)
{
    abi_ended(step_list_by_timer_with_gravity_exit_shim(StepListByTimerWithGravity(cpu.x)));
}

/*
 * @routine 051B:3C3 StepListB37FUnlessFar
 * @run 70:B3C3
 * WHAT IT DOES: When the object's X high byte is not zero, or when the animation list at $B37F ends, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
 * HOW: StepObjectFrames, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the test or the step, or the new state's N and Z.
 */
ObjArm StepListB37FUnlessFar(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    r.first = RD8(OBJ->x_hi[slot]);
    if (r.first == 0) {
        WR8(ZP->temp10, 0x7F);
        WR8(ZP->temp11, 0xB3);
        jsr_push(0xB3D3);
        r.step = StepObjectFrames(slot, &y);
        jsr_pop();
        r.y = y;
        r.flag = true;
        r.arm = UF_STEPPED;
        if (r.step != FRAME_LIST_ENDED) {
            return r;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.second = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    r.arm = UF_NEXT;
    return r;
}

/* The LDA of the X high byte, the step when it was 0, and the state's INC when it moved on. */
static Ended step_list_b37f_unless_far_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.flag) {
        e = ended_then(e, frame_step_exit_shim(r));
    }
    return r.arm == UF_STEPPED ? e : ended_then(e, ended_nz(r.second));
}

void abi_StepListB37FUnlessFar(void)
{
    abi_ended(step_list_b37f_unless_far_exit_shim(StepListB37FUnlessFar(cpu.x, cpu.y)));
}

/*
 * @routine 051B:6ED StartFourPiecesWithCount
 * @run 70:B6ED
 * WHAT IT DOES: Starts up to four kind $33 objects at this object's place, each in a free slot from $30, and gives each its number (0 to 3) as OBJ_34B8.
 * HOW: A loop of FindFreeSlotFrom30, StartObjectAtPlace and a counter in zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, or as FindFreeSlotFrom30 when no slot was free.
 */
ObjArm StartFourPiecesWithCount(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t count, found_slot, last;
    bool found;

    WR8(ZP->temp00, 0);
    do {
        jsr_push(0xB6F2);
        found_slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        r.second = found_slot;
        r.third = last;
        if (!found) {
            r.arm = FC_NO_SLOT;
            return r;
        }
        jsr_push(0xB6F9);
        StartObjectAtPlace(0x33, slot, found_slot);
        jsr_pop();
        count = RD8(ZP->temp00);
        WR8(OBJ->f34b8[found_slot], count);
        INC8(ZP->temp00);
        count = RD8(ZP->temp00);
    } while (count < 4);
    r.arm = FC_DONE;
    r.first = count;
    return r;
}

/* No slot: FindFreeSlotFrom30's exit. Four started: the last search's Y, then the count's LDA and CMP #4. */
static Ended start_four_pieces_exit_shim(ObjArm r)
{
    Ended e = ended_of_find_slot(r.second, r.third, r.arm == FC_DONE);

    return r.arm == FC_NO_SLOT ? e : ended_then(e, ended_with_a(ended_compare(r.first, 4), r.first));
}

void abi_StartFourPiecesWithCount(void)
{
    abi_ended(start_four_pieces_exit_shim(StartFourPiecesWithCount(cpu.x)));
}

/*
 * @routine 051B:648 StepListB66BToState2
 * @run 70:B648
 * WHAT IT DOES: Steps the animation list at $B66B; when it ends the object gets an X speed of $FEE0, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or A 2 when the list ended.
 */
ObjArm StepListB66BToState2(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x6B);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB653);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0xE0);
    WR8(OBJ->angle_lo[slot], 0);
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    r.first = 2;
    return r;
}

/* The step's exit; when the list ended, the LDA of the new state over it. */
static Ended step_then_load_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    return r.step == FRAME_LIST_ENDED ? ended_then(e, ended_load(r.first)) : e;
}

void abi_StepListB66BToState2(void)
{
    abi_ended(step_then_load_exit_shim(StepListB66BToState2(cpu.x, cpu.y)));
}

/*
 * @routine 051B:241 StartPhaseWhenScrolled
 * @run 70:B241
 * WHAT IT DOES: Once the level's X position high byte is 3 or more it clears the object in slot $41, clears the wave counter, sets OBJ_34B8 to $5B, sets bit 1 of the hit attack flags and zTemp $3C to 1, and goes on to the next state.
 * HOW: A compare, ClearObjectInY, stores, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new state's N and Z (A 1).
 */
ObjArm StartPhaseWhenScrolled(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t high, flags;

    high = rd8(ScrollXHiScreen);
    r.first = high;
    r.arm = PW_EARLY;
    if (high < 3) {
        return r;
    }
    jsr_push(0xB24D);
    ClearObjectInY(0x41);
    jsr_pop();
    WR8(ZP->wave_counter, 0);
    WR8(OBJ->f34b8[slot], 0x5B);
    flags = rd8(HitAttackFlags);
    wr8(HitAttackFlags, (uint8_t)(flags | 0x02));
    wr8(0x203C, 1);
    r.third = INC8(OBJ->state[slot]);
    r.second = (uint8_t)(flags | 0x02);
    r.arm = PW_STARTED;
    return r;
}

/* The LDA and CMP #3 of the screen; once started, LDA #$5B with Y $41, the ORA's A, LDA #1, the INC's N and Z. */
static Ended start_phase_when_scrolled_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_load(r.first), ended_compare(r.first, 3));

    if (r.arm == PW_EARLY) {
        return e;
    }
    e = ended_then(e, ended_with_y(ended_load(0x5B), 0x41));
    e = ended_then(e, ended_load(r.second));
    return ended_then(ended_then(e, ended_load(1)), ended_nz(r.third));
}

void abi_StartPhaseWhenScrolled(void)
{
    abi_ended(start_phase_when_scrolled_exit_shim(StartPhaseWhenScrolled(cpu.x)));
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
ObjArm LoadOrFallAlongListByTimer(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state, timer, y;
    bool carry;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        r.arm = LF_FALL;
        r.fourth = state;
        r.sub = obj_sub(StepListByTimerWithGravity(slot));
        return r;
    }
    jsr_push(0xB106);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    timer = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(timer << 2);
    r.third = timer;
    r.y = y;
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB126 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB125 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xB128 + y)));
    r.fourth = rd8((uint16_t)(0xB127 + y));
    WR8(OBJ->vy_lo[slot], r.fourth);
    r.arm = LF_LOADED;
    return r;
}

/* Past state 0: the state's LDA, then StepListByTimerWithGravity's exit. State 0: the record, the row's ASLs (C bit 6 of the timer) and TAY, the last table byte's LDA. */
static Ended load_or_fall_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == LF_FALL) {
        return ended_then(ended_load(r.fourth), step_list_by_timer_with_gravity_exit_shim(obj_arm(r.sub)));
    }
    e = ended_then(ended_load(0), record_exit_shim(r));
    e = ended_then(e, ended_with_carry(ended_with_y(ended_load((uint8_t)(r.third << 2)), r.y), (r.third & 0x40) != 0));
    return ended_then(e, ended_load(r.fourth));
}

void abi_LoadOrFallAlongListByTimer(void)
{
    abi_ended(load_or_fall_exit_shim(LoadOrFallAlongListByTimer(cpu.x)));
}

/*
 * @routine 051B:392 StepListB3B8ThenLand
 * @run 70:B392
 * WHAT IT DOES: Steps the animation list at $B3B8, then asks StopOnLanding with a probe height of 4; on landing the object gets an X speed of $FEE0, clears its animation step and timer and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, StopOnLanding, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column kept unless the step sets it, C passed on. Exit: the mover's registers, then the new OBJ_2800's N and Z when landed.
 */
MoverArm StepListB3B8ThenLand(uint8_t slot, uint8_t y_entry, bool carry_entry)
{
    MoverArm w = { true, { 0 }, { 0 } };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xB8);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB39D);
    w.arm.step = StepObjectFrames(slot, &y);
    jsr_pop();
    w.arm.y = y;
    jsr_push(0xB3A2);
    w.mover = StopOnLanding(4, slot, y, frame_step_carry_shim(w.arm, carry_entry));
    jsr_pop();
    w.arm.arm = LW_FLYING;
    if (!mover_landed(w.mover)) {
        return w;
    }
    WR8(OBJ->vx_hi[slot], 0xFE);
    WR8(OBJ->vx_lo[slot], 0xE0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    w.arm.first = INC8(OBJ->f2800[slot]);
    w.arm.arm = LW_LANDED;
    return w;
}

/* The mover's registers; when landed, LDA #$E0 and the INC's N and Z after them. */
static LandWait step_list_b3b8_then_land_exit_shim(MoverArm w)
{
    LandWait wait = { w.asked, w.mover, ended_kept() };

    if (w.arm.arm == LW_LANDED) {
        wait.end = ended_then(ended_load(0xE0), ended_nz(w.arm.first));
    }
    return wait;
}

void abi_StepListB3B8ThenLand(void)
{
    land_wait_exit_shim(step_list_b3b8_then_land_exit_shim(StepListB3B8ThenLand(cpu.x, cpu.y, (cpu.p & P_C) != 0)));
}

/*
 * @routine 051B:707 LandThenFaceAndRun
 * @run 70:B707
 * WHAT IT DOES: Asks StopOnLanding with a probe height of 4; on landing the object faces the player (bit 0 of SideOfPlayer), takes an X speed from the table at $B3BF by that side, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
 * HOW: StopOnLanding, SideOfPlayer, table reads (bank 70), stores.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then A 2 when landed.
 */
MoverArm LandThenFaceAndRun(uint8_t slot, uint8_t column, bool carry_in)
{
    MoverArm w = { true, { 0 }, { 0 } };
    uint8_t bits, y;

    jsr_push(0xB70C);
    w.mover = StopOnLanding(4, slot, column, carry_in);
    jsr_pop();
    w.arm.arm = LW_FLYING;
    if (!mover_landed(w.mover)) {
        return w;
    }
    jsr_push(0xB711);
    w.arm.side = SideOfPlayer(slot);
    jsr_pop();
    bits = (uint8_t)(w.arm.side.sides & 0x01);
    WR8(OBJ->facing_left[slot], bits);
    y = (uint8_t)(bits << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB3C0 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB3BF + y)));
    WR8(OBJ->angle_lo[slot], 0);
    WR8(OBJ->state[slot], 2);
    WR8(OBJ->f2800[slot], 0);
    w.arm.first = bits;
    w.arm.y = y;
    w.arm.arm = LW_LANDED;
    return w;
}

/* The mover's registers; when landed, SideOfPlayer, the AND, the ASL (C 0) and TAY, and LDA #2 after them. */
static LandWait land_then_face_and_run_exit_shim(MoverArm w)
{
    LandWait wait = { w.asked, w.mover, ended_kept() };
    Ended e;

    if (w.arm.arm == LW_LANDED) {
        e = ended_then(ended_of_side(w.arm.side), ended_load(w.arm.first));
        e = ended_then(e, ended_with_carry(ended_with_y(ended_load(w.arm.y), w.arm.y), false));
        wait.end = ended_then(e, ended_load(2));
    }
    return wait;
}

void abi_LandThenFaceAndRun(void)
{
    land_wait_exit_shim(land_then_face_and_run_exit_shim(LandThenFaceAndRun(cpu.x, cpu.y, (cpu.p & P_C) != 0)));
}

/*
 * @routine 051B:324 LoadThenStateByKindByte
 * @run 70:B324
 * WHAT IT DOES: Loads the object's record and sets OBJ_2800 to 1; then by OBJ_3040: zero leaves it; positive goes to state 5 with OBJ_2800 0; negative clears OBJ_3250 and bit 6 of OBJ_30F0 and goes to state 4 with OBJ_2800 0.
 * HOW: LoadObjectRecord, tests, stores.
 * WAITS: no.
 * Entry: X object. Exit: A the state, or OBJ_3040 when zero.
 */
ObjArm LoadThenStateByKindByte(uint8_t slot)
{
    ObjArm r = { 0 };
    bool carry;

    jsr_push(0xB327);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->f2800[slot], 1);
    r.third = RD8(OBJ->f3040[slot]);
    r.arm = KB_ZERO;
    if (r.third == 0) {
        return r;
    }
    if ((r.third & 0x80) != 0) {
        WR8(OBJ->hitbox_on[slot], 0);
        r.fourth = (uint8_t)(RD8(OBJ->f30f0[slot]) & 0xBF);
        WR8(OBJ->f30f0[slot], r.fourth);
        WR8(OBJ->state[slot], 4);
        WR8(OBJ->f2800[slot], 0);
        r.arm = KB_NEGATIVE;
        return r;
    }
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    r.arm = KB_POSITIVE;
    return r;
}

/* The record, LDA #1, the kind byte's LDA; then the AND and LDA #4, or LDA #5. */
static Ended load_then_state_by_kind_byte_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_then(record_exit_shim(r), ended_load(1)), ended_load(r.third));

    if (r.arm == KB_ZERO) {
        return e;
    }
    if (r.arm == KB_NEGATIVE) {
        return ended_then(ended_then(e, ended_load(r.fourth)), ended_load(4));
    }
    return ended_then(e, ended_load(5));
}

void abi_LoadThenStateByKindByte(void)
{
    abi_ended(load_then_state_by_kind_byte_exit_shim(LoadThenStateByKindByte(cpu.x)));
}

/*
 * @routine 051E:090 LoadThenSinkAlongList
 * @run 71:C890
 * WHAT IT DOES: In state 0 loads the object's record, sets OBJ_2FE8 to $80, keeps its Y low in OBJ_34B8 and gives OBJ_3408 a random $00 to $0F. Each frame it adds $FE80 to its Y position (fraction and low byte) and steps the animation list at $C8D0; when the list ends it puts its Y low back from OBJ_34B8.
 * HOW: LoadObjectRecord, NextRandom, a 16-bit add, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the load of OBJ_34B8 when the list ended.
 */
ObjArm LoadThenSinkAlongList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    RandomResult random;
    Sum low;
    uint8_t value, y = y_entry;
    bool carry;

    r.fourth = RD8(OBJ->state[slot]);
    if (r.fourth == 0) {
        jsr_push(0xC898);
        r.first = LoadObjectRecord(slot, &r.second, &carry);
        jsr_pop();
        r.flag = carry;
        WR8(OBJ->sprite_flip[slot], 0x80);
        value = RD8(OBJ->y_lo[slot]);
        WR8(OBJ->f34b8[slot], value);
        jsr_push(0xC8A6);
        random = NextRandom();
        jsr_pop();
        WR8(OBJ->anim_step[slot], (uint8_t)(random.value & 0x0F));
    }
    r.fifth = RD8(OBJ->y_frac[slot]);
    low = add_bytes(r.fifth, 0x80, false);
    WR8(OBJ->y_frac[slot], low.value);
    r.sixth = RD8(OBJ->y_lo[slot]);
    WR8(OBJ->y_lo[slot], add_bytes(r.sixth, 0xFE, low.carry).value);
    WR8(ZP->temp10, 0xD0);
    WR8(ZP->temp11, 0xC8);
    jsr_push(0xC8C7);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.seventh = RD8(OBJ->f34b8[slot]);
    WR8(OBJ->y_lo[slot], r.seventh);
    return r;
}

/* The state's LDA; in state 0 the record (its Y 11 stands; the stores, NextRandom and loads after it are all set again
 * below); the two adds; LDA #$C8; the step; when the list ended, the LDA of OBJ_34B8. */
static Ended load_then_sink_along_list_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.fourth);
    Sum low = add_bytes(r.fifth, 0x80, false);

    if (r.fourth == 0) {
        e = ended_then(e, record_exit_shim(r));
    }
    e = ended_then(ended_then(e, ended_of_sum(low)), ended_of_sum(add_bytes(r.sixth, 0xFE, low.carry)));
    e = ended_then(ended_then(e, ended_load(0xC8)), frame_step_exit_shim(r));
    return r.step == FRAME_LIST_ENDED ? ended_then(e, ended_load(r.seventh)) : e;
}

void abi_LoadThenSinkAlongList(void)
{
    abi_ended(load_then_sink_along_list_exit_shim(LoadThenSinkAlongList(cpu.x, cpu.y)));
}

/*
 * @routine 051B:5DC WalkWhileNearPlayer
 * @run 70:B5DC
 * WHAT IT DOES: Counts toward the end of a walk: when the player is within $60 along X and $20 along Y, or when OBJ_3568's low nibble is zero and OBJ_3040 counts down to zero, it clears its animation step and timer and goes on to the next OBJ_2800. Otherwise it flips OBJ_35C0 when OBJ_3568 is zero, steps the animation list at $B62F and, on a changed frame, takes its X speed from the table at $B644 by OBJ_35C0 and moves along X.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, tests, StepObjectFrames, ApplyVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last test or step.
 */
ObjArm WalkWhileNearPlayer(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry, side;
    bool end_walk = false;

    jsr_push(0xB5DF);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    if (r.distance.distance < 0x60) {
        jsr_push(0xB5E6);
        r.distance2 = DistanceYToPlayer(slot);
        jsr_pop();
        r.flag = true;
        end_walk = r.distance2.distance < 0x20;
    }
    if (!end_walk) {
        r.other = true;
        r.first = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x0F);
        if (r.first == 0) {
            r.second = DEC8(OBJ->f3040[slot]);
            end_walk = r.second == 0;
        }
    }
    if (end_walk) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        r.third = INC8(OBJ->f2800[slot]);
        r.arm = WN_ENDED;
        return r;
    }
    r.fourth = RD8(OBJ->f3568[slot]);
    if (r.fourth == 0) {
        r.fifth = (uint8_t)(RD8(OBJ->angle_lo[slot]) ^ 0x01);
        WR8(OBJ->angle_lo[slot], r.fifth);
    }
    WR8(ZP->temp10, 0x2F);
    WR8(ZP->temp11, 0xB6);
    jsr_push(0xB618);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = WN_STEPPED;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    side = RD8(OBJ->angle_lo[slot]);
    r.sixth = (uint8_t)(side << 1);
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xB644 + r.sixth)));
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xB645 + r.sixth)));
    jsr_push(0xB62E);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.arm = WN_MOVED;
    return r;
}

/* The X distance and CMP #$60; the Y distance and CMP #$20; the nibble's LDA and the DEC; then the INC at the walk's end,
 * or the LDA of OBJ_3568, the EOR, the step, the speed index's LDA and TAY, and ApplyVelocityX. */
static Ended walk_while_near_player_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_distance(r.distance), ended_compare(r.distance.distance, 0x60));

    if (r.flag) {
        e = ended_then(ended_then(e, ended_of_distance(r.distance2)), ended_compare(r.distance2.distance, 0x20));
    }
    if (r.other) {
        e = ended_then(e, ended_load(r.first));
        if (r.first == 0) {
            e = ended_then(e, ended_nz(r.second));
        }
    }
    if (r.arm == WN_ENDED) {
        return ended_then(e, ended_nz(r.third));
    }
    e = ended_then(e, ended_load(r.fourth));
    if (r.fourth == 0) {
        e = ended_then(e, ended_load(r.fifth));
    }
    e = ended_then(e, frame_step_exit_shim(r));
    if (r.arm == WN_STEPPED) {
        return e;
    }
    return ended_then(ended_then(e, ended_with_y(ended_load(r.sixth), r.sixth)), ended_of_axis_move(r.move));
}

void abi_WalkWhileNearPlayer(void)
{
    abi_ended(walk_while_near_player_exit_shim(WalkWhileNearPlayer(cpu.x, cpu.y)));
}

/*
 * @routine 051B:286 LoadThenSpawnWhileScrolling
 * @run 70:B286
 * WHAT IT DOES: In state 0 loads the object's record and looks for a free pair of slots. Later, while the screen scrolls right, it counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero it looks in slots $30, $32 and $34 for one whose two kind bytes are zero, clears that object, starts a kind $22 object there at its place and sets its timer from a four-entry table at $B2DF by a random number.
 * HOW: LoadObjectRecord, a 16-bit subtract, a search loop, ClearObjectInY, StartObjectAtPlace, NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: the last test or load.
 */
ObjArm LoadThenSpawnWhileScrolling(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    Sum low;
    uint8_t high_byte, low_byte, y, kinds, index;
    bool carry;

    r.first = RD8(OBJ->state[slot]);
    if (r.first == 0) {
        jsr_push(0xB28E);
        r.third = LoadObjectRecord(slot, &r.second, &carry);
        jsr_pop();
        r.flag = carry;
    } else {
        r.fourth = rd8(ScrollSpeedX);
        r.arm = LS_STILL;
        if (r.fourth == 0 || (r.fourth & 0x80) != 0) {
            return r;
        }
        high_byte = RD8(OBJ->f34b8[slot]);
        low_byte = RD8(OBJ->f3510[slot]);
        r.fifth = (uint8_t)(high_byte | low_byte);
        if (r.fifth != 0) {
            r.sixth = RD8(OBJ->f3510[slot]);
            low = sub_bytes(r.sixth, 1, true);
            WR8(OBJ->f3510[slot], low.value);
            r.seventh = RD8(OBJ->f34b8[slot]);
            WR8(OBJ->f34b8[slot], sub_bytes(r.seventh, 0, low.carry).value);
            r.arm = LS_COUNTING;
            return r;
        }
    }
    for (y = 0x30;; y = (uint8_t)(y + 2)) {
        kinds = rd8((uint16_t)(0x2750 + y));
        kinds = (uint8_t)(kinds | rd8((uint16_t)(0x2751 + y)));
        r.eighth = kinds;
        r.y = y;
        if (kinds == 0) {
            break;
        }
        if ((uint8_t)(y + 2) >= 0x36) {
            r.arm = LS_FULL;
            return r;
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
    r.random = random;
    index = (uint8_t)((random.value & 0x03) << 1);
    r.ninth = index;
    WR8(OBJ->f3510[slot], rd8((uint16_t)(0xB2DF + index)));
    r.tenth = rd8((uint16_t)(0xB2E0 + index));
    WR8(OBJ->f34b8[slot], r.tenth);
    r.arm = LS_SPAWNED;
    return r;
}

/* The search's state: before it, each slot looked at gave the ORA of its two kind bytes (LDA) and, when not free, INY
 * twice and CPY #$36 (N, Z, C and Y). */
static Ended spawn_search_shim(ObjArm r, Ended e)
{
    uint8_t y;

    for (y = 0x30; y < r.y; y = (uint8_t)(y + 2)) {
        e = ended_then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(y + 2 - 0x36)), (uint8_t)(y + 2) >= 0x36), (uint8_t)(y + 2)));
    }
    e = ended_then(e, ended_load(r.eighth));
    if (r.arm == LS_FULL) {
        e = ended_then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(r.y + 2 - 0x36)), true), (uint8_t)(r.y + 2)));
    }
    return e;
}

/* The state's LDA; the record in state 0; else the speed's LDA, the ORA of the timer, the 16-bit count; then the search,
 * and after a start NextRandom, the index's LDA and TAY and the last timer byte's LDA. */
static Ended load_then_spawn_while_scrolling_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);
    Sum low;

    if (r.first == 0) {
        e = ended_then(e, ended_of_record(r.third, r.second, r.flag));
    } else {
        e = ended_then(e, ended_load(r.fourth));
        if (r.arm == LS_STILL) {
            return e;
        }
        e = ended_then(e, ended_load(r.fifth));
        if (r.arm == LS_COUNTING) {
            low = sub_bytes(r.sixth, 1, true);
            return ended_then(e, ended_of_sum(sub_bytes(r.seventh, 0, low.carry)));
        }
    }
    e = spawn_search_shim(r, e);
    if (r.arm == LS_FULL) {
        return e;
    }
    /* AND #$03, then ASL A (C 0: the value is under 4) and TAY. */
    e = ended_then(ended_then(e, ended_of_random(r.random)), ended_load((uint8_t)(r.random.value & 0x03)));
    e = ended_then(e, ended_with_y(ended_with_carry(ended_load((uint8_t)((r.random.value & 0x03) << 1)), false), r.ninth));
    return ended_then(e, ended_load(r.tenth));
}

void abi_LoadThenSpawnWhileScrolling(void)
{
    abi_ended(load_then_spawn_while_scrolling_exit_shim(LoadThenSpawnWhileScrolling(cpu.x)));
}

/*
 * @routine 051E:115 SpawnPointFromTables
 * @run 71:C915
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero it gives itself a new timer (RandomTimer10), finds a free slot from $30 and there starts a kind $AF object with this object's OBJ_3040, placed from the tables at $C97A and $CA99 by that byte and OBJ_3510's low three bits (four bytes a row: X low, X high, Y low, Y high; the X and Y low plus a random 0 to 7), and OBJ_3510 low two bits; then counts OBJ_3510 up.
 * HOW: RandomTimer10, FindFreeSlotFrom30, table reads (bank 70), NextRandom twice.
 * WAITS: no.
 * Entry: X object. Exit: the new OBJ_3510's N and Z, Y the new slot; or the count.
 */
ObjArm SpawnPointFromTables(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t found_slot, last, kind, index, offset, y;
    bool found;

    r.first = DEC8(OBJ->f34b8[slot]);
    r.arm = PT_COUNTING;
    if (r.first != 0) {
        return r;
    }
    jsr_push(0xC91C);
    r.sub = obj_sub(RandomTimer10(slot));
    jsr_pop();
    jsr_push(0xC91F);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    r.second = found_slot;
    r.third = last;
    r.arm = PT_NO_SLOT;
    if (!found) {
        return r;
    }
    y = found_slot;
    WR8(OBJ->kind[y], 0xAF);
    push8(slot);
    kind = RD8(OBJ->f3040[slot]);
    WR8(OBJ->f3040[y], kind);
    WR8(ZP->temp10, kind);
    offset = (uint8_t)((RD8(OBJ->f3510[slot]) & 0x07) << 2);
    index = RD8(ZP->temp10);
    index = (uint8_t)(offset + rd8((uint16_t)(0xC97A + index)));
    WR8(OBJ->x_lo[y], rd8((uint16_t)(0xCA99 + index)));
    WR8(OBJ->x_hi[y], rd8((uint16_t)(0xCA9A + index)));
    WR8(OBJ->y_lo[y], rd8((uint16_t)(0xCA9B + index)));
    WR8(OBJ->y_hi[y], rd8((uint16_t)(0xCA9C + index)));
    slot = pull8();
    jsr_push(0xC959);
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->x_lo[y], (uint8_t)((random.value & 0x07) + RD8(OBJ->x_lo[y])));
    jsr_push(0xC965);
    random = NextRandom();
    jsr_pop();
    r.fourth = (uint8_t)(random.value & 0x07);
    r.fifth = RD8(OBJ->y_lo[y]);
    WR8(OBJ->y_lo[y], (uint8_t)(r.fourth + r.fifth));
    r.sixth = (uint8_t)(RD8(OBJ->f3510[slot]) & 0x03);
    WR8(OBJ->f3510[y], r.sixth);
    r.y = y;
    r.seventh = INC8(OBJ->f3510[slot]);
    r.arm = PT_SPAWNED;
    return r;
}

/* The DEC's N and Z; at zero RandomTimer10's exit and the search's; after a start the last add (C, V), Y the slot, the
 * AND's A and the INC's N and Z. */
static Ended spawn_point_from_tables_exit_shim(ObjArm r)
{
    Ended e = ended_nz(r.first);

    if (r.arm == PT_COUNTING) {
        return e;
    }
    e = ended_then(ended_then(e, random_timer10_exit_shim(obj_arm(r.sub))), ended_of_find_slot(r.second, r.third, r.arm == PT_SPAWNED));
    if (r.arm == PT_NO_SLOT) {
        return e;
    }
    e = ended_then(ended_with_y(ended_of_sum(add_bytes(r.fourth, r.fifth, false)), r.y), ended_load(r.sixth));
    return ended_then(e, ended_nz(r.seventh));
}

void abi_SpawnPointFromTables(void)
{
    abi_ended(spawn_point_from_tables_exit_shim(SpawnPointFromTables(cpu.x)));
}

/*
 * @routine 051B:514 ResetAnimThenNextPhase
 * @run 70:B514
 * WHAT IT DOES: Clears OBJ_3250 and the animation step and timer, sets OBJ_34B8 to 4 and goes on to the next OBJ_2800.
 * HOW: Stores and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A 4, N and Z from the new OBJ_2800.
 */
ObjArm ResetAnimThenNextPhase(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f34b8[slot], 4);
    r.first = INC8(OBJ->f2800[slot]);
    return r;
}

/* LDA #4, the INC's N and Z. */
static Ended reset_anim_then_next_phase_exit_shim(ObjArm r)
{
    return ended_then(ended_load(4), ended_nz(r.first));
}

void abi_ResetAnimThenNextPhase(void)
{
    abi_ended(reset_anim_then_next_phase_exit_shim(ResetAnimThenNextPhase(cpu.x)));
}

/*
 * @routine 051B:264 SetFirstObjectTimersThenCount
 * @run 70:B264
 * WHAT IT DOES: Clears the wave counter and sets the first object's OBJ_3510 to 4 and OBJ_3568 to $20 and the byte at $3BF0 to 2; counts OBJ_34B8 down and at zero sets zTemp $8C to $80, zTemp $38 to 3 and clears the hit attack flags.
 * HOW: Stores, DEC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or A 3 at zero.
 */
ObjArm SetFirstObjectTimersThenCount(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(ZP->wave_counter, 0);
    WR8(OBJ->f3510[0], 4);
    wr8(PlayerByte3BF0, 2);
    WR8(OBJ->f3568[0], 0x20);
    r.first = DEC8(OBJ->f34b8[slot]);
    if (r.first != 0) {
        return r;
    }
    wr8(zField8C, 0x80);
    wr8(zSubStep, 3);
    wr8(HitAttackFlags, 0);
    return r;
}

/* LDA #$20, the DEC's N and Z; at zero LDA #3. */
static Ended set_first_object_timers_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_load(0x20), ended_nz(r.first));

    return r.first != 0 ? e : ended_then(e, ended_load(3));
}

void abi_SetFirstObjectTimersThenCount(void)
{
    abi_ended(set_first_object_timers_exit_shim(SetFirstObjectTimersThenCount(cpu.x)));
}

/*
 * @routine 051B:76E FollowBlockWhileState5
 * @run 70:B76E
 * WHAT IT DOES: When the object before this one in the table is not in state 5 it sets bit 7 of OBJ_31A0 and stops. Otherwise it takes that object's place (CopyObjectPlaceBack), sets its speed to $0C00 in X and $E400 in Y (X $F600 and Y $DC00 and the next state when the object before it has frame $15), moves once and clears the speeds. (The first part ran in no script; it is written from the bytes.)
 * HOW: CopyObjectPlaceBack, stores, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: the last clear (all registers as ApplyVelocity left them but A, N and Z).
 */
ObjArm FollowBlockWhileState5(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = rd8((uint16_t)(0x27A7 + slot));
    if (r.first != 5) {
        r.second = (uint8_t)(RD8(OBJ->status_flags[slot]) | 0x80);
        WR8(OBJ->status_flags[slot], r.second);
        r.arm = FB_STOPPED;
        return r;
    }
    jsr_push(0xB784);
    r.third = CopyObjectPlaceBack((uint8_t)(slot - 1), slot);
    jsr_pop();
    r.y = (uint8_t)(slot - 1);
    WR8(OBJ->vx_hi[slot], 0x0C);
    WR8(OBJ->vy_hi[slot], 0xE4);
    r.fourth = rd8((uint16_t)(0x2A67 + slot));
    if (r.fourth == 0x15) {
        WR8(OBJ->vx_hi[slot], 0xF6);
        WR8(OBJ->vy_hi[slot], 0xDC);
        r.fifth = INC8(OBJ->state[slot]);
    }
    jsr_push(0xB7A5);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    r.arm = FB_MOVED;
    return r;
}

/* The CMP of the state before with 5; then the ORA's A; or the copy's A with Y the slot before, LDA #$E4, the frame's CMP
 * with $15, LDA #$DC and the INC on frame $15, and ApplyVelocity's exit. */
static Ended follow_block_while_state5_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_load(r.first), ended_compare(r.first, 5));

    if (r.arm == FB_STOPPED) {
        return ended_then(e, ended_load(r.second));
    }
    e = ended_then(e, ended_with_y(ended_load(r.third), r.y));
    e = ended_then(ended_then(e, ended_load(0xE4)), ended_then(ended_load(r.fourth), ended_compare(r.fourth, 0x15)));
    if (r.fourth == 0x15) {
        e = ended_then(ended_then(e, ended_load(0xDC)), ended_nz(r.fifth));
    }
    return ended_then(e, ended_of_axis_move(r.move));
}

void abi_FollowBlockWhileState5(void)
{
    abi_ended(follow_block_while_state5_exit_shim(FollowBlockWhileState5(cpu.x)));
}

/*
 * @routine 051B:02F LoadThenStepListBreakAtFive
 * @run 70:B02F
 * WHAT IT DOES: In state 0 loads the object's record and sets sprite set 7 and frame 7. Later it steps the animation list at $B075; once its hits taken reach 5 it sends sound $82, records its event, becomes the kind in OBJ_3040 with OBJ_3A90 $10 and state 0, and starts a kind $9B object in a free slot from $50 at its place. (The part after the compare ran in no script; it is written from the decoded bytes.)
 * HOW: LoadObjectRecord, StepObjectFrames, SoundCommand, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm LoadThenStepListBreakAtFive(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t found_slot, found_last, y = y_entry;
    bool carry, found;

    r.fourth = RD8(OBJ->state[slot]);
    if (r.fourth == 0) {
        jsr_push(0xB037);
        r.first = LoadObjectRecord(slot, &r.second, &carry);
        jsr_pop();
        r.flag = carry;
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 7);
        r.arm = BK_LOADED;
        return r;
    }
    WR8(ZP->temp11, 0xB0);
    WR8(ZP->temp10, 0x75);
    jsr_push(0xB04D);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.fifth = RD8(OBJ->hit_taken[slot]);
    r.arm = BK_STEPPED;
    if (r.fifth < 5) {
        return r;
    }
    r.sounded = true;
    r.command = 0x82;
    jsr_push(0xB059);
    r.call = SoundCommand(0x82, slot, y);
    jsr_pop();
    jsr_push(0xB05C);
    r.other = RecordObjectEvent(slot, &r.sixth, &r.seventh, &r.eighth);
    jsr_pop();
    r.ninth = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], r.ninth);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    jsr_push(0xB06D);
    found_slot = FindFreeSlotFrom50(&found_last, &found);
    jsr_pop();
    r.second = found_slot;
    r.third = found_last;
    r.arm = BK_NO_SLOT;
    if (!found) {
        return r;
    }
    r.tenth = StartObjectAtPlace(0x9B, slot, found_slot);
    r.arm = BK_STARTED;
    return r;
}

/* The steps up to the hits' compare: the state's LDA, the two pointer LDAs, the step, the LDA and CMP #5 of the hits. */
static Ended break_at_five_steps_shim(ObjArm r, uint8_t high, uint8_t low)
{
    Ended e = ended_then(ended_then(ended_then(ended_load(r.fourth), ended_load(high)), ended_load(low)), frame_step_exit_shim(r));

    return ended_then(ended_then(e, ended_load(r.fifth)), ended_compare(r.fifth, 5));
}

/* After the break: RecordObjectEvent's exit, the kind's LDA, LDA #$10, the slot search. */
static Ended break_at_five_after_shim(ObjArm r)
{
    Ended e = ended_of_object_event(r.other, r.sixth, r.seventh, r.eighth);

    e = ended_then(ended_then(e, ended_load(r.ninth)), ended_load(0x10));
    return ended_then(e, ended_of_find_slot(r.second, r.third, r.arm == BK_STARTED));
}

/* State 0: the state's LDA, the record, LDA #7. Later the steps; at five hits LDA #$82 and the sound, then the rest, and LDA #$9B and the start's A with Y the slot. */
static SoundEnd load_then_step_list_break_at_five_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e;

    if (r.arm == BK_LOADED) {
        s.end = ended_then(ended_then(ended_load(r.fourth), record_exit_shim(r)), ended_load(7));
        return s;
    }
    e = break_at_five_steps_shim(r, 0xB0, 0x75);
    if (r.arm == BK_STEPPED) {
        s.end = e;
        return s;
    }
    s.before = ended_then(e, ended_load(0x82));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = break_at_five_after_shim(r);
    if (r.arm == BK_STARTED) {
        s.end = ended_then(ended_then(s.end, ended_load(0x9B)), ended_with_y(ended_load(r.tenth), r.second));
    }
    return s;
}

void abi_LoadThenStepListBreakAtFive(void)
{
    abi_sound_end(load_then_step_list_break_at_five_exit_shim(LoadThenStepListBreakAtFive(cpu.x, cpu.y)));
}

/*
 * @routine 051B:3E2 WalkTurnOrJumpOnLanding
 * @run 70:B3E2
 * WHAT IT DOES: When OBJ_3568's low five bits are zero and the player is within $24 along X and $1C along Y, the object faces the player's side (bit 0 of SideOfPlayer) in OBJ_35C0, clears its animation step and timer and goes to state 3. Otherwise it steps the animation list at $B45E and, on a changed frame, asks MoveAndLand with a probe height of 4: a wall hit gives it a Y speed of $FC80 and X speed of $FF00 and state 1; a floor stop clears its speeds; both clear the animation step and timer, with OBJ_2800 0 or 1.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, SideOfPlayer, StepObjectFrames, MoveAndLand, stores.
 * WAITS: no.
 * Entry: X object, Y the column kept unless the step sets it. Exit: see the wrapper.
 */
MoverArm WalkTurnOrJumpOnLanding(uint8_t slot, uint8_t y_entry)
{
    MoverArm w = { false, { 0 }, { 0 } };
    uint8_t y = y_entry, bits, hits;

    w.arm.first = (uint8_t)(RD8(OBJ->f3568[slot]) & 0x1F);
    if (w.arm.first == 0) {
        jsr_push(0xB3EC);
        w.arm.distance = DistanceXToPlayer(slot);
        jsr_pop();
        w.arm.fourth = 1;
        if (w.arm.distance.distance < 0x24) {
            jsr_push(0xB3F3);
            w.arm.distance2 = DistanceYToPlayer(slot);
            jsr_pop();
            w.arm.fourth = 2;
            if (w.arm.distance2.distance < 0x1C) {
                jsr_push(0xB3FA);
                w.arm.side = SideOfPlayer(slot);
                jsr_pop();
                bits = (uint8_t)(w.arm.side.sides & 0x01);
                WR8(OBJ->angle_lo[slot], bits);
                WR8(OBJ->anim_step[slot], 0);
                WR8(OBJ->anim_timer[slot], 0);
                WR8(OBJ->state[slot], 3);
                w.arm.second = bits;
                w.arm.arm = WJ_TURNED;
                return w;
            }
        }
    }
    WR8(ZP->temp10, 0x5E);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB416);
    w.arm.step = StepObjectFrames(slot, &y);
    jsr_pop();
    w.arm.y = y;
    w.arm.arm = WJ_STEPPED;
    if (w.arm.step != FRAME_CHANGED) {
        return w;
    }
    jsr_push(0xB41D);
    w.mover = MoveAndLand(4, slot, y, true);
    jsr_pop();
    w.asked = true;
    hits = w.mover.hits;
    if (mover_walled(w.mover)) {
        WR8(OBJ->vy_hi[slot], 0xFC);
        WR8(OBJ->vy_lo[slot], 0x80);
        WR8(OBJ->vx_hi[slot], 0xFF);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 1);
        WR8(OBJ->f2800[slot], 0);
        w.arm.third = 1;
        w.arm.arm = WJ_MOVED;
        return w;
    }
    w.arm.third = (uint8_t)(hits & 0x01);
    w.arm.arm = WJ_MOVED;
    if ((hits & 0x01) != 0) {
        return w;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->f2800[slot], 1);
    w.arm.third = 1;
    return w;
}

/* Before the mover: the low bits' LDA, the X distance and CMP #$24, the Y distance and CMP #$1C as far as the tests went;
 * turned, SideOfPlayer, the AND and LDA #3; else LDA #$5E, LDA #$B4 and the step. When the mover was asked, its registers
 * stand for all of that (the wrapper shows none of it), then the LDA the routine ended on. */
static MoveWait walk_turn_or_jump_on_landing_exit_shim(MoverArm w)
{
    MoveWait m = { { 0 }, false, { 0 }, { 0 } };
    ObjArm r = w.arm;
    Ended e = ended_load(r.first);

    if (r.fourth >= 1) {
        e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x24));
    }
    if (r.fourth >= 2) {
        e = ended_then(ended_then(e, ended_of_distance(r.distance2)), ended_compare(r.distance2.distance, 0x1C));
    }
    if (r.arm == WJ_TURNED) {
        m.pre = ended_then(ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.second)), ended_load(3));
        return m;
    }
    if (r.arm == WJ_MOVED) {
        m.asked = w.asked;
        m.mover = w.mover;
        m.end = ended_load(r.third);
        return m;
    }
    m.pre = ended_then(ended_then(ended_then(e, ended_load(0x5E)), ended_load(0xB4)), frame_step_exit_shim(r));
    return m;
}

void abi_WalkTurnOrJumpOnLanding(void)
{
    MoveWait r = walk_turn_or_jump_on_landing_exit_shim(WalkTurnOrJumpOnLanding(cpu.x, cpu.y));

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
ObjArm LoadThenBreakIntoThrownPiece(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t found_slot, found_last, y = y_entry;
    bool carry, found;

    r.fourth = RD8(OBJ->state[slot]);
    if (r.fourth == 0) {
        jsr_push(0xAFCD);
        r.first = LoadObjectRecord(slot, &r.second, &carry);
        jsr_pop();
        r.flag = carry;
        WR8(OBJ->sprite_set[slot], 7);
        WR8(OBJ->frame[slot], 1);
        WR8(OBJ->hitbox_half_h[slot], 0x16);
        r.arm = BK_LOADED;
        return r;
    }
    WR8(ZP->temp11, 0xB0);
    WR8(ZP->temp10, 0x18);
    jsr_push(0xAFE8);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.fifth = RD8(OBJ->hit_taken[slot]);
    r.arm = BK_STEPPED;
    if (r.fifth < 5) {
        return r;
    }
    jsr_push(0xAFF2);
    r.other = RecordObjectEvent(slot, &r.sixth, &r.seventh, &r.eighth);
    jsr_pop();
    r.ninth = RD8(OBJ->f3040[slot]);
    WR8(OBJ->kind[slot], r.ninth);
    WR8(OBJ->timer[slot], 0x10);
    WR8(OBJ->state[slot], 0);
    jsr_push(0xB003);
    found_slot = FindFreeSlotFrom50(&found_last, &found);
    jsr_pop();
    r.second = found_slot;
    r.third = found_last;
    r.arm = BK_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xB00A);
    r.tenth = StartObjectAtPlace(0x9C, slot, found_slot);
    jsr_pop();
    WR8(OBJ->vy_hi[found_slot], 0xFA);
    jsr_push(0xB013);
    r.move = ApplyVelocityY(found_slot);
    jsr_pop();
    WR8(OBJ->vy_hi[found_slot], 0);
    r.arm = BK_STARTED;
    return r;
}

/* State 0: the state's LDA, the record, LDA #$16. Later the steps; at five hits the rest, then LDA #$9C, the start's A with
 * Y the slot, LDA #$FA and ApplyVelocityY's exit. */
static Ended load_then_break_into_thrown_piece_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == BK_LOADED) {
        return ended_then(ended_then(ended_load(r.fourth), record_exit_shim(r)), ended_load(0x16));
    }
    e = break_at_five_steps_shim(r, 0xB0, 0x18);
    if (r.arm == BK_STEPPED) {
        return e;
    }
    e = break_at_five_after_shim(r);
    if (r.arm == BK_NO_SLOT) {
        return e;
    }
    e = ended_then(ended_then(e, ended_load(0x9C)), ended_with_y(ended_load(r.tenth), r.second));
    return ended_then(ended_then(e, ended_load(0xFA)), ended_of_axis_move(r.move));
}

void abi_LoadThenBreakIntoThrownPiece(void)
{
    abi_ended(load_then_break_into_thrown_piece_exit_shim(LoadThenBreakIntoThrownPiece(cpu.x, cpu.y)));
}
