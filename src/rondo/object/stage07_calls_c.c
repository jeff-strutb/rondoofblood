/*
 * Calling routines of Stage 07's overlays: object starts and state tables, frame lists, landings and leaps at the
 * player, hit reactions, spawns of pieces and shots, turning toward an angle, and the tile and VRAM checks of its walls.
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
#include "stage07_calls_c.h"

/*
 * @routine 0BFA:3DE DrawTilesThenClear0BFA
 * @run 70:ABDE
 * WHAT IT DOES: Draws item $0B of tile script group 0 and removes the object (Stage 07's overlay).
 * HOW: DrawTileScript, then ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DrawTilesThenClear0BFA(void)
{
    op_lda(0x0B);
    op_cly();
    {
        uint8_t item;
        uint8_t group;

        jsr_push(0xABE4);
        item = cpu.a;
        group = cpu.y;
        abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
        jsr_pop();
    }
    jsr_push(0xABE7);
    ClearObject(cpu.x);
    jsr_pop();
}

void abi_DrawTilesThenClear0BFA(void)
{
    DrawTilesThenClear0BFA();
}

/*
 * @routine 0BFB:10D StartSinkingTop0BFB
 * @run 70:B10D
 * @stop B11F
 * WHAT IT DOES: In state 0 loads the object's record and sets OBJ_WORK_D, the height of the top the player stands
 * on, from a table by OBJ_START_ARG; in later states goes on to the contact test at $B127 (Stage 07's overlay).
 * HOW: A test of the state; LoadObjectRecord and a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartSinkingTop0BFBHeights = 0xB123,   /* the full OBJ_WORK_D height for each OBJ_START_ARG */
};

void StartSinkingTop0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        SinkUnderPlayer0BFB();
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB115);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(rd8((uint16_t)(StartSinkingTop0BFBHeights + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_StartSinkingTop0BFB(void)
{
    StartSinkingTop0BFB(cpu.x);
}

/*
 * @routine 0BFB:127 SinkUnderPlayer0BFB
 * @run 70:B127
 * @stop B198
 * WHAT IT DOES: Unless the player's Y high byte is 1 or his Y low $FA or more, tests his contact with the object
 * (width by OBJ_START_ARG, height OBJ_WORK_D); standing on it sets OBJ_WORK_E and zPlayerPush bit 4, pushes him
 * back by his own X speed, and lowers OBJ_WORK_D by a step for each of its first 8 frames (OBJ_WORK_F), then by 1
 * on even zWaveCounter with the push halved; off it, goes on at $B1A0 (Stage 07's overlay).
 * HOW: ContactWithPlayer with zTemp08/09; zPlayerPush bits 1 and 2 cleared; tables by OBJ_START_ARG and OBJ_WORK_F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SinkUnderPlayer0BFBWidths = 0xB11F,   /* the contact width for each OBJ_START_ARG */
    SinkUnderPlayer0BFBDrops = 0xB198,   /* the drop of OBJ_WORK_D for each of the first 8 frames stood on */
};

void SinkUnderPlayer0BFB(void)
{
    op_lda(RD8(OBJ->y_hi[0x00]));
    op_cmp(0x01);
    if (!flag_z()) {
        op_lda(RD8(OBJ->y_lo[0x00]));
        op_cmp(0xFA);
        if (!flag_c()) {
            op_ldy(RD8(OBJ->start_arg[cpu.x]));
            op_lda(rd8((uint16_t)(SinkUnderPlayer0BFBWidths + cpu.y)));
            WR8(ZP->temp08, cpu.a);
            op_lda(RD8(OBJ->work_d[cpu.x]));
            WR8(ZP->temp09, cpu.a);
            if (!flag_z()) {
                call_routine(0xB147, 0xAA35);
                op_php();
                op_lda(RD8(ZP->player_push));
                op_and(0xF9);
                WR8(ZP->player_push, cpu.a);
                op_plp();
                if (!flag_c()) {
                    RiseBackAndKick0BFB(cpu.x);
                    return;
                }
                op_lda(0x01);
                WR8(OBJ->work_e[cpu.x], cpu.a);
                op_lda(RD8(ZP->player_push));
                op_ora(0x10);
                WR8(ZP->player_push, cpu.a);
                op_lda(RD8(OBJ->vx_lo[0x00]));
                op_eor(0xFF);
                op_clc();
                op_adc(0x01);
                WR8(ZP->push_x.lo, cpu.a);
                op_lda(RD8(OBJ->vx_hi[0x00]));
                op_eor(0xFF);
                op_adc(0x00);
                WR8(ZP->push_x.hi, cpu.a);
                op_lda(RD8(OBJ->work_f[cpu.x]));
                op_cmp(0x08);
                if (!flag_c()) {
                    goto at_B187;
                }
                op_lda(RD8(ZP->wave_counter));
                op_and(0x01);
                if (flag_z()) {
                    op_lda(RD8(ZP->push_x.hi));
                    cpu.a = f_asl(cpu.a);
                    RMW8(ZP->push_x.hi, v, f_ror(v));
                    RMW8(ZP->push_x.lo, v, f_ror(v));
                    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
                }
            }
        }
    }
    return;
at_B187:
    op_ldy(RD8(OBJ->work_f[cpu.x]));
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_sec();
    op_sbc(rd8((uint16_t)(SinkUnderPlayer0BFBDrops + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    RMW8(OBJ->work_f[cpu.x], v, f_inc(v));
}

void abi_SinkUnderPlayer0BFB(void)
{
    SinkUnderPlayer0BFB();
}

/*
 * @routine 0BFB:1A0 RiseBackAndKick0BFB
 * @run 70:B1A0
 * WHAT IT DOES: Raises OBJ_WORK_D by 1 while it is below its full height; if the player had stood on it
 * (OBJ_WORK_E) and now moves up or down, unless zGameSubMode is 4, halves his X speed and gives him Y speed -$340;
 * then clears OBJ_WORK_E and OBJ_WORK_F (Stage 07's overlay).
 * HOW: A compare with the table by OBJ_START_ARG; stores to slot 0's speeds.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RiseBackAndKick0BFBHeights = 0xB123,   /* the full OBJ_WORK_D height for each OBJ_START_ARG */
};

void RiseBackAndKick0BFB(uint8_t slot)
{
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(rd8((uint16_t)(RiseBackAndKick0BFBHeights + cpu.y)));
    if (!flag_c()) {
        RMW8(OBJ->work_d[slot], v, f_inc(v));
    }
    op_lda(RD8(OBJ->work_e[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->vy_hi[0x00]));
        if (!flag_z()) {
            op_lda(RD8(ZP->game_sub_mode));
            op_cmp(0x04);
            if (!flag_z()) {
                op_lda(RD8(OBJ->vx_hi[0x00]));
                cpu.a = f_asl(cpu.a);
                RMW8(OBJ->vx_hi[0x00], v, f_ror(v));
                RMW8(OBJ->vx_lo[0x00], v, f_ror(v));
                op_lda(0xFC);
                WR8(OBJ->vy_hi[0x00], cpu.a);
                op_lda(0xC0);
                WR8(OBJ->vy_lo[0x00], cpu.a);
            }
        }
    }
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
}

void abi_RiseBackAndKick0BFB(void)
{
    RiseBackAndKick0BFB(cpu.x);
}

/*
 * @routine 0BFB:1F1 ShoveToEdgeWhenHit0BFB
 * @run 70:B1F1
 * WHAT IT DOES: Tests the player's contact (width $0E, height $26); on a hit below 5 moves the object by an X speed
 * chosen by the side of the player, and when its screen X reaches $1FC sounds $3A, records its event, starts kind
 * $A7 in slot $41 at the player's place and steps the state on (Stage 07's overlay).
 * HOW: ContactWithPlayer; a word table by SideOfPlayer's bit 0; a 16-bit screen X from the scroll.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ShoveToEdgeWhenHit0BFBSpeeds = 0xB24B,   /* the X speed for each side of the player, low and high byte */
};

void ShoveToEdgeWhenHit0BFB(void)
{
    op_lda(0x0E);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x26);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB1FC, 0xAA35);
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            jsr_push(0xB208);
            side_of_player_exit_shim(SideOfPlayer(cpu.x));
            jsr_pop();
            op_and(0x01);
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(ShoveToEdgeWhenHit0BFBSpeeds + 1 + cpu.y)));
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(ShoveToEdgeWhenHit0BFBSpeeds + cpu.y)));
            WR8(OBJ->vx_lo[cpu.x], cpu.a);
            jsr_push(0xB21B);
            axis_move_exit_shim(ApplyVelocityX(cpu.x));
            jsr_pop();
            op_lda(RD8(SCROLL->x_lo[0x00]));
            op_clc();
            op_adc(RD8(OBJ->x_lo[cpu.x]));
            WR8(ZP->temp00, cpu.a);
            op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
            op_adc(RD8(OBJ->x_hi[cpu.x]));
            op_cmp(0x01);
            if (flag_z()) {
                op_lda(RD8(ZP->temp00));
                op_cmp(0xFC);
                if (flag_z()) {
                    op_lda(0x3A);
                    {
                        uint8_t command;

                        jsr_push(0xB239);
                        command = cpu.a;
                        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
                        jsr_pop();
                    }
                    {
                        uint8_t stored;
                        uint8_t event;
                        uint8_t byte;

                        jsr_push(0xB23C);
                        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                            cpu.a = 0;
                            abi_nz(0);
                        } else {
                            record_event_exit_shim(event, stored, byte);
                        }
                        jsr_pop();
                    }
                    op_ldy(0x41);
                    op_phx();
                    op_ldx(0x00);
                    op_lda(0xA7);
                    jsr_push(0xB246);
                    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
                    jsr_pop();
                    op_plx();
                    RMW8(OBJ->state[cpu.x], v, f_inc(v));
                }
            }
        }
    }
}

void abi_ShoveToEdgeWhenHit0BFB(void)
{
    ShoveToEdgeWhenHit0BFB();
}

/*
 * @routine 0BFB:24F CarryPlayerSprite0BFB
 * @run 70:B24F
 * WHAT IT DOES: In state 0 loads the record, takes the player's hit box off, sets sprite flip $80, OBJ_WORK_D $B8
 * and Y speed low $40; then counts OBJ_WORK_D down, moving down and wearing the player's sprite set, frame and
 * facing (his own frame cleared, OBJ_WORK_E $10, PlayerStartPose 2); at 0 sets zField83 $0B, zField87 1 and
 * zSubStep 2 (Stage 07's overlay).
 * HOW: Two arms by the state; DEC, ApplyVelocityY and copies from slot 0.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CarryPlayerSprite0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB257);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        WR8(OBJ->hitbox_on[0x00], 0);
        op_lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
        op_lda(0xB8);
        WR8(OBJ->work_d[slot], cpu.a);
        op_lda(0x40);
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x0B);
        WR8(ZP->field83, cpu.a);
        op_lda(0x01);
        WR8(ZP->field87, cpu.a);
        op_lda(0x02);
        WR8(ZP->sub_step, cpu.a);
        WR8(OBJ->frame[0x00], 0);
        return;
    }
    jsr_push(0xB282);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(0x10);
    WR8(OBJ->work_e[0x00], cpu.a);
    op_lda(0x02);
    WR8(PVARS2->start_pose, cpu.a);
    WR8(OBJ->work_f[0x00], 0);
    op_lda(RD8(OBJ->sprite_set[0x00]));
    WR8(OBJ->sprite_set[slot], cpu.a);
    op_lda(RD8(OBJ->frame[0x00]));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[0x00]));
    WR8(OBJ->facing_left[slot], cpu.a);
    WR8(OBJ->frame[0x00], 0);
}

void abi_CarryPlayerSprite0BFB(void)
{
    CarryPlayerSprite0BFB(cpu.x);
}

/*
 * @routine 0BFB:2BD LoadRecordBoxDown08_0BFB
 * @run 70:B2BD
 * WHAT IT DOES: Loads the object's record and sets its hit box Y offset to 8 (Stage 07's overlay).
 * HOW: LoadObjectRecord and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordBoxDown08_0BFB(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB2C0);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x08);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
}

void abi_LoadRecordBoxDown08_0BFB(void)
{
    LoadRecordBoxDown08_0BFB(cpu.x);
}

/*
 * @routine 0BFB:2C6 BreakWithNextSlot0BFB
 * @run 70:B2C6
 * WHAT IT DOES: On a hit of 5 or more sounds $12, records the event of the slot after the object, stops that slot's
 * fall and steps its state and the object's on; with OBJ_START_ARG not 0 also records event $11 (Stage 07's
 * overlay).
 * HOW: A compare of OBJ_HIT_TAKEN; INX and DEX around the stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BreakWithNextSlot0BFB(void)
{
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (flag_c()) {
        op_lda(0x12);
        {
            uint8_t command;

            jsr_push(0xB2D2);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_inx();
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xB2D6);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        WR8(OBJ->vy_hi[cpu.x], 0);
        WR8(OBJ->vy_lo[cpu.x], 0);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        op_dex();
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (!flag_z()) {
            op_lda(0x11);
            {
                uint8_t event;
                uint8_t byte;
                uint8_t merged;

                jsr_push(0xB2ED);
                event = cpu.a;
                merged = RecordEvent(event, cpu.x, &byte);
                record_event_exit_shim(event, merged, byte);
                jsr_pop();
            }
        }
    }
}

void abi_BreakWithNextSlot0BFB(void)
{
    BreakWithNextSlot0BFB();
}

/*
 * @routine 0BFB:306 LoadRecordBoxDown4B_0BFB
 * @run 70:B306
 * WHAT IT DOES: Loads the object's record and sets its hit box Y offset to $4B (Stage 07's overlay).
 * HOW: LoadObjectRecord and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordBoxDown4B_0BFB(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB309);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x4B);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
}

void abi_LoadRecordBoxDown4B_0BFB(void)
{
    LoadRecordBoxDown4B_0BFB(cpu.x);
}

/*
 * @routine 0BFB:30F SoundAndGuardOnHit0BFB
 * @run 70:B30F
 * WHAT IT DOES: On a hit of 5 or more sounds $12 and makes the object untouchable for 5 frames (OBJ_INVULN_TIMER)
 * (Stage 07's overlay).
 * HOW: A compare, SoundCommand, a store.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SoundAndGuardOnHit0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x05);
    if (flag_c()) {
        op_lda(0x12);
        {
            uint8_t command;

            jsr_push(0xB31B);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x05);
        WR8(OBJ->invuln_timer[slot], cpu.a);
    }
}

void abi_SoundAndGuardOnHit0BFB(void)
{
    SoundAndGuardOnHit0BFB(cpu.x);
}

/*
 * @routine 0BFB:321 FallThenClearLow0BFB
 * @run 70:B321
 * WHAT IT DOES: Falls and moves; once its Y reaches $180 or more (high byte 1, low $80 up) sounds $4E when the slot
 * before it has an OBJ_START_ARG, and removes itself (Stage 07's overlay).
 * HOW: ApplyGravityUnlimited, ApplyVelocity, two compares of Y.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void FallThenClearLow0BFB(uint8_t slot)
{
    jsr_push(0xB324);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    jsr_push(0xB327);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_hi[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        op_lda(RD8(OBJ->y_lo[slot]));
        if (flag_n()) {
            op_lda(rd8((uint16_t)(COL(OBJ->start_arg) - 1 + slot)));
            if (!flag_z()) {
                op_lda(0x4E);
                {
                    uint8_t command;

                    jsr_push(0xB33D);
                    command = cpu.a;
                    sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                    jsr_pop();
                }
            }
            jsr_push(0xB340);
            ClearObject(slot);
            jsr_pop();
        }
    }
}

void abi_FallThenClearLow0BFB(void)
{
    FallThenClearLow0BFB(cpu.x);
}

/*
 * @routine 0BFB:341 SwingSlotsBySine0BFB
 * @run 70:B341
 * WHAT IT DOES: In state 0 loads the record; then steps OBJ_WORK_F (frame $0A or $0B by its bit 4), and moves the
 * slot two before it, and the slot just before it while that is below state 2, up or down by the sine of OBJ_WORK_D
 * scaled by the phase OBJ_WORK_E's size; OBJ_WORK_D advances by the phase's step, and wrapping past $FF sets the
 * phase back to 0 (Stage 07's overlay).
 * HOW: Tables by OBJ_WORK_E; ScaleBySine, NegateVelocityY and ApplyVelocityY on slots X-2 and X-1.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SwingSlotsBySine0BFBFrameSteps = 0xB3B6,   /* the step of OBJ_WORK_F for each phase */
    SwingSlotsBySine0BFBPhases = 0xB3BA,   /* per phase: the step of OBJ_WORK_D, then the sine's scale high byte */
};

void SwingSlotsBySine0BFB(void)
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
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(SwingSlotsBySine0BFBFrameSteps + cpu.y)));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_and(0x01);
    op_clc();
    op_adc(0x0A);
    WR8(OBJ->frame[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    WR8(ZP->temp00, 0);
    op_lda(rd8((uint16_t)(SwingSlotsBySine0BFBPhases + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    jsr_push(0xB374);
    scale_exit_shim(ScaleBySine(cpu.a));
    jsr_pop();
    op_phx();
    op_dex();
    op_dex();
    op_lda(RD8(ZP->product.hi));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    op_lda(RD8(ZP->product.mid));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    jsr_push(0xB384);
    negate_exit_shim(NegateVelocityY(cpu.x));
    jsr_pop();
    jsr_push(0xB387);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_inx();
    op_lda(RD8(OBJ->state[cpu.x]));
    op_cmp(0x02);
    if (!flag_c()) {
        op_lda(RD8(ZP->product.hi));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        op_lda(RD8(ZP->product.mid));
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        jsr_push(0xB39C);
        negate_exit_shim(NegateVelocityY(cpu.x));
        jsr_pop();
        jsr_push(0xB39F);
        axis_move_exit_shim(ApplyVelocityY(cpu.x));
        jsr_pop();
    }
    op_plx();
    op_lda(RD8(OBJ->work_e[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(SwingSlotsBySine0BFBPhases + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    if (flag_c()) {
        op_cla();
        WR8(OBJ->work_e[cpu.x], cpu.a);
    }
}

void abi_SwingSlotsBySine0BFB(void)
{
    SwingSlotsBySine0BFB();
}

/*
 * @routine 0BFB:3BD FlaggedThenByState0BFB
 * @run 70:B3BD
 * WHAT IT DOES: Puts a flagged object in state 4 (EnterState4IfFlagged), then runs the handler of its state (Stage
 * 07's overlay).
 * HOW: A call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlaggedThenByState0BFBHandlers = 0xB3D2,   /* the handler address for each state, low and high byte */
};

void FlaggedThenByState0BFB(void)
{
    call_routine(0xB3C0, 0xB3DC);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlaggedThenByState0BFBHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(FlaggedThenByState0BFBHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_FlaggedThenByState0BFB(void)
{
    FlaggedThenByState0BFB();
}

/*
 * @routine 0BFB:3FF WobbleThenLeap0BFB
 * @run 70:B3FF
 * WHAT IT DOES: Moves by an X speed from an eight-step table by OBJ_WORK_D while counting it down; at 0 leaps at
 * the player: X speed by his distance in steps of 32 pixels, Y speed high $FA, state on, turned to face him (Stage
 * 07's overlay).
 * HOW: Tables by OBJ_WORK_D's low three bits and by DistanceXToPlayer over 32; NegateVelocityX when facing left.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WobbleThenLeap0BFBSteps = 0xB43F,   /* the X speed high byte for each of the eight steps */
    WobbleThenLeap0BFBSpeeds = 0xB447,   /* the leap's X speed by distance, low and high byte */
};

void WobbleThenLeap0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    op_and(0x07);
    op_tay();
    op_lda(rd8((uint16_t)(WobbleThenLeap0BFBSteps + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    jsr_push(0xB40E);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        jsr_push(0xB416);
        distance_exit_shim(DistanceXToPlayer(slot));
        jsr_pop();
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_lsr(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(WobbleThenLeap0BFBSpeeds + 1 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(WobbleThenLeap0BFBSpeeds + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        op_lda(0xFA);
        WR8(OBJ->vy_hi[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
        jsr_push(0xB434);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
        if (!flag_z()) {
            jsr_push(0xB43E);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
    }
}

void abi_WobbleThenLeap0BFB(void)
{
    WobbleThenLeap0BFB(cpu.x);
}

/*
 * @routine 0BFB:457 AnimateUntilLanded0BFB
 * @run 70:B457
 * WHAT IT DOES: Plays the frame list at $B47F and falls (height 8); on landing sounds $4A, restarts the animation
 * and goes to state 1 with frame $27 (Stage 07's overlay).
 * HOW: StepObjectFrames, StopOnLanding, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void AnimateUntilLanded0BFB(uint8_t slot)
{
    op_lda(0x7F);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB4);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB462);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(0x08);
    jsr_push(0xB467);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x4A);
        {
            uint8_t command;

            jsr_push(0xB46E);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x27);
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_AnimateUntilLanded0BFB(void)
{
    AnimateUntilLanded0BFB(cpu.x);
}

/*
 * @routine 0BFB:48A DebrisAboveAndBelow0BFB
 * @run 70:B48A
 * WHAT IT DOES: Starts a debris burst (timer 1) 16 pixels above the object and another 16 below, moves back up and
 * removes the object (Stage 07's overlay).
 * HOW: ApplyVelocityY by $F0, $20 and $F0 around two StartDebrisBurst calls; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DebrisAboveAndBelow0BFB(void)
{
    op_lda(0xF0);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    jsr_push(0xB492);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_lda(0x01);
    call_routine(0xB497, 0xA90B);
    op_lda(0x20);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    jsr_push(0xB49F);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_lda(0x01);
    call_routine(0xB4A4, 0xA90B);
    op_lda(0xF0);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    jsr_push(0xB4AC);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    ClearObject(cpu.x);
}

void abi_DebrisAboveAndBelow0BFB(void)
{
    DebrisAboveAndBelow0BFB();
}

/*
 * @routine 0BFB:58D RiseToListEnd0BFB
 * @run 70:B58D
 * WHAT IT DOES: Moves by its Y speed and plays the frame list at $B5BE; when the list ends sets Y speed -$100,
 * sprite set 4, frame 0 and Y low $4C, restarts the animation and steps the state on (Stage 07's overlay).
 * HOW: ApplyVelocityY, StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RiseToListEnd0BFB(uint8_t slot)
{
    jsr_push(0xB590);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(0xBE);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB5);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB59B);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0xFF);
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(0x04);
        WR8(OBJ->sprite_set[slot], cpu.a);
        WR8(OBJ->frame[slot], 0);
        op_lda(0x4C);
        WR8(OBJ->y_lo[slot], cpu.a);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_RiseToListEnd0BFB(void)
{
    RiseToListEnd0BFB(cpu.x);
}

/*
 * @routine 0BFB:601 StartObjectsFromList0BFB
 * @run 70:B601
 * WHAT IT DOES: Starts the objects of a list at the object's place: for each record, clears the slot it names,
 * starts the kind it names there and gives it its OBJ_START_ARG, until the $FF end (Stage 07's overlay).
 * HOW: A loop over four-byte records; ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartObjectsFromList0BFBRecords = 0xB62D,   /* slot, kind, OBJ_START_ARG, a spare byte per record; $FF ends */
};

void StartObjectsFromList0BFB(void)
{
    op_cly();
at_B602:
    op_lda(rd8((uint16_t)(StartObjectsFromList0BFBRecords + cpu.y)));
    op_cmp(0xFF);
    if (flag_z()) {
        return;
    }
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(StartObjectsFromList0BFBRecords + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(rd8((uint16_t)(StartObjectsFromList0BFBRecords + 2 + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    op_tya();
    op_clc();
    op_adc(0x04);
    op_pha();
    op_ldy(RD8(ZP->temp00));
    jsr_push(0xB620);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(RD8(ZP->temp01));
    jsr_push(0xB625);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_lda(RD8(ZP->temp02));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    op_ply();
    goto at_B602;
}

void abi_StartObjectsFromList0BFB(void)
{
    StartObjectsFromList0BFB();
}

/*
 * @routine 0BFB:646 SteerTowardPlayer0BFB
 * @run 70:B646
 * WHAT IT DOES: Plays the frame list at $B6A6, turns its angle OBJ_WORK_E by 1 toward the point $20 above the
 * player, raises its speed OBJ_WORK_F:OBJ_WORK_G by 2 while the low byte is below $80, and moves at that angle and
 * speed (Stage 07's overlay).
 * HOW: AngleToPlayerOffset, TurnAngleToward, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SteerTowardPlayer0BFB(uint8_t slot)
{
    op_lda(0xA6);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB651);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    op_lda(0xE0);
    WR8(ZP->temp0a, cpu.a);
    op_lda(0xFF);
    WR8(ZP->temp0b, cpu.a);
    jsr_push(0xB660);
    angle_exit_shim(AngleToPlayerOffset(slot));
    jsr_pop();
    op_pha();
    op_lda(0x01);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    WR8(ZP->temp01, cpu.a);
    op_pla();
    WR8(ZP->temp02, cpu.a);
    jsr_push(0xB670);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_cmp(0x00);
    if (flag_c()) {
        op_lda(RD8(OBJ->work_g[slot]));
        op_cmp(0x80);
        if (flag_c()) {
            goto at_B692;
        }
    }
    op_lda(RD8(OBJ->work_g[slot]));
    op_clc();
    op_adc(0x02);
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_adc(0x00);
    WR8(OBJ->work_f[slot], cpu.a);
at_B692:
    op_lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xB6A2);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xB6A5);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

void abi_SteerTowardPlayer0BFB(void)
{
    SteerTowardPlayer0BFB(cpu.x);
}

/*
 * @routine 0BFB:6B7 BackAwayFromPlayer0BFB
 * @run 70:B6B7
 * WHAT IT DOES: In substate 0 aims directly away from the player at speed $0080 for $14 frames (OBJ_WORK_H); then
 * moves at that angle and, when the count runs out, goes to state 4 with its speed cleared (Stage 07's overlay).
 * HOW: AngleToPlayer plus $80; SetVelocityFromAngle and ApplyVelocity; DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void BackAwayFromPlayer0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    if (flag_z()) {
        jsr_push(0xB6BF);
        angle_exit_shim(AngleToPlayer(slot));
        jsr_pop();
        op_clc();
        op_adc(0x80);
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(0x00);
        WR8(OBJ->work_f[slot], cpu.a);
        op_lda(0x80);
        WR8(OBJ->work_g[slot], cpu.a);
        op_lda(0x14);
        WR8(OBJ->work_h[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
        return;
    }
    op_lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xB6E8);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xB6EB);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->work_f[slot], 0);
        WR8(OBJ->work_g[slot], 0);
    }
}

void abi_BackAwayFromPlayer0BFB(void)
{
    BackAwayFromPlayer0BFB(cpu.x);
}

/*
 * @routine 0BFB:714 DefeatSoundsAndRecord0BFB
 * @run 70:B714
 * WHAT IT DOES: Sends sounds $FF and $B2, records the object's event, clears OBJ_PACE bits 3 to 6, sets OBJ_WORK_H
 * $40 and steps the substate on (Stage 07's overlay).
 * HOW: The calls in turn, then stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void DefeatSoundsAndRecord0BFB(uint8_t slot)
{
    op_lda(0xFF);
    {
        uint8_t command;

        jsr_push(0xB719);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    op_lda(0xB2);
    {
        uint8_t command;

        jsr_push(0xB71E);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xB721);
        if (!RecordObjectEvent(slot, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
    op_lda(RD8(OBJ->pace[slot]));
    op_and(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    op_lda(0x40);
    WR8(OBJ->work_h[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_DefeatSoundsAndRecord0BFB(void)
{
    DefeatSoundsAndRecord0BFB(cpu.x);
}

/*
 * @routine 0BFB:76A PieceOnCount0BFB
 * @run 70:B76A
 * WHAT IT DOES: When OBJ_WORK_H's low six bits match one of eight counts, starts kind $9E in a free slot from $30,
 * sounds $18 and gives it the X and Y speed of that count, moving it once (Stage 07's overlay).
 * HOW: A search of the counts; FindFreeSlotFrom30, StartObjectAtPlace; a two-byte record by the match.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PieceOnCount0BFBCounts = 0xB7A6,   /* the eight OBJ_WORK_H counts that start a piece */
    PieceOnCount0BFBSpeeds = 0xB7AE,   /* the piece's X speed high, Y speed high for each count */
};

void PieceOnCount0BFB(void)
{
    op_cly();
at_B76B:
    op_lda(RD8(OBJ->work_h[cpu.x]));
    op_and(0x3F);
    op_cmp(rd8((uint16_t)(PieceOnCount0BFBCounts + cpu.y)));
    if (!flag_z()) {
        op_iny();
        op_cpy(0x08);
        if (!flag_c()) {
            goto at_B76B;
        }
        return;
    }
    WR8(ZP->richter_object, cpu.x);
    WR8(ZP->hit_slot, cpu.y);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xB782);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x9E);
        jsr_push(0xB789);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(0x18);
        {
            uint8_t command;

            jsr_push(0xB78E);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(RD8(ZP->hit_slot));
        cpu.a = f_asl(cpu.a);
        op_tax();
        op_lda(rd8((uint16_t)(PieceOnCount0BFBSpeeds + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(PieceOnCount0BFBSpeeds + 1 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_sxy();
        jsr_push(0xB7A2);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_sxy();
        op_ldx(RD8(ZP->richter_object));
    }
}

void abi_PieceOnCount0BFB(void)
{
    PieceOnCount0BFB();
}

/*
 * @routine 0BFB:7D6 FrameByAngleUntilHit0BFB
 * @run 70:B7D6
 * WHAT IT DOES: Shows frame $0E plus a sixteenth of its angle to the player; once hit (OBJ_HIT_STATUS bit 7) sounds
 * $B1, stops its fall, sets OBJ_WORK_D $40 and steps the state on (Stage 07's overlay).
 * HOW: AngleToPlayer shifted down 4; a test of bit 7; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FrameByAngleUntilHit0BFB(uint8_t slot)
{
    jsr_push(0xB7D9);
    angle_exit_shim(AngleToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_clc();
    op_adc(0x0E);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(0xB1);
        {
            uint8_t command;

            jsr_push(0xB7EF);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
        op_lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_FrameByAngleUntilHit0BFB(void)
{
    FrameByAngleUntilHit0BFB(cpu.x);
}

/*
 * @routine 0BFC:004 FallDroppingPieces0BFC
 * @run 70:B804
 * WHAT IT DOES: Falls and counts OBJ_WORK_D down, removing itself at 0; at eight of the counts starts kind $9C in a
 * free slot from $30 with sound $18 (Stage 07's overlay).
 * HOW: ApplyGravityUnlimited, ApplyVelocityY, DEC; a search of the counts.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FallDroppingPieces0BFCCounts = 0xB831,   /* the eight OBJ_WORK_D counts that start a piece */
};

void FallDroppingPieces0BFC(uint8_t slot)
{
    jsr_push(0xB807);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    jsr_push(0xB80A);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_cly();
at_B813:
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(rd8((uint16_t)(FallDroppingPieces0BFCCounts + cpu.y)));
    if (!flag_z()) {
        op_iny();
        op_cpy(0x08);
        if (!flag_c()) {
            goto at_B813;
        }
        return;
    }
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xB824);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x9C);
        jsr_push(0xB82B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x18);
        {
            uint8_t command;

            jsr_push(0xB830);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
    }
}

void abi_FallDroppingPieces0BFC(void)
{
    FallDroppingPieces0BFC(cpu.x);
}

/*
 * @routine 0BFC:051 StartAngleByArg0BFC
 * @run 70:B851
 * WHAT IT DOES: Loads the object's record and sets OBJ_WORK_D, its angle, from a table by OBJ_START_ARG (Stage 07's
 * overlay).
 * HOW: LoadObjectRecord and a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartAngleByArg0BFCAngles = 0xB85E,   /* the starting OBJ_WORK_D for each OBJ_START_ARG */
};

void StartAngleByArg0BFC(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB854);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(rd8((uint16_t)(StartAngleByArg0BFCAngles + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_StartAngleByArg0BFC(void)
{
    StartAngleByArg0BFC(cpu.x);
}

/*
 * @routine 0BFC:061 OrbitSlot4C0BFC
 * @run 70:B861
 * WHAT IT DOES: Plays the frame list at $B8AA and circles slot $4C: takes its place, then moves at angle OBJ_WORK_D
 * (stepped by 2) at a speed of $24 plus a sine of OBJ_WORK_E scaled by $20; the hit box is on (2) only while that
 * speed is $30 or more (Stage 07's overlay).
 * HOW: CopyObjectPlaceBack, ScaleBySine, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void OrbitSlot4C0BFC(uint8_t slot)
{
    op_lda(0xAA);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB8);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB86C);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_ldy(0x4C);
    jsr_push(0xB871);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    WR8(ZP->temp00, 0);
    op_lda(0x20);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xB87D);
    scale_exit_shim(ScaleBySine(cpu.a));
    jsr_pop();
    RMW8(OBJ->work_e[slot], v, f_inc(v));
    op_lda(RD8(ZP->product.mid));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(ZP->product.hi));
    op_clc();
    op_adc(0x24);
    WR8(ZP->temp01, cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
    op_cmp(0x30);
    if (flag_c()) {
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
    }
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xB89D);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xB8A0);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    op_clc();
    op_adc(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_OrbitSlot4C0BFC(void)
{
    OrbitSlot4C0BFC(cpu.x);
}

/*
 * @routine 0BFC:0B5 FallFromRest0BFC
 * @run 70:B8B5
 * WHAT IT DOES: In substate 0 stops its fall, takes the hit box off and clears OBJ_LOOP_FLAGS bit 1; then falls
 * (Stage 07's overlay).
 * HOW: Two arms by the substate; ApplyGravityUnlimited and ApplyVelocityY.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallFromRest0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    if (flag_z()) {
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->hitbox_on[slot], 0);
        op_lda(RD8(OBJ->loop_flags[slot]));
        op_and(0xFD);
        WR8(OBJ->loop_flags[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
        return;
    }
    jsr_push(0xB8D2);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityY(slot));
}

void abi_FallFromRest0BFC(void)
{
    FallFromRest0BFC(cpu.x);
}

/*
 * @routine 0BFC:0D5 StartKind34OnTimer0BFC
 * @run 70:B8D5
 * WHAT IT DOES: In state 0 loads the record; then, unless slot $54 shows frame 4 or the scroll stands at exactly
 * $300, counts the 16-bit OBJ_WORK_D:OBJ_WORK_E down and at 0 starts kind $34 in a free slot from $30 and reloads
 * the count from a table by OBJ_START_ARG (Stage 07's overlay).
 * HOW: A 16-bit subtract; FindFreeSlotFrom30, StartObjectAtPlace; a word table.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
enum {
    StartKind34OnTimer0BFCWaits = 0xB927,   /* the count for each OBJ_START_ARG: OBJ_WORK_E, then OBJ_WORK_D */
};

void StartKind34OnTimer0BFC(uint8_t slot)
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
    op_lda(RD8(OBJ->frame[0x54]));
    op_cmp(0x04);
    if (!flag_z()) {
        op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
        op_cmp(0x03);
        if (flag_z()) {
            op_lda(RD8(SCROLL->x_lo[0x00]));
            if (flag_z()) {
                return;
            }
        }
        op_lda(RD8(OBJ->work_e[slot]));
        op_ora(RD8(OBJ->work_d[slot]));
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
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xB90E);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0x34);
            jsr_push(0xB915);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_lda(RD8(OBJ->start_arg[slot]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(StartKind34OnTimer0BFCWaits + 1 + cpu.y)));
            WR8(OBJ->work_d[slot], cpu.a);
            op_lda(rd8((uint16_t)(StartKind34OnTimer0BFCWaits + cpu.y)));
            WR8(OBJ->work_e[slot], cpu.a);
        }
    }
}

void abi_StartKind34OnTimer0BFC(void)
{
    StartKind34OnTimer0BFC(cpu.x);
}

/*
 * @routine 0BFC:25A PiecesBoxThenByState0BFC
 * @run 70:BA5A
 * WHAT IT DOES: Spawns the object's pieces when it is hit (SpawnPiecesWhenFlaggedHit), sizes its hit box by frame,
 * then runs the handler of its state (Stage 07's overlay).
 * HOW: Two calls, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PiecesBoxThenByState0BFCHandlers = 0xBA72,   /* the handler address for each state, low and high byte */
};

void PiecesBoxThenByState0BFC(void)
{
    call_routine(0xBA5D, 0xBA7C);
    call_routine(0xBA60, 0xBAAB);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(PiecesBoxThenByState0BFCHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(PiecesBoxThenByState0BFCHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_PiecesBoxThenByState0BFC(void)
{
    PiecesBoxThenByState0BFC();
}

/*
 * @routine 0BFC:358 WalkUntilProbeBit1_0BFC
 * @run 70:BB58
 * WHAT IT DOES: Plays the frame list at $BB88 and moves by its X speed; when the terrain probe ($22, 4) reports bit
 * 1, clears OBJ_WORK_B, OBJ_WORK_C, the animation and the substate and goes to state 3 (Stage 07's overlay).
 * HOW: StepObjectFrames, ApplyVelocityX, ProbeTerrain, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void WalkUntilProbeBit1_0BFC(uint8_t slot)
{
    op_lda(0x88);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBB63);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xBB66);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    op_lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    op_lda(0x22);
    jsr_push(0xBB6F);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x02);
    if (!flag_z()) {
        WR8(OBJ->work_c[slot], 0);
        WR8(OBJ->work_b[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->substate[slot], 0);
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_WalkUntilProbeBit1_0BFC(void)
{
    WalkUntilProbeBit1_0BFC(cpu.x);
}

/*
 * @routine 0BFC:444 HitThenByState0BFC
 * @run 70:BC44
 * WHAT IT DOES: Sends a hit object to state 6 ($BC67), then runs the handler of its state (Stage 07's overlay).
 * HOW: A call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    HitThenByState0BFCHandlers = 0xBC59,   /* the handler address for each state, low and high byte */
};

void HitThenByState0BFC(uint8_t slot)
{
    jsr_push(0xBC47);
    State6WhenHit0BFC(slot);
    jsr_pop();
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitThenByState0BFCHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(HitThenByState0BFCHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_HitThenByState0BFC(void)
{
    HitThenByState0BFC(cpu.x);
}

/*
 * @routine 0BFC:467 State6WhenHit0BFC
 * @run 70:BC67
 * WHAT IT DOES: Puts the object in state 6 once it is hit (OBJ_HIT_STATUS bit 7) (Stage 07's overlay).
 * HOW: A test of bit 7, a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State6WhenHit0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(0x06);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_State6WhenHit0BFC(void)
{
    State6WhenHit0BFC(cpu.x);
}

/*
 * @routine 0BFC:4CE FacePlayerPlayList0BFC
 * @run 70:BCCE
 * WHAT IT DOES: Turns the object to face the player and plays the frame list Y picks (Stage 07's overlay).
 * HOW: SideOfPlayer; a word table by Y; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object; Y the frame list's word offset. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
enum {
    FacePlayerPlayList0BFCLists = 0xBCE3,   /* frame list addresses, low and high byte */
};

void FacePlayerPlayList0BFC(uint8_t slot)
{
    jsr_push(0xBCD1);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(rd8((uint16_t)(FacePlayerPlayList0BFCLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FacePlayerPlayList0BFCLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
    }
}

void abi_FacePlayerPlayList0BFC(void)
{
    FacePlayerPlayList0BFC(cpu.x);
}

/*
 * @routine 0BFC:619 LandThenSubstate0BFC
 * @run 70:BE19
 * WHAT IT DOES: Falls and lands with height 6, stepping the substate on when it lands (Stage 07's overlay).
 * HOW: StopOnLanding, then INC on carry.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenSubstate0BFC(uint8_t slot)
{
    op_lda(0x06);
    jsr_push(0xBE1E);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandThenSubstate0BFC(void)
{
    LandThenSubstate0BFC(cpu.x);
}

/*
 * @routine 0BFC:63A BurstSixPieces0BFC
 * @run 70:BE3A
 * WHAT IT DOES: Starts six kind $3A pieces at the object's place in free slots from $30, each with its facing, and
 * its own OBJ_WORK_D, OBJ_WORK_E and 16-bit X and Y speeds; stops at the first search with no slot, then removes
 * the object (Stage 07's overlay).
 * HOW: A loop over eight-byte records; FindFreeSlotFrom30, StartObjectAtPlace, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BurstSixPieces0BFCPieces = 0xBE85,   /* OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low, two spare */
};

void BurstSixPieces0BFC(void)
{
    op_cly();
at_BE3B:
    WR8(ZP->hit_slot, cpu.y);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xBE40);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x3A);
        jsr_push(0xBE47);
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
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + 1 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + 2 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + 3 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + 4 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstSixPieces0BFCPieces + 5 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        op_ldx(RD8(ZP->richter_object));
        op_ldy(RD8(ZP->hit_slot));
        op_iny();
        op_cpy(0x06);
        if (!flag_c()) {
            goto at_BE3B;
        }
    }
    ClearObject(cpu.x);
}

void abi_BurstSixPieces0BFC(void)
{
    BurstSixPieces0BFC();
}

/*
 * @routine 0BFC:6B5 LeapAtPlayer0BFC
 * @run 70:BEB5
 * @stop BEF0
 * WHAT IT DOES: In state 0 loads the record, moves up 16 pixels and leaps at the player: X speed by his distance in
 * steps of 32 pixels, Y speed -$480, turned toward him; in later states flies on ($BF00) (Stage 07's overlay).
 * HOW: A word table by DistanceXToPlayer over 32; NegateVelocityX by SideOfPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LeapAtPlayer0BFCSpeeds = 0xBEF0,   /* the X speed by distance, low and high byte */
};

void LeapAtPlayer0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        ArcWithGravity24_0BFC(slot);
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBEBD);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0xF0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xBEC5);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    jsr_push(0xBEC8);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LeapAtPlayer0BFCSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LeapAtPlayer0BFCSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    jsr_push(0xBEE8);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    if (!flag_z()) {
        jsr_push(0xBEEF);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_LeapAtPlayer0BFC(void)
{
    LeapAtPlayer0BFC(cpu.x);
}

/*
 * @routine 0BFC:700 ArcWithGravity24_0BFC
 * @run 70:BF00
 * WHAT IT DOES: Plays the frame list at $BCFF, adds $24 to its Y speed and moves (Stage 07's overlay).
 * HOW: StepObjectFrames, a 16-bit add, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void ArcWithGravity24_0BFC(uint8_t slot)
{
    op_lda(0xBC);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xFF);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBF0B);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_clc();
    op_adc(0x24);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(RD8(OBJ->vy_hi[slot]));
    op_adc(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_ArcWithGravity24_0BFC(void)
{
    ArcWithGravity24_0BFC(cpu.x);
}

/*
 * @routine 0BFC:71F HopThenBecome9B_0BFC
 * @run 70:BF1F
 * WHAT IT DOES: In state 0 loads the record and turns its X speed toward the player; then counts OBJ_WORK_E down,
 * playing the frame list OBJ_WORK_D picks and falling, and at 0 becomes kind $9B in state 0 (Stage 07's overlay).
 * HOW: A word table by OBJ_WORK_D; StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    HopThenBecome9B_0BFCLists = 0xBF58,   /* frame list addresses by OBJ_WORK_D, low and high byte */
};

void HopThenBecome9B_0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBF27);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        jsr_push(0xBF2A);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        if (!flag_z()) {
            jsr_push(0xBF31);
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
    op_lda(rd8((uint16_t)(HopThenBecome9B_0BFCLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(HopThenBecome9B_0BFCLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBF52);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xBF55);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_HopThenBecome9B_0BFC(void)
{
    HopThenBecome9B_0BFC(cpu.x);
}

/*
 * @routine 0BFC:760 FrameChecksThenByState0BFC
 * @run 70:BF60
 * WHAT IT DOES: Tests the player's contact, the hit, and the time-slowing skip, returning when the frame is
 * skipped; then falls with its probe, sets the hit box X by frame and runs the handler of its state (Stage 07's
 * overlay).
 * HOW: The calls in turn, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FrameChecksThenByState0BFCHandlers = 0xBF84,   /* the handler address for each state, low and high byte */
};

void FrameChecksThenByState0BFC(void)
{
    jsr_push(0xBF63);
    StandOnBox14By20_0BFC();
    jsr_pop();
    jsr_push(0xBF66);
    State4WhenHit0BFC(cpu.x);
    jsr_pop();
    jsr_push(0xBF69);
    SkipFrameWhenSlowed0BFC(cpu.x);
    jsr_pop();
    if (flag_c()) {
        return;
    }
    jsr_push(0xBF6F);
    FallProbedBelow0BFC(cpu.x);
    jsr_pop();
    call_routine(0xBF72, 0xC03B);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FrameChecksThenByState0BFCHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(FrameChecksThenByState0BFCHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_FrameChecksThenByState0BFC(void)
{
    FrameChecksThenByState0BFC();
}

/*
 * @routine 0BFC:78E StandOnBox14By20_0BFC
 * @run 70:BF8E
 * WHAT IT DOES: Tests the player's contact with the object with width $14 and height $20 (Stage 07's overlay).
 * HOW: zTemp08 and zTemp09, then ContactWithPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the player stands on it.
 */
void StandOnBox14By20_0BFC(void)
{
    op_lda(0x14);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x20);
    WR8(ZP->temp09, cpu.a);
    run_routine(0xAA35);
}

void abi_StandOnBox14By20_0BFC(void)
{
    StandOnBox14By20_0BFC();
}

/*
 * @routine 0BFC:799 State4WhenHit0BFC
 * @run 70:BF99
 * WHAT IT DOES: Unless the object is in state 4, once it is hit (OBJ_HIT_STATUS bit 7) restarts its animation in
 * state 4 (Stage 07's overlay).
 * HOW: A compare, a test of bit 7, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State4WhenHit0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x04);
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_status[slot]));
        op_and(0x80);
        if (!flag_z()) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_State4WhenHit0BFC(void)
{
    State4WhenHit0BFC(cpu.x);
}

/*
 * @routine 0BFC:7B3 SkipFrameWhenSlowed0BFC
 * @run 70:BFB3
 * WHAT IT DOES: In states other than 0 and 4 with TimeSlowMode on, decides whether this frame is skipped from the
 * pattern byte for zWaveCounter's low three bits, row $18 of one table in mode 2 and of the other otherwise (Stage
 * 07's overlay).
 * HOW: Compares of the state and the mode; a table read by $18 plus the count.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: C set when the frame is skipped, clear when the object runs.
 */
enum {
    SkipFrameWhenSlowed0BFCMode1 = 0xA146,   /* the time-slowing pattern bytes read in the other modes; nonzero skips */
    SkipFrameWhenSlowed0BFCMode2 = 0xA166,   /* the time-slowing pattern bytes read in mode 2; nonzero skips */
};

void SkipFrameWhenSlowed0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        op_cmp(0x04);
        if (!flag_z()) {
            op_lda(RD8(PVARS->time_slow_mode));
            if (!flag_z()) {
                op_cmp(0x02);
                if (!flag_z()) {
                    goto at_BFD4;
                }
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0BFCMode2 + cpu.y)));
                if (flag_z()) {
                    goto at_BFE1;
                }
at_BFD2:
                op_sec();
                return;
at_BFD4:
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0BFCMode1 + cpu.y)));
                if (!flag_z()) {
                    goto at_BFD2;
                }
            }
        }
    }
at_BFE1:
    op_clc();
}

void abi_SkipFrameWhenSlowed0BFC(void)
{
    SkipFrameWhenSlowed0BFC(cpu.x);
}

/*
 * @routine 0BFC:7E3 FallProbedBelow0BFC
 * @run 70:BFE3
 * WHAT IT DOES: In a state other than 0 with its Y high byte 0, moves down, falls and probes the floor ($09, $0A)
 * at a point OBJ_START_ARG times 16 pixels below its place, stopping its fall on floor (Stage 07's overlay).
 * HOW: Y raised by 16 times OBJ_START_ARG and lowered back around ApplyVelocityY, ApplyGravity and ProbeTerrain.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void FallProbedBelow0BFC(uint8_t slot)
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
            jsr_push(0xC00A);
            axis_move_exit_shim(ApplyVelocityY(slot));
            jsr_pop();
            jsr_push(0xC00D);
            gravity_exit_shim(ApplyGravity(slot));
            jsr_pop();
            op_lda(0x0A);
            WR8(ZP->product.mid, cpu.a);
            op_lda(0x09);
            jsr_push(0xC016);
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

void abi_FallProbedBelow0BFC(void)
{
    FallProbedBelow0BFC(cpu.x);
}

/*
 * @routine 0BFD:0B1 SideStepByArg0BFD
 * @run 71:C0B1
 * WHAT IT DOES: Sets the object's X speed by bit 0 of OBJ_START_ARG and goes on to its one-frame offset ($C0BD)
 * (Stage 07's overlay).
 * HOW: A table read, then the fall into $C0BD.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SideStepByArg0BFDSpeeds = 0xC0D4,   /* the X speed high byte for each value of OBJ_START_ARG's bit 0 */
};

void SideStepByArg0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(SideStepByArg0BFDSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    /* falls into $C0BD */
    OffsetUpByArg0BFD(slot);
}

void abi_SideStepByArg0BFD(void)
{
    SideStepByArg0BFD(cpu.x);
}

/*
 * @routine 0BFD:0BD OffsetUpByArg0BFD
 * @run 71:C0BD
 * WHAT IT DOES: Moves the object once by its X speed and up by 16 times OBJ_START_ARG, then clears both speeds'
 * high bytes (Stage 07's overlay).
 * HOW: A negated shift into the Y speed; ApplyVelocity; two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void OffsetUpByArg0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_eor(0xFF);
    cpu.a = f_inc(cpu.a);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xC0CD);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_OffsetUpByArg0BFD(void)
{
    OffsetUpByArg0BFD(cpu.x);
}

/*
 * @routine 0BFD:0D6 RandomWaitDE0BFD
 * @run 71:C0D6
 * WHAT IT DOES: Sets OBJ_WORK_D:OBJ_WORK_E to one of four values chosen at random (Stage 07's overlay).
 * HOW: NextRandom's low two bits index a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RandomWaitDE0BFDValues = 0xC0EA,   /* four values: OBJ_WORK_E, then OBJ_WORK_D */
};

void RandomWaitDE0BFD(uint8_t slot)
{
    jsr_push(0xC0D9);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RandomWaitDE0BFDValues + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(rd8((uint16_t)(RandomWaitDE0BFDValues + 1 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_RandomWaitDE0BFD(void)
{
    RandomWaitDE0BFD(cpu.x);
}

/*
 * @routine 0BFD:0F2 RandomSpeedFG0BFD
 * @run 71:C0F2
 * WHAT IT DOES: Sets OBJ_WORK_F:OBJ_WORK_G to one of four values chosen at random (Stage 07's overlay).
 * HOW: NextRandom's low two bits index a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RandomSpeedFG0BFDValues = 0xC106,   /* four values: OBJ_WORK_G, then OBJ_WORK_F */
};

void RandomSpeedFG0BFD(uint8_t slot)
{
    jsr_push(0xC0F5);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RandomSpeedFG0BFDValues + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(rd8((uint16_t)(RandomSpeedFG0BFDValues + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_RandomSpeedFG0BFD(void)
{
    RandomSpeedFG0BFD(cpu.x);
}

/*
 * @routine 0BFD:16F ListByWorkHThenState1_0BFD
 * @run 71:C16F
 * WHAT IT DOES: Plays the frame list chosen by OBJ_WORK_H's bit 0; when it ends steps OBJ_WORK_H on and goes to
 * state 1 (Stage 07's overlay).
 * HOW: A word table; StepObjectFrames; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ListByWorkHThenState1_0BFDLists = 0xC18E,   /* the two frame list addresses, low and high byte */
};

void ListByWorkHThenState1_0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ListByWorkHThenState1_0BFDLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(ListByWorkHThenState1_0BFDLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC183);
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

void abi_ListByWorkHThenState1_0BFD(void)
{
    ListByWorkHThenState1_0BFD(cpu.x);
}

/*
 * @routine 0BFD:1BE PickThrow0BFD
 * @run 71:C1BE
 * WHAT IT DOES: Goes to state 1 when the player is not on the side the object faces (OBJ_WORK_H's bit 0 flips it);
 * otherwise picks OBJ_WORK_I at random from 16, OBJ_WORK_K at random from four for that pick, OBJ_WORK_J by
 * OBJ_WORK_H's bit 0, and steps the substate on (Stage 07's overlay).
 * HOW: SideOfPlayer compared with the facing; NextRandom twice; three tables.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PickThrow0BFDPicks = 0xC206,   /* the 16 OBJ_WORK_I values chosen from */
    PickThrow0BFDCounts = 0xC216,   /* four OBJ_WORK_K values for each OBJ_WORK_I */
    PickThrow0BFDWorkJ = 0xC222,   /* OBJ_WORK_J for each value of OBJ_WORK_H's bit 0 */
};

void PickThrow0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->facing_left[slot]));
    op_eor(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    WR8(ZP->temp08, cpu.a);
    jsr_push(0xC1CB);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    op_cmp(RD8(ZP->temp08));
    if (!flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    jsr_push(0xC1DA);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(PickThrow0BFDPicks + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    jsr_push(0xC1EA);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(PickThrow0BFDCounts + cpu.y)));
    WR8(OBJ->work_k[slot], cpu.a);
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(PickThrow0BFDWorkJ + cpu.y)));
    WR8(OBJ->work_j[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_PickThrow0BFD(void)
{
    PickThrow0BFD(cpu.x);
}

/*
 * @routine 0BFD:224 ThrowKindByPick0BFD
 * @run 71:C224
 * WHAT IT DOES: Starts the kind OBJ_WORK_I picks in a free slot from $30 at the object's place, with OBJ_WORK_D the
 * pick, the object's facing, the object's slot in OBJ_WORK_F and X speed high $F8, or $EE with its facing turned
 * when OBJ_WORK_J is set, and steps the substate on; with no free slot goes back to state 1 ($C2B6) (Stage 07's
 * overlay).
 * HOW: FindFreeSlotFrom30; a table by OBJ_WORK_I; StartObjectAtPlace; stores to slot Y.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ThrowKindByPick0BFDKinds = 0xC266,   /* the kind started for each OBJ_WORK_I */
};

void ThrowKindByPick0BFD(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC227);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        Stand5COr5F_0BFD(slot);
        return;
    }
    WR8(ZP->hit_slot, cpu.y);
    op_ldy(RD8(OBJ->work_i[slot]));
    op_lda(rd8((uint16_t)(ThrowKindByPick0BFDKinds + cpu.y)));
    op_ldy(RD8(ZP->hit_slot));
    jsr_push(0xC239);
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

void abi_ThrowKindByPick0BFD(void)
{
    ThrowKindByPick0BFD(cpu.x);
}

/*
 * @routine 0BFD:269 ListByPickAndJ0BFD
 * @run 71:C269
 * WHAT IT DOES: Plays the frame list picked by twice OBJ_WORK_I plus OBJ_WORK_J, stepping the substate on when it
 * ends (Stage 07's overlay).
 * HOW: A table read by the sum; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ListByPickAndJ0BFDLists = 0xC285,   /* frame list addresses, low and high byte */
};

void ListByPickAndJ0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_j[slot]));
    op_tay();
    op_lda(rd8((uint16_t)(ListByPickAndJ0BFDLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(ListByPickAndJ0BFDLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC27F);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_ListByPickAndJ0BFD(void)
{
    ListByPickAndJ0BFD(cpu.x);
}

/*
 * @routine 0BFD:2B6 Stand5COr5F_0BFD
 * @run 71:C2B6
 * WHAT IT DOES: Puts the object in state 1 with frame $5C, or $5F when OBJ_WORK_J is set (Stage 07's overlay).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Stand5COr5F_0BFD(uint8_t slot)
{
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    op_lda(0x5C);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(0x5F);
        WR8(OBJ->frame[slot], cpu.a);
    }
}

void abi_Stand5COr5F_0BFD(void)
{
    Stand5COr5F_0BFD(cpu.x);
}

/*
 * @routine 0BFD:2CB NumberKind3CThenClear0BFD
 * @run 71:C2CB
 * WHAT IT DOES: Plays the frame list chosen by OBJ_WORK_H's bit 0; when it ends numbers each kind $3C below state 4
 * in slots OBJ_WORK_M to OBJ_WORK_N (OBJ_START_ARG 0 up, OBJ_WORK_M the first of them) and removes itself (Stage
 * 07's overlay).
 * HOW: A word table; StepObjectFrames; a loop over the slots; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    NumberKind3CThenClear0BFDLists = 0xC316,   /* the two frame list addresses, low and high byte */
};

void NumberKind3CThenClear0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(NumberKind3CThenClear0BFDLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(NumberKind3CThenClear0BFDLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC2DF);
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
            op_cmp(0x3C);
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
        jsr_push(0xC315);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_NumberKind3CThenClear0BFD(void)
{
    NumberKind3CThenClear0BFD(cpu.x);
}

/*
 * @routine 0BFD:348 LaunchWithKind3F0BFD
 * @run 71:C348
 * WHAT IT DOES: Loads the record, sets OBJ_WORK_E $28, moves once by its X speed (turned by facing), takes X and Y
 * speeds from a record by OBJ_WORK_D and facing, and starts kind $3F in a free slot from $50 with its OBJ_WORK_F
 * (Stage 07's overlay).
 * HOW: A four-byte record at eight times OBJ_WORK_D plus four times the facing; FindFreeSlotFrom50,
 * StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    LaunchWithKind3F0BFDSpeeds = 0xC395,   /* X speed low, high, Y speed low, high per record */
};

void LaunchWithKind3F0BFD(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC34B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x28);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xC358);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xC35B);
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
    op_lda(rd8((uint16_t)(LaunchWithKind3F0BFDSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchWithKind3F0BFDSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchWithKind3F0BFDSpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchWithKind3F0BFDSpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC387);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x3F);
        jsr_push(0xC38E);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
}

void abi_LaunchWithKind3F0BFD(void)
{
    LaunchWithKind3F0BFD(cpu.x);
}

/*
 * @routine 0BFD:3A5 WaitThenBoxOn0BFD
 * @run 71:C3A5
 * WHAT IT DOES: Counts OBJ_WORK_E down; at 0 sounds $25, turns the hit box on (2), restarts the animation and steps
 * the state on; then follows its owner ($C3C0) (Stage 07's overlay).
 * HOW: DEC, stores, then the call.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void WaitThenBoxOn0BFD(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x25);
        {
            uint8_t command;

            jsr_push(0xC3AF);
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
    FollowOwnerY0BFD(slot);
}

void abi_WaitThenBoxOn0BFD(void)
{
    WaitThenBoxOn0BFD(cpu.x);
}

/*
 * @routine 0BFD:3C0 FollowOwnerY0BFD
 * @run 71:C3C0
 * WHAT IT DOES: Takes the Y low byte of the slot in OBJ_WORK_F, and removes itself once that slot is in state 4
 * (Stage 07's overlay).
 * HOW: A copy, a compare, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowOwnerY0BFD(uint8_t slot)
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

void abi_FollowOwnerY0BFD(void)
{
    FollowOwnerY0BFD(cpu.x);
}

/*
 * @routine 0BFD:3D4 FlyByListWorkD0BFD
 * @run 71:C3D4
 * WHAT IT DOES: Plays the frame list OBJ_WORK_D picks and moves; with OBJ_WORK_D 1 its Y speed grows by $18 a frame
 * (Stage 07's overlay).
 * HOW: A word table; StepObjectFrames, ApplyVelocity, a 16-bit add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FlyByListWorkD0BFDLists = 0xC402,   /* frame list addresses by OBJ_WORK_D, low and high byte */
};

void FlyByListWorkD0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlyByListWorkD0BFDLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FlyByListWorkD0BFDLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC3E6);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xC3E9);
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

void abi_FlyByListWorkD0BFD(void)
{
    FlyByListWorkD0BFD(cpu.x);
}

/*
 * @routine 0BFD:42E StartWithKind3FAtOne0BFD
 * @run 71:C42E
 * WHAT IT DOES: Loads the record, sets OBJ_WORK_E $40, moves once by its X speed (turned by facing), starts kind
 * $3F in a free slot from $50 with OBJ_WORK_D 1 and its OBJ_WORK_F, then moves again by an X speed by its facing
 * (Stage 07's overlay).
 * HOW: NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace; a table by the facing.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StartWithKind3FAtOne0BFDSpeeds = 0xC463,   /* the X speed high byte for each facing */
};

void StartWithKind3FAtOne0BFD(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC431);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xC43E);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xC441);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC444);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x3F);
        jsr_push(0xC44B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(StartWithKind3FAtOne0BFDSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    jsr_push(0xC462);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

void abi_StartWithKind3FAtOne0BFD(void)
{
    StartWithKind3FAtOne0BFD(cpu.x);
}

/*
 * @routine 0BFD:47D AnimateBoxedTimed0BFD
 * @run 71:C47D
 * WHAT IT DOES: Plays the frame list at $C494 with the hit box of each frame, and removes itself when OBJ_WORK_E
 * counts down to 0 (Stage 07's overlay).
 * HOW: StepObjectFrames, HitBoxByFrame0BFD, DEC, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AnimateBoxedTimed0BFD(void)
{
    op_lda(0xC4);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x94);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC488);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    call_routine(0xC48B, 0xC4B7);
    RMW8(OBJ->work_e[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
}

void abi_AnimateBoxedTimed0BFD(void)
{
    AnimateBoxedTimed0BFD();
}

/*
 * @routine 0BFD:4F5 TimedListFollowOwner0BFD
 * @run 71:C4F5
 * WHAT IT DOES: In state 0 loads the record, sets sprite set OBJ_WORK_D plus 3 and OBJ_WORK_E $28; then plays the
 * frame list OBJ_WORK_D picks, removes itself when OBJ_WORK_E runs out, and otherwise follows its owner ($C3C0)
 * (Stage 07's overlay).
 * HOW: Two arms by the state; a word table; StepObjectFrames; DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    TimedListFollowOwner0BFDLists = 0xC529,   /* frame list addresses by OBJ_WORK_D, low and high byte */
};

void TimedListFollowOwner0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xC4FD);
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
    op_lda(rd8((uint16_t)(TimedListFollowOwner0BFDLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(TimedListFollowOwner0BFDLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC51E);
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
    FollowOwnerY0BFD(slot);
}

void abi_TimedListFollowOwner0BFD(void)
{
    TimedListFollowOwner0BFD(cpu.x);
}

/*
 * @routine 0BFD:561 State4SoundWhenHit0BFD
 * @run 71:C561
 * WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 4, sounds $B0 and puts it in state
 * 4, substate 0 (Stage 07's overlay).
 * HOW: A test of bit 7, a compare, SoundCommand, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void State4SoundWhenHit0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[slot]));
        op_cmp(0x04);
        if (!flag_c()) {
            op_lda(0xB0);
            {
                uint8_t command;

                jsr_push(0xC574);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State4SoundWhenHit0BFD(void)
{
    State4SoundWhenHit0BFD(cpu.x);
}

/*
 * @routine 0BFD:5C6 StartNineLinks0BFD
 * @run 71:C5C6
 * WHAT IT DOES: Loads the record, sets OBJ_WORK_O $80, fills the overlay's 128-byte buffer with $80, and starts
 * kind $41 in the nine slots after the object, numbered 0 to 8 in OBJ_WORK_K (Stage 07's overlay).
 * HOW: A fill loop; a loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartNineLinks0BFDBuffer = 0xC5F7,   /* 128 bytes the routine fills with $80; the links' angles are read from it */
};

void StartNineLinks0BFD(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC5C9);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x80);
    WR8(OBJ->work_o[slot], cpu.a);
    op_cly();
    do {
        op_lda(0x80);
        wr8((uint16_t)(StartNineLinks0BFDBuffer + cpu.y), cpu.a);
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
        jsr_push(0xC5E5);
        ClearObjectInY(cpu.y);
        jsr_pop();
        op_lda(0x41);
        jsr_push(0xC5EA);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(ZP->temp01));
        WR8(OBJ->work_k[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_inc(v));
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
}

void abi_StartNineLinks0BFD(void)
{
    StartNineLinks0BFD(cpu.x);
}

/*
 * @routine 0BFD:6CC AngleFromNinthToPlayer0BFD
 * @run 71:C6CC
 * WHAT IT DOES: Gives the angle to the player from the slot nine after the object (Stage 07's overlay).
 * HOW: X plus 9, AngleToPlayer, X back.
 * WAITS: no.
 * Entry: X the object. Exit: A the angle; X the object.
 */
void AngleFromNinthToPlayer0BFD(void)
{
    WR8(ZP->richter_object, cpu.x);
    op_txa();
    op_clc();
    op_adc(0x09);
    op_tax();
    jsr_push(0xC6D6);
    angle_exit_shim(AngleToPlayer(cpu.x));
    jsr_pop();
    op_ldx(RD8(ZP->richter_object));
}

void abi_AngleFromNinthToPlayer0BFD(void)
{
    AngleFromNinthToPlayer0BFD();
}

/*
 * @routine 0BFD:6D9 ScaleByLinkRow0BFD
 * @run 71:C6D9
 * WHAT IT DOES: Scales the cosine of OBJ_WORK_K by the size in the row OBJ_WORK_M picks, and leaves that row's
 * other word in zMultiplier (Stage 07's overlay).
 * HOW: A four-byte row at four times OBJ_WORK_M; ScaleByCosine.
 * WAITS: no.
 * Entry: X the object. Exit: zProduct the scaled cosine; zMultiplierLo/Hi the row's first word; Y and zHitSlot the
 * row's offset.
 */
enum {
    ScaleByLinkRow0BFDRows = 0xC6BC,   /* per row: the multiplier low, high, then the cosine's scale low, high */
};

void ScaleByLinkRow0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_m[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    WR8(ZP->hit_slot, cpu.y);
    op_lda(rd8((uint16_t)(ScaleByLinkRow0BFDRows + 2 + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ScaleByLinkRow0BFDRows + 3 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_k[slot]));
    jsr_push(0xC6F1);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_ldy(RD8(ZP->hit_slot));
    op_lda(rd8((uint16_t)(ScaleByLinkRow0BFDRows + cpu.y)));
    WR8(ZP->multiplier.lo, cpu.a);
    op_lda(rd8((uint16_t)(ScaleByLinkRow0BFDRows + 1 + cpu.y)));
    WR8(ZP->multiplier.hi, cpu.a);
}

void abi_ScaleByLinkRow0BFD(void)
{
    ScaleByLinkRow0BFD(cpu.x);
}

/*
 * @routine 0BFD:6FE NextStateOnTimer0BFD
 * @run 71:C6FE
 * WHAT IT DOES: Counts OBJ_WORK_O down; when it runs out reloads it with one of four waits at random, and takes the
 * next state of a sixteen-step pattern by OBJ_WORK_N, substate 0, stepping OBJ_WORK_N on (Stage 07's overlay).
 * HOW: NextRandom; a table by OBJ_WORK_N's low four bits indexes the states.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    NextStateOnTimer0BFDWaits = 0xC72A,   /* the four waits chosen from */
    NextStateOnTimer0BFDSteps = 0xC72E,   /* the state index for each of the 16 steps */
    NextStateOnTimer0BFDStates = 0xC73E,   /* the state for each index */
};

void NextStateOnTimer0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_o[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_o[slot], v, f_dec(v));
        return;
    }
    jsr_push(0xC70A);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(NextStateOnTimer0BFDWaits + cpu.y)));
    WR8(OBJ->work_o[slot], cpu.a);
    op_lda(RD8(OBJ->work_n[slot]));
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(NextStateOnTimer0BFDSteps + cpu.y)));
    op_tay();
    op_lda(rd8((uint16_t)(NextStateOnTimer0BFDStates + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    RMW8(OBJ->work_n[slot], v, f_inc(v));
}

void abi_NextStateOnTimer0BFD(void)
{
    NextStateOnTimer0BFD(cpu.x);
}

/*
 * @routine 0BFD:741 DragChainAndMove0BFD
 * @run 71:C741
 * WHAT IT DOES: Moves the chain of the eight slots after the object: each takes the place of the slot after it and
 * moves by its speed; then the object takes the next slot's place, moves left by X speed high $F8, and moves at
 * angle OBJ_WORK_D plus $80 at speed OBJ_WORK_F:OBJ_WORK_G (Stage 07's overlay).
 * HOW: A loop down the slots of CopyObjectPlaceBack and ApplyVelocity; SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DragChainAndMove0BFD(void)
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
        jsr_push(0xC74E);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
        jsr_pop();
        jsr_push(0xC751);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_dex();
        op_cpx(RD8(ZP->richter_object));
    } while (!flag_z());
    op_txa();
    op_tay();
    op_iny();
    jsr_push(0xC75C);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
    jsr_pop();
    op_lda(0xF8);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    jsr_push(0xC764);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(0x80);
    jsr_push(0xC777);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
    jsr_pop();
    jsr_push(0xC77A);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
}

void abi_DragChainAndMove0BFD(void)
{
    DragChainAndMove0BFD();
}

/*
 * @routine 0BFD:7DA StartKindC2ThenState1_0BFD
 * @run 71:C7DA
 * WHAT IT DOES: Starts kind $C2 in a free slot from $30 at the object's place with the object's slot in its
 * OBJ_WORK_D, and puts the object in state 1 (Stage 07's overlay).
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void StartKindC2ThenState1_0BFD(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC7DD);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0xC2);
        jsr_push(0xC7E4);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_txa();
        WR8(OBJ->work_d[cpu.y], cpu.a);
    }
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_StartKindC2ThenState1_0BFD(void)
{
    StartKindC2ThenState1_0BFD(cpu.x);
}

/*
 * @routine 0BFE:043 TurnToZeroBy2_0BFE
 * @run 71:C843
 * WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D toward 0 by 2, kept even; steps the substate
 * on when it is there (Stage 07's overlay).
 * HOW: DEC; TurnAngleToward; AND #$FE and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnToZeroBy2_0BFE(uint8_t slot)
{
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    WR8(ZP->temp02, 0);
    op_lda(0x02);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC854);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    op_and(0xFE);
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnToZeroBy2_0BFE(void)
{
    TurnToZeroBy2_0BFE(cpu.x);
}

/*
 * @routine 0BFE:0A5 TurnTowardWorkH0BFE
 * @run 71:C8A5
 * WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H less $80, and steps the substate on when
 * OBJ_WORK_I counts down to 0 (Stage 07's overlay).
 * HOW: TurnAngleToward; DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnTowardWorkH0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_sec();
    op_sbc(0x80);
    WR8(ZP->temp02, cpu.a);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC8B9);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnTowardWorkH0BFE(void)
{
    TurnTowardWorkH0BFE(cpu.x);
}

/*
 * @routine 0BFE:0C5 TurnToZeroThenState1_0BFE
 * @run 71:C8C5
 * WHAT IT DOES: Turns the angle OBJ_WORK_D toward 0 by 8; once there goes to state 1, substate 0, with OBJ_WORK_E,
 * OBJ_WORK_K and OBJ_WORK_L cleared (Stage 07's overlay).
 * HOW: TurnAngleToward, a compare, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnToZeroThenState1_0BFE(uint8_t slot)
{
    WR8(ZP->temp02, 0);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC8D3);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->work_e[slot], 0);
        WR8(OBJ->work_k[slot], 0);
        WR8(OBJ->work_l[slot], 0);
    }
}

void abi_TurnToZeroThenState1_0BFE(void)
{
    TurnToZeroThenState1_0BFE(cpu.x);
}

/*
 * @routine 0BFE:140 PieceOnCount0BFE
 * @run 71:C940
 * WHAT IT DOES: When OBJ_WORK_O's low five bits match one of eight counts, starts kind $9C in a free slot from $30
 * with sound $15 (Stage 07's overlay).
 * HOW: A search of the counts; FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PieceOnCount0BFECounts = 0xC961,   /* the eight OBJ_WORK_O counts that start a piece */
};

void PieceOnCount0BFE(uint8_t slot)
{
    op_cly();
at_C941:
    op_lda(RD8(OBJ->work_o[slot]));
    op_and(0x1F);
    op_cmp(rd8((uint16_t)(PieceOnCount0BFECounts + cpu.y)));
    if (!flag_z()) {
        op_iny();
        op_cpy(0x08);
        if (!flag_c()) {
            goto at_C941;
        }
        return;
    }
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xC954);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x9C);
        jsr_push(0xC95B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x15);
        {
            uint8_t command;

            jsr_push(0xC960);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
    }
}

void abi_PieceOnCount0BFE(void)
{
    PieceOnCount0BFE(cpu.x);
}

/*
 * @routine 0BFE:183 AimLinkFromBuffer0BFE
 * @run 71:C983
 * WHAT IT DOES: Unless OBJ_WORK_K is 8, takes its angle OBJ_WORK_D from the overlay's buffer at the entry for its
 * number OBJ_WORK_K, eases its speed toward the target (EaseWorkFToTarget0BFD) and sets its velocity from that
 * angle and speed (Stage 07's overlay).
 * HOW: Two table reads; the call; SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AimLinkFromBuffer0BFEBuffer = 0xC5F7,   /* the 128-byte buffer of angles StartNineLinks0BFD fills */
    AimLinkFromBuffer0BFEOffsets = 0xC9A9,   /* the buffer offset for each OBJ_WORK_K */
};

void AimLinkFromBuffer0BFE(void)
{
    op_lda(RD8(OBJ->work_k[cpu.x]));
    op_cmp(0x08);
    if (!flag_z()) {
        op_tay();
        op_lda(rd8((uint16_t)(AimLinkFromBuffer0BFEOffsets + cpu.y)));
        op_tay();
        op_lda(rd8((uint16_t)(AimLinkFromBuffer0BFEBuffer + cpu.y)));
        WR8(OBJ->work_d[cpu.x], cpu.a);
        call_routine(0xC998, 0xC77B);
        op_lda(RD8(OBJ->work_g[cpu.x]));
        WR8(ZP->temp00, cpu.a);
        op_lda(RD8(OBJ->work_f[cpu.x]));
        WR8(ZP->temp01, cpu.a);
        op_lda(RD8(OBJ->work_d[cpu.x]));
        jsr_push(0xC9A8);
        scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
        jsr_pop();
    }
}

void abi_AimLinkFromBuffer0BFE(void)
{
    AimLinkFromBuffer0BFE();
}

/*
 * @routine 0BFE:1B9 EaseThenBySubstate0BFE
 * @run 71:C9B9
 * WHAT IT DOES: Eases the speed OBJ_WORK_F:OBJ_WORK_G toward its target, then goes on to aim and run by the
 * substate ($C9C6) (Stage 07's overlay).
 * HOW: EaseWorkFToTarget0BFD, two loads, then the fall into $C9C6.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void EaseThenBySubstate0BFE(void)
{
    call_routine(0xC9BC, 0xC77B);
    op_lda(RD8(OBJ->work_g[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    /* falls into $C9C6 */
    AimThenBySubstate0BFE(cpu.x);
}

void abi_EaseThenBySubstate0BFE(void)
{
    EaseThenBySubstate0BFE();
}

/*
 * @routine 0BFE:1C6 AimThenBySubstate0BFE
 * @run 71:C9C6
 * WHAT IT DOES: Sets the velocity from the angle OBJ_WORK_D at the speed in zTemp00/01, then runs the handler of
 * its substate (Stage 07's overlay).
 * HOW: SetVelocityFromAngle, then a jump table by the substate.
 * WAITS: no.
 * Entry: X the object; zTemp00/01 the speed. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AimThenBySubstate0BFEHandlers = 0xC9DE,   /* the handler address for each substate, low and high byte */
};

void AimThenBySubstate0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xC9CC);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(AimThenBySubstate0BFEHandlers + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(AimThenBySubstate0BFEHandlers + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_AimThenBySubstate0BFE(void)
{
    AimThenBySubstate0BFE(cpu.x);
}

/*
 * @routine 0BFE:1FC TurnToTargetByK0BFE
 * @run 71:C9FC
 * WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2, kept even, toward the target OBJ_WORK_K
 * picks; steps the substate on when it is there (Stage 07's overlay).
 * HOW: A table read; TurnAngleToward; AND #$FE and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    TurnToTargetByK0BFETargets = 0xCA20,   /* the target angle for each OBJ_WORK_K */
};

void TurnToTargetByK0BFE(uint8_t slot)
{
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    op_ldy(RD8(OBJ->work_k[slot]));
    op_lda(rd8((uint16_t)(TurnToTargetByK0BFETargets + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    op_lda(0x02);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xCA13);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    op_and(0xFE);
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnToTargetByK0BFE(void)
{
    TurnToTargetByK0BFE(cpu.x);
}

/*
 * @routine 0BFE:23C TurnTowardHThenState1_0BFE
 * @run 71:CA3C
 * WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H; when OBJ_WORK_I counts down to 0 clears
 * OBJ_WORK_J and OBJ_WORK_G and goes to state 1, substate 0 (Stage 07's overlay).
 * HOW: TurnAngleToward; DEC; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnTowardHThenState1_0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    WR8(ZP->temp02, cpu.a);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xCA4D);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->work_j[slot], 0);
        WR8(OBJ->work_g[slot], 0);
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
}

void abi_TurnTowardHThenState1_0BFE(void)
{
    TurnTowardHThenState1_0BFE(cpu.x);
}

/*
 * @routine 0BFE:2CD LandOrClearLow0BFE
 * @run 71:CACD
 * WHAT IT DOES: Removes the object when its Y high byte is not 0; otherwise falls and lands with height 0, and on
 * landing sets OBJ_WORK_M $20 and steps the substate on (Stage 07's overlay).
 * HOW: A test of Y high; ClearObject or StopOnLanding.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandOrClearLow0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->y_hi[slot]));
    if (!flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(0x00);
    jsr_push(0xCADA);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x20);
        WR8(OBJ->work_m[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandOrClearLow0BFE(void)
{
    LandOrClearLow0BFE(cpu.x);
}

/*
 * @routine 0BFE:2E5 FlickerThenBecomeL0BFE
 * @run 71:CAE5
 * WHAT IT DOES: Counts OBJ_WORK_M down, showing one of two frames by its bit 0; at 0 moves up 4 pixels, sets Y
 * speed -1 and becomes the kind in OBJ_WORK_L, state 0 (Stage 07's overlay).
 * HOW: DEC; a table read; ApplyVelocityY; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlickerThenBecomeL0BFEFrames = 0xCB11,   /* the frame for each value of OBJ_WORK_M's bit 0 */
};

void FlickerThenBecomeL0BFE(uint8_t slot)
{
    RMW8(OBJ->work_m[slot], v, f_dec(v));
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_m[slot]));
        op_and(0x01);
        op_tay();
        op_lda(rd8((uint16_t)(FlickerThenBecomeL0BFEFrames + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    op_lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xCAFF);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(RD8(OBJ->work_l[slot]));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_FlickerThenBecomeL0BFE(void)
{
    FlickerThenBecomeL0BFE(cpu.x);
}

/*
 * @routine 0BFE:32D LoadRecordWorkF8E9_0BFE
 * @run 71:CB2D
 * WHAT IT DOES: Loads the object's record and sets OBJ_WORK_F 8 and OBJ_WORK_E 9 (Stage 07's overlay).
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordWorkF8E9_0BFE(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xCB30);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x08);
    WR8(OBJ->work_f[slot], cpu.a);
    op_lda(0x09);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_LoadRecordWorkF8E9_0BFE(void)
{
    LoadRecordWorkF8E9_0BFE(cpu.x);
}

/*
 * @routine 0BFE:33B StepDownOwnerChain0BFE
 * @run 71:CB3B
 * WHAT IT DOES: Removes itself once the slot in OBJ_WORK_D is in state 4; otherwise takes the place of the slot
 * OBJ_WORK_E after that one, stepping OBJ_WORK_E down every 8 frames (OBJ_WORK_F) and the state on when it reaches
 * 0 (Stage 07's overlay).
 * HOW: A compare; CopyObjectPlaceBack; two counters.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void StepDownOwnerChain0BFE(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_d[slot]));
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x04);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    op_clc();
    op_adc(RD8(OBJ->work_e[slot]));
    op_tay();
    jsr_push(0xCB53);
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

void abi_StepDownOwnerChain0BFE(void)
{
    StepDownOwnerChain0BFE(cpu.x);
}

/*
 * @routine 0BFE:366 ShotFromOwner0BFE
 * @run 71:CB66
 * WHAT IT DOES: Plays the frame list at $CBBC and sits at its owner's place (OBJ_WORK_D), removing itself if the
 * owner is in state 4; it plays the list at $CBC3 on the owner, and when that ends fires at the player at speed
 * $3C0 with its hit box on (2) and its state on, or removes itself unless the angle is $40 to $BF (Stage 07's
 * overlay).
 * HOW: StepObjectFrames on both slots; CopyObjectPlaceBack; AngleToPlayer, two compares, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ShotFromOwner0BFE(void)
{
    op_lda(0xBC);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCB71);
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
    jsr_push(0xCB81);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
    jsr_pop();
    WR8(ZP->richter_object, cpu.x);
    op_sxy();
    op_lda(0xC3);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCB8F);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_php();
    op_ldx(RD8(ZP->richter_object));
    op_plp();
    if (flag_z()) {
        jsr_push(0xCB98);
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
        jsr_push(0xCBB3);
        scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
        jsr_pop();
        op_lda(0x02);
        WR8(OBJ->hitbox_on[cpu.x], cpu.a);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
}

void abi_ShotFromOwner0BFE(void)
{
    ShotFromOwner0BFE();
}

/*
 * @routine 0BFE:3F2 FlyUntilOutside0BFE
 * @run 71:CBF2
 * WHAT IT DOES: Removes the object when its X or Y high byte is 2 or $FE; otherwise plays the frame list at $CBBC
 * and moves (Stage 07's overlay).
 * HOW: Four compares, StepObjectFrames, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlyUntilOutside0BFE(uint8_t slot)
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
    op_lda(0xBC);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCC1F);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_FlyUntilOutside0BFE(void)
{
    FlyUntilOutside0BFE(cpu.x);
}

/*
 * @routine 0BFE:44B FacePlayerInStates0BFE
 * @run 71:CC4B
 * WHAT IT DOES: In the states a table marks, turns the object to face the player (Stage 07's overlay).
 * HOW: A table read by the state; SideOfPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FacePlayerInStates0BFEStates = 0xCC5C,   /* nonzero for the states in which it faces the player */
};

void FacePlayerInStates0BFE(uint8_t slot)
{
    op_ldy(RD8(OBJ->state[slot]));
    op_lda(rd8((uint16_t)(FacePlayerInStates0BFEStates + cpu.y)));
    if (!flag_z()) {
        jsr_push(0xCC56);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
    }
}

void abi_FacePlayerInStates0BFE(void)
{
    FacePlayerInStates0BFE(cpu.x);
}

/*
 * @routine 0BFE:464 State6SoundWhenHit0BFE
 * @run 71:CC64
 * WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 6, sounds $13 and restarts its
 * animation in state 6, substate 0 (Stage 07's overlay).
 * HOW: A test of bit 7, a compare, SoundCommand, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void State6SoundWhenHit0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[slot]));
        op_cmp(0x06);
        if (!flag_c()) {
            op_lda(0x13);
            {
                uint8_t command;

                jsr_push(0xCC77);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x06);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State6SoundWhenHit0BFE(void)
{
    State6SoundWhenHit0BFE(cpu.x);
}

/*
 * @routine 0BFE:486 StartKind47Next0BFE
 * @run 71:CC86
 * WHAT IT DOES: Loads the record, starts kind $47 in the slot after the object (cleared first) and sets OBJ_WORK_H
 * $20 (Stage 07's overlay).
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKind47Next0BFE(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xCC89);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_txa();
    op_tay();
    op_iny();
    jsr_push(0xCC8F);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x47);
    jsr_push(0xCC94);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0x20);
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_StartKind47Next0BFE(void)
{
    StartKind47Next0BFE(cpu.x);
}

/*
 * @routine 0BFE:49A LandThenNearPlayer0BFE
 * @run 71:CC9A
 * WHAT IT DOES: Falls and lands with height $0C, and steps the state on once the player is less than $60 away in X
 * (Stage 07's overlay).
 * HOW: StopOnLanding, DistanceXToPlayer, a compare.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenNearPlayer0BFE(uint8_t slot)
{
    op_lda(0x0C);
    jsr_push(0xCC9F);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    jsr_push(0xCCA2);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x60);
    if (!flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandThenNearPlayer0BFE(void)
{
    LandThenNearPlayer0BFE(cpu.x);
}

/*
 * @routine 0BFE:4AA ListThenBoxOn0BFE
 * @run 71:CCAA
 * WHAT IT DOES: Plays the frame list at $CCC0; when it ends turns the hit box on (2) and steps the state on (Stage
 * 07's overlay).
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ListThenBoxOn0BFE(uint8_t slot)
{
    op_lda(0xC0);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCC);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCCB5);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_ListThenBoxOn0BFE(void)
{
    ListThenBoxOn0BFE(cpu.x);
}

/*
 * @routine 0BFE:4E2 StepWhileLanded0BFE
 * @run 71:CCE2
 * WHAT IT DOES: Plays the frame list at $CD06, taking an X speed by OBJ_WORK_E when the list's step reports carry,
 * falls and lands with height $0C, then clears the X speed (Stage 07's overlay).
 * HOW: StepObjectFrames, a table read, StopOnLanding, two stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    StepWhileLanded0BFESpeeds = 0xCD04,   /* the X speed high byte for each OBJ_WORK_E */
};

void StepWhileLanded0BFE(uint8_t slot)
{
    op_lda(0x06);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCD);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCCED);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_c()) {
        op_ldy(RD8(OBJ->work_e[slot]));
        op_lda(rd8((uint16_t)(StepWhileLanded0BFESpeeds + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
    }
    op_lda(0x0C);
    jsr_push(0xCCFD);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
}

void abi_StepWhileLanded0BFE(void)
{
    StepWhileLanded0BFE(cpu.x);
}

/*
 * @routine 0BFE:5FB SlideByFacing0BFE
 * @run 71:CDFB
 * WHAT IT DOES: Sets the X speed by the object's facing and falls and lands with height $0C (Stage 07's overlay).
 * HOW: A word table by the facing; StopOnLanding.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SlideByFacing0BFESpeeds = 0xCDF7,   /* the X speed for each facing, low and high byte */
};

void SlideByFacing0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->facing_left[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(SlideByFacing0BFESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(SlideByFacing0BFESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0x0C);
    jsr_push(0xCE11);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
}

void abi_SlideByFacing0BFE(void)
{
    SlideByFacing0BFE(cpu.x);
}

/*
 * @routine 0BFE:634 SlideAndSlow0BFE
 * @run 71:CE34
 * WHAT IT DOES: Moves at the X speed OBJ_WORK_I:OBJ_WORK_J, falling and landing with height $0C, and while that
 * speed is not 0 adds a step by facing to it (Stage 07's overlay).
 * HOW: StopOnLanding; a 16-bit add from a word table by the facing.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it; D as the caller left it. Exit: A, X, Y and
 * the flags as its last instruction or call left them.
 */
enum {
    SlideAndSlow0BFESteps = 0xCE66,   /* the speed step for each facing, low and high byte */
};

void SlideAndSlow0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0x0C);
    jsr_push(0xCE45);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    op_lda(RD8(OBJ->work_i[slot]));
    op_ora(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->facing_left[slot]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(RD8(OBJ->work_j[slot]));
        op_clc();
        op_adc(rd8((uint16_t)(SlideAndSlow0BFESteps + cpu.y)));
        WR8(OBJ->work_j[slot], cpu.a);
        op_lda(RD8(OBJ->work_i[slot]));
        op_adc(rd8((uint16_t)(SlideAndSlow0BFESteps + 1 + cpu.y)));
        WR8(OBJ->work_i[slot], cpu.a);
    }
}

void abi_SlideAndSlow0BFE(void)
{
    SlideAndSlow0BFE(cpu.x);
}

/*
 * @routine 0BFE:690 JumpAtPlayer0BFE
 * @run 71:CE90
 * WHAT IT DOES: Sets X and Y speeds by the player's distance in steps of 32 pixels, frame $89, and steps the
 * substate on, turning the X speed when facing left (Stage 07's overlay).
 * HOW: A four-byte record by DistanceXToPlayer over 32; NegateVelocityX.
 * WAITS: no.
 * Entry: X the object; A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    JumpAtPlayer0BFESpeeds = 0xCEC4,   /* X speed low, high, Y speed low, high by distance */
};

void JumpAtPlayer0BFE(uint8_t slot)
{
    jsr_push(0xCE93);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(JumpAtPlayer0BFESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtPlayer0BFESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtPlayer0BFESpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtPlayer0BFESpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(0x89);
    WR8(OBJ->frame[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xCEC3);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_JumpAtPlayer0BFE(void)
{
    JumpAtPlayer0BFE(cpu.x);
}

/*
 * @routine 0BFE:6E4 FrameBySignLand0BFE
 * @run 71:CEE4
 * WHAT IT DOES: Shows a frame by whether it is rising or falling, falls and lands with height $0C on its front
 * column, and steps the substate on when it lands (Stage 07's overlay).
 * HOW: The Y speed's sign bit indexes the frames; StopOnLandingFrontColumn.
 * WAITS: no.
 * Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FrameBySignLand0BFEFrames = 0xCEFD,   /* the frame falling, then rising */
};

void FrameBySignLand0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->vy_hi[slot]));
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(FrameBySignLand0BFEFrames + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x0C);
    jsr_push(0xCEF7);
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_FrameBySignLand0BFE(void)
{
    FrameBySignLand0BFE(cpu.x);
}

/*
 * @routine 0BFE:6FF ListThenState3_0BFE
 * @run 71:CEFF
 * WHAT IT DOES: Plays the frame list at $CF1B; when it ends restarts the animation in state 3, substate 0 (Stage
 * 07's overlay).
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ListThenState3_0BFE(uint8_t slot)
{
    op_lda(0x1B);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCF);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCF0A);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
}

void abi_ListThenState3_0BFE(void)
{
    ListThenState3_0BFE(cpu.x);
}

/*
 * @routine 0BFE:724 HopAtPlayer0BFE
 * @run 71:CF24
 * WHAT IT DOES: Sets an X speed by the player's distance in steps of 32 pixels, Y speed high $FB, frame $89, and
 * steps the substate on, turning the X speed when facing left (Stage 07's overlay).
 * HOW: A word table by DistanceXToPlayer over 32; NegateVelocityX.
 * WAITS: no.
 * Entry: X the object; A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HopAtPlayer0BFESpeeds = 0xCF50,   /* the X speed by distance, low and high byte */
};

void HopAtPlayer0BFE(uint8_t slot)
{
    jsr_push(0xCF27);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HopAtPlayer0BFESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(HopAtPlayer0BFESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(0x89);
    WR8(OBJ->frame[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xCF4F);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_HopAtPlayer0BFE(void)
{
    HopAtPlayer0BFE(cpu.x);
}

/*
 * @routine 0BFE:760 LandFrontColumn0BFE
 * @run 71:CF60
 * WHAT IT DOES: Falls and lands with height $0C on its front column, stepping the substate on when it lands (Stage
 * 07's overlay).
 * HOW: StopOnLandingFrontColumn, then INC on carry.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandFrontColumn0BFE(uint8_t slot)
{
    op_lda(0x0C);
    jsr_push(0xCF65);
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandFrontColumn0BFE(void)
{
    LandFrontColumn0BFE(cpu.x);
}

/*
 * @routine 0BFE:791 LandListThenWakeNext0BFE
 * @run 71:CF91
 * WHAT IT DOES: Falls and lands with height $0C and plays the frame list at $CFBC; when it ends turns the slot
 * after into kind $9C, state 0, sets sprite attribute bit 4, OBJ_WORK_D $70 and steps the substate on (Stage 07's
 * overlay).
 * HOW: StopOnLanding, StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandListThenWakeNext0BFE(uint8_t slot)
{
    op_lda(0x0C);
    jsr_push(0xCF96);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    op_lda(0xBC);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCF);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCFA1);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x9C);
        WR8(OBJ->kind[slot + 1], cpu.a);
        WR8(OBJ->state[slot + 1], 0);
        op_lda(RD8(OBJ->sprite_attr[slot]));
        op_ora(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        op_lda(0x70);
        WR8(OBJ->work_d[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandListThenWakeNext0BFE(void)
{
    LandListThenWakeNext0BFE(cpu.x);
}

/*
 * @routine 0BFE:7CD TurnDownTimed0BFE
 * @run 71:CFCD
 * WHAT IT DOES: Counts OBJ_WORK_D down, removing itself at 0; moves at angle OBJ_WORK_K at speed $120, turning it
 * by -2 a frame, and at $E0 sets it to $40 and steps the substate on (Stage 07's overlay).
 * HOW: DEC; SetVelocityFromAngle, ApplyVelocity; a subtract and a compare.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnDownTimed0BFE(uint8_t slot)
{
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(0x20);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_k[slot]));
    jsr_push(0xCFE3);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xCFE6);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_k[slot]));
    op_sec();
    op_sbc(0x02);
    WR8(OBJ->work_k[slot], cpu.a);
    op_cmp(0xE0);
    if (flag_z()) {
        op_lda(0x40);
        WR8(OBJ->work_k[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnDownTimed0BFE(void)
{
    TurnDownTimed0BFE(cpu.x);
}

/*
 * @routine 0BFE:7FC TurnUpTimed0BFE
 * @run 71:CFFC
 * WHAT IT DOES: Counts OBJ_WORK_D down, removing itself at 0; moves at angle OBJ_WORK_K at speed $120, turning it
 * by 2 a frame, and at $A0 sets it to $40 and steps the substate back (Stage 07's overlay).
 * HOW: DEC; SetVelocityFromAngle, ApplyVelocity; an add and a compare.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnUpTimed0BFE(uint8_t slot)
{
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(0x20);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_k[slot]));
    jsr_push(0xD012);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xD015);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_k[slot]));
    op_clc();
    op_adc(0x02);
    WR8(OBJ->work_k[slot], cpu.a);
    op_cmp(0xA0);
    if (flag_z()) {
        op_lda(0x40);
        WR8(OBJ->work_k[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_dec(v));
    }
}

void abi_TurnUpTimed0BFE(void)
{
    TurnUpTimed0BFE(cpu.x);
}

/*
 * @routine 0BFF:02B FollowPrevBoxByFrame0BFF
 * @run 71:D02B
 * WHAT IT DOES: In state 0 loads the record; then takes the place and facing of the slot before it and gives itself
 * the hit box of that slot's frame $86, $8A or $8B, or no box for any other frame (Stage 07's overlay).
 * HOW: CopyObjectPlaceBack; three compares pick a four-byte row.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FollowPrevBoxByFrame0BFFBoxes = 0xD077,   /* X offset, Y offset, half width, half height per row */
};

void FollowPrevBoxByFrame0BFF(uint8_t slot)
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
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xD039);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    op_lda(rd8((uint16_t)(COL(OBJ->facing_left) - 1 + slot)));
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(rd8((uint16_t)(COL(OBJ->frame) - 1 + slot)));
    op_cmp(0x86);
    if (!flag_z()) {
        op_cmp(0x8A);
        if (flag_z()) {
            goto at_D058;
        }
        op_cmp(0x8B);
        if (flag_z()) {
            goto at_D05C;
        }
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return;
    }
    op_cly();
    goto at_D05E;
at_D058:
    op_ldy(0x04);
    goto at_D05E;
at_D05C:
    op_ldy(0x08);
at_D05E:
    op_lda(rd8((uint16_t)(FollowPrevBoxByFrame0BFFBoxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowPrevBoxByFrame0BFFBoxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowPrevBoxByFrame0BFFBoxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowPrevBoxByFrame0BFFBoxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_FollowPrevBoxByFrame0BFF(void)
{
    FollowPrevBoxByFrame0BFF(cpu.x);
}

/*
 * @routine 0BFF:0AE LoadRecordDrawTiles09_0BFF
 * @run 71:D0AE
 * WHAT IT DOES: Loads the object's record and draws item 9 of tile script group 0 (Stage 07's overlay).
 * HOW: LoadObjectRecord, then DrawTileScript.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordDrawTiles09_0BFF(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD0B1);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x09);
    op_cly();
    {
        uint8_t item;
        uint8_t group;

        item = cpu.a;
        group = cpu.y;
        abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
    }
}

void abi_LoadRecordDrawTiles09_0BFF(void)
{
    LoadRecordDrawTiles09_0BFF();
}

/*
 * @routine 0BFF:0B7 BreakWallOnHit0BFF
 * @run 71:D0B7
 * WHAT IT DOES: With its X high byte not 0 draws item 9 of tile script group 0; otherwise on a hit of exactly 5
 * draws item $0A, sounds $19, starts three debris bursts (16 pixels up, then down 16 and right 4, then down 16 and
 * left 8), records its event and removes itself (Stage 07's overlay).
 * HOW: DrawTileScript; ApplyVelocityY and ApplyVelocity around StartDebrisBurst; RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BreakWallOnHit0BFF(void)
{
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (!flag_z()) {
        op_lda(0x09);
        op_cly();
        {
            uint8_t item;
            uint8_t group;

            item = cpu.a;
            group = cpu.y;
            abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
        }
        return;
    }
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (flag_z()) {
        op_lda(0x0A);
        op_cly();
        {
            uint8_t item;
            uint8_t group;

            jsr_push(0xD0CF);
            item = cpu.a;
            group = cpu.y;
            abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
            jsr_pop();
        }
        op_lda(0x19);
        {
            uint8_t command;

            jsr_push(0xD0D4);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(0xF0);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xD0DC);
        axis_move_exit_shim(ApplyVelocityY(cpu.x));
        jsr_pop();
        op_lda(0x01);
        call_routine(0xD0E1, 0xA90B);
        op_lda(0x04);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0x10);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xD0EE);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_lda(0x01);
        call_routine(0xD0F3, 0xA90B);
        op_lda(0xF8);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0x10);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xD100);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_lda(0x01);
        call_routine(0xD105, 0xA90B);
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xD108);
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
}

void abi_BreakWallOnHit0BFF(void)
{
    BreakWallOnHit0BFF();
}

/*
 * @routine 0BFF:115 TileSwapOnHit0BFF
 * @run 71:D115
 * WHAT IT DOES: In state 0 loads the record, sets the player's hit box Y offset to 4 and sets bit 7 of
 * OBJ_HITBOX_ON; then, on a hit below 3, sets OBJ_WORK_D 2 and a frame by OBJ_START_ARG, draws tile item $26 plus
 * OBJ_START_ARG the next frame, and once the hits stop draws item $1A plus OBJ_START_ARG with frame 0 (Stage 07's
 * overlay).
 * HOW: A three-value OBJ_WORK_D; compares of OBJ_HIT_TAKEN; DrawTileScript.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    TileSwapOnHit0BFFFrames = 0xD175,   /* the frame shown while hit for each OBJ_START_ARG */
};

void TileSwapOnHit0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD11D);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x04);
        WR8(OBJ->hitbox_dy[0x00], cpu.a);
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->hitbox_on[slot]));
        op_ora(0x80);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x02);
    if (flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        op_lda(RD8(OBJ->start_arg[slot]));
        op_clc();
        op_adc(0x26);
        op_cly();
        {
            uint8_t item;
            uint8_t group;

            jsr_push(0xD142);
            item = cpu.a;
            group = cpu.y;
            abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
            jsr_pop();
        }
    }
    op_lda(RD8(OBJ->hit_taken[slot]));
    if (!flag_z()) {
        op_cmp(0x03);
        if (!flag_c()) {
            op_lda(RD8(OBJ->work_d[slot]));
            if (!flag_z()) {
                goto at_D174;
            }
            op_lda(0x02);
            WR8(OBJ->work_d[slot], cpu.a);
            op_ldy(RD8(OBJ->start_arg[slot]));
            op_lda(rd8((uint16_t)(TileSwapOnHit0BFFFrames + cpu.y)));
            WR8(OBJ->frame[slot], cpu.a);
            return;
        }
    }
    op_lda(RD8(OBJ->work_d[slot]));
    if (!flag_z()) {
        WR8(OBJ->work_d[slot], 0);
        op_lda(RD8(OBJ->start_arg[slot]));
        op_clc();
        op_adc(0x1A);
        op_cly();
        {
            uint8_t item;
            uint8_t group;

            jsr_push(0xD171);
            item = cpu.a;
            group = cpu.y;
            abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
            jsr_pop();
        }
        WR8(OBJ->frame[slot], 0);
    }
at_D174:
    ;
}

void abi_TileSwapOnHit0BFF(void)
{
    TileSwapOnHit0BFF(cpu.x);
}

/*
 * @routine 0BFF:181 FlipPlayerByScroll0BFF
 * @run 71:D181
 * WHAT IT DOES: In state 0 loads the record; then sets the player's sprite flip to $80, or to 0 when OBJ_START_ARG
 * is set and the scroll is at screen 3 or more or its X low byte is $C8 or more (Stage 07's overlay).
 * HOW: Compares of the scroll; a store to slot 0.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlipPlayerByScroll0BFF(uint8_t slot)
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
    op_ldy(0x80);
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
        op_cmp(0x03);
        if (!flag_c()) {
            op_lda(RD8(SCROLL->x_lo[0x00]));
            op_cmp(0xC8);
            if (!flag_c()) {
                goto at_D19F;
            }
        }
        op_cly();
    }
at_D19F:
    WR8(OBJ->sprite_flip[0x00], cpu.y);
}

void abi_FlipPlayerByScroll0BFF(void)
{
    FlipPlayerByScroll0BFF(cpu.x);
}

/*
 * @routine 0BFF:1D4 GrabAndSinkPlayer0BFF
 * @run 71:D1D4
 * WHAT IT DOES: In state 0 loads the record and faces by OBJ_START_ARG's bit 0; then plays the frame list at $D25D:
 * in state 1 a hit below 3 sounds $AE and steps the state on; in state 2 clears the slot in OBJ_START_ARG, takes
 * the player's hit box off, gives both Y speed $180 and starts kind $A0 there at the player's place ($D220); later
 * states sink with the player ($D22F) (Stage 07's overlay).
 * HOW: Arms by the state; StepObjectFrames; ClearObjectInY; the falls into $D220.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void GrabAndSinkPlayer0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD1DC);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        op_and(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
        return;
    }
    op_pha();
    op_lda(0xD2);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x5D);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD1F1);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_pla();
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->hit_taken[slot]));
        if (flag_z()) {
            JustReturn0BFF_22E();
            return;
        }
        op_cmp(0x03);
        if (flag_c()) {
            JustReturn0BFF_22E();
            return;
        }
        op_lda(0xAE);
        {
            uint8_t command;

            jsr_push(0xD203);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        SinkWithPlayer0BFF();
        return;
    }
    op_ldy(RD8(OBJ->start_arg[slot]));
    jsr_push(0xD210);
    ClearObjectInY(cpu.y);
    jsr_pop();
    WR8(OBJ->hitbox_on[0x00], 0);
    op_lda(0x01);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    op_lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    /* falls into $D220 */
    StartKindA0AtPlayer0BFF();
}

void abi_GrabAndSinkPlayer0BFF(void)
{
    GrabAndSinkPlayer0BFF(cpu.x);
}

/*
 * @routine 0BFF:220 StartKindA0AtPlayer0BFF
 * @run 71:D220
 * WHAT IT DOES: Sets slot Y's Y speed low byte to A, starts kind $A0 in slot Y at the player's place and steps the
 * object's state on (Stage 07's overlay).
 * HOW: A store, StartObjectAtPlace with X 0, INC.
 * WAITS: no.
 * Entry: X the object; Y the slot; A the Y speed low byte. Exit: A, X, Y and the flags as its last instruction or
 * call left them.
 */
void StartKindA0AtPlayer0BFF(void)
{
    WR8(OBJ->vy_lo[cpu.y], cpu.a);
    op_phx();
    op_clx();
    op_lda(0xA0);
    jsr_push(0xD22A);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_plx();
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    /* falls into $D22E */
    JustReturn0BFF_22E();
}

void abi_StartKindA0AtPlayer0BFF(void)
{
    StartKindA0AtPlayer0BFF();
}

/*
 * @routine 0BFF:22E JustReturn0BFF_22E
 * @run 71:D22E
 * WHAT IT DOES: Returns at once (Stage 07's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void JustReturn0BFF_22E(void)
{
}

void abi_JustReturn0BFF_22E(void)
{
    JustReturn0BFF_22E();
}

/*
 * @routine 0BFF:22F SinkWithPlayer0BFF
 * @run 71:D22F
 * WHAT IT DOES: Moves the object and the slot in its OBJ_START_ARG by their Y speeds, sets the player's OBJ_WORK_E
 * $10 and OBJ_WORK_F 0, hands his frame, sprite set and facing to that slot and clears his frame, then puts him at
 * the object's place (Stage 07's overlay).
 * HOW: ApplyVelocityY on both slots; copies from slot 0; CopyObjectPlace to slot 0.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SinkWithPlayer0BFF(void)
{
    jsr_push(0xD232);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    op_sxy();
    jsr_push(0xD239);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_sxy();
    op_lda(0x10);
    WR8(OBJ->work_e[0x00], cpu.a);
    WR8(OBJ->work_f[0x00], 0);
    op_lda(RD8(OBJ->frame[0x00]));
    if (!flag_z()) {
        WR8(OBJ->frame[cpu.y], cpu.a);
        op_lda(RD8(OBJ->sprite_set[0x00]));
        WR8(OBJ->sprite_set[cpu.y], cpu.a);
    }
    op_lda(RD8(OBJ->facing_left[0x00]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    WR8(OBJ->frame[0x00], 0);
    op_cly();
    copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
}

void abi_SinkWithPlayer0BFF(void)
{
    SinkWithPlayer0BFF();
}

/*
 * @routine 0BFF:266 LoadOrPlayList0BFF
 * @run 71:D266
 * WHAT IT DOES: In state 0 loads the object's record; in later states plays the frame list at $D279 (Stage 07's
 * overlay).
 * HOW: A test of the state; LoadObjectRecord or StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadOrPlayList0BFF(uint8_t slot)
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
    op_lda(0xD2);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x79);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
    }
}

void abi_LoadOrPlayList0BFF(void)
{
    LoadOrPlayList0BFF(cpu.x);
}

/*
 * @routine 0BFF:307 ClearCarry0BFF
 * @run 71:D307
 * WHAT IT DOES: Returns with the carry clear (Stage 07's overlay).
 * HOW: CLC.
 * WAITS: no.
 * Entry: nothing.. Exit: C clear; A, X, Y and the other flags kept.
 */
void ClearCarry0BFF(void)
{
    op_clc();
}

void abi_ClearCarry0BFF(void)
{
    ClearCarry0BFF();
}

/*
 * @routine 0BFF:309 StandOnUnlessSide2_0BFF
 * @run 71:D309
 * WHAT IT DOES: Tests the player's contact with the object (width $11, height $1E), unless SideOfPlayer reports bit
 * 1 (Stage 07's overlay).
 * HOW: SideOfPlayer, then ContactWithPlayer or CLC.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the player stands on it.
 */
void StandOnUnlessSide2_0BFF(void)
{
    jsr_push(0xD30C);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x02);
    if (!flag_z()) {
        ClearCarry0BFF();
        return;
    }
    op_lda(0x11);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x1E);
    WR8(ZP->temp09, cpu.a);
    run_routine(0xAA35);
}

void abi_StandOnUnlessSide2_0BFF(void)
{
    StandOnUnlessSide2_0BFF();
}

/*
 * @routine 0BFF:31B OpenWhenPartnerReady0BFF
 * @run 71:D31B
 * WHAT IT DOES: In state 0 loads the record and, when zField8F bit 7 is set, goes straight to its open place (frame
 * $1E, $D369); in state 1 waits for the slot in OBJ_START_ARG to reach state 2, then sets zField8F bit 7 and steps
 * the state on; later states run on at $D343 (Stage 07's overlay).
 * HOW: Arms by the state; an OR into zField8F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void OpenWhenPartnerReady0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD323);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        if ((RD8(ZP->field8f) & 0x80) == 0) {
            JustReturn0BFF_342();
            return;
        }
        op_lda(0x1E);
        WR8(OBJ->frame[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
        SetRow7EAndDraw0BFF(slot);
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        DropToRow7E0BFF(slot);
        return;
    }
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x02);
    if (!flag_z()) {
        JustReturn0BFF_342();
        return;
    }
    RMW8(ZP->field8f, v, v | 0x80);
    RMW8(OBJ->state[slot], v, f_inc(v));
    /* falls into $D342 */
    JustReturn0BFF_342();
}

void abi_OpenWhenPartnerReady0BFF(void)
{
    OpenWhenPartnerReady0BFF(cpu.x);
}

/*
 * @routine 0BFF:342 JustReturn0BFF_342
 * @run 71:D342
 * WHAT IT DOES: Returns at once (Stage 07's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void JustReturn0BFF_342(void)
{
}

void abi_JustReturn0BFF_342(void)
{
    JustReturn0BFF_342();
}

/*
 * @routine 0BFF:343 DropToRow7E0BFF
 * @run 71:D343
 * WHAT IT DOES: In state 2 plays the frame list at $D391 and falls, its Y speed growing by $28 a frame, until its Y
 * low byte reaches $7E ($D369); in later states checks its tiles ($D377) (Stage 07's overlay).
 * HOW: DEC of the state in A; StepObjectFrames, a 16-bit add, ApplyVelocity, a compare.
 * WAITS: no.
 * Entry: X the object; A the state less 1. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DropToRow7E0BFF(uint8_t slot)
{
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        RedrawIfTileChanged0BFF();
        return;
    }
    op_lda(0xD3);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x91);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD351);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_clc();
    op_adc(0x28);
    WR8(OBJ->vy_lo[slot], cpu.a);
    if (flag_c()) {
        RMW8(OBJ->vy_hi[slot], v, f_inc(v));
    }
    jsr_push(0xD362);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_lo[slot]));
    op_cmp(0x7E);
    if (!flag_c()) {
        JustReturn0BFF_342();
        return;
    }
    /* falls into $D369 */
    SetRow7EAndDraw0BFF(slot);
}

void abi_DropToRow7E0BFF(void)
{
    DropToRow7E0BFF(cpu.x);
}

/*
 * @routine 0BFF:369 SetRow7EAndDraw0BFF
 * @run 71:D369
 * WHAT IT DOES: Sets Y low $7E, steps the state on and draws item $0D of tile script group 0 (Stage 07's overlay).
 * HOW: Stores, then the fall into $D371.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetRow7EAndDraw0BFF(uint8_t slot)
{
    op_lda(0x7E);
    WR8(OBJ->y_lo[slot], cpu.a);
    RMW8(OBJ->state[slot], v, f_inc(v));
    /* falls into $D371 */
    DrawTiles0D0BFF();
}

void abi_SetRow7EAndDraw0BFF(void)
{
    SetRow7EAndDraw0BFF(cpu.x);
}

/*
 * @routine 0BFF:371 DrawTiles0D0BFF
 * @run 71:D371
 * WHAT IT DOES: Draws item $0D of tile script group 0 (Stage 07's overlay).
 * HOW: DrawTileScript.
 * WAITS: no.
 * Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DrawTiles0D0BFF(void)
{
    op_lda(0x0D);
    op_cly();
    {
        uint8_t item;
        uint8_t group;

        item = cpu.a;
        group = cpu.y;
        abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
    }
}

void abi_DrawTiles0D0BFF(void)
{
    DrawTiles0D0BFF();
}

/*
 * @routine 0BFF:377 RedrawIfTileChanged0BFF
 * @run 71:D377
 * WHAT IT DOES: With the scroll at screen 0, reads the VRAM word at $0394 and, unless its low byte is $D0, draws
 * item $0D of tile script group 0 (Stage 07's overlay).
 * HOW: SelectVramReadAddress, SelectVramData, a compare.
 * WAITS: no.
 * Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RedrawIfTileChanged0BFF(void)
{
    op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
    if (!flag_z()) {
        JustReturn0BFF_342();
        return;
    }
    jsr_push(0xD37F);
    SelectVramReadAddress();
    cpu.a = STAGE_C_VDC_MARR;
    abi_nz(STAGE_C_VDC_MARR);
    jsr_pop();
    op_st1(0x94);
    op_st2(0x03);
    jsr_push(0xD386);
    SelectVramData();
    cpu.a = STAGE_C_VDC_VRAM;
    abi_nz(STAGE_C_VDC_VRAM);
    jsr_pop();
    op_lda(RD8(VDC->data_lo));
    op_ldy(RD8(VDC->data_hi));
    op_cmp(0xD0);
    if (!flag_z()) {
        DrawTiles0D0BFF();
        return;
    }
}

void abi_RedrawIfTileChanged0BFF(void)
{
    RedrawIfTileChanged0BFF();
}

/*
 * @routine 0BFF:39A AwaitHitThenList0BFF
 * @run 71:D39A
 * WHAT IT DOES: In state 0 loads the record and, when zField8F bit 7 is set, takes frame $21 and goes to state 2;
 * in state 1 a hit of 5 or more steps the state on; in state 2 plays the frame list at $D3D3 and steps the state on
 * when it ends (Stage 07's overlay).
 * HOW: Arms by the state; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AwaitHitThenList0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD3A2);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        if ((RD8(ZP->field8f) & 0x80) == 0) {
            goto at_D3BE;
        }
        op_lda(0x21);
        WR8(OBJ->frame[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_D3BF;
    }
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x05);
    if (!flag_c()) {
        JustReturn0BFF_342();
        return;
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
at_D3BE:
    return;
at_D3BF:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_D3BE;
    }
    op_lda(0xD3);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xD3);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD3CD);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (!flag_z()) {
        goto at_D3BE;
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_AwaitHitThenList0BFF(void)
{
    AwaitHitThenList0BFF(cpu.x);
}

/*
 * @routine 0BFF:42A CheckTileAndPartner0BFF
 * @run 71:D42A
 * WHAT IT DOES: Reads the VRAM word at $0418: unless its low byte is $19, reports whether the slot in OBJ_START_ARG
 * is empty (Stage 07's overlay).
 * HOW: SelectVramReadAddress, SelectVramData, a compare; a test of the slot's kind.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the tile is not $19 and the slot's kind is 0; otherwise clear.
 */
void CheckTileAndPartner0BFF(uint8_t slot)
{
    jsr_push(0xD42D);
    SelectVramReadAddress();
    cpu.a = STAGE_C_VDC_MARR;
    abi_nz(STAGE_C_VDC_MARR);
    jsr_pop();
    op_st1(0x18);
    op_st2(0x04);
    jsr_push(0xD434);
    SelectVramData();
    cpu.a = STAGE_C_VDC_VRAM;
    abi_nz(STAGE_C_VDC_VRAM);
    jsr_pop();
    op_lda(RD8(VDC->data_lo));
    op_ldy(RD8(VDC->data_hi));
    op_cmp(0x19);
    if (!flag_z()) {
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (flag_z()) {
            op_sec();
            return;
        }
    }
    op_clc();
}

void abi_CheckTileAndPartner0BFF(void)
{
    CheckTileAndPartner0BFF(cpu.x);
}

/*
 * @routine 0BFF:471 LoadRecordScreenY0BFF
 * @run 71:D471
 * WHAT IT DOES: Loads the object's record and takes the scroll's Y low byte off its Y low byte (Stage 07's
 * overlay).
 * HOW: LoadObjectRecord, a negate and an add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void LoadRecordScreenY0BFF(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD474);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(SCROLL->y_lo[0x00]));
    op_eor(0xFF);
    cpu.a = f_inc(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->y_lo[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_LoadRecordScreenY0BFF(void)
{
    LoadRecordScreenY0BFF(cpu.x);
}

/*
 * @routine 0BFF:482 SecretWallOnHit0BFF
 * @run 71:D482
 * WHAT IT DOES: With PlayerDoorReady set and a hit below 3, when the VRAM word at $05BE has low byte $17, steps
 * zField8C on, sets zSubStep 3, starts list channel $85 on channel 6 and steps the state on (Stage 07's overlay).
 * HOW: Compares; SelectVramReadAddress, SelectVramData; StartListChannelKind2.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SecretWallOnHit0BFF(void)
{
    op_lda(RD8(PVARS->door_ready));
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_taken[cpu.x]));
        if (!flag_z()) {
            op_cmp(0x03);
            if (!flag_c()) {
                jsr_push(0xD493);
                SelectVramReadAddress();
                cpu.a = STAGE_C_VDC_MARR;
                abi_nz(STAGE_C_VDC_MARR);
                jsr_pop();
                op_st1(0xBE);
                op_st2(0x05);
                jsr_push(0xD49A);
                SelectVramData();
                cpu.a = STAGE_C_VDC_VRAM;
                abi_nz(STAGE_C_VDC_VRAM);
                jsr_pop();
                op_lda(RD8(VDC->data_lo));
                op_ldy(RD8(VDC->data_hi));
                op_cmp(0x17);
                if (flag_z()) {
                    RMW8(ZP->field8c, v, f_inc(v));
                    op_lda(0x03);
                    WR8(ZP->sub_step, cpu.a);
                    op_phx();
                    op_lda(0x85);
                    op_ldx(0x06);
                    jsr_push(0xD4B2);
                    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
                    jsr_pop();
                    op_plx();
                    RMW8(OBJ->state[cpu.x], v, f_inc(v));
                }
            }
        }
    }
}

void abi_SecretWallOnHit0BFF(void)
{
    SecretWallOnHit0BFF();
}

/*
 * @routine 0BFF:4F8 CopyEightVramRows0BFF
 * @run 71:D4F8
 * WHAT IT DOES: Copies eight runs of 8 VRAM words: the source addresses from the list A's low three bits pick, the
 * destinations from a table turned within each group of four by A's upper bits (Stage 07's overlay).
 * HOW: A loop of ClearVdcControlHigh, read and write addresses and eight word copies through the VDC.
 * WAITS: no.
 * Entry: A the list (bits 0 to 2) and the turn (bits 3 up). Exit: A, X, Y and the flags as its last instruction or
 * call left them.
 */
enum {
    CopyEightVramRows0BFFDestinations = 0xD569,   /* the eight VRAM destination addresses, low and high byte */
    CopyEightVramRows0BFFLists = 0xD579,   /* the source list address for each value of A's low bits, low and high */
};

void CopyEightVramRows0BFF(void)
{
    op_pha();
    op_and(0x07);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(CopyEightVramRows0BFFLists + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(CopyEightVramRows0BFFLists + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    op_pla();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    WR8(ZP->temp10, cpu.a);
    do {
        jsr_push(0xD512);
        ClearVdcControlHigh();
        cpu.a = 0;
        abi_nz(0);
        jsr_pop();
        jsr_push(0xD515);
        SelectVramReadAddress();
        cpu.a = STAGE_C_VDC_MARR;
        abi_nz(STAGE_C_VDC_MARR);
        jsr_pop();
        op_lda(RD8_PTR(ZP->temp00));
        WR8(VDC->data_lo, cpu.a);
        op_ldy(0x01);
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        WR8(VDC->data_hi, cpu.a);
        jsr_push(0xD524);
        SelectVramWriteAddress();
        cpu.a = STAGE_C_VDC_MAWR;
        jsr_pop();
        op_lda(RD8(ZP->temp02));
        op_and(0xFC);
        WR8(ZP->temp03, cpu.a);
        op_lda(RD8(ZP->temp02));
        op_clc();
        op_adc(RD8(ZP->temp10));
        op_and(0x03);
        op_ora(RD8(ZP->temp03));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(CopyEightVramRows0BFFDestinations + cpu.y)));
        WR8(VDC->data_lo, cpu.a);
        op_lda(rd8((uint16_t)(CopyEightVramRows0BFFDestinations + 1 + cpu.y)));
        WR8(VDC->data_hi, cpu.a);
        jsr_push(0xD544);
        SelectVramData();
        cpu.a = STAGE_C_VDC_VRAM;
        abi_nz(STAGE_C_VDC_VRAM);
        jsr_pop();
        op_ldy(0x08);
        do {
            op_lda(RD8(VDC->data_lo));
            WR8(VDC->data_lo, cpu.a);
            op_lda(RD8(VDC->data_hi));
            WR8(VDC->data_hi, cpu.a);
            op_dey();
        } while (!flag_z());
        op_lda(0x02);
        op_clc();
        op_adc(RD8(ZP->temp00));
        WR8(ZP->temp00, cpu.a);
        if (flag_c()) {
            RMW8(ZP->temp01, v, f_inc(v));
        }
        RMW8(ZP->temp02, v, f_inc(v));
        op_lda(RD8(ZP->temp02));
        op_cmp(0x08);
    } while (!flag_z());
}

void abi_CopyEightVramRows0BFF(void)
{
    CopyEightVramRows0BFF();
}

/*
 * @routine 0BFF:609 LoadRecordFlagPartner0BFF
 * @run 71:D609
 * WHAT IT DOES: In state 0 loads the object's record and, when ProgressCharacter bit 1 is set, sets OBJ_WORK_E $0F
 * in the slot in its OBJ_START_ARG (Stage 07's overlay).
 * HOW: LoadObjectRecord; a test of bit 1; a store to slot Y.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordFlagPartner0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD611);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(rd8((uint16_t)(ProgressCharacter)));
        op_and(0x02);
        if (!flag_z()) {
            op_ldy(RD8(OBJ->start_arg[slot]));
            op_lda(0x0F);
            WR8(OBJ->work_e[cpu.y], cpu.a);
        }
    }
}

void abi_LoadRecordFlagPartner0BFF(void)
{
    LoadRecordFlagPartner0BFF(cpu.x);
}
