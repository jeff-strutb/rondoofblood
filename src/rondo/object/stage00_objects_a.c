/*
 * Stage 00's last objects: the gate and boss dispatchers and the fly-in at 025B.
 */
#include "stage_obj_ops.h"
#include "stage00_objects_a.h"
#include "calls_b.h"

void HoldByWorkD025B(uint8_t slot);
void HoldByWorkE025B(uint8_t slot);

static void fade_cd(void)
{
    cd_fade_exit_shim(vec_cd_fade(cpu.a));
}

/* PHX, LDA script, LDX #5, StartScriptSlotKind2, PLX. */
static void script_in_slot5(uint8_t script, uint16_t ret)
{
    phx();
    lda(script);
    ldx(0x05);
    JSR(ret, script_slot_exit_shim(StartScriptSlotKind2(cpu.a, cpu.x)));
    plx();
}

/*
 * @routine 0250:1BB StageGateByState
 * @run 6B:99BB
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $99CD.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StageGateByState(uint8_t slot)
{
    uint8_t lo, hi;

    lda(RD8(OBJ->state[slot]));
    asl_a();
    tay();
    lo = rd8((uint16_t)(0x99CD + cpu.y));
    lda(lo);
    WR8(ZP->temp00, cpu.a);
    hi = rd8((uint16_t)(0x99CE + cpu.y));
    lda(hi);
    WR8(ZP->temp01, cpu.a);
    /* the handler at $99EA is the RTS ending StageGateShow */
    if ((lo | (hi << 8)) != 0x99EA) {
        run_routine((uint16_t)(lo | (hi << 8)));
    }
}

void abi_StageGateByState(void)
{
    StageGateByState(cpu.x);
}

/*
 * @routine 025A:6FC ByState025AAEFC
 * @run 70:AEFC
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $AF0E.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState025AAEFC(uint8_t slot)
{
    jump_by_state(slot, 0xAF0E);
}

void abi_ByState025AAEFC(void)
{
    ByState025AAEFC(cpu.x);
}

/*
 * @routine 025B:36F FlyInAndAnnounce025B
 * @run 70:B36F
 * WHAT IT DOES: Pieces flying in to their places with the fade held: after the flight, number 4 starts script $82, then $81..$83 in turn by state, and the last wait sets sub-step 3 and zField8C $83.
 * HOW: Tables by the argument; the holds drop out of this routine ($B441, $B446) until their counts end.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyInAndAnnounce025B(uint8_t slot)
{
    (void)slot;
    call_routine(0xB372, 0xAF22);
    lda(0x01);
    WR8(ZP->fade_active, cpu.a);
    lda(RD8(OBJ->state[cpu.x]));
    if (EQ) {
        JSR(0xB37E, k_load_record());
        ldy(RD8(OBJ->start_arg[cpu.x]));
        tya();
        clc();
        adc_a(0x1E);
        WR8(OBJ->frame[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB44E + cpu.y)));
        WR8(OBJ->x_lo[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB458 + cpu.y)));
        WR8(OBJ->x_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB453 + cpu.y)));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB45D + cpu.y)));
        WR8(OBJ->y_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB462 + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB467 + cpu.y)));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xB46C + cpu.y)));
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        JSR(0xB3B9, k_move());
        abi_nz(DEC8(OBJ->work_d[cpu.x]));
        if (NE) {
            return;
        }
        lda(RD8(OBJ->start_arg[cpu.x]));
        cmp(cpu.a, 0x04);
        if (EQ) {
            phx();
            ldx(0x05);
            lda(0x82);
            JSR(0xB3CD, k_script1());
            plx();
        }
        abi_nz(INC8(OBJ->state[cpu.x]));
        lda(0x05);
        WR8(OBJ->work_e[cpu.x], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        lda(RD8(OBJ->start_arg[cpu.x]));
        cmp(cpu.a, 0x04);
        if (NE) {
            return;
        }
        lda(rd8(BiosJoyTrigger));
        and_a(0x08);
        if (EQ) {
            jsr_push(0xB3EB);
            HoldByWorkD025B(cpu.x);
        if (NE) {
                return;
            }
            jsr_pop();
            jsr_push(0xB3EE);
            HoldByWorkE025B(cpu.x);
        if (NE) {
                return;
            }
            jsr_pop();
        }
        lda(0x0C);
        JSR(0xB3F3, fade_cd());
        script_in_slot5(0x81, 0xB3FB);
        lda(0x50);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        abi_nz(INC8(OBJ->state[cpu.x]));
        return;
    }
    dec_a();
    if (EQ) {
        jsr_push(0xB40B);
        HoldByWorkD025B(cpu.x);
        if (NE) {
            return;
        }
        jsr_pop();
        script_in_slot5(0x82, 0xB413);
        abi_nz(INC8(OBJ->state[cpu.x]));
        lda(0x80);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        jsr_push(0xB423);
        HoldByWorkD025B(cpu.x);
        if (NE) {
            return;
        }
        jsr_pop();
        script_in_slot5(0x83, 0xB42B);
        abi_nz(INC8(OBJ->state[cpu.x]));
        lda(0x80);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    jsr_push(0xB438);
    HoldByWorkD025B(cpu.x);
        if (NE) {
        return;
    }
    jsr_pop();
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
    lda(0x83);
    WR8(ZP->field8c, cpu.a);
}

void abi_FlyInAndAnnounce025B(void)
{
    FlyInAndAnnounce025B(cpu.x);
}

/*
 * @routine 025B:441 HoldByWorkE025B
 * @run 70:B441
 * WHAT IT DOES: As HoldByWorkD025B on OBJ_PIECE_PLACE.
 * HOW: A DEC, then a branch into HoldByWorkD025B's test.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoldByWorkE025B(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_e[slot]);

    abi_nz(left);
    if (left != 0) {
        (void)pull8();
        cpu.a = pull8();
        abi_nz(cpu.a);
    }
}

void abi_HoldByWorkE025B(void)
{
    HoldByWorkE025B(cpu.x);
}

/*
 * @routine 025B:446 HoldByWorkD025B
 * @run 70:B446
 * WHAT IT DOES: Counts the object's hold down; while it lasts the caller is not returned to: the return goes to the caller's caller.
 * HOW: DEC OBJ_PIECE_NEXT; not zero, two PLA before the RTS.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoldByWorkD025B(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_d[slot]);

    abi_nz(left);
    if (left != 0) {
        (void)pull8();
        cpu.a = pull8();
        abi_nz(cpu.a);
    }
}

void abi_HoldByWorkD025B(void)
{
    HoldByWorkD025B(cpu.x);
}

/*
 * @routine 025C:2C9 SoundWhenHurtThenByStateBADE
 * @run 70:BAC9
 * WHAT IT DOES: Plays the hurt sound in states 1 to 3, then goes to the state's handler.
 * HOW: SoundWhenHurtInStatesOneToThree, then a table at $BADE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SoundWhenHurtThenByStateBADE(uint8_t slot)
{
    (void)slot;
    call_routine(0xBACC, 0xBAEA);
    jump_by_state(cpu.x, 0xBADE);
}

void abi_SoundWhenHurtThenByStateBADE(void)
{
    SoundWhenHurtThenByStateBADE(cpu.x);
}

/*
 * @routine 025C:6B9 ScriptByWaveThenByStateBECE
 * @run 70:BEB9
 * WHAT IT DOES: Starts the wave bit's script, then goes to the state's handler.
 * HOW: StartScriptByWaveBit, then a table at $BECE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ScriptByWaveThenByStateBECE(uint8_t slot)
{
    (void)slot;
    call_routine(0xBEBC, 0xBED6);
    jump_by_state(cpu.x, 0xBECE);
}

void abi_ScriptByWaveThenByStateBECE(void)
{
    ScriptByWaveThenByStateBECE(cpu.x);
}
