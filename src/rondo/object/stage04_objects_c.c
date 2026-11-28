/*
 * Stage 04's objects, runs BE5E-C219.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_c.h"
#include "calls_b.h"

/*
 * @routine 07DC:65E ByState07DABE5E
 * @run 70:BE5E
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $BE70.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DABE5E(uint8_t slot)
{
    jump_by_state(slot, 0xBE70);
}

void abi_ByState07DABE5E(void)
{
    ByState07DABE5E(cpu.x);
}

/*
 * @routine 07DC:678 LaunchByKindOfShot07DB
 * @run 70:BE78
 * WHAT IT DOES: Loads the record, starts it by its number ($BEDD) with a life of $48; numbers other than 2 fly level at the
 * speed the table gives, number 2 hops at a speed by the player's distance. Facing left turns it.
 * HOW: Speed tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LaunchByKindOfShot07DB(uint8_t slot)
{
    JSR(0xBE7B, k_load_record());
    call_routine(0xBE7E, 0xBEDD);
    slot = cpu.x;
    lda(0x48);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x02);
    if (NE) {
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xBECA + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xBEC9 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    } else {
        JSR(0xBEAF, k_distance_x());
        slot = cpu.x;
        lsr_a();
        lsr_a();
        lsr_a();
        lsr_a();
        lsr_a();
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xBECE + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xBECD + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(0xFB);
        WR8(OBJ->vy_hi[slot], cpu.a);
    }
    lda(RD8(OBJ->facing_left[slot]));
    if (EQ) {
        return;
    }
    JSR(0xBEA6, k_negate_x());
    lda(0x01);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_LaunchByKindOfShot07DB(void)
{
    LaunchByKindOfShot07DB(cpu.x);
}

/*
 * @routine 07DC:6DD StartShotByNumber07DB
 * @run 70:BEDD
 * WHAT IT DOES: Takes the speed high bytes the table gives for OBJ_PIECE_PLACE, turned when facing left, and moves one step.
 * HOW: A tail jump to ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartShotByNumber07DB(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xBEF9 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBEFA + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (NE) {
        JSR(0xBEF6, k_negate_x());
    }
    k_move();
}

void abi_StartShotByNumber07DB(void)
{
    StartShotByNumber07DB(cpu.x);
}

/*
 * @routine 07DC:6FF FlyThenNextOrFall07DB
 * @run 70:BEFF
 * WHAT IT DOES: Steps the list at $BF2F and moves; number 2 falls by $1C, others count their life out to the next state.
 * HOW: StepObjectFrames and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyThenNextOrFall07DB(uint8_t slot)
{
    lda(0xBF);
    WR8(ZP->temp11, cpu.a);
    lda(0x2F);
    WR8(ZP->temp10, cpu.a);
    JSR(0xBF0A, k_step_frames());
    JSR(0xBF0D, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x02);
    if (EQ) {
        lda(RD8(OBJ->vy_lo[slot]));
        clc();
        adc_a(0x1C);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        adc_a(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_f[slot]));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_FlyThenNextOrFall07DB(void)
{
    FlyThenNextOrFall07DB(cpu.x);
}

/*
 * @routine 07DC:740 SteerBackUntilSpeed07DB
 * @run 70:BF40
 * WHAT IT DOES: Steps the list at $BF2F and moves, accelerating by the table until the speed reaches its limit, then goes on.
 * HOW: A 16-bit add and a compare of both bytes.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SteerBackUntilSpeed07DB(uint8_t slot)
{
    lda(0xBF);
    WR8(ZP->temp11, cpu.a);
    lda(0x2F);
    WR8(ZP->temp10, cpu.a);
    JSR(0xBF4B, k_step_frames());
    JSR(0xBF4E, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    asl_a();
    asl_a();
    tay();
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xBF78 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(rd8((uint16_t)(0xBF79 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    cmp(cpu.a, rd8((uint16_t)(0xBF7B + cpu.y)));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->vx_lo[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xBF7A + cpu.y)));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SteerBackUntilSpeed07DB(void)
{
    SteerBackUntilSpeed07DB(cpu.x);
}

/*
 * @routine 07DC:780 StepAndMove07DBF80
 * @run 70:BF80
 * WHAT IT DOES: Steps the list at $BF2F and moves.
 * HOW: A tail jump to ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepAndMove07DBF80(uint8_t slot)
{
    (void)slot;
    lda(0xBF);
    WR8(ZP->temp11, cpu.a);
    lda(0x2F);
    WR8(ZP->temp10, cpu.a);
    JSR(0xBF8B, k_step_frames());
    k_move();
}

void abi_StepAndMove07DBF80(void)
{
    StepAndMove07DBF80(cpu.x);
}

/*
 * @routine 07DC:78E ByState07DABF8E
 * @run 70:BF8E
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $BFA0.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DABF8E(uint8_t slot)
{
    jump_by_state(slot, 0xBFA0);
}

void abi_ByState07DABF8E(void)
{
    ByState07DABF8E(cpu.x);
}

/*
 * @routine 07DC:7A8 FollowBodyUntilHurt07DB
 * @run 70:BFA8
 * WHAT IT DOES: Takes the next slot's attribute; once its health is under 8, clears its pace bits, this loop flag bit 1 and
 * attribute, and goes on.
 * HOW: Reads of slot X+1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FollowBodyUntilHurt07DB(uint8_t slot)
{
    unsigned next = slot + 1u;

    lda(RD8(OBJ->sprite_attr[next]));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->health[next]));
    cmp(cpu.a, 0x08);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->pace[next]));
    and_a(0xFC);
    WR8(OBJ->pace[next], cpu.a);
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xFD);
    WR8(OBJ->loop_flags[slot], cpu.a);
    WR8(OBJ->sprite_attr[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_FollowBodyUntilHurt07DB(void)
{
    FollowBodyUntilHurt07DB(cpu.x);
}

/*
 * @routine 07DC:7FA LeapByBodyFacing07DB
 * @run 70:BFFA
 * WHAT IT DOES: Rises at -2 with the X speed the next slot's facing picks, and goes on.
 * HOW: A table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LeapByBodyFacing07DB(uint8_t slot)
{
    unsigned next = slot + 1u;

    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    ldy(RD8(OBJ->facing_left[next]));
    lda(rd8((uint16_t)(0xC00C + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LeapByBodyFacing07DB(void)
{
    LeapByBodyFacing07DB(cpu.x);
}

/*
 * @routine 07DD:014 HitsThenByState07DC014
 * @run 71:C014
 * WHAT IT DOES: Takes the hits ($C031) and goes to the state's handler.
 * HOW: A table of handlers at $C029.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HitsThenByState07DC014(uint8_t slot)
{
    (void)slot;
    call_routine(0xC017, 0xC031);
    jump_by_state(cpu.x, 0xC029);
}

void abi_HitsThenByState07DC014(void)
{
    HitsThenByState07DC014(cpu.x);
}

/*
 * @routine 07DD:031 BossPartsWhenHit07DD
 * @run 71:C031
 * WHAT IT DOES: Under state 2: on section 2 at map X below $D0, or on status bit 7, records the event with sound $8D, drops the
 * three hit boxes, stops falling and goes to state 2 with a wait of $50. Else status bit 5 clears the water level
 * and puts this and the next two slots in states 3, 4 and 4.
 * HOW: A 16-bit map position and status tests.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BossPartsWhenHit07DD(uint8_t slot)
{
    bool killed = false;

    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    lda(RD8(ZP->field83));
    cmp(cpu.a, 0x02);
    if (EQ) {
        lda(RD8(OBJ->x_lo[slot]));
        clc();
        adc_a(RD8(SCROLL->x_lo[0]));
        WR8(ZP->temp00, cpu.a);
        lda(RD8(OBJ->x_hi[slot]));
        adc_a(RD8(SCROLLE->x_hi_screen[0]));
        if (EQ) {
            lda(RD8(ZP->temp00));
            cmp(cpu.a, 0xD0);
            killed = CS;
        }
    }
    if (!killed) {
        lda(RD8(OBJ->hit_status[slot]));
        and_a(0x20);
        if (NE) {
            unsigned one = slot + 1u, two = slot + 2u;

            WR8(LEVELW->y.hi, 0);
            WR8(LEVELW->y.lo, 0);
            lda(0x03);
            WR8(OBJ->state[slot], cpu.a);
            lda(0x04);
            WR8(OBJ->state[one], cpu.a);
            lda(0x04);
            WR8(OBJ->state[two], cpu.a);
            return;
        }
        lda(RD8(OBJ->hit_status[slot]));
        and_a(0x80);
        if (EQ) {
            return;
        }
    }
    JSR(0xC07C, k_record_event());
    lda(0x8D);
    JSR(0xC081, k_sound());
    slot = cpu.x;
    {
        unsigned one = slot + 1u, two = slot + 2u;

        lda(0x01);
        WR8(ZP->field92, cpu.a);
        WR8(OBJ->hitbox_on[slot], 0);
        WR8(OBJ->hitbox_on[one], 0);
        WR8(OBJ->hitbox_on[two], 0);
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xFD);
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(0x50);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_BossPartsWhenHit07DD(void)
{
    BossPartsWhenHit07DD(cpu.x);
}

/*
 * @routine 07DD:0A6 NothingState07DD
 * @run 71:C0A6
 * WHAT IT DOES: Does nothing: the handler of the state the table at $C029 gives no work.
 * HOW: An RTS.
 * WAITS: no.
 * Entry: none. Exit: unchanged.
 */
void NothingState07DD(uint8_t slot)
{
    (void)slot;
}

void abi_NothingState07DD(void)
{
    NothingState07DD(cpu.x);
}

/*
 * @routine 07DD:0A7 BossSetUp07DD
 * @run 71:C0A7
 * WHAT IT DOES: Once the boss is beaten clears the object. Else loads the record at the X the section gives and starts a kind $AB
 * and a kind $AC object in the next two slots.
 * HOW: ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BossSetUp07DD(uint8_t slot)
{
    lda(RD8(ZP->field92));
    if (NE) {
        k_clear();
        return;
    }
    JSR(0xC0B1, k_load_record());
    slot = cpu.x;
    lda(RD8(ZP->field83));
    dec_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC0D8 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC0D7 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    txa();
    tay();
    iny();
    JSR(0xC0C8, k_clear_in_y());
    lda(0xAB);
    JSR(0xC0CD, k_start_at_place());
    iny();
    JSR(0xC0D1, k_clear_in_y());
    lda(0xAC);
    JSR(0xC0D6, k_start_at_place());
}

void abi_BossSetUp07DD(void)
{
    BossSetUp07DD(cpu.x);
}

/*
 * @routine 07DD:0DB CircleBodyTwoBelow07DD
 * @run 71:C0DB
 * WHAT IT DOES: Steps the list at $C11A and places itself on a circle around the object two slots on, its height halved.
 * HOW: CopyObjectPlaceBack, ApplyVelocityX $34, SetVelocityFromAngle magnitude $0600, VX halved.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CircleBodyTwoBelow07DD(uint8_t slot)
{
    lda(0xC1);
    WR8(ZP->temp11, cpu.a);
    lda(0x1A);
    WR8(ZP->temp10, cpu.a);
    JSR(0xC0E6, k_step_frames());
    txa();
    inc_a();
    inc_a();
    tay();
    JSR(0xC0ED, k_copy_place_back());
    lda(0x34);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    JSR(0xC0F5, k_move_x());
    lda(0x06);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xC103, k_velocity_from_angle());
    slot = cpu.x;
    lda(RD8(OBJ->vx_hi[slot]));
    asl_a();
    ROR_MEM(OBJ->vx_hi[slot]);
    ROR_MEM(OBJ->vx_lo[slot]);
    JSR(0xC110, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_CircleBodyTwoBelow07DD(void)
{
    CircleBodyTwoBelow07DD(cpu.x);
}

/*
 * @routine 07DD:123 CountSparksThenFall07DD
 * @run 71:C123
 * WHAT IT DOES: Counts out sparks ($C12C), then falls.
 * HOW: ApplyGravityUnlimited and a tail jump to ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountSparksThenFall07DD(uint8_t slot)
{
    (void)slot;
    call_routine(0xC126, 0xC12C);
    JSR(0xC129, k_gravity_unlimited());
    k_move_y();
}

void abi_CountSparksThenFall07DD(void)
{
    CountSparksThenFall07DD(cpu.x);
}

/*
 * @routine 07DD:12C SparksAtCounts07DD
 * @run 71:C12C
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE down; at one of sixteen counts starts a kind $9D spark with sound $4E and that count's X
 * speed, and moves it.
 * HOW: A search of the table; X kept in zClimbStepBefore, the row in zBurstCount.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SparksAtCounts07DD(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    if (EQ) {
        return;
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    cpu.y = 0;
    for (;;) {
        lda(rd8((uint16_t)(0xC166 + cpu.y)));
        cmp(cpu.a, RD8(OBJ->work_e[slot]));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x10);
        if (CS) {
            return;
        }
    }
    WR8(ZP->climb_step_before, cpu.x);
    WR8(ZP->burst_count, cpu.y);
    JSR(0xC14A, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x9D);
    JSR(0xC151, k_start_at_place());
    lda(0x4E);
    JSR(0xC156, k_sound());
    ldx(RD8(ZP->burst_count));
    lda(rd8((uint16_t)(0xC176 + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xC162, k_move());
    sxy();
    ldx(RD8(ZP->climb_step_before));
}

void abi_SparksAtCounts07DD(void)
{
    SparksAtCounts07DD(cpu.x);
}

/*
 * @routine 07DD:186 CheckHeadThenByState07DD
 * @run 71:C186
 * WHAT IT DOES: Falls away when the part below is gone ($C1A5) and goes to the state's handler.
 * HOW: A table of handlers at $C19B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CheckHeadThenByState07DD(uint8_t slot)
{
    (void)slot;
    call_routine(0xC189, 0xC1A5);
    jump_by_state(cpu.x, 0xC19B);
}

void abi_CheckHeadThenByState07DD(void)
{
    CheckHeadThenByState07DD(cpu.x);
}

/*
 * @routine 07DD:1A5 FallWithoutHead07DD
 * @run 71:C1A5
 * WHAT IT DOES: Once the slot below no longer holds kind $AA, goes to state 3 at rest (unless there) with pace bits 3-6 and loop
 * flag bit 1 cleared and a wait of $50.
 * HOW: The kind of slot X-1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallWithoutHead07DD(uint8_t slot)
{
    lda(rd8((uint16_t)(0x274F + cpu.x)));
    cmp(cpu.a, 0xAA);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (EQ) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(RD8(OBJ->pace[slot]));
    and_a(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xFD);
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(0x50);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_FallWithoutHead07DD(void)
{
    FallWithoutHead07DD(cpu.x);
}

/*
 * @routine 07DD:1D4 LoadWithWait20_07DD
 * @run 71:C1D4
 * WHAT IT DOES: Loads the record with a wait of $20.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadWithWait20_07DD(uint8_t slot)
{
    (void)slot;
    JSR(0xC1D7, k_load_record());
    lda(0x20);
    WR8(OBJ->work_e[cpu.x], cpu.a);
}

void abi_LoadWithWait20_07DD(void)
{
    LoadWithWait20_07DD(cpu.x);
}

/*
 * @routine 07DD:1DD FlickerWaterThenNext07DD
 * @run 71:C1DD
 * WHAT IT DOES: Steps the list at $C21A, setting the water level by the player's distance and the count; at the count's end goes
 * on with a wait of $20 and $80.
 * HOW: A word table as in WaterLevelByDistance051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlickerWaterThenNext07DD(uint8_t slot)
{
    lda(0xC2);
    WR8(ZP->temp11, cpu.a);
    lda(0x1A);
    WR8(ZP->temp10, cpu.a);
    JSR(0xC1E8, k_step_frames());
    lda(RD8(OBJ->work_e[cpu.x]));
    and_a(0x07);
    asl_a();
    WR8(ZP->temp00, cpu.a);
    JSR(0xC1F3, k_distance_x());
    slot = cpu.x;
    lsr_a();
    lsr_a();
    and_a(0xF0);
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xC21F + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    lda(rd8((uint16_t)(0xC220 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_FlickerWaterThenNext07DD(void)
{
    FlickerWaterThenNext07DD(cpu.x);
}
