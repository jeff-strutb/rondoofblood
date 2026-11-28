/*
 * More routines of Stage 06's overlays: palette effects, state and substate tables, parts that swing and split, platform
 * pairs that sink and rise, timed and ranged starts, walkers that close in, back off, jump and lunge.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "ops6280.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "stage_ops_c.h"
#include "stage06_more_c.h"

/*
 * @routine 0A9A:7B7 RunByState0A9A
 * @run 70:AFB7
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9AHandlers = 0xAFC9,   /* the handler address for each state, low and high byte */
};

void RunByState0A9A(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9AHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9AHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9A(void)
{
    RunByState0A9A(cpu.x);
}

/*
 * @routine 0A9B:006 SwingDirectionAbout40_0A9B
 * @run 70:B006
 * WHAT IT DOES: Follows the slot before it and moves, checks its partner or the hit, lets the last part split off,
 * then swings its direction (OBJ_WORK_F) about $40: the 16-bit step OBJ_WORK_G:OBJ_WORK_H is added to
 * OBJ_WORK_E:OBJ_WORK_F and pulled back by $0A each frame toward $40 (Stage 06's overlay).
 * HOW: Three calls, a 16-bit add, a compare with $40 and a 16-bit add or subtract of $0A.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwingDirectionAbout40_0A9B(void)
{
    call_routine(0xB009, 0xB04A);
    call_routine(0xB00C, 0xB33F);
    jsr_push(0xB00F);
    LastPartBecomeKind2C0A9B();
    jsr_pop();
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_clc();
    op_adc(RD8(OBJ->work_g[cpu.x]));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_adc(RD8(OBJ->work_h[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_cmp(0x40);
    if (flag_c()) {
        op_lda(RD8(OBJ->work_g[cpu.x]));
        op_sec();
        op_sbc(0x0A);
        WR8(OBJ->work_g[cpu.x], cpu.a);
        op_lda(RD8(OBJ->work_h[cpu.x]));
        op_sbc(0x00);
        WR8(OBJ->work_h[cpu.x], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_g[cpu.x]));
    op_clc();
    op_adc(0x0A);
    WR8(OBJ->work_g[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_h[cpu.x]));
    op_adc(0x00);
    WR8(OBJ->work_h[cpu.x], cpu.a);
}

void abi_SwingDirectionAbout40_0A9B(void)
{
    SwingDirectionAbout40_0A9B();
}

/*
 * @routine 0A9B:2D8 LastPartBecomeKind2C0A9B
 * @run 70:B2D8
 * WHAT IT DOES: For the last part (OBJ_WORK_I 5): counts OBJ_WORK_J down; at 0, when the player is $28 or more
 * away, fewer than two kind $2C are out and a slot from $30 is free, it becomes a kind $2C there and puts the six
 * parts (the five slots before it and itself) in state 3 (Stage 06's overlay).
 * HOW: DEC; DistanceXToPlayer, CountKind2CFrom30_0A9B, FindFreeSlotFrom30, BecomeKind2CInY0A9B; a loop of six
 * stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LastPartBecomeKind2C0A9B(void)
{
    op_lda(RD8(OBJ->work_i[cpu.x]));
    op_cmp(0x05);
    if (flag_z()) {
        op_lda(RD8(OBJ->work_j[cpu.x]));
        if (!flag_z()) {
            RMW8(OBJ->work_j[cpu.x], v, f_dec(v));
            return;
        }
        jsr_push(0xB2EB);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x28);
        if (flag_c()) {
            call_routine(0xB2F2, 0xB312);
            op_cmp(0x02);
            if (!flag_c()) {
                {
                    uint8_t a;
                    bool found;
                    uint8_t found_slot;

                    jsr_push(0xB2F9);
                    found_slot = FindFreeSlotFrom30(&a, &found);
                    find_slot_exit_shim(found_slot, a, found);
                    jsr_pop();
                }
                if (flag_c()) {
                    call_routine(0xB2FE, 0xB327);
                    op_txa();
                    op_sec();
                    op_sbc(0x05);
                    op_tay();
                    op_lda(0x06);
                    WR8(ZP->temp00, cpu.a);
                    do {
                        op_lda(0x03);
                        WR8(OBJ->state[cpu.y], cpu.a);
                        op_iny();
                        RMW8(ZP->temp00, v, f_dec(v));
                    } while (!flag_z());
                }
            }
        }
    }
}

void abi_LastPartBecomeKind2C0A9B(void)
{
    LastPartBecomeKind2C0A9B();
}

/*
 * @routine 0A9B:379 RunByState0A9B_379
 * @run 70:B379
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9B_379Handlers = 0xB38B,   /* the handler address for each state, low and high byte */
};

void RunByState0A9B_379(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9B_379Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9B_379Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9B_379(void)
{
    RunByState0A9B_379(cpu.x);
}

/*
 * @routine 0A9B:391 LoadRecordAndEvent0A9B
 * @run 70:B391
 * WHAT IT DOES: Loads the object's record and records its event (Stage 06's overlay).
 * HOW: LoadObjectRecord, then a tail into RecordObjectEvent.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordAndEvent0A9B(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB394);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
    }
}

void abi_LoadRecordAndEvent0A9B(void)
{
    LoadRecordAndEvent0A9B();
}

/*
 * @routine 0A9B:3D0 RunByState0A9B_3D0
 * @run 70:B3D0
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9B_3D0Handlers = 0xB3E2,   /* the handler address for each state, low and high byte */
};

void RunByState0A9B_3D0(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9B_3D0Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9B_3D0Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9B_3D0(void)
{
    RunByState0A9B_3D0(cpu.x);
}

/*
 * @routine 0A9B:413 FallAndMove0A9B
 * @run 70:B413
 * WHAT IT DOES: Lets the object fall with gravity and moves it by its speeds (Stage 06's overlay).
 * HOW: ApplyGravityUnlimited, then a tail into ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallAndMove0A9B(void)
{
    jsr_push(0xB416);
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_FallAndMove0A9B(void)
{
    FallAndMove0A9B();
}

/*
 * @routine 0A9B:419 RunByState0A9B_419
 * @run 70:B419
 * WHAT IT DOES: Runs the handler of the object's state; state 0's is LoadObjectRecord itself (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9B_419Handlers = 0xB42B,   /* the handler address for each state, low and high byte */
};

void RunByState0A9B_419(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9B_419Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9B_419Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9B_419(void)
{
    RunByState0A9B_419(cpu.x);
}

/*
 * @routine 0A9B:4CF BuildListsUntilScrollY6_0A9B
 * @run 70:B4CF
 * WHAT IT DOES: State 0 loads the record, builds the stage lists of section 3 (zField82 3 for ExpandStageListsI,
 * then 2) and sets zField86 7; with OBJ_START_ARG set it also sets zField82 3, becomes kind $C2 in state 0 at X
 * $0080, Y $00FE with OBJ_START_ARG $0D and starts list $87 on list channel 4. Later states wait for the scroll's Y
 * high byte to be 6, then clear CameraMode, set zField82 2 and remove the object (Stage 06's overlay).
 * HOW: LoadObjectRecord, ExpandStageListsI, stores, StartListChannelKind2; a compare, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BuildListsUntilScrollY6_0A9B(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB4D7);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x03);
        WR8(ZP->field82, cpu.a);
        op_phx();
        call_routine(0xB4DF, 0x9277);
        op_plx();
        op_lda(0x02);
        WR8(ZP->field82, cpu.a);
        op_lda(0x07);
        WR8(ZP->field86, cpu.a);
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (!flag_z()) {
            op_lda(0x03);
            WR8(ZP->field82, cpu.a);
            op_lda(0xC2);
            WR8(OBJ->kind[cpu.x], cpu.a);
            WR8(OBJ->state[cpu.x], 0);
            WR8(OBJ->x_hi[cpu.x], 0);
            WR8(OBJ->y_hi[cpu.x], 0);
            op_lda(0x80);
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            op_lda(0xFE);
            WR8(OBJ->y_lo[cpu.x], cpu.a);
            op_lda(0x0D);
            WR8(OBJ->start_arg[cpu.x], cpu.a);
            op_phx();
            op_lda(0x87);
            op_ldx(0x04);
            jsr_push(0xB516);
            list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
            jsr_pop();
            op_plx();
        }
        return;
    }
    op_lda(RD8(SCROLLE->y_hi_screen[0x00]));
    op_cmp(0x06);
    if (flag_z()) {
        WR8(SCROLLLIM->camera_mode, 0);
        op_lda(0x02);
        WR8(ZP->field82, cpu.a);
        ClearObject(cpu.x);
        return;
    }
}

void abi_BuildListsUntilScrollY6_0A9B(void)
{
    BuildListsUntilScrollY6_0A9B();
}

/*
 * @routine 0A9B:560 RunByState0A9B_560
 * @run 70:B560
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9B_560Handlers = 0xB572,   /* the handler address for each state, low and high byte */
};

void RunByState0A9B_560(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9B_560Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9B_560Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9B_560(void)
{
    RunByState0A9B_560(cpu.x);
}

/*
 * @routine 0A9B:5AA CountLightHitsThenState0A9B
 * @run 70:B5AA
 * WHAT IT DOES: While the light-hit check passes, counts OBJ_WORK_E down and at 0 steps the state on; when it
 * fails, sets OBJ_WORK_E back to OBJ_WORK_D (Stage 06's overlay).
 * HOW: LightHitCheckByArg0A9B's carry; DEC or INC; a copy.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountLightHitsThenState0A9B(uint8_t slot)
{
    jsr_push(0xB5AD);
    LightHitCheckByArg0A9B(slot);
    jsr_pop();
    if (flag_c()) {
        op_lda(RD8(OBJ->work_e[slot]));
        if (!flag_z()) {
            RMW8(OBJ->work_e[slot], v, f_dec(v));
            return;
        }
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_CountLightHitsThenState0A9B(void)
{
    CountLightHitsThenState0A9B(cpu.x);
}

/*
 * @routine 0A9B:5C3 RunByStartArg0A9B
 * @run 70:B5C3
 * WHAT IT DOES: Runs the handler that OBJ_START_ARG picks (Stage 06's overlay).
 * HOW: A jump table by OBJ_START_ARG.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByStartArg0A9BHandlers = 0xB55C,   /* the handler address for each OBJ_START_ARG, low and high byte */
};

void RunByStartArg0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByStartArg0A9BHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByStartArg0A9BHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByStartArg0A9B(void)
{
    RunByStartArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:5D5 RunBySubstate0A9B
 * @run 70:B5D5
 * WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunBySubstate0A9BHandlers = 0xB5E7,   /* the handler address for each substate, low and high byte */
};

void RunBySubstate0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunBySubstate0A9BHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunBySubstate0A9BHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunBySubstate0A9B(void)
{
    RunBySubstate0A9B(cpu.x);
}

/*
 * @routine 0A9B:633 LightHitCheckByArg0A9B
 * @run 70:B633
 * WHAT IT DOES: Clears the object's OBJ_INVULN_TIMER; hit lightly (OBJ_HIT_TAKEN 1 to 4) it runs the check
 * OBJ_START_ARG picks (for 0, $B653: whether the player is in state 2), otherwise it says no (Stage 06's overlay).
 * HOW: A store, a compare, a jump table by OBJ_START_ARG; CLC.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the check passes; C clear when it fails or the object was not hit lightly.
 */
enum {
    LightHitCheckByArg0A9BChecks = 0xB55E,   /* the check address for each OBJ_START_ARG, low and high byte (RunByStartArg0A9B's table one word on) */
};

void LightHitCheckByArg0A9B(uint8_t slot)
{
    WR8(OBJ->invuln_timer[slot], 0);
    op_lda(RD8(OBJ->hit_taken[slot]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            op_lda(RD8(OBJ->start_arg[slot]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(LightHitCheckByArg0A9BChecks + cpu.y)));
            WR8(ZP->temp00, cpu.a);
            op_lda(rd8((uint16_t)(LightHitCheckByArg0A9BChecks + 1 + cpu.y)));
            WR8(ZP->temp01, cpu.a);
            run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
            return;
        }
    }
    op_clc();
}

void abi_LightHitCheckByArg0A9B(void)
{
    LightHitCheckByArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:71C StartWithTwoKindCB0A9B
 * @run 70:B71C
 * WHAT IT DOES: State 0 loads the record, sets zRasterStage 4, starts two kind $CB objects in slots from $38 that
 * keep its slot, sets its hit box half height $0C and sprite set 6, and plays its frame list; later states go on at
 * $B744 (Stage 06's overlay).
 * HOW: LoadObjectRecord, two calls of StartKindCBFrom38_0A9B, stores, a fall into PlayFixedList0A9B.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithTwoKindCB0A9B(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!flag_z()) {
        PlayUntilHitThenBurst0A9B();
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB724);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x04);
    WR8(ZP->raster_stage, cpu.a);
    jsr_push(0xB72B);
    StartKindCBFrom38_0A9B();
    jsr_pop();
    jsr_push(0xB72E);
    StartKindCBFrom38_0A9B();
    jsr_pop();
    op_lda(0x0C);
    WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
    op_lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    /* falls into $B738 */
    run_routine(0xB738);
}

void abi_StartWithTwoKindCB0A9B(void)
{
    StartWithTwoKindCB0A9B();
}

/*
 * @routine 0A9B:744 PlayUntilHitThenBurst0A9B
 * @run 70:B744
 * WHAT IT DOES: Plays the frame list; on a damaging hit (OBJ_HIT_TAKEN 5 or more) records its event, becomes the
 * kind in OBJ_START_ARG in state 0 with OBJ_INVULN_TIMER $10, plays sound $19, and starts three kind $CA
 * (OBJ_WORK_D 1 and 2 for the second and third) and a kind $9C in free slots from $30, as long as slots are found
 * (Stage 06's overlay).
 * HOW: PlayFixedList0A9B, a compare, RecordObjectEvent, stores, SoundCommand, four FindFreeSlotFrom30 and
 * StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayUntilHitThenBurst0A9B(void)
{
    call_routine(0xB746, 0xB738);
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (flag_c()) {
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xB750);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        WR8(OBJ->kind[cpu.x], cpu.a);
        op_lda(0x10);
        WR8(OBJ->invuln_timer[cpu.x], cpu.a);
        WR8(OBJ->state[cpu.x], 0);
        op_lda(0x19);
        {
            uint8_t command;

            jsr_push(0xB763);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xB766);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0xCA);
            jsr_push(0xB76D);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            jsr_pop();
            {
                uint8_t a;
                bool found;
                uint8_t found_slot;

                jsr_push(0xB770);
                found_slot = FindFreeSlotFrom30(&a, &found);
                find_slot_exit_shim(found_slot, a, found);
                jsr_pop();
            }
            if (flag_c()) {
                op_lda(0xCA);
                jsr_push(0xB777);
                copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
                jsr_pop();
                op_lda(0x01);
                WR8(OBJ->work_d[cpu.y], cpu.a);
                {
                    uint8_t a;
                    bool found;
                    uint8_t found_slot;

                    jsr_push(0xB77F);
                    found_slot = FindFreeSlotFrom30(&a, &found);
                    find_slot_exit_shim(found_slot, a, found);
                    jsr_pop();
                }
                if (flag_c()) {
                    op_lda(0xCA);
                    jsr_push(0xB786);
                    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
                    jsr_pop();
                    op_lda(0x02);
                    WR8(OBJ->work_d[cpu.y], cpu.a);
                    {
                        uint8_t a;
                        bool found;
                        uint8_t found_slot;

                        jsr_push(0xB78E);
                        found_slot = FindFreeSlotFrom30(&a, &found);
                        find_slot_exit_shim(found_slot, a, found);
                        jsr_pop();
                    }
                    if (flag_c()) {
                        op_lda(0x9C);
                        jsr_push(0xB795);
                        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
                        jsr_pop();
                    }
                }
            }
        }
    }
}

void abi_PlayUntilHitThenBurst0A9B(void)
{
    PlayUntilHitThenBurst0A9B();
}

/*
 * @routine 0A9B:796 StartKindCBFrom38_0A9B
 * @run 70:B796
 * WHAT IT DOES: Starts a kind $CB at the object's place in the first empty slot from $38, with the object's slot in
 * its OBJ_START_ARG (Stage 06's overlay).
 * HOW: FreeSlotFrom38_0A9B, StartObjectAtPlace, a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKindCBFrom38_0A9B(void)
{
    call_routine(0xB799, 0xB7A5);
    if (flag_c()) {
        op_lda(0xCB);
        jsr_push(0xB7A0);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_txa();
        WR8(OBJ->start_arg[cpu.y], cpu.a);
    }
}

void abi_StartKindCBFrom38_0A9B(void)
{
    StartKindCBFrom38_0A9B();
}

/*
 * @routine 0A9C:1AA ShowFrameByArg0A9C
 * @run 70:B9AA
 * WHAT IT DOES: In state 0 loads the record and shows the frame in OBJ_START_ARG with sprite set 6 (Stage 06's
 * overlay).
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ShowFrameByArg0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB9B2);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
    }
}

void abi_ShowFrameByArg0A9C(void)
{
    ShowFrameByArg0A9C(cpu.x);
}

/*
 * @routine 0A9C:32B SinkPairWhileStoodOn0A9C
 * @run 70:BB2B
 * WHAT IT DOES: A pair of slots (the object and the one after it) that the player stands on: in state 1, standing
 * on it while not rising, it sinks a pixel a frame down to Y $A9 while the pair in OBJ_START_ARG rises a pixel and
 * slot $2E shakes about slot $2F's place, with sound $AB once (stopped by ad_stop when he steps off); in state 2
 * the first pair (OBJ_START_ARG 0) rises at speed 3 to Y $90, the other, once OBJ_WORK_D is set, sinks while its
 * partner rises, and with the player above Y $10 sets zField83 5, zField87 0 and zSubStep 2; from state 3 it
 * springs back to Y $90 with its second slot $A0 above, and dips once at Y speed $1C0 when the player lands on it
 * at X $48 or more (Stage 06's overlay).
 * HOW: ContactWithPlayer with a $1A by 6 box, 16-bit INC and DEC of the Y positions, RandomMinusOneToOne0A9C,
 * ApplyVelocity on both slots, RiseToYInA0A9C.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SinkPairWhileStoodOn0A9C(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!flag_z()) {
        goto at_BB55;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBB33);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (!flag_z()) {
        goto at_BBAA;
    }
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xBB3B);
        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
    op_inx();
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xBB3F);
        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
    op_dex();
    RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    MarkAndSoundAB0A9C(cpu.x);
    return;
at_BB49:
    op_lda(RD8(OBJ->work_f[cpu.x]));
    if (!flag_z()) {
        jsr_push(0xBB51);
        ad_stop_exit_shim(vec_ad_stop());
        jsr_pop();
        WR8(OBJ->work_f[cpu.x], 0);
    }
    return;
at_BB55:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_BBC3;
    }
    op_lda(0x1A);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x06);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xBB63, 0xAA35);
    if (!flag_c()) {
        goto at_BB49;
    }
    op_lda(RD8(OBJ->vy_hi[0x00]));
    if (flag_n()) {
        goto at_BB49;
    }
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_cmp(0xA9);
    if (flag_c()) {
        goto at_BB49;
    }
    call_routine(0xBB74, 0xBC67);
    RMW8(OBJ->y_lo[cpu.x], v, f_inc(v));
    RMW8(OBJ->y_lo[cpu.x + 1], v, f_inc(v));
    if (flag_z()) {
        RMW8(OBJ->y_hi[cpu.x + 1], v, f_inc(v));
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_phx();
    op_tax();
    RMW8(OBJ->y_lo[cpu.x], v, f_dec(v));
    RMW8(OBJ->y_lo[cpu.x + 1], v, f_dec(v));
    op_lda(RD8(OBJ->y_lo[cpu.x + 1]));
    cpu.a = f_inc(cpu.a);
    if (flag_z()) {
        RMW8(OBJ->y_hi[cpu.x + 1], v, f_dec(v));
    }
    op_ldx(0x2E);
    call_routine(0xBB98, 0xBC5A);
    op_clc();
    op_adc(RD8(OBJ->x_lo[cpu.x + 1]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    call_routine(0xBBA2, 0xBC5A);
    op_clc();
    op_adc(RD8(OBJ->y_lo[cpu.x + 1]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_plx();
at_BBAA:
    return;
at_BBAB:
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_cmp(0x90);
    if (!flag_c()) {
        op_lda(0x90);
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        op_lda(0xF0);
        WR8(OBJ->y_lo[cpu.x + 1], cpu.a);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    op_txa();
    goto at_BBF1;
at_BBC3:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_BC16;
    }
    op_lda(0x1A);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x06);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xBBD1, 0xAA35);
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (flag_z()) {
        goto at_BBAB;
    }
    op_lda(RD8(OBJ->work_d[cpu.x]));
    if (flag_z()) {
        goto at_BBAA;
    }
    call_routine(0xBBDE, 0xBC67);
    op_lda(0x03);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    WR8(OBJ->vy_hi[cpu.x + 1], cpu.a);
    jsr_push(0xBBE9);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_inx();
    jsr_push(0xBBED);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_dex();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
at_BBF1:
    op_phx();
    op_tax();
    op_lda(0xFD);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    WR8(OBJ->vy_hi[cpu.x + 1], cpu.a);
    jsr_push(0xBBFE);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_inx();
    jsr_push(0xBC02);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_dex();
    op_plx();
    op_lda(RD8(OBJ->y_lo[0x00]));
    op_cmp(0x10);
    if (flag_c()) {
        goto at_BBAA;
    }
    op_lda(0x05);
    WR8(ZP->field83, cpu.a);
    WR8(ZP->field87, 0);
    op_lda(0x02);
    WR8(ZP->sub_step, cpu.a);
    return;
at_BC16:
    op_lda(0x90);
    jsr_push(0xBC1B);
    RiseToYInA0A9C(cpu.x);
    jsr_pop();
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_sec();
    op_sbc(0xA0);
    WR8(OBJ->y_lo[cpu.x + 1], cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_sbc(0x00);
    WR8(OBJ->y_hi[cpu.x + 1], cpu.a);
    op_lda(0x1A);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x06);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xBC37, 0xAA35);
    if (flag_c()) {
        op_lda(RD8(OBJ->work_e[cpu.x]));
        if (flag_z()) {
            op_lda(RD8(OBJ->x_lo[0x00]));
            op_cmp(0x48);
            if (flag_c()) {
                call_routine(0xBC48, 0xBC67);
                op_lda(0x01);
                WR8(OBJ->vy_hi[cpu.x], cpu.a);
                op_lda(0xC0);
                WR8(OBJ->vy_lo[cpu.x], cpu.a);
                RMW8(OBJ->work_e[cpu.x], v, f_inc(v));
            }
        }
        return;
    }
    WR8(OBJ->work_e[cpu.x], 0);
}

void abi_SinkPairWhileStoodOn0A9C(void)
{
    SinkPairWhileStoodOn0A9C();
}

/*
 * @routine 0A9C:46C MarkAndSoundAB0A9C
 * @run 70:BC6C
 * WHAT IT DOES: Sets OBJ_WORK_F on and plays sound $AB: the second half of SoundOnce0A9C, which state 0 of
 * SinkPairWhileStoodOn0A9C enters directly (Stage 06's overlay).
 * HOW: INC, then SoundCommand.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MarkAndSoundAB0A9C(uint8_t slot)
{
    RMW8(OBJ->work_f[slot], v, f_inc(v));
    op_lda(0xAB);
    {
        uint8_t command;

        jsr_push(0xBC74);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    /* falls into $BC74 */
    run_routine(0xBC74);
}

void abi_MarkAndSoundAB0A9C(void)
{
    MarkAndSoundAB0A9C(cpu.x);
}

/*
 * @routine 0A9C:475 ShakeThenDropOnPair0A9C
 * @run 70:BC75
 * WHAT IT DOES: State 0 loads the record; state 1 waits for a damaging hit, then shakes about the next slot's place
 * for $18 frames; after that it falls, and on coming within 8 pixels above the slot in OBJ_START_ARG it records its
 * event, steps that slot's state and OBJ_WORK_D on and the state of the slot in that one's OBJ_START_ARG (recording
 * the events of both pairs), then falls on at Y speed 3 and is removed once its Y high byte is set and its Y low
 * byte is $80 or more (Stage 06's overlay).
 * HOW: A compare of OBJ_HIT_TAKEN, DEC, RandomMinusTwoToTwo0A9C, a gravity add of $28, ApplyVelocity,
 * RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ShakeThenDropOnPair0A9C(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->hit_taken[cpu.x]));
        op_cmp(0x05);
        if (!flag_c()) {
            goto at_BCBC;
        }
        op_lda(0x18);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_BCBD;
    }
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (!flag_z()) {
        call_routine(0xBC9B, 0xBD22);
        op_clc();
        op_adc(RD8(OBJ->x_lo[cpu.x + 1]));
        WR8(OBJ->x_lo[cpu.x], cpu.a);
        call_routine(0xBCA5, 0xBD22);
        op_clc();
        op_adc(RD8(OBJ->y_lo[cpu.x + 1]));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        return;
    }
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->x_lo[cpu.x + 1]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x + 1]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
at_BCBC:
    return;
at_BCBD:
    op_lda(RD8(OBJ->work_e[cpu.x]));
    if (flag_z()) {
        op_lda(RD8(OBJ->vy_lo[cpu.x]));
        op_clc();
        op_adc(0x28);
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        if (flag_c()) {
            RMW8(OBJ->vy_hi[cpu.x], v, f_inc(v));
        }
        jsr_push(0xBCD3);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_ldy(RD8(OBJ->start_arg[cpu.x]));
        op_lda(RD8(OBJ->y_lo[cpu.y]));
        op_sec();
        op_sbc(RD8(OBJ->y_lo[cpu.x]));
        if (flag_c()) {
            op_cmp(0x08);
            if (flag_c()) {
                goto at_BCBC;
            }
        }
        RMW8(OBJ->work_e[cpu.x], v, f_inc(v));
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xBCE9);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        op_phx();
        op_tax();
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xBCF7);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_inx();
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xBCFB);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_dex();
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        op_tax();
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xBD06);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_inx();
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xBD0A);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_dex();
        op_plx();
        op_lda(0x03);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        return;
    }
    jsr_push(0xBD15);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    if (flag_z()) {
        goto at_BCBC;
    }
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    if (!flag_n()) {
        goto at_BCBC;
    }
    ClearObject(cpu.x);
}

void abi_ShakeThenDropOnPair0A9C(void)
{
    ShakeThenDropOnPair0A9C();
}

/*
 * @routine 0A9C:52F FallWhenSlotBeforeGone0A9C
 * @run 70:BD2F
 * WHAT IT DOES: State 0 loads the record with frame $11 and sprite set 6; then, with a Y speed and the slot before
 * it empty, stops the ADPCM sound the first time (OBJ_WORK_D) and moves by its speeds (Stage 06's overlay).
 * HOW: LoadObjectRecord, stores; tests of the Y speed and the kind of X - 1, ad_stop, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallWhenSlotBeforeGone0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBD37);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x11);
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->vy_hi[slot]));
    if (!flag_z()) {
        op_lda(rd8((uint16_t)(COL(OBJ->kind) - 1 + slot)));
        if (flag_z()) {
            op_lda(RD8(OBJ->work_d[slot]));
            if (flag_z()) {
                RMW8(OBJ->work_d[slot], v, f_inc(v));
                jsr_push(0xBD57);
                ad_stop_exit_shim(vec_ad_stop());
                jsr_pop();
            }
            jsr_push(0xBD5A);
            axis_move_exit_shim(ApplyVelocity(slot));
            jsr_pop();
        }
    }
}

void abi_FallWhenSlotBeforeGone0A9C(void)
{
    FallWhenSlotBeforeGone0A9C(cpu.x);
}

/*
 * @routine 0A9C:55B RiseToYInA0A9C
 * @run 70:BD5B
 * WHAT IT DOES: Moves the object by its Y speed, speeding it up by $40 a frame while it is below the Y in A; at or
 * above that Y puts it there and stops it, stopping the ADPCM sound if OBJ_WORK_F is set (Stage 06's overlay).
 * HOW: ApplyVelocityY, a compare, a 16-bit subtract, stores, ad_stop.
 * WAITS: no.
 * Entry: X the object; A the Y to rise to. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RiseToYInA0A9C(uint8_t slot)
{
    WR8(ZP->temp00, cpu.a);
    jsr_push(0xBD60);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_lo[slot]));
    op_cmp(RD8(ZP->temp00));
    if (!flag_z()) {
        if (flag_c()) {
            op_lda(RD8(OBJ->vy_lo[slot]));
            op_sec();
            op_sbc(0x40);
            WR8(OBJ->vy_lo[slot], cpu.a);
            op_lda(RD8(OBJ->vy_hi[slot]));
            op_sbc(0x00);
            WR8(OBJ->vy_hi[slot], cpu.a);
            return;
        }
    }
    op_lda(RD8(ZP->temp00));
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    op_lda(RD8(OBJ->work_f[slot]));
    if (!flag_z()) {
        jsr_push(0xBD8E);
        ad_stop_exit_shim(vec_ad_stop());
        jsr_pop();
        WR8(OBJ->work_f[slot], 0);
    }
}

void abi_RiseToYInA0A9C(void)
{
    RiseToYInA0A9C(cpu.x);
}

/*
 * @routine 0A9D:5AA StartKind27InXRange0A9D
 * @run 71:C5AA
 * WHAT IT DOES: State 0 loads the record; later counts the 16-bit OBJ_WORK_D:OBJ_WORK_E down and at 0, with the
 * player inside the object's X range and an empty slot in $30 to $32, starts a kind $27 there (Stage 06's overlay).
 * HOW: A 16-bit decrement, PlayerXInRange0A9D, a search loop, then $C5DE.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKind27InXRange0A9D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    op_ora(RD8(OBJ->work_e[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_e[slot]));
        op_sec();
        op_sbc(0x01);
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(RD8(OBJ->work_d[slot]));
        op_sbc(0x00);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    jsr_push(0xC5CF);
    PlayerXInRange0A9D(slot);
    jsr_pop();
    if (!flag_c()) {
        PlainReturn0A9D();
        return;
    }
    op_ldy(0x30);
    do {
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (flag_z()) {
            StartKind27InY0A9D(slot);
            return;
        }
        op_iny();
        op_cpy(0x33);
    } while (!flag_c());
    /* falls into $C5DD */
    PlainReturn0A9D();
}

void abi_StartKind27InXRange0A9D(void)
{
    StartKind27InXRange0A9D(cpu.x);
}

/*
 * @routine 0A9D:5DD PlainReturn0A9D
 * @run 71:C5DD
 * WHAT IT DOES: Returns with the flags as they are; the X range test and the timed start jump here to leave (Stage
 * 06's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlainReturn0A9D(void)
{
}

void abi_PlainReturn0A9D(void)
{
    PlainReturn0A9D();
}

/*
 * @routine 0A9D:5DE StartKind27InY0A9D
 * @run 71:C5DE
 * WHAT IT DOES: Clears slot Y, starts a kind $27 in it at the object's place and sets the object's timer
 * OBJ_WORK_D:OBJ_WORK_E to $0080 (Stage 06's overlay).
 * HOW: ClearObjectInY, StartObjectAtPlace, two stores.
 * WAITS: no.
 * Entry: X the object; Y the slot. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKind27InY0A9D(uint8_t slot)
{
    jsr_push(0xC5E1);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x27);
    jsr_push(0xC5E6);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    WR8(OBJ->work_d[slot], 0);
    op_lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_StartKind27InY0A9D(void)
{
    StartKind27InY0A9D(cpu.x);
}

/*
 * @routine 0A9D:5EF PlayerXInRange0A9D
 * @run 71:C5EF
 * WHAT IT DOES: Says whether the player's place across the stage (the scroll's X plus the player's X low byte, with
 * the object's X high byte in the high byte) lies in the range OBJ_START_ARG picks, from its low end up to below
 * its high end (Stage 06's overlay).
 * HOW: Four times OBJ_START_ARG indexes four-byte records; 16-bit compares high byte first.
 * WAITS: no.
 * Entry: X the object. Exit: C set when inside the range; C clear when outside.
 */
enum {
    PlayerXInRange0A9DRanges = 0xC626,   /* low end low, high, high end low, high per OBJ_START_ARG */
};

void PlayerXInRange0A9D(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(SCROLL->x_lo[0x00]));
    op_clc();
    op_adc(RD8(OBJ->x_lo[0x00]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
    op_adc(RD8(OBJ->x_hi[slot]));
    op_cmp(rd8((uint16_t)(PlayerXInRange0A9DRanges + 3 + cpu.y)));
    if (!flag_c()) {
        goto at_C616;
    }
    if (flag_z()) {
        goto at_C60D;
    }
at_C60B:
    op_clc();
    return;
at_C60D:
    op_lda(RD8(ZP->temp00));
    op_cmp(rd8((uint16_t)(PlayerXInRange0A9DRanges + 2 + cpu.y)));
    if (flag_c()) {
        goto at_C60B;
    }
at_C614:
    op_sec();
    return;
at_C616:
    op_cmp(rd8((uint16_t)(PlayerXInRange0A9DRanges + 1 + cpu.y)));
    if (!flag_c()) {
        PlainReturn0A9D();
        return;
    }
    if (!flag_z()) {
        goto at_C614;
    }
    op_lda(RD8(ZP->temp00));
    op_cmp(rd8((uint16_t)(PlayerXInRange0A9DRanges + cpu.y)));
    if (!flag_c()) {
        PlainReturn0A9D();
        return;
    }
    op_sec();
}

void abi_PlayerXInRange0A9D(void)
{
    PlayerXInRange0A9D(cpu.x);
}

/*
 * @routine 0A9D:7EC FacePlayerThenChoose0A9D
 * @run 71:C7EC
 * WHAT IT DOES: Turns the object to face the player, clears OBJ_WORK_D, makes the choice for the player's distance
 * and steps the state on (Stage 06's overlay).
 * HOW: SideOfPlayer's bit 0, a store, ChooseByPlayerDistance0A9D, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FacePlayerThenChoose0A9D(void)
{
    jsr_push(0xC7EF);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    WR8(OBJ->work_d[cpu.x], 0);
    call_routine(0xC7FA, 0xC7FE);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
}

void abi_FacePlayerThenChoose0A9D(void)
{
    FacePlayerThenChoose0A9D();
}

/*
 * @routine 0A9E:078 SetWorkD4_0A9E
 * @run 71:C878
 * WHAT IT DOES: Sets OBJ_WORK_D to 4 (Stage 06's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetWorkD4_0A9E(uint8_t slot)
{
    op_lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SetWorkD4_0A9E(void)
{
    SetWorkD4_0A9E(cpu.x);
}

/*
 * @routine 0A9E:13F ApproachUntilNear0A9E
 * @run 71:C93F
 * WHAT IT DOES: Walks toward the player (OBJ_WORK_E from his side); closer than $4C pixels it goes to state 3; then
 * checks the ground ahead (Stage 06's overlay).
 * HOW: WalkByWorkE0A9E, SideOfPlayer, DistanceXToPlayer, a store, then $C9E9.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ApproachUntilNear0A9E(void)
{
    jsr_push(0xC942);
    WalkByWorkE0A9E();
    jsr_pop();
    jsr_push(0xC945);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xC94D);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x4C);
    if (!flag_c()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    StopAtLedgeOrWall0A9E(cpu.x);
}

void abi_ApproachUntilNear0A9E(void)
{
    ApproachUntilNear0A9E();
}

/*
 * @routine 0A9E:159 WalkByWorkE0A9E
 * @run 71:C959
 * WHAT IT DOES: Faces the player and plays frame list 0; as each step begins, takes the X speed OBJ_WORK_E picks
 * ($FC left, $04 right) and moves across (Stage 06's overlay).
 * HOW: FacePlayerPlayList0A9E with Y 0, its carry, a table read, ApplyVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WalkByWorkE0A9ESpeeds = 0xC96C,   /* the X speed high byte for each OBJ_WORK_E */
};

void WalkByWorkE0A9E(void)
{
    op_cly();
    call_routine(0xC95D, 0xC96E);
    if (flag_c()) {
        op_ldy(RD8(OBJ->work_e[cpu.x]));
        op_lda(rd8((uint16_t)(WalkByWorkE0A9ESpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        jsr_push(0xC96B);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
    }
}

void abi_WalkByWorkE0A9E(void)
{
    WalkByWorkE0A9E();
}

/*
 * @routine 0A9E:1E9 StopAtLedgeOrWall0A9E
 * @run 71:C9E9
 * WHAT IT DOES: Probes the floor (height 8): with floor under all three points and no wall, counts OBJ_WORK_D down
 * and at 0 restarts the animation in state 4; off a ledge or at a wall restarts it in state 5, substate 0 (Stage
 * 06's overlay).
 * HOW: ProbeTerrain with 9, then with $22; an AND of the three TileHit bytes; DEC, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StopAtLedgeOrWall0A9E(uint8_t slot)
{
    op_lda(0x08);
    WR8(ZP->product.mid, cpu.a);
    op_lda(0x09);
    jsr_push(0xC9F2);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    op_lda(RD8(LEVEL->tile_hit[0]));
    op_and(RD8(LEVEL->tile_hit[1]));
    op_and(RD8(LEVEL->tile_hit[2]));
    op_and(0x01);
    if (!flag_z()) {
        op_lda(0x08);
        WR8(ZP->product.mid, cpu.a);
        op_lda(0x22);
        jsr_push(0xCA08);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x02);
        if (flag_z()) {
            goto at_CA1B;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
    op_lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    return;
at_CA1B:
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_StopAtLedgeOrWall0A9E(void)
{
    StopAtLedgeOrWall0A9E(cpu.x);
}

/*
 * @routine 0A9E:22C BackAwayUntilFar0A9E
 * @run 71:CA2C
 * WHAT IT DOES: Walks away from the player (OBJ_WORK_E the other side); $6C pixels or more away it goes to state 2;
 * then checks the ground ahead (Stage 06's overlay).
 * HOW: WalkByWorkE0A9E, SideOfPlayer, EOR, DistanceXToPlayer, a store, then $C9E9.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackAwayUntilFar0A9E(void)
{
    jsr_push(0xCA2F);
    WalkByWorkE0A9E();
    jsr_pop();
    jsr_push(0xCA32);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    op_eor(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xCA3C);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x6C);
    if (flag_c()) {
        op_lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    StopAtLedgeOrWall0A9E(cpu.x);
}

void abi_BackAwayUntilFar0A9E(void)
{
    BackAwayUntilFar0A9E();
}

/*
 * @routine 0A9E:248 AttackListSpawn2D0A9E
 * @run 71:CA48
 * WHAT IT DOES: Faces the player and plays frame list 2; at a new step on frame $59 (with X and Y high bytes 0)
 * starts a kind $2D in a free slot from $30; when the list ends, or a step begins with a high byte set, restarts
 * the animation in state 3 and takes the next of four waits for OBJ_WORK_D by OBJ_START_ARG (Stage 06's overlay).
 * HOW: FacePlayerPlayList0A9E with Y 2, FindFreeSlotFrom30, StartObjectAtPlace; INC OBJ_WORK_F, whose low two bits
 * plus four times OBJ_START_ARG index the table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AttackListSpawn2D0A9EWaits = 0xCA8F,   /* four values of OBJ_WORK_D per OBJ_START_ARG */
};

void AttackListSpawn2D0A9E(void)
{
    op_ldy(0x02);
    call_routine(0xCA4D, 0xC96E);
    if (!flag_z()) {
        if (!flag_c()) {
            goto at_CA8E;
        }
        op_lda(RD8(OBJ->x_hi[cpu.x]));
        op_ora(RD8(OBJ->y_hi[cpu.x]));
        if (flag_z()) {
            op_lda(RD8(OBJ->frame[cpu.x]));
            op_cmp(0x59);
            if (!flag_z()) {
                goto at_CA8E;
            }
            {
                uint8_t a;
                bool found;
                uint8_t found_slot;

                jsr_push(0xCA63);
                found_slot = FindFreeSlotFrom30(&a, &found);
                find_slot_exit_shim(found_slot, a, found);
                jsr_pop();
            }
            if (!flag_c()) {
                goto at_CA8E;
            }
            op_lda(0x2D);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            return;
        }
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    op_lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
    RMW8(OBJ->work_f[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_and(0x03);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(AttackListSpawn2D0A9EWaits + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
at_CA8E:
    ;
}

void abi_AttackListSpawn2D0A9E(void)
{
    AttackListSpawn2D0A9E();
}

/*
 * @routine 0A9E:297 RunBySubstate0A9E_297
 * @run 71:CA97
 * WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunBySubstate0A9E_297Handlers = 0xCAA9,   /* the handler address for each substate, low and high byte */
};

void RunBySubstate0A9E_297(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunBySubstate0A9E_297Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunBySubstate0A9E_297Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunBySubstate0A9E_297(void)
{
    RunBySubstate0A9E_297(cpu.x);
}

/*
 * @routine 0A9E:2AF JumpAfterList0A9E
 * @run 71:CAAF
 * WHAT IT DOES: Faces the player and plays frame list 4; when it ends, jumps: with OBJ_START_ARG set toward the
 * screen's middle (by bit 7 of its X), else the way of OBJ_WORK_E reversed one time in four; X speed $FE or $02, Y
 * speed $FD00, then the next substate (Stage 06's overlay).
 * HOW: FacePlayerPlayList0A9E with Y 4; a ROL of X low or PlayerDistanceSum's low two bits; zHitSlot the side; a
 * table read, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    JumpAfterList0A9ESpeeds = 0xCAF7,   /* the X speed high byte for each side */
};

void JumpAfterList0A9E(void)
{
    op_ldy(0x04);
    call_routine(0xCAB4, 0xC96E);
    if (flag_z()) {
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (!flag_z()) {
            op_lda(RD8(OBJ->x_lo[cpu.x]));
            cpu.a = f_rol(cpu.a);
            cpu.a = f_rol(cpu.a);
            op_and(0x01);
            op_eor(0x01);
            WR8(ZP->hit_slot, cpu.a);
        } else {
            op_ldy(RD8(OBJ->work_e[cpu.x]));
            WR8(ZP->hit_slot, cpu.y);
            jsr_push(0xCAD0);
            distance_sum_exit_shim(PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0));
            jsr_pop();
            op_and(0x03);
            if (flag_z()) {
                op_lda(RD8(ZP->hit_slot));
                op_eor(0x01);
                WR8(ZP->hit_slot, cpu.a);
            }
        }
        op_ldy(RD8(ZP->hit_slot));
        op_lda(rd8((uint16_t)(JumpAfterList0A9ESpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0xFD);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        WR8(OBJ->vx_lo[cpu.x], 0);
        WR8(OBJ->vy_lo[cpu.x], 0);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
    }
}

void abi_JumpAfterList0A9E(void)
{
    JumpAfterList0A9E();
}

/*
 * @routine 0A9E:304 ListThenState3_0A9E
 * @run 71:CB04
 * WHAT IT DOES: Faces the player and plays frame list 6; when it ends restarts the animation in state 3, substate 0
 * (Stage 06's overlay).
 * HOW: FacePlayerPlayList0A9E with Y 6, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ListThenState3_0A9E(void)
{
    op_ldy(0x06);
    call_routine(0xCB09, 0xC96E);
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_ListThenState3_0A9E(void)
{
    ListThenState3_0A9E();
}

/*
 * @routine 0A9E:684 RunBySubstate0A9E_684
 * @run 71:CE84
 * WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunBySubstate0A9E_684Handlers = 0xCE96,   /* the handler address for each substate, low and high byte */
};

void RunBySubstate0A9E_684(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunBySubstate0A9E_684Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunBySubstate0A9E_684Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunBySubstate0A9E_684(void)
{
    RunBySubstate0A9E_684(cpu.x);
}

/*
 * @routine 0A9F:010 RunByState0A9F_010
 * @run 71:D010
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9F_010Handlers = 0xD022,   /* the handler address for each state, low and high byte */
};

void RunByState0A9F_010(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9F_010Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9F_010Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9F_010(void)
{
    RunByState0A9F_010(cpu.x);
}

/*
 * @routine 0A9F:0F0 ByStateThenFollow0A9F
 * @run 71:D0F0
 * WHAT IT DOES: Runs the handler of the object's state, then takes the Y of the slot in OBJ_WORK_F, removed once
 * that slot is in state 4 (Stage 06's overlay).
 * HOW: RunByState0A9F_0F6, then a tail into FollowParentYOrClear0A9F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ByStateThenFollow0A9F(void)
{
    jsr_push(0xD0F3);
    RunByState0A9F_0F6(cpu.x);
    jsr_pop();
    run_routine(0xD0A0);
}

void abi_ByStateThenFollow0A9F(void)
{
    ByStateThenFollow0A9F();
}

/*
 * @routine 0A9F:0F6 RunByState0A9F_0F6
 * @run 71:D0F6
 * WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0A9F_0F6Handlers = 0xD108,   /* the handler address for each state, low and high byte */
};

void RunByState0A9F_0F6(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0A9F_0F6Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0A9F_0F6Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0A9F_0F6(void)
{
    RunByState0A9F_0F6(cpu.x);
}

/*
 * @routine 0A9F:145 CountThenArm0A9F
 * @run 71:D145
 * WHAT IT DOES: Counts OBJ_WORK_E down; at 0 plays sound $26, sets OBJ_WORK_E $40 and OBJ_HITBOX_ON 2 and steps the
 * state on (Stage 06's overlay).
 * HOW: DEC, SoundCommand, stores, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountThenArm0A9F(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x26);
        {
            uint8_t command;

            jsr_push(0xD14F);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x40);
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_CountThenArm0A9F(void)
{
    CountThenArm0A9F(cpu.x);
}

/*
 * @routine 0A9F:223 CarryChecksThenByState0A9F
 * @run 71:D223
 * WHAT IT DOES: Steps OBJ_WORK_G on, carries the slot in OBJ_WORK_E, picks a new OBJ_WORK_D now and then, drops on
 * a light hit, then runs the handler of its state (Stage 06's overlay).
 * HOW: INC, CarrySlotInWorkE0A9F, RandomWorkDOnZeroG0A9F, LightHitDropThenState4_0A9F, a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    CarryChecksThenByState0A9FHandlers = 0xD241,   /* the handler address for each state, low and high byte */
};

void CarryChecksThenByState0A9F(void)
{
    RMW8(OBJ->work_g[cpu.x], v, f_inc(v));
    call_routine(0xD229, 0xD24D);
    call_routine(0xD22C, 0xD2C0);
    jsr_push(0xD22F);
    LightHitDropThenState4_0A9F();
    jsr_pop();
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(CarryChecksThenByState0A9FHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(CarryChecksThenByState0A9FHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_CarryChecksThenByState0A9F(void)
{
    CarryChecksThenByState0A9F();
}

/*
 * @routine 0A9F:2D0 LightHitDropThenState4_0A9F
 * @run 71:D2D0
 * WHAT IT DOES: Hit lightly (OBJ_HIT_TAKEN 1 to 4) in a state below 4: starts its drop, restarts the animation and
 * goes to state 4 with OBJ_WORK_F one of four values picked at random (Stage 06's overlay).
 * HOW: Compares, SpawnDropByField93_0A9F, PlayerDistanceSum's low two bits index a table, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LightHitDropThenState4_0A9FValues = 0xD2FB,   /* four values of OBJ_WORK_F */
};

void LightHitDropThenState4_0A9F(void)
{
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            op_lda(RD8(OBJ->state[cpu.x]));
            op_cmp(0x04);
            if (!flag_c()) {
                call_routine(0xD2E3, 0xD2FF);
                WR8(OBJ->anim_step[cpu.x], 0);
                WR8(OBJ->anim_timer[cpu.x], 0);
                jsr_push(0xD2EC);
                distance_sum_exit_shim(PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0));
                jsr_pop();
                op_and(0x03);
                op_tay();
                op_lda(rd8((uint16_t)(LightHitDropThenState4_0A9FValues + cpu.y)));
                WR8(OBJ->work_f[cpu.x], cpu.a);
                op_lda(0x04);
                WR8(OBJ->state[cpu.x], cpu.a);
            }
        }
    }
}

void abi_LightHitDropThenState4_0A9F(void)
{
    LightHitDropThenState4_0A9F();
}

/*
 * @routine 0A9F:331 LoadRecordAndEvent0A9F
 * @run 71:D331
 * WHAT IT DOES: Loads the object's record and records its event (Stage 06's overlay).
 * HOW: LoadObjectRecord, then a tail into RecordObjectEvent.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordAndEvent0A9F(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD334);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
    }
}

void abi_LoadRecordAndEvent0A9F(void)
{
    LoadRecordAndEvent0A9F();
}

/*
 * @routine 0A9F:49A RunBySubstate0A9F_49A
 * @run 71:D49A
 * WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunBySubstate0A9F_49AHandlers = 0xD4AC,   /* the handler address for each substate, low and high byte */
};

void RunBySubstate0A9F_49A(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunBySubstate0A9F_49AHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunBySubstate0A9F_49AHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunBySubstate0A9F_49A(void)
{
    RunBySubstate0A9F_49A(cpu.x);
}

/*
 * @routine 0A9F:54C ChecksThenByState0A9F
 * @run 71:D54C
 * WHAT IT DOES: Steps OBJ_WORK_I on, faces the player every 16 frames, staggers or goes to state 8 when hit, sets
 * the hit box by frame, then runs the handler of its state (Stage 06's overlay).
 * HOW: INC, FacePlayerEvery16_0A9F, StaggerWhenHit0A9F, State8WhenHit0A9F, HitBoxByFrame0A9F_5F4, a jump table by
 * the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ChecksThenByState0A9FHandlers = 0xD56D,   /* the handler address for each state, low and high byte */
};

void ChecksThenByState0A9F(void)
{
    RMW8(OBJ->work_i[cpu.x], v, f_inc(v));
    call_routine(0xD552, 0xD57F);
    call_routine(0xD555, 0xD5A7);
    call_routine(0xD558, 0xD5D2);
    call_routine(0xD55B, 0xD5F4);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ChecksThenByState0A9FHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ChecksThenByState0A9FHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ChecksThenByState0A9F(void)
{
    ChecksThenByState0A9F();
}

/*
 * @routine 0A9F:693 AdvanceUntilClose0A9F
 * @run 71:D693
 * WHAT IT DOES: Moves the way it faces, bouncing at walls; closer to the player than $30 pixels it goes to state 5,
 * otherwise it counts toward state 6 (Stage 06's overlay).
 * HOW: BounceByListStep0A9F, OBJ_WORK_F from the facing, DistanceXToPlayer, a store or $D6D9.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AdvanceUntilClose0A9F(void)
{
    call_routine(0xD696, 0xD6AC);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    jsr_push(0xD69F);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x30);
    if (flag_c()) {
        CountToState6_0A9F(cpu.x);
        return;
    }
    op_lda(0x05);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_AdvanceUntilClose0A9F(void)
{
    AdvanceUntilClose0A9F();
}

/*
 * @routine 0A9F:6D9 CountToState6_0A9F
 * @run 71:D6D9
 * WHAT IT DOES: Counts OBJ_WORK_E down; at 0 goes to state 6, substate 0 (Stage 06's overlay).
 * HOW: DEC or two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountToState6_0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_e[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_e[slot], v, f_dec(v));
        return;
    }
    op_lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_CountToState6_0A9F(void)
{
    CountToState6_0A9F(cpu.x);
}

/*
 * @routine 0A9F:6EB BackOffUntilFar0A9F
 * @run 71:D6EB
 * WHAT IT DOES: Moves against the way it faces, bouncing at walls; $50 pixels or more from the player it goes to
 * state 4, otherwise it counts toward state 6 (Stage 06's overlay).
 * HOW: BounceByListStep0A9F, OBJ_WORK_F the facing reversed, DistanceXToPlayer, a store or $D6D9.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackOffUntilFar0A9F(void)
{
    call_routine(0xD6EE, 0xD6AC);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_eor(0x01);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    jsr_push(0xD6F9);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x50);
    if (!flag_c()) {
        CountToState6_0A9F(cpu.x);
        return;
    }
    op_lda(0x04);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_BackOffUntilFar0A9F(void)
{
    BackOffUntilFar0A9F();
}

/*
 * @routine 0A9F:706 RunBySubstate0A9F_706
 * @run 71:D706
 * WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunBySubstate0A9F_706Handlers = 0xD718,   /* the handler address for each substate, low and high byte */
};

void RunBySubstate0A9F_706(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunBySubstate0A9F_706Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunBySubstate0A9F_706Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunBySubstate0A9F_706(void)
{
    RunBySubstate0A9F_706(cpu.x);
}

/*
 * @routine 0A9F:7BA LungeOnFramesB7_0A9F
 * @run 71:D7BA
 * WHAT IT DOES: Plays the frame list at $D7E1; on frames $B7 to $BA it takes the X speed of its facing ($0C or $F4)
 * and moves, bouncing at walls; when the list ends it starts the next round in state 4 (Stage 06's overlay).
 * HOW: StepObjectFrames, compares of the frame, a table read by the facing, BounceAtWallsHeightF0A9F,
 * NextRoundToState4_0A9F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LungeOnFramesB7_0A9FSpeeds = 0xD7FC,   /* the X speed high byte for each facing */
};

void LungeOnFramesB7_0A9F(void)
{
    op_lda(0xE1);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD7);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD7C5);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        run_routine(0xD78D);
        return;
    }
    if (flag_c()) {
        op_lda(RD8(OBJ->frame[cpu.x]));
        op_cmp(0xB7);
        if (flag_c()) {
            op_cmp(0xBB);
            if (!flag_c()) {
                op_ldy(RD8(OBJ->facing_left[cpu.x]));
                op_lda(rd8((uint16_t)(LungeOnFramesB7_0A9FSpeeds + cpu.y)));
                WR8(OBJ->vx_hi[cpu.x], cpu.a);
                call_routine(0xD7E0, 0xD6C2);
            }
        }
    }
}

void abi_LungeOnFramesB7_0A9F(void)
{
    LungeOnFramesB7_0A9F();
}

/*
 * @routine 0AA0:298 RunByState0AA0
 * @run 71:DA98
 * WHAT IT DOES: Runs the handler of the object's state; states 0 and 2 are LoadObjectRecord itself (Stage 06's
 * overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunByState0AA0Handlers = 0xDAAA,   /* the handler address for each state, low and high byte */
};

void RunByState0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunByState0AA0Handlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunByState0AA0Handlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunByState0AA0(void)
{
    RunByState0AA0(cpu.x);
}
