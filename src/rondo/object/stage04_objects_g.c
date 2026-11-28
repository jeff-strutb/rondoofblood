/*
 * Stage 04's objects, runs CC92-D01C.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_g.h"
#include "calls_b.h"

/*
 * @routine 07DE:492 FallPaleAndLand07DE
 * @run 71:CC92
 * WHAT IT DOES: While OBJ_WORK_H counts down, a palette from $8C by it. At Y $D0-$DF bursts debris ($CCE7). Lands (height 8) to
 * the next state; then shows frame $79 or steps ($CCC0).
 * HOW: StopOnLanding with SpriteFlipMask 8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallPaleAndLand07DE(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    if (NE) {
        lsr_a();
        lsr_a();
        lsr_a();
        lsr_a();
        tay();
        clc();
        adc_a(0x8C);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        abi_nz(DEC8(OBJ->work_h[slot]));
    }
    lda(RD8(OBJ->y_lo[slot]));
    and_a(0xF0);
    cmp(cpu.a, 0xD0);
    if (EQ) {
        call_routine(0xCCB1, 0xCCE7);
    }
    lda(0x08);
    WR8(ZP->sprite_flip_mask, cpu.a);
    JSR(0xCCB8, k_stop_on_landing());
    if (CS) {
        slot = cpu.x;
        WR8(OBJ->substate[slot], 0);
        abi_nz(INC8(OBJ->state[slot]));
    }
    run_routine(0xCCC0);
}

void abi_FallPaleAndLand07DE(void)
{
    FallPaleAndLand07DE(cpu.x);
}

/*
 * @routine 07DE:4C0 PictureByRise07DE
 * @run 71:CCC0
 * WHAT IT DOES: Frame $79 while falling; while rising, steps the list at $CCD6.
 * HOW: The sign of the Y speed.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PictureByRise07DE(uint8_t slot)
{
    lda(RD8(OBJ->vy_hi[slot]));
    if (PL) {
        lda(0x79);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    list_to_temp10(0xCCD6);
    k_step_frames();
}

void abi_PictureByRise07DE(void)
{
    PictureByRise07DE(cpu.x);
}

/*
 * @routine 07DE:4E7 DebrisWithSound07DE
 * @run 71:CCE7
 * WHAT IT DOES: Sound $23 on the first screen and a debris burst with count 0.
 * HOW: A tail jump to StartDebrisBurst.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DebrisWithSound07DE(uint8_t slot)
{
    (void)slot;
    lda(0x23);
    JSR(0xCCEC, k_sound_first_screen());
    cpu.a = 0;
    run_routine(0xA90B);
}

void abi_DebrisWithSound07DE(void)
{
    DebrisWithSound07DE(cpu.x);
}

/*
 * @routine 07DE:4F0 RestThenRise07DE
 * @run 71:CCF0
 * WHAT IT DOES: Substate 0: stops with a wait of $20 and clears the object far off the screen. Then steps the list at $CD4C and,
 * the wait over, faces the player and goes on; checks the ground ($CD55).
 * HOW: Substate branches.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RestThenRise07DE(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        lda(0x20);
        WR8(OBJ->work_e[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        lda(RD8(OBJ->x_hi[slot]));
        if (EQ) {
            return;
        }
        if (PL) {
            lda(RD8(OBJ->x_lo[slot]));
            cmp(cpu.a, 0x40);
            if (CC) {
                return;
            }
            k_clear();
            return;
        }
        lda(RD8(OBJ->x_lo[slot]));
        cmp(cpu.a, 0xC0);
        if (CS) {
            return;
        }
        k_clear();
        return;
    }
    list_to_temp10(0xCD4C);
    JSR(0xCD31, k_step_frames());
    abi_nz(DEC8(OBJ->work_e[cpu.x]));
    if (EQ) {
        JSR(0xCD39, k_side());
        and_a(0x01);
        WR8(OBJ->facing_left[cpu.x], cpu.a);
        lda(0x40);
        WR8(OBJ->work_e[cpu.x], cpu.a);
        abi_nz(INC8(OBJ->state[cpu.x]));
        WR8(OBJ->substate[cpu.x], 0);
    }
    run_routine(0xCD55);
}

void abi_RestThenRise07DE(void)
{
    RestThenRise07DE(cpu.x);
}

/*
 * @routine 07DE:555 RestOffEdge07DE
 * @run 71:CD55
 * WHAT IT DOES: Without ground below (probe $2B), stops and goes to state 3.
 * HOW: ProbeTerrain with SpriteFlipMask 8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RestOffEdge07DE(uint8_t slot)
{
    lda(0x08);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x2B);
    JSR(0xCD5E, k_probe());
    and_a(0x01);
    if (NE) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_RestOffEdge07DE(void)
{
    RestOffEdge07DE(cpu.x);
}

/*
 * @routine 07DE:574 WalkThenChoose07DE
 * @run 71:CD74
 * @stop CD9B
 * WHAT IT DOES: Steps the list at $CD4C and walks the way it faces while OBJ_PIECE_PLACE counts; at zero chooses ($CD9F).
 * HOW: A speed table by facing; then the ground check.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkThenChoose07DE(uint8_t slot)
{
    list_to_temp10(0xCD4C);
    JSR(0xCD7F, k_step_frames());
    slot = cpu.x;
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (EQ) {
        run_routine(0xCD9F);
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCD9C + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCD9B + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    JSR(0xCD98, k_move_x());
    run_routine(0xCD55);
}

void abi_WalkThenChoose07DE(void)
{
    WalkThenChoose07DE(cpu.x);
}

/*
 * @routine 07DE:59F ChooseGhoulAttack07DE
 * @run 71:CD9F
 * WHAT IT DOES: Faces the player, stops, and by a random table goes to state 6, to state 4, or leaps (state 7) walking on
 * ($CDF2).
 * HOW: NextRandom's high bits.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChooseGhoulAttack07DE(uint8_t slot)
{
    JSR(0xCDA2, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    WR8(OBJ->vx_hi[cpu.x], 0);
    WR8(OBJ->vx_lo[cpu.x], 0);
    JSR(0xCDB0, k_random());
    slot = cpu.x;
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    tay();
    lda(rd8((uint16_t)(0xCE04 + cpu.y)));
    if (EQ) {
        lda(0x06);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x16);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x08);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x74);
    WR8(OBJ->frame[slot], cpu.a);
    WR8(OBJ->work_k[slot], 0);
    lda(0x07);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    run_routine(0xCDF2);
}

void abi_ChooseGhoulAttack07DE(void)
{
    ChooseGhoulAttack07DE(cpu.x);
}

/*
 * @routine 07DE:5F2 WalkSpeedByFacing07DE
 * @run 71:CDF2
 * WHAT IT DOES: The X speed the table gives for the facing.
 * HOW: Two loads.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkSpeedByFacing07DE(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCE15 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCE14 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
}

void abi_WalkSpeedByFacing07DE(void)
{
    WalkSpeedByFacing07DE(cpu.x);
}

/*
 * @routine 07DE:618 ThrowAtListEnd07DE
 * @run 71:CE18
 * WHAT IT DOES: Steps the list at $CE40; at its end starts a kind $2F object facing the same way and goes to state 4. Then the
 * ground check.
 * HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAtListEnd07DE(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xCE40);
    JSR(0xCE23, k_step_frames());
    if (EQ) {
        JSR(0xCE28, k_free_from30());
        if (CS) {
            lda(0x2F);
            JSR(0xCE2F, k_start_at_place());
            lda(RD8(OBJ->facing_left[cpu.x]));
            WR8(OBJ->facing_left[cpu.y], cpu.a);
        }
        lda(0x04);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
    }
    run_routine(0xCD55);
}

void abi_ThrowAtListEnd07DE(void)
{
    ThrowAtListEnd07DE(cpu.x);
}

/*
 * @routine 07DE:64D LeapAndBounce07DE
 * @run 71:CE4D
 * @stop CE7C
 * WHAT IT DOES: Substate 0: on landing takes the walking speed and bounces up to twice by the table; then substate 1 ($CE80).
 * HOW: StopOnLanding with height $0A.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LeapAndBounce07DE(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xCE80);
        return;
    }
    lda(0x0A);
    JSR(0xCE57, k_stop_on_landing());
    if (CC) {
        return;
    }
    call_routine(0xCE5C, 0xCDF2);
    slot = cpu.x;
    lda(RD8(OBJ->work_k[slot]));
    cmp(cpu.a, 0x02);
    if (CS) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->work_k[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCE7D + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCE7C + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->work_k[slot]));
}

void abi_LeapAndBounce07DE(void)
{
    LeapAndBounce07DE(cpu.x);
}

/*
 * @routine 07DE:680 SlowAndLandAfterLeap07DE
 * @run 71:CE80
 * WHAT IT DOES: Slows the X speed and moves with the ground. Stopped, hit a wall or fallen off: hops up in state 3 ($CEA9) or
 * rests ($CECD).
 * HOW: MoveAndLand with height $0A.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SlowAndLandAfterLeap07DE(uint8_t slot)
{
    bool hop;

    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xCEE2 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(rd8((uint16_t)(0xCEE3 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    ora_a(RD8(OBJ->vx_lo[slot]));
    hop = EQ;
    if (!hop) {
        lda(0x0A);
        JSR(0xCEA2, k_move_and_land());
        slot = cpu.x;
        hop = CS;
        if (!hop) {
            and_a(0x01);
            if (NE) {
                return;
            }
            WR8(OBJ->vx_hi[slot], 0);
            WR8(OBJ->vx_lo[slot], 0);
            WR8(OBJ->vy_hi[slot], 0);
            WR8(OBJ->vy_lo[slot], 0);
            lda(0x03);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
            return;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x0C);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_SlowAndLandAfterLeap07DE(void)
{
    SlowAndLandAfterLeap07DE(cpu.x);
}

/*
 * @routine 07DE:6E6 DieRisingThenFall07DE
 * @run 71:CEE6
 * WHAT IT DOES: Substate 0 starts a rise of -4; later bursts at Y $D0-$DF, falls, and shows the falling picture.
 * HOW: Into PictureByRise07DE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DieRisingThenFall07DE(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        lda(0xFC);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    and_a(0xF0);
    cmp(cpu.a, 0xD0);
    if (EQ) {
        call_routine(0xCF05, 0xCCE7);
    }
    JSR(0xCF08, k_gravity_unlimited());
    JSR(0xCF0B, k_move());
    run_routine(0xCCC0);
}

void abi_DieRisingThenFall07DE(void)
{
    DieRisingThenFall07DE(cpu.x);
}

/*
 * @routine 07DE:70E SinkPaleAndClear07DE
 * @run 71:CF0E
 * WHAT IT DOES: Palette $8B, moves down the Y speed and clears the object when the list at $CF27 ends.
 * HOW: ApplyVelocityY and StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SinkPaleAndClear07DE(uint8_t slot)
{
    lda(0x8B);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    JSR(0xCF16, k_move_y());
    list_to_temp10(0xCF27);
    JSR(0xCF21, k_step_frames());
    if (EQ) {
        k_clear();
    }
}

void abi_SinkPaleAndClear07DE(void)
{
    SinkPaleAndClear07DE(cpu.x);
}

/*
 * @routine 07DE:758 DroppedBoneRise07DE
 * @run 71:CF58
 * WHAT IT DOES: State 0: sound $25, loads the record, rises one step and takes the X speed by facing. Later steps the list at
 * $CF91 and moves sideways.
 * HOW: Stores and tail jumps.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DroppedBoneRise07DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        lda(0x25);
        JSR(0xCF62, k_sound_first_screen());
        JSR(0xCF65, k_load_record());
        lda(0xF5);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        JSR(0xCF6D, k_move_y());
        lda(RD8(OBJ->facing_left[cpu.x]));
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xCF8E + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xCF8D + cpu.y)));
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
        return;
    }
    list_to_temp10(0xCF91);
    JSR(0xCF8A, k_step_frames());
    k_move_x();
}

void abi_DroppedBoneRise07DE(void)
{
    DroppedBoneRise07DE(cpu.x);
}

/*
 * @routine 07DE:796 DieCheckThenByState07DE
 * @run 71:CF96
 * WHAT IT DOES: Takes a killing hit ($CFB3) and goes to the state's handler.
 * HOW: A table of handlers at $CFAB.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DieCheckThenByState07DE(uint8_t slot)
{
    (void)slot;
    call_routine(0xCF99, 0xCFB3);
    jump_by_state(cpu.x, 0xCFAB);
}

void abi_DieCheckThenByState07DE(void)
{
    DieCheckThenByState07DE(cpu.x);
}

/*
 * @routine 07DE:7B3 DieWhenHit07DECFB3
 * @run 71:CFB3
 * WHAT IT DOES: A hit under state 3 turns it into sprite set 3 rising slowly, without a hit box, in state 3.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DieWhenHit07DECFB3(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->sprite_set[slot], cpu.a);
    WR8(OBJ->frame[slot], 0);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_DieWhenHit07DECFB3(void)
{
    DieWhenHit07DECFB3(cpu.x);
}

/*
 * @routine 07DE:7E2 EnterAtRandomEdge07DE
 * @run 71:CFE2
 * WHAT IT DOES: Loads the record; a random speed and X from one of two sides, a random Y, facing the player.
 * HOW: Tables by NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EnterAtRandomEdge07DE(uint8_t slot)
{
    (void)slot;
    JSR(0xCFE5, k_load_record());
    JSR(0xCFE8, k_random());
    WR8(OBJ->work_d[cpu.x], cpu.a);
    and_a(0x01);
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD01E + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xD01D + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xD020 + cpu.y)));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xD01F + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    JSR(0xD00B, k_random());
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xD025 + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    JSR(0xD017, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
}

void abi_EnterAtRandomEdge07DE(void)
{
    EnterAtRandomEdge07DE(cpu.x);
}
