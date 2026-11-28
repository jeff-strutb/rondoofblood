/*
 * Routines of the overlays of stages 06 to 08 whose entries the code map also decodes a byte or two off (jump tables
 * read past their end): each place here was settled from the bytes as the real instruction stream.
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
#include "stage0608_entries_c.h"

/*
 * @routine 0A8E:0FB ReloadStageEntryAndScroll0A8E
 * @run 6B:88FB
 * WHAT IT DOES: Reloads the stage's entry with the scroll's place kept: the screen and X low byte of the scroll
 * into zBiosAl and zBiosAh, the scroll's tables zeroed, the player's sprite flip cleared, the entry loaded
 * (zField8E kept across it), the bank mapped and the scroll set up; where the raster effect's table says so, the
 * tables are zeroed and the entry loaded again with the effect's scroll; at the end the tables are zeroed and the
 * entry loaded a last time (Stage 06's $8000 overlay; stages 07 and 08 hold the same bytes here).
 * HOW: TAI from the kernel's zero bytes; the calls to the routines loaded at $8952, $80B7, $A94C and $A9B1, each to
 * the one the overlay holds; a table read by zRasterEffect.
 * WAITS: no.
 * Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ReloadStageEntryAndScroll0A8ERasterReload = 0x88CE,   /* nonzero for the raster effects that load twice */
};

void ReloadStageEntryAndScroll0A8E(void)
{
    op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
    WR8(ZP->bios_a.lo, cpu.a);
    op_lda(RD8(SCROLL->x_lo[0x00]));
    WR8(ZP->bios_a.hi, cpu.a);
    op_tai(KernelZeros, ADDR_OF(SCROLL->x_frac[0x00]), 0x0030);
    op_tai(KernelZeros, ADDR_OF(SCROLLE->f3e66[0x00]), 0x0088);
    WR8(OBJ->sprite_flip[0x00], 0);
    op_lda(RD8(ZP->field8e));
    op_pha();
    call_routine(0x891B, 0x8952);
    op_pla();
    WR8(ZP->field8e, cpu.a);
    call_routine(0x8921, 0x80B7);
    call_routine(0x8924, 0xA94C);
    op_ldy(RD8(ZP->raster_effect));
    op_lda(rd8((uint16_t)(ReloadStageEntryAndScroll0A8ERasterReload + cpu.y)));
    if (!flag_z()) {
        op_tai(KernelZeros, ADDR_OF(SCROLL->x_frac[0x00]), 0x0030);
        op_tai(KernelZeros, ADDR_OF(SCROLLE->f3e66[0x00]), 0x0088);
        op_lda(RD8(ZP->field8e));
        op_pha();
        call_routine(0x893E, 0x8952);
        op_pla();
        WR8(ZP->field8e, cpu.a);
        call_routine(0x8944, 0xA9B1);
    }
    op_tai(KernelZeros, ADDR_OF(SCROLL->x_frac[0x00]), 0x0030);
    op_tai(KernelZeros, ADDR_OF(SCROLLE->f3e66[0x00]), 0x0088);
    /* falls into $8952 */
    run_routine(0x8952);
}

void abi_ReloadStageEntryAndScroll0A8E(void)
{
    ReloadStageEntryAndScroll0A8E();
}

/*
 * @routine 0A9A:0BC DebrisFlightStep0A9A
 * @run 70:A8BC
 * WHAT IT DOES: One frame of a debris piece's flight: sound $19 and a burst, then OBJ_WORK_D counts the flight's
 * steps; until the count of its record (OBJ_WORK_E) it moves by the step's speeds; then it sets its OBJ_START_ARG's
 * bit in zField8F, turns into the record's next kind and stops (Stage 06's overlay).
 * HOW: SoundCommand, StartDebrisBurst, table reads by the record and the step, ApplyVelocity; BitMaskFromIndex.
 * WAITS: no.
 * Entry: X the object; Y as the caller left it (the sound command keeps it). Exit: A, X, Y and the flags as its
 * last instruction or call left them.
 */
enum {
    DebrisFlightStep0A9ASteps = 0xB677,   /* the number of steps of each record */
    DebrisFlightStep0A9AKinds = 0xB67C,   /* the kind each record turns into */
    DebrisFlightStep0A9ASpeeds = 0xB67D,   /* X and Y speed high bytes per step */
};

void DebrisFlightStep0A9A(void)
{
    op_lda(0x19);
    {
        uint8_t command;

        jsr_push(0xA8C1);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
        jsr_pop();
    }
    op_lda(0x01);
    call_routine(0xA8C6, 0xA90B);
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_cmp(rd8((uint16_t)(DebrisFlightStep0A9ASteps + cpu.y)));
    if (!flag_c()) {
        op_lda(RD8(OBJ->work_d[cpu.x]));
        cpu.a = f_dec(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_clc();
        op_adc(RD8(OBJ->work_e[cpu.x]));
        op_tay();
        op_lda(rd8((uint16_t)(DebrisFlightStep0A9ASpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(DebrisFlightStep0A9ASpeeds + 1 + cpu.y)));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xA8ED);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        return;
    }
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    call_routine(0xA8F4, 0xA855);
    op_ora(RD8(ZP->field8f));
    WR8(ZP->field8f, cpu.a);
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    op_lda(rd8((uint16_t)(DebrisFlightStep0A9AKinds + cpu.y)));
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
    WR8(OBJ->vx_hi[cpu.x], 0);
    WR8(OBJ->vy_hi[cpu.x], 0);
}

void abi_DebrisFlightStep0A9A(void)
{
    DebrisFlightStep0A9A();
}

/*
 * @routine 0A9E:125 LoadWaitAndClearWork0A9E
 * @run 71:C925
 * WHAT IT DOES: Loads the object's record, sets a wait of $50 in OBJ_WORK_D and clears OBJ_WORK_E and OBJ_WORK_F
 * (Stage 06's overlay).
 * HOW: LoadObjectRecord, three stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadWaitAndClearWork0A9E(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC928);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x50);
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
}

void abi_LoadWaitAndClearWork0A9E(void)
{
    LoadWaitAndClearWork0A9E(cpu.x);
}

/*
 * @routine 0BFA:0BC DebrisFlightStep0BFA
 * @run 70:A8BC
 * WHAT IT DOES: One frame of a debris piece's flight: sound $19 and a burst, then OBJ_WORK_D counts the flight's
 * steps; until the count of its record (OBJ_WORK_E) it moves by the step's speeds; then it sets its OBJ_START_ARG's
 * bit in zField8F, turns into the record's next kind and stops (Stage 07's overlay).
 * HOW: SoundCommand, StartDebrisBurst, table reads by the record and the step, ApplyVelocity; BitMaskFromIndex.
 * WAITS: no.
 * Entry: X the object; Y as the caller left it (the sound command keeps it). Exit: A, X, Y and the flags as its
 * last instruction or call left them.
 */
enum {
    DebrisFlightStep0BFASteps = 0xB036,   /* the number of steps of each record */
    DebrisFlightStep0BFAKinds = 0xB03B,   /* the kind each record turns into */
    DebrisFlightStep0BFASpeeds = 0xB03C,   /* X and Y speed high bytes per step */
};

void DebrisFlightStep0BFA(void)
{
    op_lda(0x19);
    {
        uint8_t command;

        jsr_push(0xA8C1);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
        jsr_pop();
    }
    op_lda(0x01);
    call_routine(0xA8C6, 0xA90B);
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_cmp(rd8((uint16_t)(DebrisFlightStep0BFASteps + cpu.y)));
    if (!flag_c()) {
        op_lda(RD8(OBJ->work_d[cpu.x]));
        cpu.a = f_dec(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_clc();
        op_adc(RD8(OBJ->work_e[cpu.x]));
        op_tay();
        op_lda(rd8((uint16_t)(DebrisFlightStep0BFASpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(DebrisFlightStep0BFASpeeds + 1 + cpu.y)));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        jsr_push(0xA8ED);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        return;
    }
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    call_routine(0xA8F4, 0xA855);
    op_ora(RD8(ZP->field8f));
    WR8(ZP->field8f, cpu.a);
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    op_lda(rd8((uint16_t)(DebrisFlightStep0BFAKinds + cpu.y)));
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
    WR8(OBJ->vx_hi[cpu.x], 0);
    WR8(OBJ->vy_hi[cpu.x], 0);
}

void abi_DebrisFlightStep0BFA(void)
{
    DebrisFlightStep0BFA();
}

/*
 * @routine 0D5C:113 FollowYWithOffset0D5C
 * @run 70:B913
 * WHAT IT DOES: Takes the slot OBJ_WORK_D plus OBJ_WORK_E names into Y, then follows it as FollowYAndCount0D5C does
 * (Stage 08's overlay).
 * HOW: An add into Y; it runs into FollowYAndCount0D5C.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowYWithOffset0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    op_clc();
    op_adc(RD8(OBJ->work_e[slot]));
    op_tay();
    /* falls into $B91B */
    run_routine(0xB91B);
}

void abi_FollowYWithOffset0D5C(void)
{
    FollowYWithOffset0D5C(cpu.x);
}
