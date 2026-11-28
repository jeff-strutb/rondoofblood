/* The stage row blend the overlays at $9181 run: two row pairs blended with the stage's banks mapped. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_enemies_l7.h"
#include "enemy_objects.h"

/*
 * @routine 05BF:181 BlendStageRowsOfPlayer
 * @run 6B:9181
 * WHAT IT DOES: Maps the stage's banks and, unless the byte at $366F (StageVars blend_on[1]) is set, clears the blend step $366E and blends the two row pairs the bytes zScoreAddLo and zScoreAddHi name
 * (steps 3 and 5); then puts the banks back.
 * HOW: SaveAndMapStageBanksLong, BlendStageRowPair twice with X pushed around them, a tail jump into RestoreStageBanksLong. The
 * body is blend_rows_of_player_with, which 024F's group's copy at the same address (BlendStageScoreAddRows) runs with its bank
 * routines at $933F and $9354.
 * WAITS: no.
 * Entry: X kept, Y the second row of the first blend. Exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them.
 */
/* The arms BlendStageRowsOfPlayer ends on. */
enum { BR_BLEND_OFF, BR_BLENDED };

ObjArm blend_rows_of_player_with(uint8_t slot, uint8_t y, const StageBanks *banks)
{
    return blend_rows_of_player_at(slot, y, banks, 0x9181, BlendStageRowPair);
}

/* The body, at the address it runs at (its JSRs return to at + 3, + $13 and + $1A), with the C of its row pair blend:
 * the builds of the overlay that hold these instructions at another address (stage_programs_census.c) pass their own. */
ObjArm blend_rows_of_player_at(uint8_t slot, uint8_t y, const StageBanks *banks, uint16_t at,
                               TileRowsExit (*row_pair)(uint8_t, uint8_t, uint8_t))
{
    ObjArm r = { 0 };
    TileRowsExit blend;

    jsr_push((uint16_t)(at + 3));
    r.sum = banks->save_and_map();
    jsr_pop();
    r.first = RD8(STAGEVARS->blend_on[1]);
    r.arm = r.first == 0 ? BR_BLENDED : BR_BLEND_OFF;
    if (r.arm == BR_BLENDED) {
        push8(slot);
        WR8(STAGEVARS->blend_on[0], 0);
        r.second = RD8(ZP->score_add.lo);
        jsr_push((uint16_t)(at + 0x13));
        blend = row_pair(3, r.second, y);
        jsr_pop();
        r.third = blend.a;
        r.flag = blend.v;
        r.fourth = RD8(ZP->score_add.hi);
        jsr_push((uint16_t)(at + 0x1A));
        blend = row_pair(5, r.fourth, 5);
        jsr_pop();
        r.fifth = blend.a;
        r.other = blend.v;
        r.sixth = pull8();
    }
    r.seventh = banks->restore();
    return r;
}

ObjArm BlendStageRowsOfPlayer(uint8_t slot, uint8_t y)
{
    return blend_rows_of_player_with(slot, y, &stage_banks_05bf);
}

/* BlendStageRowsOfPlayer's exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them. */
Ended blend_stage_rows_of_player_exit_shim(ObjArm r)
{
    Ended e = ended_with_a(ended_with_overflow(ended_with_carry(ended_nz((uint8_t)(r.sum.value + 1)), r.sum.carry), r.sum.overflow),
                           (uint8_t)(r.sum.value + 1));

    e = ended_then(e, ended_load(r.first));
    if (r.arm == BR_BLENDED) {
        e = ended_then(e, ended_load(r.second));
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), r.third), true), r.flag), 5));
        e = ended_then(e, ended_load(r.fourth));
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), r.fifth), true), r.other), 5));
        e = ended_then(e, ended_nz(r.sixth));
    }
    return ended_then(e, ended_load(r.seventh));
}

/* BlendStageRowsOfPlayer's end as a step: A the bank restored; with a blend, C 1 and V the second pair's; with none, C and V
 * the bank add's. */
BlendStepEnd blend_rows_of_player_step(ObjArm r)
{
    BlendStepEnd step;

    step.a = r.seventh;
    step.y_set = r.arm == BR_BLENDED;
    if (r.arm == BR_BLENDED) {
        step.c = true;
        step.v = r.other;
    } else {
        step.c = r.sum.carry;
        step.v = r.sum.overflow;
    }
    return step;
}

void abi_BlendStageRowsOfPlayer(void)
{
    abi_ended(blend_stage_rows_of_player_exit_shim(BlendStageRowsOfPlayer(cpu.x, cpu.y)));
}
