/*
 * Stage 02's objects, runs D4EA-D84F: the flying head and its body.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_g.h"
#include "calls_b.h"

/*
 * @routine 051F:4EA ChainHeadByState051B
 * @run 71:D4EA
 * WHAT IT DOES: Checks the head for a hit ($D734) and goes to the state's handler.
 * HOW: A table of handlers at $D4FF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChainHeadByState051B(uint8_t slot)
{
    (void)slot;
    call_routine(0xD4ED, 0xD734);
    jump_by_state(cpu.x, 0xD4FF);
}

void abi_ChainHeadByState051B(void)
{
    ChainHeadByState051B(cpu.x);
}

/*
 * @routine 051F:509 SetUpFlyingHead051B
 * @run 71:D509
 * WHAT IT DOES: State 0 of the two-slot flier: loads the record, starts the body's list in the next slot, sets the angle, swing
 * speed and counts, and makes the next slot kind $4A.
 * HOW: LoadObjectRecord, StepObjectFrames and ClearObject on slot X+1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetUpFlyingHead051B(uint8_t slot)
{
    JSR(0xD50C, k_load_record());
    inx();
    list_to_temp10(0xD4C4);
    JSR(0xD518, k_step_frames());
    dex();
    slot = cpu.x;
    lda(0xC0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->work_i[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_m[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_m[slot], cpu.a);
    inx();
    JSR(0xD540, k_clear());
    dex();
    {
        unsigned next = cpu.x + 1u;

        lda(0x4A);
        WR8(OBJ->kind[next], cpu.a);
    }
}

void abi_SetUpFlyingHead051B(void)
{
    SetUpFlyingHead051B(cpu.x);
}

/*
 * @routine 051F:547 AimFlyingHead051B
 * @run 71:D547
 * WHAT IT DOES: State 1: moves the parts ($D60E) unless the head is gone; with II newly pressed, copies the swing sizes to the
 * next slot, aims at the player from there, sets a velocity of $0140 that way, turns it back when rising leftward and
 * starts a count of $30. Then moves as $D59C.
 * HOW: AngleToPlayer from slot X+1, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AimFlyingHead051B(uint8_t slot)
{
    call_routine(0xD54A, 0xD60E);
    slot = cpu.x;
    lda(RD8(OBJ->substate[slot]));
    cmp(cpu.a, 0x02);
    if (EQ) {
        run_routine(0xD59C);
        return;
    }
    {
        uint8_t pressed = RD8(ZP->pad_pressed);

        abi_flag(P_Z, (pressed & 0x02) == 0);
        abi_flag(P_N, (pressed & 0x80) != 0);
        abi_flag(P_V, (pressed & 0x40) != 0);
    }
    if (EQ) {
        run_routine(0xD59C);
        return;
    }
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[next], cpu.a);
        lda(RD8(OBJ->work_g[slot]));
        WR8(OBJ->work_g[next], cpu.a);
        lda(RD8(OBJ->work_h[slot]));
        WR8(OBJ->work_h[next], cpu.a);
        lda(RD8(OBJ->work_i[slot]));
        WR8(OBJ->work_i[next], cpu.a);
    }
    inx();
    JSR(0xD572, k_angle_to_player());
    dex();
    slot = cpu.x;
    WR8(OBJ->work_j[slot], cpu.a);
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    JSR(0xD584, k_velocity_from_angle());
    slot = cpu.x;
    lda(RD8(OBJ->vx_hi[slot]));
    if (MI) {
        JSR(0xD58C, k_negate_x());
        slot = cpu.x;
    }
    lda(RD8(OBJ->vx_hi[slot]));
    if (MI) {
        JSR(0xD594, k_negate_y());
        slot = cpu.x;
    }
    lda(0x30);
    WR8(OBJ->work_k[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
    run_routine(0xD59C);
}

void abi_AimFlyingHead051B(void)
{
    AimFlyingHead051B(cpu.x);
}

/*
 * @routine 051F:59C PlaceFlyingBody051B
 * @run 71:D59C
 * WHAT IT DOES: Places the next slot on the cosine and sine of the angle at the swing sizes from the head, after moving the head
 * with the camera, and advances the angle unless the head is gone.
 * HOW: ScaleByCosine and ScaleBySine into X+1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PlaceFlyingBody051B(uint8_t slot)
{
    inx();
    call_routine(0xD5A0, 0xD756);
    dex();
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    JSR(0xD5B1, k_cosine());
    slot = cpu.x;
    WR8(ZP->temp00, 0);
    lda(RD8(ZP->plane_rows_left));
    if (MI) {
        abi_nz(DEC8(ZP->temp00));
    }
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->x_lo[slot]));
        clc();
        adc_a(RD8(ZP->plane_rows_left));
        WR8(OBJ->x_lo[next], cpu.a);
        lda(RD8(OBJ->x_hi[slot]));
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->x_hi[next], cpu.a);
    }
    lda(RD8(OBJ->work_h[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    JSR(0xD5DA, k_sine());
    slot = cpu.x;
    WR8(ZP->temp00, 0);
    lda(RD8(ZP->plane_rows_left));
    if (MI) {
        abi_nz(DEC8(ZP->temp00));
    }
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->y_lo[slot]));
        clc();
        adc_a(RD8(ZP->plane_rows_left));
        WR8(OBJ->y_lo[next], cpu.a);
        lda(RD8(OBJ->y_hi[slot]));
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->y_hi[next], cpu.a);
    }
    lda(RD8(OBJ->substate[slot]));
    cmp(cpu.a, 0x02);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->work_m[slot]));
    clc();
    adc_a(RD8(OBJ->work_d[slot]));
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->work_m[slot]));
    adc_a(RD8(OBJ->work_e[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_PlaceFlyingBody051B(void)
{
    PlaceFlyingBody051B(cpu.x);
}

/*
 * @routine 051F:60E FlyingHeadParts051B
 * @run 71:D60E
 * WHAT IT DOES: Drips ($D6E7). Substate 0 steps the body's list and, five hits taken, remembers the substate and goes to
 * substate 2, speeding the angle when health is low. Substate 1 steps another list the same way. Substate 2 steps
 * the third list and, at its end, goes back.
 * HOW: StepObjectFrames on slot X+1; the hit count of X+1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyingHeadParts051B(uint8_t slot)
{
    call_routine(0xD611, 0xD6E7);
    slot = cpu.x;
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        dec_a();
        if (NE) {
            inx();
            list_to_temp10(0xD4D0);
            JSR(0xD666, k_step_frames());
            dex();
            slot = cpu.x;
            cmp(cpu.a, 0x00);
            if (NE) {
                return;
            }
            WR8(OBJ->anim_timer[slot], 0);
            WR8(OBJ->anim_step[slot], 0);
            lda(RD8(OBJ->work_l[slot]));
            WR8(OBJ->substate[slot], cpu.a);
            return;
        }
        inx();
        list_to_temp10(0xD4C9);
    } else {
        inx();
        list_to_temp10(0xD4C4);
    }
    JSR(0xD622, k_step_frames());
    dex();
    slot = cpu.x;
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->hit_taken[next]));
        cmp(cpu.a, 0x05);
        if (CC) {
            return;
        }
        lda(RD8(OBJ->substate[slot]));
        WR8(OBJ->work_l[slot], cpu.a);
        lda(0x02);
        WR8(OBJ->substate[slot], cpu.a);
        lda(RD8(OBJ->health[next]));
    }
    cmp(cpu.a, 0x04);
    if (CS) {
        return;
    }
    lda(0xC0);
    WR8(OBJ->work_m[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->work_m[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_l[slot], cpu.a);
}

void abi_FlyingHeadParts051B(void)
{
    FlyingHeadParts051B(cpu.x);
}

/*
 * @routine 051F:678 FlyingHeadSwell051B
 * @run 71:D678
 * WHAT IT DOES: State 2: moves the parts and the body; unless the head is gone, grows the swing sizes by the velocity and, its
 * count over, turns the velocity back and goes on.
 * HOW: 16-bit adds of VX into the two swing words.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyingHeadSwell051B(uint8_t slot)
{
    call_routine(0xD67B, 0xD60E);
    call_routine(0xD67E, 0xD59C);
    slot = cpu.x;
    lda(RD8(OBJ->substate[slot]));
    cmp(cpu.a, 0x02);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(RD8(OBJ->work_f[slot]));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(RD8(OBJ->work_g[slot]));
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(RD8(OBJ->work_h[slot]));
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_i[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_k[slot]));
    if (NE) {
        return;
    }
    JSR(0xD6B3, k_negate_x());
    JSR(0xD6B6, k_negate_y());
    slot = cpu.x;
    lda(0x30);
    WR8(OBJ->work_k[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_FlyingHeadSwell051B(void)
{
    FlyingHeadSwell051B(cpu.x);
}

/*
 * @routine 051F:6BF FlyingHeadShrink051B
 * @run 71:D6BF
 * WHAT IT DOES: State 3: as state 2 until the next, then back to state 1 with the swing sizes taken from the next slot.
 * HOW: A call and four copies.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyingHeadShrink051B(uint8_t slot)
{
    call_routine(0xD6C2, 0xD678);
    slot = cpu.x;
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (CC) {
        return;
    }
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->work_f[next]));
        WR8(OBJ->work_f[slot], cpu.a);
        lda(RD8(OBJ->work_g[next]));
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(OBJ->work_h[next]));
        WR8(OBJ->work_h[slot], cpu.a);
        lda(RD8(OBJ->work_i[next]));
        WR8(OBJ->work_i[slot], cpu.a);
    }
}

void abi_FlyingHeadShrink051B(void)
{
    FlyingHeadShrink051B(cpu.x);
}

/*
 * @routine 051F:6E7 DripFromBody051B
 * @run 71:D6E7
 * WHAT IT DOES: Every fourth frame, a kind $4B drop beside the next slot at one of eight offsets, 12 above.
 * HOW: FindFreeSlotFrom30 and NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DripFromBody051B(uint8_t slot)
{
    (void)slot;
    {
        uint8_t counter = RD8(ZP->wave_counter);

        abi_flag(P_Z, (counter & 0x03) == 0);
        abi_flag(P_N, (counter & 0x80) != 0);
        abi_flag(P_V, (counter & 0x40) != 0);
    }
    if (NE) {
        return;
    }
    JSR(0xD6EF, k_free_from30());
    if (CC) {
        return;
    }
    phy();
    JSR(0xD6F5, k_random());
    and_a(0x07);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD72C + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(rd8((uint16_t)(0xD72D + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    ply();
    {
        unsigned next = cpu.x + 1u;

        lda(0x4B);
        WR8(OBJ->kind[cpu.y], cpu.a);
        lda(RD8(OBJ->x_lo[next]));
        clc();
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->x_lo[cpu.y], cpu.a);
        lda(RD8(OBJ->x_hi[next]));
        adc_a(RD8(ZP->temp01));
        WR8(OBJ->x_hi[cpu.y], cpu.a);
        lda(RD8(OBJ->y_lo[next]));
        clc();
        adc_a(0xF4);
        WR8(OBJ->y_lo[cpu.y], cpu.a);
        lda(0xFF);
        adc_a(RD8(OBJ->y_hi[cpu.y]));
        WR8(OBJ->y_hi[cpu.y], cpu.a);
    }
}

void abi_DripFromBody051B(void)
{
    DripFromBody051B(cpu.x);
}

/*
 * @routine 051F:734 KillHeadWhenBodyHit051B
 * @run 71:D734
 * WHAT IT DOES: Under state 4, a hit on the next slot sounds $83, records the event and goes to state 4 with the next slot's hit
 * box off.
 * HOW: SoundCommand and RecordObjectEvent.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void KillHeadWhenBodyHit051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (CS) {
        return;
    }
    {
        unsigned next = slot + 1u;

        lda(RD8(OBJ->hit_status[next]));
    }
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(0x83);
    JSR(0xD747, k_sound());
    JSR(0xD74A, k_record_event());
    slot = cpu.x;
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    {
        unsigned next = slot + 1u;

        WR8(OBJ->hitbox_on[next], 0);
    }
}

void abi_KillHeadWhenBodyHit051B(void)
{
    KillHeadWhenBodyHit051B(cpu.x);
}

/*
 * @routine 051F:756 MoveWithScroll051B
 * @run 71:D756
 * WHAT IT DOES: Adds the camera's signed X speed to the object's X.
 * HOW: A signed 8-bit add to the 16-bit X.
 * WAITS: no.
 * Entry: X object. Exit: A the X high byte.
 */
void MoveWithScroll051B(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    lda(RD8(SCROLLE->speed_x[0]));
    if (MI) {
        abi_nz(DEC8(ZP->temp00));
    }
    clc();
    adc_a(RD8(OBJ->x_lo[slot]));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(ZP->temp00));
    adc_a(RD8(OBJ->x_hi[slot]));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_MoveWithScroll051B(void)
{
    MoveWithScroll051B(cpu.x);
}

/*
 * @routine 051F:76F FlyingHeadFall051B
 * @run 71:D76F
 * WHAT IT DOES: State 4: substate 0 starts the next slot falling at $0100 without loop flag bit 0. Then drips, splashes ($D7C6),
 * steps the next slot's list and moves it down, flickering; off the bottom both slots are cleared.
 * HOW: ApplyVelocityY on slot X+1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyingHeadFall051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        unsigned next = slot + 1u;

        WR8(OBJ->anim_timer[next], 0);
        WR8(OBJ->anim_step[next], 0);
        lda(0x00);
        WR8(OBJ->vy_lo[next], cpu.a);
        lda(0x01);
        WR8(OBJ->vy_hi[next], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        lda(RD8(OBJ->loop_flags[next]));
        and_a(0xFE);
        WR8(OBJ->loop_flags[next], cpu.a);
        return;
    }
    call_routine(0xD793, 0xD6E7);
    call_routine(0xD796, 0xD7C6);
    inx();
    list_to_temp10(0xD4D0);
    JSR(0xD7A2, k_step_frames());
    JSR(0xD7A5, k_move_y());
    dex();
    {
        unsigned next = cpu.x + 1u;

        lda(0x10);
        ora_a(RD8(OBJ->sprite_attr[next]));
        WR8(OBJ->sprite_attr[next], cpu.a);
        lda(RD8(OBJ->y_hi[next]));
        cmp(cpu.a, 0x01);
        if (NE) {
            lda(RD8(OBJ->y_lo[next]));
            cmp(cpu.a, 0xF0);
            if (CC) {
                return;
            }
        }
    }
    JSR(0xD7C0, k_clear());
    inx();
    JSR(0xD7C4, k_clear());
    dex();
}

void abi_FlyingHeadFall051B(void)
{
    FlyingHeadFall051B(cpu.x);
}

/*
 * @routine 051F:7C6 SplashBesideBody051B
 * @run 71:D7C6
 * WHAT IT DOES: One frame in eight, a kind $9D splash in a free slot within 8 of the next slot each way.
 * HOW: Two signed random offsets.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SplashBesideBody051B(uint8_t slot)
{
    (void)slot;
    lda(RD8(ZP->wave_counter));
    and_a(0x07);
    cmp(cpu.a, 0x06);
    if (NE) {
        return;
    }
    JSR(0xD7D1, k_free_from30());
    if (CC) {
        return;
    }
    WR8(ZP->temp01, 0);
    JSR(0xD7D8, k_random());
    and_a(0x0F);
    sec();
    sbc_a(0x08);
    WR8(ZP->temp00, cpu.a);
    if (MI) {
        abi_nz(DEC8(ZP->temp01));
    }
    WR8(ZP->temp03, 0);
    JSR(0xD7E8, k_random());
    and_a(0x0F);
    sec();
    sbc_a(0x08);
    WR8(ZP->temp02, cpu.a);
    if (MI) {
        abi_nz(DEC8(ZP->temp03));
    }
    {
        unsigned next = cpu.x + 1u;

        lda(RD8(OBJ->x_lo[next]));
        clc();
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->x_lo[cpu.y], cpu.a);
        lda(RD8(OBJ->x_hi[next]));
        adc_a(RD8(ZP->temp01));
        WR8(OBJ->x_hi[cpu.y], cpu.a);
        lda(RD8(OBJ->y_lo[next]));
        clc();
        adc_a(RD8(ZP->temp02));
        WR8(OBJ->y_lo[cpu.y], cpu.a);
        lda(RD8(OBJ->y_hi[next]));
        adc_a(RD8(ZP->temp03));
        WR8(OBJ->y_hi[cpu.y], cpu.a);
    }
    lda(0x9D);
    WR8(OBJ->kind[cpu.y], cpu.a);
}

void abi_SplashBesideBody051B(void)
{
    SplashBesideBody051B(cpu.x);
}

/*
 * @routine 0520:01B ClearWhenSlotBelowGone051B
 * @run 71:D81B
 * WHAT IT DOES: State 0 loads the record; later clears the object once the slot below holds no object.
 * HOW: The kind of slot X-1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ClearWhenSlotBelowGone051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(rd8((uint16_t)(0x274F + cpu.x)));
    if (EQ) {
        k_clear();
    }
}

void abi_ClearWhenSlotBelowGone051B(void)
{
    ClearWhenSlotBelowGone051B(cpu.x);
}

/*
 * @routine 0520:02C RisingBubblePicture051B
 * @run 71:D82C
 * WHAT IT DOES: State 0 loads the record as sprite set 6, then steps ($D839); later rises ($D845).
 * HOW: A branch by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RisingBubblePicture051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xD845);
        return;
    }
    JSR(0xD834, k_load_record());
    lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    run_routine(0xD839);
}

void abi_RisingBubblePicture051B(void)
{
    RisingBubblePicture051B(cpu.x);
}

/*
 * @routine 0520:039 StepBubbleList051B
 * @run 71:D839
 * WHAT IT DOES: Steps the list at $D4DB.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepBubbleList051B(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD4DB);
    JSR(0xD844, k_step_frames());
}

void abi_StepBubbleList051B(void)
{
    StepBubbleList051B(cpu.x);
}

/*
 * @routine 0520:045 RiseAndClearAtListEnd051B
 * @run 71:D845
 * WHAT IT DOES: Rises one dot, steps the list and clears the object at its end.
 * HOW: StepBubbleList051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RiseAndClearAtListEnd051B(uint8_t slot)
{
    abi_nz(DEC8(OBJ->y_lo[slot]));
    call_routine(0xD84A, 0xD839);
    if (EQ) {
        k_clear();
    }
}

void abi_RiseAndClearAtListEnd051B(void)
{
    RiseAndClearAtListEnd051B(cpu.x);
}
