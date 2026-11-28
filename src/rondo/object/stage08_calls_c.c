/*
 * Calling routines of Stage 08's overlays: object starts that spawn their parts and chains, state and substate jump
 * tables, swings and turns by angle, frame lists, landings, and the pieces the routines jump or fall into.
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
#include "stage08_calls_c.h"

/*
 * @routine 0D2D:335 FrameAndTimerByArgSound0D2D
 * @run 81:C335
 * WHAT IT DOES: Sets the object's frame and OBJ_WORK_E from two tables by OBJ_START_ARG, plays sound $3A and steps
 * the substate on (Stage 08's overlay).
 * HOW: Two table reads by OBJ_START_ARG, the sound call, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FrameAndTimerByArgSound0D2DFrames = 0xC194,   /* the frame for each OBJ_START_ARG */
    FrameAndTimerByArgSound0D2DWorkE = 0xC1A8,   /* OBJ_WORK_E for each OBJ_START_ARG */
};

void FrameAndTimerByArgSound0D2D(uint8_t slot)
{
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(rd8((uint16_t)(FrameAndTimerByArgSound0D2DFrames + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(rd8((uint16_t)(FrameAndTimerByArgSound0D2DWorkE + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(0x3A);
    {
        uint8_t command;

        jsr_push(0xC349);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_FrameAndTimerByArgSound0D2D(void)
{
    FrameAndTimerByArgSound0D2D(cpu.x);
}

/*
 * @routine 0D2D:34D RiseThenBecomeKind0D2D
 * @run 81:C34D
 * WHAT IT DOES: Moves the object up a pixel and counts OBJ_WORK_E down; at zero records its event and turns it into
 * the kind a table gives by OBJ_START_ARG, in state 0 with no hit box offset and no Y speed (Stage 08's overlay).
 * HOW: DEC of the Y low byte and of the count, then RecordObjectEvent, a table read and stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RiseThenBecomeKind0D2DKinds = 0xC1D0,   /* the new kind for each OBJ_START_ARG */
};

void RiseThenBecomeKind0D2D(uint8_t slot)
{
    RMW8(OBJ->y_lo[slot], v, f_dec(v));
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xC358);
            if (!RecordObjectEvent(slot, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(rd8((uint16_t)(RiseThenBecomeKind0D2DKinds + cpu.y)));
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        WR8(OBJ->hitbox_dy[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    }
}

void abi_RiseThenBecomeKind0D2D(void)
{
    RiseThenBecomeKind0D2D(cpu.x);
}

/*
 * @routine 0D2D:3E7 StartSevenLinks0D2D
 * @run 81:C3E7
 * WHAT IT DOES: Starts the object with hit box $82, OBJ_WORK_D $40 and OBJ_WORK_G 7, and seven objects of kind $C4
 * at its place in the seven slots below it, each with its OBJ_START_ARG and its index 0 to 6 in OBJ_WORK_G;
 * OBJ_WORK_H comes from a table (Stage 08's overlay).
 * HOW: LoadObjectRecord, stores, a loop of ClearObjectInY and StartObjectAtPlace, then a table read by eight times
 * OBJ_START_ARG plus OBJ_WORK_G.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StartSevenLinks0D2DCounts = 0xC430,   /* OBJ_WORK_H, eight per OBJ_START_ARG, one per link index */
};

void StartSevenLinks0D2D(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC3EA);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x82);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    op_lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(0x07);
    WR8(OBJ->work_g[slot], cpu.a);
    op_txa();
    op_sec();
    op_sbc(0x07);
    op_tay();
    op_lda(0x07);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    do {
        jsr_push(0xC407);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0xC4);
        jsr_push(0xC40C);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        op_lda(RD8(ZP->temp01));
        WR8(OBJ->work_g[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_inc(v));
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_g[slot]));
    op_tay();
    op_lda(rd8((uint16_t)(StartSevenLinks0D2DCounts + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_StartSevenLinks0D2D(void)
{
    StartSevenLinks0D2D(cpu.x);
}

/*
 * @routine 0D2D:448 FollowLinkAtAngle0D2D
 * @run 81:C448
 * WHAT IT DOES: With OBJ_WORK_G not 0, puts the object at the place of the slot below it and moves it at angle
 * OBJ_WORK_D by a speed a table gives by OBJ_WORK_G; counts OBJ_WORK_H down and steps the state on at zero (Stage
 * 08's overlay).
 * HOW: CopyObjectPlaceBack from X less 1, a table read, SetVelocityFromAngle and ApplyVelocity, then DEC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FollowLinkAtAngle0D2DSpeeds = 0xC46F,   /* the speed's high byte for each link index */
};

void FollowLinkAtAngle0D2D(uint8_t slot)
{
    op_lda(RD8(OBJ->work_g[slot]));
    if (!flag_z()) {
        op_txa();
        op_tay();
        op_dey();
        jsr_push(0xC453);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
        jsr_pop();
        WR8(ZP->temp00, 0);
        op_ldy(RD8(OBJ->work_g[slot]));
        op_lda(rd8((uint16_t)(FollowLinkAtAngle0D2DSpeeds + cpu.y)));
        WR8(ZP->temp01, cpu.a);
        op_lda(RD8(OBJ->work_d[slot]));
        jsr_push(0xC463);
        scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
        jsr_pop();
        jsr_push(0xC466);
        axis_move_exit_shim(ApplyVelocity(slot));
        jsr_pop();
    }
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_FollowLinkAtAngle0D2D(void)
{
    FollowLinkAtAngle0D2D(cpu.x);
}

/*
 * @routine 0D2D:477 SwingLinkByCosine0D2D
 * @run 81:C477
 * WHAT IT DOES: Clears the object unless the slot below holds kind $C4; else puts it at that slot's place, moves it
 * at angle OBJ_WORK_D with speed $1000, and swings the 16-bit angle OBJ_WORK_D-E by a cosine of OBJ_WORK_F, which
 * steps by 2; when that wraps to 0 the angle goes back to $40: that part is MoveAndSwingByCosine0D2D, which it runs
 * into (Stage 08's overlay).
 * HOW: A kind compare, CopyObjectPlaceBack, SetVelocityFromAngle, then MoveAndSwingByCosine0D2D's ApplyVelocity,
 * ScaleByCosine of a word from a table by OBJ_START_ARG and a 16-bit add.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwingLinkByCosine0D2D(uint8_t slot)
{
    op_lda(rd8((uint16_t)(COL(OBJ->kind) - 1 + slot)));
    op_cmp(0xC4);
    if (!flag_z()) {
        ClearObject(slot);
        return;
    }
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xC487);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    WR8(ZP->temp00, 0);
    op_lda(0x10);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xC493);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    /* falls into $C493 */
    run_routine(0xC493);
}

void abi_SwingLinkByCosine0D2D(void)
{
    SwingLinkByCosine0D2D(cpu.x);
}

/*
 * @routine 0D2D:4EE StartLinkByIndex0D2D
 * @run 81:C4EE
 * WHAT IT DOES: Starts the object with OBJ_WORK_D $40, moves it once by a Y speed a table gives by OBJ_WORK_G, and
 * sets OBJ_WORK_H from the table of StartSevenLinks0D2D (Stage 08's overlay).
 * HOW: LoadObjectRecord, a table read, ApplyVelocityY, a table read by eight times OBJ_START_ARG plus OBJ_WORK_G.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StartLinkByIndex0D2DCounts = 0xC430,   /* OBJ_WORK_H, eight per OBJ_START_ARG, one per link index */
    StartLinkByIndex0D2DSpeeds = 0xC514,   /* the Y speed's high byte for each link index */
};

void StartLinkByIndex0D2D(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC4F1);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    op_ldy(RD8(OBJ->work_g[slot]));
    op_lda(rd8((uint16_t)(StartLinkByIndex0D2DSpeeds + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xC502);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_g[slot]));
    op_tay();
    op_lda(rd8((uint16_t)(StartLinkByIndex0D2DCounts + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_StartLinkByIndex0D2D(void)
{
    StartLinkByIndex0D2D(cpu.x);
}

/*
 * @routine 0D5B:09B StartFromArgBits0D5B
 * @run 70:B09B
 * WHAT IT DOES: Starts the object, adds $80 to its hit box byte, and from OBJ_START_ARG sets OBJ_WORK_E (its low
 * four bits plus $10), OBJ_WORK_D (bits 4 and 5) and OBJ_WORK_F (bits 6 and 7); then makes it kind $BB (Stage 08's
 * overlay).
 * HOW: LoadObjectRecord, an add, then the argument's bits taken by AND and ROL.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFromArgBits0D5B(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB09E);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x80);
    op_clc();
    op_adc(RD8(OBJ->hitbox_on[slot]));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    op_tay();
    op_and(0x0F);
    op_ora(0x10);
    WR8(OBJ->work_e[slot], cpu.a);
    op_tya();
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_tay();
    op_and(0x03);
    WR8(OBJ->work_f[slot], cpu.a);
    op_tya();
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_and(0x03);
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(0xBB);
    WR8(OBJ->kind[slot], cpu.a);
}

void abi_StartFromArgBits0D5B(void)
{
    StartFromArgBits0D5B(cpu.x);
}

/*
 * @routine 0D5B:0CB SetSectionBySide0D5B
 * @run 70:B0CB
 * @stop B0E4
 * WHAT IT DOES: In state 0 starts the object; then, with OBJ_START_ARG 0, sets zField82 (the stage section) from a
 * table by whether the object is left of the player; with OBJ_START_ARG not 0 runs BuildSection2AndClear0D5B (Stage
 * 08's overlay).
 * HOW: LoadObjectRecord, or SideOfPlayer's bit 0 indexing a two-byte table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SetSectionBySide0D5BSections = 0xB0E4,   /* zField82 when right of the player, then when left */
};

void SetSectionBySide0D5B(uint8_t slot)
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
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        BuildSection2AndClear0D5B();
        return;
    }
    jsr_push(0xB0DB);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(SetSectionBySide0D5BSections + cpu.y)));
    WR8(ZP->field82, cpu.a);
}

void abi_SetSectionBySide0D5B(void)
{
    SetSectionBySide0D5B(cpu.x);
}

/*
 * @routine 0D5B:0E6 BuildSection2AndClear0D5B
 * @run 70:B0E6
 * WHAT IT DOES: Sets zField82 to 2, builds that section's stage lists, sets zField86 to 2 and clears the object
 * (Stage 08's overlay).
 * HOW: A store, ExpandStageListsM with X kept, a store and ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BuildSection2AndClear0D5B(void)
{
    op_lda(0x02);
    WR8(ZP->field82, cpu.a);
    op_phx();
    call_routine(0xB0EE, 0x9277);
    op_plx();
    op_lda(0x02);
    WR8(ZP->field86, cpu.a);
    ClearObject(cpu.x);
}

void abi_BuildSection2AndClear0D5B(void)
{
    BuildSection2AndClear0D5B();
}

/*
 * @routine 0D5B:192 ActByDistanceToPlayer0D5B
 * @run 70:B192
 * WHAT IT DOES: An object hit 1 to 4 times goes to SetPhase2; otherwise runs the routine a table picks by its X
 * distance to the player in steps of 16 pixels (Stage 08's overlay).
 * HOW: A compare of OBJ_HIT_TAKEN; DistanceXToPlayer over 16 indexes a byte table that picks from a jump table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByDistanceToPlayer0D5BChoices = 0xB1B8,   /* the jump table's entry for each 16 pixels of distance */
    ActByDistanceToPlayer0D5BJumps = 0xB1C8,   /* the routines' addresses, a word each */
};

void ActByDistanceToPlayer0D5B(void)
{
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            run_routine(0xB200);
            return;
        }
    }
    jsr_push(0xB1A1);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByDistanceToPlayer0D5BChoices + cpu.y)));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByDistanceToPlayer0D5BJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByDistanceToPlayer0D5BJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByDistanceToPlayer0D5B(void)
{
    ActByDistanceToPlayer0D5B();
}

/*
 * @routine 0D5B:269 FallFrameThenLand0D5B
 * @run 70:B269
 * WHAT IT DOES: Gives the object a frame by whether its Y speed is upward, falls and lands with probe height 4, and
 * on landing goes to state 3 (Stage 08's overlay).
 * HOW: The speed's sign bit indexes a two-byte table; StopOnLanding's carry picks the landing.
 * WAITS: no.
 * Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FallFrameThenLand0D5BFrames = 0xB284,   /* the frame moving down, then moving up */
};

void FallFrameThenLand0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->vy_hi[slot]));
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(FallFrameThenLand0D5BFrames + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x04);
    jsr_push(0xB27C);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (!flag_c()) {
        ReturnOnly0D5B();
        return;
    }
    /* falls into $B27E */
    EnterState3_0D5B(slot);
}

void abi_FallFrameThenLand0D5B(void)
{
    FallFrameThenLand0D5B(cpu.x);
}

/*
 * @routine 0D5B:27E EnterState3_0D5B
 * @run 70:B27E
 * WHAT IT DOES: Puts the object in state 3 (Stage 08's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void EnterState3_0D5B(uint8_t slot)
{
    op_lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    /* falls into $B283 */
    ReturnOnly0D5B();
}

void abi_EnterState3_0D5B(void)
{
    EnterState3_0D5B(cpu.x);
}

/*
 * @routine 0D5B:283 ReturnOnly0D5B
 * @run 70:B283
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D5B(void)
{
}

void abi_ReturnOnly0D5B(void)
{
    ReturnOnly0D5B();
}

/*
 * @routine 0D5B:286 Frame5FallOrLand0D5B
 * @run 70:B286
 * WHAT IT DOES: Gives the object frame 5 and falls with probe height 4: on landing it goes to state 3; while in the
 * air $14 comes off its Y speed each frame (Stage 08's overlay).
 * HOW: StopOnLanding, then a 16-bit subtract.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Frame5FallOrLand0D5B(uint8_t slot)
{
    op_lda(0x05);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x04);
    jsr_push(0xB290);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        EnterState3_0D5B(slot);
        return;
    }
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_sec();
    op_sbc(0x14);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(RD8(OBJ->vy_hi[slot]));
    op_sbc(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
}

void abi_Frame5FallOrLand0D5B(void)
{
    Frame5FallOrLand0D5B(cpu.x);
}

/*
 * @routine 0D5B:2A4 SpawnKind21WhileFlag0D5B
 * @run 70:B2A4
 * WHAT IT DOES: In state 0 starts the object. While event flag bit 2 is set it starts an object of kind $21
 * (OBJ_START_ARG 1) at its place in the first free slot from $38 to $3F every $29 frames, shifted by a random X
 * step; once flag bit 4 is set too the object is cleared (Stage 08's overlay).
 * HOW: Flag tests, a countdown in OBJ_WORK_D, a search loop, StartObjectAtPlace, NextRandom as the X speed for one
 * ApplyVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SpawnKind21WhileFlag0D5B(void)
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
    op_lda(RD8(LEVEL->event_flags[0]));
    op_and(0x04);
    if (!flag_z()) {
        op_lda(RD8(LEVEL->event_flags[0]));
        op_and(0x10);
        if (!flag_z()) {
            ClearObject(cpu.x);
            return;
        }
        op_lda(RD8(OBJ->work_d[cpu.x]));
        if (!flag_z()) {
            RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
            return;
        }
        op_lda(0x28);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        op_ldy(0x38);
at_B2CD:
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (!flag_z()) {
            op_iny();
            op_cpy(0x40);
            if (!flag_c()) {
                goto at_B2CD;
            }
            return;
        }
        jsr_push(0xB2DB);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0x21);
        jsr_push(0xB2E0);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        op_sxy();
        jsr_push(0xB2E9);
        random_exit_shim(NextRandom());
        jsr_pop();
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        jsr_push(0xB2EF);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
        WR8(OBJ->vx_hi[cpu.x], 0);
        op_sxy();
    }
}

void abi_SpawnKind21WhileFlag0D5B(void)
{
    SpawnKind21WhileFlag0D5B();
}

/*
 * @routine 0D5B:372 StartNineParts0D5B
 * @run 70:B372
 * WHAT IT DOES: Starts the object with OBJ_WORK_O $80, fills a 128-byte buffer with $70, and starts nine objects of
 * kind $23 at its place in the nine slots after it, each with its index 0 to 8 in OBJ_WORK_K (Stage 08's overlay).
 * HOW: LoadObjectRecord, a fill loop, a loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartNineParts0D5BBuffer = 0xB3A3,   /* 128 bytes of angles, each $70 here */
};

void StartNineParts0D5B(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB375);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x80);
    WR8(OBJ->work_o[slot], cpu.a);
    op_lda(0x70);
    op_cly();
    do {
        wr8((uint16_t)(StartNineParts0D5BBuffer + cpu.y), cpu.a);
        op_iny();
        op_cpy(0x80);
    } while (!flag_c());
    op_txa();
    op_tay();
    op_iny();
    op_lda(0x09);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    do {
        jsr_push(0xB391);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0x23);
        jsr_push(0xB396);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(ZP->temp01));
        WR8(OBJ->work_k[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_inc(v));
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
}

void abi_StartNineParts0D5B(void)
{
    StartNineParts0D5B(cpu.x);
}

/*
 * @routine 0D5B:485 ScaleByRecordWords0D5B
 * @run 70:B485
 * WHAT IT DOES: Takes the four-byte record OBJ_WORK_M picks: scales its second word by the cosine of OBJ_WORK_K and
 * puts its first word in zMultiplier (Stage 08's overlay).
 * HOW: Four times OBJ_WORK_M indexes the records, kept in zHitSlot across ScaleByCosine.
 * WAITS: no.
 * Entry: X the object. Exit: zProduct the scaled word; zMultiplier the record's first word; zHitSlot the record's
 * index.
 */
enum {
    ScaleByRecordWords0D5BRecords = 0xB468,   /* two words per record: the multiplier, then the size to scale */
};

void ScaleByRecordWords0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_m[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    WR8(ZP->hit_slot, cpu.y);
    op_lda(rd8((uint16_t)(ScaleByRecordWords0D5BRecords + 2 + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ScaleByRecordWords0D5BRecords + 3 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_k[slot]));
    jsr_push(0xB49D);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_ldy(RD8(ZP->hit_slot));
    op_lda(rd8((uint16_t)(ScaleByRecordWords0D5BRecords + cpu.y)));
    WR8(ZP->multiplier.lo, cpu.a);
    op_lda(rd8((uint16_t)(ScaleByRecordWords0D5BRecords + 1 + cpu.y)));
    WR8(ZP->multiplier.hi, cpu.a);
}

void abi_ScaleByRecordWords0D5B(void)
{
    ScaleByRecordWords0D5B(cpu.x);
}

/*
 * @routine 0D5B:4AA NextStateByPattern0D5B
 * @run 70:B4AA
 * WHAT IT DOES: Counts OBJ_WORK_O down; at zero sets a new random wait and puts the object in the next state of a
 * 16-step pattern counted by OBJ_WORK_N, substate 0 (Stage 08's overlay).
 * HOW: NextRandom's low two bits index the waits; OBJ_WORK_N's low four bits index a table that indexes the states.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    NextStateByPattern0D5BWaits = 0xB4D6,   /* four waits for OBJ_WORK_O */
    NextStateByPattern0D5BSteps = 0xB4DA,   /* the state table's index for each of 16 steps */
    NextStateByPattern0D5BStates = 0xB4EA,   /* the states */
};

void NextStateByPattern0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_o[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_o[slot], v, f_dec(v));
        return;
    }
    jsr_push(0xB4B6);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(NextStateByPattern0D5BWaits + cpu.y)));
    WR8(OBJ->work_o[slot], cpu.a);
    op_lda(RD8(OBJ->work_n[slot]));
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(NextStateByPattern0D5BSteps + cpu.y)));
    op_tay();
    op_lda(rd8((uint16_t)(NextStateByPattern0D5BStates + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    RMW8(OBJ->work_n[slot], v, f_inc(v));
}

void abi_NextStateByPattern0D5B(void)
{
    NextStateByPattern0D5B(cpu.x);
}

/*
 * @routine 0D5B:4ED MoveChainOfNine0D5B
 * @run 70:B4ED
 * WHAT IT DOES: Moves a chain of nine objects: from the slot eight after the object down to the one after it, each
 * takes the place of the slot above it and moves by its speed; the object itself takes the next slot's place, moves
 * 8 pixels left, then moves at angle OBJ_WORK_D plus $70 with the speed in OBJ_WORK_F and OBJ_WORK_G (Stage 08's
 * overlay).
 * HOW: A loop down the slots of CopyObjectPlaceBack and ApplyVelocity, with the object's slot kept in
 * zRichterObject; then ApplyVelocityX, SetVelocityFromAngle and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveChainOfNine0D5B(void)
{
    WR8(ZP->richter_object, cpu.x);
    op_txa();
    op_clc();
    op_adc(0x08);
    op_tax();
    do {
        op_txa();
        op_tay();
        op_iny();
        jsr_push(0xB4FA);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
        jsr_pop();
        jsr_push(0xB4FD);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_dex();
        op_cpx(RD8(ZP->richter_object));
    } while (!flag_z());
    op_txa();
    op_tay();
    op_iny();
    jsr_push(0xB508);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
    jsr_pop();
    op_lda(0xF8);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    jsr_push(0xB510);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(0x70);
    jsr_push(0xB523);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
    jsr_pop();
    jsr_push(0xB526);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
}

void abi_MoveChainOfNine0D5B(void)
{
    MoveChainOfNine0D5B();
}

/*
 * @routine 0D5B:586 StartKindA4Child0D5B
 * @run 70:B586
 * WHAT IT DOES: Starts an object of kind $A4 at the object's place in a free slot from $30, with the object's slot
 * in its OBJ_WORK_D, and puts the object in state 1 (Stage 08's overlay).
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void StartKindA4Child0D5B(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xB589);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0xA4);
        jsr_push(0xB590);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_txa();
        WR8(OBJ->work_d[cpu.y], cpu.a);
    }
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_StartKindA4Child0D5B(void)
{
    StartKindA4Child0D5B(cpu.x);
}

/*
 * @routine 0D5B:651 TurnTowardWorkH0D5B
 * @run 70:B651
 * WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H less $70 and counts OBJ_WORK_I down, stepping the
 * substate on at zero (Stage 08's overlay).
 * HOW: TurnAngleToward, then DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnTowardWorkH0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_sec();
    op_sbc(0x70);
    WR8(ZP->temp02, cpu.a);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB665);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnTowardWorkH0D5B(void)
{
    TurnTowardWorkH0D5B(cpu.x);
}

/*
 * @routine 0D5B:6EC SpawnKind9COnBeat0D5B
 * @run 70:B6EC
 * WHAT IT DOES: When OBJ_WORK_O's low five bits match one of eight values, starts an object of kind $9C at the
 * object's place in a free slot from $30 and plays sound $15 (Stage 08's overlay).
 * HOW: A search of the table, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SpawnKind9COnBeat0D5BBeats = 0xB70D,   /* the eight counts at which an object starts */
};

void SpawnKind9COnBeat0D5B(uint8_t slot)
{
    op_cly();
at_B6ED:
    op_lda(RD8(OBJ->work_o[slot]));
    op_and(0x1F);
    op_cmp(rd8((uint16_t)(SpawnKind9COnBeat0D5BBeats + cpu.y)));
    if (!flag_z()) {
        op_iny();
        op_cpy(0x08);
        if (!flag_c()) {
            goto at_B6ED;
        }
        return;
    }
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xB700);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x9C);
        jsr_push(0xB707);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x15);
        {
            uint8_t command;

            jsr_push(0xB70C);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
    }
}

void abi_SpawnKind9COnBeat0D5B(void)
{
    SpawnKind9COnBeat0D5B(cpu.x);
}

/*
 * @routine 0D5B:72F StartWithHitBox82_0D5B
 * @run 70:B72F
 * WHAT IT DOES: Starts the object and sets its hit box byte to $82 (Stage 08's overlay).
 * HOW: LoadObjectRecord, one store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithHitBox82_0D5B(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB732);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x82);
    WR8(OBJ->hitbox_on[slot], cpu.a);
}

void abi_StartWithHitBox82_0D5B(void)
{
    StartWithHitBox82_0D5B(cpu.x);
}

/*
 * @routine 0D5B:738 AimPartFromBuffer0D5B
 * @run 70:B738
 * WHAT IT DOES: Unless OBJ_WORK_K is 8, takes the angle OBJ_WORK_D from the buffer StartNineParts0D5B filled at the
 * place a table gives by OBJ_WORK_K, eases the speed (EaseWorkFToTarget0D5B) and sets the speed at that angle
 * (Stage 08's overlay).
 * HOW: Two table reads, a call, SetVelocityFromAngle with OBJ_WORK_G and OBJ_WORK_F as the speed.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AimPartFromBuffer0D5BBuffer = 0xB3A3,   /* the 128-byte angle buffer */
    AimPartFromBuffer0D5BPlaces = 0xB75E,   /* the buffer index for each part 0 to 7 */
};

void AimPartFromBuffer0D5B(void)
{
    op_lda(RD8(OBJ->work_k[cpu.x]));
    op_cmp(0x08);
    if (!flag_z()) {
        op_tay();
        op_lda(rd8((uint16_t)(AimPartFromBuffer0D5BPlaces + cpu.y)));
        op_tay();
        op_lda(rd8((uint16_t)(AimPartFromBuffer0D5BBuffer + cpu.y)));
        WR8(OBJ->work_d[cpu.x], cpu.a);
        call_routine(0xB74D, 0xB527);
        op_lda(RD8(OBJ->work_g[cpu.x]));
        WR8(ZP->temp00, cpu.a);
        op_lda(RD8(OBJ->work_f[cpu.x]));
        WR8(ZP->temp01, cpu.a);
        op_lda(RD8(OBJ->work_d[cpu.x]));
        jsr_push(0xB75D);
        scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
        jsr_pop();
    }
}

void abi_AimPartFromBuffer0D5B(void)
{
    AimPartFromBuffer0D5B();
}

/*
 * @routine 0D5B:76E MoveThenBySubstate0D5B
 * @run 70:B76E
 * WHAT IT DOES: Eases the speed (EaseWorkFToTarget0D5B), sets the speed at angle OBJ_WORK_D, then runs the routine
 * for its substate (Stage 08's overlay).
 * HOW: A call, SetVelocityFromAngle, then a jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MoveThenBySubstate0D5BJumps = 0xB793,   /* the routine for each substate, a word each */
};

void MoveThenBySubstate0D5B(void)
{
    call_routine(0xB771, 0xB527);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    jsr_push(0xB781);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->substate[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(MoveThenBySubstate0D5BJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(MoveThenBySubstate0D5BJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_MoveThenBySubstate0D5B(void)
{
    MoveThenBySubstate0D5B();
}

/*
 * @routine 0D5B:7B1 TurnPartToTarget0D5B
 * @run 70:B7B1
 * WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2 toward the target a table gives by
 * OBJ_WORK_K, keeping it even; on reaching it the substate steps on (Stage 08's overlay).
 * HOW: A table read, TurnAngleToward, AND #$FE and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    TurnPartToTarget0D5BTargets = 0xB7D5,   /* the target angle for each part */
};

void TurnPartToTarget0D5B(uint8_t slot)
{
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    op_ldy(RD8(OBJ->work_k[slot]));
    op_lda(rd8((uint16_t)(TurnPartToTarget0D5BTargets + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    op_lda(0x02);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB7C8);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    op_and(0xFE);
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnPartToTarget0D5B(void)
{
    TurnPartToTarget0D5B(cpu.x);
}

/*
 * @routine 0D5C:09A BlinkThenRise0D5C
 * @run 70:B89A
 * WHAT IT DOES: Counts OBJ_WORK_M down, switching between two frames as it goes; at zero moves the object up 4
 * pixels, sets its Y speed to -1 and runs BecomeWorkLKind0D5C (Stage 08's overlay).
 * HOW: DEC; the count's bit 0 indexes the frames; ApplyVelocityY, stores, then falls into the next routine.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BlinkThenRise0D5CFrames = 0xB8C6,   /* the two frames switched between */
};

void BlinkThenRise0D5C(uint8_t slot)
{
    RMW8(OBJ->work_m[slot], v, f_dec(v));
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_m[slot]));
        op_and(0x01);
        op_tay();
        op_lda(rd8((uint16_t)(BlinkThenRise0D5CFrames + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    op_lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xB8B4);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], cpu.a);
    /* falls into $B8BC */
    run_routine(0xB8BC);
}

void abi_BlinkThenRise0D5C(void)
{
    BlinkThenRise0D5C(cpu.x);
}

/*
 * @routine 0D5C:0C8 ActByStateUnlessBroken0D5C
 * @run 70:B8C8
 * WHAT IT DOES: Runs BreakUnlessLeaderAlive0D5C; unless that set the carry, runs the routine for the object's state
 * (Stage 08's overlay).
 * HOW: A call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByStateUnlessBroken0D5CJumps = 0xB8E0,   /* the routine for each state, a word each */
};

void ActByStateUnlessBroken0D5C(void)
{
    call_routine(0xB8CB, 0xB8E8);
    if (flag_c()) {
        return;
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByStateUnlessBroken0D5CJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByStateUnlessBroken0D5CJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByStateUnlessBroken0D5C(void)
{
    ActByStateUnlessBroken0D5C();
}

/*
 * @routine 0D5C:11B FollowYAndCount0D5C
 * @run 70:B91B
 * WHAT IT DOES: Puts the object at the place of slot Y and counts OBJ_WORK_F down by eights of OBJ_WORK_E: every 8
 * frames OBJ_WORK_E steps down, and at zero the state steps on (Stage 08's overlay).
 * HOW: CopyObjectPlaceBack, then two nested countdowns.
 * WAITS: no.
 * Entry: X the object; Y the slot whose place it takes. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
void FollowYAndCount0D5C(uint8_t slot)
{
    jsr_push(0xB91E);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    RMW8(OBJ->work_f[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x08);
        WR8(OBJ->work_f[slot], cpu.a);
        RMW8(OBJ->work_e[slot], v, f_dec(v));
        if (flag_z()) {
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_FollowYAndCount0D5C(void)
{
    FollowYAndCount0D5C(cpu.x);
}

/*
 * @routine 0D5C:131 LaunchWhenOwnerAnimEnds0D5C
 * @run 70:B931
 * WHAT IT DOES: Plays the object's frame list; clears it once the slot in OBJ_WORK_D is in state 4, else puts it at
 * that slot's place and plays that slot's own list; when that list ends, launches the object toward the player at
 * speed $3C0 with hit box 2 and steps its state on, or clears it when the angle is under $40 or $C0 and up (Stage
 * 08's overlay).
 * HOW: StepObjectFrames twice with zRichterObject keeping the slot, CopyObjectPlaceBack, AngleToPlayer and two
 * compares, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LaunchWhenOwnerAnimEnds0D5C(void)
{
    op_lda(0x87);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB93C);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_ldy(RD8(OBJ->work_d[cpu.x]));
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x04);
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
    jsr_push(0xB94C);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
    jsr_pop();
    WR8(ZP->richter_object, cpu.x);
    op_sxy();
    op_lda(0x8E);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB95A);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_php();
    op_ldx(RD8(ZP->richter_object));
    op_plp();
    if (flag_z()) {
        jsr_push(0xB963);
        angle_exit_shim(AngleToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x40);
        if (!flag_c()) {
            ClearObject(cpu.x);
            return;
        }
        op_cmp(0xC0);
        if (flag_c()) {
            ClearObject(cpu.x);
            return;
        }
        op_pha();
        op_lda(0xC0);
        WR8(ZP->temp00, cpu.a);
        op_lda(0x03);
        WR8(ZP->temp01, cpu.a);
        op_pla();
        jsr_push(0xB97E);
        scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
        jsr_pop();
        op_lda(0x02);
        WR8(OBJ->hitbox_on[cpu.x], cpu.a);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
}

void abi_LaunchWhenOwnerAnimEnds0D5C(void)
{
    LaunchWhenOwnerAnimEnds0D5C();
}

/*
 * @routine 0D5C:1BD FlyUntilFarOff0D5C
 * @run 70:B9BD
 * WHAT IT DOES: Clears the object once its X or Y high byte is 2 or $FE; otherwise plays its frame list and moves
 * it by its speed (Stage 08's overlay).
 * HOW: Four compares, then StepObjectFrames and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlyUntilFarOff0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->x_hi[slot]));
    op_cmp(0x02);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_cmp(0xFE);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(RD8(OBJ->y_hi[slot]));
    op_cmp(0x02);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_cmp(0xFE);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(0x87);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB9EA);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_FlyUntilFarOff0D5C(void)
{
    FlyUntilFarOff0D5C(cpu.x);
}

/*
 * @routine 0D5C:1ED ActByStateAfterHitCheck0D5C
 * @run 70:B9ED
 * WHAT IT DOES: Runs BreakUpWhenHit0D5C, then the routine for the object's state (Stage 08's overlay).
 * HOW: A call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByStateAfterHitCheck0D5CJumps = 0xBA02,   /* the routine for each state, a word each */
};

void ActByStateAfterHitCheck0D5C(void)
{
    call_routine(0xB9F0, 0xBA0E);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByStateAfterHitCheck0D5CJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByStateAfterHitCheck0D5CJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByStateAfterHitCheck0D5C(void)
{
    ActByStateAfterHitCheck0D5C();
}

/*
 * @routine 0D5C:252 StartWithKind26Partner0D5C
 * @run 70:BA52
 * WHAT IT DOES: Starts the object with OBJ_WORK_E $60 and OBJ_WORK_D $F0 and an object of kind $26 at its place in
 * the next slot; with OBJ_START_ARG negative it begins in state 5 (Stage 08's overlay).
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, stores, a sign test.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithKind26Partner0D5C(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBA55);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    op_txa();
    op_tay();
    op_iny();
    jsr_push(0xBA60);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x26);
    jsr_push(0xBA65);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0xF0);
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    if (flag_n()) {
        op_lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_StartWithKind26Partner0D5C(void)
{
    StartWithKind26Partner0D5C(cpu.x);
}

/*
 * @routine 0D5C:2B0 BounceThreeTimes0D5C
 * @run 70:BAB0
 * WHAT IT DOES: Moves the object under gravity; on reaching Y $B8 it stops there and, for its first three landings,
 * bounces with the Y speed a table gives by OBJ_WORK_J; after that OBJ_WORK_D becomes $90 and the state 2 (Stage
 * 08's overlay).
 * HOW: ApplyGravity and ApplyVelocity, a compare of Y, a table read by twice OBJ_WORK_J.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BounceThreeTimes0D5CSpeeds = 0xBAEB,   /* the Y speed of each bounce, a word each */
};

void BounceThreeTimes0D5C(uint8_t slot)
{
    jsr_push(0xBAB3);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    jsr_push(0xBAB6);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_hi[slot]));
    if (flag_z()) {
        op_lda(RD8(OBJ->y_lo[slot]));
        op_cmp(0xB8);
        if (flag_c()) {
            op_lda(0xB8);
            WR8(OBJ->y_lo[slot], cpu.a);
            op_lda(RD8(OBJ->work_j[slot]));
            op_cmp(0x03);
            if (!flag_c()) {
                cpu.a = f_asl(cpu.a);
                op_tay();
                op_lda(rd8((uint16_t)(BounceThreeTimes0D5CSpeeds + cpu.y)));
                WR8(OBJ->vy_lo[slot], cpu.a);
                op_lda(rd8((uint16_t)(BounceThreeTimes0D5CSpeeds + 1 + cpu.y)));
                WR8(OBJ->vy_hi[slot], cpu.a);
                RMW8(OBJ->work_j[slot], v, f_inc(v));
                return;
            }
            op_lda(0x90);
            WR8(OBJ->work_d[slot], cpu.a);
            op_lda(0x02);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_BounceThreeTimes0D5C(void)
{
    BounceThreeTimes0D5C(cpu.x);
}

/*
 * @routine 0D5C:2F3 StateOnPlayerNear0D5C
 * @run 70:BAF3
 * WHAT IT DOES: Steps the object's state on once the player is less than $78 pixels away along X (Stage 08's
 * overlay).
 * HOW: DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnPlayerNear0D5C(uint8_t slot)
{
    jsr_push(0xBAF6);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x78);
    if (!flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_StateOnPlayerNear0D5C(void)
{
    StateOnPlayerNear0D5C(cpu.x);
}

/*
 * @routine 0D5C:2FE SpawnSixKind27AfterAnim0D5C
 * @run 70:BAFE
 * WHAT IT DOES: Plays the object's frame list; when it ends, steps the state on with hit box 2 and health 3 and
 * starts six objects of kind $27 at its place in the slots two to seven after it, each with the object's slot in
 * OBJ_WORK_L, its index 0 to 5 in OBJ_WORK_M and OBJ_WORK_D 3, $0B, $13 and so on (Stage 08's overlay).
 * HOW: StepObjectFrames, stores, a loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SpawnSixKind27AfterAnim0D5C(uint8_t slot)
{
    op_lda(0x4F);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBB09);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        op_lda(0x03);
        WR8(OBJ->health[slot], cpu.a);
        op_txa();
        op_tay();
        op_iny();
        op_iny();
        op_lda(0x06);
        WR8(ZP->temp00, cpu.a);
        op_lda(0x03);
        WR8(ZP->temp01, cpu.a);
        WR8(ZP->temp02, 0);
        do {
            jsr_push(0xBB2F);
            ClearObjectInY(cpu.y);
            jsr_pop();
            op_lda(0x27);
            jsr_push(0xBB34);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_txa();
            WR8(OBJ->work_l[cpu.y], cpu.a);
            op_lda(RD8(ZP->temp01));
            WR8(OBJ->work_d[cpu.y], cpu.a);
            op_clc();
            op_adc(0x08);
            WR8(ZP->temp01, cpu.a);
            op_lda(RD8(ZP->temp02));
            WR8(OBJ->work_m[cpu.y], cpu.a);
            RMW8(ZP->temp02, v, f_inc(v));
            op_iny();
            RMW8(ZP->temp00, v, f_dec(v));
        } while (!flag_z());
    }
}

void abi_SpawnSixKind27AfterAnim0D5C(void)
{
    SpawnSixKind27AfterAnim0D5C(cpu.x);
}

/*
 * @routine 0D5C:35C HomeOnPlayerWavering0D5C
 * @run 70:BB5C
 * WHAT IT DOES: Plays the object's frame list and turns its angle OBJ_WORK_D by 1 toward the point $20 above the
 * player, or toward $C0 when OBJ_START_ARG is 1 and slot $10 holds kind $22; then moves at that angle with speed
 * $140 plus a cosine swing by OBJ_WORK_H, which steps by 2. In that second case, once its Y high byte is not 0 it
 * clears its own and the next seven slots (Stage 08's overlay).
 * HOW: StepObjectFrames, AngleToPlayerOffset, TurnAngleToward, ScaleByCosine, SetVelocityFromAngle, ApplyVelocity;
 * a loop of ClearObjectInY.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void HomeOnPlayerWavering0D5C(uint8_t slot)
{
    op_lda(0xDB);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBB67);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->start_arg[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        op_lda(RD8(OBJ->kind[0x10]));
        op_cmp(0x22);
        if (flag_z()) {
            op_lda(RD8(OBJ->y_hi[slot]));
            if (!flag_z()) {
                op_txa();
                op_tay();
                op_lda(0x08);
                WR8(ZP->temp00, cpu.a);
                do {
                    jsr_push(0xBB83);
                    ClearObjectInY(cpu.y);
                    jsr_pop();
                    op_iny();
                    RMW8(ZP->temp00, v, f_dec(v));
                } while (!flag_z());
                return;
            }
            op_lda(0xC0);
            goto at_BB9C;
        }
    }
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    op_lda(0xE0);
    WR8(ZP->temp0a, cpu.a);
    op_lda(0xFF);
    WR8(ZP->temp0b, cpu.a);
    jsr_push(0xBB9C);
    angle_exit_shim(AngleToPlayerOffset(slot));
    jsr_pop();
at_BB9C:
    WR8(ZP->temp02, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xBBAA);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(0xC0);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x00);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_h[slot]));
    jsr_push(0xBBBB);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_lda(RD8(OBJ->work_h[slot]));
    op_clc();
    op_adc(0x02);
    WR8(OBJ->work_h[slot], cpu.a);
    op_lda(RD8(ZP->product.mid));
    op_clc();
    op_adc(0x40);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(ZP->product.hi));
    op_adc(0x01);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xBBD7);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xBBDA);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

void abi_HomeOnPlayerWavering0D5C(void)
{
    HomeOnPlayerWavering0D5C(cpu.x);
}

/*
 * @routine 0D5C:409 RideLeaderThenRelease0D5C
 * @run 70:BC09
 * WHAT IT DOES: Puts the object at the place of the slot in OBJ_WORK_L; once that slot's state is 3 or more, counts
 * OBJ_WORK_D down and at zero gives the object the frame a table picks by OBJ_WORK_M, OBJ_WORK_E $40 and the next
 * state (Stage 08's overlay).
 * HOW: CopyObjectPlaceBack, a compare, DEC, a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RideLeaderThenRelease0D5CFrames = 0xBC2D,   /* the frame for each OBJ_WORK_M */
};

void RideLeaderThenRelease0D5C(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_l[slot]));
    jsr_push(0xBC0F);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x03);
    if (flag_c()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        if (flag_z()) {
            op_ldy(RD8(OBJ->work_m[slot]));
            op_lda(rd8((uint16_t)(RideLeaderThenRelease0D5CFrames + cpu.y)));
            WR8(OBJ->frame[slot], cpu.a);
            op_lda(0x40);
            WR8(OBJ->work_e[slot], cpu.a);
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_RideLeaderThenRelease0D5C(void)
{
    RideLeaderThenRelease0D5C(cpu.x);
}

/*
 * @routine 0D5C:433 SwingAndSpeedUp0D5C
 * @run 70:BC33
 * WHAT IT DOES: Takes the sprite attributes of the slot in OBJ_WORK_L and moves (FollowAndMoveAtAngle0D5C); counts
 * OBJ_WORK_N down to turn into kind $9B in state 0 with sound $10. Otherwise raises the speed OBJ_WORK_J-K by 8 a
 * frame up to $600, swings the angle OBJ_WORK_E-F by a cosine of OBJ_WORK_G, and steps OBJ_WORK_G-H by a word of
 * four phases picked by OBJ_WORK_I; when that carries the angle goes back to $40 and the phase steps on (Stage 08's
 * overlay).
 * HOW: A call, DEC, SoundCommand, 16-bit adds, ScaleByCosine and a table read by twice OBJ_WORK_I's low two bits.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SwingAndSpeedUp0D5CSteps = 0xBCB6,   /* the step of OBJ_WORK_G-H for each phase, a word each */
};

void SwingAndSpeedUp0D5C(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_l[slot]));
    op_lda(RD8(OBJ->sprite_attr[cpu.y]));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    jsr_push(0xBC3F);
    FollowAndMoveAtAngle0D5C(slot);
    jsr_pop();
    op_lda(RD8(OBJ->work_n[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_n[slot], v, f_dec(v));
        if (flag_z()) {
            op_lda(0x10);
            {
                uint8_t command;

                jsr_push(0xBC4E);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
            op_lda(0x9B);
            WR8(OBJ->kind[slot], cpu.a);
            WR8(OBJ->state[slot], 0);
            return;
        }
    }
    op_lda(RD8(OBJ->work_j[slot]));
    op_cmp(0x06);
    if (!flag_c()) {
        op_lda(RD8(OBJ->work_k[slot]));
        op_clc();
        op_adc(0x08);
        WR8(OBJ->work_k[slot], cpu.a);
        op_lda(RD8(OBJ->work_j[slot]));
        op_adc(0x00);
        WR8(OBJ->work_j[slot], cpu.a);
    }
    op_lda(0x04);
    WR8(ZP->temp01, cpu.a);
    op_lda(0x00);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xBC7D);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_lda(RD8(OBJ->work_f[slot]));
    op_clc();
    op_adc(RD8(ZP->product.mid));
    WR8(OBJ->work_f[slot], cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    op_adc(RD8(ZP->product.hi));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->work_i[slot]));
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->work_h[slot]));
    op_clc();
    op_adc(rd8((uint16_t)(SwingAndSpeedUp0D5CSteps + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    op_adc(rd8((uint16_t)(SwingAndSpeedUp0D5CSteps + 1 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    if (flag_c()) {
        op_lda(0x40);
        WR8(OBJ->work_e[slot], cpu.a);
        WR8(OBJ->work_f[slot], 0);
        RMW8(OBJ->work_i[slot], v, f_inc(v));
    }
}

void abi_SwingAndSpeedUp0D5C(void)
{
    SwingAndSpeedUp0D5C(cpu.x);
}

/*
 * @routine 0D5C:4BE FollowAndMoveAtAngle0D5C
 * @run 70:BCBE
 * WHAT IT DOES: Puts the object at the place of the slot below it, or with OBJ_WORK_M 0 of the slot two below and 4
 * pixels lower; then moves it at angle OBJ_WORK_E with the speed in OBJ_WORK_J-K (Stage 08's overlay).
 * HOW: CopyObjectPlaceBack, a 16-bit add to Y, SetVelocityFromAngle and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowAndMoveAtAngle0D5C(uint8_t slot)
{
    op_txa();
    op_tay();
    op_dey();
    op_lda(RD8(OBJ->work_m[slot]));
    if (!flag_z()) {
        jsr_push(0xBCC9);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
        jsr_pop();
    } else {
        op_dey();
        jsr_push(0xBCCF);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
        jsr_pop();
        op_lda(RD8(OBJ->y_lo[slot]));
        op_clc();
        op_adc(0x04);
        WR8(OBJ->y_lo[slot], cpu.a);
        if (flag_c()) {
            RMW8(OBJ->y_hi[slot], v, f_inc(v));
        }
    }
    op_lda(RD8(OBJ->work_k[slot]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xBCED);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xBCF0);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

void abi_FollowAndMoveAtAngle0D5C(void)
{
    FollowAndMoveAtAngle0D5C(cpu.x);
}

/*
 * @routine 0D5C:4F1 FaceTowardPlayer0D5C
 * @run 70:BCF1
 * WHAT IT DOES: In state 0 starts the object; then it takes the sprite attributes and place of the slot below it
 * and the frame $13 to $1A that points toward the player, one per eighth of a turn (Stage 08's overlay).
 * HOW: LoadObjectRecord, or CopyObjectPlace, AngleToPlayer plus $10 shifted down five bits plus $13.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FaceTowardPlayer0D5C(void)
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
    op_lda(rd8((uint16_t)(COL(OBJ->sprite_attr) - 1 + cpu.x)));
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    op_txa();
    op_tay();
    op_dey();
    op_sxy();
    jsr_push(0xBD06);
    copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
    jsr_pop();
    op_sxy();
    jsr_push(0xBD0A);
    angle_exit_shim(AngleToPlayer(cpu.x));
    jsr_pop();
    op_clc();
    op_adc(0x10);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_clc();
    op_adc(0x13);
    WR8(OBJ->frame[cpu.x], cpu.a);
}

void abi_FaceTowardPlayer0D5C(void)
{
    FaceTowardPlayer0D5C();
}

/*
 * @routine 0D5C:525 StartAndAnimate0D5C
 * @run 70:BD25
 * WHAT IT DOES: In state 0 starts the object with sprite set 6; plays its frame list every frame (Stage 08's
 * overlay).
 * HOW: LoadObjectRecord, a store, StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartAndAnimate0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBD2D);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
    }
    op_lda(0x3D);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xBD);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
    }
}

void abi_StartAndAnimate0D5C(void)
{
    StartAndAnimate0D5C(cpu.x);
}

/*
 * @routine 0D5C:562 StartThenStateByArg0D5C
 * @run 70:BD62
 * WHAT IT DOES: Starts the object and runs StateByStartArg0D5C (Stage 08's overlay).
 * HOW: LoadObjectRecord, then falls into the next piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartThenStateByArg0D5C(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBD65);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    /* falls into $BD65 */
    StateByStartArg0D5C(cpu.x);
}

void abi_StartThenStateByArg0D5C(void)
{
    StartThenStateByArg0D5C();
}

/*
 * @routine 0D5C:565 StateByStartArg0D5C
 * @run 70:BD65
 * WHAT IT DOES: Sets the object's state by OBJ_START_ARG: positive runs State3UnlessEnd0D5C, negative gives state
 * 5; with 0, unless OBJ_WORK_D is 3, it runs SetFrame60_0D5C (Stage 08's overlay).
 * HOW: Sign and compare tests, then jumps or falls into the next routine.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateByStartArg0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        if (!flag_n()) {
            State3UnlessEnd0D5C();
            return;
        }
        op_lda(0x05);
        SetStateFromA0D5C(slot);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x03);
    if (flag_z()) {
        ReturnOnly0D5C();
        return;
    }
    /* falls into $BD77 */
    run_routine(0xBD77);
}

void abi_StateByStartArg0D5C(void)
{
    StateByStartArg0D5C(cpu.x);
}

/*
 * @routine 0D5C:57F ReturnOnly0D5C
 * @run 70:BD7F
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D5C(void)
{
}

void abi_ReturnOnly0D5C(void)
{
    ReturnOnly0D5C();
}

/*
 * @routine 0D5C:580 State3UnlessEnd0D5C
 * @run 70:BD80
 * WHAT IT DOES: Runs SetFrame60_0D5C unless OBJ_WORK_D is 3 or the object is in slot $23, then puts the object in
 * state 3 (Stage 08's overlay).
 * HOW: Two compares, a call, then falls into SetStateFromA0D5C with A 3.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State3UnlessEnd0D5C(void)
{
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_cmp(0x03);
    if (!flag_z()) {
        op_cpx(0x23);
        if (!flag_z()) {
            call_routine(0xBD8D, 0xBD77);
        }
    }
    op_lda(0x03);
    /* falls into $BD8F */
    SetStateFromA0D5C(cpu.x);
}

void abi_State3UnlessEnd0D5C(void)
{
    State3UnlessEnd0D5C();
}

/*
 * @routine 0D5C:58F SetStateFromA0D5C
 * @run 70:BD8F
 * WHAT IT DOES: Puts the object in the state in A (Stage 08's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object; A the state. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetStateFromA0D5C(uint8_t slot)
{
    WR8(OBJ->state[slot], cpu.a);
}

void abi_SetStateFromA0D5C(void)
{
    SetStateFromA0D5C(cpu.x);
}

/*
 * @routine 0D5C:593 BecomeKindACFrame5C0D5C
 * @run 70:BD93
 * WHAT IT DOES: Turns the object into kind $AC, starts it with frame $5C and sets its state by OBJ_START_ARG
 * (StateByStartArg0D5C) (Stage 08's overlay).
 * HOW: A store, LoadObjectRecord, a store, then the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BecomeKindACFrame5C0D5C(uint8_t slot)
{
    op_lda(0xAC);
    WR8(OBJ->kind[slot], cpu.a);
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBD9B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x5C);
    WR8(OBJ->frame[slot], cpu.a);
    StateByStartArg0D5C(slot);
}

void abi_BecomeKindACFrame5C0D5C(void)
{
    BecomeKindACFrame5C0D5C(cpu.x);
}

/*
 * @routine 0D5C:5A2 StartFrame5F0D5C
 * @run 70:BDA2
 * WHAT IT DOES: Starts the object with OBJ_WORK_C 0, sprite set 6 and frame $5F (Stage 08's overlay).
 * HOW: LoadObjectRecord, three stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFrame5F0D5C(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBDA5);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    WR8(OBJ->work_c[slot], 0);
    op_lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
    op_lda(0x5F);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_StartFrame5F0D5C(void)
{
    StartFrame5F0D5C(cpu.x);
}

/*
 * @routine 0D5C:5B3 BuildChainOfKindBA0D5C
 * @run 70:BDB3
 * WHAT IT DOES: By OBJ_WORK_D 3, 2 or 1 runs BecomeKindACFrame5C0D5C, StartFrame5F0D5C or StartThenStateByArg0D5C.
 * With 0 starts the object with frame $5D and builds a chain of kind $BA objects in the slots after it, each with
 * the slot before it in OBJ_WORK_F: with OBJ_START_ARG 0 five (OBJ_WORK_E $0E, the last OBJ_WORK_D 2), then six, or
 * seven for a negative argument, that take its OBJ_START_ARG and OBJ_WORK_E $E0, or $72 for 0, the last OBJ_WORK_D
 * 3 and the first the object's slot; then clears the slots up to $2B (Stage 08's overlay).
 * HOW: Compares of OBJ_WORK_D, LoadObjectRecord, loops of ClearObjectInY and StartObjectAtPlace, a clearing loop.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void BuildChainOfKindBA0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x03);
    if (flag_z()) {
        BecomeKindACFrame5C0D5C(slot);
        return;
    }
    op_cmp(0x02);
    if (flag_z()) {
        StartFrame5F0D5C(slot);
        return;
    }
    op_cmp(0x01);
    if (flag_z()) {
        StartThenStateByArg0D5C();
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBDC5);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x5D);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x05);
    WR8(ZP->temp00, cpu.a);
    op_txa();
    op_tay();
    op_iny();
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        if (!flag_n()) {
            goto at_BDFD;
        }
        op_lda(0x07);
        goto at_BDFF;
    }
    do {
        jsr_push(0xBDDF);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0xBA);
        jsr_push(0xBDE4);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(0x0E);
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_tya();
        cpu.a = f_dec(cpu.a);
        WR8(OBJ->work_f[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
    op_lda(0x02);
    wr8((uint16_t)(COL(OBJ->work_d) - 1 + cpu.y), cpu.a);
at_BDFD:
    op_lda(0x06);
at_BDFF:
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, cpu.a);
    do {
        jsr_push(0xBE06);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0xBA);
        jsr_push(0xBE0B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        if (!flag_z()) {
            op_lda(0xE0);
        } else {
            op_lda(0x72);
        }
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_tya();
        cpu.a = f_dec(cpu.a);
        WR8(OBJ->work_f[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
    op_lda(0x03);
    wr8((uint16_t)(COL(OBJ->work_d) - 1 + cpu.y), cpu.a);
    op_phy();
    op_tya();
    op_sec();
    op_sbc(RD8(ZP->temp01));
    op_tay();
    op_txa();
    WR8(OBJ->work_f[cpu.y], cpu.a);
    op_ply();
at_BE3B:
    op_cpy(0x2C);
    if (!flag_z()) {
        jsr_push(0xBE42);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_iny();
        goto at_BE3B;
    }
}

void abi_BuildChainOfKindBA0D5C(void)
{
    BuildChainOfKindBA0D5C(cpu.x);
}

/*
 * @routine 0D5C:78C SwitchBlockByEvent0D5C
 * @run 70:BF8C
 * WHAT IT DOES: In state 0 starts the object with hit box Y offset $10; in state 1, once the event flag
 * OBJ_START_ARG names is set, steps OBJ_WORK_D and the state on; in every state after 0 it is a block the player
 * can stand on, $18 wide and $16 high (Stage 08's overlay).
 * HOW: LoadObjectRecord, or BitOfIndex and an AND of the event flags, then ContactWithPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwitchBlockByEvent0D5C(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBF94);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x10);
        WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        {
            uint8_t index;
            uint8_t byte;
            uint8_t mask;

            jsr_push(0xBFA3);
            index = cpu.a;
            mask = BitOfIndex(index, cpu.x, &byte);
            bit_of_index_exit_shim(index, mask, byte);
            jsr_pop();
        }
        op_and(RD8(LEVEL->event_flags[cpu.y]));
        if (!flag_z()) {
            RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
        }
    }
    op_lda(0x18);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xBFB9, 0xAA35);
}

void abi_SwitchBlockByEvent0D5C(void)
{
    SwitchBlockByEvent0D5C();
}

/*
 * @routine 0D5C:7BB SagUnderSlotArg0D5C
 * @run 70:BFBB
 * WHAT IT DOES: In state 0 starts the object with sprite set 6 and keeps in OBJ_WORK_D the Y of the slot
 * OBJ_START_ARG names. In state 1, while that slot's OBJ_WORK_D is 0, counts OBJ_WORK_E up and picks a sag 0 to
 * $1F: 0 below $40, then growing by a speed that gains $10 a frame, then from $80 shrinking back to 0; it then sets
 * the frame and that slot's Y (SetFrameAndSlotY0D5D); once that slot's OBJ_WORK_D is not 0 the state steps on
 * (Stage 08's overlay).
 * HOW: LoadObjectRecord; a count, 16-bit adds of a speed to OBJ_WORK_F-G, a clamp, then the shared piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SagUnderSlotArg0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBFC3);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->y_lo[cpu.y]));
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->work_d[cpu.y]));
        if (flag_z()) {
            RMW8(OBJ->work_e[slot], v, f_inc(v));
            if (!flag_z()) {
                op_lda(RD8(OBJ->work_e[slot]));
                if (flag_n()) {
                    goto at_C023;
                }
                op_cmp(0x40);
                if (!flag_c()) {
                    goto at_C020;
                }
                op_lda(0x10);
                op_clc();
                op_adc(RD8(OBJ->vy_lo[slot]));
                WR8(OBJ->vy_lo[slot], cpu.a);
                if (flag_c()) {
                    RMW8(OBJ->vy_hi[slot], v, f_inc(v));
                }
                op_lda(RD8(OBJ->work_f[slot]));
                op_clc();
                op_adc(RD8(OBJ->vy_lo[slot]));
                WR8(OBJ->work_f[slot], cpu.a);
                op_lda(RD8(OBJ->work_g[slot]));
                op_adc(RD8(OBJ->vy_hi[slot]));
                WR8(OBJ->work_g[slot], cpu.a);
                op_cmp(0x1F);
                if (!flag_c()) {
                    SetFrameAndSlotY0D5D(slot);
                    return;
                }
                op_lda(0x1F);
                SetFrameAndSlotY0D5D(slot);
                return;
            }
            WR8(OBJ->vy_lo[slot], 0);
            WR8(OBJ->vy_hi[slot], 0);
            WR8(OBJ->work_f[slot], 0);
            WR8(OBJ->work_g[slot], 0);
at_C020:
            op_cla();
            SetFrameAndSlotY0D5D(slot);
            return;
at_C023:
            op_eor(0xFF);
            cpu.a = f_lsr(cpu.a);
            cpu.a = f_lsr(cpu.a);
            SetFrameAndSlotY0D5D(slot);
            return;
        }
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_SagUnderSlotArg0D5C(void)
{
    SagUnderSlotArg0D5C(cpu.x);
}

/*
 * @routine 0D5D:02D SwaySlotArgBySine0D5D
 * @run 71:C02D
 * WHAT IT DOES: In state 0 starts the object with sprite set 6 and keeps in OBJ_WORK_D the Y of the slot
 * OBJ_START_ARG names. In state 1, while that slot's OBJ_WORK_D is 0, steps OBJ_WORK_E and takes a sag of $10 plus
 * a sine swing, 0 to $1F, for the frame and that slot's Y (SetFrameAndSlotY0D5D); once it is not 0 the state steps
 * on (Stage 08's overlay).
 * HOW: LoadObjectRecord; ScaleBySine of twice OBJ_WORK_E by $0FFF, then the shared piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwaySlotArgBySine0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC035);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->y_lo[cpu.y]));
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        ReturnOnly0D5D();
        return;
    }
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(RD8(OBJ->work_d[cpu.y]));
    if (!flag_z()) {
        StepStateThenReturn0D5D(slot);
        return;
    }
    op_lda(0xFF);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x0F);
    WR8(ZP->temp01, cpu.a);
    RMW8(OBJ->work_e[slot], v, f_inc(v));
    op_lda(RD8(OBJ->work_e[slot]));
    cpu.a = f_asl(cpu.a);
    jsr_push(0xC061);
    scale_exit_shim(ScaleBySine(cpu.a));
    jsr_pop();
    op_lda(RD8(ZP->product.hi));
    op_clc();
    op_adc(0x10);
    op_and(0x1F);
    /* falls into $C068 */
    SetFrameAndSlotY0D5D(slot);
}

void abi_SwaySlotArgBySine0D5D(void)
{
    SwaySlotArgBySine0D5D(cpu.x);
}

/*
 * @routine 0D5D:068 SetFrameAndSlotY0D5D
 * @run 71:C068
 * WHAT IT DOES: From a sag in A gives the object frame $33 less the sag, and puts the slot OBJ_START_ARG names at Y
 * OBJ_WORK_D plus twice the sag (Stage 08's overlay).
 * HOW: A subtract, a doubled add, two stores.
 * WAITS: no.
 * Entry: X the object; A the sag, 0 to $1F. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetFrameAndSlotY0D5D(uint8_t slot)
{
    WR8(ZP->temp10, cpu.a);
    op_lda(0x33);
    op_sec();
    op_sbc(RD8(ZP->temp10));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(ZP->temp10));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_d[slot]));
    op_ldy(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
}

void abi_SetFrameAndSlotY0D5D(void)
{
    SetFrameAndSlotY0D5D(cpu.x);
}

/*
 * @routine 0D5D:080 StepStateThenReturn0D5D
 * @run 71:C080
 * WHAT IT DOES: Steps the object's state on (Stage 08's overlay).
 * HOW: INC, then falls into the RTS.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StepStateThenReturn0D5D(uint8_t slot)
{
    RMW8(OBJ->state[slot], v, f_inc(v));
    /* falls into $C083 */
    ReturnOnly0D5D();
}

void abi_StepStateThenReturn0D5D(void)
{
    StepStateThenReturn0D5D(cpu.x);
}

/*
 * @routine 0D5D:083 ReturnOnly0D5D
 * @run 71:C083
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D5D(void)
{
}

void abi_ReturnOnly0D5D(void)
{
    ReturnOnly0D5D();
}

/*
 * @routine 0D5D:084 SlideInThenRideOrFall0D5D
 * @run 71:C084
 * WHAT IT DOES: In state 0 starts the object moving left at 4 pixels a frame (hit box X offset -8, OBJ_WORK_D $35);
 * state 2 plays sound 6, records its event and moves on. In state 3 it moves along X counting OBJ_WORK_D down, at
 * $25 stepping on the state of the slot OBJ_START_ARG's low bits name; hit once or twice it keeps its offset from
 * the player and stays there 16 frames before it is cleared (state 5); hit 3 times or more, or when the count runs
 * out with OBJ_START_ARG not negative, it wakes that slot (WakeSlotInStartArg0D5D) and falls away with X speed $80
 * and Y speed -2, gaining $20 a frame (state 4) (Stage 08's overlay).
 * HOW: A chain of DEC tests of the state; LoadObjectRecord, SoundCommand, RecordObjectEvent, ApplyVelocityX,
 * ApplyVelocity, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SlideInThenRideOrFall0D5D(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC08C);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0xF8);
        WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
        op_lda(0xFC);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0x35);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_C0A0;
    }
at_C09F:
    return;
at_C0A0:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_C0AF;
    }
    op_lda(0x06);
    {
        uint8_t command;

        jsr_push(0xC0A8);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
        jsr_pop();
    }
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xC0AB);
        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
at_C0AB:
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    return;
at_C0AF:
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->hit_taken[cpu.x]));
        if (!flag_z()) {
            op_cmp(0x03);
            if (flag_c()) {
                goto at_C102;
            }
            op_lda(RD8(OBJ->x_lo[cpu.x]));
            op_sec();
            op_sbc(RD8(OBJ->x_lo[0x00]));
            WR8(OBJ->vx_lo[cpu.x], cpu.a);
            op_lda(RD8(OBJ->y_lo[cpu.x]));
            op_sec();
            op_sbc(RD8(OBJ->y_lo[0x00]));
            WR8(OBJ->vy_lo[cpu.x], cpu.a);
            op_lda(0x10);
            WR8(OBJ->work_d[cpu.x], cpu.a);
            call_routine(0xC0D7, 0xC145);
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
            return;
        }
        jsr_push(0xC0E1);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
        RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
        if (!flag_z()) {
            op_lda(RD8(OBJ->work_d[cpu.x]));
            op_cmp(0x25);
            if (!flag_z()) {
                goto at_C09F;
            }
            op_lda(RD8(OBJ->start_arg[cpu.x]));
            op_and(0x7F);
            if (flag_z()) {
                goto at_C09F;
            }
            op_tay();
            op_lda(RD8(OBJ->state[cpu.y]));
            cpu.a = f_inc(cpu.a);
            WR8(OBJ->state[cpu.y], cpu.a);
            return;
        }
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (flag_n()) {
            goto at_C09F;
        }
at_C102:
        call_routine(0xC105, 0xC145);
        op_lda(0x80);
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
        WR8(OBJ->vx_hi[cpu.x], 0);
        op_lda(0xFE);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        goto at_C0AB;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(0x20);
        op_clc();
        op_adc(RD8(OBJ->vy_lo[cpu.x]));
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        if (flag_c()) {
            RMW8(OBJ->vy_hi[cpu.x], v, f_inc(v));
        }
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        return;
    }
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (!flag_z()) {
        op_lda(RD8(OBJ->x_lo[0x00]));
        op_clc();
        op_adc(RD8(OBJ->vx_lo[cpu.x]));
        WR8(OBJ->x_lo[cpu.x], cpu.a);
        op_lda(RD8(OBJ->y_lo[0x00]));
        op_clc();
        op_adc(RD8(OBJ->vy_lo[cpu.x]));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        return;
    }
    ClearObject(cpu.x);
}

void abi_SlideInThenRideOrFall0D5D(void)
{
    SlideInThenRideOrFall0D5D();
}

/*
 * @routine 0D5D:159 MoveOnEventThenStand0D5D
 * @run 71:C159
 * @stop C181
 * WHAT IT DOES: In state 0 starts the object with hit box X offset $14, X speed 2 and OBJ_WORK_D $28; in state 1,
 * once the event flag OBJ_START_ARG names is set, steps on; in state 2 moves $28 frames then steps on. In states 1
 * and up it is a block the player can stand on (StandOnBlock0D5D) (Stage 08's overlay).
 * HOW: LoadObjectRecord, BitOfIndex and an AND of the event flags, or the pieces after it.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveOnEventThenStand0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC161);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x14);
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        op_lda(0x02);
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(0x28);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        MoveForCountThenStep0D5D(slot);
        return;
    }
    op_lda(RD8(OBJ->start_arg[slot]));
    {
        uint8_t index;
        uint8_t byte;
        uint8_t mask;

        jsr_push(0xC17A);
        index = cpu.a;
        mask = BitOfIndex(index, slot, &byte);
        bit_of_index_exit_shim(index, mask, byte);
        jsr_pop();
    }
    op_and(RD8(LEVEL->event_flags[cpu.y]));
    if (!flag_z()) {
        StepState0D5D(slot);
        return;
    }
    StandOnBlock0D5D();
}

void abi_MoveOnEventThenStand0D5D(void)
{
    MoveOnEventThenStand0D5D(cpu.x);
}

/*
 * @routine 0D5D:182 MoveForCountThenStep0D5D
 * @run 71:C182
 * WHAT IT DOES: In state 2 moves the object by its speed and counts OBJ_WORK_D down, stepping the state on at zero;
 * in any state it then is a block the player can stand on (StandOnBlock0D5D) (Stage 08's overlay).
 * HOW: DEC of the state copy in A, ApplyVelocity, DEC, then the pieces after it.
 * WAITS: no.
 * Entry: X the object; A its state less 1. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveForCountThenStep0D5D(uint8_t slot)
{
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        StandOnBlock0D5D();
        return;
    }
    jsr_push(0xC188);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (!flag_z()) {
        StandOnBlock0D5D();
        return;
    }
    /* falls into $C18D */
    StepState0D5D(slot);
}

void abi_MoveForCountThenStep0D5D(void)
{
    MoveForCountThenStep0D5D(cpu.x);
}

/*
 * @routine 0D5D:18D StepState0D5D
 * @run 71:C18D
 * WHAT IT DOES: Steps the object's state on (Stage 08's overlay).
 * HOW: INC and RTS.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StepState0D5D(uint8_t slot)
{
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_StepState0D5D(void)
{
    StepState0D5D(cpu.x);
}

/*
 * @routine 0D5D:191 StandOnBlock0D5D
 * @run 71:C191
 * WHAT IT DOES: Lets the player stand on the object as a block $18 wide and $26 high, tested with its hit box X
 * offset at 0 and then put back to $14 (Stage 08's overlay).
 * HOW: Two stores around ContactWithPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the player stands on it, from ContactWithPlayer.
 */
void StandOnBlock0D5D(void)
{
    WR8(OBJ->hitbox_dx[cpu.x], 0);
    op_lda(0x18);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x26);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC19F, 0xAA35);
    op_lda(0x14);
    WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
}

void abi_StandOnBlock0D5D(void)
{
    StandOnBlock0D5D();
}

/*
 * @routine 0D5D:1A5 AnimateWithHeightByFrame0D5D
 * @run 71:C1A5
 * WHAT IT DOES: In state 0 starts the object; plays its frame list and gives it the hit box half height of its
 * frame from 3 on (Stage 08's overlay).
 * HOW: LoadObjectRecord, StepObjectFrames, a table read by the frame less 3.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    AnimateWithHeightByFrame0D5DHeights = 0xC1C6,   /* the hit box half height for each frame from 3 */
};

void AnimateWithHeightByFrame0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC1AD);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
    }
    op_lda(0xCE);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xC1);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC1B8);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->frame[slot]));
    op_sec();
    op_sbc(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(AnimateWithHeightByFrame0D5DHeights + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_AnimateWithHeightByFrame0D5D(void)
{
    AnimateWithHeightByFrame0D5D(cpu.x);
}

/*
 * @routine 0D5D:1DB StepWhenStoodOn0D5D
 * @run 71:C1DB
 * WHAT IT DOES: Passes its record on (PassRecordFromState2_0D5D) and is a block $12 wide and $15 high; when the
 * player stands on it and the first three of RichterColumnHit are 0, steps the substate on (Stage 08's overlay).
 * HOW: Two calls, then an OR of three bytes.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StepWhenStoodOn0D5D(void)
{
    call_routine(0xC1DE, 0xC2A6);
    op_lda(0x12);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x15);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC1E9, 0xAA35);
    if (flag_c()) {
        op_lda(RD8(PVARS->column_hit[0]));
        op_ora(RD8(PVARS->column_hit[1]));
        op_ora(RD8(PVARS->column_hit[2]));
        if (flag_z()) {
            RMW8(OBJ->substate[cpu.x], v, f_inc(v));
        }
    }
}

void abi_StepWhenStoodOn0D5D(void)
{
    StepWhenStoodOn0D5D();
}

/*
 * @routine 0D5D:1FA StartOrSinkBlock0D5D
 * @run 71:C1FA
 * WHAT IT DOES: In state 0 keeps its Y plus 8 in OBJ_WORK_D and starts with frame $0C (StartFrameC0D5D); in the
 * other states runs SinkWhenStoodOn0D5D (Stage 08's overlay).
 * HOW: A test of the state, an add, then falls into the next routine.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void StartOrSinkBlock0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        SinkWhenStoodOn0D5D();
        return;
    }
    op_lda(RD8(OBJ->y_lo[slot]));
    op_clc();
    op_adc(0x08);
    WR8(OBJ->work_d[slot], cpu.a);
    /* falls into $C208 */
    StartFrameC0D5D(slot);
}

void abi_StartOrSinkBlock0D5D(void)
{
    StartOrSinkBlock0D5D(cpu.x);
}

/*
 * @routine 0D5D:208 StartFrameC0D5D
 * @run 71:C208
 * WHAT IT DOES: Starts the object with sprite set 6 and frame $0C (Stage 08's overlay).
 * HOW: LoadObjectRecord, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFrameC0D5D(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC20B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
    op_lda(0x0C);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_StartFrameC0D5D(void)
{
    StartFrameC0D5D(cpu.x);
}

/*
 * @routine 0D5D:216 SinkWhenStoodOn0D5D
 * @run 71:C216
 * WHAT IT DOES: A block $12 wide and $15 high that passes its record on (PassRecordFromState2_0D5D). State 1: in
 * substate 0 waits to be stood on (StepWhenStoodOn0D5D), then sinks a pixel a frame to the Y in OBJ_WORK_D, plays
 * sound $C5 and steps on, or with OBJ_START_ARG 0 records its event and is cleared. State 2 steps on once
 * OthersInState2_0D5D sets Z. Later states rise a pixel a frame back to 8 above OBJ_WORK_D and return to state 1,
 * substate 0 (Stage 08's overlay).
 * HOW: DEC tests of the state in A; the calls, INC or DEC of Y and a compare.
 * WAITS: no.
 * Entry: X the object; A its state. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SinkWhenStoodOn0D5D(void)
{
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->substate[cpu.x]));
        if (flag_z()) {
            StepWhenStoodOn0D5D();
            return;
        }
        call_routine(0xC221, 0xC2A6);
        op_lda(0x12);
        WR8(ZP->temp08, cpu.a);
        op_lda(0x15);
        WR8(ZP->temp09, cpu.a);
        call_routine(0xC22C, 0xAA35);
        RMW8(OBJ->y_lo[cpu.x], v, f_inc(v));
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_cmp(RD8(OBJ->work_d[cpu.x]));
        if (!flag_z()) {
            return;
        }
        op_lda(0xC5);
        {
            uint8_t command;

            jsr_push(0xC23D);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (flag_z()) {
            {
                uint8_t stored;
                uint8_t event;
                uint8_t byte;

                jsr_push(0xC245);
                if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                    cpu.a = 0;
                    abi_nz(0);
                } else {
                    record_event_exit_shim(event, stored, byte);
                }
                jsr_pop();
            }
            ClearObject(cpu.x);
            return;
        }
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        call_routine(0xC251, 0xC27F);
        if (flag_z()) {
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
        }
        call_routine(0xC259, 0xC2A6);
        return;
    }
    call_routine(0xC25D, 0xC2A6);
    op_lda(0x12);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x15);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC268, 0xAA35);
    RMW8(OBJ->y_lo[cpu.x], v, f_dec(v));
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(0x08);
    op_cmp(RD8(OBJ->work_d[cpu.x]));
    if (flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_SinkWhenStoodOn0D5D(void)
{
    SinkWhenStoodOn0D5D();
}

/*
 * @routine 0D5D:2E3 StartFrameCHitUp0D5D
 * @run 71:C2E3
 * WHAT IT DOES: Starts the object with sprite set 6 and frame $0C (StartFrameC0D5D) and hit box Y offset $F0 (Stage
 * 08's overlay).
 * HOW: A call and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFrameCHitUp0D5D(uint8_t slot)
{
    jsr_push(0xC2E6);
    StartFrameC0D5D(slot);
    jsr_pop();
    op_lda(0xF0);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
}

void abi_StartFrameCHitUp0D5D(void)
{
    StartFrameCHitUp0D5D(cpu.x);
}

/*
 * @routine 0D5D:2EC RiseToY58AfterHits0D5D
 * @run 71:C2EC
 * WHAT IT DOES: In state 0 starts the object (StartFrameCHitUp0D5D); in state 1, once hit 5 times, plays sound $3A
 * and steps on; then rises a pixel every fourth frame of zWaveCounter until Y $58, where it becomes kind $AF in
 * state 0 (Stage 08's overlay).
 * HOW: DEC tests of the state, a compare of OBJ_HIT_TAKEN, a TST of the counter, DEC of Y, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RiseToY58AfterHits0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        StartFrameCHitUp0D5D(slot);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_C304;
    }
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x05);
    if (flag_c()) {
        op_lda(0x3A);
        {
            uint8_t command;

            jsr_push(0xC300);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
at_C303:
    return;
at_C304:
    op_tst(0x03, RD8(ZP->wave_counter));
    if (!flag_z()) {
        goto at_C303;
    }
    op_lda(RD8(OBJ->y_lo[slot]));
    op_cmp(0x58);
    if (!flag_z()) {
        RMW8(OBJ->y_lo[slot], v, f_dec(v));
        return;
    }
    op_lda(0xAF);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->substate[slot], 0);
}

void abi_RiseToY58AfterHits0D5D(void)
{
    RiseToY58AfterHits0D5D(cpu.x);
}

/*
 * @routine 0D5D:320 SwitchPressedByStanding0D5D
 * @run 71:C320
 * WHAT IT DOES: A block $18 wide and $15 high. In state 0 starts with sprite set 6, frame $34, OBJ_WORK_E 8 and its
 * Y plus 8 in OBJ_WORK_D, already at that Y in state 2 when its event (OBJ_EVENT_ID plus 1) is recorded; in state 1
 * steps on once the player stands on it and the first three of RichterColumnHit are 0; in state 2 sinks a pixel a
 * frame for nine frames, then steps on the state of the slot OBJ_START_ARG names, its own state, plays sound $C5
 * and records the event (Stage 08's overlay).
 * HOW: DEC tests of the state; LoadObjectRecord, BitOfIndex, ContactWithPlayer, SoundCommand, RecordEvent.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwitchPressedByStanding0D5D(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC328);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_clc();
        op_adc(0x08);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        op_lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_lda(0x34);
        WR8(OBJ->frame[cpu.x], cpu.a);
        op_lda(0x08);
        WR8(OBJ->work_e[cpu.x], cpu.a);
        op_lda(RD8(OBJ->event_id[cpu.x]));
        cpu.a = f_inc(cpu.a);
        {
            uint8_t index;
            uint8_t byte;
            uint8_t mask;

            jsr_push(0xC347);
            index = cpu.a;
            mask = BitOfIndex(index, cpu.x, &byte);
            bit_of_index_exit_shim(index, mask, byte);
            jsr_pop();
        }
        op_and(RD8(LEVEL->event_flags[cpu.y]));
        if (!flag_z()) {
            op_lda(RD8(OBJ->work_d[cpu.x]));
            WR8(OBJ->y_lo[cpu.x], cpu.a);
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
            return;
        }
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_C378;
    }
    op_lda(0x18);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x15);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC367, 0xAA35);
    if (flag_c()) {
        op_lda(RD8(PVARS->column_hit[0]));
        op_ora(RD8(PVARS->column_hit[1]));
        op_ora(RD8(PVARS->column_hit[2]));
        if (flag_z()) {
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
        }
    }
at_C377:
    return;
at_C378:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_C377;
    }
    RMW8(OBJ->work_e[cpu.x], v, f_dec(v));
    if (!flag_n()) {
        RMW8(OBJ->y_lo[cpu.x], v, f_inc(v));
        op_lda(0x18);
        WR8(ZP->temp08, cpu.a);
        op_lda(0x15);
        WR8(ZP->temp09, cpu.a);
        run_routine(0xAA35);
        return;
    }
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[cpu.y]));
        cpu.a = f_inc(cpu.a);
        WR8(OBJ->state[cpu.y], cpu.a);
    }
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    op_lda(0xC5);
    {
        uint8_t command;

        jsr_push(0xC3A2);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
        jsr_pop();
    }
    op_lda(RD8(OBJ->event_id[cpu.x]));
    cpu.a = f_inc(cpu.a);
    {
        uint8_t event;
        uint8_t byte;
        uint8_t merged;

        event = cpu.a;
        merged = RecordEvent(event, cpu.x, &byte);
        record_event_exit_shim(event, merged, byte);
    }
}

void abi_SwitchPressedByStanding0D5D(void)
{
    SwitchPressedByStanding0D5D();
}

/*
 * @routine 0D5D:3FB FacePlayerInStates0D5D
 * @run 71:C3FB
 * WHAT IT DOES: In the states a table marks, turns the object to face the player (Stage 08's overlay).
 * HOW: A table read by the state, SideOfPlayer's bit 0 into OBJ_FACING_LEFT.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FacePlayerInStates0D5DStates = 0xC40C,   /* nonzero for the states in which it turns */
};

void FacePlayerInStates0D5D(uint8_t slot)
{
    op_ldy(RD8(OBJ->state[slot]));
    op_lda(rd8((uint16_t)(FacePlayerInStates0D5DStates + cpu.y)));
    if (!flag_z()) {
        jsr_push(0xC406);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
    }
}

void abi_FacePlayerInStates0D5D(void)
{
    FacePlayerInStates0D5D(cpu.x);
}

/*
 * @routine 0D5D:47A StartWithKind38Below0D5D
 * @run 71:C47A
 * WHAT IT DOES: Starts the object facing the player with OBJ_WORK_D $40, and an object of kind $38 at its place in
 * the slot below it; with OBJ_START_ARG 2 its sprite attributes are $87 (Stage 08's overlay).
 * HOW: LoadObjectRecord, SideOfPlayer, ClearObjectInY, StartObjectAtPlace, a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithKind38Below0D5D(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC47D);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    jsr_push(0xC480);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xC490);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x38);
    jsr_push(0xC495);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[slot]));
    op_cmp(0x02);
    if (flag_z()) {
        op_lda(0x87);
        WR8(OBJ->sprite_attr[slot], cpu.a);
    }
}

void abi_StartWithKind38Below0D5D(void)
{
    StartWithKind38Below0D5D(cpu.x);
}

/*
 * @routine 0D5D:4C8 WalkBySpeedIndex0D5D
 * @run 71:C4C8
 * WHAT IT DOES: Plays the object's frame list and walks it at the X speed a table gives by OBJ_WORK_F, turning
 * round at walls (BounceAtWalls with probe height 6) (Stage 08's overlay).
 * HOW: StepObjectFrames, a word table read by twice OBJ_WORK_F, BounceAtWalls.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WalkBySpeedIndex0D5DSpeeds = 0xC4EC,   /* the X speed for each OBJ_WORK_F, a word each */
};

void WalkBySpeedIndex0D5D(void)
{
    op_lda(0xC4);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xF0);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC4D3);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->work_f[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(WalkBySpeedIndex0D5DSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(WalkBySpeedIndex0D5DSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(0x06);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xC4EB, 0xAB3A);
}

void abi_WalkBySpeedIndex0D5D(void)
{
    WalkBySpeedIndex0D5D();
}

/*
 * @routine 0D5D:57F PickMoveByPattern0D5D
 * @run 71:C57F
 * WHAT IT DOES: With the object in the first 256 pixels across and the player less than $18 pixels away in Y, takes
 * its substate from a 16-step pattern by OBJ_START_ARG and OBJ_WORK_I; otherwise state 3, or substate 3 every
 * fourth count of OBJ_WORK_I when only the Y distance is too large (Stage 08's overlay).
 * HOW: A test of the X high byte, DistanceYToPlayer, a table read by 16 times OBJ_START_ARG plus OBJ_WORK_I's low
 * four bits.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PickMoveByPattern0D5DSubstates = 0xC5BD,   /* 16 substates for each OBJ_START_ARG */
};

void PickMoveByPattern0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->x_hi[slot]));
    if (flag_z()) {
        jsr_push(0xC587);
        distance_exit_shim(DistanceYToPlayer(slot));
        jsr_pop();
        op_cmp(0x18);
        if (!flag_c()) {
            goto at_C5A1;
        }
        RMW8(OBJ->work_i[slot], v, f_inc(v));
        op_lda(RD8(OBJ->work_i[slot]));
        op_and(0x03);
        if (flag_z()) {
            goto at_C59B;
        }
    }
    op_lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    return;
at_C59B:
    op_lda(0x03);
    WR8(OBJ->substate[slot], cpu.a);
    return;
at_C5A1:
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_i[slot]));
    op_and(0x0F);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveByPattern0D5DSubstates + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    RMW8(OBJ->work_i[slot], v, f_inc(v));
}

void abi_PickMoveByPattern0D5D(void)
{
    PickMoveByPattern0D5D(cpu.x);
}

/*
 * @routine 0D5D:600 RestartInState3_0D5D
 * @run 71:C600
 * WHAT IT DOES: Restarts the object's animation and puts it in state 3, substate 0 (Stage 08's overlay).
 * HOW: Four stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RestartInState3_0D5D(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    op_lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_RestartInState3_0D5D(void)
{
    RestartInState3_0D5D(cpu.x);
}

/*
 * @routine 0D5D:680 WalkByFacing0D5D
 * @run 71:C680
 * WHAT IT DOES: Plays the object's frame list, and when it ends restarts in state 3 (RestartInState3_0D5D);
 * otherwise sets the X speed by its facing and, from the list's step 2 on, walks, turning round at walls
 * (BounceAtWalls with probe height 6) (Stage 08's overlay).
 * HOW: StepObjectFrames, a word table read by twice OBJ_FACING_LEFT, a compare of OBJ_ANIM_STEP, BounceAtWalls.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WalkByFacing0D5DSpeeds = 0xC6C9,   /* the X speed facing right, then left, a word each */
};

void WalkByFacing0D5D(void)
{
    op_lda(0xB0);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xC6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC68B);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RestartInState3_0D5D(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(WalkByFacing0D5DSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(WalkByFacing0D5DSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->anim_step[cpu.x]));
    op_cmp(0x02);
    if (flag_c()) {
        op_lda(0x06);
        WR8(ZP->product.mid, cpu.a);
        call_routine(0xC6AF, 0xAB3A);
    }
}

void abi_WalkByFacing0D5D(void)
{
    WalkByFacing0D5D();
}

/*
 * @routine 0D5D:724 ShootKind9COnCount0D5D
 * @run 71:C724
 * WHAT IT DOES: When OBJ_WORK_D matches one of eight counts, starts an object of kind $9C at the object's place in
 * a free slot from $30, plays sound $15 if on the first screen, and moves it once by the X and Y speeds for that
 * count (Stage 08's overlay).
 * HOW: A search of the table, FindFreeSlotFrom30, StartObjectAtPlace, SoundIfOnFirstScreen, a pair read by twice
 * the match, ApplyVelocity on the new slot.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ShootKind9COnCount0D5DCounts = 0xC75E,   /* the eight counts at which it shoots */
    ShootKind9COnCount0D5DSpeeds = 0xC766,   /* X speed high, Y speed high for each count */
};

void ShootKind9COnCount0D5D(void)
{
    op_cly();
at_C725:
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_cmp(rd8((uint16_t)(ShootKind9COnCount0D5DCounts + cpu.y)));
    if (!flag_z()) {
        op_iny();
        op_cpy(0x08);
        if (!flag_c()) {
            goto at_C725;
        }
        return;
    }
    WR8(ZP->richter_object, cpu.x);
    WR8(ZP->hit_slot, cpu.y);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC73A);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x9C);
        jsr_push(0xC741);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(0x15);
        jsr_push(0xC746);
        sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(RD8(ZP->hit_slot));
        cpu.a = f_asl(cpu.a);
        op_tax();
        op_lda(rd8((uint16_t)(ShootKind9COnCount0D5DSpeeds + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(ShootKind9COnCount0D5DSpeeds + 1 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_sxy();
        jsr_push(0xC75A);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_sxy();
        op_ldx(RD8(ZP->richter_object));
    }
}

void abi_ShootKind9COnCount0D5D(void)
{
    ShootKind9COnCount0D5D();
}

/*
 * @routine 0D5D:7F5 MoveByDirectionIndex0D5D
 * @run 71:C7F5
 * WHAT IT DOES: Sets the object's X and Y speeds from a pair of bytes by OBJ_WORK_E, turning the X speed round when
 * it faces left, and moves it (Stage 08's overlay).
 * HOW: A table read by twice OBJ_WORK_E, NegateVelocityX, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MoveByDirectionIndex0D5DSpeeds = 0xC811,   /* X speed high, Y speed high for each OBJ_WORK_E */
};

void MoveByDirectionIndex0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->work_e[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(MoveByDirectionIndex0D5DSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(MoveByDirectionIndex0D5DSpeeds + 1 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xC80E);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_MoveByDirectionIndex0D5D(void)
{
    MoveByDirectionIndex0D5D(cpu.x);
}

/*
 * @routine 0D5E:017 FlyListOrFall0D5E
 * @run 71:C817
 * WHAT IT DOES: Plays the object's frame list and moves it by its speed; with OBJ_WORK_E 2 it gains $1C of Y speed
 * a frame, otherwise it counts OBJ_WORK_F down and steps its state on at zero (Stage 08's overlay).
 * HOW: StepObjectFrames, ApplyVelocity, a compare, a 16-bit add or DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void FlyListOrFall0D5E(uint8_t slot)
{
    op_lda(0xC8);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x47);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC822);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xC825);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_e[slot]));
    op_cmp(0x02);
    if (flag_z()) {
        op_lda(RD8(OBJ->vy_lo[slot]));
        op_clc();
        op_adc(0x1C);
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(RD8(OBJ->vy_hi[slot]));
        op_adc(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    RMW8(OBJ->work_f[slot], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_FlyListOrFall0D5E(void)
{
    FlyListOrFall0D5E(cpu.x);
}

/*
 * @routine 0D5E:058 SpeedXToTarget0D5E
 * @run 71:C858
 * WHAT IT DOES: Plays the object's frame list, moves it, and adds a step to its X speed until the speed reaches a
 * target, when the state steps on; both come from a record by OBJ_WORK_D (Stage 08's overlay).
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit add and compare of four-byte records by four times OBJ_WORK_D.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SpeedXToTarget0D5ERecords = 0xC890,   /* the X speed's step, then its target, a word each per record */
};

void SpeedXToTarget0D5E(uint8_t slot)
{
    op_lda(0xC8);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x47);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC863);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xC866);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->vx_lo[slot]));
    op_clc();
    op_adc(rd8((uint16_t)(SpeedXToTarget0D5ERecords + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(RD8(OBJ->vx_hi[slot]));
    op_adc(rd8((uint16_t)(SpeedXToTarget0D5ERecords + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_cmp(rd8((uint16_t)(SpeedXToTarget0D5ERecords + 3 + cpu.y)));
    if (flag_z()) {
        op_lda(RD8(OBJ->vx_lo[slot]));
        op_cmp(rd8((uint16_t)(SpeedXToTarget0D5ERecords + 2 + cpu.y)));
        if (flag_z()) {
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_SpeedXToTarget0D5E(void)
{
    SpeedXToTarget0D5E(cpu.x);
}

/*
 * @routine 0D5E:098 FlyWithList0D5E
 * @run 71:C898
 * WHAT IT DOES: Plays the object's frame list and moves it by its speed (Stage 08's overlay).
 * HOW: StepObjectFrames, then ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlyWithList0D5E(void)
{
    op_lda(0xC8);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x47);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC8A3);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_FlyWithList0D5E(void)
{
    FlyWithList0D5E();
}

/*
 * @routine 0D5E:12C ActByStateUnlessFlagged0D5E
 * @run 71:C92C
 * WHAT IT DOES: Sends the object to state 6 when flagged (EnterState6IfFlaggedB), then runs the routine for its
 * state (Stage 08's overlay).
 * HOW: A call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByStateUnlessFlagged0D5EJumps = 0xC941,   /* the routine for each state, a word each */
};

void ActByStateUnlessFlagged0D5E(void)
{
    call_routine(0xC92F, 0xC94F);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByStateUnlessFlagged0D5EJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByStateUnlessFlagged0D5EJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByStateUnlessFlagged0D5E(void)
{
    ActByStateUnlessFlagged0D5E();
}

/*
 * @routine 0D5E:2E3 JumpAfterList0D5E
 * @run 71:CAE3
 * WHAT IT DOES: Faces the player and plays the list at offset 4 (StepFramesFacingPlayerE); when it ends, gives the
 * object the X speed a table picks by OBJ_WORK_E and Y speed -3, restarts its animation, sets OBJ_WORK_D 1 and
 * steps the substate on (Stage 08's overlay).
 * HOW: A call, then a word table read by twice OBJ_WORK_E and stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    JumpAfterList0D5ESpeeds = 0xCB12,   /* the X speed for each OBJ_WORK_E, a word each */
};

void JumpAfterList0D5E(void)
{
    op_ldy(0x04);
    call_routine(0xCAE8, 0xC9A4);
    if (flag_z()) {
        op_lda(RD8(OBJ->work_e[cpu.x]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(JumpAfterList0D5ESpeeds + 1 + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(JumpAfterList0D5ESpeeds + cpu.y)));
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
        op_lda(0xFD);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        WR8(OBJ->vy_lo[cpu.x], 0);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
    }
}

void abi_JumpAfterList0D5E(void)
{
    JumpAfterList0D5E();
}

/*
 * @routine 0D5E:316 LandThenSubstate0D5E
 * @run 71:CB16
 * WHAT IT DOES: Falls and lands with probe height $0A; on landing steps the object's substate on (Stage 08's
 * overlay).
 * HOW: StopOnLanding, then INC on the carry.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenSubstate0D5E(uint8_t slot)
{
    op_lda(0x0A);
    jsr_push(0xCB1B);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandThenSubstate0D5E(void)
{
    LandThenSubstate0D5E(cpu.x);
}

/*
 * @routine 0D5E:321 ListThenState3_0D5E
 * @run 71:CB21
 * WHAT IT DOES: Faces the player and plays the list at offset 6 (StepFramesFacingPlayerE); when it ends, restarts
 * the animation in state 3, substate 0 (Stage 08's overlay).
 * HOW: A call, then four stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ListThenState3_0D5E(void)
{
    op_ldy(0x06);
    call_routine(0xCB26, 0xC9A4);
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_ListThenState3_0D5E(void)
{
    ListThenState3_0D5E();
}

/*
 * @routine 0D5E:337 BurstIntoSixKind42_0D5E
 * @run 71:CB37
 * WHAT IT DOES: Starts up to six objects of kind $42 at the object's place in free slots from $30, each facing as
 * it does with OBJ_WORK_D, OBJ_WORK_E and X and Y speeds from its own record; then clears the object (Stage 08's
 * overlay).
 * HOW: A loop of FindFreeSlotFrom30 and StartObjectAtPlace with the count in zHitSlot and X in zRichterObject;
 * eight-byte records by eight times the count; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BurstIntoSixKind42_0D5ERecords = 0xCB82,   /* OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low, two unused, per piece */
};

void BurstIntoSixKind42_0D5E(void)
{
    op_cly();
at_CB38:
    WR8(ZP->hit_slot, cpu.y);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xCB3D);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x42);
        jsr_push(0xCB44);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        WR8(ZP->richter_object, cpu.x);
        op_lda(RD8(ZP->hit_slot));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_tax();
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + 1 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + 2 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + 3 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + 4 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind42_0D5ERecords + 5 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        op_ldx(RD8(ZP->richter_object));
        op_ldy(RD8(ZP->hit_slot));
        op_iny();
        op_cpy(0x06);
        if (!flag_c()) {
            goto at_CB38;
        }
    }
    ClearObject(cpu.x);
}

void abi_BurstIntoSixKind42_0D5E(void)
{
    BurstIntoSixKind42_0D5E();
}

/*
 * @routine 0D5E:41C FlyArcThenKind9B0D5E
 * @run 71:CC1C
 * WHAT IT DOES: In state 0 starts the object, turning its X speed round when it is left of the player; then counts
 * OBJ_WORK_E down to become kind $9B in state 0, and meanwhile plays the frame list OBJ_WORK_D picks and moves
 * under gravity (Stage 08's overlay).
 * HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX; DEC, a word table read, StepObjectFrames,
 * ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlyArcThenKind9B0D5ELists = 0xCC55,   /* the frame list's address for each OBJ_WORK_D, a word each */
};

void FlyArcThenKind9B0D5E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xCC24);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        jsr_push(0xCC27);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        if (!flag_z()) {
            jsr_push(0xCC2E);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
        return;
    }
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlyArcThenKind9B0D5ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FlyArcThenKind9B0D5ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCC4F);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xCC52);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_FlyArcThenKind9B0D5E(void)
{
    FlyArcThenKind9B0D5E(cpu.x);
}

/*
 * @routine 0D5E:481 State5WhenStruck0D5E
 * @run 71:CC81
 * WHAT IT DOES: When a player attack hits this frame (HitAttackFlags bits 0 and 1) or the object has been hit 5
 * times, and it is in a state below 5, plays sound $16 if on the first screen, turns its hit box off and restarts
 * it in state 5, substate 0 with OBJ_WORK_C 0 (Stage 08's overlay).
 * HOW: Two tests, a compare of the state, SoundIfOnFirstScreen, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void State5WhenStruck0D5E(uint8_t slot)
{
    op_lda(rd8((uint16_t)(HitAttackFlags)));
    op_and(0x03);
    if (flag_z()) {
        op_lda(RD8(OBJ->hit_taken[slot]));
        op_cmp(0x05);
        if (!flag_c()) {
            goto at_CCAF;
        }
    }
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x05);
    if (!flag_c()) {
        op_lda(0x16);
        jsr_push(0xCC9B);
        sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, slot, cpu.y));
        jsr_pop();
        WR8(OBJ->hitbox_on[slot], 0);
        WR8(OBJ->work_c[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
at_CCAF:
    ;
}

void abi_State5WhenStruck0D5E(void)
{
    State5WhenStruck0D5E(cpu.x);
}

/*
 * @routine 0D5E:4C9 WakeWhenPlayerNear0D5E
 * @run 71:CCC9
 * WHAT IT DOES: Falls and lands with probe height 8; once the player is within $60 pixels along X and $20 along Y,
 * steps the state on and, unless OBJ_START_ARG bit 4 is set, plays sound $17 if on the first screen (Stage 08's
 * overlay).
 * HOW: StopOnLanding, DistanceXToPlayer and DistanceYToPlayer with compares, SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void WakeWhenPlayerNear0D5E(uint8_t slot)
{
    op_lda(0x08);
    jsr_push(0xCCCE);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    jsr_push(0xCCD1);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x60);
    if (!flag_c()) {
        jsr_push(0xCCD8);
        distance_exit_shim(DistanceYToPlayer(slot));
        jsr_pop();
        op_cmp(0x20);
        if (!flag_c()) {
            RMW8(OBJ->state[slot], v, f_inc(v));
            op_lda(RD8(OBJ->start_arg[slot]));
            op_and(0x10);
            if (flag_z()) {
                op_lda(0x17);
                jsr_push(0xCCEB);
                sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, slot, cpu.y));
                jsr_pop();
            }
        }
    }
}

void abi_WakeWhenPlayerNear0D5E(void)
{
    WakeWhenPlayerNear0D5E(cpu.x);
}

/*
 * @routine 0D5E:560 WalkOnFloorOrFall0D5E
 * @run 71:CD60
 * WHAT IT DOES: With floor under the object plays its frame list and, when the frame changes, sets the X speed from
 * a table by OBJ_START_ARG's low four bits and OBJ_WORK_F and walks, turning round at walls; with no floor stops
 * along X and falls with probe height 8 (Stage 08's overlay).
 * HOW: ProbeTerrain request 9, StepObjectFrames, a word table read by four times the argument plus twice
 * OBJ_WORK_F, BounceAtWalls; or StopOnLanding.
 * WAITS: no.
 * Entry: X the object; Y the probe's column. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WalkOnFloorOrFall0D5ESpeeds = 0xCDAB,   /* two X speeds, a word each, for each OBJ_START_ARG */
};

void WalkOnFloorOrFall0D5E(void)
{
    op_lda(0x08);
    WR8(ZP->product.mid, cpu.a);
    op_lda(0x09);
    jsr_push(0xCD69);
    probe_exit_shim(ProbeTerrain(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x01);
    if (!flag_z()) {
        op_lda(0xB3);
        WR8(ZP->temp10, cpu.a);
        op_lda(0xCD);
        WR8(ZP->temp11, cpu.a);
        {
            uint8_t y;
            FrameStep step;

            jsr_push(0xCD78);
            y = cpu.y;
            step = StepObjectFrames(cpu.x, &y);
            step_frames_exit_shim(step, y);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(RD8(OBJ->start_arg[cpu.x]));
            op_and(0x0F);
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            WR8(ZP->temp00, cpu.a);
            op_lda(RD8(OBJ->work_f[cpu.x]));
            cpu.a = f_asl(cpu.a);
            op_clc();
            op_adc(RD8(ZP->temp00));
            op_tay();
            op_lda(rd8((uint16_t)(WalkOnFloorOrFall0D5ESpeeds + 1 + cpu.y)));
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(WalkOnFloorOrFall0D5ESpeeds + cpu.y)));
            WR8(OBJ->vx_lo[cpu.x], cpu.a);
            op_lda(0x08);
            WR8(ZP->product.mid, cpu.a);
            call_routine(0xCD9E, 0xAB3A);
        }
        return;
    }
    WR8(OBJ->vx_hi[cpu.x], 0);
    WR8(OBJ->vx_lo[cpu.x], 0);
    op_lda(0x08);
    jsr_push(0xCDAA);
    mover_exit_shim(StopOnLanding(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
}

void abi_WalkOnFloorOrFall0D5E(void)
{
    WalkOnFloorOrFall0D5E();
}

/*
 * @routine 0D5E:601 ListThenArm0D5E
 * @run 71:CE01
 * WHAT IT DOES: Sets the object's hit box from its frame (SetHitboxFromFrameRowsB). In substate 0 plays the frame
 * list OBJ_START_ARG's low four bits pick; when it ends, unless a player attack with HitAttackFlags bit 0 hits this
 * frame, plays sound $17 if on the first screen, turns the hit box on (2) and steps the substate on. Later
 * substates run ReverseListWhenStruck0D5E (Stage 08's overlay).
 * HOW: A call, a word table read, StepObjectFrames and tests.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ListThenArm0D5ELists = 0xCE34,   /* the frame list's address for each OBJ_START_ARG, a word each */
};

void ListThenArm0D5E(void)
{
    call_routine(0xCE04, 0xCEEE);
    op_lda(RD8(OBJ->substate[cpu.x]));
    if (!flag_z()) {
        ReverseListWhenStruck0D5E(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_and(0x0F);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ListThenArm0D5ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(ListThenArm0D5ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCE1D);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (!flag_z()) {
        ReturnOnly0D5E();
        return;
    }
    op_lda(rd8((uint16_t)(HitAttackFlags)));
    op_and(0x01);
    if (!flag_z()) {
        ReturnOnly0D5E();
        return;
    }
    op_lda(0x17);
    jsr_push(0xCE2B);
    sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_lda(0x02);
    WR8(OBJ->hitbox_on[cpu.x], cpu.a);
    RMW8(OBJ->substate[cpu.x], v, f_inc(v));
    /* falls into $CE33 */
    ReturnOnly0D5E();
}

void abi_ListThenArm0D5E(void)
{
    ListThenArm0D5E();
}

/*
 * @routine 0D5E:633 ReturnOnly0D5E
 * @run 71:CE33
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D5E(void)
{
}

void abi_ReturnOnly0D5E(void)
{
    ReturnOnly0D5E();
}

/*
 * @routine 0D5E:66A ReverseListWhenStruck0D5E
 * @run 71:CE6A
 * WHAT IT DOES: Plays the frame list OBJ_START_ARG's low four bits pick; when it ends runs
 * EnterState3WithTallHitbox. When a player attack hits this frame (HitAttackFlags bits 0 and 1) or it has been hit
 * 5 times, plays sound $16 if on the first screen, sets its list step to $0A less the step, turns the hit box off
 * and goes back to substate 0 (Stage 08's overlay).
 * HOW: A word table read, StepObjectFrames, tests, SoundIfOnFirstScreen, a subtract and stores.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ReverseListWhenStruck0D5ELists = 0xCE38,   /* the frame list's address for each OBJ_START_ARG, a word each */
};

void ReverseListWhenStruck0D5E(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    op_and(0x0F);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ReverseListWhenStruck0D5ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(ReverseListWhenStruck0D5ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCE7E);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        run_routine(0xCEA6);
        return;
    }
    op_lda(rd8((uint16_t)(HitAttackFlags)));
    op_and(0x03);
    if (flag_z()) {
        op_lda(RD8(OBJ->hit_taken[slot]));
        op_cmp(0x05);
        if (!flag_c()) {
            ReturnOnly0D5E();
            return;
        }
    }
    op_lda(0x16);
    jsr_push(0xCE93);
    sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0x0A);
    op_sec();
    op_sbc(RD8(OBJ->anim_step[slot]));
    WR8(OBJ->anim_step[slot], cpu.a);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->substate[slot], 0);
}

void abi_ReverseListWhenStruck0D5E(void)
{
    ReverseListWhenStruck0D5E(cpu.x);
}

/*
 * @routine 0D5E:77E StartOrRoamNearPlayer0D5E
 * @run 71:CF7E
 * WHAT IT DOES: In state 0 starts the object, at X $0080, Y -$40 unless OBJ_START_ARG is negative, with OBJ_WORK_D
 * $40; clears $40 bytes of four speed buffers from a place by OBJ_START_ARG, and starts seven objects of kind $50
 * at its place in the slots after it, each with its index 0 to 6 in OBJ_WORK_D and the object's slot in OBJ_WORK_E.
 * Later it takes the frame $80 to $87 of its angle OBJ_WORK_D, moves at speed $100 at that angle, and turns toward
 * a point near the player (TurnTowardPlayerOffset0D5F) chosen at random from eight for a random $40 to $5F frames
 * (Stage 08's overlay).
 * HOW: LoadObjectRecord, a clearing loop, a loop of ClearObjectInY and StartObjectAtPlace; NextRandom twice,
 * SetVelocityFromAngle, ApplyVelocity, then falls into the next routine.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StartOrRoamNearPlayer0D5ESpeedXHi = 0xB2F4,   /* a buffer of X speed high bytes */
    StartOrRoamNearPlayer0D5ESpeedXLo = 0xB374,   /* a buffer of X speed low bytes */
    StartOrRoamNearPlayer0D5ESpeedYHi = 0xB3F4,   /* a buffer of Y speed high bytes */
    StartOrRoamNearPlayer0D5ESpeedYLo = 0xB474,   /* a buffer of Y speed low bytes */
    StartOrRoamNearPlayer0D5EBases = 0xCF7C,   /* the buffers' index, plus $3F, for each OBJ_START_ARG */
};

void StartOrRoamNearPlayer0D5E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xCF86);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        if (!flag_n()) {
            WR8(OBJ->x_hi[slot], 0);
            op_lda(0x80);
            WR8(OBJ->x_lo[slot], cpu.a);
            op_lda(0xFF);
            WR8(OBJ->y_hi[slot], cpu.a);
            op_lda(0xC0);
            WR8(OBJ->y_lo[slot], cpu.a);
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        op_and(0x7F);
        WR8(OBJ->start_arg[slot], cpu.a);
        op_lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        op_lda(0x40);
        WR8(ZP->temp00, cpu.a);
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(rd8((uint16_t)(StartOrRoamNearPlayer0D5EBases + cpu.y)));
        op_sec();
        op_sbc(0x3F);
        op_tay();
        op_cla();
        do {
            wr8((uint16_t)(StartOrRoamNearPlayer0D5ESpeedXHi + cpu.y), cpu.a);
            wr8((uint16_t)(StartOrRoamNearPlayer0D5ESpeedXLo + cpu.y), cpu.a);
            wr8((uint16_t)(StartOrRoamNearPlayer0D5ESpeedYHi + cpu.y), cpu.a);
            wr8((uint16_t)(StartOrRoamNearPlayer0D5ESpeedYLo + cpu.y), cpu.a);
            op_iny();
            RMW8(ZP->temp00, v, f_dec(v));
        } while (!flag_z());
        op_txa();
        op_tay();
        op_iny();
        op_lda(0x07);
        WR8(ZP->temp00, cpu.a);
        WR8(ZP->temp01, 0);
        do {
            jsr_push(0xCFD6);
            ClearObjectInY(cpu.y);
            jsr_pop();
            op_lda(0x50);
            jsr_push(0xCFDB);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_lda(RD8(ZP->temp01));
            WR8(OBJ->work_d[cpu.y], cpu.a);
            op_txa();
            WR8(OBJ->work_e[cpu.y], cpu.a);
            op_iny();
            RMW8(ZP->temp01, v, f_inc(v));
            RMW8(ZP->temp00, v, f_dec(v));
        } while (!flag_z());
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_clc();
    op_adc(0x80);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_h[slot]));
    if (flag_z()) {
        jsr_push(0xD002);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x1F);
        op_clc();
        op_adc(0x40);
        WR8(OBJ->work_h[slot], cpu.a);
        jsr_push(0xD00D);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x07);
        WR8(OBJ->work_g[slot], cpu.a);
    }
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    op_lda(0x00);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xD023);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xD026);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_g[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    /* falls into $D02B */
    TurnTowardPlayerOffset0D5F(slot);
}

void abi_StartOrRoamNearPlayer0D5E(void)
{
    StartOrRoamNearPlayer0D5E(cpu.x);
}

/*
 * @routine 0D5F:02B TurnTowardPlayerOffset0D5F
 * @run 71:D02B
 * WHAT IT DOES: Turns the object's angle OBJ_WORK_D by 1 toward the player's place plus an offset from a table
 * (Stage 08's overlay).
 * HOW: Two 16-bit adds into zTemp08-0B, AngleToPoint, TurnAngleToward.
 * WAITS: no.
 * Entry: X the object; A four times the offset's index. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
enum {
    TurnTowardPlayerOffset0D5FOffsets = 0xD063,   /* an X offset and a Y offset, a word each, per entry */
};

void TurnTowardPlayerOffset0D5F(uint8_t slot)
{
    op_tay();
    op_lda(RD8(OBJ->x_lo[0x00]));
    op_clc();
    op_adc(rd8((uint16_t)(TurnTowardPlayerOffset0D5FOffsets + cpu.y)));
    WR8(ZP->temp08, cpu.a);
    op_lda(RD8(OBJ->x_hi[0x00]));
    op_adc(rd8((uint16_t)(TurnTowardPlayerOffset0D5FOffsets + 1 + cpu.y)));
    WR8(ZP->temp09, cpu.a);
    op_lda(RD8(OBJ->y_lo[0x00]));
    op_clc();
    op_adc(rd8((uint16_t)(TurnTowardPlayerOffset0D5FOffsets + 2 + cpu.y)));
    WR8(ZP->temp0a, cpu.a);
    op_lda(RD8(OBJ->y_hi[0x00]));
    op_adc(rd8((uint16_t)(TurnTowardPlayerOffset0D5FOffsets + 3 + cpu.y)));
    WR8(ZP->temp0b, cpu.a);
    jsr_push(0xD051);
    angle_exit_shim(AngleToPoint(slot));
    jsr_pop();
    WR8(ZP->temp02, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xD05F);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_TurnTowardPlayerOffset0D5F(void)
{
    TurnTowardPlayerOffset0D5F(cpu.x);
}

/*
 * @routine 0D5F:09B MoveByOwnerBuffers0D5F
 * @run 71:D09B
 * WHAT IT DOES: While the slot in OBJ_WORK_E holds kind $4F, moves the object by the X and Y speeds kept in four
 * buffers at a place by that slot's OBJ_START_ARG and its own OBJ_WORK_D; otherwise sets OBJ_WORK_F to four times
 * OBJ_WORK_D plus $10, turns its hit box off and steps its state on (Stage 08's overlay).
 * HOW: A kind compare, table reads for the index, four buffer reads, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    MoveByOwnerBuffers0D5FSpeedXHi = 0xB2F4,   /* a buffer of X speed high bytes */
    MoveByOwnerBuffers0D5FSpeedXLo = 0xB374,   /* a buffer of X speed low bytes */
    MoveByOwnerBuffers0D5FSpeedYHi = 0xB3F4,   /* a buffer of Y speed high bytes */
    MoveByOwnerBuffers0D5FSpeedYLo = 0xB474,   /* a buffer of Y speed low bytes */
    MoveByOwnerBuffers0D5FBases = 0xCF7C,   /* the buffers' index, plus $3F, for each OBJ_START_ARG */
    MoveByOwnerBuffers0D5FDelays = 0xD0E8,   /* the buffer offset for each OBJ_WORK_D */
};

void MoveByOwnerBuffers0D5F(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_e[slot]));
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_cmp(0x4F);
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_d[slot]));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_clc();
        op_adc(0x10);
        WR8(OBJ->work_f[slot], cpu.a);
        WR8(OBJ->hitbox_on[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    op_lda(RD8(OBJ->start_arg[cpu.y]));
    op_tay();
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FBases + cpu.y)));
    op_sec();
    op_sbc(0x3F);
    WR8(ZP->temp00, cpu.a);
    op_ldy(RD8(OBJ->work_d[slot]));
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FDelays + cpu.y)));
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FSpeedXHi + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FSpeedXLo + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FSpeedYHi + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(MoveByOwnerBuffers0D5FSpeedYLo + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_MoveByOwnerBuffers0D5F(void)
{
    MoveByOwnerBuffers0D5F(cpu.x);
}

/*
 * @routine 0D5F:0EF CountThenKind9B0D5F
 * @run 71:D0EF
 * WHAT IT DOES: Counts OBJ_WORK_F down; at zero plays sound $11 and turns the object into kind $9B in state 0
 * (Stage 08's overlay).
 * HOW: DEC, SoundCommand, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountThenKind9B0D5F(uint8_t slot)
{
    RMW8(OBJ->work_f[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x11);
        {
            uint8_t command;

            jsr_push(0xD0F9);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
    }
}

void abi_CountThenKind9B0D5F(void)
{
    CountThenKind9B0D5F(cpu.x);
}

/*
 * @routine 0D5F:102 SpawnKind4FOnTimer0D5F
 * @run 71:D102
 * WHAT IT DOES: In state 0 starts the object; then counts the 16-bit OBJ_WORK_D-E down. At zero, starts an object
 * of kind $4F at its place in slot $20, OBJ_START_ARG 0, when slots $20 and $27 are empty, or with OBJ_START_ARG 0
 * in slot $28, argument 1, when $28 and $2F are; then waits again by a table. With no group free it waits a shorter
 * time by another table (Stage 08's overlay).
 * HOW: A 16-bit DEC, ORs of two kinds, ClearObjectInY and StartObjectAtPlace, word tables by twice OBJ_START_ARG.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SpawnKind4FOnTimer0D5FWaits = 0xD172,   /* the wait after a start, OBJ_WORK_E then OBJ_WORK_D, per OBJ_START_ARG */
    SpawnKind4FOnTimer0D5FRetries = 0xD176,   /* the wait when no group of slots is free, the same layout */
};

void SpawnKind4FOnTimer0D5F(uint8_t slot)
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
    WR8(ZP->temp00, 0);
    op_ldy(0x20);
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_ora(RD8(OBJ->kind[cpu.y + 7]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->start_arg[slot]));
        if (flag_z()) {
            RMW8(ZP->temp00, v, f_inc(v));
            op_ldy(0x28);
            op_lda(RD8(OBJ->kind[cpu.y]));
            op_ora(RD8(OBJ->kind[cpu.y + 7]));
            if (flag_z()) {
                goto at_D153;
            }
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(SpawnKind4FOnTimer0D5FRetries + cpu.y)));
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(rd8((uint16_t)(SpawnKind4FOnTimer0D5FRetries + 1 + cpu.y)));
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
at_D153:
    jsr_push(0xD156);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x4F);
    jsr_push(0xD15B);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(RD8(ZP->temp00));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(SpawnKind4FOnTimer0D5FWaits + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(rd8((uint16_t)(SpawnKind4FOnTimer0D5FWaits + 1 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SpawnKind4FOnTimer0D5F(void)
{
    SpawnKind4FOnTimer0D5F(cpu.x);
}

/*
 * @routine 0D5F:1FD FallWithLowerProbe0D5F
 * @run 71:D1FD
 * WHAT IT DOES: Outside state 0, and while its Y high byte is 0, the object falls under gravity and stops on the
 * floor (probe request 9, height $0C) with its Y taken $10 times OBJ_START_ARG lower for the probe (Stage 08's
 * overlay).
 * HOW: A 16-bit add to Y, ApplyVelocityY, ApplyGravity, ProbeTerrain, the same subtract back.
 * WAITS: no.
 * Entry: X the object; Y the probe's column. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallWithLowerProbe0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->y_hi[slot]));
        if (flag_z()) {
            op_lda(RD8(OBJ->start_arg[slot]));
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            WR8(ZP->temp00, cpu.a);
            op_lda(RD8(OBJ->y_lo[slot]));
            op_clc();
            op_adc(RD8(ZP->temp00));
            WR8(OBJ->y_lo[slot], cpu.a);
            op_lda(RD8(OBJ->y_hi[slot]));
            op_adc(0x00);
            WR8(OBJ->y_hi[slot], cpu.a);
            jsr_push(0xD224);
            axis_move_exit_shim(ApplyVelocityY(slot));
            jsr_pop();
            jsr_push(0xD227);
            gravity_exit_shim(ApplyGravity(slot));
            jsr_pop();
            op_lda(0x0C);
            WR8(ZP->product.mid, cpu.a);
            op_lda(0x09);
            jsr_push(0xD230);
            probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
            jsr_pop();
            op_and(0x01);
            if (!flag_z()) {
                WR8(OBJ->vy_hi[slot], 0);
                WR8(OBJ->vy_lo[slot], 0);
            }
            op_lda(RD8(OBJ->start_arg[slot]));
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            cpu.a = f_asl(cpu.a);
            WR8(ZP->temp00, cpu.a);
            op_lda(RD8(OBJ->y_lo[slot]));
            op_sec();
            op_sbc(RD8(ZP->temp00));
            WR8(OBJ->y_lo[slot], cpu.a);
            op_lda(RD8(OBJ->y_hi[slot]));
            op_sbc(0x00);
            WR8(OBJ->y_hi[slot], cpu.a);
        }
    }
}

void abi_FallWithLowerProbe0D5F(void)
{
    FallWithLowerProbe0D5F(cpu.x);
}

/*
 * @routine 0D5F:2CB ShiftOnceByArg0D5F
 * @run 71:D2CB
 * WHAT IT DOES: Moves the object once by an X step a table gives by OBJ_START_ARG bit 0 and up by 16 times
 * OBJ_START_ARG, then zeroes those speeds (Stage 08's overlay).
 * HOW: A table read, a negated shift, ApplyVelocity, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ShiftOnceByArg0D5FSteps = 0xD2EE,   /* the X speed's high byte for OBJ_START_ARG bit 0 clear, then set */
};

void ShiftOnceByArg0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(ShiftOnceByArg0D5FSteps + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_eor(0xFF);
    cpu.a = f_inc(cpu.a);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xD2E7);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_ShiftOnceByArg0D5F(void)
{
    ShiftOnceByArg0D5F(cpu.x);
}

/*
 * @routine 0D5F:2F0 RandomWaitDE0D5F
 * @run 71:D2F0
 * WHAT IT DOES: Sets OBJ_WORK_E and OBJ_WORK_D from one of four words chosen at random (Stage 08's overlay).
 * HOW: NextRandom's low two bits index a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RandomWaitDE0D5FWords = 0xD304,   /* four words: OBJ_WORK_E, then OBJ_WORK_D */
};

void RandomWaitDE0D5F(uint8_t slot)
{
    jsr_push(0xD2F3);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RandomWaitDE0D5FWords + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(rd8((uint16_t)(RandomWaitDE0D5FWords + 1 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_RandomWaitDE0D5F(void)
{
    RandomWaitDE0D5F(cpu.x);
}

/*
 * @routine 0D5F:30C RandomWordFG0D5F
 * @run 71:D30C
 * WHAT IT DOES: Sets OBJ_WORK_G and OBJ_WORK_F from one of four words chosen at random (Stage 08's overlay).
 * HOW: NextRandom's low two bits index a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RandomWordFG0D5FWords = 0xD320,   /* four words: OBJ_WORK_G, then OBJ_WORK_F */
};

void RandomWordFG0D5F(uint8_t slot)
{
    jsr_push(0xD30F);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RandomWordFG0D5FWords + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(rd8((uint16_t)(RandomWordFG0D5FWords + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_RandomWordFG0D5F(void)
{
    RandomWordFG0D5F(cpu.x);
}

/*
 * @routine 0D5F:389 AlternateListThenState1_0D5F
 * @run 71:D389
 * WHAT IT DOES: Plays one of two frame lists by OBJ_WORK_H bit 0; when it ends, steps OBJ_WORK_H on and puts the
 * object in state 1 (Stage 08's overlay).
 * HOW: A word table read, StepObjectFrames, INC and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AlternateListThenState1_0D5FLists = 0xD3A8,   /* the two frame lists' addresses, a word each */
};

void AlternateListThenState1_0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(AlternateListThenState1_0D5FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(AlternateListThenState1_0D5FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD39D);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->work_h[slot], v, f_inc(v));
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_AlternateListThenState1_0D5F(void)
{
    AlternateListThenState1_0D5F(cpu.x);
}

/*
 * @routine 0D5F:3D8 PickMoveWhenFacing0D5F
 * @run 71:D3D8
 * WHAT IT DOES: When the object's facing, flipped by OBJ_WORK_H bit 0, differs from its side of the player, goes to
 * state 1; otherwise picks OBJ_WORK_I from 16 at random, OBJ_WORK_K from four for that choice at random, OBJ_WORK_J
 * by OBJ_WORK_H bit 0, and steps the substate on (Stage 08's overlay).
 * HOW: An EOR and SideOfPlayer, NextRandom twice and three table reads.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PickMoveWhenFacing0D5FChoices = 0xD420,   /* 16 values of OBJ_WORK_I */
    PickMoveWhenFacing0D5FWorkK = 0xD430,   /* four values of OBJ_WORK_K for each OBJ_WORK_I */
    PickMoveWhenFacing0D5FWorkJ = 0xD43C,   /* OBJ_WORK_J for OBJ_WORK_H bit 0 clear, then set */
};

void PickMoveWhenFacing0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->facing_left[slot]));
    op_eor(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    WR8(ZP->temp08, cpu.a);
    jsr_push(0xD3E5);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    op_cmp(RD8(ZP->temp08));
    if (!flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    jsr_push(0xD3F4);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveWhenFacing0D5FChoices + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    jsr_push(0xD404);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveWhenFacing0D5FWorkK + cpu.y)));
    WR8(OBJ->work_k[slot], cpu.a);
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveWhenFacing0D5FWorkJ + cpu.y)));
    WR8(OBJ->work_j[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_PickMoveWhenFacing0D5F(void)
{
    PickMoveWhenFacing0D5F(cpu.x);
}

/*
 * @routine 0D5F:43E StartChildByWorkI0D5F
 * @run 71:D43E
 * WHAT IT DOES: Starts an object of the kind a table gives by OBJ_WORK_I at the object's place in a free slot from
 * $30, with OBJ_WORK_I in its OBJ_WORK_D, the object's facing and its slot in OBJ_WORK_F, and X speed $F8, or $EE
 * facing the other way when OBJ_WORK_J is set; steps the substate on. With no free slot runs BackToState1Frame0D5F
 * (Stage 08's overlay).
 * HOW: FindFreeSlotFrom30, a table read, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartChildByWorkI0D5FKinds = 0xD480,   /* the kind for each OBJ_WORK_I */
};

void StartChildByWorkI0D5F(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD441);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        BackToState1Frame0D5F(slot);
        return;
    }
    WR8(ZP->hit_slot, cpu.y);
    op_ldy(RD8(OBJ->work_i[slot]));
    op_lda(rd8((uint16_t)(StartChildByWorkI0D5FKinds + cpu.y)));
    op_ldy(RD8(ZP->hit_slot));
    jsr_push(0xD453);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_d[cpu.y], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    op_txa();
    WR8(OBJ->work_f[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (flag_z()) {
        op_lda(0xF8);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
    } else {
        op_lda(0xEE);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(RD8(OBJ->facing_left[cpu.y]));
        op_eor(0x01);
        WR8(OBJ->facing_left[cpu.y], cpu.a);
    }
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_StartChildByWorkI0D5F(void)
{
    StartChildByWorkI0D5F(cpu.x);
}

/*
 * @routine 0D5F:483 PlayChosenList0D5F
 * @run 71:D483
 * WHAT IT DOES: Plays the frame list at twice OBJ_WORK_I plus OBJ_WORK_J in a table; when it ends steps the
 * substate on (Stage 08's overlay).
 * HOW: A word table read, StepObjectFrames, INC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PlayChosenList0D5FLists = 0xD49F,   /* frame lists' addresses, a word each */
};

void PlayChosenList0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_j[slot]));
    op_tay();
    op_lda(rd8((uint16_t)(PlayChosenList0D5FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(PlayChosenList0D5FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD499);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_PlayChosenList0D5F(void)
{
    PlayChosenList0D5F(cpu.x);
}

/*
 * @routine 0D5F:4D0 BackToState1Frame0D5F
 * @run 71:D4D0
 * WHAT IT DOES: Puts the object in state 1 with frame $8A, or $8D when OBJ_WORK_J is set (Stage 08's overlay).
 * HOW: Stores and a test.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackToState1Frame0D5F(uint8_t slot)
{
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    op_lda(0x8A);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(0x8D);
        WR8(OBJ->frame[slot], cpu.a);
    }
}

void abi_BackToState1Frame0D5F(void)
{
    BackToState1Frame0D5F(cpu.x);
}

/*
 * @routine 0D5F:4E5 RenumberKind52ThenClear0D5F
 * @run 71:D4E5
 * WHAT IT DOES: Plays one of two frame lists by OBJ_WORK_H bit 0; when it ends, numbers the objects of kind $52 in
 * a state below 4 from slot OBJ_WORK_M to OBJ_WORK_N in their OBJ_START_ARG, gives each the first one's slot in
 * OBJ_WORK_M, and clears the object (Stage 08's overlay).
 * HOW: A word table read, StepObjectFrames, a loop over the slots with the count in zTemp00, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RenumberKind52ThenClear0D5FLists = 0xD530,   /* the two frame lists' addresses, a word each */
};

void RenumberKind52ThenClear0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RenumberKind52ThenClear0D5FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(RenumberKind52ThenClear0D5FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD4F9);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_ldy(RD8(OBJ->work_m[slot]));
        WR8(ZP->temp00, 0);
        op_lda(RD8(OBJ->work_n[slot]));
        WR8(ZP->temp01, cpu.a);
        RMW8(ZP->temp01, v, f_inc(v));
        do {
            op_lda(RD8(OBJ->kind[cpu.y]));
            op_cmp(0x52);
            if (flag_z()) {
                op_lda(RD8(OBJ->state[cpu.y]));
                op_cmp(0x04);
                if (!flag_c()) {
                    op_lda(RD8(ZP->temp00));
                    if (flag_z()) {
                        WR8(ZP->temp03, cpu.y);
                    }
                    op_lda(RD8(ZP->temp00));
                    WR8(OBJ->start_arg[cpu.y], cpu.a);
                    op_lda(RD8(ZP->temp03));
                    WR8(OBJ->work_m[cpu.y], cpu.a);
                    RMW8(ZP->temp00, v, f_inc(v));
                }
            }
            op_iny();
            op_cpy(RD8(ZP->temp01));
        } while (!flag_c());
        jsr_push(0xD52F);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_RenumberKind52ThenClear0D5F(void)
{
    RenumberKind52ThenClear0D5F(cpu.x);
}

/*
 * @routine 0D5F:562 StartWithKind55AndSpeed0D5F
 * @run 71:D562
 * WHAT IT DOES: Starts the object with OBJ_WORK_E $28, moves it once along X (its speed turned round when facing
 * left), sets its X and Y speeds from a record by OBJ_WORK_D and its facing, and starts an object of kind $55 at
 * its place in a free slot from $50 with its OBJ_WORK_F (Stage 08's overlay).
 * HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, a read of four-byte records by eight times OBJ_WORK_D
 * plus four times the facing, FindFreeSlotFrom50, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StartWithKind55AndSpeed0D5FSpeeds = 0xD5AF,   /* X speed, then Y speed, a word each, by OBJ_WORK_D and facing */
};

void StartWithKind55AndSpeed0D5F(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD565);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x28);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD572);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xD575);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(StartWithKind55AndSpeed0D5FSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(StartWithKind55AndSpeed0D5FSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(StartWithKind55AndSpeed0D5FSpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(StartWithKind55AndSpeed0D5FSpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD5A1);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x55);
        jsr_push(0xD5A8);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
}

void abi_StartWithKind55AndSpeed0D5F(void)
{
    StartWithKind55AndSpeed0D5F(cpu.x);
}

/*
 * @routine 0D5F:5BF CountThenArm0D5F
 * @run 71:D5BF
 * WHAT IT DOES: Counts OBJ_WORK_E down; at zero plays sound $25, turns the hit box on (2), restarts the animation
 * and steps the state on; then keeps the Y of the slot in OBJ_WORK_F (KeepYOfWorkFSlot0D5F) (Stage 08's overlay).
 * HOW: DEC, SoundCommand, stores, then the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountThenArm0D5F(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x25);
        {
            uint8_t command;

            jsr_push(0xD5C9);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
    KeepYOfWorkFSlot0D5F(slot);
}

void abi_CountThenArm0D5F(void)
{
    CountThenArm0D5F(cpu.x);
}

/*
 * @routine 0D5F:5DA KeepYOfWorkFSlot0D5F
 * @run 71:D5DA
 * WHAT IT DOES: Puts the object at the Y of the slot in OBJ_WORK_F, and clears it once that slot is in state 4
 * (Stage 08's overlay).
 * HOW: A copy of the Y low byte, a compare, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void KeepYOfWorkFSlot0D5F(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_f[slot]));
    op_lda(RD8(OBJ->y_lo[cpu.y]));
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x04);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
}

void abi_KeepYOfWorkFSlot0D5F(void)
{
    KeepYOfWorkFSlot0D5F(cpu.x);
}

/*
 * @routine 0D5F:5EE FlyListByWorkD0D5F
 * @run 71:D5EE
 * WHAT IT DOES: Plays the frame list OBJ_WORK_D picks and moves the object by its speed; with OBJ_WORK_D 1 it gains
 * $18 of Y speed a frame (Stage 08's overlay).
 * HOW: A word table read, StepObjectFrames, ApplyVelocity, a 16-bit add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FlyListByWorkD0D5FLists = 0xD61C,   /* the frame list's address for each OBJ_WORK_D, a word each */
};

void FlyListByWorkD0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlyListByWorkD0D5FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FlyListByWorkD0D5FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD600);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xD603);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        op_lda(RD8(OBJ->vy_lo[slot]));
        op_clc();
        op_adc(0x18);
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(RD8(OBJ->vy_hi[slot]));
        op_adc(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
    }
}

void abi_FlyListByWorkD0D5F(void)
{
    FlyListByWorkD0D5F(cpu.x);
}

/*
 * @routine 0D5F:648 StartWithKind55Step0D5F
 * @run 71:D648
 * WHAT IT DOES: Starts the object with OBJ_WORK_E $40, moves it once along X (its speed turned round when facing
 * left), starts an object of kind $55 at its place in a free slot from $50 with OBJ_WORK_D 1 and its OBJ_WORK_F,
 * then moves it once more by an X step for its facing (Stage 08's overlay).
 * HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace, a table read by
 * the facing, ApplyVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartWithKind55Step0D5FSteps = 0xD67D,   /* the X speed's high byte facing right, then left */
};

void StartWithKind55Step0D5F(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD64B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD658);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xD65B);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD65E);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x55);
        jsr_push(0xD665);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(StartWithKind55Step0D5FSteps + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    jsr_push(0xD67C);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

void abi_StartWithKind55Step0D5F(void)
{
    StartWithKind55Step0D5F(cpu.x);
}

/*
 * @routine 0D5F:697 AnimateHitBoxForCount0D5F
 * @run 71:D697
 * WHAT IT DOES: Plays the object's frame list, gives it its frame's hit box (HitBoxByFrame0D5F), and counts
 * OBJ_WORK_E down, clearing the object at zero (Stage 08's overlay).
 * HOW: StepObjectFrames, a call, DEC, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AnimateHitBoxForCount0D5F(void)
{
    op_lda(0xD6);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xAE);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD6A2);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    call_routine(0xD6A5, 0xD6D1);
    RMW8(OBJ->work_e[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
}

void abi_AnimateHitBoxForCount0D5F(void)
{
    AnimateHitBoxForCount0D5F();
}

/*
 * @routine 0D5F:70F AnimateFollowingYForCount0D5F
 * @run 71:D70F
 * WHAT IT DOES: In state 0 starts the object with sprite set OBJ_WORK_D plus 3 and OBJ_WORK_E $28; then plays the
 * frame list OBJ_WORK_D picks, counts OBJ_WORK_E down to clear it, and keeps the Y of the slot in OBJ_WORK_F
 * (KeepYOfWorkFSlot0D5F) (Stage 08's overlay).
 * HOW: LoadObjectRecord, an add; a word table read, StepObjectFrames, DEC, then the piece.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    AnimateFollowingYForCount0D5FLists = 0xD743,   /* the frame list's address for each OBJ_WORK_D, a word each */
};

void AnimateFollowingYForCount0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD717);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->work_d[slot]));
        op_clc();
        op_adc(0x03);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_lda(0x28);
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(AnimateFollowingYForCount0D5FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(AnimateFollowingYForCount0D5FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD738);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    KeepYOfWorkFSlot0D5F(slot);
}

void abi_AnimateFollowingYForCount0D5F(void)
{
    AnimateFollowingYForCount0D5F(cpu.x);
}

/*
 * @routine 0D60:138 RideAboveOwner0D60
 * @run 71:D938
 * WHAT IT DOES: In state 0 starts the object with frame 0, not cleared far off screen when OBJ_START_ARG is
 * negative, and item 2 in its hit reaction. Later it plays its frame list and follows the slot in OBJ_WORK_D: on
 * kind $C5 it sits $17 above that slot's place, facing left when that slot's X speed is negative unless the slot is
 * in state 2; on kind $BF it sits $17 above that slot's place plus its hit box offset; on any other kind it is
 * cleared (Stage 08's overlay).
 * HOW: LoadObjectRecord and ORs; StepObjectFrames, kind compares, 16-bit copies and subtracts.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RideAboveOwner0D60(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD940);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        WR8(OBJ->frame[slot], 0);
        op_lda(RD8(OBJ->start_arg[slot]));
        if (flag_n()) {
            op_lda(RD8(OBJ->loop_flags[slot]));
            op_ora(0x02);
            WR8(OBJ->loop_flags[slot], cpu.a);
        }
        op_lda(RD8(OBJ->hit_reaction[slot]));
        op_and(0xE3);
        op_ora(0x08);
        WR8(OBJ->hit_reaction[slot], cpu.a);
    } else {
        op_lda(0xD9);
        WR8(ZP->temp11, cpu.a);
        op_lda(0xF0);
        WR8(ZP->temp10, cpu.a);
        {
            uint8_t y;
            FrameStep step;

            jsr_push(0xD967);
            y = cpu.y;
            step = StepObjectFrames(slot, &y);
            step_frames_exit_shim(step, y);
            jsr_pop();
        }
        op_ldy(RD8(OBJ->work_d[slot]));
        op_lda(RD8(OBJ->kind[cpu.y]));
        op_cmp(0xC5);
        if (!flag_z()) {
            op_cmp(0xBF);
            if (flag_z()) {
                goto at_D9A8;
            }
            ClearObject(slot);
            return;
        }
        op_lda(RD8(OBJ->x_lo[cpu.y]));
        WR8(OBJ->x_lo[slot], cpu.a);
        op_lda(RD8(OBJ->x_hi[cpu.y]));
        WR8(OBJ->x_hi[slot], cpu.a);
        op_lda(RD8(OBJ->y_lo[cpu.y]));
        op_sec();
        op_sbc(0x17);
        WR8(OBJ->y_lo[slot], cpu.a);
        op_lda(RD8(OBJ->y_hi[cpu.y]));
        op_sbc(0x00);
        WR8(OBJ->y_hi[slot], cpu.a);
        op_lda(RD8(OBJ->state[cpu.y]));
        op_cmp(0x02);
        if (flag_z()) {
            goto at_D9A7;
        }
    }
    WR8(OBJ->facing_left[slot], 0);
    op_lda(RD8(OBJ->vx_hi[cpu.y]));
    if (flag_n()) {
        RMW8(OBJ->facing_left[slot], v, f_inc(v));
    }
at_D9A7:
    return;
at_D9A8:
    WR8(ZP->temp00, 0);
    op_lda(RD8(OBJ->hitbox_dx[cpu.y]));
    if (flag_n()) {
        RMW8(ZP->temp00, v, f_dec(v));
    }
    WR8(ZP->temp01, 0);
    op_lda(RD8(OBJ->hitbox_dy[cpu.y]));
    if (flag_n()) {
        RMW8(ZP->temp01, v, f_dec(v));
    }
    op_lda(RD8(OBJ->x_lo[cpu.y]));
    op_clc();
    op_adc(RD8(OBJ->hitbox_dx[cpu.y]));
    WR8(OBJ->x_lo[slot], cpu.a);
    op_lda(RD8(ZP->temp00));
    op_adc(RD8(OBJ->x_hi[cpu.y]));
    WR8(OBJ->x_hi[slot], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.y]));
    op_clc();
    op_adc(RD8(OBJ->hitbox_dy[cpu.y]));
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(ZP->temp01));
    op_adc(RD8(OBJ->y_hi[cpu.y]));
    WR8(OBJ->y_hi[slot], cpu.a);
    op_lda(RD8(OBJ->y_lo[slot]));
    op_sec();
    op_sbc(0x17);
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(OBJ->y_hi[slot]));
    op_sbc(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_RideAboveOwner0D60(void)
{
    RideAboveOwner0D60(cpu.x);
}

/*
 * @routine 0D60:323 CarryPlayerByHeightTable0D60
 * @run 71:DB23
 * WHAT IT DOES: Is a block $17 wide whose height above $16 a table gives by the player's X distance less 6, from 0
 * to $0F; when the player stands on it, sets bits 0, 4 and 5 of zPlayerPush and gives the player its Y speed and
 * one and a half times its X speed as the push (Stage 08's overlay).
 * HOW: DistanceXToPlayer clamped, a table read, ContactWithPlayer, then the speeds copied and halved with LSR and
 * ROR.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    CarryPlayerByHeightTable0D60Heights = 0xDB74,   /* the height above $16 for each of 16 distances */
};

void CarryPlayerByHeightTable0D60(void)
{
    jsr_push(0xDB26);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_sec();
    op_sbc(0x06);
    if (flag_c()) {
        op_cmp(0x10);
        if (!flag_c()) {
            goto at_DB34;
        }
        op_lda(0x0F);
    } else {
        op_cla();
    }
at_DB34:
    op_tay();
    op_lda(rd8((uint16_t)(CarryPlayerByHeightTable0D60Heights + cpu.y)));
    op_clc();
    op_adc(0x16);
    WR8(ZP->temp09, cpu.a);
    op_lda(0x17);
    WR8(ZP->temp08, cpu.a);
    call_routine(0xDB44, 0xAA35);
    if (flag_c()) {
        op_lda(RD8(ZP->player_push));
        op_ora(0x31);
        WR8(ZP->player_push, cpu.a);
        op_lda(RD8(OBJ->vy_hi[cpu.x]));
        WR8(ZP->push_y.hi, cpu.a);
        op_lda(RD8(OBJ->vy_lo[cpu.x]));
        WR8(ZP->push_y.lo, cpu.a);
        op_lda(RD8(OBJ->vx_hi[cpu.x]));
        cpu.a = f_lsr(cpu.a);
        op_lda(RD8(OBJ->vx_lo[cpu.x]));
        cpu.a = f_ror(cpu.a);
        op_clc();
        op_adc(RD8(OBJ->vx_lo[cpu.x]));
        WR8(ZP->push_x.lo, cpu.a);
        op_lda(RD8(OBJ->vx_hi[cpu.x]));
        cpu.a = f_asl(cpu.a);
        op_lda(RD8(OBJ->vx_hi[cpu.x]));
        op_php();
        cpu.a = f_ror(cpu.a);
        op_plp();
        op_adc(RD8(OBJ->vx_hi[cpu.x]));
        WR8(ZP->push_x.hi, cpu.a);
    }
}

void abi_CarryPlayerByHeightTable0D60(void)
{
    CarryPlayerByHeightTable0D60();
}

/*
 * @routine 0D60:384 StartDrifting0D60
 * @run 71:DB84
 * WHAT IT DOES: Starts the object in state 1 with X speed $40, turned round when OBJ_START_ARG bit 0 is set; with
 * OBJ_START_ARG negative it is not cleared far off screen and starts in state 2, or 1 when OBJ_WORK_H is set (Stage
 * 08's overlay).
 * HOW: LoadObjectRecord, INC, an OR of OBJ_LOOP_FLAGS, NegateVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartDrifting0D60(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xDB87);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
    op_lda(RD8(OBJ->start_arg[slot]));
    if (flag_n()) {
        op_lda(RD8(OBJ->loop_flags[slot]));
        op_ora(0x02);
        WR8(OBJ->loop_flags[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
        op_lda(RD8(OBJ->work_h[slot]));
        if (!flag_z()) {
            op_lda(0x01);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
    op_lda(0x40);
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_lsr(cpu.a);
    if (flag_c()) {
        jsr_push(0xDBB2);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_StartDrifting0D60(void)
{
    StartDrifting0D60(cpu.x);
}

/*
 * @routine 0D60:4FA ReturnOnly0D60
 * @run 71:DCFA
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D60(void)
{
}

void abi_ReturnOnly0D60(void)
{
    ReturnOnly0D60();
}

/*
 * @routine 0D60:4FB SpawnKindC5WhenClear0D60
 * @run 71:DCFB
 * WHAT IT DOES: In state 0 starts the object and moves it 256 pixels up. Later, with zField8C 0 and no object of
 * kind $C5 in slots $10 to $57 still above the screen or short of the X and Y a table gives by OBJ_START_ARG,
 * starts one at its place in a free slot from $30: with OBJ_START_ARG set at X $90, Y 8 with argument 1; else at
 * the X, Y and argument tables give by the scroll's Y over 8, the Y $40 above the scroll when the table's byte is
 * positive. Then, except one time in four at random for the second case, starts an object of kind $2E in a free
 * slot from $30 with the new slot in OBJ_WORK_D (Stage 08's overlay).
 * HOW: LoadObjectRecord and DEC; a scan of the slots with compares; FindFreeSlotFrom30, StartObjectAtPlace and
 * table reads by the scroll's Y shifted three bits; NextRandom.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SpawnKindC5WhenClear0D60MinX = 0xDDC0,   /* the X a kind $C5 must have reached, per OBJ_START_ARG */
    SpawnKindC5WhenClear0D60MinY = 0xDDC2,   /* the Y a kind $C5 must have reached, per OBJ_START_ARG */
    SpawnKindC5WhenClear0D60X = 0xDDC4,   /* the new object's X low byte by the scroll's Y over 8 */
    SpawnKindC5WhenClear0D60YLo = 0xDE05,   /* its Y low byte, or a positive byte for $40 above the scroll */
    SpawnKindC5WhenClear0D60YHi = 0xDE46,   /* its Y high byte */
    SpawnKindC5WhenClear0D60Args = 0xDE87,   /* its OBJ_START_ARG */
};

void SpawnKindC5WhenClear0D60(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xDD03);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        RMW8(OBJ->y_hi[cpu.x], v, f_dec(v));
        return;
    }
    op_lda(RD8(ZP->field8c));
    if (!flag_z()) {
        ReturnOnly0D60();
        return;
    }
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60MinX + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60MinY + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_ldy(0x10);
    do {
        op_lda(RD8(OBJ->kind[cpu.y]));
        op_cmp(0xC5);
        if (flag_z()) {
            op_lda(RD8(OBJ->x_hi[cpu.y]));
            if (flag_z()) {
                op_lda(RD8(OBJ->y_hi[cpu.y]));
                if (flag_n()) {
                    ReturnOnly0D60();
                    return;
                }
                op_lda(RD8(OBJ->y_hi[cpu.y]));
                if (flag_z()) {
                    op_lda(RD8(OBJ->x_lo[cpu.y]));
                    op_cmp(RD8(ZP->temp00));
                    if (!flag_c()) {
                        ReturnOnly0D60();
                        return;
                    }
                    op_lda(RD8(OBJ->y_lo[cpu.y]));
                    op_cmp(RD8(ZP->temp01));
                    if (!flag_c()) {
                        ReturnOnly0D60();
                        return;
                    }
                }
            }
        }
        op_iny();
        op_cpy(0x58);
    } while (!flag_z());
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xDD46);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0xC5);
        jsr_push(0xDD4D);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (!flag_z()) {
            op_lda(0x90);
            WR8(OBJ->x_lo[cpu.y], cpu.a);
            op_lda(0x08);
            WR8(OBJ->y_lo[cpu.y], cpu.a);
            op_cla();
            WR8(OBJ->y_hi[cpu.y], cpu.a);
            op_lda(0x01);
            WR8(OBJ->start_arg[cpu.y], cpu.a);
        } else {
            op_phx();
            op_lda(RD8(SCROLLE->y_hi_screen[0x00]));
            WR8(ZP->temp00, cpu.a);
            op_lda(RD8(SCROLL->y_lo[0x00]));
            RMW8(ZP->temp00, v, f_lsr(v));
            cpu.a = f_ror(cpu.a);
            RMW8(ZP->temp00, v, f_lsr(v));
            cpu.a = f_ror(cpu.a);
            RMW8(ZP->temp00, v, f_lsr(v));
            cpu.a = f_ror(cpu.a);
            op_tax();
            op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60X + cpu.x)));
            WR8(OBJ->x_lo[cpu.y], cpu.a);
            op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60YLo + cpu.x)));
            if (!flag_n()) {
                op_lda(0x40);
                op_sec();
                op_sbc(RD8(SCROLL->y_lo[0x00]));
                WR8(OBJ->y_lo[cpu.y], cpu.a);
                op_cla();
                op_sbc(RD8(SCROLLE->y_hi_screen[0x00]));
                WR8(OBJ->y_hi[cpu.y], cpu.a);
            } else {
                WR8(OBJ->y_lo[cpu.y], cpu.a);
                op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60YHi + cpu.x)));
                WR8(OBJ->y_hi[cpu.y], cpu.a);
            }
            op_lda(rd8((uint16_t)(SpawnKindC5WhenClear0D60Args + cpu.x)));
            WR8(OBJ->start_arg[cpu.y], cpu.a);
            op_plx();
            jsr_push(0xDDAA);
            random_exit_shim(NextRandom());
            jsr_pop();
            op_and(0x03);
            if (flag_z()) {
                goto at_DDBF;
            }
        }
        WR8(ZP->hit_slot, cpu.y);
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xDDB3);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0x2E);
            jsr_push(0xDDBA);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            jsr_pop();
            op_lda(RD8(ZP->hit_slot));
            WR8(OBJ->work_d[cpu.y], cpu.a);
        }
    }
at_DDBF:
    ;
}

void abi_SpawnKindC5WhenClear0D60(void)
{
    SpawnKindC5WhenClear0D60();
}

/*
 * @routine 0D6C:2E9 LevelWordByStep0D6C
 * @run 74:DAE9
 * WHAT IT DOES: Sets the level word at $3EB8 (RasterWaterWanted, LevelYHi) from a table by OBJ_WORK_F's low three
 * bits; when those are 0 plays sound $4E if the first byte of a second table is not 0, the index being always 0
 * since the AND comes before the shifts (Stage 08's overlay).
 * HOW: A word table read by twice the low bits, a test, a byte table read, SoundCommand.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LevelWordByStep0D6CWords = 0xDB12,   /* the level word for each of eight steps */
    LevelWordByStep0D6CSounds = 0xDB52,   /* nonzero when the sound plays */
};

void LevelWordByStep0D6C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_f[slot]));
    op_and(0x07);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LevelWordByStep0D6CWords + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    op_lda(rd8((uint16_t)(LevelWordByStep0D6CWords + 1 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_and(0x07);
    if (flag_z()) {
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(LevelWordByStep0D6CSounds + cpu.y)));
        if (!flag_z()) {
            op_lda(0x4E);
            {
                uint8_t command;

                jsr_push(0xDB11);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
        }
    }
}

void abi_LevelWordByStep0D6C(void)
{
    LevelWordByStep0D6C(cpu.x);
}

/*
 * @routine 0D6C:372 SpawnKind9EEvery8_0D6C
 * @run 74:DB72
 * WHAT IT DOES: Every eighth frame of zWaveCounter starts an object of kind $9E (SpawnKind9EScattered0D6C) (Stage
 * 08's overlay).
 * HOW: A TST of the counter, then falls into the next routine.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SpawnKind9EEvery8_0D6C(void)
{
    op_tst(0x07, RD8(ZP->wave_counter));
    if (!flag_z()) {
        ReturnOnly0D6C();
        return;
    }
    /* falls into $DB77 */
    SpawnKind9EScattered0D6C();
}

void abi_SpawnKind9EEvery8_0D6C(void)
{
    SpawnKind9EEvery8_0D6C();
}

/*
 * @routine 0D6C:377 SpawnKind9EScattered0D6C
 * @run 74:DB77
 * WHAT IT DOES: Starts an object of kind $9E at the object's place in a free slot from $20, with the object's
 * OBJ_WORK_G as its OBJ_START_ARG, sprite attributes $10 and Y $B0, moved by a random Y step of -$10 to $0F, an X
 * step a table gives by its argument's low four bits and a random X step of -3 to 4; then steps OBJ_WORK_G on
 * (Stage 08's overlay).
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace, NextRandom twice with ApplyVelocityY and ApplyVelocityX three times.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SpawnKind9EScattered0D6CSteps = 0xDD1A,   /* the X step for each argument's low four bits */
};

void SpawnKind9EScattered0D6C(void)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xDB7A);
        found_slot = FindFreeSlotFrom20(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        ReturnOnly0D6C();
        return;
    }
    op_lda(0x9E);
    jsr_push(0xDB81);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    op_phx();
    op_sxy();
    op_lda(0x10);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    WR8(OBJ->y_hi[cpu.x], 0);
    op_lda(0xB0);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    jsr_push(0xDB99);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x1F);
    op_sec();
    op_sbc(0x10);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    jsr_push(0xDBA4);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(SpawnKind9EScattered0D6CSteps + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    jsr_push(0xDBB3);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    jsr_push(0xDBB6);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x07);
    op_sec();
    op_sbc(0x03);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    jsr_push(0xDBC1);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    op_plx();
    RMW8(OBJ->work_g[cpu.x], v, f_inc(v));
    /* falls into $DBC5 */
    ReturnOnly0D6C();
}

void abi_SpawnKind9EScattered0D6C(void)
{
    SpawnKind9EScattered0D6C();
}

/*
 * @routine 0D6C:3C5 ReturnOnly0D6C
 * @run 74:DBC5
 * WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
 * HOW: An RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnOnly0D6C(void)
{
}

void abi_ReturnOnly0D6C(void)
{
    ReturnOnly0D6C();
}

/*
 * @routine 0D6C:401 CopyBlockToVram0D6C
 * @run 74:DC01
 * WHAT IT DOES: Copies the 32 bytes at $C000 plus 32 times A to VRAM as four rows of four words, from the address
 * in zTemp10-11 with the rows $40 words apart, the address kept below $1000 (Stage 08's overlay).
 * HOW: Shifts build the source in zTemp12 and zProductLo; ClearVdcControlHigh, then per row SelectVramWriteAddress,
 * SelectVramData and four word writes counted in zMultiplierHi.
 * WAITS: no.
 * Entry: A the block's index; zTemp10-11 the VRAM address. Exit: A, X, Y and the flags as its last instruction or
 * call left them.
 */
void CopyBlockToVram0D6C(void)
{
    WR8(ZP->product.lo, 0);
    cpu.a = f_asl(cpu.a);
    RMW8(ZP->product.lo, v, f_rol(v));
    cpu.a = f_asl(cpu.a);
    RMW8(ZP->product.lo, v, f_rol(v));
    cpu.a = f_asl(cpu.a);
    RMW8(ZP->product.lo, v, f_rol(v));
    cpu.a = f_asl(cpu.a);
    RMW8(ZP->product.lo, v, f_rol(v));
    cpu.a = f_asl(cpu.a);
    RMW8(ZP->product.lo, v, f_rol(v));
    op_clc();
    op_adc(0x00);
    WR8(ZP->temp12, cpu.a);
    op_lda(RD8(ZP->product.lo));
    op_adc(0xC0);
    WR8(ZP->product.lo, cpu.a);
    jsr_push(0xDC20);
    ClearVdcControlHigh();
    cpu.a = 0;
    abi_nz(0);
    jsr_pop();
    WR8(ZP->multiplier.hi, 0);
    op_cly();
    do {
        jsr_push(0xDC26);
        SelectVramWriteAddress();
        cpu.a = STAGE_C_VDC_MAWR;
        jsr_pop();
        op_lda(RD8(ZP->temp10));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(ZP->temp11));
        WR8(VDC->data_hi, cpu.a);
        jsr_push(0xDC33);
        SelectVramData();
        cpu.a = STAGE_C_VDC_VRAM;
        abi_nz(STAGE_C_VDC_VRAM);
        jsr_pop();
        do {
            op_lda(RD8_PTR_Y(ZP->temp12, cpu.y));
            WR8(VDC->data_lo, cpu.a);
            op_iny();
            op_lda(RD8_PTR_Y(ZP->temp12, cpu.y));
            WR8(VDC->data_hi, cpu.a);
            op_iny();
            RMW8(ZP->multiplier.hi, v, f_inc(v));
            op_lda(RD8(ZP->multiplier.hi));
            op_and(0x03);
        } while (!flag_z());
        op_lda(0x40);
        op_clc();
        op_adc(RD8(ZP->temp10));
        WR8(ZP->temp10, cpu.a);
        op_lda(0x00);
        op_adc(RD8(ZP->temp11));
        op_and(0x0F);
        WR8(ZP->temp11, cpu.a);
        op_lda(RD8(ZP->multiplier.hi));
        op_cmp(0x10);
    } while (!flag_z());
}

void abi_CopyBlockToVram0D6C(void)
{
    CopyBlockToVram0D6C();
}

/*
 * @routine 0D6C:568 FallToB0ThenArm0D6C
 * @run 74:DD68
 * WHAT IT DOES: Moves the object down under gravity; once its Y low byte reaches $B0 it stops there with hit box 1
 * and steps its state on (Stage 08's overlay).
 * HOW: ApplyGravity, ApplyVelocityY, a compare and stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallToB0ThenArm0D6C(uint8_t slot)
{
    jsr_push(0xDD6B);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    jsr_push(0xDD6E);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_lo[slot]));
    op_cmp(0xB0);
    if (flag_c()) {
        op_lda(0xB0);
        WR8(OBJ->y_lo[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_FallToB0ThenArm0D6C(void)
{
    FallToB0ThenArm0D6C(cpu.x);
}

/*
 * @routine 0D6C:583 Kind9BAtFifthHit0D6C
 * @run 74:DD83
 * WHAT IT DOES: When the object has been hit exactly 5 times, starts an object of kind $9B in slot $1E at its
 * place, 12 pixels left and 8 down, and steps its state on with OBJ_WORK_M 4 and OBJ_WORK_L 0 (Stage 08's overlay).
 * HOW: A compare, ClearObjectInY, StartObjectAtPlace, ApplyVelocityX and ApplyVelocityY on the new slot, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Kind9BAtFifthHit0D6C(void)
{
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (flag_z()) {
        op_ldy(0x1E);
        jsr_push(0xDD8F);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0x9B);
        jsr_push(0xDD94);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_sxy();
        op_lda(0xF4);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        jsr_push(0xDD9D);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
        op_lda(0x08);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xDDA5);
        axis_move_exit_shim(ApplyVelocityY(cpu.x));
        jsr_pop();
        op_sxy();
        op_lda(0x04);
        WR8(OBJ->work_m[cpu.x], cpu.a);
        WR8(OBJ->work_l[cpu.x], 0);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
}

void abi_Kind9BAtFifthHit0D6C(void)
{
    Kind9BAtFifthHit0D6C();
}

/*
 * @routine 0D6C:60F StartKind9EInY0D6C
 * @run 74:DE0F
 * WHAT IT DOES: Starts an object of kind $9E at the object's place in slot Y and moves it once by its own speeds;
 * the two random numbers drawn in A beside those moves are not read (Stage 08's overlay).
 * HOW: StartObjectAtPlace, NextRandom with ApplyVelocityX, NextRandom with ApplyVelocityY, on the new slot.
 * WAITS: no.
 * Entry: X the object; Y the new slot. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKind9EInY0D6C(void)
{
    op_lda(0x9E);
    jsr_push(0xDE14);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_sxy();
    jsr_push(0xDE18);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_sec();
    op_sbc(0x08);
    jsr_push(0xDE20);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    jsr_push(0xDE23);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    jsr_push(0xDE28);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_sxy();
}

void abi_StartKind9EInY0D6C(void)
{
    StartKind9EInY0D6C();
}

/*
 * @routine 0D6C:62A StartKind5CByWorkL0D6C
 * @run 74:DE2A
 * WHAT IT DOES: Starts an object of kind $5C at the object's place in a free slot from $20 and moves it once by the
 * X and Y steps a table gives by OBJ_WORK_L (Stage 08's overlay).
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace, a pair read by twice OBJ_WORK_L, ApplyVelocityX and ApplyVelocityY.
 * WAITS: no.
 * Entry: X the object. Exit: C set when an object was started; Y its slot.
 */
enum {
    StartKind5CByWorkL0D6CSteps = 0xDE59,   /* X step, Y step for each OBJ_WORK_L */
};

void StartKind5CByWorkL0D6C(void)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xDE2D);
        found_slot = FindFreeSlotFrom20(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x5C);
        jsr_push(0xDE34);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_phy();
        op_lda(RD8(OBJ->work_l[cpu.x]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(StartKind5CByWorkL0D6CSteps + cpu.y)));
        WR8(ZP->temp00, cpu.a);
        op_lda(rd8((uint16_t)(StartKind5CByWorkL0D6CSteps + 1 + cpu.y)));
        WR8(ZP->temp01, cpu.a);
        op_ply();
        op_sxy();
        op_lda(RD8(ZP->temp00));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        jsr_push(0xDE4E);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
        op_lda(RD8(ZP->temp01));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xDE56);
        axis_move_exit_shim(ApplyVelocityY(cpu.x));
        jsr_pop();
        op_sxy();
        op_sec();
    }
}

void abi_StartKind5CByWorkL0D6C(void)
{
    StartKind5CByWorkL0D6C();
}

/*
 * @routine 0D6C:6E6 MaskTilesByRecord0D6C
 * @run 74:DEE6
 * WHAT IT DOES: Takes the five-byte record OBJ_WORK_L picks and masks VRAM words at $3F00 plus its first byte
 * (MaskVramWords0D6C) with its second byte, then its third, and its third again unless that is $FF (Stage 08's
 * overlay).
 * HOW: Five times OBJ_WORK_L indexes the records; three calls with zTemp08, zTemp09 $3F and zTemp0F set.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MaskTilesByRecord0D6CRecords = 0xDF83,   /* the VRAM column, then the mask bytes, five bytes per record */
};

void MaskTilesByRecord0D6C(void)
{
    op_lda(RD8(OBJ->work_l[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(0x3F);
    WR8(ZP->temp09, cpu.a);
    op_lda(rd8((uint16_t)(MaskTilesByRecord0D6CRecords + cpu.y)));
    WR8(ZP->temp08, cpu.a);
    op_lda(rd8((uint16_t)(MaskTilesByRecord0D6CRecords + 1 + cpu.y)));
    WR8(ZP->temp0f, cpu.a);
    op_phy();
    jsr_push(0xDF03);
    MaskVramWords0D6C();
    jsr_pop();
    op_ply();
    op_lda(rd8((uint16_t)(MaskTilesByRecord0D6CRecords + 2 + cpu.y)));
    WR8(ZP->temp0f, cpu.a);
    op_phy();
    jsr_push(0xDF0D);
    MaskVramWords0D6C();
    jsr_pop();
    op_ply();
    op_lda(rd8((uint16_t)(MaskTilesByRecord0D6CRecords + 2 + cpu.y)));
    WR8(ZP->temp0f, cpu.a);
    op_cmp(0xFF);
    if (!flag_z()) {
        MaskVramWords0D6C();
        return;
    }
}

void abi_MaskTilesByRecord0D6C(void)
{
    MaskTilesByRecord0D6C();
}

/*
 * @routine 0D6C:718 MaskVramWords0D6C
 * @run 74:DF18
 * WHAT IT DOES: ANDs four VRAM words, $10 apart from zTemp09-08 plus zTemp0F's high four bits, with the low and
 * high mask bytes zTemp0F's low four bits pick (Stage 08's overlay).
 * HOW: A loop of SelectVramReadAddress, SelectVramWriteAddress and SelectVramData with a read, AND and write.
 * WAITS: no.
 * Entry: zTemp08 the address low byte; zTemp09 its high byte; zTemp0F the offset and mask nibbles. Exit: A, X, Y
 * and the flags as its last instruction or call left them.
 */
enum {
    MaskVramWords0D6CMasksLo = 0xDF6B,   /* the low byte's mask for each of eight entries */
    MaskVramWords0D6CMasksHi = 0xDF73,   /* the high byte's mask for each of eight entries */
};

void MaskVramWords0D6C(void)
{
    op_lda(RD8(ZP->temp0f));
    op_and(0x0F);
    op_tay();
    op_lda(RD8(ZP->temp0f));
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp08));
    WR8(ZP->temp0a, cpu.a);
    op_lda(RD8(ZP->temp09));
    WR8(ZP->temp0b, cpu.a);
    op_lda(0x04);
    WR8(ZP->temp0e, cpu.a);
    do {
        op_phx();
        op_phy();
        op_ldx(RD8(ZP->temp0a));
        op_ldy(RD8(ZP->temp0b));
        jsr_push(0xDF39);
        SelectVramReadAddress();
        cpu.a = STAGE_C_VDC_MARR;
        abi_nz(STAGE_C_VDC_MARR);
        jsr_pop();
        WR8(VDC->data_lo, cpu.x);
        WR8(VDC->data_hi, cpu.y);
        jsr_push(0xDF42);
        SelectVramWriteAddress();
        cpu.a = STAGE_C_VDC_MAWR;
        jsr_pop();
        WR8(VDC->data_lo, cpu.x);
        WR8(VDC->data_hi, cpu.y);
        jsr_push(0xDF4B);
        SelectVramData();
        cpu.a = STAGE_C_VDC_VRAM;
        abi_nz(STAGE_C_VDC_VRAM);
        jsr_pop();
        op_ply();
        op_lda(RD8(VDC->data_lo));
        op_and(rd8((uint16_t)(MaskVramWords0D6CMasksLo + cpu.y)));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(VDC->data_hi));
        op_and(rd8((uint16_t)(MaskVramWords0D6CMasksHi + cpu.y)));
        WR8(VDC->data_hi, cpu.a);
        op_plx();
        op_lda(RD8(ZP->temp0a));
        op_clc();
        op_adc(0x10);
        WR8(ZP->temp0a, cpu.a);
        RMW8(ZP->temp0e, v, f_dec(v));
    } while (!flag_z());
}

void abi_MaskVramWords0D6C(void)
{
    MaskVramWords0D6C();
}
