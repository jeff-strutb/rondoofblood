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
 * HOW: SaveAndMapStageBanksLong, BlendStageRowPair twice with X pushed around them, a tail jump into RestoreStageBanksLong.
 * WAITS: no.
 * Entry: X kept, Y the second row of the first blend. Exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them.
 */
Ended BlendStageRowsOfPlayer(uint8_t slot, uint8_t y)
{
    TileRowsExit blend;
    Sum mapped;
    uint8_t flag, row_a, row_b, bank;
    Ended e;

    jsr_push(0x9184);
    mapped = SaveAndMapStageBanks();
    jsr_pop();
    e = ended_with_a(ended_with_overflow(ended_with_carry(ended_nz((uint8_t)(mapped.value + 1)), mapped.carry), mapped.overflow),
                     (uint8_t)(mapped.value + 1));
    flag = RD8(STAGEVARS->blend_on[1]);
    e = ended_then(e, ended_load(flag));
    if (flag == 0) {
        push8(slot);
        WR8(STAGEVARS->blend_on[0], 0);
        row_a = RD8(ZP->score_add.lo);
        e = ended_then(e, ended_load(row_a));
        jsr_push(0x9194);
        blend = BlendStageRowPair(3, row_a, y);
        jsr_pop();
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), blend.a), true), blend.v), 5));
        row_b = RD8(ZP->score_add.hi);
        e = ended_then(e, ended_load(row_b));
        jsr_push(0x919B);
        blend = BlendStageRowPair(5, row_b, 5);
        jsr_pop();
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), blend.a), true), blend.v), 5));
        slot = pull8();
        e = ended_then(e, ended_nz(slot));
    }
    bank = RestoreStageBanks();
    return ended_then(e, ended_load(bank));
}

void abi_BlendStageRowsOfPlayer(void)
{
    abi_ended(BlendStageRowsOfPlayer(cpu.x, cpu.y));
}
