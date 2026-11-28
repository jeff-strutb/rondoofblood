/*
 * Stage 05's objects, runs D81C-DDD8.
 */
#include "stage_obj_ops.h"
#include "stage05_objects_e.h"
#include "calls_b.h"

/*
 * @routine 0940:01C BySubstate0940D81C
 * @run 71:D81C
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $D82E.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate0940D81C(uint8_t slot)
{
    jump_by_substate(slot, 0xD82E);
}

void abi_BySubstate0940D81C(void)
{
    BySubstate0940D81C(cpu.x);
}

/*
 * @routine 0940:034 PickMinionKind0940
 * @run 71:D834
 * WHAT IT DOES: Picks one of two kinds at random and goes to the next substate.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PickMinionKind0940(uint8_t slot)
{
    (void)slot;
    JSR(0xD837, k_random());
    and_a(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_PickMinionKind0940(void)
{
    PickMinionKind0940(cpu.x);
}

/*
 * @routine 0940:040 StartMinion0940
 * @run 71:D840
 * @stop D86D
 * WHAT IT DOES: Starts the picked kind in an empty slot of $34-$37, facing this way, each keeping the other's slot; with none, state 2 ($D86F).
 * HOW: A walk of the kinds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartMinion0940(uint8_t slot)
{
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xD86D + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    ldy(0x34);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x38);
        if (CS) {
            run_routine(0xD86F);
            return;
        }
    }
    lda(RD8(ZP->temp00));
    JSR(0xD85B, k_start_at_place());
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    txa();
    WR8(OBJ->work_d[cpu.y], cpu.a);
    tya();
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartMinion0940(void)
{
    StartMinion0940(cpu.x);
}

/*
 * @routine 0940:06F EndSummon0940
 * @run 71:D86F
 * WHAT IT DOES: State 2, substate 0.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EndSummon0940(uint8_t slot)
{
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_EndSummon0940(void)
{
    EndSummon0940(cpu.x);
}

/*
 * @routine 0940:078 WaitWhileMinionLives0940
 * @run 71:D878
 * WHAT IT DOES: Once the minion's slot no longer holds its kind, state 2.
 * HOW: A compare of the kind.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WaitWhileMinionLives0940(uint8_t slot)
{
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xD86D + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    ldy(RD8(OBJ->work_f[slot]));
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, RD8(ZP->temp00));
    if (NE) {
        run_routine(0xD86F);
    }
}

void abi_WaitWhileMinionLives0940(void)
{
    WaitWhileMinionLives0940(cpu.x);
}

/*
 * @routine 0940:08B DripEmitter0940
 * @run 71:D88B
 * WHAT IT DOES: State 0 loads it off the right with a wait. Every wait starts a kind $3C drop with a random argument in an empty slot of $30-$33.
 * HOW: ClearObjectInY, StartObjectAtPlace, NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DripEmitter0940(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD893, k_load_record());
        slot = cpu.x;
        lda(0x01);
        WR8(OBJ->x_hi[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    lda(0xE0);
    WR8(OBJ->work_d[slot], cpu.a);
    ldy(0x30);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x34);
        if (CS) {
            return;
        }
    }
    JSR(0xD8BD, k_clear_in_y());
    lda(0x3C);
    JSR(0xD8C2, k_start_at_place());
    JSR(0xD8C5, k_random());
    and_a(0x07);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
}

void abi_DripEmitter0940(void)
{
    DripEmitter0940(cpu.x);
}

/*
 * @routine 0940:0CB SplashOfDrop0940
 * @run 71:D8CB
 * WHAT IT DOES: State 0 loads it. While its drop (OBJ_WORK_D) is kind $3C, steps the list at $D904 with the frame's X offset; else cleared.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SplashOfDrop0940(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD8D3, k_load_record());
        lda(0x02);
        WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
        return;
    }
    ldy(RD8(OBJ->work_d[slot]));
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, 0x3C);
    if (NE) {
        k_clear();
        return;
    }
    list_to_temp10(0xD904);
    JSR(0xD8F1, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    sec();
    sbc_a(0x8A);
    tay();
    lda(rd8((uint16_t)(0xD925 + cpu.y)));
    WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
}

void abi_SplashOfDrop0940(void)
{
    SplashOfDrop0940(cpu.x);
}

/*
 * @routine 0940:129 TrailOfDrop0940
 * @run 71:D929
 * WHAT IT DOES: As SplashOfDrop0940 with the list at $D963 and both offsets.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TrailOfDrop0940(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    ldy(RD8(OBJ->work_d[slot]));
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, 0x3C);
    if (NE) {
        k_clear();
        return;
    }
    list_to_temp10(0xD963);
    JSR(0xD949, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    sec();
    sbc_a(0x8E);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD98C + cpu.y)));
    WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xD98D + cpu.y)));
    WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
}

void abi_TrailOfDrop0940(void)
{
    TrailOfDrop0940(cpu.x);
}

/*
 * @routine 0940:19C ReadyDoorThenExit0940
 * @run 71:D99C
 * WHAT IT DOES: Sets the door ready and runs ExitWhenDoorHit093A.
 * HOW: A store and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ReadyDoorThenExit0940(uint8_t slot)
{
    (void)slot;
    lda(0x01);
    WR8(PVARS->door_ready, cpu.a);
    run_routine(0xB0CF);
}

void abi_ReadyDoorThenExit0940(void)
{
    ReadyDoorThenExit0940(cpu.x);
}

/*
 * @routine 0940:1A4 ReadyDoorThenTrigger0940
 * @run 71:D9A4
 * WHAT IT DOES: Sets the door ready and runs the stage trigger object ($8AFC).
 * HOW: A store and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ReadyDoorThenTrigger0940(uint8_t slot)
{
    (void)slot;
    lda(0x01);
    WR8(PVARS->door_ready, cpu.a);
    run_routine(0x8AFC);
}

void abi_ReadyDoorThenTrigger0940(void)
{
    ReadyDoorThenTrigger0940(cpu.x);
}

/*
 * @routine 0940:1AC ByState0940D9AC
 * @run 71:D9AC
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D9BE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState0940D9AC(uint8_t slot)
{
    jump_by_state(slot, 0xD9BE);
}

void abi_ByState0940D9AC(void)
{
    ByState0940D9AC(cpu.x);
}

/*
 * @routine 0940:1C6 ElevatorWaitForRider0940
 * @run 71:D9C6
 * WHAT IT DOES: A platform: frame 7 for odd numbers. Once the player stands on it ($30 by $16), starts a kind $33 below and: even numbers sink at $0140 with a wait; others at $DA19.
 * HOW: ContactWithPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ElevatorWaitForRider0940(uint8_t slot)
{
    {
        uint8_t arg = RD8(OBJ->start_arg[slot]);

        abi_flag(P_Z, (arg & 0x01) == 0);
        abi_flag(P_N, (arg & 0x80) != 0);
        abi_flag(P_V, (arg & 0x40) != 0);
    }
    if (NE) {
        lda(0x07);
        WR8(OBJ->frame[slot], cpu.a);
    }
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xD9DC, 0xAA35);
    if (CC) {
        return;
    }
    ldy(0x10);
    JSR(0xD9E3, k_clear_in_y());
    lda(0x33);
    JSR(0xD9E8, k_start_at_place());
    slot = cpu.x;
    {
        uint8_t arg = RD8(OBJ->start_arg[slot]);

        abi_flag(P_Z, (arg & 0x01) == 0);
        abi_flag(P_N, (arg & 0x80) != 0);
        abi_flag(P_V, (arg & 0x40) != 0);
    }
    if (NE) {
        run_routine(0xDA19);
        return;
    }
    lda(0x01);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    run_routine(0xDA02);
}

void abi_ElevatorWaitForRider0940(void)
{
    ElevatorWaitForRider0940(cpu.x);
}

/*
 * @routine 0940:202 HoldPlayerForRide0940
 * @run 71:DA02
 * WHAT IT DOES: The player's piece place A, start pose 2 and swoop count 0; loop flag bit 0, then on ($DA15).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoldPlayerForRide0940(uint8_t slot)
{
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    WR8(PVARS2->start_pose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    ora_a(0x01);
    WR8(OBJ->loop_flags[slot], cpu.a);
    run_routine(0xDA15);
}

void abi_HoldPlayerForRide0940(void)
{
    HoldPlayerForRide0940(cpu.x);
}

/*
 * @routine 0940:215 NextStateOfPlatform0940
 * @run 71:DA15
 * WHAT IT DOES: Goes on.
 * HOW: An increment.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextStateOfPlatform0940(uint8_t slot)
{
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_NextStateOfPlatform0940(void)
{
    NextStateOfPlatform0940(cpu.x);
}

/*
 * @routine 0940:219 HoldPlayerForOtherRide0940
 * @run 71:DA19
 * WHAT IT DOES: Holds the player with $40 ($DA02), marks the other object's argument and goes on.
 * HOW: HoldPlayerForRide0940.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoldPlayerForOtherRide0940(uint8_t slot)
{
    (void)slot;
    lda(0x40);
    call_routine(0xDA1D, 0xDA02);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    run_routine(0xDA15);
}

void abi_HoldPlayerForOtherRide0940(void)
{
    HoldPlayerForOtherRide0940(cpu.x);
}

/*
 * @routine 0940:224 FloatThenNext0940
 * @run 71:DA24
 * WHAT IT DOES: With the player's weight, floats to Y $9C; the wait over, on.
 * HOW: FloatToLine0940.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FloatThenNext0940(uint8_t slot)
{
    (void)slot;
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xDA2F, 0xAA35);
    lda(0x9C);
    call_routine(0xDA34, 0xDA9A);
    abi_nz(DEC8(OBJ->work_e[cpu.x]));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_FloatThenNext0940(void)
{
    FloatThenNext0940(cpu.x);
}

/*
 * @routine 0940:23D RideAndCarryPlayer0940
 * @run 71:DA3D
 * WHAT IT DOES: Follows the scroll bands ($DB20), floats to Y $9C and drifts ($DAC6). After its wait, a standing player is pushed along, with a dip as they step on.
 * HOW: ContactWithPlayer and zPlayerPush.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RideAndCarryPlayer0940(uint8_t slot)
{
    call_routine(0xDA40, 0xDB20);
    lda(0x9C);
    call_routine(0xDA45, 0xDA9A);
    call_routine(0xDA48, 0xDAC6);
    slot = cpu.x;
    lda(RD8(OBJ->work_e[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_e[slot]));
        return;
    }
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xDA5C, 0xAA35);
    slot = cpu.x;
    if (CC) {
        lda(RD8(OBJ->work_d[slot]));
        if (NE) {
            lda(0x08);
            WR8(OBJ->work_e[slot], cpu.a);
        }
        WR8(OBJ->work_d[slot], 0);
        return;
    }
    lda(RD8(ZP->player_push));
    ora_a(0x10);
    and_a(0xF9);
    WR8(ZP->player_push, cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    WR8(ZP->push_x.lo, cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    WR8(ZP->push_x.hi, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    if (EQ) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0x9C);
        if (EQ) {
            lda(0x01);
            WR8(OBJ->vy_hi[slot], cpu.a);
            lda(0x40);
            WR8(OBJ->vy_lo[slot], cpu.a);
        }
    }
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_RideAndCarryPlayer0940(void)
{
    RideAndCarryPlayer0940(cpu.x);
}

/*
 * @routine 0940:2C6 DriftWithTheCurrent0940
 * @run 71:DAC6
 * WHAT IT DOES: Steps the list at $DB15 (odd numbers' frames 6 on). At the argument's end frame turns back at -$0190; else eases toward rest from the left, and moves sideways.
 * HOW: ApplyVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriftWithTheCurrent0940(uint8_t slot)
{
    bool turn = false;

    list_to_temp10_high_first(0xDB15);
    JSR(0xDAD1, k_step_frames());
    slot = cpu.x;
    if (CS) {
        ldy(RD8(OBJ->start_arg[slot]));
        if (NE) {
            lda(RD8(OBJ->frame[slot]));
            clc();
            adc_a(0x06);
            WR8(OBJ->frame[slot], cpu.a);
        }
        lda(RD8(OBJ->frame[slot]));
        cmp(cpu.a, rd8((uint16_t)(0xDB1E + cpu.y)));
        turn = EQ;
    }
    if (turn) {
        abi_nz(INC8(OBJ->work_f[slot]));
        lda(0xFE);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0x70);
        WR8(OBJ->vx_lo[slot], cpu.a);
    } else {
        lda(RD8(OBJ->vx_lo[slot]));
        clc();
        adc_a(0x04);
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(RD8(OBJ->vx_hi[slot]));
        adc_a(0x00);
        WR8(OBJ->vx_hi[slot], cpu.a);
        if (PL) {
            WR8(OBJ->vx_hi[slot], 0);
            WR8(OBJ->vx_lo[slot], 0);
        }
    }
    JSR(0xDB14, k_move_x());
}

void abi_DriftWithTheCurrent0940(void)
{
    DriftWithTheCurrent0940(cpu.x);
}

/*
 * @routine 0940:320 MoveAgainstScrollBand0940
 * @run 71:DB20
 * WHAT IT DOES: Moves the object against its scroll band's movement this frame, in X and Y.
 * HOW: The band's scroll less the main one, subtracted from the position.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void MoveAgainstScrollBand0940(uint8_t slot)
{
    lda(RD8(OBJ->scroll_band[slot]));
    and_a(0x7F);
    tay();
    lda(RD8(SCROLL->x_frac[cpu.y]));
    sec();
    sbc_a(RD8(BANDS->band_first[cpu.y]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLL->x_lo[cpu.y]));
    sbc_a(RD8(BANDS->bgx_lo[cpu.y]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(SCROLL->y_frac[cpu.y]));
    sec();
    sbc_a(RD8(BANDS->band_fourth[cpu.y]));
    WR8(ZP->temp02, cpu.a);
    wr8((uint16_t)(ScrollBandYFracStep + cpu.y), cpu.a);
    lda(RD8(SCROLL->y_lo[cpu.y]));
    sbc_a(RD8(BANDS->bgy_lo[cpu.y]));
    WR8(ZP->temp03, cpu.a);
    WR8(ZP->temp10, 0);
    lda(RD8(ZP->temp01));
    if (MI) {
        abi_nz(DEC8(ZP->temp10));
    }
    lda(RD8(OBJ->x_frac[slot]));
    sec();
    sbc_a(RD8(ZP->temp00));
    WR8(OBJ->x_frac[slot], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    sbc_a(RD8(ZP->temp01));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    sbc_a(RD8(ZP->temp10));
    WR8(OBJ->x_hi[slot], cpu.a);
    WR8(ZP->temp10, 0);
    lda(RD8(ZP->temp03));
    if (MI) {
        abi_nz(DEC8(ZP->temp10));
    }
    lda(RD8(OBJ->y_frac[slot]));
    sec();
    sbc_a(RD8(ZP->temp02));
    WR8(OBJ->y_frac[slot], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    sec();
    sbc_a(RD8(ZP->temp03));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    sbc_a(RD8(ZP->temp10));
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_MoveAgainstScrollBand0940(void)
{
    MoveAgainstScrollBand0940(cpu.x);
}

/*
 * @routine 0940:4C6 SpawnAtScreensEnds0940
 * @run 71:DCC6
 * WHAT IT DOES: On map screens other than 0 and 3, every wait starts a kind $46 object in a free slot from $20; the next wait is this one's argument, or $20 before screen 3.
 * HOW: FindFreeSlotFrom20.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpawnAtScreensEnds0940(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(RD8(SCROLLE->x_hi_screen[0]));
    if (EQ) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        return;
    }
    JSR(0xDCE3, k_free_from20());
    if (CC) {
        return;
    }
    lda(0x46);
    WR8(OBJ->kind[cpu.y], cpu.a);
    lda(RD8(OBJ->start_arg[cpu.x]));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    lda(RD8(SCROLLE->x_hi_screen[0]));
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    lda(0x20);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_SpawnAtScreensEnds0940(void)
{
    SpawnAtScreensEnds0940(cpu.x);
}

/*
 * @routine 0940:4FD FishLeapOut0940
 * @run 71:DCFD
 * @stop DD3E
 * WHAT IT DOES: State 0: loads it with attribute $8F at a random place, facing the player, leaping at -$0A with sound $98. Later at $DD4A.
 * HOW: Position tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FishLeapOut0940(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xDD4A);
        return;
    }
    JSR(0xDD05, k_load_record());
    lda(0x8F);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    JSR(0xDD0D, k_random());
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xDD3E + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    JSR(0xDD19, k_random());
    and_a(0x07);
    tay();
    lda(rd8((uint16_t)(0xDD42 + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    JSR(0xDD25, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    lda(0xF6);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    lda(0x40);
    WR8(OBJ->work_h[cpu.x], cpu.a);
    lda(0x98);
    k_sound();
}

void abi_FishLeapOut0940(void)
{
    FishLeapOut0940(cpu.x);
}

/*
 * @routine 0940:54A FishFlight0940
 * @run 71:DD4A
 * @stop DDAD
 * WHAT IT DOES: State 1: a hit turns it into sprite set 3 rising slowly; else its paling counts out, at Y $A0-$AF bursting debris, moving and falling while the list at $DDAD steps. Later at $DDC0.
 * HOW: StartDebrisBurst.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FishFlight0940(uint8_t slot)
{
    dec_a();
    if (NE) {
        run_routine(0xDDC0);
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (NE) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->hitbox_on[slot], 0);
        lda(0x03);
        WR8(OBJ->sprite_set[slot], cpu.a);
        WR8(OBJ->frame[slot], 0);
        lda(0xFE);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x80);
        WR8(OBJ->vy_lo[slot], cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    if (NE) {
        lsr_a();
        lsr_a();
        lsr_a();
        lsr_a();
        clc();
        adc_a(0x8C);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        abi_nz(DEC8(OBJ->work_h[slot]));
    }
    lda(RD8(OBJ->y_lo[slot]));
    and_a(0xF0);
    cmp(cpu.a, 0xA0);
    if (EQ) {
        lda(0x23);
        JSR(0xDD93, k_sound());
        cpu.a = 0;
        call_routine(0xDD97, 0xA90B);
        lda(0x8F);
        WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    }
    JSR(0xDD9F, k_move());
    JSR(0xDDA2, k_gravity());
    list_to_temp10(0xDDAD);
    k_step_frames();
}

void abi_FishFlight0940(void)
{
    FishFlight0940(cpu.x);
}

/*
 * @routine 0940:5C0 FishSink0940
 * @run 71:DDC0
 * WHAT IT DOES: Palette $8B, moves down and is cleared when the list at $DDD9 ends.
 * HOW: ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FishSink0940(uint8_t slot)
{
    lda(0x8B);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    JSR(0xDDC8, k_move_y());
    list_to_temp10(0xDDD9);
    JSR(0xDDD3, k_step_frames());
    if (EQ) {
        k_clear();
    }
}

void abi_FishSink0940(void)
{
    FishSink0940(cpu.x);
}
