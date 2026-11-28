/*
 * Stage 04's objects, runs AA35-B300.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_a.h"
#include "calls_b.h"

void abi_VecStopSound(void);   /* system/vectors.c */

/*
 * @routine 07DA:235 BounceOffWallsHigh07DA
 * @run 70:AA35
 * WHAT IT DOES: Moves sideways; when the terrain ahead is solid at height 9 and height 2, turns back and moves again. Carry set when
 * it turned.
 * HOW: ApplyVelocityX, two ProbeTerrain on the side of the movement, the answers in TileHit.
 * WAITS: no.
 * Entry: X object, Y and C for the probe. Exit: C whether it turned.
 */
void BounceOffWallsHigh07DA(uint8_t slot)
{
    JSR(0xAA38, k_move_x());
    lda(0x09);
    JSR(0xAA3D, k_probe());
    slot = cpu.x;
    lda(RD8(OBJ->vx_hi[slot]));
    rol_a();
    rol_a();
    and_a(0x01);
    eor_a(0x01);
    tay();
    lda(rd8((uint16_t)(TileHit + 1 + cpu.y)));
    and_a(0x01);
    if (NE) {
        lda(0x02);
        JSR(0xAA53, k_probe());
        slot = cpu.x;
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        and_a(0x01);
        eor_a(0x01);
        asl_a();
        tay();
        lda(rd8((uint16_t)(TileHit + cpu.y)));
        ora_a(rd8((uint16_t)(TileHit + 1 + cpu.y)));
        and_a(0x02);
        if (EQ) {
            clc();
            return;
        }
    }
    JSR(0xAA6D, k_negate_x());
    JSR(0xAA70, k_move_x());
    sec();
}

void abi_BounceOffWallsHigh07DA(void)
{
    BounceOffWallsHigh07DA(cpu.x);
}

/*
 * @routine 07DA:3DD ByState07DAABDD
 * @run 70:ABDD
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $ABEF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DAABDD(uint8_t slot)
{
    jump_by_state(slot, 0xABEF);
}

void abi_ByState07DAABDD(void)
{
    ByState07DAABDD(cpu.x);
}

/*
 * @routine 07DA:3F7 LoadAndLockPlayer07DA
 * @run 70:ABF7
 * WHAT IT DOES: Loads the record and records the event; zField92 cleared, a wait of $40, and the player's piece place $10, start
 * pose 2, swoop count 0 and attack flags 2.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadAndLockPlayer07DA(uint8_t slot)
{
    (void)slot;
    JSR(0xABFA, k_load_record());
    JSR(0xABFD, k_record_event());
    WR8(ZP->field92, 0);
    lda(0x40);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    lda(0x10);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    WR8(PVARS2->start_pose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(0x02);
    wr8(HitAttackFlags, cpu.a);
}

void abi_LoadAndLockPlayer07DA(void)
{
    LoadAndLockPlayer07DA(cpu.x);
}

/*
 * @routine 07DA:417 StopSoundWhenTimeRuns07DA
 * @run 70:AC17
 * WHAT IT DOES: Keeps the player's piece place at $10 while time is slowed; once not, at $C0 stops the sound, plays $56, clears
 * the attack flags and goes on.
 * HOW: StopSound and SoundCommand.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StopSoundWhenTimeRuns07DA(uint8_t slot)
{
    (void)slot;
    lda(0x10);
    WR8(OBJ->work_e[0], cpu.a);
    lda(RD8(PVARS->time_slow_mode));
    if (NE) {
        return;
    }
    lda(0xC0);
    WR8(OBJ->work_e[0], cpu.a);
    JSR(0xAC29, abi_VecStopSound());
    lda(0x56);
    JSR(0xAC2E, k_sound());
    wr8(HitAttackFlags, 0);
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_StopSoundWhenTimeRuns07DA(void)
{
    StopSoundWhenTimeRuns07DA(cpu.x);
}

/*
 * @routine 07DA:435 FlickerThenShatter07DA
 * @run 70:AC35
 * WHAT IT DOES: Counts the wait down, flickering the water level from the table at $AC60; at zero draws tile script 3, throws the
 * shards ($ACAF) and waits $80.
 * HOW: DrawTileScript and a call.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlickerThenShatter07DA(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        lda(0x03);
        cpu.y = 0;
        JSR(0xAC40, k_draw_tiles());
        call_routine(0xAC43, 0xACAF);
        lda(0x80);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        abi_nz(INC8(OBJ->state[cpu.x]));
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    and_a(0x07);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xAC60 + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    lda(rd8((uint16_t)(0xAC61 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
}

void abi_FlickerThenShatter07DA(void)
{
    FlickerThenShatter07DA(cpu.x);
}

/*
 * @routine 07DA:470 FlickerThenDrop07DA
 * @run 70:AC70
 * WHAT IT DOES: Counts the wait down, flickering the water level; at zero clears the water level and the time slowing, clears slot
 * $4B, starts a kind $AA object here and clears this one.
 * HOW: ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlickerThenDrop07DA(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        lda(RD8(OBJ->work_d[slot]));
        and_a(0x07);
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xAC9F + cpu.y)));
        WR8(LEVELW->y.lo, cpu.a);
        lda(rd8((uint16_t)(0xACA0 + cpu.y)));
        WR8(LEVELW->y.hi, cpu.a);
        return;
    }
    WR8(LEVELW->y.lo, 0);
    WR8(LEVELW->y.hi, 0);
    WR8(PVARS->time_slow_mode, 0);
    ldy(0x4B);
    JSR(0xAC97, k_clear_in_y());
    lda(0xAA);
    JSR(0xAC9C, k_start_at_place());
    k_clear();
}

void abi_FlickerThenDrop07DA(void)
{
    FlickerThenDrop07DA(cpu.x);
}

/*
 * @routine 07DA:4AF ThrowShards07DA
 * @run 70:ACAF
 * WHAT IT DOES: Starts a kind $3F shard in each of up to $18 free slots from $20, its place, speeds and counts from the table at
 * $ACFE, eight bytes each.
 * HOW: FindFreeSlotFrom20; the row in zBurstCount, X kept in zClimbStepBefore.
 * WAITS: no.
 * Entry: X object. Exit: as the last search, compare or load left them.
 */
void ThrowShards07DA(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    do {
        WR8(ZP->burst_count, cpu.y);
        JSR(0xACB5, k_free_from20());
        if (CC) {
            return;
        }
        lda(0x3F);
        WR8(OBJ->kind[cpu.y], cpu.a);
        WR8(ZP->climb_step_before, cpu.x);
        lda(RD8(ZP->burst_count));
        asl_a();
        asl_a();
        asl_a();
        tax();
        lda(rd8((uint16_t)(0xACFE + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xACFF + cpu.x)));
        WR8(OBJ->x_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD00 + cpu.x)));
        WR8(OBJ->y_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD01 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD02 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD03 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD04 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xAD05 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        ldx(RD8(ZP->climb_step_before));
        ldy(RD8(ZP->burst_count));
        iny();
        cmp(cpu.y, 0x18);
    } while (CC);
}

void abi_ThrowShards07DA(void)
{
    ThrowShards07DA(cpu.x);
}

/*
 * @routine 07DA:5BE FallAfterWaitList07DA
 * @run 70:ADBE
 * WHAT IT DOES: State 0 loads the record; then waits out OBJ_PIECE_PLACE, then steps the list OBJ_PIECE_NEXT picks and falls.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallAfterWaitList07DA(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_e[slot]));
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xADE7 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xADE8 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xADE1, k_step_frames());
    JSR(0xADE4, k_gravity_unlimited());
    k_move();
}

void abi_FallAfterWaitList07DA(void)
{
    FallAfterWaitList07DA(cpu.x);
}

/*
 * @routine 07DA:649 FloatingLanterns07DA
 * @run 70:AE49
 * WHAT IT DOES: State 0 sets it from the table by OBJ_START_ARG: speeds, entry place and, on later sections, the flip. Then it
 * flickers every fourth frame by its number, slows by the table, moves and returns to its place when low.
 * HOW: Eight-byte rows of speeds; a place word per number.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FloatingLanterns07DA(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xAE51, k_load_record());
        JSR(0xAE54, k_record_event());
        slot = cpu.x;
        lda(RD8(OBJ->start_arg[slot]));
        asl_a();
        asl_a();
        asl_a();
        tay();
        WR8(OBJ->work_d[slot], cpu.a);
        lda(rd8((uint16_t)(0xAF0E + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xAF0D + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xAF10 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xAF0F + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        WR8(OBJ->x_hi[slot], 0);
        lda(0xFF);
        WR8(OBJ->y_hi[slot], cpu.a);
        lda(RD8(OBJ->start_arg[slot]));
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xAF05 + cpu.y)));
        WR8(OBJ->x_lo[slot], cpu.a);
        WR8(OBJ->work_e[slot], cpu.a);
        lda(rd8((uint16_t)(0xAF06 + cpu.y)));
        WR8(OBJ->y_lo[slot], cpu.a);
        WR8(OBJ->work_f[slot], cpu.a);
        lda(RD8(ZP->field83));
        if (NE) {
            lda(0x80);
            WR8(OBJ->sprite_flip[slot], cpu.a);
        }
        return;
    }
    lda(0x29);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(ZP->wave_counter));
    and_a(0x03);
    cmp(cpu.a, RD8(OBJ->start_arg[slot]));
    if (EQ) {
        WR8(OBJ->frame[slot], 0);
    }
    ldy(RD8(OBJ->work_d[slot]));
    lda(RD8(OBJ->vy_hi[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xAF13 + cpu.y)));
    if (CC) {
        lda(RD8(OBJ->vy_lo[slot]));
        clc();
        adc_a(rd8((uint16_t)(0xAF11 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        adc_a(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(RD8(OBJ->vx_lo[slot]));
        sec();
        sbc_a(rd8((uint16_t)(0xAF12 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(RD8(OBJ->vx_hi[slot]));
        sbc_a(0x00);
        WR8(OBJ->vx_hi[slot], cpu.a);
    }
    JSR(0xAEE2, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x01);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x20);
    if (CC) {
        return;
    }
    WR8(OBJ->x_hi[slot], 0);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_FloatingLanterns07DA(void)
{
    FloatingLanterns07DA(cpu.x);
}

/*
 * @routine 07DA:72D PaletteSwapEveryThird07DA
 * @run 70:AF2D
 * WHAT IT DOES: State 0 sets a count of 3. Then each frame starts script $80 or $81 in slot 4 and swaps which every third frame.
 * HOW: StartScriptSlotKind4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PaletteSwapEveryThird07DA(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xAF35, k_load_record());
        lda(0x03);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    phx();
    lda(RD8(OBJ->work_e[slot]));
    and_a(0x01);
    clc();
    adc_a(0x80);
    ldx(0x04);
    JSR(0xAF49, k_script4());
    plx();
    abi_nz(DEC8(OBJ->work_d[cpu.x]));
    if (NE) {
        return;
    }
    lda(0x03);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    lda(RD8(OBJ->work_e[cpu.x]));
    eor_a(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
}

void abi_PaletteSwapEveryThird07DA(void)
{
    PaletteSwapEveryThird07DA(cpu.x);
}

/*
 * @routine 07DA:75D ByState07DAAF5D
 * @run 70:AF5D
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $AF6F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DAAF5D(uint8_t slot)
{
    jump_by_state(slot, 0xAF6F);
}

void abi_ByState07DAAF5D(void)
{
    ByState07DAAF5D(cpu.x);
}

/*
 * @routine 07DA:775 LoadShard07DA
 * @run 70:AF75
 * WHAT IT DOES: Loads the record with the frame and speeds OBJ_START_ARG picks and a wait of $28.
 * HOW: Tables by argument.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadShard07DA(uint8_t slot)
{
    JSR(0xAF78, k_load_record());
    slot = cpu.x;
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xAFD7 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x28);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xAFDC + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xAFDB + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xAFDE + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xAFDD + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadShard07DA(void)
{
    LoadShard07DA(cpu.x);
}

/*
 * @routine 07DA:7A5 CountOutWhenFlagged07DA
 * @run 70:AFA5
 * WHAT IT DOES: Once zField92 is clear, counts the wait out, records the event and goes on, with sound $4E for number 0.
 * HOW: RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountOutWhenFlagged07DA(uint8_t slot)
{
    lda(RD8(ZP->field92));
    if (NE) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    JSR(0xAFB1, k_record_event());
    abi_nz(INC8(OBJ->state[cpu.x]));
    lda(RD8(OBJ->start_arg[cpu.x]));
    if (EQ) {
        lda(0x4E);
        JSR(0xAFBE, k_sound());
    }
}

void abi_CountOutWhenFlagged07DA(void)
{
    CountOutWhenFlagged07DA(cpu.x);
}

/*
 * @routine 07DA:7BF FallWithListByArgument07DA
 * @run 70:AFBF
 * WHAT IT DOES: Steps the list OBJ_START_ARG picks from the table at $AFEB and falls.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallWithListByArgument07DA(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xAFEB + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xAFEC + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xAFD1, k_step_frames());
    JSR(0xAFD4, k_gravity_unlimited());
    k_move();
}

void abi_FallWithListByArgument07DA(void)
{
    FallWithListByArgument07DA(cpu.x);
}

/*
 * @routine 07DB:010 TrailedOrbit07DB
 * @run 70:B010
 * WHAT IT DOES: State 0: loads the record, fills the angle history with $40 and starts seven kind $C8 links in the slots below,
 * numbered. Then follows the object in the slot below, orbiting it: a hit turns the swing, which eases back.
 * HOW: The angle history at $B1D0, shifted each frame; a 16-bit swing.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TrailedOrbit07DB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xB018, k_load_record());
        lda(0x40);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        cpu.y = 0;
        do {
            lda(0x40);
            wr8((uint16_t)(0xB1D0 + cpu.y), cpu.a);
            iny();
            cmp(cpu.y, 0x20);
        } while (CC);
        txa();
        sec();
        sbc_a(0x07);
        tay();
        lda(0x07);
        WR8(ZP->temp00, cpu.a);
        WR8(ZP->temp01, 0);
        do {
            JSR(0xB036, k_clear_in_y());
            lda(0xC8);
            JSR(0xB03B, k_start_at_place());
            lda(RD8(ZP->temp01));
            WR8(OBJ->work_e[cpu.y], cpu.a);
            iny();
            abi_nz(INC8(ZP->temp01));
            abi_nz(DEC8(ZP->temp00));
        } while (NE);
        return;
    }
    txa();
    tay();
    dey();
    JSR(0xB04E, k_copy_place_back());
    slot = cpu.x;
    lda(0x08);
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xB056, k_move_y());
    WR8(ZP->temp00, 0);
    lda(0x08);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xB062, k_velocity_from_angle());
    JSR(0xB065, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->hit_taken[slot]));
    if (NE) {
        lda(0x12);
        JSR(0xB06F, k_sound());
        lda(0x20);
        WR8(OBJ->invuln_timer[cpu.x], cpu.a);
        JSR(0xB077, k_side());
        slot = cpu.x;
        and_a(0x01);
        tay();
        lda(RD8(OBJ->work_f[slot]));
        clc();
        adc_a(rd8((uint16_t)(0xB17E + cpu.y)));
        WR8(OBJ->work_f[slot], cpu.a);
        lda(0x0A);
        WR8(OBJ->work_i[slot], cpu.a);
    }
    call_routine(0xB08C, 0xB0DB);
    call_routine(0xB08F, 0xB0ED);
    call_routine(0xB092, 0xB100);
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    clc();
    adc_a(RD8(OBJ->work_g[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    adc_a(RD8(OBJ->work_f[slot]));
    WR8(OBJ->work_d[slot], cpu.a);
    call_routine(0xB0AE, 0xB151);
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x40);
    if (CS) {
        lda(RD8(OBJ->work_g[slot]));
        sec();
        sbc_a(RD8(OBJ->work_i[slot]));
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(OBJ->work_f[slot]));
        sbc_a(0x00);
        WR8(OBJ->work_f[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->work_g[slot]));
    clc();
    adc_a(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    adc_a(0x00);
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_TrailedOrbit07DB(void)
{
    TrailedOrbit07DB(cpu.x);
}

/*
 * @routine 07DB:0DB ShiftAngleHistory07DB
 * @run 70:B0DB
 * WHAT IT DOES: Shifts the 32-angle history along and puts this object's angle first.
 * HOW: A copy from the end.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ShiftAngleHistory07DB(uint8_t slot)
{
    ldy(0x1F);
    do {
        lda(rd8((uint16_t)(0xB1CF + cpu.y)));
        wr8((uint16_t)(0xB1D0 + cpu.y), cpu.a);
        dey();
    } while (NE);
    lda(RD8(OBJ->work_d[slot]));
    wr8(0xB1D0, cpu.a);
}

void abi_ShiftAngleHistory07DB(void)
{
    ShiftAngleHistory07DB(cpu.x);
}

/*
 * @routine 07DB:0ED DampSwingAtTurn07DB
 * @run 70:B0ED
 * WHAT IT DOES: When the swing speed changes sign, takes one from the easing, down to 1.
 * HOW: An EOR of the old and new high bytes.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DampSwingAtTurn07DB(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    eor_a(RD8(OBJ->work_h[slot]));
    if (PL) {
        return;
    }
    lda(RD8(OBJ->work_i[slot]));
    cmp(cpu.a, 0x01);
    if (EQ) {
        return;
    }
    abi_nz(DEC8(OBJ->work_i[slot]));
}

void abi_DampSwingAtTurn07DB(void)
{
    DampSwingAtTurn07DB(cpu.x);
}

/*
 * @routine 07DB:100 DampSwingAtBottom07DB
 * @run 70:B100
 * WHAT IT DOES: Each time the angle passes the bottom ($40), takes a quarter off the swing speed, stopping the easing at zero.
 * HOW: A signed 16-bit divide by four; OBJ_WORK_J the half passed.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DampSwingAtBottom07DB(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    if (NE) {
        lda(RD8(OBJ->work_d[slot]));
        cmp(cpu.a, 0x40);
        if (CC) {
            return;
        }
    } else {
        lda(RD8(OBJ->work_d[slot]));
        cmp(cpu.a, 0x40);
        if (CS) {
            return;
        }
    }
    lda(RD8(OBJ->work_j[slot]));
    eor_a(0x01);
    WR8(OBJ->work_j[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    for (int i = 0; i < 2; i++) {
        lda(RD8(ZP->temp01));
        asl_a();
        abi_flag(P_C, ROR8(ZP->temp01, CS));
        abi_flag(P_C, ROR8(ZP->temp00, CS));
    }
    lda(RD8(OBJ->work_g[slot]));
    sec();
    sbc_a(RD8(ZP->temp00));
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    sbc_a(RD8(ZP->temp01));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    ora_a(RD8(OBJ->work_g[slot]));
    if (NE) {
        return;
    }
    WR8(OBJ->work_i[slot], cpu.a);
}

void abi_DampSwingAtBottom07DB(void)
{
    DampSwingAtBottom07DB(cpu.x);
}

/*
 * @routine 07DB:151 ClampSwingAngle07DB
 * @run 70:B151
 * WHAT IT DOES: Keeps the angle between $C0 and $80 round the top: past $80 it stops there, and below 0 it stops at 0.
 * HOW: Compares; a stop clears the swing speed.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ClampSwingAngle07DB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0xC0);
    if (CC) {
        cmp(cpu.a, 0x40);
        if (CS) {
            lda(RD8(OBJ->work_d[slot]));
            cmp(cpu.a, 0x80);
            if (CC) {
                return;
            }
            lda(0x80);
            WR8(OBJ->work_d[slot], cpu.a);
            WR8(OBJ->work_f[slot], 0);
            WR8(OBJ->work_g[slot], 0);
            return;
        }
    }
    lda(RD8(OBJ->work_d[slot]));
    if (PL) {
        return;
    }
    WR8(OBJ->work_d[slot], 0);
    WR8(OBJ->work_f[slot], 0);
    WR8(OBJ->work_g[slot], 0);
}

void abi_ClampSwingAngle07DB(void)
{
    ClampSwingAngle07DB(cpu.x);
}

/*
 * @routine 07DB:184 LinkLoadThenFollow07DB
 * @run 70:B184
 * @stop B199
 * WHAT IT DOES: State 0 loads the record and drops by the link's number; later follows the chain ($B1A0).
 * HOW: ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LinkLoadThenFollow07DB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xB1A0);
        return;
    }
    JSR(0xB18C, k_load_record());
    slot = cpu.x;
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB199 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xB198, k_move_y());
}

void abi_LinkLoadThenFollow07DB(void)
{
    LinkLoadThenFollow07DB(cpu.x);
}

/*
 * @routine 07DB:1A0 LinkFollowChain07DB
 * @run 70:B1A0
 * WHAT IT DOES: Takes the angle from the history by the link's number and places itself at that angle from the link below.
 * HOW: CopyObjectPlaceBack and SetVelocityFromAngle with magnitude $0800.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LinkFollowChain07DB(uint8_t slot)
{
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB1C8 + cpu.y)));
    tay();
    lda(rd8((uint16_t)(0xB1D0 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    txa();
    tay();
    dey();
    lda(RD8(OBJ->work_e[slot]));
    if (EQ) {
        return;
    }
    JSR(0xB1B8, k_copy_place_back());
    WR8(ZP->temp00, 0);
    lda(0x08);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xB1C4, k_velocity_from_angle());
    JSR(0xB1C7, k_move());
}

void abi_LinkFollowChain07DB(void)
{
    LinkFollowChain07DB(cpu.x);
}

/*
 * @routine 07DB:1F0 ByState07DAB1F0
 * @run 70:B1F0
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $B202.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DAB1F0(uint8_t slot)
{
    jump_by_state(slot, 0xB202);
}

void abi_ByState07DAB1F0(void)
{
    ByState07DAB1F0(cpu.x);
}

/*
 * @routine 07DB:20A BobOnList07DAB234
 * @run 70:B20A
 * WHAT IT DOES: Steps the list at $B234, bobbing on the cosine of a stepping angle at half speed.
 * HOW: ScaleByCosine, ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BobOnList07DAB234(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xB234);
    JSR(0xB215, k_step_frames());
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xB223, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xB230, k_move_y());
    abi_nz(INC8(OBJ->work_d[cpu.x]));
}

void abi_BobOnList07DAB234(void)
{
    BobOnList07DAB234(cpu.x);
}

/*
 * @routine 07DB:239 SinkToLineBC07DB
 * @run 70:B239
 * WHAT IT DOES: Steps the list at $B234 and sinks at $0140 to Y $BC, then goes on.
 * HOW: ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SinkToLineBC07DB(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xB234);
    JSR(0xB244, k_step_frames());
    lda(0x01);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xB251, k_move_y());
    lda(RD8(OBJ->y_lo[cpu.x]));
    cmp(cpu.a, 0xBC);
    if (CC) {
        return;
    }
    lda(0xBC);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_SinkToLineBC07DB(void)
{
    SinkToLineBC07DB(cpu.x);
}

/*
 * @routine 07DB:261 ByState07DAB261
 * @run 70:B261
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $B273.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DAB261(uint8_t slot)
{
    jump_by_state(slot, 0xB273);
}

void abi_ByState07DAB261(void)
{
    ByState07DAB261(cpu.x);
}

/*
 * @routine 07DB:27B WaitForPlayerState07DB
 * @run 70:B27B
 * WHAT IT DOES: Steps the list at $B297; once the player is in substate 1, restarts the animation and goes on.
 * HOW: The player's substate.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WaitForPlayerState07DB(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xB297);
    JSR(0xB286, k_step_frames());
    lda(RD8(OBJ->substate[0]));
    cmp(cpu.a, 0x01);
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_WaitForPlayerState07DB(void)
{
    WaitForPlayerState07DB(cpu.x);
}

/*
 * @routine 07DB:2A2 FadeCdAtListEnd07DB
 * @run 70:B2A2
 * WHAT IT DOES: Steps the list at $B2CC; at its end fades the CD out ($0C) and goes on.
 * HOW: cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FadeCdAtListEnd07DB(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xB2CC);
    JSR(0xB2AD, k_step_frames());
    if (NE) {
        return;
    }
    phx();
    lda(0x0C);
    JSR(0xB2B5, cd_fade_exit_shim(vec_cd_fade(cpu.a)));
    plx();
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_FadeCdAtListEnd07DB(void)
{
    FadeCdAtListEnd07DB(cpu.x);
}

/*
 * @routine 07DB:2BA EndStageAfterCount07DB
 * @run 70:B2BA
 * WHAT IT DOES: Counts the wait down by two; at zero raises mode event bit 1, sets zFieldA3 to 9 and clears the object.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EndStageAfterCount07DB(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    RMW8(ZP->mode_events, was, (uint8_t)(was | 0x02));
    lda(0x09);
    WR8(ZP->fielda3, cpu.a);
    k_clear();
}

void abi_EndStageAfterCount07DB(void)
{
    EndStageAfterCount07DB(cpu.x);
}

/*
 * @routine 07DB:2DB ExpandListsForPart3_07DB
 * @run 70:B2DB
 * WHAT IT DOES: Expands the stage lists of part 3 and goes back to part 2, then clears the object.
 * HOW: ExpandStageListsE with zField82 set around it.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ExpandListsForPart3_07DB(uint8_t slot)
{
    (void)slot;
    lda(0x03);
    WR8(ZP->field82, cpu.a);
    phx();
    call_routine(0xB2E3, 0x9277);
    plx();
    lda(0x02);
    WR8(ZP->field82, cpu.a);
    k_clear();
}

void abi_ExpandListsForPart3_07DB(void)
{
    ExpandListsForPart3_07DB(cpu.x);
}

/*
 * @routine 07DB:2EB CountCheckThenByState07DB
 * @run 70:B2EB
 * WHAT IT DOES: Counts OBJ_FALL_LO, bursts when hit and flagged ($B311) and goes to the state's handler.
 * HOW: A table of handlers at $B303.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountCheckThenByState07DB(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_h[slot]));
    call_routine(0xB2F1, 0xB311);
    jump_by_state(cpu.x, 0xB303);
}

void abi_CountCheckThenByState07DB(void)
{
    CountCheckThenByState07DB(cpu.x);
}
