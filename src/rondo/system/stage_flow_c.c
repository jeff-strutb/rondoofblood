/*
 * Stage 00's program: the entrances, the ways out of a cleared stage and the death's first steps.
 */
#include "stage_obj_ops.h"
#include "stage_flow_c.h"
#include "calls_b.h"

void abi_VecPlayerAndCameraFrame(void);   /* system/vectors.c */

/* jmp (table,X). */
static void jump_table_x(uint16_t table)
{
    uint8_t lo = rd8((uint16_t)(table + cpu.x));
    uint8_t hi = rd8((uint16_t)(table + cpu.x + 1));

    run_routine((uint16_t)(lo | (hi << 8)));
}

static void adpcm_load(void)
{
    AdpcmLoadRun run = AdpcmLoad(cpu.a);

    adpcm_load_exit_shim(&run);
}

/* (zField8C - 1) AND 1: the walk's route, into Y. */
static void route_to_y(void)
{
    lda(RD8(ZP->field8c));
    dec_a();
    and_a(0x01);
    tay();
}

/*
 * @routine 024D:2E5 StageEntranceStep
 * @run 6B:82E5
 * WHAT IT DOES: A frame of the player's entrance: the music started (not in stage $10; $FE for stage $87, else $FF), the entrance run, and once play has begun the stage's objects set up.
 * HOW: Sound commands, then StageEntranceByKind; in game sub-mode 1 the objects are cleared and SetUpStageObjects follows.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageEntranceStep(void)
{
    lda(RD8(ZP->stage));
    cmp(cpu.a, 0x10);
    if (NE) {
        cmp(cpu.a, 0x87);
        if (EQ) {
            lda(0xFE);
            JSR(0x82FB, k_sound());
        } else {
            lda(0xFF);
            JSR(0x82F4, k_sound());
        }
    }
    call_routine(0x82FE, 0x830E);
    lda(RD8(ZP->game_sub_mode));
    cmp(cpu.a, 0x01);
    if (NE) {
        return;
    }
    call_routine(0x8308, 0x80A0);
    call_routine(0x830B, 0xA015);
    run_routine(0x8A5F);
}

void abi_StageEntranceStep(void)
{
    StageEntranceStep();
}

/*
 * @routine 024D:30E StageEntranceByKind
 * @run 6B:830E
 * WHAT IT DOES: Goes to the entrance zField87 names.
 * HOW: A jump through the table at $8315.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageEntranceByKind(void)
{
    lda(RD8(ZP->field87));
    asl_a();
    tax();
    jump_table_x(0x8315);
}

void abi_StageEntranceByKind(void)
{
    StageEntranceByKind();
}

/*
 * @routine 024D:400 StageClearedByKind
 * @run 6B:8400
 * WHAT IT DOES: A frame of the cleared stage: the frame counted, then the way out zClearKind names.
 * HOW: INC zWaveCounter, a jump through the table at $8409.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedByKind(void)
{
    abi_nz(INC8(ZP->wave_counter));
    lda(RD8(ZP->clear_kind));
    asl_a();
    tax();
    jump_table_x(0x8409);
}

void abi_StageClearedByKind(void)
{
    StageClearedByKind();
}

/*
 * @routine 024D:458 StageClearedWalkByStep
 * @run 6B:8458
 * WHAT IT DOES: The walk-out way: the scroll and stage frame run, then the step zField85 names.
 * HOW: Two calls, a jump through the table at $8465.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWalkByStep(void)
{
    call_routine(0x845B, 0x8A36);
    call_routine(0x845E, 0x8B6D);
    lda(RD8(ZP->field85));
    asl_a();
    tax();
    jump_table_x(0x8465);
}

void abi_StageClearedWalkByStep(void)
{
    StageClearedWalkByStep();
}

/*
 * @routine 024D:46F StageClearedWaitEmpty
 * @run 6B:846F
 * WHAT IT DOES: Runs the player and objects until no slot from $50 down to 1 holds a kind below $80, then clears the attack marks and goes to the next step.
 * HOW: A walk of OBJ_KIND.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWaitEmpty(void)
{
    call_routine(0x8472, 0x8448);
    call_routine(0x8475, 0x80B1);
    call_routine(0x8478, 0x80BF);
    call_routine(0x847B, 0x8A4F);
    ldx(0x50);
    do {
        lda(RD8(OBJ->kind[cpu.x]));
        if (PL && NE) {
            return;
        }
        dex();
    } while (NE);
    wr8(HitAttackFlags, 0);
    abi_nz(INC8(ZP->field85));
}

void abi_StageClearedWaitEmpty(void)
{
    StageClearedWaitEmpty();
}

/*
 * @routine 024D:4D1 StageClearedWalkToStop
 * @run 6B:84D1
 * WHAT IT DOES: Walks the player back to the route's mark; there, clears the walk, sets zField8A to 2, starts Maria and goes on (StageShowAndStep).
 * HOW: The route's step taken from the player's X, compared with WalkStopXByRoute.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWalkToStop(void)
{
    call_routine(0x84D4, 0x8A4F);
    lda(0xE0);
    WR8(OBJ->work_e[0], cpu.a);
    route_to_y();
    lda(rd8((uint16_t)(WalkStopXByRoute + 4 + cpu.y)));
    WR8(OBJ->work_f[0], cpu.a);
    call_routine(0x84E8, 0x80AB);
    route_to_y();
    lda(RD8(OBJ->x_lo[0]));
    sec();
    sbc_a(rd8((uint16_t)(WalkPlayerStepByRoute + cpu.y)));
    WR8(OBJ->x_lo[0], cpu.a);
    cmp(cpu.a, rd8((uint16_t)(WalkStopXByRoute + 2 + cpu.y)));
    if (NE) {
        return;
    }
    WR8(ZP->field8c, (uint8_t)(RD8(ZP->field8c) & 0x7F));
    WR8(OBJ->work_e[0], 0);
    WR8(OBJ->work_f[0], 0);
    lda(0x02);
    WR8(ZP->field8a, cpu.a);
    run_routine(0x850A);
}

void abi_StageClearedWalkToStop(void)
{
    StageClearedWalkToStop();
}

/*
 * @routine 024D:513 StageClearedNextRoom
 * @run 6B:8513
 * WHAT IT DOES: Scrolls the next room in with the player carried along; at the screen's edge changes section and music, loads the section's lists, runs and entry, sets up the scroll again where the section needs it, and returns to play.
 * HOW: The route's scroll step each frame; on arrival StageSectionNext, the stage's loads, and game sub-mode 1.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedNextRoom(void)
{
    route_to_y();
    lda(rd8((uint16_t)(WalkScrollStepByRoute + cpu.y)));
    WR8(SCROLLE->step_x_hi[0], cpu.a);
    WR8(SCROLLE->step_x_lo[0], 0);
    lda(RD8(OBJ->x_lo[0]));
    clc();
    adc_a(rd8((uint16_t)(WalkPlayerStepByRoute + cpu.y)));
    WR8(OBJ->x_lo[0], cpu.a);
    call_routine(0x852F, 0x80BF);
    call_routine(0x8532, 0x8A4F);
    lda(RD8(SCROLL->x_lo[0]));
    if (NE) {
        return;
    }
    lda(RD8(ZP->field8c));
    cmp(cpu.a, 0x07);
    if (CC) {
        call_routine(0x8541, 0x866A);
        WR8(ZP->field86, cpu.a);
    } else {
        abi_nz(INC8(ZP->field83));
        lda(RD8(ZP->stage));
        cmp(cpu.a, 0x83);
        if (NE) {
            cmp(cpu.a, 0x89);
        }
        if (NE) {
            bool play = false;
            bool load = false;

            cmp(cpu.a, 0x0C);
            if (EQ) {
                load = true;
            } else {
                cmp(cpu.a, 0x8C);
                if (EQ) {
                    play = true;
                } else {
                    cmp(cpu.a, 0x8E);
                    if (NE) {
                        WR8(ZP->field8e, (uint8_t)(RD8(ZP->field8e) | 0x10));
                        lda(0xFE);
                        JSR(0x8564, k_sound());
                    }
                }
            }
            if (load) {
                lda(RD8(ZP->field92));
                cmp(cpu.a, 0x04);
                if (NE) {
                    lda(0x1C);
                    clc();
                    adc_a(RD8(ZP->character));
                    JSR(0x8574, adpcm_load());
                }
                play = true;
            }
            if (play) {
                lda(0x5B);
                WR8(ZP->fade_sound, cpu.a);
                JSR(0x857B, k_sound());
            }
        }
    }
    WR8(ZP->field8e, (uint8_t)(RD8(ZP->field8e) | 0x02));
    lda(RD8(ZP->field8c));
    cmp(cpu.a, 0x07);
    if (CC) {
        call_routine(0x8586, 0x88B1);
    }
    call_routine(0x8589, 0x8882);
    tai(KernelZeros, ScrollXFrac, 0x30);
    tai(KernelZeros, ScrollColumnBlocks, 0x88);
    lda(RD8(ZP->field8e));
    pha();
    call_routine(0x859D, 0x8952);
    pla();
    WR8(ZP->field8e, cpu.a);
    lda(RD8(ZP->raster_effect));
    cmp(cpu.a, 0x0E);
    if (NE) {
        cmp(cpu.a, 0x0F);
    }
    if (EQ) {
        call_routine(0x85AD, 0x88FB);
    } else {
        lda(RD8(SCROLLLIM->camera_mode));
        cmp(cpu.a, 0x0D);
        if (NE) {
            cmp(cpu.a, 0x11);
        }
        if (EQ) {
            call_routine(0x85BD, 0x80B7);
            call_routine(0x85C0, 0xA94C);
            call_routine(0x85C3, 0x8952);
        }
    }
    WR8(ZP->field8e, (uint8_t)(RD8(ZP->field8e) & 0xFD));
    lda(RD8(ZP->field8c));
    cmp(cpu.a, 0x09);
    if (CC) {
        cmp(cpu.a, 0x07);
        if (CC) {
            call_routine(0x85D2, 0x8A5F);
        }
    } else {
        call_routine(0x85D2, 0x8A5F);
    }
    WR8(ZP->field8c, 0);
    WR8(ZP->paused, 0);
    lda(0x01);
    WR8(ZP->game_sub_mode, cpu.a);
}

void abi_StageClearedNextRoom(void)
{
    StageClearedNextRoom();
}

/*
 * @routine 024D:5DB StageClearedFinishByStep
 * @run 6B:85DB
 * WHAT IT DOES: The third way out: the stage frame and fade timer run, then the step zField85 names.
 * HOW: A jump through the table at $85E8.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedFinishByStep(void)
{
    call_routine(0x85DE, 0x8B6D);
    call_routine(0x85E1, 0x8178);
    lda(RD8(ZP->field85));
    asl_a();
    tax();
    jump_table_x(0x85E8);
}

void abi_StageClearedFinishByStep(void)
{
    StageClearedFinishByStep();
}

/*
 * @routine 024D:5EE StageClearedScrollToEnd
 * @run 6B:85EE
 * WHAT IT DOES: Scrolls on a pixel a frame with the player walking against it until screen 7; there stops the scroll, plays sound $8C and goes to the next step.
 * HOW: The scroll's X step set, the player's X counted down.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedScrollToEnd(void)
{
    call_routine(0x85F1, 0x8448);
    call_routine(0x85F4, 0x80B1);
    lda(0x01);
    WR8(SCROLLE->step_x_hi[0], cpu.a);
    WR8(SCROLLE->step_x_lo[0], 0);
    abi_nz(DEC8(OBJ->x_lo[0]));
    call_routine(0x8602, 0x80BF);
    call_routine(0x8605, 0x8A4F);
    lda(RD8(SCROLL->x_hi[0]));
    cmp(cpu.a, 0x07);
    if (NE) {
        return;
    }
    call_routine(0x8610, 0x80C5);
    WR8(SCROLLE->step_x_hi[0], 0);
    WR8(SCROLLE->step_x_lo[2], 0);
    lda(0x8C);
    JSR(0x861B, k_sound());
    abi_nz(INC8(ZP->field85));
}

void abi_StageClearedScrollToEnd(void)
{
    StageClearedScrollToEnd();
}

/*
 * @routine 024D:61E StageClearedWaitObject22
 * @run 6B:861E
 * WHAT IT DOES: Runs the player and objects until object $22 reaches state 4 with its OBJ_FALL_LO at zero, then starts the walk on screen.
 * HOW: Two tests of slot $22.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWaitObject22(void)
{
    call_routine(0x8621, 0x80B1);
    call_routine(0x8624, 0x8A4F);
    lda(RD8(OBJ->state[0x22]));
    cmp(cpu.a, 0x04);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->work_h[0x22]));
    if (NE) {
        return;
    }
    lda(0x20);
    WR8(OBJ->work_f[0], cpu.a);
    abi_nz(INC8(ZP->field85));
}

void abi_StageClearedWaitObject22(void)
{
    StageClearedWaitObject22();
}

/*
 * @routine 024D:638 StageClearedWalkOnScreen
 * @run 6B:8638
 * WHAT IT DOES: Walks the player on to X $F0, then changes section ($8654).
 * HOW: The player's frame with the walk's mark set.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWalkOnScreen(void)
{
    lda(0xB8);
    WR8(OBJ->work_e[0], cpu.a);
    call_routine(0x8640, 0x80AB);
    JSR(0x8643, abi_VecPlayerAndCameraFrame());
    call_routine(0x8646, 0xA009);
    call_routine(0x8649, 0x80BF);
    call_routine(0x864C, 0x8A4F);
    lda(RD8(OBJ->x_lo[0]));
    cmp(cpu.a, 0xF0);
    if (EQ) {
        run_routine(0x8654);
    }
}

void abi_StageClearedWalkOnScreen(void)
{
    StageClearedWalkOnScreen();
}

/*
 * @routine 024D:654 StageNextSectionAfterWalk
 * @run 6B:8654
 * WHAT IT DOES: After a walk: a 2 frame display delay, the walk cleared, the next section, back to play.
 * HOW: Stores and StageSectionNext.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageNextSectionAfterWalk(void)
{
    lda(0x02);
    WR8(ZP->display_delay, cpu.a);
    WR8(OBJ->work_e[0], 0);
    wr8(HitAttackFlags, 0);
    WR8(OBJ->work_f[0], 0);
    call_routine(0x8664, 0x866A);
    WR8(ZP->clear_kind, 0);
    WR8(OBJ->frame[0], 0);
}

void abi_StageNextSectionAfterWalk(void)
{
    StageNextSectionAfterWalk();
}

/*
 * @routine 024D:675 StageClearedDoorByStep
 * @run 6B:8675
 * WHAT IT DOES: The boss door way: the scroll and stage frame run, then the step zField85 names.
 * HOW: A jump through the table at $8682.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedDoorByStep(void)
{
    call_routine(0x8678, 0x8A36);
    call_routine(0x867B, 0x8B6D);
    lda(RD8(ZP->field85));
    asl_a();
    tax();
    jump_table_x(0x8682);
}

void abi_StageClearedDoorByStep(void)
{
    StageClearedDoorByStep();
}

/*
 * @routine 024D:69D StageClearedWalkOut
 * @run 6B:869D
 * WHAT IT DOES: Walks the player out of the stage to the route's mark; there ends the stage: from zField8C $0F up chooses the next section and entrance and goes to the death step's reload ($823D); at $0B or $0C starts a list channel and changes section; else ends the mode.
 * HOW: The route's step added to the player's 16-bit X.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedWalkOut(void)
{
    call_routine(0x86A0, 0x8A4F);
    route_to_y();
    lda(rd8((uint16_t)(WalkStopXByRoute + 4 + cpu.y)));
    WR8(OBJ->work_f[0], cpu.a);
    WR8(OBJ->work_e[0], cpu.a);
    call_routine(0x86B2, 0x80AB);
    route_to_y();
    lda(rd8((uint16_t)(WalkScrollStepByRoute + cpu.y)));
    clc();
    adc_a(RD8(OBJ->x_lo[0]));
    WR8(OBJ->x_lo[0], cpu.a);
    if (EQ) {
        lda(rd8((uint16_t)(WalkScrollStepByRoute + cpu.y)));
        clc();
        adc_a(RD8(OBJ->x_hi[0]));
        WR8(OBJ->x_hi[0], cpu.a);
    }
    route_to_y();
    lda(RD8(OBJ->x_lo[0]));
    cmp(cpu.a, rd8((uint16_t)(WalkStopXByRoute + 6 + cpu.y)));
    if (NE) {
        return;
    }
    lda(RD8(ZP->field8c));
    cmp(cpu.a, 0x0F);
    if (CC) {
        cmp(cpu.a, 0x0B);
        if (NE) {
            cmp(cpu.a, 0x0C);
        }
        if (NE) {
            abi_nz(INC8(ZP->mode_done));
            return;
        }
        lda(0x66);
        ldx(0x05);
        JSR(0x8707, k_list2());
        run_routine(0x8654);
        return;
    }
    dec_a();
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    WR8(ZP->field83, cpu.a);
    lda(RD8(ZP->field8c));
    dec_a();
    and_a(0x01);
    ora_a(0x02);
    WR8(ZP->field87, cpu.a);
    run_routine(0x823D);
}

void abi_StageClearedWalkOut(void)
{
    StageClearedWalkOut();
}

/*
 * @routine 024D:70A StageClearedLeaveScene
 * @run 6B:870A
 * WHAT IT DOES: Fades the music, clears the objects; with zField8C's low bits zero changes section, else leaves by route (StageLeaveByRoute).
 * HOW: Sound $FF, MariaVecClearObjectsBut, a test of zField8C.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageClearedLeaveScene(void)
{
    lda(0xFF);
    JSR(0x870F, k_sound());
    call_routine(0x8712, 0x80A0);
    call_routine(0x8715, 0xA015);
    lda(RD8(ZP->field8c));
    and_a(0x7F);
    if (NE) {
        run_routine(0x8720);
        return;
    }
    WR8(ZP->field8c, 0);
    run_routine(0x8654);
}

void abi_StageClearedLeaveScene(void)
{
    StageClearedLeaveScene();
}

/*
 * @routine 024D:7A1 StageDeathByStep
 * @run 6B:87A1
 * WHAT IT DOES: A frame of the death: but for step 3 the camera and objects run slowed (TimeSlowMode 3) with their hit tests; then the step zField80 names.
 * HOW: A jump through the table at $87C7.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageDeathByStep(void)
{
    abi_nz(INC8(ZP->wave_counter));
    call_routine(0x87A6, 0x8B6D);
    lda(RD8(ZP->field80));
    cmp(cpu.a, 0x03);
    if (NE) {
        lda(0x03);
        WR8(PVARS->time_slow_mode, cpu.a);
        call_routine(0x87B4, 0x80BF);
        call_routine(0x87B7, 0x8A4F);
        call_routine(0x87BA, 0x93F8);
        WR8(PVARS->time_slow_mode, 0);
    }
    call_routine(0x87C0, 0x80A0);
    lda(RD8(ZP->field80));
    asl_a();
    tax();
    jump_table_x(0x87C7);
}

void abi_StageDeathByStep(void)
{
    StageDeathByStep();
}

/*
 * @routine 024D:7D5 StageDeathLeapStart
 * @run 6B:87D5
 * WHAT IT DOES: Maria's death leap and its sound, then the next step ($87DB).
 * HOW: Two calls into her overlay.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageDeathLeapStart(void)
{
    call_routine(0x87D8, 0xA00C);
    call_routine(0x87DB, 0xA012);
    run_routine(0x87DB);
}

void abi_StageDeathLeapStart(void)
{
    StageDeathLeapStart();
}

/*
 * @routine 024D:7DB StageNextDeathStep
 * @run 6B:87DB
 * WHAT IT DOES: Goes to the death's next step.
 * HOW: INC zField80.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageNextDeathStep(void)
{
    abi_nz(INC8(ZP->field80));
}

void abi_StageNextDeathStep(void)
{
    StageNextDeathStep();
}

/*
 * @routine 024D:7DE StageDeathFall
 * @run 6B:87DE
 * WHAT IT DOES: Runs Maria's death objects, with the animation's sound when the frame changes; with zField89 set, the next step.
 * HOW: The frame kept across the run and compared.
 * WAITS: no.
 * Entry: nothing. Exit: as the last instruction, call or jump left them.
 */
void StageDeathFall(void)
{
    lda(RD8(OBJ->frame[0]));
    pha();
    call_routine(0x87E5, 0xA00F);
    pla();
    cmp(cpu.a, RD8(OBJ->frame[0]));
    if (NE) {
        call_routine(0x87EE, 0xA012);
    }
    lda(RD8(ZP->field89));
    if (NE) {
        run_routine(0x87DB);
    }
}

void abi_StageDeathFall(void)
{
    StageDeathFall();
}
